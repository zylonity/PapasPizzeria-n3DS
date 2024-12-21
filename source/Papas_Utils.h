#pragma once

#include "Papas_Constants.h"
#include <3ds.h>
#include <citro2d.h>

namespace Papas {

	struct v2 {
		float x;
		float y;

		// Constructor for single value
		v2(float value) : x(value), y(value) {}

		// Constructor for two values
		v2(float _x, float _y) : x(_x), y(_y) {}

		// Default constructor
		v2() : x(0.0f), y(0.0f) {}
	};

	struct rect {
		float left;
		float top;
		float width;
		float height;
	};

	class Button {
	public:
		Button() {};
		Button(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, v2 position = v2(0));
		void createButton(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, v2 position = v2(0));
		bool showButton(touchPosition& touch, bool selected = false); //Returns true whilst pressed
		void setPosition(v2 postoSet);

		const rect getRect() const { return hitBox; };

	private:
		
		C2D_Image img_Pressed;
		C2D_Image img_Selected;
		C2D_Image img_Unpressed;

		v2 pos;
		rect hitBox;

		bool pressed;
	};


}