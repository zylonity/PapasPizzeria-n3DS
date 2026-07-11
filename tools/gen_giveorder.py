#!/usr/bin/env python3
# Convert the original 94-frame Roy give-order composite (DefineSprite_850,
# 678x498 @ 24fps: slide in with closed box -> hold for drumroll -> flip lid
# open with glow -> settle) into 3DS atlases plus a generated frame table.
#
# The full clip is too much texture for the 3DS, so frames are sampled by
# phase (the hold is near-static, the motion phases keep 12fps), cropped to
# content, halved with the same premultiplied-Lanczos + alpha-bleed pipeline
# as gen_rig.py, and packed into rgba4444 atlases capped below 1024x512.
#
# Usage: python3 tools/gen_giveorder.py [repo root]

import os, sys
import numpy as np
from PIL import Image

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
SRC = os.path.join(ROOT, "papas_extract/sprites/DefineSprite_850")
GFX = os.path.join(ROOT, "gfx")
STAGE = os.path.join(GFX, "GiveOrderRoy")
HEADER = os.path.join(ROOT, "source/Papas_GiveOrderFrames.h")

SCALE = 0.5
CANVAS = (678, 498)
FPS = 24
TOTAL_SRC_FRAMES = 94
# Largest atlas allowed per sheet: 512x1024 rgba4444 = 0.5 MB resident
SHEET_MAX_PX = 512 * 1024

# Sampled source frames. The original carry (1-17) and drumroll hold (18-45)
# both have Roy tilting/jiggling the box; the user wants the box steady, so
# both phases collapse to one static pose - frame 37, the closest match to
# the lid flip's first frame (46). The runtime glides that pose in
# procedurally (renderGiveOrderRoy) instead of playing carry frames. The lid
# flip keeps the full 24fps; the settle reads fine at 12fps.
FRAMES = [37] + list(range(46, 68)) + list(range(68, 95, 2))


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


def stage_scaled(im, dst, w, h):
    px = np.asarray(im).astype(np.float32)
    a = px[..., 3:4] / 255.0
    prem = np.concatenate([px[..., :3] * a, px[..., 3:4]], axis=-1)
    small = np.stack([np.asarray(Image.fromarray(prem[..., c], mode="F")
                                 .resize((w, h), Image.LANCZOS)) for c in range(4)], axis=-1)
    np.clip(small, 0, 255, out=small)
    sa = small[..., 3:4]
    solid = sa[..., 0] > 8.0
    small[..., :3] = np.where(solid[..., None],
                              np.clip(small[..., :3] * 255.0 / np.maximum(sa, 1e-6), 0, 255), 0)
    small = alpha_bleed(small, solid)
    Image.fromarray(small.astype(np.uint8)).save(dst)


os.makedirs(STAGE, exist_ok=True)
entries = []  # (srcFrame, ox, oy, w, h, pngpath)
for f in FRAMES:
    src = os.path.join(SRC, f"{f}.png")
    im = Image.open(src).convert("RGBA")
    assert im.size == CANVAS, f"frame {f} is {im.size}, expected {CANVAS}"
    bbox = im.getbbox()
    crop = im.crop(bbox)
    w = max(1, round(crop.width * SCALE))
    h = max(1, round(crop.height * SCALE))
    dst = os.path.join(STAGE, f"roy_{f}.png")
    if not (os.path.isfile(dst) and os.path.getmtime(dst) >= os.path.getmtime(src)
            and Image.open(dst).size == (w, h)):
        stage_scaled(crop, dst, w, h)
    entries.append((f, round(bbox[0] * SCALE), round(bbox[1] * SCALE), w, h, dst))

# Split into sheets by real packing: keep adding frames while tex3ds still
# fits them into an atlas no larger than SHEET_MAX_PX (area-based splits
# leave atlases half empty when the packer rounds up to the next power of
# two; asking the packer directly costs a few trial runs and saves ~1 MB).
import struct
import subprocess
import tempfile


def packed_dims(files):
    with tempfile.NamedTemporaryFile(suffix=".t3x") as tmp:
        r = subprocess.run(["tex3ds", "--atlas", "--border", "edge",
                            "-f", "rgba4444", "-o", tmp.name] + files,
                           capture_output=True)
        if r.returncode != 0:
            return None
        n, dims = struct.unpack("<HB", open(tmp.name, "rb").read(3))
        return 8 << (dims & 7), 8 << ((dims >> 3) & 7)


sheets = []
i = 0
while i < len(entries):
    current = [entries[i]]
    i += 1
    while i < len(entries):
        dims = packed_dims([e[5] for e in current + [entries[i]]])
        if dims and dims[0] * dims[1] <= SHEET_MAX_PX:
            current.append(entries[i])
            i += 1
        else:
            break
    sheets.append(current)

table = []  # (srcFrame, sheet, index, ox, oy)
for si, sheet in enumerate(sheets):
    lines = ["--atlas --border edge -f rgba4444 -z auto", ""]
    for idx, (f, ox, oy, w, h, png) in enumerate(sheet):
        lines.append(os.path.relpath(png, GFX))
        table.append((f, si, idx, ox, oy))
    path = os.path.join(GFX, f"giveorder_roy{si + 1}.t3s")
    open(path, "w").write("\n".join(lines) + "\n")
    print(f"wrote {path} ({len(sheet)} frames, {sum(e[3]*e[4] for e in sheet)} px)")

rows = "\n".join(f"\t{{ {f}, {s}, {i}, {ox}, {oy} }}," for f, s, i, ox, oy in table)
open(HEADER, "w").write(f"""#pragma once
// Generated by tools/gen_giveorder.py - do not edit by hand.
// Sampled frames of the original 94-frame (24fps) Roy give-order clip
// (DefineSprite_850), each cropped to content and scaled by {SCALE}.
// ox/oy place a frame's cropped image relative to the scaled
// {round(CANVAS[0] * SCALE)}x{round(CANVAS[1] * SCALE)} clip canvas.

namespace Papas {{

	struct GiveOrderFrame {{
		unsigned char srcFrame; // 1-based frame in the original clip
		unsigned char sheet;    // giveorder_roy<sheet+1>.t3x
		unsigned char index;    // image index within that sheet
		short ox, oy;
	}};

	const int GIVEORDER_SHEET_COUNT = {len(sheets)};
	const int GIVEORDER_FRAME_COUNT = {len(table)};
	const int GIVEORDER_SRC_FRAMES = {TOTAL_SRC_FRAMES};
	const float GIVEORDER_FPS = {FPS}.0f;

	const GiveOrderFrame GIVEORDER_FRAMES[GIVEORDER_FRAME_COUNT] = {{
{rows}
	}};
}}
""")
print(f"wrote {HEADER} ({len(table)} frames, {len(sheets)} sheets)")
