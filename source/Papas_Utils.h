#pragma once

#include "Papas_Constants.h"
#include <3ds.h>
#include <citro2d.h>

namespace Papas {

	struct v2 {
		float x;
		float y;
		v2(float num = 0) { x = num, y = num; };
	};

	struct rect {
		float left;
		float top;
		float width;
		float height;
	};

	class Button {
	public:
		PapasError createButton(C2D_SpriteSheet& spriteSheet, int unpressed, int pressed, v2 position = v2(0));
		bool showButton(touchPosition& touch); //Returns true whilst pressed

		const rect getRect() const { return hitBox; };

	private:
		
		C2D_Image img_Pressed;
		C2D_Image img_Unpressed;

		v2 pos;
		rect hitBox;

		bool pressed;
	};


}