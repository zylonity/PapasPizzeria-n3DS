#!/usr/bin/env python3
# Split the start-of-day clip into a background, changing patches, and day plate.

import os
import sys
import numpy as np
from PIL import Image, ImageDraw

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
SRC = os.path.join(ROOT, "papas_extract/sprites/DefineSprite_549_startofday")
GFX = os.path.join(ROOT, "gfx")
STAGE = os.path.join(GFX, "StartOfDay")
HEADER = os.path.join(ROOT, "source/Papas_StartOfDayFrames.h")

# Crop the storefront's opaque span to the screen's full 240px height.
SCALE = 240.0 / 450.0
N_FRAMES = 113
FPS = 24
# Scene crop in source coordinates -> 323x240 at SCALE
CROP = (102, 174, 707, 624)
# Only this region (source y < 528) ever changes besides the plate
PATCH_MAX_Y_SRC = 528
# Vertical band the plate moves through (x extent comes from the plate art)
PLATE_Y0, PLATE_Y1 = 480, 772
# Keep all 72 changing frames so Roy's walk doesn't stutter.
PATCH_FRAMES = list(range(1, 73))


# Combine early and late frames so every background pixel is free of Roy.
PLATE_SPANS = (list(range(1, 56, 2)), list(range(1, 26, 2)))
# Frames the plate is checked against when picking between those two
PLATE_VOTE_FRAMES = list(range(1, 114, 2))
# Stop scrubbing above the day plate's tracking band.
DELEAK_MAX_Y = 470


def load_raw(f):
    return np.asarray(Image.open(f"{SRC}/{f}.png").convert("RGBA")).astype(np.float32)


def flatten(px):
    a = px[..., 3:4] / 255.0
    return np.clip(px[..., :3] * a + 255.0 * (1.0 - a), 0, 255)


def is_glass(px):
    """Green-tinted and bright: the panes, as against grey metal and brick."""
    f = flatten(px)
    return ((f[..., 1] - np.maximum(f[..., 0], f[..., 2])) > 6) & (f.max(axis=2) > 130)


def fill_holes(mask):
    """Fill enclosed holes while leaving outside-connected gaps alone."""
    im = Image.fromarray(np.where(mask, 0, 255).astype(np.uint8), "L")
    pad = Image.new("L", (im.width + 2, im.height + 2), 255)
    pad.paste(im, (1, 1))
    ImageDraw.floodfill(pad, (0, 0), 128)
    return mask | ~(mask | (np.asarray(pad)[1:-1, 1:-1] == 128))


_plate = None
_content = None


def storefront_plate():
    """Combine clean frame spans into an empty storefront and motion mask."""
    global _plate, _content
    if _plate is None:
        cands = [np.median(np.stack([load_raw(f) for f in s]), axis=0) for s in PLATE_SPANS]
        flats = [flatten(c) for c in cands]
        votes = [np.zeros(c.shape[:2], np.int32) for c in cands]
        for f in PLATE_VOTE_FRAMES:
            ff = flatten(load_raw(f))
            for v, fc in zip(votes, flats):
                v += np.abs(ff - fc).max(axis=2) <= 4
        pick = (votes[1] > votes[0])[..., None]
        _plate = np.where(pick, cands[1], cands[0])
        # A pane is glass in whichever span Roy is not standing in it
        _content = fill_holes(is_glass(cands[0]) | is_glass(cands[1]))
    return _plate, _content


def load(f):
    """Replace Roy's bleed outside the glass with the clean storefront."""
    px = load_raw(f)
    plate, content = storefront_plate()
    y = DELEAK_MAX_Y
    px[:y] = np.where(content[:y, ..., None], px[:y], plate[:y])
    return px


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
    return alpha_bleed(small, solid)


def scene_downscale(px):
    crop = px[CROP[1]:CROP[3], CROP[0]:CROP[2]]
    w = round((CROP[2] - CROP[0]) * SCALE)
    h = round((CROP[3] - CROP[1]) * SCALE)
    return downscale(crop, w, h)


# Track the day plate by its dark text against the plain sidewalk.
f1 = load(1)
SIDEWALK = np.array([197.0, 188.0, 177.0])
TRACK_Y0, TRACK_Y1 = 530, 750


def dark_text_top(px):
    zone = px[TRACK_Y0:TRACK_Y1, 300:500]
    dark = (zone[..., :3].max(axis=2) < 90) & (zone[..., 3] > 200)
    rows = np.where(dark.sum(axis=1) > 3)[0]
    return TRACK_Y0 + int(rows.min())


