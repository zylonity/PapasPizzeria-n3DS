//===============================================================================
// name: Papas_CustomerRig.cpp
//===============================================================================
#include "Papas_CustomerRig.h"
#include <citro3d.h>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace Papas {

CustomerRig& CustomerRig::shared() {
    static CustomerRig instance;
    return instance;
}

PapasError CustomerRig::load(const char* path) {
    if (m_buf) return PAPAS_OK; // already loaded

    FILE* f = fopen(path, "rb");
    ASSERT(f != nullptr, "customer.rig not found in romfs");
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    m_buf = static_cast<uint8_t*>(malloc(sz));
    ASSERT(m_buf != nullptr, "OOM loading customer.rig");
    size_t got = fread(m_buf, 1, sz, f);
    fclose(f);
    ASSERT(got == static_cast<size_t>(sz), "short read on customer.rig");

    m_hdr = reinterpret_cast<const RigHeader*>(m_buf);
    ASSERT(memcmp(m_hdr->magic, "PRIG", 4) == 0, "bad rig magic");

    const uint8_t* p = m_buf + sizeof(RigHeader);          // 32
    m_slots = reinterpret_cast<const RigSlot*>(p);
    p += m_hdr->numSlots * sizeof(RigSlot);
    m_segs = reinterpret_cast<const RigSegment*>(p);
    p += m_hdr->numSegments * sizeof(RigSegment);
    m_mats = reinterpret_cast<const float*>(p);
    p += static_cast<size_t>(m_hdr->numFrames) * m_hdr->numSlots * 6 * sizeof(float);
    m_types = p;
    m_typeStride = 2 + m_hdr->numParts * sizeof(RigPart);
    p += static_cast<size_t>(m_hdr->numTypes) * m_typeStride;
    // v2+: per-frame expression sub-frame track
    m_expr = (m_hdr->version >= 2) ? p : nullptr;
    return PAPAS_OK;
}

void CustomerRig::unload() {
    if (m_buf) { free(m_buf); m_buf = nullptr; }
    m_hdr = nullptr; m_slots = nullptr; m_segs = nullptr; m_mats = nullptr; m_types = nullptr;
}

const RigPart* CustomerRig::partTable(int typeId) const {
    for (int i = 0; i < m_hdr->numTypes; ++i) {
        const uint8_t* rec = m_types + static_cast<size_t>(i) * m_typeStride;
        uint16_t id;
        memcpy(&id, rec, 2);
        if (id == typeId)
            return reinterpret_cast<const RigPart*>(rec + 2);
    }
    return nullptr;
}

PapasError CustomerRig::loadType(int typeId, RigTypeAtlas& out) const {
    char path[64];
    snprintf(path, sizeof(path), "romfs:/gfx/customer%d.t3x", typeId);
    out.sheet = C2D_SpriteSheetLoad(path);
    ASSERT(out.sheet != nullptr, "failed to load customer type atlas");
    out.typeId = typeId;
    return PAPAS_OK;
}

void CustomerRig::freeType(RigTypeAtlas& atlas) const {
    if (atlas.sheet) { C2D_SpriteSheetFree(atlas.sheet); atlas.sheet = nullptr; }
    atlas.typeId = -1;
}

int CustomerRig::segmentIndex(const char* name) const {
    for (int i = 0; i < m_hdr->numSegments; ++i)
        if (strncmp(m_segs[i].name, name, 16) == 0) return i;
    return -1;
}

int CustomerRig::frameForTime(int segIdx, float seconds) const {
    if (segIdx < 0 || segIdx >= m_hdr->numSegments) return 0;
    const RigSegment& s = m_segs[segIdx];
    if (s.length <= 1) return s.start;
    int local = static_cast<int>(seconds * m_hdr->fps);
    if (s.loop) local %= s.length;
    else if (local >= s.length) local = s.length - 1;
    return s.start + local;
}

// Build a citro2d view matrix from a 2x3 affine [a b c d tx ty] (pixel space).
static void affineToMtx(C3D_Mtx* m, float a, float b, float c, float d, float tx, float ty) {
    memset(m, 0, sizeof(*m));
    m->r[0].x = a; m->r[0].y = c; m->r[0].z = 0; m->r[0].w = tx;
    m->r[1].x = b; m->r[1].y = d; m->r[1].z = 0; m->r[1].w = ty;
    m->r[2].z = 1;
    m->r[3].w = 1;
}

void CustomerRig::draw(const RigTypeAtlas& atlas, int typeId, int absFrame,
                       float x, float y, float scaleX, float scaleY,
                       float baseDepth, int exprEyes, int exprMouth) const {
    if (!m_buf || !atlas.sheet) return;
    const RigPart* parts = partTable(typeId);
    if (!parts) return;
    if (absFrame < 0) absFrame = 0;
    if (absFrame >= m_hdr->numFrames) absFrame = m_hdr->numFrames - 1;

    C3D_Mtx saved;
    C2D_ViewSave(&saved);
    // citro2d applies one model matrix per batch at flush time, so flush any
    // geometry queued before us (drawn with the identity/current matrix) first.
    C2D_Flush();

    for (int si = 0; si < m_hdr->numSlots; ++si) {
        const RigSlot& slot = m_slots[si];
        const RigPart& part = parts[slot.partId];
        if (part.frameCount == 0) continue; // absent limb for this type

        // Per-frame expression sub-frame (eyes/mouth/hands/feet), baked from the
        // original timeline's gotoAndStop() scripts. exprEyes/exprMouth override.
        int sub = 0;
        if (part.frameCount > 1) {
            if (m_expr)
                sub = m_expr[static_cast<size_t>(absFrame) * m_hdr->numSlots + si];
            if      (strncmp(slot.name, "eyes", 16) == 0 && exprEyes  >= 0) sub = exprEyes;
            else if (strncmp(slot.name, "mouth", 16) == 0 && exprMouth >= 0) sub = exprMouth;
            if (sub < 0) sub = 0;
            if (sub >= part.frameCount) sub = part.frameCount - 1;
        }
        C2D_Image img = C2D_SpriteSheetGetImage(atlas.sheet, part.imageIndex + sub);

        // combined = clipPlacement(scale,translate) * slotMatrix
        const float* sm = slotMatrix(absFrame, si);
        float a = scaleX * sm[0];
        float b = scaleY * sm[1];
        float c = scaleX * sm[2];
        float d = scaleY * sm[3];
        float tx = scaleX * sm[4] + x;
        float ty = scaleY * sm[5] + y;

        C3D_Mtx m;
        affineToMtx(&m, a, b, c, d, tx, ty);
        C2D_ViewRestore(&m);

        // draw the limb quad at its registration offset in slot-local space
        C2D_DrawParams p = {
            { (float)part.ox, (float)part.oy, (float)part.w, (float)part.h },
            { 0.0f, 0.0f },
            baseDepth,
            0.0f
        };
        C2D_DrawImage(img, &p, nullptr);
        // flush so THIS limb is rendered with matrix m before the next one
        // changes the model matrix.
        C2D_Flush();
    }

    C2D_ViewRestore(&saved);
}

} // namespace Papas
