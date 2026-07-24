"""Shared SWF parsing core for the intro pipeline (extracted from gen_rig.py
patterns). Parses tags, matrices, cxforms; resolves char kinds, sprite
timelines, JPEXS export anchors (SVG bounds for shapes, union render bounds
for sprites), and child-sprite stop frames from exported scripts."""
import os, re, glob, zlib, struct

class R:
    def __init__(s, d, p=0): s.d = d; s.p = p; s.bb = 0; s.bc = 0
    def align(s): s.bb = 0; s.bc = 0
    def u8(s): v = s.d[s.p]; s.p += 1; return v
    def u16(s): v = struct.unpack_from("<H", s.d, s.p)[0]; s.p += 2; return v
    def u32(s): v = struct.unpack_from("<I", s.d, s.p)[0]; s.p += 4; return v
    def bits(s, n):
        v = 0
        for _ in range(n):
            if s.bc == 0: s.bb = s.d[s.p]; s.p += 1; s.bc = 8
            v = (v << 1) | ((s.bb >> 7) & 1); s.bb = (s.bb << 1) & 0xFF; s.bc -= 1
        return v
    def sbits(s, n):
        v = s.bits(n)
        if n and (v >> (n - 1)) & 1: v -= (1 << n)
        return v
    def string0(s):
        st = s.p
        while s.d[s.p] != 0: s.p += 1
        v = s.d[st:s.p].decode("latin1"); s.p += 1
        return v

def read_matrix(r):
    r.align(); a = d = 1.0; b = c = 0.0
    if r.bits(1): nb = r.bits(5); a = r.sbits(nb) / 65536.0; d = r.sbits(nb) / 65536.0
    if r.bits(1): nb = r.bits(5); b = r.sbits(nb) / 65536.0; c = r.sbits(nb) / 65536.0
    nb = r.bits(5); tx = r.sbits(nb); ty = r.sbits(nb); r.align()
    return (a, b, c, d, tx / 20.0, ty / 20.0)

def read_cxform(r):
    r.align(); hasAdd = r.bits(1); hasMult = r.bits(1); nb = r.bits(4)
    mult = [256, 256, 256, 256]; add = [0, 0, 0, 0]
    if hasMult: mult = [r.sbits(nb) for _ in range(4)]
    if hasAdd: add = [r.sbits(nb) for _ in range(4)]
    r.align()
    return (tuple(mult), tuple(add))

IDENT_CX = ((256, 256, 256, 256), (0, 0, 0, 0))

def parse_tags(d, start, end):
    r = R(d, start); out = []
    while r.p < end:
        rec = r.u16(); code = rec >> 6; ln = rec & 0x3F
        if ln == 0x3F: ln = r.u32()
        out.append((code, d[r.p:r.p + ln])); r.p += ln
        if code == 0: break
    return out

def mat_mul(m1, m2):
    """Apply m2 first, then m1 (world = m1 . m2)."""
    a1, b1, c1, d1, tx1, ty1 = m1
    a2, b2, c2, d2, tx2, ty2 = m2
    return (a1 * a2 + c1 * b2, b1 * a2 + d1 * b2,
            a1 * c2 + c1 * d2, b1 * c2 + d1 * d2,
            a1 * tx2 + c1 * ty2 + tx1, b1 * tx2 + d1 * ty2 + ty1)

