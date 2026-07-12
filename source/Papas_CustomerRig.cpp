//===============================================================================
// name: Papas_CustomerRig.cpp
// desc: Loads/draws the shared customer rig. Is a SINGLETON
//===============================================================================

#include "Papas_CustomerRig.h"
#include <citro3d.h>
#include <cstdio>
#include <cstring>
#include <cmath>

PapasError Papas::CustomerRig::load(const char* path)
{
	if (buffer)
		return PAPAS_OK; // already loaded

	FILE* f = fopen(path, "rb");
	ASSERT(f != nullptr, "customer.rig not found in romfs");
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer = static_cast<uint8_t*>(malloc(sz));
	ASSERT(buffer != nullptr, "OOM loading customer.rig");
	size_t got = fread(buffer, 1, sz, f);
	fclose(f);
	ASSERT(got == static_cast<size_t>(sz), "short read on customer.rig");

	header = reinterpret_cast<const RigHeader*>(buffer);
	ASSERT(memcmp(header->magic, "PRIG", 4) == 0, "bad rig magic");

	const uint8_t* p = buffer + sizeof(RigHeader);         // 32
	slots = reinterpret_cast<const RigSlot*>(p);
	p += header->numSlots * sizeof(RigSlot);
	segments = reinterpret_cast<const RigSegment*>(p);
	p += header->numSegments * sizeof(RigSegment);
	// v4+: the file pads here so the matrix block sits on a 4-byte boundary.
	// We read it through a raw float*, and VFP loads on the real ARM11
	// data-abort on unaligned addresses (emulators don't check).
	ASSERT(header->version >= 4, "customer.rig too old, regenerate with gen_rig.py");
	p += (4 - ((p - buffer) & 3)) & 3;
	matrices = reinterpret_cast<const float*>(p);
	p += static_cast<size_t>(header->numFrames) * header->numSlots * 6 * sizeof(float);
	types = p;
	typeStride = 2 + header->numParts * sizeof(RigPart);
	p += static_cast<size_t>(header->numTypes) * typeStride;
	// v2+: per-frame expression sub-frame track
	exprTrack = (header->version >= 2) ? p : nullptr;

	return PAPAS_OK;
}

void Papas::CustomerRig::unload()
{
	if (buffer)
	{
		free(buffer);
		buffer = nullptr;
	}
	header = nullptr;
	slots = nullptr;
	segments = nullptr;
	matrices = nullptr;
	types = nullptr;
	exprTrack = nullptr;
}

const Papas::RigPart* Papas::CustomerRig::partTable(int typeId) const
{
	for (int i = 0; i < header->numTypes; ++i)
	{
		const uint8_t* rec = types + static_cast<size_t>(i) * typeStride;
		uint16_t id;
		memcpy(&id, rec, 2);
		if (id == typeId)
			return reinterpret_cast<const RigPart*>(rec + 2);
	}
	return nullptr;
}

PapasError Papas::CustomerRig::loadType(int typeId, RigTypeAtlas& out, const char* variant) const
{
	char path[64];
	snprintf(path, sizeof(path), "romfs:/gfx/customer%d%s.t3x", typeId, variant);
	out.sheet = C2D_SpriteSheetLoad(path);
	ASSERT(out.sheet != nullptr, "failed to load customer type atlas");
	out.typeId = typeId;

	for (size_t i = 0; i < C2D_SpriteSheetCount(out.sheet); i++)
	{
		C2D_Image img = C2D_SpriteSheetGetImage(out.sheet, i);
		C3D_TexSetFilter(img.tex, GPU_LINEAR, GPU_LINEAR); //Adds bilinear filtering, the sprite looks a bit pixelated otherwise
	}

	return PAPAS_OK;
}

void Papas::CustomerRig::freeType(RigTypeAtlas& atlas) const
{
	if (atlas.sheet)
	{
		C2D_SpriteSheetFree(atlas.sheet);
		atlas.sheet = nullptr;
	}
	atlas.typeId = -1;
}

int Papas::CustomerRig::segmentIndex(const char* name) const
{
	for (int i = 0; i < header->numSegments; ++i)
	{
		if (strncmp(segments[i].name, name, 16) == 0)
			return i;
	}
	return -1;
}

int Papas::CustomerRig::frameForTime(int segIdx, float seconds) const
{
	if (segIdx < 0 || segIdx >= header->numSegments)
		return 0;

	const RigSegment& s = segments[segIdx];
	if (s.length <= 1)
		return s.start;

	int local = static_cast<int>(seconds * header->fps);
	if (s.loop)
		local %= s.length;
	else if (local >= s.length)
		local = s.length - 1;

	return s.start + local;
}

