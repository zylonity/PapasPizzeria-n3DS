#!/usr/bin/env python3
# Convert the original intro cutscene (DefineSprite_2930_intro_master,
# 1171 frames @ 30fps on the 600x450 stage: the delivery car drives by, Roy
# drives the truck, arrives at the pizzeria, walks in through the dark and
# finds Papa's note) into 3DS assets, replacing the prerecorded ready.ogv.
#
# Unlike startofday/giveorder this is NOT stored as frame patches: the SWF
# timeline is replayed as LAYERS. Each top-level placement run becomes a
# handful of atlas images (child sprites use their JPEXS PNG exports, bare
# shapes/morphshapes are rasterised from their SVG exports) plus per-sampled-
# frame affine matrices + tints. That keeps the whole 39s cutscene in a few
# sheets and gives every layer its own stereo plane for the 3D effect.
#
# Emits:
#   gfx/Intro/*.png + gfx/intro_<n>.t3s   (atlas sheets)
#   source/Papas_IntroFrames.h            (layer/draw/frame tables)
# Prints a reconstruction-vs-reference diff for a few sample frames.
#
# Usage: python3 tools/gen_intro.py [repo root]

import io
import os
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cairosvg
from swf_core import Swf, R, read_matrix, read_cxform, parse_tags, mat_mul, IDENT_CX

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
SRC = os.path.join(ROOT, "papas_extract/sprites/DefineSprite_2930_intro_master")
GFX = os.path.join(ROOT, "gfx")
STAGE_DIR = os.path.join(GFX, "Intro")
HEADER = os.path.join(ROOT, "source/Papas_IntroFrames.h")
SCRATCH = os.environ.get("INTRO_SCRATCH")  # side-by-side diffs land here if set

INTRO = 2930
FPS = 30
SAMPLE_STEP = 2          # stored at 15fps, runtime lerps back to smooth
STAGE_W, STAGE_H = 600, 450
SCALE = 240.0 / 450.0    # dst stage is 320x240, centred on the top screen
# The game places intro_master at the stage centre, so layer coordinates are
# CENTRE-origin: the visible stage is sprite (-300..300, -225..225), which
# lands at dst (-160..160, -120..120). The runtime anchors that at the middle
# of the top screen (INTRO_X/Y = 200,120).
SHEET_MAX_AREA = 720 * 1024    # leaves tex3ds packing slack in a 1024x1024 sheet

# The original clip bakes in its own Flash-era UI; the port has its own.
BLACKLIST = {
    2829,  # "SKIP INTRO?" mouse-over overlay
    2833,  # papalouie.com watermark text
}

# Stereo plane per char (into-the-screen units for Papas::Stereo::plane).
# Hand-tuned for the hero layers; anything unlisted falls back to its depth
# rank within the frame (backgrounds deep, top layers near the glass).
PLANES = {
    2816: 0.95, 2817: 0.70, 2818: 0.70, 2819: 0.70,             # shot 1 street
    2825: 0.42, 2828: 0.42, 2834: 0.40,                          # delivery car
    2836: 0.95, 2837: 0.85, 2838: 0.80, 2840: 0.75, 2841: 0.70,  # shot 2 scenery
    2843: 0.70, 2844: 0.65, 2855: 0.70, 2856: 0.85, 2857: 0.70,
    2853: 0.30, 2854: 0.10,                                      # Roy / cab frame
    2858: 0.95, 2821: 0.45, 2822: 0.45, 2823: 0.35, 2824: 0.35,  # shot 3 arrival
    2860: 0.45,
    2861: 0.95, 2863: 0.90, 2865: 0.50, 2867: 0.50,              # dark walk
    2869: 0.80, 2871: 0.80,
}

swf = Swf(ROOT)
ub = swf.union_bounds(INTRO)
CX0, CY0 = -ub[0], -ub[1]  # stage (0,0) in the JPEXS export canvas


