#include "papas_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iomanip>
#include <sstream>

Papas::Button::Button(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, v2 position) {

	createButton(spriteSheet, unpressed, selected, pressed, position);
}

void Papas::Button::createButton(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, v2 position) {

	// Load the sprites
	img_Unpressed = C2D_SpriteSheetGetImage(spriteSheet, selected);
	img_Selected = C2D_SpriteSheetGetImage(spriteSheet, unpressed);
	img_Pressed = C2D_SpriteSheetGetImage(spriteSheet, pressed);

	pos = position;
	hitBox.left = pos.x;
	hitBox.top = pos.y;
	hitBox.height = img_Pressed.subtex->height;
	hitBox.width = img_Pressed.subtex->width;
}

bool Papas::Button::showButton(touchPosition& touch, bool selected, bool* aPressed) {

	hitBox.left = pos.x;
	hitBox.top = pos.y;
	hitBox.height = img_Pressed.subtex->height;
	hitBox.width = img_Pressed.subtex->width;

	//Detect touch
	//If between left and between right and between top and between bottom
	if (touch.px > hitBox.left && touch.px < hitBox.left + hitBox.width && touch.py > hitBox.top && touch.py < hitBox.top + hitBox.height) {
		C2D_DrawImageAt(img_Pressed, pos.x, pos.y, 1, NULL, 1, 1);
		return true;
	}
	else {

		if (selected && *aPressed == true) {
			*aPressed = false;
			C2D_DrawImageAt(img_Pressed, pos.x, pos.y, 1, NULL, 1, 1);
			return true;
		}

		if(selected)
			C2D_DrawImageAt(img_Selected, pos.x, pos.y, 1, NULL, 1, 1);
		else
			C2D_DrawImageAt(img_Unpressed, pos.x, pos.y, 1, NULL, 1, 1);

		return false;
	}

	

}

void Papas::Button::setPosition(v2 showPos) {

	pos = showPos;
	hitBox.left = pos.x;
	hitBox.top = pos.y;
	hitBox.height = img_Pressed.subtex->height;
	hitBox.width = img_Pressed.subtex->width;

}

Papas::AnimatedSprite::AnimatedSprite(const char *spriteSheet, float time, v2 position, v2 scale, float defaultRotation)
{

	createAnim(spriteSheet, time, position, scale, defaultRotation);
}

void Papas::AnimatedSprite::createAnim(const char *spriteSheet, float time, v2 position, v2 scale, float defaultRotation)
{

	s_spriteSheet = C2D_SpriteSheetLoad(spriteSheet);

	numOfSprites = C2D_SpriteSheetCount(s_spriteSheet);
	animTime = time;
	pos = position;
	size = scale;
	rot = defaultRotation;



	for (size_t i = 0; i < numOfSprites; i++)
	{
		C2D_Sprite spr;
		C2D_SpriteFromSheet(&spr, s_spriteSheet, i);
		C2D_SpriteSetPos(&spr, pos.x, pos.y);
		C2D_SpriteSetScale(&spr, size.x, size.y);
		C2D_SpriteSetRotationDegrees(&spr, rot);
		C2D_SpriteSetDepth(&spr, 0.7f);
		C3D_TexSetFilter(spr.image.tex, GPU_LINEAR, GPU_LINEAR); //Adds bilinear filtering, the sprite looks a bit pixelated otherwise
		
		each_sprite.push_back(spr);
	}
	start = osGetTime();
	currentSprite = 0;
}

void Papas::AnimatedSprite::renderAnim(bool loop)
{

	end = osGetTime();

	if (start - end >= animTime)
	{
		if(currentSprite < numOfSprites - 1){
			currentSprite++;
		}
		else if (currentSprite == numOfSprites - 1)
		{
			if(loop){
				currentSprite = 0;
			}
			else{
				finished = true;
			}
			
		}

		start = osGetTime();
	}

	if (!finished)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
}

void Papas::AnimatedSprite::destroyAnim()
{

	C2D_SpriteSheetFree(s_spriteSheet);
}

void Papas::GuyPeeking::renderAnim(bool loop)
{
	end = osGetTime();

	if (start - end >= animTime)
	{
		if(ranOnce == false){
			if (currentSprite < numOfSprites - 1)
			{
				currentSprite++;
			}
			else if (currentSprite == numOfSprites - 1) 
			{
				// We slow down the last few frames, since I had to remove them to load them onto the sprite sheet
				// Don't need to worry about copy cos C2D_Sprite holds a pointer to the texture anyways
				for (size_t i = 0; i < 45; i++)
				{
					each_sprite.push_back(each_sprite[28]);
				}
				numOfSprites += 45;
				ranOnce = true;
			}
		}
		else{
			if (currentSprite < numOfSprites - 1)
			{
				currentSprite++;
			}
			else if (currentSprite == numOfSprites - 1)
			{
				if(loop){
					currentSprite = 16;
				}
				else{
					finished = true;
				}
				
			}
		}
		

		start = osGetTime();
	}

	if (!finished)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
}

