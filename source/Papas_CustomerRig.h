#pragma once
//===============================================================================
// name: Papas_CustomerRig.h
// desc: Skeletal (Flash-cutout) customer animation, reconstructed from the
//       original SWF. One shared rig (clip 409) drives all 36 customer types:
//       15 flat limb slots, drawn back-to-front by depth, each with a baked
//       per-frame affine matrix. Data lives in romfs:/rig/customer.rig (see
//       tools/gen_rig.py); limb art is a per-type t3x atlas
//       (romfs:/gfx/customerN.t3x) loaded on demand. Is a SINGLETON
//===============================================================================

#include "Papas_Constants.h"
#include <citro2d.h>
#include <cstdint>

namespace Papas {

	// On-disk POD layout (little-endian, mirrors gen_rig.py)
#pragma pack(push, 1)
	struct RigHeader {
		char     magic[4];      // "PRIG"
		uint32_t version;
		uint16_t numSlots;      // 15
		uint16_t numFrames;     // 816
		uint16_t numSegments;   // 12
		uint16_t numTypes;      // 36
		uint16_t numParts;      // 13 (v3+: the shirt logo rides last)
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
		PapasError load(const char* path = "romfs:/rig/customer.rig");
		void       unload();

		// Per-type limb textures (VRAM win: only load types currently on screen).
		// variant "" = take-order-sized art, "_line" = lobby-line-sized art;
		// each context samples ~1:1 so baked edge AA stays one screen pixel.
		PapasError loadType(int typeId, RigTypeAtlas& out, const char* variant = "") const;
		void       freeType(RigTypeAtlas& atlas) const;

		// Segment lookup + time->absolute-frame mapping.
		int  segmentIndex(const char* name) const;   // -1 if not found
		int  frameForTime(int segIdx, float seconds) const;
		bool segmentDone(int segIdx, float seconds) const; // non-looping segment played out

		// Draw a posed customer. (x,y) is where the rig origin lands; scaleX<0 flips
		// (facing). exprEyes/exprMouth pick a sub-frame for those expression parts.
		void draw(const RigTypeAtlas& atlas, int typeId, int absFrame,
				  float x, float y, float scaleX, float scaleY,
				  float baseDepth = 0.5f,
				  int exprEyes = -1, int exprMouth = -1) const; // <0 = use baked track

		bool loaded() const { return buffer != nullptr; }
		const RigHeader* getHeader() const { return header; }

		//===============================================================================
		// Singleton Implementations
		static CustomerRig& getInstance()
		{
			static CustomerRig instance; // Guaranteed to be destroyed.
			return instance;
		}
		// Make deleted functions public for nicer error messages (~ Scott Myers)
		CustomerRig(CustomerRig const&)  = delete;	// Copy constructor
		void operator=(CustomerRig const&) = delete;	// Assignment Operator
		//===============================================================================

	private:
		const float* slotMatrix(int frame, int slot) const {
			return matrices + (static_cast<size_t>(frame) * header->numSlots + slot) * 6;
		}
		const RigPart* partTable(int typeId) const; // nullptr if type absent

		uint8_t*          buffer = nullptr;     // whole file
		const RigHeader*  header = nullptr;
		const RigSlot*    slots = nullptr;
		const RigSegment* segments = nullptr;
		const float*      matrices = nullptr;
		const uint8_t*    types = nullptr;      // stride = 2 + numParts*sizeof(RigPart)
		int               typeStride = 0;
		const uint8_t*    exprTrack = nullptr;  // [frame*numSlots + slot] sub-frame index

		//===============================================================================
		// Singleton Implementations (Banned functions to prevent a new instance)
		CustomerRig()
		{
		} // Default Constructor private so can only be called from within
		//===============================================================================
	};
}
//===============================================================================
