#include "Papas_Scenes.h"
#include "Papas_SceneManager.h"
#include "Papas_ResourceManager.h"
#include <string>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <citro2d.h>
#include <citro3d.h>
#include <theoraplayer.h>
#include "Papas_Customers.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

PapasError Papas::MainMenu::init(Papas::SceneManager *sceneManager)
{

	Papas::ResourceManager::getInstance().initMusicPlayer();
	Papas::ResourceManager::getInstance().loadSong("topping_music", "romfs:/music/toppingscreen_music.ogg");
	Papas::ResourceManager::getInstance().loadSong("baking_music", "romfs:/music/bakingscreen_music.ogg");
	Papas::ResourceManager::getInstance().loadSong("cutting_music", "romfs:/music/cuttingscreen_music.ogg");
	Papas::ResourceManager::getInstance().loadSong("orders_music", "romfs:/music/orderscreen_music.ogg");
	Papas::ResourceManager::getInstance().playMusic("topping_music");


	// Load the backgrounds
	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);
	bottom_bg = C2D_SpriteSheetGetImage(sheet_bg, 0);
	logo = C2D_SpriteSheetGetImage(sheet_bg, 2);

	// Load the buttons
	sheet_buttons = C2D_SpriteSheetLoad("romfs:/gfx/buttons.t3x");

	// Start button
	Papas::Button b_start(sheet_buttons, 0, 1, 2);
	Papas::v2 startPos = {(SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2), 50.0f};
	b_start.setPosition(startPos);
	v_buttons.push_back(b_start);

	float buttonheight = b_start.getRect().height;
	float padding = 10;

	Papas::Button b_help(sheet_buttons, 3, 4, 5);
	Papas::v2 helpPos = {(SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2), startPos.y + buttonheight + padding};
	b_help.setPosition(helpPos);
	v_buttons.push_back(b_help);

	Papas::Button b_credits(sheet_buttons, 6, 7, 8);
	Papas::v2 creditsPos = {(SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2), helpPos.y + buttonheight + padding};
	b_credits.setPosition(creditsPos);
	v_buttons.push_back(b_credits);


	buttonIndex = 0;
	aPressed = false;

	p_sceneManager = sceneManager;



	return PAPAS_OK;
}

PapasError Papas::MainMenu::update()
{

	hidScanInput();
	hidTouchRead(&touch);

	// Respond to user input
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu

	// Browse through menu with DPAD
	if (kDown & KEY_DDOWN)
	{
		if (buttonIndex < v_buttons.size())
		{
			buttonIndex++;
		}

		if (buttonIndex == v_buttons.size())
		{
			buttonIndex = 0;
		}
	}

	if (kDown & KEY_DUP)
	{
		if (buttonIndex >= 0)
		{
			buttonIndex--;
		}

		if (buttonIndex < 0)
		{
			buttonIndex = v_buttons.size() - 1;
		}
	}

	// Detect when A's pressed to pass it through to the on screen button
	if (kDown & KEY_A)
	{
		aPressed = true;
	}

	return PAPAS_OK;
}

PapasError Papas::MainMenu::render_top()
{

	// Draw the top background
	C2D_DrawImageAt(top_bg, 0, 0, 0, NULL, 1, 1);

	// Draw the logo
	float scaling = 0.7f;
	float xmiddle = (SCREEN_WIDTH_TOP / 2) - ((logo.subtex->width * scaling) / 2);
	float ymiddle = (SCREEN_HEIGHT_TOP / 2) - ((logo.subtex->height * scaling) / 2);
	C2D_DrawImageAt(logo, xmiddle, ymiddle, 0, NULL, scaling, scaling);

	return PAPAS_OK;
}

PapasError Papas::MainMenu::render_bottom()
{

	// Draw the background
	C2D_DrawImageAt(bottom_bg, 0, 0, 0, NULL, 1, 1);

	for (size_t i = 0; i < v_buttons.size(); i++)
	{
		// Detect when button is pressed either through keys or touchscreen
		if (v_buttons[i].showButton(touch, i == buttonIndex, &aPressed))
		{
			if(i == 0){
				p_sceneManager->changeScene(new Papas::IntroVid()); //skip intro video temporarily
				ResourceManager::getInstance().stopMusic();
			}

		}
	}

	return PAPAS_OK;
}

