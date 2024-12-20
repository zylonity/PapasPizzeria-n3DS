#include "Papas_Scenes.h"

PapasError Papas::MainMenu::init() {

	// Load the backgrounds
	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);
	bottom_bg = C2D_SpriteSheetGetImage(sheet_bg, 0);

	//Load the icons
	sheet_icons = C2D_SpriteSheetLoad("romfs:/gfx/icons.t3x");
	logo = C2D_SpriteSheetGetImage(sheet_icons, 0);

	//Positions for the buttons
	float buttonheight = b_start.getRect().height;
	float padding = 10;

	Papas::v2 startPos;
	startPos.x = (SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2);
	startPos.y = 50;

	/*Papas::v2 helpPos;
	helpPos.x = (SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2);
	helpPos.y = startPos.y + buttonheight + padding;

	Papas::v2 contactPos;
	contactPos.x = (SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2);
	contactPos.y = helpPos.y + buttonheight + padding;*/
	
	
	//Load the buttons
	sheet_buttons = C2D_SpriteSheetLoad("romfs:/gfx/buttons.t3x");
	b_start.createButton(sheet_buttons, 0, 1, startPos);
	//b_help.createButton(sheet_buttons, 2, 3, helpPos);
	//b_credits.createButton(sheet_buttons, 4, 5, contactPos);


	return PAPAS_OK;
}

PapasError Papas::MainMenu::update() {

	hidScanInput();
	hidTouchRead(&touch);

	// Respond to user input
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu

	return PAPAS_OK;

}

PapasError Papas::MainMenu::render_top() {

	// Draw the top background
	C2D_DrawImageAt(top_bg, 0, 0, 0, NULL, 1, 1);

	//Draw the logo
	float scaling = 0.7f;
	float xmiddle = (SCREEN_WIDTH_TOP / 2) - ((logo.subtex->width * scaling) / 2);
	float ymiddle = (SCREEN_HEIGHT_TOP / 2) - ((logo.subtex->height * scaling) / 2);
	C2D_DrawImageAt(logo, xmiddle, ymiddle, 0, NULL, scaling, scaling);

	return PAPAS_OK;
}

PapasError Papas::MainMenu::render_bottom() {

	// Draw the background
	C2D_DrawImageAt(bottom_bg, 0, 0, 0, NULL, 1, 1);

	//Draw buttons
	b_start.showButton(touch);
	//b_help.showButton(touch);
	//b_credits.showButton(touch);

	//C2D_DrawImageAt(start, middle, box1_y, 1, NULL, 1, 1);
	//C2D_DrawImageAt(help, middle, box2_y, 1, NULL, 1, 1);
	//C2D_DrawImageAt(credits, middle, box3_y, 1, NULL, 1, 1);


	return PAPAS_OK;
}

PapasError Papas::MainMenu::terminate() {
	
	return PAPAS_OK;
}