void Papas::GuyPeeking::renderAnimBackwards(bool loop)
{

	if(finished == false){
		if (currentSprite == 0)
		{
			currentSprite = 15;
		}
	}


	end = osGetTime();

	if (start - end >= animTime)
	{

		if (currentSprite > 1)
		{
			currentSprite--;
		}
		else if (currentSprite == 1)
		{
			finished = true;
		}

		start = osGetTime();
	}

	if (finished == false)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
}

void Papas::GuyPeeking::resetAnim()
{
	finished = false;
	currentSprite = 0;
}


void Papas::Receipt::init(const char *receiptNum, C2D_Font *font, C2D_SpriteSheet &pass_receipt_spriteSheet, v2 posToGive, v2 scaleToGive)
{
	dokyo = font;

	pos = posToGive;
	scale = scaleToGive;

	*receipt_spriteSheet = pass_receipt_spriteSheet;

	C2D_SpriteFromSheet(pReceipt_bg, *receipt_spriteSheet, 0);

	receipt_Buf = C2D_TextBufNew(4);

	C2D_TextFontParse(&receipt_text, *dokyo, receipt_Buf, receiptNum);
	C2D_TextOptimize(&receipt_text);

	C2D_SpriteSetPos(pReceipt_bg, pos.x, pos.y);
	C2D_SpriteSetScale(pReceipt_bg, scale.x, scale.y);
	C3D_TexSetFilter(pReceipt_bg->image.tex, GPU_LINEAR, GPU_LINEAR);
	hitBox = {pReceipt_bg->params.pos.x, pReceipt_bg->params.pos.y, pReceipt_bg->params.pos.w, pReceipt_bg->params.pos.h};
}

Papas::ReceiptManager::ReceiptManager(C2D_Font *font, int num)
{
	createReceipt(font, num);
}

void Papas::ReceiptManager::createReceipt(C2D_Font *font, int num)
{
	dokyo = font;

	receipt_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/receipt.t3x");
	C2D_SpriteFromSheet(&receipt_bg, receipt_spriteSheet, 0);
	C2D_SpriteFromSheet(&receipt_bg_top, receipt_spriteSheet, 0);
	tempReceipt.pReceipt_bg = &receipt_bg;
	tempReceipt_top.pReceipt_bg = &receipt_bg_top;

	tempReceipt.receipt_Buf = C2D_TextBufNew(4);
	tempReceipt_top.receipt_Buf = C2D_TextBufNew(4);

	//int to 000 whatever number
	std::ostringstream formatted;
	formatted << std::setw(3) << std::setfill('0') << num;

	C2D_TextFontParse(&tempReceipt.receipt_text, *dokyo, tempReceipt.receipt_Buf, formatted.str().c_str());
	C2D_TextOptimize(&tempReceipt.receipt_text);

	C2D_TextFontParse(&tempReceipt_top.receipt_text, *dokyo, tempReceipt_top.receipt_Buf, formatted.str().c_str());
	C2D_TextOptimize(&tempReceipt_top.receipt_text);

	tempReceipt_top.pos = {277.6f, 0};
	tempReceipt_top.scale = {0.95f, 0.95f};
	C2D_SpriteSetPos(tempReceipt_top.pReceipt_bg, tempReceipt_top.pos.x, tempReceipt_top.pos.y);
	//C2D_SpriteSetDepth(tempReceipt_top.pReceipt_bg, 10.0f);
	C2D_SpriteSetScale(tempReceipt_top.pReceipt_bg, tempReceipt_top.scale.x, tempReceipt_top.scale.y);
	C3D_TexSetFilter(tempReceipt_top.pReceipt_bg->image.tex, GPU_LINEAR, GPU_LINEAR);

	tempReceipt.pos = {198, 40};
	tempReceipt.scale = {0.85f, 0.85f};
	C2D_SpriteSetPos(tempReceipt.pReceipt_bg, tempReceipt.pos.x, tempReceipt.pos.y);
	C2D_SpriteSetScale(tempReceipt.pReceipt_bg, tempReceipt.scale.x, tempReceipt.scale.y);
	C3D_TexSetFilter(tempReceipt.pReceipt_bg->image.tex, GPU_LINEAR, GPU_LINEAR);
	tempReceipt.hitBox = {tempReceipt.pReceipt_bg->params.pos.x, tempReceipt.pReceipt_bg->params.pos.y, tempReceipt.pReceipt_bg->params.pos.w, tempReceipt.pReceipt_bg->params.pos.h};
}

void Papas::ReceiptManager::showReceipt()
{
	C2D_DrawSprite(tempReceipt.pReceipt_bg);
	// Calculate the new position for the text based on scale
	float scaledTextX = tempReceipt.pos.x + (39 * tempReceipt.scale.x);
	float scaledTextY = tempReceipt.pos.y + (19 * tempReceipt.scale.y);
	
	// Draw the text at the scaled position
	C2D_DrawText(&tempReceipt.receipt_text, C2D_WithColor, scaledTextX, scaledTextY, 0.0f, tempReceipt.scale.x, tempReceipt.scale.y, C2D_Color32(240, 156, 156, 255));
}