PapasError Papas::MainMenu::terminate()
{

	if (sheet_bg)
	{
		C2D_SpriteSheetFree(sheet_bg);
		sheet_bg = nullptr;
	}
	if (sheet_buttons)
	{
		C2D_SpriteSheetFree(sheet_buttons);
		sheet_buttons = nullptr;
	}


	return PAPAS_OK;
}

PapasError Papas::IntroVid::init(Papas::SceneManager *sceneManager)
{

	p_sceneManager = sceneManager;
	startedPlaying = false;
	Papas::ResourceManager::getInstance().endMusicPlayer();

	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	skip_bg = C2D_SpriteSheetGetImage(sheet_bg, 3);

	ndspInit();

	ndspSetCallback(TP_audioCallback, NULL);

	return PAPAS_OK;
}

PapasError Papas::IntroVid::update()
{

	hidScanInput();

	// Respond to user input
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu

	if (!THEORA_isplaying && startedPlaying == false)
	{
		TP_changeFile("romfs:/videos/ready.ogv");
	}
	else{
		startedPlaying = true;
	}

	if (kDown & KEY_B && THEORA_isplaying && startedPlaying)
	{
		TP_exitThread(); //finishes playing the video (skips)
	}

	return PAPAS_OK;
}

PapasError Papas::IntroVid::render_top()
{

	if (THEORA_isplaying && THEORA_HasVideo(&THEORA_vidCtx)){
		frameDrawAtCentered(&THEORA_frame, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.5f, THEORA_scaleframe, THEORA_scaleframe);
	}

	if (!THEORA_isplaying && startedPlaying)
	{
		p_sceneManager->changeScene(new Papas::Game());
	}



	return PAPAS_OK;
}

PapasError Papas::IntroVid::render_bottom()
{
	C2D_DrawImageAt(skip_bg, 0, 0, 0, NULL, 1, 1);

	return PAPAS_OK;
}

PapasError Papas::IntroVid::terminate()
{
	TP_exitThread();
	ndspExit();

	if (sheet_bg)
	{
		C2D_SpriteSheetFree(sheet_bg);
		sheet_bg = nullptr;
	}


	return PAPAS_OK;
}

PapasError Papas::Game::init(Papas::SceneManager *sceneManager)
{


	Papas::ResourceManager::getInstance().initMusicPlayer();
	Papas::ResourceManager::getInstance().loadSfx("writepencil", "romfs:/sfx/writepencil.wav");
	
	takingOrder = false;
	s_stations = C2D_SpriteSheetLoad("romfs:/gfx/stations.t3x");

	ticketsStationImg = C2D_SpriteSheetGetImage(s_stations, 8);
	ticketsHolderImg = C2D_SpriteSheetGetImage(s_stations, 9);

	
	orderStation = C2D_SpriteSheetLoad("romfs:/gfx/taking_order.t3x");
	to_counter = C2D_SpriteSheetGetImage(orderStation, 0);
	to_wallpaper = C2D_SpriteSheetGetImage(orderStation, 1);

	v2 pos = {-24, 45};
	v2 scl = {0.8f, 0.8f};
	Roy.createAnim("romfs:/gfx/Roy_peeking.t3x", 0.0f, pos, scl, 35.0f);

	v2 pos_order = {-45, 45};
	v2 scl_order = {1.0f, 1.0f};
	Roy2.createAnim("romfs:/gfx/Roy_takingorder.t3x", 0.0f, pos_order, scl_order, 0.0f);

	// Receipt system stuff to move later
	dokyo = C2D_FontLoad("romfs:/fonts/Dokyo.bcfnt");
	resultTextBuf = C2D_TextBufNew(256);
	showingResult = false;
	resultTouchHeld = false;
	totalScore = 0;
	totalTipsCents = 0;
	resultCustomerNumber = 0;

	r_manager.initManager(&dokyo);

	createReceipt.createButton(orderStation, 2, 2, 3, {34, 141});

	SwitchStation(TicketStation);

	to_firstRun = false;
	to_currentAction = 0;

	// Spawn today's customers (loads the rig + per-type atlases on demand)
	c_manager.initManager();

	// Pizza making at the topping station
	pz_manager.initManager();

	return PAPAS_OK;
}

// The receipt sitting in the dock is the "active ticket" the stations work from
Papas::Receipt *Papas::Game::dockedReceipt()
{
	Receipt *docked = nullptr;
	r_manager.getDockedReceipt(&docked);
	return docked;
}

