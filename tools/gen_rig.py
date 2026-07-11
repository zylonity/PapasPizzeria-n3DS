#!/usr/bin/env python3
"""Generate romfs:/rig/customer.rig (LE binary) from the SWF + papas_extract.
Shared rig: 15 slots, segment labels, baked per-frame matrices.
Per type: part offsets/sizes/imageIndex/frameCount (12 parts).
Also emits gfx/<type>.t3s for the requested types."""
import sys, os, zlib, struct, glob, re

# Atlas art is pre-downscaled so each on-screen context samples its texture
# ~1:1: raw JPEXS exports are full Flash size, and GPU minification of the
# baked-in antialiased edge makes silhouettes stair-step (Roy's sprites avoid
# this by being exported at display size). Two variants per type: "" for the
# take-order close-up (scale 0.7) and "_line" for the lobby lines (0.32; the
# waitline's 0.27 samples that at a gentle 0.84). The .rig keeps ORIGINAL
# pixel sizes; the runtime already stretches each image into its (w,h) quad.
ART_SCALES=[("",0.7),("_line",0.32)]

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
SWF  = os.path.join(ROOT, "papaspizzeria_v2.swf")
SPR  = os.path.join(ROOT, "papas_extract/sprites")
OUT  = os.path.join(ROOT, "romfs/rig/customer.rig")

# ---------------- SWF core ----------------
raw = open(SWF, "rb").read(); assert raw[0:3] == b"CWS"
body = zlib.decompress(raw[8:])

class R:
    def __init__(s,d,p=0): s.d=d;s.p=p;s.bb=0;s.bc=0
    def align(s): s.bb=0;s.bc=0
    def u8(s): v=s.d[s.p];s.p+=1;return v
    def u16(s): v=struct.unpack_from("<H",s.d,s.p)[0];s.p+=2;return v
    def u32(s): v=struct.unpack_from("<I",s.d,s.p)[0];s.p+=4;return v
    def bits(s,n):
        v=0
        for _ in range(n):
            if s.bc==0: s.bb=s.d[s.p];s.p+=1;s.bc=8
            v=(v<<1)|((s.bb>>7)&1);s.bb=(s.bb<<1)&0xFF;s.bc-=1
        return v
    def sbits(s,n):
        v=s.bits(n)
        if n and (v>>(n-1))&1: v-=(1<<n)
        return v
    def string0(s):
        st=s.p
        while s.d[s.p]!=0: s.p+=1
        v=s.d[st:s.p].decode("latin1");s.p+=1;return v

def rect(r):
    r.align();nb=r.bits(5)
    xmin=r.sbits(nb);xmax=r.sbits(nb);ymin=r.sbits(nb);ymax=r.sbits(nb);r.align()
    return (xmin,xmax,ymin,ymax)
def matrix(r):
    r.align();a=d=1.0;b=c=0.0
    if r.bits(1): nb=r.bits(5);a=r.sbits(nb)/65536.0;d=r.sbits(nb)/65536.0
    if r.bits(1): nb=r.bits(5);b=r.sbits(nb)/65536.0;c=r.sbits(nb)/65536.0
    nb=r.bits(5);tx=r.sbits(nb);ty=r.sbits(nb);r.align()
    return (a,b,c,d,tx/20.0,ty/20.0)
def cxform_skip(r):
    r.align();hasAdd=r.bits(1);hasMult=r.bits(1);nb=r.bits(4)
    if hasMult:
        for _ in range(4): r.sbits(nb)
    if hasAdd:
        for _ in range(4): r.sbits(nb)
    r.align()
def parse_tags(d,start,end):
    r=R(d,start);out=[]
    while r.p<end:
        rec=r.u16();code=rec>>6;ln=rec&0x3F
        if ln==0x3F: ln=r.u32()
        out.append((code,d[r.p:r.p+ln]));r.p+=ln
        if code==0: break
    return out

r=R(body);rect(r);r.u16();r.u16()
tags=parse_tags(body,r.p,len(body))