# Build the plate from two frames because its short travel never reveals it all.
f113 = load(113)
tt1 = dark_text_top(f1)
TPL_Y0 = tt1 - 4
below = f1[700:772, 260:540]
ys, xs = np.where(below[..., 3] > 10)
TPL_Y1 = min(f1.shape[0], 700 + int(ys.max()) + 2)
top113 = TPL_Y0 - (tt1 - dark_text_top(f113))  # plate top in frame 113
# x extent: motion diff at the frame-113 position (static scenery cancels)
zone = np.abs(f113[top113:top113 + 60, 260:540] - f1[top113:top113 + 60, 260:540]).max(axis=2) > 12
xs2 = np.where(zone.any(axis=0))[0]
TPL_X0 = 260 + min(xs.min(), xs2.min()) - 2
TPL_X1 = 260 + max(xs.max(), xs2.max()) + 3
plate_h = TPL_Y1 - TPL_Y0
print(f"plate bbox x {TPL_X0}-{TPL_X1}, y {TPL_Y0}-{TPL_Y1} (f113 top {top113})")

top1 = dark_text_top(f1)
track = [TPL_Y0 + dark_text_top(load(f)) - top1 for f in range(1, N_FRAMES + 1)]
# Keep only the plate's first rise by clamping away its later bounce.
for i in range(1, N_FRAMES):
    track[i] = min(track[i], track[i - 1])
print("plate top: f1", track[0], "f40", track[39], "f55", track[54], "f113", track[112])

# Join the clean upper plate from frame 113 to the lower plate from frame 1.
plate = np.zeros((plate_h, TPL_X1 - TPL_X0, 4), np.float32)
SEAM = 80
seg113 = f113[top113:top113 + SEAM, TPL_X0:TPL_X1]
ref1 = f1[top113:top113 + SEAM, TPL_X0:TPL_X1]
m = np.abs(seg113 - ref1).max(axis=2) > 12
for _ in range(2):  # close AA fringes (no wrap: np.roll would smear the
    m2 = m.copy()   # ellipse's wide bottom row onto the top as a stray line)
    m2[1:] |= m[:-1]
    m2[:-1] |= m[1:]
    m2[:, 1:] |= m[:, :-1]
    m2[:, :-1] |= m[:, 1:]
    m = m2
plate[:SEAM][m] = seg113[m]
plate[:SEAM, :, 3] *= 0.0
plate[:SEAM, :, 3][m] = 255.0
seg1 = f1[TPL_Y0 + SEAM:TPL_Y1, TPL_X0:TPL_X1]
plate[SEAM:] = np.where((seg1[..., 3:4] > 10), seg1, 0.0)
dark = (plate[..., :3].max(axis=2) < 100) & (plate[..., 3] > 200)
rows_with_dark = np.where(dark.any(axis=1))[0]
# Find the gap below "DAY" and paint over the sample number.
gaps = np.diff(rows_with_dark)
split_row = rows_with_dark[int(np.argmax(gaps))]
digit_rows = rows_with_dark[rows_with_dark > split_row]
digit_cols = np.where(dark[digit_rows.min():digit_rows.max() + 1].any(axis=0))[0]
ry0, ry1 = digit_rows.min() - 5, min(plate.shape[0], digit_rows.max() + 8)
rx0, rx1 = max(0, digit_cols.min() - 6), min(plate.shape[1], digit_cols.max() + 7)
band = plate[ry0:ry1, rx0:rx1, :3]
gray = (band.max(axis=2) >= 100) & (band.min(axis=2) < 246)
ellipse_gray = np.median(band[gray], axis=0)
plate[ry0:ry1, rx0:rx1, :3] = ellipse_gray
plate[ry0:ry1, rx0:rx1, 3] = 255.0
digit_cy = (ry0 + ry1) / 2.0
print(f"erased number rect rows {ry0}-{ry1}, cols {rx0}-{rx1}, fill {ellipse_gray.astype(int)}")

# ---------------- background (frame 113, plate scrubbed) ----------------
f113 = load(113)
f55 = load(55)
bg = f113.copy()
zone_y0, zone_y1 = PLATE_Y0, PLATE_Y1
plate_h = TPL_Y1 - TPL_Y0


def covers(y, top):  # does a plate whose top edge is `top` cover row y?
    return top <= y < top + plate_h


for y in range(zone_y0, zone_y1):
    if not covers(y, track[112]):
        continue  # frame 113's own pixels are plate-free here
    if not covers(y, track[54]):
        bg[y, TPL_X0:TPL_X1] = f55[y, TPL_X0:TPL_X1]
    elif not covers(y, track[0]):
        bg[y, TPL_X0:TPL_X1] = f1[y, TPL_X0:TPL_X1]
    elif y >= 650:
        # always covered, below the sidewalk -> nothing is really there
        bg[y, TPL_X0:TPL_X1] = 0.0
    else:
        # always covered, on the sidewalk -> stretch the row above
        bg[y, TPL_X0:TPL_X1] = bg[y - 1, TPL_X0:TPL_X1]

