#pragma once
#include <citro2d.h>

// Top-screen stereo: plane() sets parallax while Citro2D depth keeps draw order.
namespace Papas {
	namespace Stereo {

		enum Eye { EyeLeft, EyeRight };

		// Horizontal shift per plane at full slider.
		constexpr float STRENGTH = 5.0f;

		// Wrap each eye pass, then reset the view for the bottom screen.
		void beginEye(Eye eye, float slider);
		void endEye();

		// 0 is the glass; positive goes inward and negative pops outward.
		void plane(float depth);
	}
}
