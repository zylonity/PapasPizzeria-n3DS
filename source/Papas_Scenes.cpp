#include "Papas_Scenes.h"
#include "Papas_SceneManager.h"
#include "Papas_ResourceManager.h"
#include <string>
#include <chrono>
#include <citro2d.h>

PapasError Papas::MainMenu::init(Papas::SceneManager *sceneManager)
{

	Papas::ResourceManager::getInstance().playMusic("romfs:/music/toppingscreen_music.ogg");

	// Load the backgrounds
	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);
	bottom_bg = C2D_SpriteSheetGetImage(sheet_bg, 0);

	// Load the icons
	sheet_icons = C2D_SpriteSheetLoad("romfs:/gfx/icons.t3x");
	logo = C2D_SpriteSheetGetImage(sheet_icons, 0);

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

			Game *game = new Game();
			p_sceneManager->changeScene(game);
			//ResourceManager::getInstance().stopMusic();
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
	if (sheet_icons)
	{
		C2D_SpriteSheetFree(sheet_icons);
		sheet_icons = nullptr;
	}
	if (sheet_buttons)
	{
		C2D_SpriteSheetFree(sheet_buttons);
		sheet_buttons = nullptr;
	}

	return PAPAS_OK;
}

PapasError Papas::Game::init(Papas::SceneManager *sceneManager)
{
	bottomStations = C2D_SpriteSheetLoad("romfs:/gfx/stations.t3x");
	topStation = C2D_SpriteSheetLoad("romfs:/gfx/top_stations.t3x");
	popups = C2D_SpriteSheetLoad("romfs:/gfx/popups.t3x");

	ticketsStationImg = C2D_SpriteSheetGetImage(topStation, 0);
	ticketsHolderImg = C2D_SpriteSheetGetImage(topStation, 1);

	v2 pos = {-24, 45};
	v2 scl = {0.8f, 0.8f};
	guy.createAnim("romfs:/gfx/guy_peeking.t3x", 84.0f, pos, scl, 35.0f);

	rep.createReceipt(1);

	SwitchStation(TicketStation);

	return PAPAS_OK;
}

PapasError Papas::Game::render_top()
{

	C2D_DrawImageAt(ticketsStationImg, 0, 0, -1);
	C2D_DrawImageAt(ticketsHolderImg, 260, 0, 0);
	C2D_DrawImageAt(currentPopupImg, 0, 214, 1);

		if(currentStation == TicketStation){
		guy.renderAnim(true);
	}
	else{
		guy.renderAnimBackwards(false);
	}
	

	return PAPAS_OK;
}

PapasError Papas::Game::render_bottom()
{

	C2D_DrawImageAt(currentStationImg, 0, 0, -1);
	rep.showReceipt();

	return PAPAS_OK;
}

PapasError Papas::Game::update()
{

	hidScanInput();

	// Respond to user input
	u32 kDown = hidKeysDown();

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

	return PAPAS_OK;
}

void Papas::Game::SwitchStation(Stations station)
{
	if (station == TicketStation || currentStation == TicketStation)
	{
		guy.resetAnim();
	}

	switch (station)
	{
	case TicketStation:
		Papas::ResourceManager::getInstance().switchMusic("romfs:/music/orderscreen_music.ogg");
		currentStationImg = C2D_SpriteSheetGetImage(bottomStations, 0);
		currentPopupImg = C2D_SpriteSheetGetImage(popups, 0);
		currentStation = station;
		break;
	case ToppingStation:
		Papas::ResourceManager::getInstance().switchMusic("romfs:/music/toppingscreen_music.ogg");
		currentStationImg = C2D_SpriteSheetGetImage(bottomStations, 1);
		currentPopupImg = C2D_SpriteSheetGetImage(popups, 1);
		currentStation = station;
		break;
	case BakingStation:
		Papas::ResourceManager::getInstance().switchMusic("romfs:/music/bakingscreen_music.ogg");
		currentStationImg = C2D_SpriteSheetGetImage(bottomStations, 2);
		currentPopupImg = C2D_SpriteSheetGetImage(popups, 2);
		currentStation = station;
		break;
	case CuttingStation:
		Papas::ResourceManager::getInstance().switchMusic("romfs:/music/cuttingscreen_music.ogg");
		currentStationImg = C2D_SpriteSheetGetImage(bottomStations, 3);
		currentPopupImg = C2D_SpriteSheetGetImage(popups, 3);
		currentStation = station;
		break;
	}

	
}

PapasError Papas::Game::terminate()
{

	if (bottomStations)
	{
		C2D_SpriteSheetFree(bottomStations);
		bottomStations = nullptr;
	}
	if (topStation)
	{
		C2D_SpriteSheetFree(topStation);
		topStation = nullptr;
	}

	guy.destroyAnim();

	return PAPAS_OK;
}