# ---------------- master timeline -> placement runs ----------------
# A run = one char occupying one depth for a continuous span of frames.
class Run:
    def __init__(self, depth, char, born):
        self.depth = depth; self.char = char; self.born = born
        self.frames = {}   # master frame -> (matrix, cxform, childframe)
        self.id = None

def walk_master():
    tb = swf.sprite_tag[INTRO]
    inner = parse_tags(tb, 4, len(tb))
    frame = 0
    cur = {}    # depth -> [run, matrix, cxform]
    runs = []
    for code, t in inner:
        if code == 1:
            for dp, (run, mt, cx) in cur.items():
                cf = swf.child_frame(run.char, frame - run.born) \
                    if swf.kind.get(run.char) == "sprite" else 1
                run.frames[frame] = (mt, cx, cf)
            frame += 1
        elif code == 26:
            rr = R(t, 0); flags = rr.u8(); dp = rr.u16()
            prev = cur.get(dp)
            ch = rr.u16() if flags & 2 else (prev[0].char if prev else None)
            mt = read_matrix(rr) if flags & 4 else (prev[1] if prev else (1, 0, 0, 1, 0, 0))
            cx = read_cxform(rr) if flags & 8 else (prev[2] if prev else IDENT_CX)
            if flags & 16: rr.u16()  # morph ratio: rendered at its start state
            if ch is None: continue
            if flags & 2 or prev is None:
                run = Run(dp, ch, frame)
                runs.append(run)
                cur[dp] = [run, mt, cx]
            else:
                cur[dp][1] = mt; cur[dp][2] = cx
        elif code == 28:
            import struct as _s
            dp = _s.unpack_from("<H", t, 0)[0]; cur.pop(dp, None)
    runs = [r for r in runs if r.char not in BLACKLIST and r.frames]
    return runs, frame

runs, n_frames = walk_master()
print(f"{len(runs)} placement runs over {n_frames} frames")


# ---------------- raster scale per char ----------------
def mat_scale(m):
    a, b, c, d, _, _ = m
    return max((a * a + b * b) ** 0.5, (c * c + d * d) ** 0.5)

char_scale = {}
for r in runs:
    s = max(mat_scale(f[0]) for f in r.frames.values()) * SCALE
    char_scale[r.char] = max(char_scale.get(r.char, 0.0), s)
for ch in char_scale:
    cap = 2.0 if swf.kind.get(ch) in ("shape", "morph") else 1.0  # PNGs are 1:1
    char_scale[ch] = min(char_scale[ch], cap)


# ---------------- image rendering (same halo-safe path as gen_rig) ----------
def alpha_bleed(px, known, iters=6):
    rgb = px[..., :3]
    known = known.copy()
    for _ in range(iters):
        if known.all():
            break
        acc = np.zeros_like(rgb)
        cnt = np.zeros(known.shape, np.float32)
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if dy == 0 and dx == 0:
                    continue
                sk = np.roll(np.roll(known, dy, 0), dx, 1)
                sr = np.roll(np.roll(rgb, dy, 0), dx, 1)
                acc += sr * sk[..., None]
                cnt += sk
        fill = (~known) & (cnt > 0)
        rgb[fill] = acc[fill] / cnt[fill, None]
        known |= fill
    return px

def downscale(px, w, h):
    a = px[..., 3:4] / 255.0
    prem = np.concatenate([px[..., :3] * a, px[..., 3:4]], axis=-1)
    small = np.stack([np.asarray(Image.fromarray(prem[..., c], mode="F")
                                 .resize((w, h), Image.LANCZOS)) for c in range(4)], axis=-1)
    np.clip(small, 0, 255, out=small)
    sa = small[..., 3:4]
    solid = sa[..., 0] > 8.0
    small[..., :3] = np.where(solid[..., None],
                              np.clip(small[..., :3] * 255.0 / np.maximum(sa, 1e-6), 0, 255), 0)
    return small.astype(np.uint8)

