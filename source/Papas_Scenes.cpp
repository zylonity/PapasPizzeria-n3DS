#include "Papas_Scenes.h"
#include "Papas_SceneManager.h"
#include <string>
#include <chrono>
#include <gif_lib.h>
#include <gif_lib_private.h>
#include <limits.h>
#include <citro3d.h>
#include <c3d/renderqueue.h>
#include <c3d/base.h>
#include <3ds/gpu/gx.h>

PapasError Papas::MainMenu::init(Papas::SceneManager *sceneManager)
{

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

			IntroVideo *ivid = new IntroVideo();
			p_sceneManager->changeScene(ivid);
		}
	}

	return PAPAS_OK;
}

PapasError Papas::MainMenu::terminate()
{

	// Ensure all Citro2D resources are properly freed
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

PapasError Papas::IntroVideo::init(Papas::SceneManager *sceneManager)
{
	PapasError ret;

	// 1) Open the GIF
	GifFileType *GifFile = DGifOpenFileName("romfs:/lowres.gif", NULL);
	if (!GifFile)
	{
		// Couldn’t open the file
		return PAPAS_NOT_OK;
	}

	// // 2) Slurp to populate all frames
	// if (DGifSlurp(g_intro) == GIF_ERROR) {
	//     // Clean up if slurp fails
	//     DGifCloseFile(g_intro, NULL);
	//     return PAPAS_NOT_OK;
	// }

	// 3) Dimensions and subtexture
	int width = GifFile->SWidth;
	int height = GifFile->SHeight;

	sprite.subtex = new Tex3DS_SubTexture({(u16)width, (u16)height, 0.0f, 1.0f, width / 256.0f, 1.0f - (height / 256.0f)});

	tempTex = new C3D_Tex;
	if (!C3D_TexInit(tempTex, 256, 256, GPU_RGBA8))
	{
		return PAPAS_NOT_OK;
	}

	C3D_TexSetFilter(tempTex, GPU_LINEAR, GPU_LINEAR);
	tempTex->border = 0xFFFFFFFF;
	C3D_TexSetWrap(tempTex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);


	//C3D_SyncTextureCopy((u32*)temp, GX_BUFFER_DIM(256, 256), (u32 *)tempTex->data, GX_BUFFER_DIM(256, 256), (160 * 120 * 4), NULL);
	
	// I dont wanna slurp
	// it sounds weird :(

	// Trying to break down the slurp function for each frame insead
	size_t ImageSize;
	GifRecordType RecordType;
	SavedImage *sp;
	GifByteType *ExtData;
	int ExtFunction;

	GifFile->ExtensionBlocks = NULL;
	GifFile->ExtensionBlockCount = 0;
	counter = 0;

	size_t sizer = 4;
	void *temp [sizer];
	for (size_t i = 0; i < sizer; i++)
	{
		temp[i] = malloc(160 * 120 * 4);
		memcpy(tempTex->data, temp[i], 160 * 120 * 4);
	}
	
	//void *temp = malloc(160 * 120 * 4);
	// C3D_SyncTextureCopy((u32 *)tempTex->data, GX_BUFFER_DIM(256, 256), (u32 *)temp, GX_BUFFER_DIM(256, 256), (256 * 256 * 4), NULL);
	memcpy(tempTex->data, temp, 160 * 120 * 4);

	// Slurp every frame
	do
	{
		DGifGetRecordType(GifFile, &RecordType);// if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) return (PAPAS_NOT_OK);

		switch (RecordType)
		{
		case IMAGE_DESC_RECORD_TYPE:
		{
			// If its a picture
			if (DGifGetImageDesc(GifFile) == GIF_ERROR)
				return (PAPAS_NOT_OK);

			sp = &GifFile->SavedImages[GifFile->ImageCount - 1];
			/* Allocate memory for the image */
			// eror checkin
			if (sp->ImageDesc.Width <= 0 || sp->ImageDesc.Height <= 0 ||
				sp->ImageDesc.Width > (INT_MAX / sp->ImageDesc.Height))
			{
				return PAPAS_NOT_OK;
			}

			ImageSize = sp->ImageDesc.Width * sp->ImageDesc.Height;

			if (ImageSize > (SIZE_MAX / sizeof(GifPixelType)))
			{
				return PAPAS_NOT_OK;
			}
			sp->RasterBits = (unsigned char *)malloc(ImageSize * sizeof(GifPixelType));

			if (sp->RasterBits == NULL)
			{
				return PAPAS_NOT_OK;
			}

			if (sp->ImageDesc.Interlace)
			{
				int i, j;
				/*
				 * The way an interlaced image should be read -
				 * offsets and jumps...
				 */
				int InterlacedOffset[] = {0, 4, 2, 1};
				int InterlacedJumps[] = {8, 8, 4, 2};
				/* Need to perform 4 passes on the image */
				for (i = 0; i < 4; i++)
					for (j = InterlacedOffset[i];
						 j < sp->ImageDesc.Height;
						 j += InterlacedJumps[i])
					{
						if (DGifGetLine(GifFile,
										sp->RasterBits + j * sp->ImageDesc.Width,
										sp->ImageDesc.Width) == GIF_ERROR)
							return PAPAS_NOT_OK;
					}
			}
			else
			{
				if (DGifGetLine(GifFile, sp->RasterBits, ImageSize) == GIF_ERROR)
					return (PAPAS_NOT_OK);
			}

			if (GifFile->ExtensionBlocks) {
				sp->ExtensionBlocks = GifFile->ExtensionBlocks;
				sp->ExtensionBlockCount = GifFile->ExtensionBlockCount;

				GifFile->ExtensionBlocks = NULL;
				GifFile->ExtensionBlockCount = 0;
			}
			// break;
			
				// All the stuff for the picture's been established, lets do ours now?
			//C3D_SyncTextureCopy((u32 *)temp, GX_BUFFER_DIM(256, 256), (u32 *)tempTex->data, GX_BUFFER_DIM(256, 256), (160 * 120 * 4), NULL);
			

			//=;
			//u32 total_size = C3D_TexCalcTotalSize(256 * 256 * 4, 0);


			// Start reading from frame we just got
			u32 *rgbabuffer = (u32 *)malloc(width * height * 4);
			if (!rgbabuffer)
			{
				return PAPAS_NOT_OK;
			}
			memset(rgbabuffer, 0, width * height * 4);

			ColorMapObject *colorMap = (sp->ImageDesc.ColorMap ? sp->ImageDesc.ColorMap : GifFile->SColorMap);
			if (!colorMap)
			{
				return PAPAS_NOT_OK;
			}

			 GifByteType* raster = sp->RasterBits;

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int index = *raster++;
					if (index < colorMap->ColorCount)
					{
						GifColorType color = colorMap->Colors[index];
						int offset = (y * width + x) * 4;
						rgbabuffer[offset + 1] = color.Green;
						rgbabuffer[offset + 0] = color.Red;
						rgbabuffer[offset + 2] = color.Blue;
						rgbabuffer[offset + 3] = 0xFF; // Opaque alpha
					}
				}
			}

			for (u32 x = 0; x < width && x < 256; x++)
			{
				for (u32 y = 0; y < height && y < 256; y++)
				{
					const u32 dstPos = ((((y >> 3) * (256 >> 3) + (x >> 3)) << 6) +
										((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
										 ((x & 4) << 2) | ((y & 4) << 3))) *
									   4;

					const u32 srcPos = (y * width + x) * 4;
					((uint8_t *)temp[counter])[dstPos + 0] = rgbabuffer[srcPos + 3];
					((uint8_t *)temp[counter])[dstPos + 1] = rgbabuffer[srcPos + 2];
					((uint8_t *)temp[counter])[dstPos + 2] = rgbabuffer[srcPos + 1];
					((uint8_t *)temp[counter])[dstPos + 3] = rgbabuffer[srcPos + 0];
				}
			}

			// Free RGBA buffer
			free(rgbabuffer);

			//voids.emplace_back(temp);
			//memcpy(temp, tempTex->data, 256 * 256 * 4);
			//tempTex->data = temp;
			
			// And now we close the memory for the stuff we just opened

			if (sp->ImageDesc.ColorMap != NULL)
			{
				GifFreeMapObject(sp->ImageDesc.ColorMap);
				sp->ImageDesc.ColorMap = NULL;
			}

			if (sp->RasterBits != NULL)
				free((char *)sp->RasterBits);

			GifFreeExtensions(&sp->ExtensionBlockCount, &sp->ExtensionBlocks);
			counter++;
			if (counter >= sizer)
			{
				RecordType = TERMINATE_RECORD_TYPE;
			}
			

			break;
		}
		case EXTENSION_RECORD_TYPE:
		{
			//break;
			if (DGifGetExtension(GifFile, &ExtFunction, &ExtData) == GIF_ERROR)
				return (PAPAS_NOT_OK);
			/* Create an extension block with our data */
			if (ExtData != NULL)
			{
				if (GifAddExtensionBlock(&GifFile->ExtensionBlockCount,
										 &GifFile->ExtensionBlocks,
										 ExtFunction, ExtData[0], &ExtData[1]) == GIF_ERROR)
					return (PAPAS_NOT_OK);
			}
			for (;;)
			{
				if (DGifGetExtensionNext(GifFile, &ExtData) == GIF_ERROR)
					return (PAPAS_NOT_OK);
				if (ExtData == NULL)
					break;
				/* Continue the extension block */
				if (ExtData != NULL)
					if (GifAddExtensionBlock(&GifFile->ExtensionBlockCount,
											 &GifFile->ExtensionBlocks,
											 CONTINUE_EXT_FUNC_CODE,
											 ExtData[0], &ExtData[1]) == GIF_ERROR)
						return (PAPAS_NOT_OK);
			}
			break;
		}
		case TERMINATE_RECORD_TYPE:
		{
			break;
		}

		case UNDEFINED_RECORD_TYPE:
		{
			break;
		}

		default: /* Should be trapped by DGifGetRecordType */
		{
			break;
		}
		}
	} while (RecordType != TERMINATE_RECORD_TYPE);

	//DGifCloseFile(GifFile, NULL);

	//C3D_SyncDisplayTransfer();
	//C3D_SyncTextureCopy((u32 *)voids[currentFrame], GX_BUFFER_DIM(256, 256), (u32 *)tempTex->data, GX_BUFFER_DIM(256, 256), (160 * 120 * 4), NULL);
	//tempTex->data = voids[8];

	C3D_FrameSplit(0);
	Result result = GX_TextureCopy((u32 *)temp[1], GX_BUFFER_DIM(160, 120), (u32 *)tempTex->data, GX_BUFFER_DIM(160, 120), (160 * 120 * 4), GX_TRANSFER_RAW_COPY(0));
	if (R_FAILED(result))
	{
		printf("GX_TextureCopy failed: 0x%08lX\n", result);
		return PAPAS_NOT_OK;
	}
	else
	{
		printf("GX_TextureCopy succeeded: 0x%08lX\n", result);
	}
	// C3D_SyncTextureCopy((u32 *)temp, GX_BUFFER_DIM(160, 120), (u32 *)tempTex->data, GX_BUFFER_DIM(160, 120), (160 * 120 * 4), GX_TRANSFER_RAW_COPY(0));
	free(temp);

	sprite.tex = tempTex;

	start = std::chrono::steady_clock::now();
	currentFrame = 0;

	tempTex = new C3D_Tex;

	return PAPAS_OK;
}

