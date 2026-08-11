#!/usr/bin/env python3
# Bake the extra station buttons in the same pill style as btn_serve/btn_oven.
#
# Usage: python3 tools/gen_buttons.py [repo root]

import os
import sys

from PIL import Image, ImageDraw, ImageFont

root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..")
out_dir = os.path.join(root, "gfx", "Pizza")
font_path = os.path.join(root, "romfs", "fonts", "Dokyo.ttf")

HEIGHT = 28
TEXT_PX = 15
# Layer colours sampled straight off btn_serve1.png
IDLE = {
    "shadow": (94, 87, 80, 70),
    "outline": (95, 89, 80, 255),
    "ring": (36, 105, 4, 255),
    "fill": (59, 186, 8, 255),
    "text": (11, 33, 0, 255),
}
PRESSED = {
    "shadow": (57, 44, 43, 60),
    "outline": (0, 0, 0, 209),
    "ring": (0, 59, 0, 209),
    "fill": (13, 140, 0, 209),
    "text": (0, 0, 0, 209),
}

# Supersample so the pill's curves land as cleanly as the extracted art
SS = 4


def pill(draw, box, radius, colour):
    draw.rounded_rectangle(box, radius=radius, fill=colour)


def make_button(text, width, palette, path):
    w, h = width * SS, HEIGHT * SS
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    radius = h // 2

    # Shadow, dark outline, dark green ring, then the bright face
    pill(draw, (2 * SS, 2 * SS, w - 1, h - 1), radius, palette["shadow"])
    pill(draw, (SS, SS, w - 2 * SS - 1, h - 2 * SS - 1), radius, palette["outline"])
    pill(draw, (3 * SS, 3 * SS, w - 4 * SS - 1, h - 4 * SS - 1), radius, palette["ring"])
    pill(draw, (5 * SS, 5 * SS, w - 6 * SS - 1, h - 6 * SS - 1), radius, palette["fill"])

    font = ImageFont.truetype(font_path, TEXT_PX * SS)
    left, top, right, bottom = draw.textbbox((0, 0), text, font=font)
    draw.text(((w - (right + left)) / 2, (h - 2 * SS - (bottom + top)) / 2), text,
              font=font, fill=palette["text"])

    img.resize((width, HEIGHT), Image.LANCZOS).save(path)


def build(name, text, width):
    make_button(text, width, IDLE, os.path.join(out_dir, f"{name}1.png"))
    make_button(text, width, PRESSED, os.path.join(out_dir, f"{name}2.png"))


build("btn_save", "save", 96)

print("wrote", sorted(n for n in os.listdir(out_dir) if n.startswith("btn_")))
