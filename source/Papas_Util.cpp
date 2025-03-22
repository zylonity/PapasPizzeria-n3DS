#include "papas_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdlib> // for rand() and srand()
#include <vector>

Papas::Button::Button(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, glm::vec2 position) {

	createButton(spriteSheet, unpressed, selected, pressed, position);
}

void Papas::Button::createButton(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, glm::vec2 position) {

	wasPressed = false;
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

	static touchPosition lastTouch;

	hitBox.left = pos.x;
	hitBox.top = pos.y;
	hitBox.height = img_Pressed.subtex->height;
	hitBox.width = img_Pressed.subtex->width;

	//Detect touch
	//If between left and between right and between top and between bottom
	if (touch.px == 0.0f && touch.py == 0.0f && lastTouch.px > hitBox.left && lastTouch.px < hitBox.left + hitBox.width && lastTouch.py > hitBox.top && lastTouch.py < hitBox.top + hitBox.height)
	{
		C2D_DrawImageAt(img_Pressed, pos.x, pos.y, 1, NULL, 1, 1);

		if (!wasPressed)
		{
			wasPressed = true; 
			return true;
		}
	}
	else {
		wasPressed = false;

		if (selected && *aPressed == true) {
			*aPressed = false;
			C2D_DrawImageAt(img_Pressed, pos.x, pos.y, 1, NULL, 1, 1);
			return true;
		}

		if(selected)
			C2D_DrawImageAt(img_Selected, pos.x, pos.y, 1, NULL, 1, 1);
		else
			C2D_DrawImageAt(img_Unpressed, pos.x, pos.y, 1, NULL, 1, 1);

		lastTouch = touch;
	}


	return false;
}

void Papas::Button::setPosition(glm::vec2 showPos) {

	pos = showPos;
	hitBox.left = pos.x;
	hitBox.top = pos.y;
	hitBox.height = img_Pressed.subtex->height;
	hitBox.width = img_Pressed.subtex->width;

}

Papas::AnimatedSprite::AnimatedSprite(const char *spriteSheet, float time, glm::vec2 position, glm::vec2 scale, float defaultRotation)
{

	createAnim(spriteSheet, time, position, scale, defaultRotation);
}

void Papas::AnimatedSprite::createAnim(const char *spriteSheet, float time, glm::vec2 position, glm::vec2 scale, float defaultRotation)
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

	if (end - start >= animTime)
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

void Papas::RoyPeeking::renderAnim(bool loop)
{
	end = osGetTime();

	if (end - start >= animTime)
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

void Papas::RoyPeeking::renderAnimBackwards(bool loop)
{

	if(finished == false){
		if (currentSprite == 0)
		{
			currentSprite = 15;
		}
	}


	end = osGetTime();

	if (end - start >= animTime)
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

void Papas::RoyPeeking::resetAnim()
{
	finished = false;
	currentSprite = 0;
}

bool Papas::RoyTakingOrder::renderAnimWithPauses(int pauses, float pauseTime)
{
	end = osGetTime();
	pauseTriggered = false;

	if (paused == false && end - start >= animTime)
	{
		if(currentSprite == 1){
			pauseTriggered = true;
		}
		if (currentSprite < numOfSprites - 1)
		{
			currentSprite++;
		}
		else if (currentSprite == numOfSprites - 1)
		{
			if (currentPauses < pauses)
			{
				
				paused = true;
				currentSprite = 0;
			}
			else
			{
				finished = true;
			}
		}

		start = osGetTime();
	}

	if (!finished && !paused)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}

	if (paused)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
		size_t currentWait = end - start;
		if (currentWait >= pauseTime)
		{
			
			paused = false;
			currentPauses++;
			start = osGetTime();
		}
	}

	if (finished)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
	if (pauseTriggered)
	{
		pauseTriggered = false; // Reset the flag immediately
		return true;
	}

	return false;
}

void Papas::RoyTakingOrder::resetAnim(){
	finished = false;
	pauseTriggered = false;
	paused = false;
	currentSprite = 0;
	currentPauses = 0;
}

