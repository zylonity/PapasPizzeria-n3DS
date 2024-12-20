#include "Papas_Renderer.h"
//#include <cassert>
#include <assert.h>

PapasError Papas::Renderer::init() {
	PapasError ret;

	// Init various things
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	// Don't do that
	// consoleInit(GFX_BOTTOM, NULL);

	// Create a C3D render target
	topRenderTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottomRenderTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Load the backgrounds
	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);
	bottom_bg = C2D_SpriteSheetGetImage(sheet_bg, 0);

	//Load the icons
	sheet_icons = C2D_SpriteSheetLoad("romfs:/gfx/icons.t3x");
	logo = C2D_SpriteSheetGetImage(sheet_icons, 0);

	//Load the buttons
	sheet_buttons = C2D_SpriteSheetLoad("romfs:/gfx/buttons.t3x");
	start = C2D_SpriteSheetGetImage(sheet_buttons, 0);
	help = C2D_SpriteSheetGetImage(sheet_buttons, 1);
	credits = C2D_SpriteSheetGetImage(sheet_buttons, 2);


	return PAPAS_OK;
}

PapasError Papas::Renderer::update(Papas::SceneManager* sceneManager) {
	PapasError ret;

	hidScanInput();

	// Respond to user input
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu


	ret = render(sceneManager);
	assert(ret == PAPAS_OK);

	return PAPAS_OK;
}

PapasError Papas::Renderer::render(Papas::SceneManager* sceneManager) {
	PapasError ret;

	// Render the scene
	C2D_TargetClear(topRenderTarget, C2D_Color32(0x00, 0x00, 0x00, 0xff));
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_SceneBegin(topRenderTarget);

	// Draw the top background
	C2D_DrawImageAt(top_bg, 0, 0, 0, NULL, 1, 1);

	//Draw the logo
	float scaling = 0.7f;
	float xmiddle = (SCREEN_WIDTH_TOP / 2) - ((logo.subtex->width * scaling) / 2);
	float ymiddle = (SCREEN_HEIGHT_TOP / 2) - ((logo.subtex->height * scaling) / 2);
	C2D_DrawImageAt(logo, xmiddle, ymiddle, 0, NULL, scaling, scaling);

	//assert(false);
	sceneManager->render_top();


	C3D_FrameEnd(0);

	// Render the scene
	C2D_TargetClear(bottomRenderTarget, C2D_Color32(0x00, 0x00, 0x00, 0xff));
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_SceneBegin(bottomRenderTarget);

	float middle = (SCREEN_WIDTH_BOTTOM / 2) - (start.subtex->width / 2);
	float buttonheight = start.subtex->height;
	float padding = 10;
	float box1_y = 50;
	float box2_y = box1_y + buttonheight + padding;
	float box3_y = box2_y + buttonheight + padding;

	// Draw the background
	C2D_DrawImageAt(bottom_bg, 0, 0, 0, NULL, 1, 1);
	C2D_DrawImageAt(start, middle, box1_y, 1, NULL, 1, 1);
	C2D_DrawImageAt(help, middle, box2_y, 1, NULL, 1, 1);
	C2D_DrawImageAt(credits, middle, box3_y, 1, NULL, 1, 1);

	sceneManager->render_bottom();

	C3D_FrameEnd(0);


	return PAPAS_OK;
}

PapasError Papas::Renderer::terminate() {
	PapasError ret;

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();

	return PAPAS_OK;
}