bool Papas::CustomerRig::segmentDone(int segIdx, float seconds) const
{
	if (segIdx < 0 || segIdx >= header->numSegments)
		return true;

	const RigSegment& s = segments[segIdx];
	if (s.loop)
		return false; // looping segments never finish

	return static_cast<int>(seconds * header->fps) >= s.length;
}

// Build a citro2d view matrix from a 2x3 affine [a b c d tx ty] (pixel space).
static void affineToMtx(C3D_Mtx* m, float a, float b, float c, float d, float tx, float ty)
{
	memset(m, 0, sizeof(*m));
	m->r[0].x = a; m->r[0].y = c; m->r[0].z = 0; m->r[0].w = tx;
	m->r[1].x = b; m->r[1].y = d; m->r[1].z = 0; m->r[1].w = ty;
	m->r[2].z = 1;
	m->r[3].w = 1;
}

void Papas::CustomerRig::draw(const RigTypeAtlas& atlas, int typeId, int absFrame,
							  float x, float y, float scaleX, float scaleY,
							  float baseDepth, int exprEyes, int exprMouth) const
{
	if (!buffer || !atlas.sheet)
		return;

	const RigPart* parts = partTable(typeId);
	if (!parts)
		return;

	if (absFrame < 0)
		absFrame = 0;
	if (absFrame >= header->numFrames)
		absFrame = header->numFrames - 1;

	C3D_Mtx saved;
	C2D_ViewSave(&saved);
	// citro2d applies one model matrix per batch at flush time, so flush any
	// geometry queued before us (drawn with the identity/current matrix) first.
	C2D_Flush();

	for (int si = 0; si < header->numSlots; ++si)
	{
		const RigSlot& slot = slots[si];
		const RigPart& part = parts[slot.partId];
		if (part.frameCount == 0)
			continue; // absent limb for this type

		// Per-frame expression sub-frame (eyes/mouth/hands/feet), baked from the
		// original timeline's gotoAndStop() scripts. exprEyes/exprMouth override.
		int sub = 0;
		if (part.frameCount > 1)
		{
			if (exprTrack)
				sub = exprTrack[static_cast<size_t>(absFrame) * header->numSlots + si];
			if (strncmp(slot.name, "eyes", 16) == 0 && exprEyes >= 0)
				sub = exprEyes;
			else if (strncmp(slot.name, "mouth", 16) == 0 && exprMouth >= 0)
				sub = exprMouth;
			if (sub < 0)
				sub = 0;
			if (sub >= part.frameCount)
				sub = part.frameCount - 1;
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
		// Compose with whatever view was active when we were called (the
		// stereo eye shift), instead of stomping it.
		C3D_Mtx composed;
		Mtx_Multiply(&composed, &saved, &m);
		C2D_ViewRestore(&composed);

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

		// v3+: the body can carry a shirt logo (customer 1 only). A flipped
		// customer would show it mirror-reversed, so redraw it mirrored about
		// its own center (the original swapped to a pre-mirrored logo frame 2).
		if (scaleX < 0.0f && header->version >= 3 &&
			strncmp(slot.name, "body", 16) == 0)
		{
			const RigPart& logo = parts[header->numParts - 1]; // logo is the last part
			if (logo.frameCount > 0)
			{
				C2D_Image logoImg = C2D_SpriteSheetGetImage(atlas.sheet, logo.imageIndex);
				// compose "mirror about the logo's local center x" into the slot matrix
				float cx = logo.ox + logo.w * 0.5f;
				C3D_Mtx lm;
				affineToMtx(&lm, -a, -b, c, d, tx + 2.0f * cx * a, ty + 2.0f * cx * b);
				C3D_Mtx lcomposed;
				Mtx_Multiply(&lcomposed, &saved, &lm);
				C2D_ViewRestore(&lcomposed);

				// logo.w/h is the display size; the atlas image is larger and
				// gets scaled into this quad.
				C2D_DrawParams lp = {
					{ (float)logo.ox, (float)logo.oy, (float)logo.w, (float)logo.h },
					{ 0.0f, 0.0f },
					baseDepth,
					0.0f
				};
				C2D_DrawImage(logoImg, &lp, nullptr);
				C2D_Flush();
			}
		}
	}

	C2D_ViewRestore(&saved);
}
//===============================================================================