void Papas::ReceiptParts::init(const char *receiptNum, C2D_Font *font, C2D_SpriteSheet &receipt_spriteSheet, glm::vec2 posToGive, glm::vec2 scaleToGive, bool top)
{
	dokyo = font;

	topReceipt = top;
	pos = posToGive;
	scale = scaleToGive;
	currentDepth = 0.01f;
	currentItems = 0;
	needleDrawn = false;
	selectedCut = -1;

	C2D_SpriteFromSheet(&receipt_bg, receipt_spriteSheet, 0);



	//Needle
	C2D_SpriteFromSheet(&needle, receipt_spriteSheet, 25);
	C2D_SpriteSetCenterRaw(&needle, 3, 19);
	C3D_TexSetFilter(needle.image.tex, GPU_LINEAR, GPU_LINEAR);

	//Cuts/Slices
	for (size_t i = 0; i < 4; i++)
	{
		s_cuts[i] = C2D_SpriteSheetGetImage(receipt_spriteSheet, i + 26);
		C3D_TexSetFilter(s_cuts[i].tex, GPU_LINEAR, GPU_LINEAR);
	}
	//Numbers and cross
	//I want cross to be 0th item cos that way each number corresponds to itself soo:
	s_nums_cross = C2D_SpriteSheetGetImage(receipt_spriteSheet, 13);
	C3D_TexSetFilter(s_nums_cross.tex, GPU_LINEAR, GPU_LINEAR);
	for (size_t i = 0; i < 12; i++)
	{
		s_nums[i] = C2D_SpriteSheetGetImage(receipt_spriteSheet, i + 1);
		C3D_TexSetFilter(s_nums[i].tex, GPU_LINEAR, GPU_LINEAR);
	}
	//Toppings for receipt
	for (size_t i = 0; i < 7; i++)
	{
		s_topps[i] = C2D_SpriteSheetGetImage(receipt_spriteSheet, i + 14);
		C3D_TexSetFilter(s_topps[i].tex, GPU_LINEAR, GPU_LINEAR);
	}
	//Coverage for receipt
	for (size_t i = 0; i < 4; i++)
	{
		s_quarters[i] = C2D_SpriteSheetGetImage(receipt_spriteSheet, i + 21);
		C3D_TexSetFilter(s_quarters[i].tex, GPU_LINEAR, GPU_LINEAR);
	}

	//Deal with text (number on the top)
	receipt_Buf = C2D_TextBufNew(4);
	C2D_TextFontParse(&receipt_text, *dokyo, receipt_Buf, receiptNum);
	C2D_TextOptimize(&receipt_text);

	//Deal with the background sprite and hitbox
	C2D_SpriteSetPos(&receipt_bg, pos.x, pos.y);
	C2D_SpriteSetScale(&receipt_bg, scale.x, scale.y);
	C3D_TexSetFilter(receipt_bg.image.tex, GPU_LINEAR, GPU_LINEAR);
	C2D_SpriteSetDepth(&receipt_bg, currentDepth);
	hitBox = {receipt_bg.params.pos.x, receipt_bg.params.pos.y, receipt_bg.params.pos.w, receipt_bg.params.pos.h};
}

void Papas::ReceiptParts::addItem(int size[4], Toppings top, int Quant)
{
	for (size_t i = 0; i < 4; i++)
	{
		sections[currentItems].cover[i] = size[i];
		sections[currentItems].i_cover[i] = s_quarters[i];
	}
	
	
	sections[currentItems].topping = top;
	sections[currentItems].i_topping = s_topps[top];
	sections[currentItems].Quantity = Quant;
	sections[currentItems].i_Quantity = s_nums[Quant-1];
	currentItems++;
}

void Papas::ReceiptParts::addTime(int time)
{
	needleDrawn = true;
	C2D_SpriteSetRotationDegrees(&needle, time * 45);
}

void Papas::ReceiptParts::addCut(int slices)
{
	switch (slices){
		case 2:
		selectedCut = 0;
		break;
		case 4:
		selectedCut = 1;
		break;
		case 6:
		selectedCut = 2;
		break;
		case 8:
		selectedCut = 3;
		break;
	}
}

void Papas::ReceiptParts::moveReceipt(glm::vec2 moveBy)
{
	C2D_SpriteMove(&receipt_bg, moveBy.x, moveBy.y);

	pos.x = pos.x + moveBy.x;
	pos.y = pos.y + moveBy.y;
}

void Papas::ReceiptParts::setPosReceipt(glm::vec2 moveBy)
{
	C2D_SpriteSetPos(&receipt_bg, moveBy.x, moveBy.y);

	pos.x = moveBy.x;
	pos.y = moveBy.y;
}

void Papas::ReceiptParts::scaleReceipt(glm::vec2 scaleBy)
{
	C2D_SpriteScale(&receipt_bg, scaleBy.x, scaleBy.y);
	scale.x = scale.x * scaleBy.x;
	scale.y = scale.y * scaleBy.y;
}

