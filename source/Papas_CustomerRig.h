#pragma once
//===============================================================================
// name: Papas_CustomerRig.h
// Skeletal (Flash-cutout) customer animation, reconstructed from the original
// SWF. One shared rig (clip 409) drives all 36 customer types: 15 flat limb
// slots, drawn back-to-front by depth, each with a baked per-frame affine matrix.
// Data lives in romfs:/rig/customer.rig (see tools/gen_rig.py); limb art is a
// per-type t3x atlas (romfs:/gfx/customerN.t3x) loaded on demand.
//===============================================================================
#include "Papas_Constants.h"
#include <citro2d.h>
#include <cstdint>

namespace Papas {

// ---- On-disk POD layout (little-endian, mirrors gen_rig.py) -------------------
#pragma pack(push, 1)
struct RigHeader {
    char     magic[4];      // "PRIG"
    uint32_t version;       // 1
    uint16_t numSlots;      // 15
    uint16_t numFrames;     // 816
    uint16_t numSegments;   // 12
    uint16_t numTypes;      // 36
    uint16_t numParts;      // 12
    uint16_t fps;           // 30
    uint8_t  _pad[12];      // header padded to 32 bytes (4+4+12+12)
};
static_assert(sizeof(RigHeader) == 32, "RigHeader must match gen_rig.py's 32-byte header");
struct RigSlot {
    char    name[16];
    uint8_t partId;         // index into a type's part table
    uint8_t _pad;
};
struct RigSegment {
    char     name[16];
    uint16_t start;         // absolute start frame
    uint16_t length;        // frames in the segment
    uint8_t  loop;          // 1 = loops
    uint8_t  _pad;
};
struct RigPart {            // per (type, part)
    int16_t  ox, oy;        // registration offset: PNG top-left in slot-local px
    uint16_t w, h;          // image size px
    uint16_t imageIndex;    // first frame's index in the type atlas
    uint16_t frameCount;    // 1 for rigid parts, N for expression parts, 0 = absent
};
#pragma pack(pop)

// A loaded per-type limb atlas.
struct RigTypeAtlas {
    C2D_SpriteSheet sheet = nullptr;
    int             typeId = -1;
};

class CustomerRig {
public:
    // Loads romfs:/rig/customer.rig once. Safe to call repeatedly.
    static CustomerRig& shared();
    PapasError load(const char* path = "romfs:/rig/customer.rig");
    void       unload();

    // Per-type limb textures (VRAM win: only load types currently on screen).
    PapasError loadType(int typeId, RigTypeAtlas& out) const;
    void       freeType(RigTypeAtlas& atlas) const;

    // Segment lookup + time->absolute-frame mapping.
    int  segmentIndex(const char* name) const;   // -1 if not found
    int  frameForTime(int segIdx, float seconds) const;

    // Draw a posed customer. (x,y) is where the rig origin lands; scaleX<0 flips
    // (facing). exprEyes/exprMouth pick a sub-frame for those expression parts.
    void draw(const RigTypeAtlas& atlas, int typeId, int absFrame,
              float x, float y, float scaleX, float scaleY,
              float baseDepth = 0.5f,
              int exprEyes = -1, int exprMouth = -1) const; // <0 = use baked track

    bool loaded() const { return m_buf != nullptr; }
    const RigHeader* header() const { return m_hdr; }

private:
    CustomerRig() = default;
    const float* slotMatrix(int frame, int slot) const {
        return m_mats + (static_cast<size_t>(frame) * m_hdr->numSlots + slot) * 6;
    }
    const RigPart* partTable(int typeId) const; // nullptr if type absent

    uint8_t*          m_buf = nullptr;   // whole file
    const RigHeader*  m_hdr = nullptr;
    const RigSlot*    m_slots = nullptr;
    const RigSegment* m_segs = nullptr;
    const float*      m_mats = nullptr;
    const uint8_t*    m_types = nullptr; // stride = 2 + numParts*sizeof(RigPart)
    int               m_typeStride = 0;
    const uint8_t*    m_expr = nullptr;  // [frame*numSlots + slot] sub-frame index
};

} // namespace Papas