# ---------------- downscale everything, cut patches ----------------
os.makedirs(STAGE, exist_ok=True)
bg_small = scene_downscale(bg)


def on_stage(px):
    """Make a patch opaque against the clip's white stage."""
    out = px.copy()
    a = out[..., 3:4] / 255.0
    out[..., :3] = out[..., :3] * a + 255.0 * (1.0 - a)
    out[..., 3] = 255.0
    return out


def staged(px):
    """Square off crop edges so pillarboxes can't reveal a pale fringe."""
    out = px.copy()
    w = out.shape[1]
    solid = [x for x in range(w) if out[:, x, 3].mean() >= 250.0]
    lo, hi = solid[0], solid[-1]
    for x in range(lo):
        out[:, x] = out[:, lo]
    for x in range(hi + 1, w):
        out[:, x] = out[:, hi]
    return on_stage(out)


# Patches keep diffing against the untouched bg_small
Image.fromarray(staged(bg_small).astype(np.uint8)).save(f"{STAGE}/bg.png")

plate_small = downscale(plate, max(1, round(plate.shape[1] * SCALE)),
                        max(1, round(plate.shape[0] * SCALE)))
Image.fromarray(plate_small.astype(np.uint8)).save(f"{STAGE}/plate.png")

patch_max_y = round((PATCH_MAX_Y_SRC - CROP[1]) * SCALE)
entries = []  # (srcFrame, ox, oy, file)
for f in PATCH_FRAMES:
    small = scene_downscale(load(f))
    diff = np.abs(small - bg_small).max(axis=2) > 12
    diff[patch_max_y:, :] = False
    ys, xs = np.where(diff)
    if len(xs) == 0:
        continue
    x0, x1 = max(0, xs.min() - 1), min(small.shape[1], xs.max() + 2)
    y0, y1 = max(0, ys.min() - 1), min(small.shape[0], ys.max() + 2)
    patch = on_stage(small[y0:y1, x0:x1])
    Image.fromarray(patch.astype(np.uint8)).save(f"{STAGE}/patch_{f}.png")
    entries.append((f, int(x0), int(y0), f"patch_{f}.png"))
print(f"{len(entries)} patches; bg {bg_small.shape[1]}x{bg_small.shape[0]}, "
      f"plate {plate_small.shape[1]}x{plate_small.shape[0]}")

# ---------------- t3s + header ----------------
lines = ["--atlas --border edge -f rgba4444 -z auto", "", "StartOfDay/bg.png", "StartOfDay/plate.png"]
for f, ox, oy, name in entries:
    lines.append(f"StartOfDay/{name}")
open(f"{GFX}/startofday.t3s", "w").write("\n".join(lines) + "\n")

plate_x = round((TPL_X0 - CROP[0]) * SCALE)
# Shift the settled plate up so it doesn't poke below the sidewalk.
plate_dest_h = plate_small.shape[0]
shift = 236.0 - (track[112] - CROP[1]) * SCALE - plate_dest_h
if shift > 0:
    shift = 0.0
track_dest = [round((y - CROP[1]) * SCALE + shift, 2) for y in track]
print(f"plate track shift {shift:.1f} dest px")
digit_anchor_y = round(digit_cy * SCALE, 1)
rows = "\n".join(f"\t{{ {f}, {i + 2}, {ox}, {oy} }}," for i, (f, ox, oy, _) in enumerate(entries))
track_rows = ",".join(f"{v}f" for v in track_dest)
open(HEADER, "w").write(f"""#pragma once
// Generated by tools/gen_startofday.py for the {bg_small.shape[1]}x{bg_small.shape[0]} start-of-day scene.

namespace Papas {{

	struct StartOfDayPatch {{
		unsigned char srcFrame; // 1-based frame in the original clip
		unsigned char index;    // image index in startofday.t3x
		short ox, oy;
	}};

	const int STARTOFDAY_SRC_FRAMES = {N_FRAMES};
	const float STARTOFDAY_FPS = {FPS}.0f;
	const int STARTOFDAY_PATCH_COUNT = {len(entries)};
	const float STARTOFDAY_PLATE_X = {plate_x}.0f;
	// Centre of the erased day-number area, relative to the plate image top
	const float STARTOFDAY_DIGIT_Y = {digit_anchor_y}f;
	// Plate top-edge y per source frame (index 0 = frame 1)
	const float STARTOFDAY_PLATE_TRACK[STARTOFDAY_SRC_FRAMES] = {{{track_rows}}};

	const StartOfDayPatch STARTOFDAY_PATCHES[STARTOFDAY_PATCH_COUNT] = {{
{rows}
	}};
}}
""")
print(f"wrote {GFX}/startofday.t3s and {HEADER}")