void Papas::ReceiptParts::setScaleReceipt(glm::vec2 scaleBy)
{
	C2D_SpriteSetScale(&receipt_bg, scaleBy.x, scaleBy.y);
	scale.x = scaleBy.x;
	scale.y = scaleBy.y;
}

void Papas::ReceiptParts::renderReceipt()
{
	C2D_SpriteSetDepth(&receipt_bg, currentDepth);
	C2D_DrawSprite(&receipt_bg);

	hitBox = {receipt_bg.params.pos.x, receipt_bg.params.pos.y, receipt_bg.params.pos.w, receipt_bg.params.pos.h};
	// Calculate the new position for the text based on scale
	float scaledTextX = pos.x + (39 * scale.x);
	float scaledTextY = pos.y + (19 * scale.y);

	float textDepth = std::min(currentDepth + 0.01f, 1.0f);
	// Draw the text at the scaled position
	C2D_DrawText(&receipt_text, C2D_WithColor, scaledTextX, scaledTextY, textDepth, scale.x, scale.y, C2D_Color32(240, 156, 156, 255));

	//For the sections:
	for (size_t i = 0; i < currentItems; i++)
	{
		float PosY = pos.y + (i * 20 * scale.y) + (49 * scale.y);

		float CrossPosX = pos.x + (59 * scale.x);
		C2D_DrawImageAt(s_nums_cross, CrossPosX, PosY, textDepth, nullptr, scale.x * 0.8f, scale.y * 0.8f);

		float QuantPosX = pos.x + (83 * scale.x);
		C2D_DrawImageAt(sections[i].i_Quantity, QuantPosX, PosY, textDepth, nullptr, scale.x * 0.8f, scale.y * 0.8f);

		float ToppPosX = pos.x + (37 * scale.x);
		float ToppPosY = pos.y + (i * 20 * scale.y) + (48 * scale.y);
		C2D_DrawImageAt(sections[i].i_topping, ToppPosX, ToppPosY, textDepth, nullptr, scale.x * 0.45f, scale.y * 0.45f);

		float CoveragePosX = pos.x + (10 * scale.x);
		for (size_t z = 0; z < 4; z++)
		{
			if(sections[i].cover[z] == 1){
				C2D_DrawImageAt(sections[i].i_cover[z], CoveragePosX, PosY, textDepth+0.01f, nullptr, scale.x * 0.15f, scale.y * 0.15f);
			}
		}
		
		
	}
	// Calculate the needle
	if (needleDrawn)
	{
		float NeedlePosY = pos.y + (203.4f * scale.y);

		float NeedlePosX = pos.x + (31.8f * scale.x);
		C2D_SpriteSetDepth(&needle, textDepth);
		C2D_SpriteSetPos(&needle, NeedlePosX, NeedlePosY);
		C2D_SpriteSetScale(&needle, 0.75f * scale.x, 0.75f * scale.y);
		C2D_DrawSprite(&needle);
	}

	if(selectedCut != -1){
		float SlicesPosY = pos.y + (189 * scale.y);
		float SlicesPosX = pos.x + (75 * scale.x);
		C2D_DrawImageAt(s_cuts[selectedCut], SlicesPosX, SlicesPosY, textDepth, nullptr, scale.x * 0.5f, scale.y * 0.5f);
	}
	
}

void Papas::Receipt::init(int receiptNum, C2D_Font *font, C2D_SpriteSheet &receipt_spriteSheet)
{

	// int to 000 whatever number
	std::ostringstream formatted;
	formatted << std::setw(3) << std::setfill('0') << receiptNum;

	top.init(formatted.str().c_str(), font, receipt_spriteSheet, snapPosTop, bigScaleTop, true);
	bottom.init(formatted.str().c_str(), font, receipt_spriteSheet, snapPosBottom, bigScaleBottom, false);
	dockInUse = true;
	bottom.pinnedTop = false;
}

void Papas::Receipt::showReceipt(bool topReceipt)
{
	if(topReceipt){
		top.renderReceipt();
	}
	else{
		bottom.renderReceipt();
	}
	
}

bool Papas::Receipt::detectTouch(touchPosition &touch)
{

	if (touch.px != 0 || touch.py != 0) // I wanna account for latency, if the receipt is moving but you move outside the hitbox faster than the hitbox can move, it still drags
	{
		if (
			(touch.px > bottom.hitBox.left && touch.px < bottom.hitBox.left + bottom.hitBox.width &&
			 touch.py > bottom.hitBox.top && touch.py < bottom.hitBox.top + bottom.hitBox.height))
		{

			return true;
		}
	}
	

	return false;
}

