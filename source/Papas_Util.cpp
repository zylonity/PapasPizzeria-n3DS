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

Papas::ReceiptManager::ReceiptManager(C2D_Font *font, int num)
{
	createReceipt(font, num);
}

void Papas::ReceiptManager::createReceipt(C2D_Font *font, int num)
{
	dokyo = font;

	receipt_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/receipt.t3x");
	C2D_SpriteFromSheet(&receipt_bg, receipt_spriteSheet, 0);
	tempReceipt.pReceipt_bg = &receipt_bg;

	tempReceipt.receipt_Buf = C2D_TextBufNew(4);

	//int to 000 whatever number
	std::ostringstream formatted;
	formatted << std::setw(3) << std::setfill('0') << num;

	C2D_TextFontParse(&tempReceipt.receipt_text, *dokyo, tempReceipt.receipt_Buf, formatted.str().c_str());
	C2D_TextOptimize(&tempReceipt.receipt_text);

	tempReceipt.pos = {0, 0};
	tempReceipt.scale = {1, 1};
	tempReceipt.hitBox = {tempReceipt.pReceipt_bg->params.pos.x, tempReceipt.pReceipt_bg->params.pos.y, tempReceipt.pReceipt_bg->params.pos.w, tempReceipt.pReceipt_bg->params.pos.h};
}

void Papas::ReceiptManager::showReceipt()
{
	C2D_DrawSprite(tempReceipt.pReceipt_bg);
	// Calculate the new position for the text based on scale
	float scaledTextX = tempReceipt.pos.x + (39 * tempReceipt.scale.x);
	float scaledTextY = tempReceipt.pos.y + (19 * tempReceipt.scale.y);

	// Draw the text at the scaled position
	C2D_DrawText(&tempReceipt.receipt_text, C2D_WithColor, scaledTextX, scaledTextY, 0, tempReceipt.scale.x, tempReceipt.scale.y, C2D_Color32(240, 156, 156, 255));
	//C2D_DrawText(&tempReceipt.receipt_text, C2D_WithColor, tempReceipt.pos.x + 39, tempReceipt.pos.y + 19, 0, tempReceipt.scale.x, tempReceipt.scale.y, C2D_Color32(240, 156, 156, 255));
}

void Papas::ReceiptManager::moveReceipt(v2 moveBy)
{
	C2D_SpriteMove(tempReceipt.pReceipt_bg, moveBy.x, moveBy.y);
	tempReceipt.hitBox = {tempReceipt.pReceipt_bg->params.pos.x, tempReceipt.pReceipt_bg->params.pos.y, tempReceipt.pReceipt_bg->params.pos.w, tempReceipt.pReceipt_bg->params.pos.h};
	tempReceipt.pos.x = tempReceipt.pos.x + moveBy.x;
	tempReceipt.pos.y = tempReceipt.pos.y + moveBy.y;
}

void Papas::ReceiptManager::scaleReceipt(v2 scaleBy)
{
	C2D_SpriteScale(tempReceipt.pReceipt_bg, scaleBy.x, scaleBy.y);
	tempReceipt.hitBox = {tempReceipt.pReceipt_bg->params.pos.x, tempReceipt.pReceipt_bg->params.pos.y, tempReceipt.pReceipt_bg->params.pos.w, tempReceipt.pReceipt_bg->params.pos.h};
	tempReceipt.scale.x = tempReceipt.scale.x * scaleBy.x;
	tempReceipt.scale.y = tempReceipt.scale.y * scaleBy.y;
}

void Papas::ReceiptManager::destroyReceipt()
{
	C2D_SpriteSheetFree(receipt_spriteSheet);
	C2D_TextBufDelete(tempReceipt.receipt_Buf);
}