def cx_mul(c1, c2):
    m1, a1 = c1; m2, a2 = c2
    return (tuple(m1[i] * m2[i] // 256 for i in range(4)),
            tuple((a2[i] * m1[i] // 256) + a1[i] for i in range(4)))

def apply_pt(m, x, y):
    a, b, c, d, tx, ty = m
    return (a * x + c * y + tx, b * x + d * y + ty)

class Swf:
    SHAPE = {2, 22, 32, 83}
    MORPH = {46, 84}

    def __init__(self, root):
        self.root = root
        raw = open(os.path.join(root, "papaspizzeria_v2.swf"), "rb").read()
        assert raw[0:3] == b"CWS"
        body = zlib.decompress(raw[8:])
        r = R(body)
        nb = r.bits(5)
        for _ in range(4): r.sbits(nb)
        r.align()
        self.fps = r.u16() / 256.0
        r.u16()
        self.tags = parse_tags(body, r.p, len(body))
        self.kind = {}
        self.sprite_tag = {}
        self.sprite_nframes = {}
        for code, tb in self.tags:
            if code in self.SHAPE:
                cid = struct.unpack_from("<H", tb, 0)[0]; self.kind[cid] = "shape"
            elif code in self.MORPH:
                cid = struct.unpack_from("<H", tb, 0)[0]; self.kind[cid] = "morph"
            elif code == 39:
                cid = struct.unpack_from("<H", tb, 0)[0]
                self.kind[cid] = "sprite"
                self.sprite_tag[cid] = tb
                self.sprite_nframes[cid] = struct.unpack_from("<H", tb, 2)[0]
        self._svg = {}
        self._ub = {}
        self._tl = {}
        self._stop = {}

    # JPEXS shape SVG render bounds: canvas size + origin (see gen_rig.py)
    _size_re = re.compile(r'height="([0-9.]+)px" width="([0-9.]+)px"')
    _g_re = re.compile(r'<g transform="matrix\(([-0-9.]+), ([-0-9.]+), ([-0-9.]+), ([-0-9.]+), ([-0-9.]+), ([-0-9.]+)\)">')

    def svg_path(self, cid):
        for sub in ("shapes", "morphshapes"):
            p = os.path.join(self.root, f"papas_extract/{sub}/{cid}.svg")
            if os.path.isfile(p): return p
        return None

    def shape_bounds(self, cid):
        """(xmin,ymin,xmax,ymax) px in shape space from the JPEXS SVG."""
        if cid in self._svg: return self._svg[cid]
        res = None
        p = self.svg_path(cid)
        if p:
            svg = open(p).read()
            m = self._size_re.search(svg); g = self._g_re.search(svg)
            if m and g:
                h, w = float(m.group(1)), float(m.group(2))
                tx, ty = float(g.group(5)), float(g.group(6))
                res = (-tx, -ty, -tx + w, -ty + h)
        self._svg[cid] = res; return res

    def timeline(self, cid):
        """List of frames; each frame is {depth: (char, matrix, cxform)}.
        Matrices/cxforms persist across frames as in the SWF display list."""
        if cid in self._tl: return self._tl[cid]
        inner = parse_tags(self.sprite_tag[cid], 4, len(self.sprite_tag[cid]))
        frames = []; cur = {}
        for code, tb in inner:
            if code == 1:
                frames.append(dict(cur))
            elif code == 26:
                rr = R(tb, 0); flags = rr.u8(); dp = rr.u16()
                prev = cur.get(dp)
                ch = rr.u16() if flags & 2 else (prev[0] if prev else None)
                mt = read_matrix(rr) if flags & 4 else (prev[1] if prev else (1, 0, 0, 1, 0, 0))
                cx = read_cxform(rr) if flags & 8 else (prev[2] if prev else IDENT_CX)
                if flags & 16: rr.u16()  # ratio (morph state) - not reconstructed
                # reset the child's clock when a new char is placed at the depth
                if ch is not None:
                    cur[dp] = (ch, mt, cx)
            elif code == 28:
                dp = struct.unpack_from("<H", tb, 0)[0]; cur.pop(dp, None)
        self._tl[cid] = frames; return frames

    def morph_ratios(self, cid, want_depth):
        """[(frame, char, ratio)] for one depth of a sprite, 1-based frames,
        covering only the frames the depth is occupied. Morph tweens are the
        one thing timeline() throws away, and they can't be recovered from
    the JPEXS exports whenever anything else on the clip renders wrong."""
        inner = parse_tags(self.sprite_tag[cid], 4, len(self.sprite_tag[cid]))
        out = []; f = 1; ch = None; ratio = None
        for code, tb in inner:
            if code == 1:
                if ch is not None: out.append((f, ch, ratio))
                f += 1
            elif code == 26:
                rr = R(tb, 0); flags = rr.u8(); dp = rr.u16()
                if dp != want_depth: continue
                if flags & 2: ch = rr.u16(); ratio = None
                if flags & 4: read_matrix(rr)
                if flags & 8: read_cxform(rr)
                if flags & 16: ratio = rr.u16()
            elif code == 28:
                if struct.unpack_from("<H", tb, 0)[0] == want_depth:
                    ch = None; ratio = None
        return out

    def union_bounds(self, cid, depth=0):
        """Union render bounds across all frames = JPEXS PNG export canvas."""
        if cid in self._ub: return self._ub[cid]
        if depth > 14: return None
        res = self.shape_bounds(cid)
        if res is None and cid in self.sprite_tag:
            xmin = ymin = 1e18; xmax = ymax = -1e18; ok = False
            for fr in self.timeline(cid):
                for dp, (ch, mt, cx) in fr.items():
                    if ch is None: continue
                    cb = self.union_bounds(ch, depth + 1)
                    if not cb: continue
                    x0, y0, x1, y1 = cb
                    for px, py in ((x0, y0), (x1, y0), (x0, y1), (x1, y1)):
                        wx, wy = apply_pt(mt, px, py)
                        xmin = min(xmin, wx); xmax = max(xmax, wx)
                        ymin = min(ymin, wy); ymax = max(ymax, wy); ok = True
            res = (xmin, ymin, xmax, ymax) if ok else None
        self._ub[cid] = res; return res

    def stop_frame(self, cid):
        """1-based frame a child clip stops on (from JPEXS script exports),
        or None if it loops freely."""
        if cid in self._stop: return self._stop[cid]
        res = None
        pat = os.path.join(self.root, f"papas_extract/scripts/DefineSprite_{cid}*/frame_*")
        for d in glob.glob(pat):
            m = re.search(r"frame_(\d+)$", d)
            if not m: continue
            for asf in glob.glob(os.path.join(d, "*.as")):
                if "stop()" in open(asf).read():
                    f = int(m.group(1))
                    res = f if res is None else min(res, f)
        self._stop[cid] = res; return res

    def child_frame(self, cid, elapsed):
        """Which 1-based frame a child sprite shows `elapsed` frames after
        being placed (loops unless a stop() script pins it)."""
        n = self.sprite_nframes.get(cid, 1)
        if n <= 1: return 1
        stop = self.stop_frame(cid)
        if stop is not None and elapsed >= stop - 1:
            return stop
        return (elapsed % n) + 1