int Papas::Game::scoreWaiting(const Pizza &pizza, const CustomerData &order) const
{
	if (pizza.ticket == nullptr || pizza.ticket->orderStartedAt == 0) return 100;
	float elapsedMs = (float)(osGetTime() - pizza.ticket->orderStartedAt);
	float expectedMs = order.time * 45.0f * 500.0f + 30000.0f;
	float lateSeconds = std::max(0.0f, elapsedMs - expectedMs) / 1000.0f;
	return std::max(0, (int)std::floor(100.0f - lateSeconds * 0.3f));
}

int Papas::Game::scoreBaking(const Pizza &pizza, const CustomerData &order) const
{
	float difference = std::abs(order.time * 45.0f - pizza.cookDegrees);
	return std::max(0, (int)std::floor(100.0f - difference / 90.0f * 100.0f));
}

int Papas::Game::scoreToppings(const Pizza &pizza, const CustomerData &order) const
{
	int targetQuantity[7] = {};
	int targetCoverage[7][4] = {};
	for (size_t i = 0; i < order.items.size(); i++)
	{
		int type = order.items[i].Topping;
		targetQuantity[type] += order.items[i].Quantity;
		for (int q = 0; q < 4; q++)
			targetCoverage[type][q] = std::max(targetCoverage[type][q], order.items[i].Coverage[q]);
	}

	int actual[7][4] = {};
	int unwanted = 0;
	for (size_t i = 0; i < pizza.toppings.size(); i++)
	{
		const PlacedTopping &placed = pizza.toppings[i];
		int type = placed.type;
		if (targetQuantity[type] == 0)
		{
			unwanted++;
			continue;
		}
		int quadrant;
		if (placed.pos.x > 0.0f && placed.pos.y < 0.0f) quadrant = 0;
		else if (placed.pos.x > 0.0f && placed.pos.y >= 0.0f) quadrant = 1;
		else if (placed.pos.x <= 0.0f && placed.pos.y >= 0.0f) quadrant = 2;
		else quadrant = 3;
		actual[type][quadrant]++;
	}

	float scoreTotal = 0.0f;
	int scoredTypes = 0;
	for (int type = 0; type < 7; type++)
	{
		int target = targetQuantity[type];
		if (target == 0) continue;
		int current = actual[type][0] + actual[type][1] + actual[type][2] + actual[type][3];
		int coverageCount = targetCoverage[type][0] + targetCoverage[type][1] + targetCoverage[type][2] + targetCoverage[type][3];
		float unitPenalty = 100.0f / std::max(target, current);
		float typeScore = 100.0f - std::abs(target - current) * unitPenalty;
		float expectedPerQuadrant = coverageCount > 0 ? (float)target / coverageCount : 0.0f;
		for (int q = 0; q < 4; q++)
		{
			if (!targetCoverage[type][q])
				typeScore -= actual[type][q] * unitPenalty * 0.5f;
			else
				typeScore -= std::abs(actual[type][q] - expectedPerQuadrant) * unitPenalty * 0.25f;
		}
		scoreTotal += std::max(0.0f, typeScore);
		scoredTypes++;
	}

	if (scoredTypes == 0) return 0;
	float average = scoreTotal / scoredTypes;
	average -= unwanted * (0.2f * average);
	return std::max(0, (int)std::floor(average));
}

int Papas::Game::scoreCutting(const Pizza &pizza, const CustomerData &order) const
{
	int expectedCuts = order.CutPizzaIn / 2;
	if (expectedCuts <= 0) return pizza.cuts.empty() ? 100 : 0;
	std::vector<float> idealAngles;
	for (int i = 0; i < expectedCuts; i++)
		idealAngles.push_back(i * (180.0f / expectedCuts));

	std::vector<bool> used(pizza.cuts.size(), false);
	float angleError = 0.0f;
	for (size_t target = 0; target < idealAngles.size(); target++)
	{
		float best = 45.0f;
		int bestIndex = -1;
		for (size_t i = 0; i < pizza.cuts.size(); i++)
		{
			if (used[i]) continue;
			float dx = pizza.cuts[i].end.x - pizza.cuts[i].start.x;
			float dy = pizza.cuts[i].end.y - pizza.cuts[i].start.y;
			float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
			while (angle < 0.0f) angle += 180.0f;
			while (angle >= 180.0f) angle -= 180.0f;
			float difference = std::abs(angle - idealAngles[target]);
			difference = std::min(difference, 180.0f - difference);
			if (difference < best) { best = difference; bestIndex = i; }
		}
		angleError += best;
		if (bestIndex >= 0) used[bestIndex] = true;
	}
	float angleScore = std::max(0.0f, 100.0f - angleError / (45.0f * expectedCuts) * 100.0f);

	float lengthTotal = 0.0f;
	for (size_t i = 0; i < pizza.cuts.size(); i++)
	{
		float dx = pizza.cuts[i].end.x - pizza.cuts[i].start.x;
		float dy = pizza.cuts[i].end.y - pizza.cuts[i].start.y;
		float length = std::sqrt(dx * dx + dy * dy);
		lengthTotal += std::min(100.0f, std::max(0.0f, (length - 274.0f / 1.5f) * 3.0f / 274.0f * 100.0f));
	}
	float lengthScore = lengthTotal / std::max((int)pizza.cuts.size(), expectedCuts);
	float countMultiplier = std::max(0.0f, 1.0f - std::abs(expectedCuts - (int)pizza.cuts.size()) * 0.15f);
	int score = (int)std::floor((lengthScore * 0.5f + angleScore * 0.5f) * countMultiplier) + 1;
	if (pizza.cuts.empty()) score = 0;
	return std::min(100, std::max(0, score));
}

