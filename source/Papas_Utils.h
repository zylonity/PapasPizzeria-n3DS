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

	enum Toppings{
		Pepperoni,
		Meat,
		Mushroom,
		Pepper,
		Onion,
		Olive,
		Anochovie
	};

	enum Places
	{
		Quarter,
		Half,
		Quarters3,
		Full
	};

	struct ReceiptSection
	{
		Places place;
		Toppings topping;
		int Quantity;
	};

	struct ReceiptParts{
		C2D_Font *dokyo;
		v2 pos;
		v2 scale;
		C2D_Sprite receipt_bg;
		C2D_TextBuf receipt_Buf;
		C2D_Text receipt_text;
		bool pinnedTop;
		bool topReceipt;
		rect hitBox;
		//ReceiptSection sections[8];
		void init(const char *receiptNum, C2D_Font *font, C2D_SpriteSheet& receipt_spriteSheet, v2 posToGive, v2 scaleToGive, bool top);
		void moveReceipt(v2 moveTo);
		void setPosReceipt(v2 moveTo);
		void scaleReceipt(v2 scaleTo);
		void setScaleReceipt(v2 scaleTo);
		void renderReceipt();
	};

	struct Receipt{
		ReceiptParts top, bottom;
		//Position and scales to dock the ticket screens
		const v2 snapPosTop = {277.6f, 0.0f};
		const v2 bigScaleTop = {0.95f, 0.95f};
		const v2 smallScaleTop = {0.35f, 0.35f};

		const v2 snapPosBottom = {198.0f, 40.0f};
		const v2 bigScaleBottom = {0.85f, 0.85f};
		const v2 smallScaleBottom = {0.32f, 0.32f};

		//Actual functions to do with creating a receipt on both screens;
		void init(int receiptNum, C2D_Font *font, C2D_SpriteSheet& receipt_spriteSheet);
		void showReceipt(bool topReceipt);
		void detectMovement(touchPosition &touch);
		void terminate();
	};
	

	class ReceiptManager{
		public:
		ReceiptManager() {};
		ReceiptManager(C2D_Font *font);

		void initManager(C2D_Font *font);
		void terminateManager();

		void createReceipt();
		void renderReceipt(bool topReceipt);
		void detectMovement(touchPosition &touch);
		

	private:
	//Text stuff
		C2D_Font* dokyo;
		C2D_SpriteSheet receipt_spriteSheet;

		std::vector<Receipt> v_receipts;
		u16 maxReceipts;

		
	};
}