#pragma once
#include <citro2d.h>

// Stereoscopic 3D for the top screen. The renderer draws the top scene twice
// (one pass per eye); scenes mark how deep each layer sits with plane(), and
// the pass shifts that layer horizontally by that eye's parallax. The citro2d
// depth values stay pure draw-order keys; stereo depth is set only by plane().
namespace Papas {
	namespace Stereo {

		enum Eye { EyeLeft, EyeRight };

		// px of horizontal shift per plane unit at full slider; layers one
		// plane unit apart end up 2*STRENGTH px separated between the eyes
		constexpr float STRENGTH = 5.0f;

		// Renderer only: wrap each top-screen pass. beginEye starts the pass
		// at plane 0; endEye resets the view matrix for the bottom screen.
		void beginEye(Eye eye, float slider);
		void endEye();

		// Layer depth for the draws that follow, in plane units:
		// 0 = at the screen glass, positive = into the scene (wallpaper ~1),
		// negative = pops out toward the player. No-op while the slider is off.
		void plane(float depth);
	}
}