void Papas::Game::completeServedPizza(Pizza &pizza)
{
	Receipt *ticket = pizza.ticket;
	if (ticket == nullptr) return;
	auto found = map_customers.find(ticket->customerType);
	if (found == map_customers.end()) return;
	const CustomerData &order = found->second;

	result.waiting = scoreWaiting(pizza, order);
	result.topping = scoreToppings(pizza, order);
	result.baking = scoreBaking(pizza, order);
	result.cutting = scoreCutting(pizza, order);
	result.overall = (result.waiting + result.topping + result.baking + result.cutting) / 4;
	result.tipCents = std::max(0, (int)std::round((result.overall * 2.0f - 100.0f) * 3.0f));
	result.customerName = order.name;
	totalScore += result.overall;
	totalTipsCents += result.tipCents;

	int customerNumber = ticket->customerNumber;
	resultCustomerNumber = customerNumber;
	pizza.ticket = nullptr;
	r_manager.removeReceipt(ticket);
	prepareResultText();
	showingResult = true;
}

void Papas::Game::prepareResultText()
{
	C2D_TextBufClear(resultTextBuf);
	char lines[8][64];
	std::snprintf(lines[0], sizeof(lines[0]), "%s's Order", result.customerName.c_str());
	std::snprintf(lines[1], sizeof(lines[1]), "Overall  %d%%", result.overall);
	std::snprintf(lines[2], sizeof(lines[2]), "Waiting  %d%%", result.waiting);
	std::snprintf(lines[3], sizeof(lines[3]), "Toppings %d%%", result.topping);
	std::snprintf(lines[4], sizeof(lines[4]), "Baking   %d%%", result.baking);
	std::snprintf(lines[5], sizeof(lines[5]), "Cutting  %d%%", result.cutting);
	std::snprintf(lines[6], sizeof(lines[6]), "Tip $%d.%02d", result.tipCents / 100, result.tipCents % 100);
	std::snprintf(lines[7], sizeof(lines[7]), "Continue");
	for (int i = 0; i < 8; i++)
	{
		C2D_TextFontParse(&resultText[i], dokyo, resultTextBuf, lines[i]);
		C2D_TextOptimize(&resultText[i]);
	}
}

static void drawCenteredText(const C2D_Text &text, float y, float scale, u32 color)
{
	float width = 0.0f;
	C2D_TextGetDimensions(&text, scale, scale, &width, nullptr);
	C2D_DrawText(&text, C2D_WithColor, (SCREEN_WIDTH_BOTTOM - width) * 0.5f, y, 0.94f, scale, scale, color);
}

