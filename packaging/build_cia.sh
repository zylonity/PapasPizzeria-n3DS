#!/usr/bin/env bash
# Build the installable CIA with its icon, 3D banner, and stock boot logo.
# Keep cxitool's stock logo: this firmware rejects removed or custom logos.
set -euo pipefail

PROJ="$(cd "$(dirname "$0")/.." && pwd)"
TOOLS="/home/khaleel/Downloads/3dstools-cxi-stuff"
PKG="$PROJ/packaging"

# Keep the unique ID in 0xFF000-0xFF7FF and its hardware nibble at zero.
TID="000400000FF4A000"
NAME="pizzeria"                 # process name (<=8 chars)
CODE="CTR-P-PAPA"               # product code
VER_MAJOR=0; VER_MINOR=5; VER_MICRO=0   # title version 0.5.0

# Rebuild banner3d/banner_3d.bnr after changing its art or scene.
BANNER="$PKG/banner3d/banner_3d.bnr"

cd "$PROJ"
[ -f PapasPizzeria-n3DS.3dsx ] || { echo "Build the .3dsx first (make)"; exit 1; }
[ -f "$BANNER" ] || { echo "Missing $BANNER (run packaging/banner3d/make_banner.py)"; exit 1; }

echo ">> cxi (+ 3D banner)"
"$TOOLS/cxitool" --name="$NAME" --code="$CODE" -t "$TID" \
    -b "$BANNER" PapasPizzeria-n3DS.3dsx PapasPizzeria-n3DS.cxi

echo ">> cia"
"$TOOLS/makerom" -f cia -o PapasPizzeria-n3DS.cia \
    -target t -ignoresign -content PapasPizzeria-n3DS.cxi:0:0
rm -f PapasPizzeria-n3DS.cxi

# Patch the TMD version because this makerom's -minor flag is broken.
echo ">> set title version v$VER_MAJOR.$VER_MINOR.$VER_MICRO"
python3 - "$VER_MAJOR" "$VER_MINOR" "$VER_MICRO" "$TID" <<'PY'
import struct as st, sys
maj,mino,mic=(int(x) for x in sys.argv[1:4]); tid=int(sys.argv[4],16)
ver=((maj&0x3f)<<10)|((mino&0x3f)<<4)|(mic&0xf)
p="PapasPizzeria-n3DS.cia"; d=bytearray(open(p,"rb").read())
def u32(o):return st.unpack("<I",d[o:o+4])[0]
def align(x,a=64):return (x+a-1)&~(a-1)
off=align(u32(0)); off=align(off+u32(8)); tmd=align(off+u32(0xC))
sizes={0x10000:0x200,0x10001:0x100,0x10002:0x3C,0x10003:0x200,0x10004:0x100,0x10005:0x3C}
body=tmd+4+sizes[st.unpack(">I",d[tmd:tmd+4])[0]]; body=(body+0x3F)&~0x3F
assert st.unpack(">Q",d[body+0x4C:body+0x54])[0]==tid, "TMD anchor check failed"
d[body+0x9C:body+0x9E]=st.pack(">H",ver)
open(p,"wb").write(d)
print("   TMD title version set to raw=%d (v%d.%d.%d)"%(ver,maj,mino,mic))
PY

echo ">> done: $PROJ/PapasPizzeria-n3DS.cia"
