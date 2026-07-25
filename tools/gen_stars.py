#!/usr/bin/env python3
# Build filled, empty, flashing, and seal art for the star-customer row.
#
# Usage: python3 tools/gen_stars.py [repo root]

import os
import sys

import cairosvg
from PIL import Image, ImageDraw

root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..")
shapes = os.path.join(root, "papas_extract", "shapes")
sprites = os.path.join(root, "papas_extract", "sprites")
out_dir = os.path.join(root, "gfx", "Stars")
os.makedirs(out_dir, exist_ok=True)

# Drawn at ~0.5 scale in game, so export at 2x the SVG size for clean minification
STAR_SCALE = 2.0


def render_shape(shape_id, name, scale=STAR_SCALE):
    path = os.path.join(out_dir, name)
    cairosvg.svg2png(url=os.path.join(shapes, f"{shape_id}.svg"), write_to=path, scale=scale)
    return Image.open(path).convert("RGBA")


star = render_shape(330, "star_filled.png")

# Empty slot: silhouette in the takeorder_fg counter-outline green
empty = star.copy()
px = empty.load()
for y in range(empty.height):
    for x in range(empty.width):
        r, g, b, a = px[x, y]
        px[x, y] = (44, 74, 33, a)
empty.save(os.path.join(out_dir, "star_empty.png"))

render_shape(812, "star_flash.png")

# Mask the badge as a circle to leave out the overlapping green splash.
fg = Image.open(os.path.join(sprites, "DefineSprite_884_giveorder_fg", "1.png")).convert("RGBA")
cx, cy, r = 1053, 379, 167
box = fg.crop((cx - r, cy - r, cx + r, cy + r))
mask = Image.new("L", box.size, 0)
ImageDraw.Draw(mask).ellipse((0, 0, box.size[0] - 1, box.size[1] - 1), fill=255)
seal = Image.new("RGBA", box.size, (0, 0, 0, 0))
seal.paste(box, (0, 0), mask)
seal.thumbnail((44, 44), Image.LANCZOS)
seal.save(os.path.join(out_dir, "seal.png"))

print("wrote", sorted(os.listdir(out_dir)))