PapasError Papas::IntroVideo::render_top()
{

	// Set up texture



	

	end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	if (elapsed.count() >= 85)
	{

		
		currentFrame++;
		//C3D_SyncTextureCopy((u32 *)voids[currentFrame], GX_BUFFER_DIM(256, 256), (u32 *)tempTex->data, GX_BUFFER_DIM(256, 256), (160 * 120 * 4), NULL);
		printf("%d\n", currentFrame);
		start = end;
	}
	

	C2D_DrawImageAt(sprite, 0, 0, 0);
	// printf("loaded");

	return PAPAS_OK;
}

PapasError Papas::IntroVideo::update()
{

	hidScanInput();

	// Respond to user input
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START)
		return PAPAS_NOT_OK; // break in order to return to hbmenu

	return PAPAS_OK;
}

PapasError Papas::IntroVideo::render_bottom()
{

	// Debug Texture Data
	// for (int i = 0; i < 10; i++) {
	// 	printf("Texture Pixel %d: R=%d, G=%d, B=%d, A=%d\n",
	// 		i,
	// 		((u8*)sprite.tex->data)[i * 4 + 0],
	// 		((u8*)sprite.tex->data)[i * 4 + 1],
	// 		((u8*)sprite.tex->data)[i * 4 + 2],
	// 		((u8*)sprite.tex->data)[i * 4 + 3]);
	// }

	// static auto sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	// auto a = C2D_SpriteSheetGetImage(sheet_bg, 0);
	// printf("W: %d, H: %d\nTop: %f, Left %f\nBottom, %f, Right %f \n", sprite.subtex->width, sprite.subtex->height, sprite.subtex->top, sprite.subtex->left, sprite.subtex->bottom, sprite.subtex->right);
	// C2D_DrawImageAt(sprite, 0, 0, 0);
	// C2D_DrawImage(sprite, );

	return PAPAS_OK;
}

PapasError Papas::IntroVideo::terminate()
{

	return PAPAS_OK;
}