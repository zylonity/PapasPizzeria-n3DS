#pragma once

#include "Papas_Constants.h"
#include <3ds.h>
#include <citro2d.h>
#include <vector>
#include <chrono>

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
		bool showButton(touchPosition& touch, bool selected = false, bool* aPressed = nullptr); //Returns true whilst pressed
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

	class AnimatedSprite {
	public:
		AnimatedSprite() {};
		AnimatedSprite(const char *spriteSheet, float time, v2 position = v2(0), v2 scale = v2(0), float rotation = 0);
		void createAnim(const char *spriteSheet, float time, v2 position = v2(0), v2 scale = v2(0), float rotation = 0);
		void destroyAnim();

		virtual void renderAnim(bool loop);

		void setPosition(v2 postoSet);
		void setRotation(v2 postoSet);

	protected:
		
		std::vector<C2D_Sprite> each_sprite;
		C2D_SpriteSheet s_spriteSheet;

		v2 pos;
		v2 size;
		float rot;

		u64 start;
		u64 end;
		int numOfSprites;
		float animTime;

		int currentSprite;

		bool finished;

	};

	class GuyPeeking : public AnimatedSprite
	{
	public:
		using AnimatedSprite::AnimatedSprite;

		void renderAnim(bool loop) override;
		void resetAnim();
		void renderAnimBackwards(bool loop);

	private:
		bool ranOnce = false;
	};

	class Receipt{
		public:
		Receipt() {};
		Receipt(int receiptNum);
		
		void createReceipt(int receiptNum);
		void showReceipt();

		private:
			C2D_SpriteSheet receipt_spriteSheet;
			C2D_Sprite receipt_bg;
	};

	// class ReceiptManager
	// {
	// public:
	// 	ReceiptManager() {};
	// 	//ReceiptManager(int receiptNum);

	// 	void createReceipt(int receiptNum);
	// 	void showReceipt();

	// private:
	// 	C2D_SpriteSheet receipt_spriteSheet;
	// 	C2D_Sprite receipt_bg;
	// };
}