SHAPE={2,22,32,83}
shape_bounds={}; sprite_tag={}
for code,tb in tags:
    if code in SHAPE:
        sid=struct.unpack_from("<H",tb,0)[0]; rr=R(tb,2); shape_bounds[sid]=rect(rr)
    elif code==39:
        sid=struct.unpack_from("<H",tb,0)[0]; sprite_tag[sid]=tb

def apply(m,x,y):
    a,b,c,d,tx,ty=m; return (a*x+c*y+tx, b*x+d*y+ty)

# Registration = union RENDER bounds over ALL frames of a symbol, which is
# exactly the canvas JPEXS crops its PNG exports to. SWF SHAPEBOUNDS won't do:
# they include bezier control points, so curvy shapes (eyes/mouth/hands)
# declare a bigger box than they render. JPEXS's own render bounds leak into
# the shape SVG exports: each papas_extract/shapes/<id>.svg has the render
# size in width/height and the origin offset in its root <g> translate
# (render xmin,ymin = -tx,-ty). Verified: sizes match the exported PNG canvas
# for all 416 limb symbols, offsets match SWF bounds wherever those were tight.
svg_size_re=re.compile(r'height="([0-9.]+)px" width="([0-9.]+)px"')
svg_g_re=re.compile(r'<g transform="matrix\(([-0-9.]+), ([-0-9.]+), ([-0-9.]+), ([-0-9.]+), ([-0-9.]+), ([-0-9.]+)\)">')
_svg_cache={}
def shape_svg_bounds(sid):
    """(xmin,ymin,xmax,ymax) px in shape space, or None if no SVG export."""
    if sid in _svg_cache: return _svg_cache[sid]
    res=None
    p=os.path.join(ROOT,f"papas_extract/shapes/{sid}.svg")
    if os.path.isfile(p):
        svg=open(p).read()
        m=svg_size_re.search(svg); g=svg_g_re.search(svg)
        if m and g:
            h,w=float(m.group(1)),float(m.group(2))
            tx,ty=float(g.group(5)),float(g.group(6))
            res=(-tx,-ty,-tx+w,-ty+h)
    _svg_cache[sid]=res; return res

_cache={}
def union_bounds(cid, depth=0):
    """(xmin,ymin,xmax,ymax) px: union render bounds across every frame."""
    if cid in _cache: return _cache[cid]
    if depth>14: return None
    res=shape_svg_bounds(cid)
    if res is None and cid in sprite_tag:
        inner=parse_tags(sprite_tag[cid],4,len(sprite_tag[cid]))
        cur={}  # depth -> (childid, matrix)  persists across frames
        xmin=ymin=1e18; xmax=ymax=-1e18; ok=False
        def accumulate():
            nonlocal xmin,xmax,ymin,ymax,ok
            for dp,(ch,mt) in cur.items():
                if ch is None: continue
                cb=union_bounds(ch,depth+1)
                if not cb: continue
                x0,y0,x1,y1=cb
                for (px,py) in [(x0,y0),(x1,y0),(x0,y1),(x1,y1)]:
                    wx,wy=apply(mt,px,py)
                    xmin=min(xmin,wx);xmax=max(xmax,wx);ymin=min(ymin,wy);ymax=max(ymax,wy);ok=True
        for code,tb in inner:
            if code==26:
                rr=R(tb,0);flags=rr.u8()
                hasChar=flags&2;hasMat=flags&4;hasCx=flags&8;hasRatio=flags&16;hasName=flags&32
                dp=rr.u16()
                ch=rr.u16() if hasChar else cur.get(dp,(None,None))[0]
                mt=matrix(rr) if hasMat else cur.get(dp,(None,(1,0,0,1,0,0)))[1]
                cur[dp]=(ch,mt)
            elif code==28:
                dp=struct.unpack_from("<H",tb,0)[0]; cur.pop(dp,None)
            elif code==1:  # ShowFrame -> accumulate current display list
                accumulate()
        accumulate()
        res=(xmin,ymin,xmax,ymax) if ok else None
    _cache[cid]=res; return res