def render_scaled(im, rs):
    px = np.asarray(im.convert("RGBA")).astype(np.float32)
    px = alpha_bleed(px, px[..., 3] > 16)
    w = max(1, int(round(im.width * rs)))
    h = max(1, int(round(im.height * rs)))
    return Image.fromarray(downscale(px, w, h), "RGBA")

def sprite_dir(cid):
    p = os.path.join(ROOT, f"papas_extract/sprites/DefineSprite_{cid}")
    if os.path.isdir(p): return p
    for d in os.listdir(os.path.join(ROOT, "papas_extract/sprites")):
        if d.startswith(f"DefineSprite_{cid}_"):
            return os.path.join(ROOT, "papas_extract/sprites", d)
    raise FileNotFoundError(cid)

# image key = (char, childframe) -> {"img": PIL, "anchor": (x0,y0), "rs": s}
images = {}
def image_for(char, childframe):
    key = (char, childframe)
    if key in images: return key
    rs = char_scale[char]
    kind = swf.kind.get(char)
    if kind == "sprite":
        src = Image.open(os.path.join(sprite_dir(char), f"{childframe}.png"))
        b = swf.union_bounds(char)
        img = render_scaled(src, rs)
    else:
        png = cairosvg.svg2png(url=swf.svg_path(char), scale=max(rs, 0.05))
        src = Image.open(io.BytesIO(png))
        b = swf.shape_bounds(char)
        # cairosvg already rendered at rs; just clean the fringe
        px = np.asarray(src.convert("RGBA")).astype(np.float32)
        px = alpha_bleed(px, px[..., 3] > 16)
        img = Image.fromarray(px.astype(np.uint8), "RGBA")
    images[key] = {"img": img, "anchor": (b[0], b[1]), "rs": rs}
    return key

used_adds = set()
for r in runs:
    for f, (mt, cx, cf) in r.frames.items():
        image_for(r.char, cf)
        if cx[1] != (0, 0, 0, 0):
            used_adds.add((r.char, cx[1]))
if used_adds:
    print(f"note: {len(used_adds)} placements use cxform ADD terms "
          f"(approximated by folding into the multiplier)")

total_px = sum(v["img"].width * v["img"].height for v in images.values())
print(f"{len(images)} atlas images, {total_px/1e6:.2f} Mpx")


# ---------------- plane per run ----------------
def run_plane(r, rank, nlayers):
    if r.char in PLANES:
        return PLANES[r.char]
    key = (r.char, list(r.frames.values())[0][2])
    im = images[key]["img"]
    covers = im.width >= STAGE_W * SCALE * 0.98 and im.height >= STAGE_H * SCALE * 0.98
    if covers and rank > nlayers * 0.5:
        return 0.0            # full-screen overlay (fade): keep at the glass
    if nlayers <= 1:
        return 0.95
    return 0.95 - 0.85 * (rank / (nlayers - 1))


# ---------------- sampled frames -> draw list ----------------
for i, r in enumerate(runs):
    r.id = i

def fold_matrix(mt, anchor, rs):
    # dst = S(SCALE) . M . T(x0,y0) . S(1/rs): maps the rs-prescaled image's
    # pixels to centre-origin dst px on the 320x240 stage.
    m = mat_mul((SCALE, 0, 0, SCALE, 0, 0),
                mat_mul(mt, mat_mul((1, 0, 0, 1, anchor[0], anchor[1]),
                                    (1 / rs, 0, 0, 1 / rs, 0, 0))))
    return m

def tint_of(cx):
    mult, add = cx
    out = []
    for i in range(4):
        m = mult[i] + (add[i] if add[i] < 0 else 0)   # darken folds fine
        m = max(0, min(256, m))
        out.append(int(round(m * 255 / 256)))
    return tuple(out)

