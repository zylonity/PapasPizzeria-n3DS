#include "Papas_Stereo.h"
#include <cmath>

namespace {
	float s_scale = 0.0f;         // signed px per plane unit for the current eye
	float s_currentOffset = 0.0f; // what the view matrix is set to right now

	// Swapping the view matrix flushes the sprite batch, so only touch it
	// when the offset actually changes.
	void applyOffset(float xoff, bool force = false)
	{
		if (!force && xoff == s_currentOffset)
			return;
		s_currentOffset = xoff;
		C3D_Mtx m;
		Mtx_Identity(&m);
		m.r[0].w = xoff;
		C2D_ViewRestore(&m);
	}
}

void Papas::Stereo::beginEye(Eye eye, float slider)
{
	// Layers behind the screen project left for the left eye and right for
	// the right eye, so the left eye takes the negative shift.
	s_scale = (eye == EyeLeft ? -STRENGTH : STRENGTH) * slider;
	applyOffset(0.0f, true);
}

void Papas::Stereo::endEye()
{
	s_scale = 0.0f;
	applyOffset(0.0f, true);
}

void Papas::Stereo::plane(float depth)
{
	// Whole pixels: the art is drawn pixel-snapped and a fractional shift
	// would smear it under bilinear filtering.
	applyOffset(roundf(depth * s_scale));
}