# ---------------- clip 409 baked animation ----------------
clip=sprite_tag[409]
inner=parse_tags(clip,4,len(clip))
SLOT_ORDER=[]   # depth-sorted slot names
depth_name={}
labels={}
frames=[]; cur={}; frame=0
for code,tb in inner:
    if code==43:
        nul=tb.index(0); labels[frame]=tb[:nul].decode("latin1")
    elif code==1:
        frames.append(dict(cur)); frame+=1
    elif code==26:
        rr=R(tb,0);flags=rr.u8()
        hasChar=flags&2;hasMat=flags&4;hasCx=flags&8;hasRatio=flags&16;hasName=flags&32
        dp=rr.u16()
        if hasChar: rr.u16()
        mt=matrix(rr) if hasMat else None
        if hasCx: cxform_skip(rr)
        if hasRatio: rr.u16()
        nm=rr.string0() if hasName else None
        if nm is not None: depth_name[dp]=nm
        if mt is not None: cur[dp]=mt
    elif code==28:
        dp=struct.unpack_from("<H",tb,0)[0]; cur.pop(dp,None)

depths_sorted=sorted(depth_name)
SLOT_ORDER=[depth_name[d] for d in depths_sorted]     # back->front draw order
NUMFRAMES=len(frames); NUMSLOTS=len(SLOT_ORDER)
IDENT=(1,0,0,1,0,0)

# ---------------- parts model ----------------
# "logo" (last) is not a slot: it's the 2-frame shirt-logo child inside each
# body clip. Frame 2 is just frame 1 pre-mirrored so the logo reads correctly
# when the whole customer is x-flipped (leave line); the runtime redraws frame
# 1 mirrored about its own center instead, so only one image is stored.
PARTS=["body","head","neck","upperarm","forearm","foot","hair","back_hair","mouth","eyes","hand","hand2","logo"]
PARTIDX={p:i for i,p in enumerate(PARTS)}
# slot name -> part name
SLOT2PART={
 "body":"body","head":"head","neck":"neck","hair":"hair","back_hair":"back_hair",
 "eyes":"eyes","mouth":"mouth",
 "front_upperarm":"upperarm","back_upperarm":"upperarm",
 "front_forearm":"forearm","back_forearm":"forearm",
 "front_shoe":"foot","back_shoe":"foot",
 "fronthand":"hand","backhand":"hand2",
}

# discover per-type symbol ids from folder names: DefineSprite_<id>_customer<type>_<part>
folder_re=re.compile(r"DefineSprite_(\d+)_customer(\d+)_(.+)$")
type_part_dir={}   # (type,part) -> dir
type_part_id ={}
for d in glob.glob(os.path.join(SPR,"DefineSprite_*_customer*")):
    m=folder_re.match(os.path.basename(d))
    if not m: continue
    sid=int(m.group(1)); typ=int(m.group(2)); part=m.group(3)
    if part in PARTIDX:
        type_part_dir[(typ,part)]=d; type_part_id[(typ,part)]=sid

TYPES=sorted({t for (t,_) in type_part_dir})

# ---------------- shirt logo (child of each body clip) ----------------
# body sprite = body shape + a 2-frame child sprite named "logo". JPEXS bakes
# logo frame 1 into the exported body PNG; here we pull the logo's own bitmap
# (embedded in the frame-1 shape's SVG export) + its placement so the runtime
# can redraw it mirrored while the customer walks away flipped.
import base64
LOGO_DIR=os.path.join(ROOT,"gfx/rig_logos")
def sprite_children(sid):
    """[(depth, childId, name, (a,b,c,d,tx,ty))] from PlaceObject2 tags."""
    out=[]
    if sid not in sprite_tag: return out
    for code,tb in parse_tags(sprite_tag[sid],4,len(sprite_tag[sid])):
        if code!=26: continue
        rr=R(tb,0);flags=rr.u8()
        dp=rr.u16()
        ch=rr.u16() if flags&2 else None
        mt=matrix(rr) if flags&4 else None
        if flags&8: cxform_skip(rr)
        if flags&16: rr.u16()
        nm=rr.string0() if flags&32 else None
        out.append((dp,ch,nm,mt))
    return out