sample_frames = list(range(0, n_frames, SAMPLE_STEP))
frames_out = []   # per sample: list of draw dicts
for f in sample_frames:
    active = [r for r in runs if f in r.frames]
    active.sort(key=lambda r: r.depth)
    n = len(active)
    draws = []
    for rank, r in enumerate(active):
        mt, cx, cf = r.frames[f]
        key = image_for(r.char, cf)
        info = images[key]
        m = fold_matrix(mt, info["anchor"], info["rs"])
        draws.append({
            "run": r.id, "img": key,
            "m": m, "tint": tint_of(cx),
            "plane": run_plane(r, rank, n),
        })
    frames_out.append((f, draws))

n_draws = sum(len(d) for _, d in frames_out)
print(f"{len(frames_out)} sampled frames, {n_draws} draws "
      f"(avg {n_draws/len(frames_out):.1f} layers/frame)")


# ---------------- pack into sheets ----------------
os.makedirs(STAGE_DIR, exist_ok=True)
for old in os.listdir(STAGE_DIR):
    os.remove(os.path.join(STAGE_DIR, old))

order = sorted(images.keys(), key=lambda k: -(images[k]["img"].width * images[k]["img"].height))
sheets = []   # list of [keys, area]
for key in order:
    im = images[key]["img"]
    area = (im.width + 2) * (im.height + 2)
    for sh in sheets:
        if sh[1] + area <= SHEET_MAX_AREA:
            sh[0].append(key); sh[1] += area
            break
    else:
        sheets.append([[key], area])

img_index = {}
for si, (keys, area) in enumerate(sheets):
    lines = ["--atlas --border edge -f rgba4444 -z auto", ""]
    for ii, key in enumerate(keys):
        ch, cf = key
        name = f"Intro/i{ch}_{cf}.png"
        images[key]["img"].save(os.path.join(GFX, name))
        lines.append(f"./{name}")
        img_index[key] = (si, ii)
    with open(os.path.join(GFX, f"intro_{si + 1}.t3s"), "w") as fh:
        fh.write("\n".join(lines) + "\n")
print(f"{len(sheets)} sheets: " +
      ", ".join(f"#{i+1}={a/1e6:.2f}Mpx" for i, (k, a) in enumerate(sheets)))


# ---------------- emit header ----------------
def fx(v):
    s = f"{v:.6g}"
    if "." not in s and "e" not in s:
        s += ".0"
    return s + "f"

with open(HEADER, "w") as fh:
    w = fh.write
    w("// Generated by tools/gen_intro.py - do not edit.\n")
    w("// Layered playback of the original intro clip (DefineSprite_2930):\n")
    w("// per sampled frame a back-to-front list of atlas images with affine\n")
    w("// matrices (dst px on the 320x240 stage), multiply-tints and stereo\n")
    w("// planes. The runtime lerps matrices between samples for 30fps+.\n")
    w("#pragma once\n#include <3ds.h>\n\n")
    w(f"#define INTRO_FPS {FPS}\n")
    w(f"#define INTRO_SRC_FRAMES {n_frames}\n")
    w(f"#define INTRO_SAMPLE_STEP {SAMPLE_STEP}\n")
    w(f"#define INTRO_SHEET_COUNT {len(sheets)}\n\n")
    w("struct IntroDraw\n{\n"
      "\tu16 run;         // placement id, for lerp matching\n"
      "\tu8 sheet, index; // atlas image\n"
      "\ts8 plane;        // stereo plane * 100\n"
      "\tu8 tint[4];      // rgba multiplier * 255\n"
      "\tfloat a, b, c, d, tx, ty;\n};\n\n")
    w("struct IntroFrame\n{\n"
      "\tu16 srcFrame;    // 0-based frame in the 30fps source clip\n"
      "\tu16 first;       // index into INTRO_DRAWS\n"
      "\tu16 count;\n};\n\n")
    w(f"static const IntroDraw INTRO_DRAWS[{n_draws}] = {{\n")
    for f, draws in frames_out:
        for d in draws:
            si, ii = img_index[d["img"]]
            a, b, c, dd, tx, ty = d["m"]
            t = d["tint"]
            w(f"\t{{{d['run']}, {si}, {ii}, {int(round(d['plane']*100))}, "
              f"{{{t[0]}, {t[1]}, {t[2]}, {t[3]}}}, "
              f"{fx(a)}, {fx(b)}, {fx(c)}, {fx(dd)}, {fx(tx)}, {fx(ty)}}},\n")
    w("};\n\n")
    w(f"static const IntroFrame INTRO_FRAMES[{len(frames_out)}] = {{\n")
    first = 0
    for f, draws in frames_out:
        w(f"\t{{{f}, {first}, {len(draws)}}},\n")
        first += len(draws)
    w("};\n")
