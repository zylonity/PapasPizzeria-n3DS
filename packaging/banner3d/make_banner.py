#!/usr/bin/env python3
# Build the 3D home-menu banner with toppings orbiting the logo.
# Needs PYCGFX, BANNERTOOL, gltflib, and Pillow.
import math, os, subprocess, sys
from gltfbuild import GLTF

HERE = os.path.dirname(os.path.abspath(__file__))
PYCGFX     = os.environ.get("PYCGFX", "/home/khaleel/Downloads/pycgfx/main.py")
BANNERTOOL = os.environ.get("BANNERTOOL", "/home/khaleel/Downloads/bannertool-3d/build/bannertool")
AUDIO      = os.path.join(HERE, "banner_audio.wav")   # must be STEREO, <= 3 seconds

# --- scene parameters (tweak these) ---
R    = 11.0                  # orbit radius
TILT = math.radians(25)      # ring tilt so the camera sees an ellipse
YC   = 1.0                   # ring centre height (= logo centre, = camera y)
HALF = 1.6                   # topping quad half-size
DUR  = 8.0                   # seconds per full orbit (seamless loop; higher = slower)
NKEY = 25                    # keyframes per topping (last == first)
TOPS = ["pepperoni", "mushroom", "olive", "onion", "pepper", "sausage", "anchovy"]

def ring_pos(theta):
    x = R * math.cos(theta); z0 = R * math.sin(theta)
    return (x, YC - z0 * math.sin(TILT), z0 * math.cos(TILT))

def build_gltf(path):
    g = GLTF()
    g.cameras = [{"name": "Banner Camera", "type": "perspective",
                  "perspective": {"aspectRatio": 1.66666666667, "yfov": 0.523599,
                                  "zfar": 1000, "znear": 26.5}}]
    cam = g.node(camera=0, translation=[0, 1, 44.786], name="Banner Camera")
    ltex = g.image("tex_logo.png"); lmat = g.material(ltex, "mat_logo", mask=True)
    logo = g.node(mesh=g.quad(10, 10, lmat, name="mesh_logo"), translation=[0, 1, 0], name="Logo")
    roots = [cam, logo]; channels = []
    for i, t in enumerate(TOPS):
        tex = g.image(f"tex_{t}.png"); mat = g.material(tex, f"mat_{t}", mask=True)
        node = g.node(mesh=g.quad(HALF, HALF, mat, name=f"mesh_{t}"),
                      translation=list(ring_pos(2 * math.pi * i / len(TOPS))), name=f"top_{t}")
        roots.append(node)
        th0 = 2 * math.pi * i / len(TOPS)
        times = [DUR * k / (NKEY - 1) for k in range(NKEY)]
        pos = [ring_pos(th0 + 2 * math.pi * k / (NKEY - 1)) for k in range(NKEY)]
        channels.append((node, times, pos))
    g.add_anim(channels)
    g.save(path, roots)

def main():
    os.chdir(HERE)
    build_gltf("banner.gltf")
    subprocess.run([sys.executable, PYCGFX, "banner.gltf"], check=True)
    subprocess.run([BANNERTOOL, "makebanner", "-ci", "banner.cgfx", "-a", AUDIO,
                    "-o", "banner_3d.bnr"], check=True)
    print("built banner_3d.bnr")

if __name__ == "__main__":
    main()