svg_img_re=re.compile(r'xlink:href="data:image/PNG;base64,([A-Za-z0-9+/=\s]+)"')
def extract_logo(typ):
    """-> (ox,oy,w,h,pngpath) or None. w/h = display px (image is drawn scaled)."""
    body_sid=type_part_id.get((typ,"body"))
    if body_sid is None: return None
    logo=[(ch,mt) for (_,ch,nm,mt) in sprite_children(body_sid) if nm=="logo"]
    if not logo: return None
    logo_sid,logo_mt=logo[0]
    shapes=[ch for (_,ch,_,_) in sprite_children(logo_sid) if ch is not None]
    if not shapes: return None
    svgp=os.path.join(ROOT,f"papas_extract/shapes/{shapes[0]}.svg")
    if not os.path.isfile(svgp): return None
    svg=open(svgp).read()
    m=svg_size_re.search(svg); mi=svg_img_re.search(svg)
    if not m or not mi: return None
    h,w=float(m.group(1)),float(m.group(2))
    os.makedirs(LOGO_DIR,exist_ok=True)
    pngp=os.path.join(LOGO_DIR,f"customer{typ}_logo.png")
    open(pngp,"wb").write(base64.b64decode(mi.group(1)))
    tx,ty=logo_mt[4],logo_mt[5]
    return (round(tx),round(ty),round(w),round(h),pngp)
def png_frames(d):
    return sorted(glob.glob(os.path.join(d,"*.png")),
                  key=lambda p:int(re.search(r"(\d+)\.png$",p).group(1)))
def png_dim(p):
    with open(p,"rb") as f: f.seek(16); return struct.unpack(">II",f.read(8))

