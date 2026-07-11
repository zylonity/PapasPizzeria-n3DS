#include "Papas_Scenes.h"
#include "Papas_SceneManager.h"
#include "Papas_ResourceManager.h"
#include <string>
#include <chrono>
#include <cstring>
#include <citro2d.h>
#include <citro3d.h>
#include <theoraplayer.h>
#include "Papas_Customers.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

// ===========================================================================
// RigTest: isolated harness for the customer rig on a blank background.
// Four experiments across the top screen tell us exactly where drawing breaks.
// ===========================================================================
PapasError Papas::RigTest::init(Papas::SceneManager* sceneManager)
{
	p_sceneManager = sceneManager;
	Papas::CustomerRig::shared().load();
	Papas::CustomerRig::shared().loadType(1, rigAtlas);
	rigSeg = Papas::CustomerRig::shared().segmentIndex("walk"); // start walking
	rigType = 1;
	rigStart = std::chrono::steady_clock::now();
	return PAPAS_OK;
}

PapasError Papas::RigTest::update()
{
	hidScanInput();
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // exit to hbmenu

	// A / B cycle through every animation segment (walk, stand, reactions...).
	int nSeg = Papas::CustomerRig::shared().header()->numSegments;
	if (kDown & KEY_A) { rigSeg = (rigSeg + 1) % nSeg;        rigStart = std::chrono::steady_clock::now(); }
	if (kDown & KEY_B) { rigSeg = (rigSeg + nSeg - 1) % nSeg; rigStart = std::chrono::steady_clock::now(); }
	return PAPAS_OK;
}

PapasError Papas::RigTest::render_top()
{
	// Dark background so we know this scene is live.
	C2D_DrawRectSolid(0, 0, 0.0f, 400, 240, C2D_Color32(20, 20, 60, 255));
	if (!rigAtlas.sheet) return PAPAS_OK;

	auto& rig = Papas::CustomerRig::shared();
	float secs = std::chrono::duration<float>(
		std::chrono::steady_clock::now() - rigStart).count();
	int frame = rig.frameForTime(rigSeg, secs);
	rig.draw(rigAtlas, rigType, frame, 160.0f, 25.0f, 0.6f, 0.6f, 0.5f);
	return PAPAS_OK;
}

PapasError Papas::RigTest::render_bottom()
{
	return PAPAS_OK;
}

PapasError Papas::RigTest::terminate()
{
	Papas::CustomerRig::shared().freeType(rigAtlas);
	return PAPAS_OK;
}

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

	r_manager.initManager(&dokyo);

	createReceipt.createButton(orderStation, 2, 2, 3, {34, 141});

	SwitchStation(TicketStation);

	to_firstRun = false;
	to_currentAction = 0;

	// Customer skeletal rig: load shared animation data + one type's limb atlas.
	Papas::CustomerRig::shared().load();
	Papas::CustomerRig::shared().loadType(1, rigAtlas);
	rigSeg = Papas::CustomerRig::shared().segmentIndex("stand");
	rigStart = std::chrono::steady_clock::now();

	return PAPAS_OK;
}

PapasError Papas::Game::render_top()
{

	C2D_DrawImageAt(ticketsHolderImg, 260, 0, 0.002f);

	if(!takingOrder){

		C2D_DrawImageAt(ticketsStationImg, 0, 0, 0.001f);
		C2D_DrawImageAt(currentPopupImg, 0, 214, 0.003f);
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
		TakeOrder(7);
		r_manager.renderDockedReceipt(true);
	}
	

	return PAPAS_OK;
}

// Commiting a bullshittery here
void Papas::Game::TakeOrder(int customerNum)
{
	C2D_DrawImageAt(to_wallpaper, 0, 0, 0);
	C2D_DrawImageAt(to_counter, 0, 0, 0);
	//Roy2.renderAnimWithPauses(5, 2000);

	// --- Customer rig test render (type 1, looping "stand") ---
	{
		auto& rig = Papas::CustomerRig::shared();
		float secs = std::chrono::duration<float>(
			std::chrono::steady_clock::now() - rigStart).count();
		int frame = rig.frameForTime(rigSeg, secs);
		rig.draw(rigAtlas, 1, frame, /*x*/200.0f, /*y*/12.0f,
				 /*scaleX*/0.6f, /*scaleY*/0.6f, /*depth*/0.5f);
	}


	if(to_firstRun == false){
		to_n_actions = map_customers[customerNum].items.size();
		r_manager.getDockedReceipt(&to_tempReceipt);
		
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
			to_firstRun = false;
			to_currentAction = 0;
			to_n_actions = 0;
			to_tempReceipt = nullptr;
			Roy2.resetAnim();
			takingOrder = false;
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

	if (currentStation == TicketStation){
		r_manager.renderReceipt(false);
	}

	if (currentStation == TicketStation && takingOrder == false)
	{
		if(createReceipt.showButton(touch)){
			takingOrder = true;
			r_manager.createReceipt();
		}
	}

	return PAPAS_OK;
}

PapasError Papas::Game::update()
{

	hidScanInput();
	hidTouchRead(&touch);

	// Respond to user input
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu

	if (kDown & KEY_L)
	{
		if (currentStation > TicketStation)
		{
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
		r_manager.detectMovement(touch);
	}
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
	C2D_SpriteSheetFree(orderStation);
	C2D_FontFree(dokyo);

	Papas::CustomerRig::shared().freeType(rigAtlas);

	return PAPAS_OK;
}