#!/usr/bin/env python3
# Pull the pause screen's wood backdrop and napkin board out of the original clip.
#
# Usage: python3 tools/gen_pause.py [repo root]

import os
import sys

from PIL import Image, ImageDraw

root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..")
src = os.path.join(root, "papas_extract", "sprites", "DefineSprite_939_pause_screen", "1.png")
out_dir = os.path.join(root, "gfx", "Pause")
os.makedirs(out_dir, exist_ok=True)

TOP_W, TOP_H = 400, 240
# The clip's own numbers: clean wood band, the napkin stack, and the buttons we drop.
# The straw pokes in at row 52, so the band has to stop short of it or the tiling
# repeats a slice of cup all the way down the screen.
WOOD_BAND = (0, 0, 455, 48)
BOARD_BOX = (16, 84, 430, 420)
BUTTON_BOX = (193, 230, 362, 384)
NAPKIN = (250, 247, 240, 255)
BOARD_H = 208

clip = Image.open(src).convert("RGBA")

# Wood: the grain runs sideways, so stacking the band keeps it sharp where a
# stretch would just smear it.
band = clip.crop(WOOD_BAND).crop((0, 0, TOP_W, WOOD_BAND[3]))
band.save(os.path.join(out_dir, "pause_wood.png"))

# Board: napkins, cup and the Pause title, with the original's buttons wiped off
board = clip.crop(BOARD_BOX)
draw = ImageDraw.Draw(board)
draw.rectangle((BUTTON_BOX[0] - BOARD_BOX[0], BUTTON_BOX[1] - BOARD_BOX[1],
                BUTTON_BOX[2] - BOARD_BOX[0], BUTTON_BOX[3] - BOARD_BOX[1]), fill=NAPKIN)
scale = BOARD_H / board.height
board = board.resize((int(round(board.width * scale)), BOARD_H), Image.LANCZOS)
board.save(os.path.join(out_dir, "pause_board.png"))

print("wrote", sorted(os.listdir(out_dir)), "board", board.size)
