#include "Papas_Scenes.h"

PapasError Papas::MainMenu::init() {

	// Load the backgrounds
	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);
	bottom_bg = C2D_SpriteSheetGetImage(sheet_bg, 0);

	//Load the icons
	sheet_icons = C2D_SpriteSheetLoad("romfs:/gfx/icons.t3x");
	logo = C2D_SpriteSheetGetImage(sheet_icons, 0);
	
	
	//Load the buttons
	sheet_buttons = C2D_SpriteSheetLoad("romfs:/gfx/buttons.t3x");

	//Start button
	Papas::Button b_start(sheet_buttons, 0, 1, 2);
	Papas::v2 startPos = { (SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2), 50.0f };
	b_start.setPosition(startPos);
	v_buttons.push_back(b_start);

	float buttonheight = b_start.getRect().height;
	float padding = 10;

	Papas::Button b_help(sheet_buttons, 3, 4, 5);
	Papas::v2 helpPos = { (SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2), startPos.y + buttonheight + padding };
	b_help.setPosition(helpPos);
	v_buttons.push_back(b_help);

	Papas::Button b_credits(sheet_buttons, 6, 7, 8);
	Papas::v2 creditsPos = { (SCREEN_WIDTH_BOTTOM / 2) - (b_start.getRect().width / 2), helpPos.y + buttonheight + padding };
	b_credits.setPosition(creditsPos);
	v_buttons.push_back(b_credits);

	buttonIndex = 0;
	aPressed = false;

	return PAPAS_OK;
}

PapasError Papas::MainMenu::update() {

	hidScanInput();
	hidTouchRead(&touch);

	// Respond to user input
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu


	//Browse through menu with DPAD
	if (kDown & KEY_DDOWN) {
		if (buttonIndex < v_buttons.size()) {
			buttonIndex++;
		}

		if (buttonIndex == v_buttons.size()) {
			buttonIndex = 0;
		}
		
	}

	if (kDown & KEY_DUP) {
		if (buttonIndex >= 0) {
			buttonIndex--;
		}

		if (buttonIndex < 0) {
			buttonIndex = v_buttons.size() - 1;
		}

	}

	//Detect when A's pressed to pass it through to the on screen button
	if (kDown & KEY_A) {
		aPressed = true;
	}

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




	for (size_t i = 0; i < v_buttons.size(); i++)
	{
		//Detect when button is pressed either through keys or touchscreen	
		if (v_buttons[i].showButton(touch, i == buttonIndex, &aPressed)) {

			v_buttons[i].setPosition(v2(0));
		}
	}


	return PAPAS_OK;
}

PapasError Papas::MainMenu::terminate() {
	
	return PAPAS_OK;
}