# per-type part meta
type_meta={}   # type -> list over PARTS of dict(ox,oy,w,h,frameCount, dir, imageIndex)
for t in TYPES:
    imgindex=0; meta=[]
    for part in PARTS:
        if part=="logo":
            lg=extract_logo(t)
            if lg is None:
                meta.append(dict(ox=0,oy=0,w=0,h=0,fc=0,idx=0,dir=None,png=None)); continue
            ox,oy,w,h,pngp=lg
            meta.append(dict(ox=ox,oy=oy,w=w,h=h,fc=1,idx=imgindex,dir=None,png=pngp))
            imgindex+=1; continue
        d=type_part_dir.get((t,part)); sid=type_part_id.get((t,part))
        if d is None:
            meta.append(dict(ox=0,oy=0,w=0,h=0,fc=0,idx=0,dir=None)); continue
        pngs=png_frames(d); fc=len(pngs)
        if fc==0:
            meta.append(dict(ox=0,oy=0,w=0,h=0,fc=0,idx=0,dir=None)); continue
        w,h=png_dim(pngs[0])
        b=union_bounds(sid)
        if b:
            ox=round(b[0]); oy=round(b[1])
        else:
            ox=-(w//2); oy=-(h//2)   # empty symbol (1x1 png): center, moot
        meta.append(dict(ox=ox,oy=oy,w=w,h=h,fc=fc,idx=imgindex,dir=d))
        imgindex+=fc
    type_meta[t]=meta

# ---------------- segments ----------------
lab=sorted(labels.items())
SEGMENTS=[]
LOOPING={"walk","stand","takeorder"}
for i,(f,name) in enumerate(lab):
    end=lab[i+1][0] if i+1<len(lab) else NUMFRAMES
    SEGMENTS.append((name,f,end-f,1 if name in LOOPING else 0))

# ---------------- expression track ----------------
# Eyes/mouth/hands/feet sub-frames are driven per timeline frame by
# `<slot>.clip.gotoAndStop(N)` calls in clip 409's frame scripts (JPEXS folder
# frame_<K> holds the DoAction for 0-indexed frame K-1). Bake the current
# sub-frame (0-indexed) for every slot, every frame, by playing the timeline
# linearly and holding each value until it changes.
SCRIPTS_409=os.path.join(ROOT,"papas_extract/scripts/DefineSprite_409_customer")
goto_re=re.compile(r"([A-Za-z_]+)\.clip\.gotoAndStop\((\d+)\)")
expr_state={nm:0 for nm in SLOT_ORDER}
expr_frames=[]
for f in range(NUMFRAMES):
    folder=os.path.join(SCRIPTS_409,f"frame_{f+1}")
    if os.path.isdir(folder):
        for asf in sorted(glob.glob(os.path.join(folder,"*.as"))):
            for m in goto_re.finditer(open(asf).read()):
                slot,val=m.group(1),int(m.group(2))
                if slot in expr_state: expr_state[slot]=max(0,val-1)
    expr_frames.append([expr_state[nm] for nm in SLOT_ORDER])

# ---------------- write binary ----------------
os.makedirs(os.path.dirname(OUT),exist_ok=True)
def s16(x): return struct.pack("<h",int(x))
def u16(x): return struct.pack("<H",int(x))
def cstr(x,n):
    b=x.encode("latin1")[:n-1]; return b+b"\0"*(n-len(b))

buf=bytearray()
buf+=b"PRIG"+struct.pack("<I",4)   # v2: per-frame expression track; v3: logo part; v4: matrices 4-byte aligned
buf+=u16(NUMSLOTS)+u16(NUMFRAMES)+u16(len(SEGMENTS))+u16(len(TYPES))+u16(len(PARTS))+u16(30)
buf+=b"\0"*(32-len(buf))                       # pad header to 32
for nm in SLOT_ORDER:
    buf+=cstr(nm,16)+struct.pack("<BB",PARTIDX[SLOT2PART[nm]],0)
for (nm,st,ln,lp) in SEGMENTS:
    buf+=cstr(nm,16)+u16(st)+u16(ln)+struct.pack("<BB",lp,0)
# v4: pad so the float matrix block is 4-byte aligned in the file. The
# runtime reads it through a raw float*, and VFP loads on the real ARM11
# data-abort on unaligned addresses (emulators don't check).
buf+=b"\0"*(-len(buf)%4)
# matrices baked [frame][slot][6]
mflat=bytearray()
for fr in frames:
    for d in depths_sorted:
        a,b,c,dd,tx,ty=fr.get(d,IDENT)
        mflat+=struct.pack("<6f",a,b,c,dd,tx,ty)
buf+=mflat
# type table: type id then 12 parts
for t in TYPES:
    buf+=u16(t)
    for part in PARTS:
        m=type_meta[t][PARTIDX[part]]
        buf+=s16(m["ox"])+s16(m["oy"])+u16(m["w"])+u16(m["h"])+u16(m["idx"])+u16(m["fc"])
# expression track: uint8[frame][slot] sub-frame index
for fr in expr_frames:
    for v in fr:
        buf+=struct.pack("<B",min(max(v,0),255))
open(OUT,"wb").write(buf)

print(f"slots({NUMSLOTS}) draw order:", SLOT_ORDER)
print(f"frames={NUMFRAMES} segments={len(SEGMENTS)} types={len(TYPES)}")
for s in SEGMENTS: print("  seg", s)
print(f"wrote {OUT}  ({len(buf)} bytes)")

# ---------------- emit t3s (+ scaled art) for requested types ----------------
want=[int(x) for x in sys.argv[2:]] if len(sys.argv)>2 else []
GFX=os.path.join(ROOT,"gfx")
STAGE=os.path.join(GFX,"rig_parts")

def alpha_bleed(px, known, iters=6):
    """Flood RGB from 'known' texels outward into the rest (alpha untouched):
    the GPU bilinear filter blends edge pixels with their transparent
    neighbors' RGB, so leaving black there puts a dark halo around limbs."""
    import numpy as np
    rgb=px[...,:3]
    known=known.copy()
    for _ in range(iters):
        if known.all(): break
        acc=np.zeros_like(rgb); cnt=np.zeros(known.shape,np.float32)
        for dy in (-1,0,1):
            for dx in (-1,0,1):
                if dy==0 and dx==0: continue
                sk=np.roll(np.roll(known,dy,0),dx,1)
                sr=np.roll(np.roll(rgb, dy,0),dx,1)
                acc+=sr*sk[...,None]; cnt+=sk
        fill=(~known)&(cnt>0)
        rgb[fill]=acc[fill]/cnt[fill,None]
        known|=fill
    return px

def stage_scaled(src, dst, w, h):
    """Lanczos-downscale src to (w,h) at dst: premultiplied for the resize
    (no fringe pull-in), then alpha-bled for the GPU's straight-alpha bilinear.
    The whole pipeline stays in float: quantizing the premultiplied image to
    8-bit first turns rounding noise at near-zero-alpha texels into saturated
    white specks when unpremultiplied (noise/alpha clips to 255)."""
    from PIL import Image
    import numpy as np
    if os.path.isfile(dst) and os.path.getmtime(dst) >= os.path.getmtime(src):
        return
    im=Image.open(src).convert("RGBA")
    px=np.asarray(im).astype(np.float32)
    a=px[...,3:4]/255.0
    prem=np.concatenate([px[...,:3]*a, px[...,3:4]],axis=-1)
    small=np.stack([np.asarray(Image.fromarray(prem[...,c],mode="F")
                               .resize((w,h),Image.LANCZOS)) for c in range(4)],axis=-1)
    np.clip(small,0,255,out=small)      # Lanczos ringing can under/overshoot
    sa=small[...,3:4]
    # Unpremultiply only where alpha carries real signal; texels fainter than
    # that get their RGB from the bleed pass below (their alpha stays as-is).
    solid=sa[...,0]>8.0
    small[...,:3]=np.where(solid[...,None],
                           np.clip(small[...,:3]*255.0/np.maximum(sa,1e-6),0,255), 0)
    small=alpha_bleed(small, solid)
    Image.fromarray(small.astype(np.uint8)).save(dst)

def scaled_size(w,h,scale):
    return max(1,round(w*scale)), max(1,round(h*scale))
for t in want:
    os.makedirs(STAGE,exist_ok=True)
    for suffix,scale in ART_SCALES:
        # --border edge: 1px replicated border per image, else bilinear filtering
        # bleeds neighboring atlas sprites in as flickering lines around the limbs
        # (must be the long form: tex3ds 2.3.0's parser rejects the short -b)
        lines=["--atlas --border edge -f rgba -z auto",""]
        for part in PARTS:
            m=type_meta[t][PARTIDX[part]]
            if part=="logo":
                if m.get("png"):
                    sw,sh=scaled_size(m["w"],m["h"],scale)  # logo w,h = display size
                    dst=os.path.join(STAGE,f"customer{t}_logo{suffix}.png")
                    stage_scaled(m["png"],dst,sw,sh)
                    lines.append(os.path.relpath(dst, GFX))
                continue
            d=m["dir"]
            if d is None: continue
            sw,sh=scaled_size(m["w"],m["h"],scale)          # uniform canvas per part
            for i,p in enumerate(png_frames(d)):
                dst=os.path.join(STAGE,f"customer{t}_{part}_{i+1}{suffix}.png")
                stage_scaled(p,dst,sw,sh)
                lines.append(os.path.relpath(dst, GFX))
        path=os.path.join(GFX,f"customer{t}{suffix}.t3s")
        open(path,"w").write("\n".join(lines)+"\n")
        print(f"wrote {path} ({sum(type_meta[t][PARTIDX[p]]['fc'] for p in PARTS)} images)")