void Papas::Receipt::detectMovement(touchPosition &touch)
{

	//bottom.hitBox = {bottom.receipt_bg.params.pos.x, bottom.receipt_bg.params.pos.y, bottom.receipt_bg.params.pos.w, bottom.receipt_bg.params.pos.h};

	static bool isDragging = false;
	static touchPosition prevTouch;
	static glm::vec2 offset;

	if (touch.px != 0 || touch.py != 0) // I wanna account for latency, if the receipt is moving but you move outside the hitbox faster than the hitbox can move, it still drags
	{
		if (isDragging ||
			(touch.px > bottom.hitBox.left && touch.px < bottom.hitBox.left + bottom.hitBox.width &&
			 touch.py > bottom.hitBox.top && touch.py < bottom.hitBox.top + bottom.hitBox.height))
		{

			if (!isDragging)
			{
				// Set the initial position and scale as soon as you start grabbing it, to prevent a "jolt" or "jump" (idk but this fixes that)
				bottom.setScaleReceipt(smallScaleBottom);

				if (bottom.pinnedTop)
				{
					bottom.setPosReceipt(glm::vec2(touch.px - (bottom.hitBox.width * 0.5f), touch.py - (bottom.hitBox.height * 0.3f)));
				}
				else
				{
					bottom.setPosReceipt(glm::vec2(touch.px - bottom.hitBox.width * 0.2f, touch.py - bottom.hitBox.height * 0.1f));
				}

				isDragging = true;
			}
			else
			{
				bottom.moveReceipt(glm::vec2(touch.px - prevTouch.px, touch.py - prevTouch.py));
			}

			prevTouch = touch;
		}
	}
	else if (isDragging)
	{
		if (bottom.pos.y < 30.0f)
		{
			dockInUse = false;
			bottom.setPosReceipt(glm::vec2(bottom.pos.x, 5));
			bottom.pinnedTop = true;
			top.setScaleReceipt(smallScaleTop);
			top.setPosReceipt(glm::vec2(bottom.pos.x, -7.0f));
		}
		else
		{
			dockInUse = true;
			bottom.setPosReceipt(snapPosBottom);
			bottom.setScaleReceipt(bigScaleBottom);
			bottom.pinnedTop = false;
			top.setPosReceipt(snapPosTop);
			top.setScaleReceipt(bigScaleTop);
		}

		isDragging = false;
		
	}
}

void Papas::Receipt::forceDocking()
{
	std::srand(osGetTime());
	int randomPos = std::rand() % 20 + 1;
	bottom.setScaleReceipt(smallScaleBottom);
	bottom.setPosReceipt(glm::vec2(randomPos, 5));
	bottom.pinnedTop = false;
	top.setScaleReceipt(smallScaleTop);
	top.setPosReceipt(glm::vec2(randomPos, -7.0f));
	dockInUse = false;
}

void Papas::Receipt::addItem(int size[4], Toppings topping, int Quant)
{
	top.addItem(size, topping, Quant);
	bottom.addItem(size, topping, Quant);
}

void Papas::Receipt::addTime(int time)
{
	top.addTime(time);
	bottom.addTime(time);
}
void Papas::Receipt::addCut(int slices)
{
	top.addCut(slices);
	bottom.addCut(slices);
}

void Papas::Receipt::terminate()
{
	C2D_TextBufDelete(bottom.receipt_Buf);
	C2D_TextBufDelete(top.receipt_Buf);
}

Papas::ReceiptManager::ReceiptManager(C2D_Font *font)
{
	initManager(font);
}

void Papas::ReceiptManager::initManager(C2D_Font *font)
{
	dokyo = font;
	receipt_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/receipt.t3x");
	maxReceipts = 1;
	activeReceiptIndex = -1;
	currentMaxDepth = 0.1f;
	
}

void Papas::ReceiptManager::createReceipt()
{

	for (size_t i = v_receipts.size(); i-- > 0;)
	{
		if (v_receipts[i]->dockInUse){
			v_receipts[i]->forceDocking();
		}
	}
	Receipt* temp = new Receipt();
	temp->init(maxReceipts, dokyo, receipt_spriteSheet);
	currentMaxDepth += 0.1f;
	temp->bottom.currentDepth = normalizedDepth();
	temp->top.currentDepth = normalizedDepth();
	v_receipts.push_back(temp);
	maxReceipts++;
}

