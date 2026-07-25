#include "Papas_Stereo.h"
#include <cmath>

namespace {
	float s_scale = 0.0f;         // signed px per plane unit for the current eye
	float s_currentOffset = 0.0f; // what the view matrix is set to right now

	// Change the view only when needed because it flushes the sprite batch.
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
	// Inward layers shift left for the left eye and right for the right eye.
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
	// Keep shifts on whole pixels so filtered art stays crisp.
	applyOffset(roundf(depth * s_scale));
}
