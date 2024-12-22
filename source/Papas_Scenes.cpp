#include "Papas_Scenes.h"
#include "Papas_SceneManager.h"
#include <string>
#include <chrono>
#include <gif_lib.h>

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


PapasError Papas::IntroVideo::init(Papas::SceneManager* sceneManager) 
{
	PapasError ret;

    // 1) Open the GIF
    GifFileType* g_intro = DGifOpenFileName("romfs:/frame0.gif", NULL);
    if (!g_intro) {
        // Couldn’t open the file
        return PAPAS_NOT_OK;
    }

    // 2) Slurp to populate all frames
    if (DGifSlurp(g_intro) == GIF_ERROR) {
        // Clean up if slurp fails
        DGifCloseFile(g_intro, NULL);
        return PAPAS_NOT_OK;
    }

    // 3) Dimensions and subtexture
    int width  = g_intro->SWidth;
    int height = g_intro->SHeight;

	sprite.tex = new C3D_Tex;
	sprite.subtex = new Tex3DS_SubTexture({(u16)width, (u16)height, 0.0f, 1.0f, width / 256.0f, 1.0f - (height / 256.0f)});

	if (!C3D_TexInit(sprite.tex, 256, 256, GPU_RGBA8)) {
			return PAPAS_NOT_OK;
	}

	C3D_TexSetFilter(sprite.tex, GPU_LINEAR, GPU_LINEAR);
	sprite.tex->border = 0xFFFFFFFF;
	C3D_TexSetWrap(sprite.tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);


	for (size_t i = 0; i < g_intro->ImageCount; i++)
    {
        SavedImage* image = &g_intro->SavedImages[i];

        // 4a) Allocate the RGBA buffer
        u32* rgbaBuffer = (u32*)malloc(width * height * 4);
        if (!rgbaBuffer) {
            // On error, clean up and return
            DGifCloseFile(g_intro, NULL);
            return PAPAS_NOT_OK;
        }
        memset(rgbaBuffer, 0, width * height * 4);


        // 4b) Get the color map
        ColorMapObject* colorMap = (image->ImageDesc.ColorMap
                                    ? image->ImageDesc.ColorMap
                                    : g_intro->SColorMap);
        if (!colorMap) {
            free(rgbaBuffer);
            DGifCloseFile(g_intro, NULL);
            return PAPAS_NOT_OK;
        }

        // 4c) Decode the raster bits into RGBA
        GifByteType* raster = image->RasterBits;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = *raster++;
                if (index < colorMap->ColorCount) {
                    GifColorType color = colorMap->Colors[index];
                    int offset = (y * width + x) * 4;
                    rgbaBuffer[offset + 0] = color.Red;
                    rgbaBuffer[offset + 1] = color.Green;
                    rgbaBuffer[offset + 2] = color.Blue;
                    rgbaBuffer[offset + 3] = 0xFF; // Opaque alpha
                }
            }
        }

		for (u32 x = 0; x < width && x < 256; x++) {
			for (u32 y = 0; y < height && y < 256; y++) {
				const u32 dstPos = ((((y >> 3) * (256 >> 3) + (x >> 3)) << 6) +
									((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
									((x & 4) << 2) | ((y & 4) << 3))) * 4;

				const u32 srcPos = (y * width + x) * 4;
				((uint8_t *)sprite.tex->data)[dstPos + 0] = rgbaBuffer[srcPos + 3];
				((uint8_t *)sprite.tex->data)[dstPos + 1] = rgbaBuffer[srcPos + 2];
				((uint8_t *)sprite.tex->data)[dstPos + 2] = rgbaBuffer[srcPos + 1];
				((uint8_t *)sprite.tex->data)[dstPos + 3] = rgbaBuffer[srcPos + 0];
			}
		}

		// Free RGBA buffer
		free(rgbaBuffer);


    }

    DGifCloseFile(g_intro, NULL);


    return PAPAS_OK;
}

PapasError Papas::IntroVideo::render_top() {

	//auto frame = C2D_SpriteSheetGetImage(p_cSheet, 0);
	//sprite.subtex.
	
	C2D_DrawImageAt(sprite, 0, 0, 0);

	

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


PapasError Papas::IntroVideo::render_bottom() {

	// Debug Texture Data
		// for (int i = 0; i < 10; i++) {
		// 	printf("Texture Pixel %d: R=%d, G=%d, B=%d, A=%d\n",
		// 		i,
		// 		((u8*)sprite.tex->data)[i * 4 + 0],
		// 		((u8*)sprite.tex->data)[i * 4 + 1],
		// 		((u8*)sprite.tex->data)[i * 4 + 2],
		// 		((u8*)sprite.tex->data)[i * 4 + 3]);
		// }

	//static auto sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	//auto a = C2D_SpriteSheetGetImage(sheet_bg, 0);
	//printf("W: %d, H: %d\nTop: %f, Left %f\nBottom, %f, Right %f \n", sprite.subtex->width, sprite.subtex->height, sprite.subtex->top, sprite.subtex->left, sprite.subtex->bottom, sprite.subtex->right);
	//C2D_DrawImageAt(sprite, 0, 0, 0);
	//C2D_DrawImage(sprite, );


	return PAPAS_OK;
}

PapasError Papas::IntroVideo::terminate() {

	return PAPAS_OK;
}