print(f"wrote {HEADER}")


# ---------------- validation ----------------
def compose_model(sample_idx):
    canvas = Image.new("RGBA", (int(STAGE_W * SCALE), int(STAGE_H * SCALE)), (0, 0, 0, 0))
    _, draws = frames_out[sample_idx]
    for d in draws:
        info = images[d["img"]]
        a, b, c, dd, tx, ty = d["m"]
        # canvas (0,0) = centre-origin dst (-160,-120)
        tx += STAGE_W * SCALE / 2.0
        ty += STAGE_H * SCALE / 2.0
        det = a * dd - b * c
        if abs(det) < 1e-9: continue
        ia, ib, ic, idd = dd / det, -b / det, -c / det, a / det
        itx = -(ia * tx + ic * ty); ity = -(ib * tx + idd * ty)
        layer = info["img"].transform(canvas.size, Image.AFFINE,
                                      (ia, ic, itx, ib, idd, ity),
                                      resample=Image.BILINEAR)
        t = d["tint"]
        if t != (255, 255, 255, 255):
            px = np.asarray(layer, np.float32)
            for i in range(4):
                px[..., i] *= t[i] / 255.0
            layer = Image.fromarray(np.clip(px, 0, 255).astype(np.uint8), "RGBA")
        canvas.alpha_composite(layer)
    return canvas

def over_white(im):
    bg = Image.new("RGBA", im.size, (255, 255, 255, 255))
    bg.alpha_composite(im)
    return np.asarray(bg.convert("RGB"), np.float32)

worst = 0.0
for probe in (60, 180, 300, 500, 760, 1000, 1100, 1160):
    si = min(range(len(sample_frames)), key=lambda i: abs(sample_frames[i] - (probe - 1)))
    rec = compose_model(si)
    # the stage is centred on the sprite origin: sprite (-300..300, -225..225)
    ref = Image.open(f"{SRC}/{sample_frames[si] + 1}.png").convert("RGBA") \
        .crop((int(round(CX0)) - STAGE_W // 2, int(round(CY0)) - STAGE_H // 2,
               int(round(CX0)) + STAGE_W // 2, int(round(CY0)) + STAGE_H // 2)) \
        .resize(rec.size, Image.LANCZOS)
    dmap = np.abs(over_white(rec) - over_white(ref))
    frac = (dmap.max(axis=2) > 40).mean() * 100
    worst = max(worst, frac)
    print(f"validate frame {sample_frames[si]+1}: mean|diff|={dmap.mean():.2f} "
          f">40px%={frac:.1f}%")
    if SCRATCH:
        side = Image.new("RGB", (rec.width * 2 + 8, rec.height), (64, 64, 64))
        side.paste(Image.fromarray(over_white(rec).astype(np.uint8)), (0, 0))
        side.paste(Image.fromarray(over_white(ref).astype(np.uint8)), (rec.width + 8, 0))
        side.save(os.path.join(SCRATCH, f"gen_validate_{sample_frames[si]+1}.png"))
print(f"validation worst >40 fraction: {worst:.1f}%")
