#!/usr/bin/env python3
"""Build the dim and bright oven-coal tiles used by gfx/stations.t3s."""
import io
import os

import cairosvg
import numpy as np
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SHAPES = [f"{c}" for c in range(942, 981, 2)]        # the 20 coal frames, in order

# Store quarter-bed tiles small enough to keep the station atlas at 1024x512.
TILE_W, TILE_H = 128, 96


def main():
    frames = []
    for cid in SHAPES:
        png = cairosvg.svg2png(url=os.path.join(ROOT, "papas_extract", "shapes", f"{cid}.svg"))
        tile = Image.open(io.BytesIO(png)).convert("RGB")       # 225x179 in original units
        frames.append(np.array(tile.resize((TILE_W, TILE_H), Image.LANCZOS), dtype=np.uint8))
    stack = np.stack(frames)

    out = os.path.join(ROOT, "gfx", "Stations")
    for name, img in (("coals_dim", stack.min(0)), ("coals_bright", stack.max(0))):
        path = os.path.join(out, f"{name}.png")
        Image.fromarray(img).save(path)
        print(f"wrote {path} {TILE_W}x{TILE_H} (mean {img.mean():.1f})")
    print(f"dim->bright gap: mean {(stack.max(0).astype(int) - stack.min(0)).mean():.1f}/255")


if __name__ == "__main__":
    main()
