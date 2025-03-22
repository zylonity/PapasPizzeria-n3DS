#pragma once

#include "Papas_Constants.h"
#include <3ds.h>
#include <citro2d.h>
#include <vector>
#include <chrono>
#include <glm/vec2.hpp>

namespace Papas {

	// struct v2 {
	// 	float x;
	// 	float y;

	// 	// Constructor for single value
	// 	v2(float value) : x(value), y(value) {}

	// 	// Constructor for two values
	// 	v2(float _x, float _y) : x(_x), y(_y) {}

	// 	// Default constructor
	// 	v2() : x(0.0f), y(0.0f) {}
	// };

	struct rect {
		float left;
		float top;
		float width;
		float height;
	};

	class Button {
	public:
		Button() {};
		Button(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, glm::vec2 position = glm::vec2(0));
		void createButton(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, glm::vec2 position = glm::vec2(0));
		bool showButton(touchPosition& touch, bool selected = false, bool* aPressed = nullptr); //Returns true whilst pressed
		void setPosition(glm::vec2 postoSet);

		const rect getRect() const { return hitBox; };

	private:
		
		C2D_Image img_Pressed;
		C2D_Image img_Selected;
		C2D_Image img_Unpressed;

		glm::vec2 pos;
		rect hitBox;

		bool wasPressed;
	};

	class AnimatedSprite {
	public:
		AnimatedSprite() {};
		AnimatedSprite(const char *spriteSheet, float time, glm::vec2 position = glm::vec2(0), glm::vec2 scale = glm::vec2(0), float rotation = 0);
		void createAnim(const char *spriteSheet, float time, glm::vec2 position = glm::vec2(0), glm::vec2 scale = glm::vec2(0), float rotation = 0);
		void destroyAnim();

		virtual void renderAnim(bool loop);

		void setPosition(glm::vec2 postoSet);
		void setRotation(glm::vec2 postoSet);

	protected:
		
		std::vector<C2D_Sprite> each_sprite;
		C2D_SpriteSheet s_spriteSheet;

		glm::vec2 pos;
		glm::vec2 size;
		float rot;

		u64 start;
		u64 end;
		int numOfSprites;
		float animTime;

		int currentSprite;

		bool finished;

	};

	class RoyPeeking : public AnimatedSprite
	{
	public:
		using AnimatedSprite::AnimatedSprite;

		void renderAnim(bool loop) override;
		void resetAnim();
		void renderAnimBackwards(bool loop);

	private:
		bool ranOnce = false;
	};

	class RoyTakingOrder : public AnimatedSprite
	{
	public:
		using AnimatedSprite::AnimatedSprite;

		bool renderAnimWithPauses(int pauses, float pauseTime);
		void resetAnim();

	private:
		bool pauseTriggered = false;
		bool paused = false;
		int currentPauses = 0;
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

	struct ItemOrder{
		int Coverage[4];
		Toppings Topping;
		int Quantity;
	};

	struct ReceiptSubSection
	{
		int cover[4];
		C2D_Image i_cover[4];
		Toppings topping;
		C2D_Image i_topping;
		int Quantity;
		C2D_Image i_Quantity;
	};

	struct ReceiptParts{
		C2D_Font *dokyo;
		glm::vec2  pos;
		glm::vec2  scale;
		C2D_Sprite receipt_bg;
		C2D_Image s_nums_cross;
		bool needleDrawn;
		int selectedCut;
		C2D_Sprite needle;
		C2D_Image s_cuts[4];
		C2D_Image s_nums[12];
		C2D_Image s_topps[7];
		C2D_Image s_quarters[4];
		C2D_TextBuf receipt_Buf;
		C2D_Text receipt_text;
		bool pinnedTop;
		bool topReceipt;
		rect hitBox;
		float currentDepth;
		size_t currentItems;
		ReceiptSubSection sections[7];
		void init(const char *receiptNum, C2D_Font *font, C2D_SpriteSheet &receipt_spriteSheet, glm::vec2 posToGive, glm::vec2 scaleToGive, bool top);
		void moveReceipt(glm::vec2  moveTo);
		void setPosReceipt(glm::vec2  moveTo);
		void scaleReceipt(glm::vec2  scaleTo);
		void setScaleReceipt(glm::vec2  scaleTo);
		void addItem(int size[4], Toppings top, int Quant);
		void addTime(int time);
		void addCut(int slices);
		void renderReceipt();
	};

	struct Receipt{
		ReceiptParts top, bottom;
		//Position and scales to dock the ticket screens
		const glm::vec2  snapPosTop = {277.6f, 0.0f};
		const glm::vec2  bigScaleTop = {0.95f, 0.95f};
		const glm::vec2  smallScaleTop = {0.35f, 0.35f};

		const glm::vec2  snapPosBottom = {198.0f, 40.0f};
		const glm::vec2  bigScaleBottom = {0.85f, 0.85f};
		const glm::vec2  smallScaleBottom = {0.32f, 0.32f};

		bool dockInUse;

		//Actual functions to do with creating a receipt on both screens;
		void init(int receiptNum, C2D_Font *font, C2D_SpriteSheet &receipt_spriteSheet);
		void showReceipt(bool topReceipt);
		void detectMovement(touchPosition &touch);
		bool detectTouch(touchPosition &touch);
		void addItem(int size[4], Toppings topping, int Quant);
		void addTime(int time);
		void addCut(int slices);
		void forceDocking();
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
		void renderDockedReceipt(bool topReceipt);

		void detectMovement(touchPosition &touch);
		void moveReceiptToBack(int indexToMove);

		void getDockedReceipt(Receipt** returnReceipt);
		

	private:
	//Text stuff
		float normalizedDepth();
		C2D_Font* dokyo;
		C2D_SpriteSheet receipt_spriteSheet;
		C2D_SpriteSheet receipt_spriteSheet2;
		std::vector<Receipt*> v_receipts;
		u16 maxReceipts;
		size_t activeReceiptIndex;
		float currentMaxDepth;
	};
}