#!/usr/bin/env python3
# Turn the NEW CUSTOMER! and no-Papa clips into compact art and motion tracks.

import io
import os
import sys

import cairosvg
import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from swf_core import Swf

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
GFX = os.path.join(ROOT, "gfx")
STAGE = os.path.join(GFX, "NewCustomer")
HEADER = os.path.join(ROOT, "source/Papas_NewCustomerFrames.h")

# Scale the stage uniformly and centre-crop the striped background to 400x240.
SCALE = 240.0 / 450.0
STAGE_W, STAGE_H = 600.0, 450.0
SCREEN_W, SCREEN_H = 400, 240

FPS = 30.0
SRC_FRAMES = 100        # DefineSprite_474 stops on frame 100 -> endAnimation()
NOPAPA_SRC_FRAMES = 86  # DefineSprite_489 stops on frame 86

CIRCLE_MC = (89.0, 15.95)   # circleMC placement inside DefineSprite_482
TITLE_AT = (109.8, 14.85)   # DefineSprite_478
SEAL_MC = (240.9, 156.6)    # sealMC (DefineSprite_485) inside DefineSprite_489

DISC_SRC = os.path.join(ROOT, "papas_extract/sprites/DefineSprite_474")

swf = Swf(ROOT)


def rasterise(cid, sub, scale):
    """Rasterise a JPEXS shape in memory at exactly the requested scale."""
    src = os.path.join(ROOT, f"papas_extract/{sub}/{cid}.svg")
    png = cairosvg.svg2png(url=src, scale=scale)
    return np.asarray(Image.open(io.BytesIO(png)).convert("RGBA")).astype(np.float32)


def alpha_bleed(px, iters=6):
    """Bleed edge colours outward so filtering won't add dark halos."""
    rgb = px[..., :3]
    known = px[..., 3] > 8.0
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


def save(px, name):
    Image.fromarray(np.clip(px, 0, 255).astype(np.uint8)).save(os.path.join(STAGE, name))
    return px.shape[1], px.shape[0]


os.makedirs(STAGE, exist_ok=True)

# Centre-crop instead of squashing so the stripes keep their shape.
bg_scale = SCREEN_W / STAGE_W
bg_full = rasterise(467, "shapes", bg_scale)
crop_top = (bg_full.shape[0] - SCREEN_H) // 2
bg = bg_full[crop_top:crop_top + SCREEN_H, :SCREEN_W]
bg_w, bg_h = save(bg, "bg.png")

# Crop the captioned no-Papa stripes the same way.
nopapa_full = rasterise(484, "shapes", bg_scale)
nopapa_bg = nopapa_full[crop_top:crop_top + SCREEN_H, :SCREEN_W]
save(nopapa_bg, "nopapa_bg.png")

# ---------------- static pieces ----------------
disc_w, disc_h = save(alpha_bleed(rasterise(473, "shapes", SCALE)), "disc.png")
shadow_w, shadow_h = save(alpha_bleed(rasterise(468, "shapes", SCALE)), "shadow.png")
title_w, title_h = save(alpha_bleed(rasterise(476, "shapes", SCALE)), "title.png")
# Bake the seal at its 2.64x maximum, then only scale it down.
SEAL_MAX = 2.637
seal_w, seal_h = save(alpha_bleed(rasterise(340, "shapes", SCALE * SEAL_MAX)), "seal.png")
# The nopapa shadow is the same ellipse morph; its settled state is shape 488.
nopapa_shadow_w, nopapa_shadow_h = save(alpha_bleed(rasterise(488, "shapes", SCALE)),
                                        "nopapa_shadow.png")

# Measure the disc morph from JPEXS's correctly rendered frames.
ub = swf.union_bounds(474)
ox, oy = ub[0], ub[1]
disc_track = []
for f in range(1, SRC_FRAMES + 1):
    px = np.asarray(Image.open(f"{DISC_SRC}/{f}.png").convert("RGBA"))
    # The disc is the only pure-white opaque region.
    m = (px[..., 3] > 128) & (px[..., 0] > 240) & (px[..., 1] > 240) & (px[..., 2] > 240)
    ys, xs = np.nonzero(m)
    if len(xs) == 0:
        disc_track.append(None)   # frame 1: the disc has not been placed yet
        continue
    x0, x1 = xs.min() + ox, xs.max() + ox
    y0, y1 = ys.min() + oy, ys.max() + oy
    disc_track.append((x0, y0, x1 - x0 + 1, y1 - y0 + 1))
    if len(disc_track) > 1 and disc_track[-1] == disc_track[-2]:
        break  # settled; the runtime holds the last entry