void Papas::ReceiptManager::renderReceipt(bool topReceipt)
{
	for (size_t i = 0; i < v_receipts.size(); i++)
	{
		v_receipts[i]->showReceipt(topReceipt);
	}
}

void Papas::ReceiptManager::renderDockedReceipt(bool topReceipt)
{
	for (size_t i = 0; i < v_receipts.size(); i++)
	{
		if (v_receipts[i]->dockInUse){
			v_receipts[i]->showReceipt(topReceipt);
		}
			
	}
}

float Papas::ReceiptManager::normalizedDepth()
{
	const float maxDepth = 0.95f; // Closest to the camera
	const float minDepth = 0.1f; // Farthest from the camera
	return (currentMaxDepth - minDepth) / (maxDepth / minDepth);
}

void Papas::ReceiptManager::moveReceiptToBack(int indexToMove)
{
	Receipt* tempReceipt = v_receipts[indexToMove];
	v_receipts.erase(v_receipts.begin() + indexToMove);
	v_receipts.push_back(tempReceipt);
}

void Papas::ReceiptManager::detectMovement(touchPosition &touch)
{
	/*There is a bug where the transparency of a receipt lower in the array isnt processed on top of the transparency of a receipt higher in the array
	i'm 96% sure its because of the rendering order, since technically the one lower in the array is processed first
	to fix this i have to reorder the array in terms of depth, which i can't be fucked to do right now, but will do later*/

	/*UPDATE: I kinda fixed it, but it's still a bit odd when you move the receipt lol*/
	if (touch.px != 0 || touch.py != 0) // Check if the screen is touched
	{
		if (activeReceiptIndex == -1) // No receipt is being dragged
		{

			// Basically, we need to figure out what the best receipt to grab is, depending on how deep it is
			//  so these guys lmk which the best index is by going through the loop
			float maxDepth = -1.0f;
			float bestReceiptIndex = -1;

			// Find the first receipt that is touched
			for (size_t i = v_receipts.size(); i-- > 0;)
			{
				
				if (touch.px > v_receipts[i]->bottom.hitBox.left &&
					touch.px < v_receipts[i]->bottom.hitBox.left + v_receipts[i]->bottom.hitBox.width &&
					touch.py > v_receipts[i]->bottom.hitBox.top &&
					touch.py < v_receipts[i]->bottom.hitBox.top + v_receipts[i]->bottom.hitBox.height)
				{
					if (v_receipts[i]->bottom.currentDepth > maxDepth)
					{
						maxDepth = v_receipts[i]->bottom.currentDepth;
						bestReceiptIndex = i;
					}
				}
			}

			if (bestReceiptIndex != -1)
			{
				activeReceiptIndex = bestReceiptIndex;

				// add the depth and allow it to moove
				// this might fuck up later, if you move the receipts too much, pain in my ass
				currentMaxDepth += 0.1f;
				v_receipts[activeReceiptIndex]->bottom.currentDepth = normalizedDepth();
				v_receipts[activeReceiptIndex]->top.currentDepth = normalizedDepth();
			}
		}

		if (activeReceiptIndex != -1)
		{
			v_receipts[activeReceiptIndex]->detectMovement(touch);
		}
	}
	else
	{
		if (activeReceiptIndex != -1)
		{
			//Once the receipt's been let go
			v_receipts[activeReceiptIndex]->detectMovement(touch);
			if (v_receipts[activeReceiptIndex]->dockInUse){
				for (size_t j = 0; j < v_receipts.size(); j++)
				{
					if (v_receipts[j]->dockInUse && v_receipts[j] != v_receipts[activeReceiptIndex])
					{
						v_receipts[j]->forceDocking();
					}
				}
				
			}
			moveReceiptToBack(activeReceiptIndex);
			activeReceiptIndex = -1;
			
		}
	}
}

void Papas::ReceiptManager::getDockedReceipt(Receipt** returnReceipt)
{
	for (size_t j = 0; j < v_receipts.size(); j++)
	{
		if (v_receipts[j]->dockInUse)
		{
			*returnReceipt = v_receipts[j];
		}
	}
}

void Papas::ReceiptManager::terminateManager()
{
	C2D_SpriteSheetFree(receipt_spriteSheet);
	for (size_t j = 0; j < v_receipts.size(); j++)
	{
		v_receipts[j]->terminate();
		delete v_receipts[j];
		v_receipts[j] = nullptr;
	}
	v_receipts.clear();
}