void Papas::Game::renderResult()
{
	C2D_DrawRectSolid(18.0f, 12.0f, 0.90f, 284.0f, 220.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(22.0f, 16.0f, 0.91f, 276.0f, 212.0f, C2D_Color32(244, 239, 218, 255));
	drawCenteredText(resultText[0], 24.0f, 0.72f, C2D_Color32(35, 61, 31, 255));
	drawCenteredText(resultText[1], 55.0f, 0.68f, C2D_Color32(170, 63, 24, 255));
	for (int i = 2; i <= 6; i++)
		drawCenteredText(resultText[i], 82.0f + (i - 2) * 22.0f, 0.52f, C2D_Color32(39, 42, 35, 255));
	C2D_DrawRectSolid(100.0f, 198.0f, 0.92f, 120.0f, 30.0f, C2D_Color32(36, 105, 4, 255));
	C2D_DrawRectSolid(103.0f, 201.0f, 0.93f, 114.0f, 24.0f, C2D_Color32(76, 190, 16, 255));
	drawCenteredText(resultText[7], 204.0f, 0.50f, C2D_Color32(11, 33, 0, 255));
}

PapasError Papas::Game::render_top()
{
	if (showingResult)
	{
		C2D_DrawImageAt(to_wallpaper, 0, 0, 0);
		Customer *customer = c_manager.getCustomer(resultCustomerNumber);
		if (customer != nullptr) customer->renderOrdering(0.0f);
		C2D_DrawImageAt(to_counter, 0, 0, 0);
		return PAPAS_OK;
	}

	C2D_DrawImageAt(ticketsHolderImg, 260, 0, 0.002f);

	if(!takingOrder){

		C2D_DrawImageAt(ticketsStationImg, 0, 0, 0.001f);
		C2D_DrawImageAt(currentPopupImg, 0, 214, 0.003f);
		// Customers in the lobby sit between the station art and the popup
		c_manager.renderLines(0.0015f);
		r_manager.renderReceipt(true);
		if (currentStation == TicketStation)
		{
			Roy.renderAnim(true);
		}
		else
		{
			Roy.renderAnimBackwards(false);
		}
	}
	else{
		TakeOrder(c_manager.getOrderingCustomer());
		r_manager.renderDockedReceipt(true);
	}
	

	return PAPAS_OK;
}

// Commiting a bullshittery here
void Papas::Game::TakeOrder(Customer* customer)
{
	if (customer == nullptr)
	{
		// The customer left mid-order somehow; bail out of the flow.
		takingOrder = false;
		return;
	}
	int customerNum = customer->getType();

	// Painter's order: wallpaper -> customer -> counter, so the customer
	// stands behind the countertop (transparent pixels still write depth on
	// this screen, so layering by depth alone doesn't work here).
	C2D_DrawImageAt(to_wallpaper, 0, 0, 0);
	customer->renderOrdering(0.0f);
	C2D_DrawImageAt(to_counter, 0, 0, 0);
	//Roy2.renderAnimWithPauses(5, 2000);

	if(to_firstRun == false){
		to_n_actions = map_customers[customerNum].items.size();
		r_manager.getDockedReceipt(&to_tempReceipt);
		to_tempReceipt->customerType = customerNum;
		to_tempReceipt->customerNumber = customer->getNumber();

		to_firstRun = true;
	}

	//Returns true for one frame on the first frame, and when the animation is paused
	if (Roy2.renderAnimWithPauses(to_n_actions + 2, 2000))
	{
		if (to_currentAction < to_n_actions)
		{
			Papas::ResourceManager::getInstance().playSfx("writepencil");
			to_tempReceipt->addItem(map_customers[customerNum].items[to_currentAction].Coverage,
									map_customers[customerNum].items[to_currentAction].Topping,
									map_customers[customerNum].items[to_currentAction].Quantity);
		}

		if (to_currentAction == to_n_actions)
		{
			Papas::ResourceManager::getInstance().playSfx("writepencil");
			to_tempReceipt->addTime(map_customers[customerNum].time);
		}

		if (to_currentAction == to_n_actions + 1)
		{
			Papas::ResourceManager::getInstance().playSfx("writepencil");
			to_tempReceipt->addCut(map_customers[customerNum].CutPizzaIn);
		}

		if (to_currentAction == to_n_actions + 2)
		{
			to_tempReceipt->orderStartedAt = osGetTime();
			to_firstRun = false;
			to_currentAction = 0;
			to_n_actions = 0;
			to_tempReceipt = nullptr;
			Roy2.resetAnim();
			takingOrder = false;
			// Order's on the ticket: customer walks off to the wait line
			c_manager.orderTaken();
		}

		if(takingOrder == true){
			to_currentAction++;
		}
		
	}

	// if (currentAction == n_actions + 2)
	// {
	// 	tempReceipt->add
	// }
}

PapasError Papas::Game::render_bottom()
{

	C2D_DrawImageAt(currentStationImg, 0, 0, 0.01f);
	if (showingResult)
	{
		renderResult();
		return PAPAS_OK;
	}

	if (currentStation == TicketStation){
		r_manager.renderReceipt(false);
	}

	if (currentStation == TicketStation && takingOrder == false)
	{
		// Only take an order once a customer has reached the counter
		if(createReceipt.showButton(touch) && c_manager.getOrderingCustomer() != nullptr){
			takingOrder = true;
			r_manager.createReceipt();
		}
	}

	if (currentStation == ToppingStation)
	{
		pz_manager.renderBottom(touch, dockedReceipt());
	}

	if (currentStation == BakingStation)
	{
		pz_manager.renderBaking();
	}

	if (currentStation == CuttingStation)
	{
		pz_manager.renderCutting(touch);
		Pizza *served = pz_manager.consumeServedPizza();
		if (served != nullptr)
		{
			completeServedPizza(*served);
			renderResult();
		}
	}

	return PAPAS_OK;
}

PapasError Papas::Game::update()
{

	hidScanInput();
	hidTouchRead(&touch);
	pz_manager.updateTimers();

	// Respond to user input
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu

	if (showingResult)
	{
		bool touching = touch.px != 0 || touch.py != 0;
		bool onContinue = touch.px >= 100 && touch.px <= 220 && touch.py >= 198 && touch.py <= 232;
		if (touching && onContinue)
			resultTouchHeld = true;
		if ((!touching && resultTouchHeld) || (kDown & KEY_A))
		{
			c_manager.completeOrder(resultCustomerNumber);
			resultCustomerNumber = 0;
			showingResult = false;
			resultTouchHeld = false;
			SwitchStation(TicketStation);
		}
		return PAPAS_OK;
	}

	if (kDown & KEY_L)
	{
		if (currentStation > TicketStation)
		{
			if (currentStation == CuttingStation)
			{
				pz_manager.cancelCutting();
			}
			SwitchStation((Stations)(currentStation - 1));
		}
	}

	if (kDown & KEY_R)
	{
		if (currentStation < CuttingStation)
		{
			SwitchStation((Stations)(currentStation + 1));
		}
	}

	if (!takingOrder){
		// Each work station owns the touchscreen; receipts move only at tickets.
		if (currentStation == ToppingStation)
		{
			pz_manager.update(touch, dockedReceipt());
		}
		else if (currentStation == BakingStation)
		{
			pz_manager.updateBaking(touch);
		}
		else if (currentStation == CuttingStation)
		{
			pz_manager.updateCutting(touch);
		}
		else if (currentStation == TicketStation)
		{
			r_manager.detectMovement(touch);
		}
	}

	// Customers keep walking/spawning whatever screen we're on
	c_manager.update();

	return PAPAS_OK;
}

void Papas::Game::SwitchStation(Stations station)
{
	if (station == TicketStation || currentStation == TicketStation)
	{
		Roy.resetAnim();
	}

	switch (station)
	{
	case TicketStation:
		Papas::ResourceManager::getInstance().switchMusic("orders_music");
		currentStationImg = C2D_SpriteSheetGetImage(s_stations, 0);
		currentPopupImg = C2D_SpriteSheetGetImage(s_stations, 1);
		currentStation = station;
		break;
	case ToppingStation:
		Papas::ResourceManager::getInstance().switchMusic("topping_music");
		currentStationImg = C2D_SpriteSheetGetImage(s_stations, 2);
		currentPopupImg = C2D_SpriteSheetGetImage(s_stations, 3);
		currentStation = station;
		break;
	case BakingStation:
		Papas::ResourceManager::getInstance().switchMusic("baking_music");
		currentStationImg = C2D_SpriteSheetGetImage(s_stations, 4);
		currentPopupImg = C2D_SpriteSheetGetImage(s_stations, 5);
		currentStation = station;
		break;
	case CuttingStation:
		Papas::ResourceManager::getInstance().switchMusic("cutting_music");
		currentStationImg = C2D_SpriteSheetGetImage(s_stations, 6);
		currentPopupImg = C2D_SpriteSheetGetImage(s_stations, 7);
		currentStation = station;
		break;
	}
}

PapasError Papas::Game::terminate()
{

	if (s_stations)
	{
		C2D_SpriteSheetFree(s_stations);
		s_stations = nullptr;
	}

	Roy.destroyAnim();
	Roy2.destroyAnim();
	r_manager.terminateManager();
	c_manager.terminateManager();
	pz_manager.terminateManager();
	C2D_SpriteSheetFree(orderStation);
	C2D_TextBufDelete(resultTextBuf);
	C2D_FontFree(dokyo);

	return PAPAS_OK;
}