disc_frames = len(disc_track)
settled = disc_track[-1]
print(f"disc: {disc_frames} tracked frames, settles at {settled[2]:.0f}x{settled[3]:.0f} src px")

# ---------------- title drop track ----------------
title_track = []
for fr in swf.timeline(478):
    place = fr.get(1)
    title_track.append(None if place is None else place[1][5])  # ty
print(f"title: drops over {len(title_track)} frames")

# ---------------- seal slam track ----------------
seal_track = []
for fr in swf.timeline(485):
    place = fr.get(1)
    if place is None:
        seal_track.append(None)
        continue
    m = place[1]
    seal_track.append((m[0], m[4], m[5]))  # uniform scale, tx, ty
# Drop the seal's offscreen lead-in and keep its parent-frame timing.
seal_start = next(i for i, s in enumerate(seal_track)
                  if s is not None and SEAL_MC[0] + s[1] < STAGE_W)
seal_track = seal_track[seal_start:]
print(f"seal: {len(seal_track)} frames from src frame {seal_start + 1}")

# Rebuild the plain shadow ellipse from its morph ratios; exports loop incorrectly.
nshadow_track = []
for f, cid, ratio in swf.morph_ratios(489, 5):
    if cid == 488:
        a = b = swf.shape_bounds(488)
    else:
        a = swf.shape_bounds(cid)
        b = swf.shape_bounds(487 if cid == 486 else 488)
    t = (ratio or 0) / 65535.0
    x0, y0, x1, y1 = (a[i] + (b[i] - a[i]) * t for i in range(4))
    nshadow_track.append((x0, y0, x1 - x0, y1 - y0))
# Both tracks hold their last entry at runtime, so the static tail is dropped
while len(nshadow_track) > 1 and nshadow_track[-1] == nshadow_track[-2]:
    nshadow_track.pop()
assert seal_start + 1 == swf.morph_ratios(489, 5)[0][0], \
    "the seal and its shadow are expected to enter on the same frame"
print(f"nopapa shadow: {len(nshadow_track)} frames, "
      f"{nshadow_track[0][2]:.0f}x{nshadow_track[0][3]:.0f} -> "
      f"{nshadow_track[-1][2]:.0f}x{nshadow_track[-1][3]:.0f} src px")


# ---------------- t3s + header ----------------
def to_screen_x(x):
    return round(40.0 + x * SCALE, 2)


def to_screen_y(y):
    return round(y * SCALE, 2)


images = ["bg.png", "disc.png", "shadow.png", "title.png",
          "nopapa_bg.png", "seal.png", "nopapa_shadow.png"]
open(f"{GFX}/newcustomer.t3s", "w").write(
    "\n".join(["--atlas --border edge -f rgba4444 -z auto", ""]
              + [f"NewCustomer/{n}" for n in images]) + "\n")


def box_row(entry, at=(0.0, 0.0)):
    if entry is None:
        return "\t{ 0.0f, 0.0f, 0.0f, 0.0f },"
    x, y, w, h = entry
    return (f"\t{{ {to_screen_x(at[0] + x)}f, {to_screen_y(at[1] + y)}f, "
            f"{round(w * SCALE, 2)}f, {round(h * SCALE, 2)}f }},")


def seal_row(entry):
    if entry is None:
        return "\t{ 0.0f, 0.0f, 0.0f },"
    s, tx, ty = entry
    return (f"\t{{ {to_screen_x(SEAL_MC[0] + tx)}f, {to_screen_y(SEAL_MC[1] + ty)}f, "
            f"{round(s / SEAL_MAX, 4)}f }},")


title_rows = ",".join("-1000.0f" if t is None else f"{to_screen_y(TITLE_AT[1] + t)}f"
                      for t in title_track)

