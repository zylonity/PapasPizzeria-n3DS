#!/usr/bin/env python3
"""Read customer.rig back and draw customer 1's standing pose."""
import struct, sys
d=open(sys.argv[1],"rb").read()
p=0
magic=d[0:4]; ver=struct.unpack_from("<I",d,4)[0]
nSlots,nFrames,nSeg,nTypes,nParts,fps=struct.unpack_from("<6H",d,8)
assert magic==b"PRIG"
p=32
slots=[]
for _ in range(nSlots):
    nm=d[p:p+16].split(b"\0")[0].decode(); part,_=struct.unpack_from("<BB",d,p+16); p+=18
    slots.append((nm,part))
segs=[]
for _ in range(nSeg):
    nm=d[p:p+16].split(b"\0")[0].decode(); st,ln,lp,_=struct.unpack_from("<HHBB",d,p+16); p+=22
    segs.append((nm,st,ln,lp))
# Rig v4 pads the raw matrix block up to four bytes for real ARM11 VFP loads
if ver>=4:
    p=(p+3)&~3
mat_off=p
mat_stride=nSlots*6*4
p+=nFrames*mat_stride
PARTS=["body","head","neck","upperarm","forearm","foot","hair","back_hair","mouth","eyes","hand","hand2"]
types={}
for _ in range(nTypes):
    tid=struct.unpack_from("<H",d,p)[0]; p+=2
    parts=[]
    for _ in range(nParts):
        ox,oy,w,h,idx,fc=struct.unpack_from("<hhHHHH",d,p); p+=12
        parts.append(dict(ox=ox,oy=oy,w=w,h=h,idx=idx,fc=fc))
    types[tid]=parts
print(f"magic={magic} ver={ver} slots={nSlots} frames={nFrames} segs={nSeg} types={nTypes} parts={nParts} fps={fps}")
print("consumed",p,"of",len(d),"bytes")

def slot_mat(frame,slot):
    o=mat_off+frame*mat_stride+slot*24
    return struct.unpack_from("<6f",d,o)

# composite stand frame (44) for type 1
STAND=[s for s in segs if s[0]=="stand"][0][1]
print(f"\n--- type 1, frame {STAND} ('stand') composited limb boxes ---")
minx=miny=1e9;maxx=maxy=-1e9
for si,(nm,part) in enumerate(slots):
    pm=types[1][part]
    if pm["fc"]==0:
        print(f"  {nm:16s} part={PARTS[part]:10s} ABSENT"); continue
    a,b,c,dd,tx,ty=slot_mat(STAND,si)
    ox,oy,w,h=pm["ox"],pm["oy"],pm["w"],pm["h"]
    corners=[(ox,oy),(ox+w,oy),(ox,oy+h),(ox+w,oy+h)]
    xs=[a*x+c*y+tx for x,y in corners]; ys=[b*x+dd*y+ty for x,y in corners]
    x0,x1,y0,y1=min(xs),max(xs),min(ys),max(ys)
    minx=min(minx,x0);maxx=max(maxx,x1);miny=min(miny,y0);maxy=max(maxy,y1)
    print(f"  {nm:16s} part={PARTS[part]:9s} img{pm['idx']:>3}x{pm['fc']:<2} world=({x0:6.1f},{y0:6.1f})..({x1:6.1f},{y1:6.1f})")
print(f"\ncustomer overall AABB: ({minx:.1f},{miny:.1f}) .. ({maxx:.1f},{maxy:.1f})  size {maxx-minx:.1f} x {maxy-miny:.1f}")
