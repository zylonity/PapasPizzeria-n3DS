#include "Papas_Utils.h"
#include "Papas_ResourceManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdlib> // for rand() and srand()
#include <vector>

// Pause bookkeeping: when we froze, and how much time we've swallowed so far
static u64 clockPausedAt = 0;
static u64 clockLostMs = 0;

u64 Papas::Clock::now()
{
	if (clockPausedAt != 0)
		return clockPausedAt - clockLostMs;
	return osGetTime() - clockLostMs;
}

void Papas::Clock::pause()
{
	if (clockPausedAt == 0)
		clockPausedAt = osGetTime();
}

void Papas::Clock::resume()
{
	if (clockPausedAt != 0)
	{
		clockLostMs += osGetTime() - clockPausedAt;
		clockPausedAt = 0;
	}
}

bool Papas::Clock::isPaused()
{
	return clockPausedAt != 0;
}

Papas::Button::Button(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, v2 position) {

	createButton(spriteSheet, unpressed, selected, pressed, position);
}

void Papas::Button::createButton(C2D_SpriteSheet& spriteSheet, int unpressed, int selected, int pressed, v2 position) {

	wasPressed = false;
	lastTouch = {0, 0};
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

	// Check whether the touch sits inside the button.
	if (touch.px == 0.0f && touch.py == 0.0f && lastTouch.px > hitBox.left && lastTouch.px < hitBox.left + hitBox.width && lastTouch.py > hitBox.top && lastTouch.py < hitBox.top + hitBox.height)
	{
		C2D_DrawImageAt(img_Pressed, pos.x, pos.y, 1, NULL, 1, 1);

		if (!wasPressed)
		{
			wasPressed = true;
			lastTouch = {0, 0}; // consume the release so the press only fires once
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
	start = Papas::Clock::now();
	currentSprite = 0;
}

// Advance a frame whenever animTime ms have passed, then draw
void Papas::AnimatedSprite::renderAnim(bool loop)
{

	end = Papas::Clock::now();

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

		start = Papas::Clock::now();
	}

	if (!finished)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
}

// Draw where the animation currently sits without stepping it on
void Papas::AnimatedSprite::renderCurrentFrame()
{
	if (!finished && !each_sprite.empty())
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
	end = Papas::Clock::now();

	if (end - start >= animTime)
	{
		if(ranOnce == false){
			if (currentSprite < numOfSprites - 1)
			{
				currentSprite++;
			}
			else if (currentSprite == numOfSprites - 1) 
			{
				// Slow the trimmed ending; C2D_Sprite already points at the texture.
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
		

		start = Papas::Clock::now();
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


	end = Papas::Clock::now();

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

		start = Papas::Clock::now();
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

// Loop the scribble with pauses and report when a new order line starts.
bool Papas::RoyTakingOrder::renderAnimWithPauses(int pauses, float pauseTime)
{
	end = Papas::Clock::now();
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

		start = Papas::Clock::now();
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
			start = Papas::Clock::now();
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

// Grab all the receipt art out of the sheet and set up the blank ticket
void Papas::ReceiptParts::init(const char *receiptNum, C2D_Font *font, C2D_SpriteSheet &receipt_spriteSheet, v2 posToGive, v2 scaleToGive, bool top)
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
	// Put the cross at zero so every number matches its own index.
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

// Print a topping line onto the receipt
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

// Requested bake time, drawn as a needle on the little dial (45 deg/notch)
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

void Papas::ReceiptParts::moveReceipt(v2 moveBy)
{
	C2D_SpriteMove(&receipt_bg, moveBy.x, moveBy.y);

	pos.x = pos.x + moveBy.x;
	pos.y = pos.y + moveBy.y;
}

void Papas::ReceiptParts::setPosReceipt(v2 moveBy)
{
	C2D_SpriteSetPos(&receipt_bg, moveBy.x, moveBy.y);

	pos.x = moveBy.x;
	pos.y = moveBy.y;
}

void Papas::ReceiptParts::scaleReceipt(v2 scaleBy)
{
	C2D_SpriteScale(&receipt_bg, scaleBy.x, scaleBy.y);
	scale.x = scale.x * scaleBy.x;
	scale.y = scale.y * scaleBy.y;
}

void Papas::ReceiptParts::setScaleReceipt(v2 scaleBy)
{
	C2D_SpriteSetScale(&receipt_bg, scaleBy.x, scaleBy.y);
	scale.x = scaleBy.x;
	scale.y = scaleBy.y;
}

// Background, ticket number, every order line, then the dial and slices
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

	// Keep dragging if the touch outruns the moving hitbox.
	if (touch.px != 0 || touch.py != 0)
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

	static bool isDragging = false;
	static touchPosition prevTouch;
	static v2 offset;

	// Keep dragging if the touch outruns the moving hitbox.
	if (touch.px != 0 || touch.py != 0)
	{
		if (isDragging ||
			(touch.px > bottom.hitBox.left && touch.px < bottom.hitBox.left + bottom.hitBox.width &&
			 touch.py > bottom.hitBox.top && touch.py < bottom.hitBox.top + bottom.hitBox.height))
		{

			if (!isDragging)
			{
				// Set the grabbed state right away so the receipt doesn't jump.
				Papas::ResourceManager::getInstance().playSfx("grabticket");
				bottom.setScaleReceipt(smallScaleBottom);

				if (bottom.pinnedTop)
				{
					bottom.setPosReceipt(v2(touch.px - (bottom.hitBox.width * 0.5f), touch.py - (bottom.hitBox.height * 0.3f)));
				}
				else
				{
					bottom.setPosReceipt(v2(touch.px - bottom.hitBox.width * 0.2f, touch.py - bottom.hitBox.height * 0.1f));
				}

				isDragging = true;
			}
			else
			{
				bottom.moveReceipt(v2(touch.px - prevTouch.px, touch.py - prevTouch.py));
			}

			prevTouch = touch;
		}
	}
	else if (isDragging)
	{
		Papas::ResourceManager::getInstance().playSfx("dropticket");
		if (bottom.pos.y < 30.0f)
		{
			dockInUse = false;
			bottom.setPosReceipt(v2(bottom.pos.x, 5));
			bottom.pinnedTop = true;
			top.setScaleReceipt(smallScaleTop);
			top.setPosReceipt(v2(bottom.pos.x, -7.0f));
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

// Kick a receipt out of the dock and pin it up top at a random spot
void Papas::Receipt::forceDocking()
{
	std::srand(osGetTime());
	int randomPos = std::rand() % 20 + 1;
	bottom.setScaleReceipt(smallScaleBottom);
	bottom.setPosReceipt(v2(randomPos, 5));
	bottom.pinnedTop = false;
	top.setScaleReceipt(smallScaleTop);
	top.setPosReceipt(v2(randomPos, -7.0f));
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
	// New ticket takes the dock, whoever was there gets pinned up top
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

// Only the docked ticket, for screens where the pinned ones would be in the way
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
	// Kinda fixed receipt overlap; proper depth sorting can wait cos I can't be fucked lol.
	if (touch.px != 0 || touch.py != 0) // Check if the screen is touched
	{
		if (activeReceiptIndex == -1) // No receipt is being dragged
		{

			// Grab the frontmost matching receipt.
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

				// Bring it forward; too much movement might fuck with depth later, pain in my ass.
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

// The docked ticket is the active order everything else works from
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

void Papas::ReceiptManager::removeReceipt(Receipt* receipt)
{
	for (size_t i = 0; i < v_receipts.size(); i++)
	{
		if (v_receipts[i] == receipt)
		{
			v_receipts[i]->terminate();
			delete v_receipts[i];
			v_receipts.erase(v_receipts.begin() + i);
			activeReceiptIndex = -1;
			return;
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

// The help book, adapted from the original's 13 board pages to our controls.
namespace {
	struct HelpPage
	{
		const char *title;
		const char *lines[7];
	};

	const HelpPage HELP_PAGES[] = {
		{"Welcome to Papa's!", {
			"Roy has left you in charge of the",
			"pizzeria while he's away.",
			"Take orders, top the pizzas, bake",
			"them and slice them up.",
			"The happier they are, the fatter",
			"your tips and the faster you rank up.",
			nullptr}},
		{"The Stations", {
			"L and R shuffle between the four",
			"stations at the bottom of the screen.",
			"Orders, Topping, Baking, Cutting.",
			"Ovens keep cooking while you're",
			"somewhere else, so keep an eye on",
			"the timers.",
			nullptr}},
		{"Taking Orders", {
			"Wait for a customer to reach the",
			"counter at the Orders station.",
			"Tap TAKE ORDER and Roy writes down",
			"whatever they rattle off.",
			"The balloon over their head shows",
			"each line as it lands on the ticket.",
			nullptr}},
		{"Reading Tickets", {
			"Each row is one topping: the icon,",
			"how many, and where they go.",
			"The pie chart marks which quarters",
			"of the pizza get covered.",
			"The clock is the bake notch, and the",
			"knife shows how many slices.",
			nullptr}},
		{"Moving Tickets", {
			"The ticket sitting in the dock is the",
			"order every station works from.",
			"Drag it up to the rail to park it, and",
			"drag another one down to swap.",
			"Only the docked ticket can make a",
			"pizza, so pick before you top.",
			nullptr}},
		{"Topping", {
			"Drag toppings out of the cups onto",
			"the pizza. Watch the quarters!",
			"Drag one back to its cup to bin it,",
			"or drag it around to nudge it.",
			"TO OVEN sends it off to bake, SAVE",
			"parks it so you can top another.",
			nullptr}},
		{"Baking", {
			"Four ovens, four timers. The needle",
			"sweeps round as the pizza cooks.",
			"Match the notch the customer asked",
			"for on the ticket's clock.",
			"Tap a pizza to pull it out. Overdone",
			"is just as bad as underdone.",
			nullptr}},
		{"Cutting", {
			"Drag a line clean across the pizza to",
			"slice it. Short cuts don't count.",
			"4 slices needs 2 cuts, 6 needs 3,",
			"and 8 needs 4, spaced evenly.",
			"THROW AWAY bins a ruined one so you",
			"can start that order over.",
			nullptr}},
		{"Serving Up", {
			"SERVE hands the pizza over and Roy",
			"scores you on four things:",
			"waiting, toppings, baking, cutting.",
			"The average decides their reaction",
			"and the tip that follows.",
			"Keep them quick and they tip better.",
			nullptr}},
		{"Stars and Seals", {
			"Score 80% or more and that customer",
			"gives you a star. Five makes a seal.",
			"Drop under 60% and you lose the lot,",
			"so don't get sloppy.",
			"Every seal bumps their maximum tip.",
			"Three on everyone unlocks Papa.",
			nullptr}},
	};

	const int HELP_PAGE_COUNT = (int)(sizeof(HELP_PAGES) / sizeof(HELP_PAGES[0]));
}

int Papas::HelpBook::count()
{
	return HELP_PAGE_COUNT;
}

// Rows are drawn from y=16 in 19px steps, so work backwards from the touch
int Papas::HelpBook::rowAt(const touchPosition &touch)
{
	if (touch.px < 14 || touch.px > 306 || touch.py < 16) return -1;
	int row = (touch.py - 16) / 19;
	return row >= 0 && row < HELP_PAGE_COUNT && row < 12 ? row : -1;
}

void Papas::HelpBook::init(C2D_Font *font)
{
	dokyo = font;
	buf = C2D_TextBufNew(2048);
	// The contents list never changes, but it shares the page buffer, so
	// setPage rebuilds the lot each time it's called.
	setPage(0);
}

void Papas::HelpBook::terminate()
{
	if (buf != nullptr)
	{
		C2D_TextBufDelete(buf);
		buf = nullptr;
	}
}

void Papas::HelpBook::setPage(int newPage)
{
	page = ((newPage % HELP_PAGE_COUNT) + HELP_PAGE_COUNT) % HELP_PAGE_COUNT;

	C2D_TextBufClear(buf);
	const HelpPage &current = HELP_PAGES[page];

	C2D_TextFontParse(&title, *dokyo, buf, current.title);
	C2D_TextOptimize(&title);

	bodyLines = 0;
	for (int i = 0; i < 7 && current.lines[i] != nullptr; i++)
	{
		C2D_TextFontParse(&body[bodyLines], *dokyo, buf, current.lines[i]);
		C2D_TextOptimize(&body[bodyLines]);
		bodyLines++;
	}

	for (int i = 0; i < HELP_PAGE_COUNT && i < 12; i++)
	{
		C2D_TextFontParse(&contents[i], *dokyo, buf, HELP_PAGES[i].title);
		C2D_TextOptimize(&contents[i]);
	}

	C2D_TextFontParse(&hint, *dokyo, buf, "L/R or D-Pad turns pages    B Back");
	C2D_TextOptimize(&hint);
}

void Papas::HelpBook::turnPage(int by)
{
	setPage(page + by);
}

void Papas::HelpBook::renderTop()
{
	const u32 colBoard = C2D_Color32(35, 42, 37, 245);
	const u32 colPaper = C2D_Color32(244, 239, 218, 255);
	const u32 colTitle = C2D_Color32(170, 63, 24, 255);
	const u32 colInk = C2D_Color32(39, 42, 35, 255);

	C2D_DrawRectSolid(14.0f, 10.0f, 0.90f, 372.0f, 220.0f, colBoard);
	C2D_DrawRectSolid(19.0f, 15.0f, 0.91f, 362.0f, 210.0f, colPaper);

	float width = 0.0f;
	C2D_TextGetDimensions(&title, 0.7f, 0.7f, &width, nullptr);
	C2D_DrawText(&title, C2D_WithColor, (SCREEN_WIDTH_TOP - width) * 0.5f, 22.0f, 0.94f, 0.7f, 0.7f, colTitle);

	for (int i = 0; i < bodyLines; i++)
		C2D_DrawText(&body[i], C2D_WithColor, 34.0f, 58.0f + i * 26.0f, 0.94f, 0.5f, 0.5f, colInk);
}

void Papas::HelpBook::renderBottom()
{
	const u32 colPaper = C2D_Color32(244, 239, 218, 255);
	const u32 colInk = C2D_Color32(39, 42, 35, 255);
	const u32 colSoft = C2D_Color32(120, 105, 90, 255);
	const u32 colPick = C2D_Color32(236, 121, 42, 255);

	C2D_DrawRectSolid(10.0f, 6.0f, 0.90f, 300.0f, 206.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(14.0f, 10.0f, 0.91f, 292.0f, 198.0f, colPaper);

	for (int i = 0; i < HELP_PAGE_COUNT && i < 12; i++)
		C2D_DrawText(&contents[i], C2D_WithColor, 26.0f, 16.0f + i * 19.0f, 0.94f, 0.44f, 0.44f,
			i == page ? colPick : colInk);

	float width = 0.0f;
	C2D_TextGetDimensions(&hint, 0.42f, 0.42f, &width, nullptr);
	C2D_DrawText(&hint, C2D_WithColor, (SCREEN_WIDTH_BOTTOM - width) * 0.5f, 218.0f, 0.94f, 0.42f, 0.42f, colSoft);
}