void Papas::ReceiptManager::showReceiptTop()
{
	C2D_SpriteSetDepth(tempReceipt_top.pReceipt_bg, 0.9f);
	C2D_DrawSprite(tempReceipt_top.pReceipt_bg);
	// Calculate the new position for the text based on scale
	float scaledTextX = tempReceipt_top.pos.x + (39 * tempReceipt_top.scale.x);
	float scaledTextY = tempReceipt_top.pos.y + (19 * tempReceipt_top.scale.y);

	// Draw the text at the scaled position
	C2D_DrawText(&tempReceipt_top.receipt_text, C2D_WithColor, scaledTextX, scaledTextY, 1.0f, tempReceipt_top.scale.x, tempReceipt_top.scale.y, C2D_Color32(240, 156, 156, 255));
}

void Papas::ReceiptManager::moveReceipt(v2 moveBy, Receipt& receiptToMove)
{
	C2D_SpriteMove(receiptToMove.pReceipt_bg, moveBy.x, moveBy.y);

	receiptToMove.pos.x = receiptToMove.pos.x + moveBy.x;
	receiptToMove.pos.y = receiptToMove.pos.y + moveBy.y;
}

void Papas::ReceiptManager::setPosReceipt(v2 pos, Receipt& receiptToMove)
{
	C2D_SpriteSetPos(receiptToMove.pReceipt_bg, pos.x, pos.y);

	receiptToMove.pos.x = pos.x;
	receiptToMove.pos.y = pos.y;
}

void Papas::ReceiptManager::scaleReceipt(v2 scaleBy, Receipt& receiptToMove)
{
	C2D_SpriteScale(receiptToMove.pReceipt_bg, scaleBy.x, scaleBy.y);
	receiptToMove.scale.x = receiptToMove.scale.x * scaleBy.x;
	receiptToMove.scale.y = receiptToMove.scale.y * scaleBy.y;
}

void Papas::ReceiptManager::setScaleReceipt(v2 scaleBy, Receipt& receiptToMove)
{
	C2D_SpriteSetScale(receiptToMove.pReceipt_bg, scaleBy.x, scaleBy.y);
	receiptToMove.scale.x = scaleBy.x;
	receiptToMove.scale.y = scaleBy.y;
}

void Papas::ReceiptManager::detectMovement(touchPosition &touch)
{

	tempReceipt.hitBox = {tempReceipt.pReceipt_bg->params.pos.x, tempReceipt.pReceipt_bg->params.pos.y, tempReceipt.pReceipt_bg->params.pos.w, tempReceipt.pReceipt_bg->params.pos.h};

	static bool isDragging = false; 
	static touchPosition prevTouch;
	static v2 offset;

	if (touch.px != 0 || touch.py !=0) //I wanna account for latency, if the receipt is moving but you move outside the hitbox faster than the hitbox can move, it still drags
	{
		if (isDragging ||
			(touch.px > tempReceipt.hitBox.left && touch.px < tempReceipt.hitBox.left + tempReceipt.hitBox.width &&
			 touch.py > tempReceipt.hitBox.top && touch.py < tempReceipt.hitBox.top + tempReceipt.hitBox.height))
		{

			if (!isDragging)
			{
				//Set the initial position and scale as soon as you start grabbing it, to prevent a "jolt" or "jump" (idk but this fixes that)
				setScaleReceipt(v2(0.32f, 0.32f), tempReceipt);

				if (tempReceipt.pinnedTop)
				{
					setPosReceipt(v2(touch.px - (tempReceipt.hitBox.width * 0.5f), touch.py - (tempReceipt.hitBox.height * 0.3f)), tempReceipt);
				}
				else{
					setPosReceipt(v2(touch.px - tempReceipt.hitBox.width * 0.2f, touch.py - tempReceipt.hitBox.height * 0.1f), tempReceipt);
				}

				

				isDragging = true;
			}
			else
			{
				moveReceipt(v2(touch.px - prevTouch.px, touch.py - prevTouch.py), tempReceipt);
			}

			prevTouch = touch;
			
		}
	}
	else if (isDragging)
	{
		if(tempReceipt.pos.y < 30.0f){
			setPosReceipt(v2(tempReceipt.pos.x, 5), tempReceipt);
			tempReceipt.pinnedTop = true;
			setScaleReceipt(v2(0.35f, 0.35f), tempReceipt_top);
			setPosReceipt(v2(tempReceipt.pos.x, -7.0f), tempReceipt_top);
			
		}
		else{
			setPosReceipt(v2(198, 40), tempReceipt);
			setScaleReceipt(v2(0.85f, 0.85f), tempReceipt);
			tempReceipt.pinnedTop = false;
			setPosReceipt(v2(277.6f, 0), tempReceipt_top);
			setScaleReceipt(v2(0.95f, 0.95f), tempReceipt_top);
			
		}
		
		isDragging = false;
	}
}

void Papas::ReceiptManager::destroyReceipt()
{
	C2D_SpriteSheetFree(receipt_spriteSheet);
	C2D_TextBufDelete(tempReceipt.receipt_Buf);
}