open(HEADER, "w").write(f"""#pragma once
// Generated by tools/gen_newcustomer.py in ready-to-draw top-screen coordinates.

namespace Papas {{

	enum NewCustomerImage {{
		NC_IMG_BG,
		NC_IMG_DISC,
		NC_IMG_SHADOW,
		NC_IMG_TITLE,
		NC_IMG_NOPAPA_BG,
		NC_IMG_SEAL,
		NC_IMG_NOPAPA_SHADOW
	}};

	// A frame's stretch box, used to fake the disc and shadow morphs.
	struct NewCustomerBox {{ float x, y, w, h; }};
	// Top-left corner + scale of the seal on each frame of its slam-in
	struct NewCustomerSealFrame {{ float x, y, scale; }};

	const float NEWCUSTOMER_FPS = {FPS}f;
	const int NEWCUSTOMER_SRC_FRAMES = {SRC_FRAMES};
	const int NEWCUSTOMER_NOPAPA_SRC_FRAMES = {NOPAPA_SRC_FRAMES};

	// The disc grows through these frames, then holds; zero width means hidden.
	const int NEWCUSTOMER_DISC_FRAMES = {disc_frames};
	const NewCustomerBox NEWCUSTOMER_DISC[NEWCUSTOMER_DISC_FRAMES] = {{
{chr(10).join(box_row(e, CIRCLE_MC) for e in disc_track)}
	}};

	// Title y per frame; -1000 = not on stage yet. Holds its last entry.
	const int NEWCUSTOMER_TITLE_FRAMES = {len(title_track)};
	const float NEWCUSTOMER_TITLE_X = {to_screen_x(TITLE_AT[0])}f;
	const float NEWCUSTOMER_TITLE_Y[NEWCUSTOMER_TITLE_FRAMES] = {{{title_rows}}};

	// Place the shadow and the customer playing "overjoyed".
	const float NEWCUSTOMER_SHADOW_X = {to_screen_x(CIRCLE_MC[0] + swf.shape_bounds(468)[0])}f;
	const float NEWCUSTOMER_SHADOW_Y = {to_screen_y(CIRCLE_MC[1] + swf.shape_bounds(468)[1])}f;
	const float NEWCUSTOMER_RIG_X = {to_screen_x(CIRCLE_MC[0] + 141.0)}f;
	const float NEWCUSTOMER_RIG_Y = {to_screen_y(CIRCLE_MC[1] + 74.95)}f;
	const float NEWCUSTOMER_RIG_SCALE = {round(0.8 * SCALE, 4)}f;

	// Scale the substitute game font to the original name field's height.
	const float NEWCUSTOMER_NAME_CX = {to_screen_x(CIRCLE_MC[0] + 56.0 + (-2.0 + 289.95) / 2.0)}f;
	const float NEWCUSTOMER_NAME_Y = {to_screen_y(CIRCLE_MC[1] + 329.35 + 2.0)}f;
	const float NEWCUSTOMER_NAME_HEIGHT = {round(48.0 * SCALE, 2)}f;

	// The no-Papa seal and shadow start together, then hold their last frame.
	const int NEWCUSTOMER_SEAL_START = {seal_start + 1};
	const int NEWCUSTOMER_SEAL_FRAMES = {len(seal_track)};
	const NewCustomerSealFrame NEWCUSTOMER_SEAL[NEWCUSTOMER_SEAL_FRAMES] = {{
{chr(10).join(seal_row(e) for e in seal_track)}
	}};
	const int NEWCUSTOMER_NOPAPA_SHADOW_FRAMES = {len(nshadow_track)};
	const NewCustomerBox NEWCUSTOMER_NOPAPA_SHADOW[NEWCUSTOMER_NOPAPA_SHADOW_FRAMES] = {{
{chr(10).join(box_row(e) for e in nshadow_track)}
	}};
}}
""")
print(f"wrote {GFX}/newcustomer.t3s and {HEADER}")
print(f"  bg {bg_w}x{bg_h}, disc {disc_w}x{disc_h}, shadow {shadow_w}x{shadow_h}, "
      f"title {title_w}x{title_h}, seal {seal_w}x{seal_h}")
