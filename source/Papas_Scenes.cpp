#include "Papas_Scenes.h"
#include "Papas_SceneManager.h"
#include <string>
#include <chrono>
PapasError Papas::MainMenu::init(Papas::SceneManager* sceneManager) {

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

	p_sceneManager = sceneManager;

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

			IntroVideo* ivid = new IntroVideo();
			p_sceneManager->changeScene(ivid);
		}
	}


	return PAPAS_OK;
}

PapasError Papas::MainMenu::terminate() {
	
	// Ensure all Citro2D resources are properly freed
	if (sheet_bg) {
		C2D_SpriteSheetFree(sheet_bg);
		sheet_bg = nullptr;
	}
	if (sheet_icons) {
		C2D_SpriteSheetFree(sheet_icons);
		sheet_icons = nullptr;
	}
	if (sheet_buttons) {
		C2D_SpriteSheetFree(sheet_buttons);
		sheet_buttons = nullptr;
	}

	return PAPAS_OK;
}


PapasError Papas::IntroVideo::init(Papas::SceneManager* sceneManager) {

	for (size_t i = 1; i <= 10; i++) {
		std::string path = "romfs:/gfx/introvideo_part" + std::to_string(i) + ".t3x";

		// Load the sprite sheet
		C2D_SpriteSheet sheet = C2D_SpriteSheetLoad(path.c_str());

		video_parts.push_back(sheet);

	}

	p_sceneManager = sceneManager;

	return PAPAS_OK;
}

PapasError Papas::IntroVideo::update() {

	hidScanInput();

	// Respond to user input
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu


	return PAPAS_OK;

}

PapasError Papas::IntroVideo::render_top() {
	static size_t currentFrame = 0;               // Current frame index
	static auto lastFrameTime = std::chrono::steady_clock::now();
	static size_t currentSheetBatch = 0;         // Index of the current batch of 10 sprite sheets

	// Calculate elapsed time since the last frame was rendered
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameTime);

	// Update frame if 1 second (1000ms) has passed
	if (elapsed.count() >= 1000) {
		// Unload the previously loaded frame (if any)
		if (!video_frames.empty()) {
			video_frames.clear(); // Free memory for the previous frame
		}

		// Determine which sprite sheet and frame to load
		size_t totalSheets = video_parts.size();
		if (totalSheets == 0) {
			return PAPAS_NOT_OK; // Fail gracefully if no sprite sheets are loaded
		}

		size_t framesPerSheet = C2D_SpriteSheetCount(video_parts[0]); // Assuming all sheets have the same frame count
		size_t totalFrames = totalSheets * framesPerSheet;

		if (currentFrame >= totalFrames) {
			currentFrame = 0; // Loop back to the start
		}

		size_t sheetIndex = currentFrame / framesPerSheet;
		size_t frameIndexInSheet = currentFrame % framesPerSheet;

		// Check if the current sheet index is outside the current batch
		size_t newSheetBatch = sheetIndex / 10; // Calculate the new batch (10 sheets per batch)
		if (newSheetBatch != currentSheetBatch) {
			// Unload the current batch of sprite sheets
			size_t startSheetIndex = currentSheetBatch * 10;
			size_t endSheetIndex = std::min(startSheetIndex + 10, totalSheets);
			for (size_t i = startSheetIndex; i < endSheetIndex; ++i) {
				C2D_SpriteSheetFree(video_parts[i]);
				video_parts[i] = nullptr; // Mark the sheet as unloaded
			}

			// Load the new batch of sprite sheets
			currentSheetBatch = newSheetBatch;
			startSheetIndex = currentSheetBatch * 10;
			endSheetIndex = std::min(startSheetIndex + 10, totalSheets);
			for (size_t i = startSheetIndex; i < endSheetIndex; ++i) {
				std::string path = "romfs:/gfx/introvideo_part" + std::to_string(i + 1) + ".t3x";
				video_parts[i] = C2D_SpriteSheetLoad(path.c_str());
				if (!video_parts[i]) {
					return PAPAS_NOT_OK; // Handle failed sheet loading
				}
			}
		}

		// Load the specific frame from the sprite sheet
		C2D_SpriteSheet sheet = video_parts[sheetIndex];
		C2D_Image frame = C2D_SpriteSheetGetImage(sheet, frameIndexInSheet);
		video_frames.push_back(frame); // Store the currently loaded frame

		// Advance to the next frame
		currentFrame++;

		lastFrameTime = now; // Update the last frame time
	}

	// Draw the current frame
	if (!video_frames.empty()) {
		C2D_DrawImageAt(video_frames[0], 0, 0, 0, NULL, 1, 1);
	}

	return PAPAS_OK;
}

PapasError Papas::IntroVideo::render_bottom() {

	




	return PAPAS_OK;
}

PapasError Papas::IntroVideo::terminate() {

	return PAPAS_OK;
}