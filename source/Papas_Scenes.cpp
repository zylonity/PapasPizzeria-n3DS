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
#include "Papas_Customers.h"
#include "Papas_GiveOrderFrames.h"
#include "Papas_StartOfDayFrames.h"
#include "Papas_DayNumber.h"
#include "Papas_Sign.h"
#include "Papas_NewCustomerFrames.h"
#include "Papas_IntroFrames.h"
#include "Papas_Stereo.h"
#include "Papas_Save.h"

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
	Papas::ResourceManager::getInstance().loadSfx("buttonclick", "romfs:/sfx/buttonclick.wav");

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
	if (kDown & (KEY_START | KEY_SELECT))
		return PAPAS_EXIT_REQUESTED;

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
	Papas::Stereo::plane(0.8f);
	C2D_DrawImageAt(top_bg, 0, 0, 0, NULL, 1, 1);

	// Draw the logo, floating a touch in front of the screen
	Papas::Stereo::plane(-0.1f);
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
			Papas::ResourceManager::getInstance().playSfx("buttonclick");
			if(i == 0){
				// Keep menu music through the picker; it stops when the game starts.
				p_sceneManager->changeScene(new Papas::SaveSelect());
				return PAPAS_OK; // changeScene deleted us; touch nothing else
			}
			if(i == 1){
				p_sceneManager->changeScene(new Papas::HelpScreen());
				return PAPAS_OK; // changeScene deleted us; touch nothing else
			}
			if(i == 2){
				p_sceneManager->changeScene(new Papas::CreditsScreen());
				return PAPAS_OK; // changeScene deleted us; touch nothing else
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

// Save-slot panel layout on the bottom screen
static const float SLOT_X = 20.0f;
static const float SLOT_W = 280.0f;
static const float SLOT_H = 52.0f;
static float slotY(int i) { return 34.0f + i * 60.0f; }

// Show three save tickets, popping the selected one above the row.
static const float RCPT_SCALE = 0.85f;
static const float RCPT_PEEK = 65.0f;	// px of ticket visible while tucked

static void drawTextCentered(const C2D_Text *text, float centerX, float y, float z, float scale, u32 color)
{
	float tw = 0.0f, th = 0.0f;
	C2D_TextGetDimensions(text, scale, scale, &tw, &th);
	C2D_DrawText(text, C2D_WithColor, centerX - tw / 2.0f, y, z, scale, scale, color);
}

PapasError Papas::SaveSelect::init(Papas::SceneManager *sceneManager)
{
	p_sceneManager = sceneManager;

	// Same backdrop as the main menu so the transition reads as one screen
	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);
	bottom_bg = C2D_SpriteSheetGetImage(sheet_bg, 0);
	logo = C2D_SpriteSheetGetImage(sheet_bg, 2);

	// The blank order ticket doubles as the save-file card on the top screen
	sheet_receipt = C2D_SpriteSheetLoad("romfs:/gfx/receipt.t3x");
	receiptImg = C2D_SpriteSheetGetImage(sheet_receipt, 0);
	C3D_TexSetFilter(receiptImg.tex, GPU_LINEAR, GPU_LINEAR);
	for (int i = 0; i < SaveManager::SLOT_COUNT; i++)
		popAmount[i] = 0.0f;

	dokyo = C2D_FontLoad("romfs:/fonts/Dokyo.bcfnt");
	textBuf = C2D_TextBufNew(512);

	selected = 0;
	deleteArmed = -1;
	refreshSlotText();

	return PAPAS_OK;
}

// (Re)build every label: called on init and after a delete
void Papas::SaveSelect::refreshSlotText()
{
	C2D_TextBufClear(textBuf);

	C2D_TextFontParse(&headerText, dokyo, textBuf, "Choose a File");
	C2D_TextOptimize(&headerText);
	C2D_TextFontParse(&hintText, dokyo, textBuf, "A Select    B Back    X Delete");
	C2D_TextOptimize(&hintText);
	C2D_TextFontParse(&deleteText, dokyo, textBuf, "Delete? Press X again!");
	C2D_TextOptimize(&deleteText);

	for (int i = 0; i < SaveManager::SLOT_COUNT; i++)
	{
		char title[16];
		std::snprintf(title, sizeof(title), "File %d", i + 1);
		C2D_TextFontParse(&slotTitleText[i], dokyo, textBuf, title);
		C2D_TextOptimize(&slotTitleText[i]);

		SaveData peeked;
		slotUsed[i] = SaveManager::getInstance().peekSlot(i, peeked);
		char info[48];
		if (slotUsed[i])
			std::snprintf(info, sizeof(info), "Day %ld  -  Rank %ld", (long)peeked.day, (long)peeked.rank);
		else
			std::snprintf(info, sizeof(info), "New Game");
		C2D_TextFontParse(&slotInfoText[i], dokyo, textBuf, info);
		C2D_TextOptimize(&slotInfoText[i]);

		// Used slots show day and rank; blank ones just say "New Game".
		char line[16];
		if (slotUsed[i])
		{
			std::snprintf(line, sizeof(line), "Day %ld", (long)peeked.day);
			C2D_TextFontParse(&receiptDayText[i], dokyo, textBuf, line);
			std::snprintf(line, sizeof(line), "Rank %ld", (long)peeked.rank);
			C2D_TextFontParse(&receiptRankText[i], dokyo, textBuf, line);
			C2D_TextOptimize(&receiptRankText[i]);
		}
		else
		{
			C2D_TextFontParse(&receiptDayText[i], dokyo, textBuf, "New Game");
		}
		C2D_TextOptimize(&receiptDayText[i]);
	}
}

// changeScene deletes this picker, so callers must return right away.
PapasError Papas::SaveSelect::activateSlot(int slot)
{
	// Both destinations start their own audio, so end menu music here.
	ResourceManager::getInstance().stopMusic();
	if (slotUsed[slot] && SaveManager::getInstance().loadSlot(slot))
	{
		// Returning player: skip the intro, straight into the saved day
		p_sceneManager->changeScene(new Papas::Game());
	}
	else
	{
		SaveManager::getInstance().startNewGame(slot);
		p_sceneManager->changeScene(new Papas::IntroCutscene());
	}
	return PAPAS_OK;
}

PapasError Papas::SaveSelect::update()
{
	hidScanInput();
	hidTouchRead(&touch);
	u32 kDown = hidKeysDown();
	if (kDown & (KEY_START | KEY_SELECT))
		return PAPAS_EXIT_REQUESTED;

	if (kDown & KEY_B)
	{
		p_sceneManager->changeScene(new Papas::MainMenu());
		return PAPAS_OK; // changeScene deleted us; touch nothing else
	}

	if (kDown & KEY_DDOWN)
	{
		selected = (selected + 1) % SaveManager::SLOT_COUNT;
		deleteArmed = -1;
	}
	if (kDown & KEY_DUP)
	{
		selected = (selected + SaveManager::SLOT_COUNT - 1) % SaveManager::SLOT_COUNT;
		deleteArmed = -1;
	}

	if (kDown & KEY_A)
		return activateSlot(selected);

	if (kDown & KEY_TOUCH)
	{
		for (int i = 0; i < SaveManager::SLOT_COUNT; i++)
		{
			if (touch.px >= SLOT_X && touch.px <= SLOT_X + SLOT_W &&
			    touch.py >= slotY(i) && touch.py <= slotY(i) + SLOT_H)
			{
				selected = i;
				return activateSlot(i);
			}
		}
		deleteArmed = -1;
	}

	// Deleting takes two X presses on the same slot; anything else disarms
	if (kDown & KEY_X)
	{
		if (slotUsed[selected])
		{
			if (deleteArmed == selected)
			{
				SaveManager::getInstance().eraseSlot(selected);
				deleteArmed = -1;
				refreshSlotText();
			}
			else
			{
				deleteArmed = selected;
			}
		}
	}

	// Animate here so both stereo eye renders see the same ticket height.
	for (int i = 0; i < SaveManager::SLOT_COUNT; i++)
	{
		float target = (i == selected) ? 1.0f : 0.0f;
		popAmount[i] += (target - popAmount[i]) * 0.22f;
	}

	return PAPAS_OK;
}

PapasError Papas::SaveSelect::render_top()
{
	Papas::Stereo::plane(0.8f);
	C2D_DrawImageAt(top_bg, 0, 0, 0, NULL, 1, 1);

	// Logo shrunk up top so the tickets have room to rise over it
	Papas::Stereo::plane(0.3f);
	float scaling = 0.45f;
	float xmiddle = (SCREEN_WIDTH_TOP / 2) - ((logo.subtex->width * scaling) / 2);
	C2D_DrawImageAt(logo, xmiddle, 6.0f, 0.2f, NULL, scaling, scaling);

	const u32 colInk = C2D_Color32(58, 58, 58, 255);
	const u32 colInkSoft = C2D_Color32(155, 125, 105, 255);

	float w = receiptImg.subtex->width * RCPT_SCALE;
	float h = receiptImg.subtex->height * RCPT_SCALE;
	float gap = (SCREEN_WIDTH_TOP - SaveManager::SLOT_COUNT * w) / (SaveManager::SLOT_COUNT + 1);

	for (int i = 0; i < SaveManager::SLOT_COUNT; i++)
	{
		float x = gap + i * (w + gap);
		float yTucked = SCREEN_HEIGHT_TOP - RCPT_PEEK;	// header + hole showing
		float yPopped = SCREEN_HEIGHT_TOP - h + 4.0f;	// whole ticket in view
		float y = yTucked + (yPopped - yTucked) * popAmount[i];

		// The ticket floats toward the player as it rises
		Papas::Stereo::plane(0.5f - 0.45f * popAmount[i]);
		C2D_DrawImageAt(receiptImg, x, y, 0.4f, NULL, RCPT_SCALE, RCPT_SCALE);

		// Place the file line just below the header so tucked tickets stay readable.
		float cx = x + w / 2.0f;
		drawTextCentered(&slotTitleText[i], cx, y + 47.0f * RCPT_SCALE, 0.5f, 0.55f, colInk);
		if (slotUsed[i])
		{
			drawTextCentered(&receiptDayText[i], cx, y + 100.0f * RCPT_SCALE, 0.5f, 0.5f, colInk);
			drawTextCentered(&receiptRankText[i], cx, y + 135.0f * RCPT_SCALE, 0.5f, 0.5f, colInk);
		}
		else
		{
			drawTextCentered(&receiptDayText[i], cx, y + 110.0f * RCPT_SCALE, 0.5f, 0.42f, colInkSoft);
		}
	}

	return PAPAS_OK;
}

PapasError Papas::SaveSelect::render_bottom()
{
	C2D_DrawImageAt(bottom_bg, 0, 0, 0, NULL, 1, 1);

	const u32 colCream    = C2D_Color32(255, 248, 231, 255);
	const u32 colBrown    = C2D_Color32(139, 98, 62, 255);
	const u32 colOrange   = C2D_Color32(236, 121, 42, 255);
	const u32 colRed      = C2D_Color32(205, 60, 50, 255);
	const u32 colText     = C2D_Color32(58, 58, 58, 255);
	const u32 colTextSoft = C2D_Color32(120, 105, 90, 255);

	float tw = 0.0f, th = 0.0f;
	C2D_TextGetDimensions(&headerText, 0.7f, 0.7f, &tw, &th);
	C2D_DrawText(&headerText, C2D_WithColor, (SCREEN_WIDTH_BOTTOM - tw) / 2.0f, 6.0f, 0.5f, 0.7f, 0.7f, colText);

	for (int i = 0; i < SaveManager::SLOT_COUNT; i++)
	{
		float y = slotY(i);
		u32 border = (deleteArmed == i) ? colRed : (selected == i) ? colOrange : colBrown;
		// Border is just a bigger rect underneath the cream panel
		C2D_DrawRectSolid(SLOT_X - 3.0f, y - 3.0f, 0.1f, SLOT_W + 6.0f, SLOT_H + 6.0f, border);
		C2D_DrawRectSolid(SLOT_X, y, 0.2f, SLOT_W, SLOT_H, colCream);

		C2D_DrawText(&slotTitleText[i], C2D_WithColor, SLOT_X + 14.0f, y + 6.0f, 0.5f, 0.55f, 0.55f, colText);
		if (deleteArmed == i)
			C2D_DrawText(&deleteText, C2D_WithColor, SLOT_X + 14.0f, y + 28.0f, 0.5f, 0.45f, 0.45f, colRed);
		else
			C2D_DrawText(&slotInfoText[i], C2D_WithColor, SLOT_X + 14.0f, y + 28.0f, 0.5f, 0.45f, 0.45f, colTextSoft);
	}

	C2D_TextGetDimensions(&hintText, 0.45f, 0.45f, &tw, &th);
	C2D_DrawText(&hintText, C2D_WithColor, (SCREEN_WIDTH_BOTTOM - tw) / 2.0f, 218.0f, 0.5f, 0.45f, 0.45f, colTextSoft);

	return PAPAS_OK;
}

PapasError Papas::SaveSelect::terminate()
{
	if (sheet_bg)
	{
		C2D_SpriteSheetFree(sheet_bg);
		sheet_bg = nullptr;
	}
	if (sheet_receipt)
	{
		C2D_SpriteSheetFree(sheet_receipt);
		sheet_receipt = nullptr;
	}
	if (textBuf)
	{
		C2D_TextBufDelete(textBuf);
		textBuf = nullptr;
	}
	if (dokyo)
	{
		C2D_FontFree(dokyo);
		dokyo = nullptr;
	}

	return PAPAS_OK;
}

PapasError Papas::HelpScreen::init(Papas::SceneManager *sceneManager)
{
	p_sceneManager = sceneManager;

	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);
	bottom_bg = C2D_SpriteSheetGetImage(sheet_bg, 0);

	dokyo = C2D_FontLoad("romfs:/fonts/Dokyo.bcfnt");
	book.init(&dokyo);

	return PAPAS_OK;
}

PapasError Papas::HelpScreen::update()
{
	hidScanInput();
	hidTouchRead(&touch);
	u32 kDown = hidKeysDown();

	if (kDown & (KEY_B | KEY_START | KEY_SELECT))
	{
		p_sceneManager->changeScene(new Papas::MainMenu());
		return PAPAS_OK; // changeScene deleted us; touch nothing else
	}

	if (kDown & (KEY_R | KEY_DRIGHT | KEY_DDOWN | KEY_A)) book.turnPage(1);
	if (kDown & (KEY_L | KEY_DLEFT | KEY_DUP)) book.turnPage(-1);
	if (kDown & KEY_TOUCH)
	{
		int row = HelpBook::rowAt(touch);
		if (row >= 0) book.setPage(row);
	}

	return PAPAS_OK;
}

PapasError Papas::HelpScreen::render_top()
{
	Papas::Stereo::plane(0.8f);
	C2D_DrawImageAt(top_bg, 0, 0, 0, NULL, 1, 1);
	Papas::Stereo::plane(-0.1f);
	book.renderTop();

	return PAPAS_OK;
}

PapasError Papas::HelpScreen::render_bottom()
{
	C2D_DrawImageAt(bottom_bg, 0, 0, 0, NULL, 1, 1);
	book.renderBottom();

	return PAPAS_OK;
}

PapasError Papas::HelpScreen::terminate()
{
	book.terminate();
	if (sheet_bg)
	{
		C2D_SpriteSheetFree(sheet_bg);
		sheet_bg = nullptr;
	}
	if (dokyo)
	{
		C2D_FontFree(dokyo);
		dokyo = nullptr;
	}

	return PAPAS_OK;
}

PapasError Papas::CreditsScreen::init(Papas::SceneManager *sceneManager)
{
	p_sceneManager = sceneManager;

	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);
	bottom_bg = C2D_SpriteSheetGetImage(sheet_bg, 0);
	logo = C2D_SpriteSheetGetImage(sheet_bg, 2);

	dokyo = C2D_FontLoad("romfs:/fonts/Dokyo.bcfnt");
	textBuf = C2D_TextBufNew(512);

	static const char *CREDITS[] = {
		"Papa's Pizzeria",
		"originally by Flipline Studios",
		"",
		"3DS port made by zylonity",
		"for my beautiful gorgeous wife",
		"",
		"art and audio remain Flipline's",
	};
	lineCount = (int)(sizeof(CREDITS) / sizeof(CREDITS[0]));
	for (int i = 0; i < lineCount; i++)
	{
		C2D_TextFontParse(&lines[i], dokyo, textBuf, CREDITS[i]);
		C2D_TextOptimize(&lines[i]);
	}
	C2D_TextFontParse(&hintText, dokyo, textBuf, "B Back");
	C2D_TextOptimize(&hintText);

	return PAPAS_OK;
}

PapasError Papas::CreditsScreen::update()
{
	hidScanInput();
	u32 kDown = hidKeysDown();

	if (kDown & (KEY_A | KEY_B | KEY_START | KEY_SELECT | KEY_TOUCH))
	{
		p_sceneManager->changeScene(new Papas::MainMenu());
		return PAPAS_OK; // changeScene deleted us; touch nothing else
	}

	return PAPAS_OK;
}

// Logo up top, the words down on the touch screen where they're easier to read
PapasError Papas::CreditsScreen::render_top()
{
	Papas::Stereo::plane(0.8f);
	C2D_DrawImageAt(top_bg, 0, 0, 0, NULL, 1, 1);

	Papas::Stereo::plane(-0.1f);
	float scaling = 0.7f;
	C2D_DrawImageAt(logo, (SCREEN_WIDTH_TOP / 2) - ((logo.subtex->width * scaling) / 2),
		(SCREEN_HEIGHT_TOP / 2) - ((logo.subtex->height * scaling) / 2), 0.2f, NULL, scaling, scaling);

	return PAPAS_OK;
}

PapasError Papas::CreditsScreen::render_bottom()
{
	C2D_DrawImageAt(bottom_bg, 0, 0, 0, NULL, 1, 1);

	const u32 colInk = C2D_Color32(39, 42, 35, 255);
	C2D_DrawRectSolid(18.0f, 44.0f, 0.90f, 284.0f, 150.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(22.0f, 48.0f, 0.91f, 276.0f, 142.0f, C2D_Color32(244, 239, 218, 255));

	// Centre the block in the napkin so adding a line doesn't push it off the bottom
	static const float PANEL_TOP = 48.0f;
	static const float PANEL_HEIGHT = 142.0f;
	static const float LINE_STEP = 19.0f;
	static const float LINE_HEIGHT = 15.0f;
	float blockHeight = (lineCount - 1) * LINE_STEP + LINE_HEIGHT;
	float firstLineY = PANEL_TOP + (PANEL_HEIGHT - blockHeight) * 0.5f;

	for (int i = 0; i < lineCount; i++)
		drawTextCentered(&lines[i], SCREEN_WIDTH_BOTTOM * 0.5f, firstLineY + i * LINE_STEP, 0.94f, 0.5f, colInk);

	drawTextCentered(&hintText, SCREEN_WIDTH_BOTTOM * 0.5f, 210.0f, 0.5f, 0.45f,
		C2D_Color32(120, 105, 90, 255));

	return PAPAS_OK;
}

PapasError Papas::CreditsScreen::terminate()
{
	if (sheet_bg)
	{
		C2D_SpriteSheetFree(sheet_bg);
		sheet_bg = nullptr;
	}
	if (textBuf)
	{
		C2D_TextBufDelete(textBuf);
		textBuf = nullptr;
	}
	if (dokyo)
	{
		C2D_FontFree(dokyo);
		dokyo = nullptr;
	}

	return PAPAS_OK;
}

PapasError Papas::IntroCutscene::init(Papas::SceneManager *sceneManager)
{

	p_sceneManager = sceneManager;

	sheet_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	skip_bg = C2D_SpriteSheetGetImage(sheet_bg, 3);

	static_assert(INTRO_SHEET_COUNT <= 4, "grow IntroCutscene::introSheets");
	for (int i = 0; i < INTRO_SHEET_COUNT; i++)
	{
		char path[40];
		std::snprintf(path, sizeof(path), "romfs:/gfx/intro_%d.t3x", i + 1);
		introSheets[i] = C2D_SpriteSheetLoad(path);
		for (size_t j = 0; j < C2D_SpriteSheetCount(introSheets[i]); j++)
			C3D_TexSetFilter(C2D_SpriteSheetGetImage(introSheets[i], j).tex, GPU_LINEAR, GPU_LINEAR);
	}

	// Layer tints are Flash colour-transform multipliers
	C2D_SetTintMode(C2D_TintMult);

	Papas::ResourceManager::getInstance().loadSong("gameintro", "romfs:/music/gameintro.ogg");
	Papas::ResourceManager::getInstance().playMusicOnce("gameintro");

	startedAt = osGetTime();

	return PAPAS_OK;
}

PapasError Papas::IntroCutscene::update()
{

	hidScanInput();

	// Respond to user input
	u32 kDown = hidKeysDown();
	if (kDown & (KEY_START | KEY_SELECT))
		return PAPAS_EXIT_REQUESTED;

	u64 elapsed = osGetTime() - startedAt;
	u64 duration = (u64)INTRO_SRC_FRAMES * 1000 / INTRO_FPS;

	// Handle skipping here because render_top runs once per eye.
	if ((kDown & KEY_B) || elapsed >= duration)
	{
		p_sceneManager->changeScene(new Papas::Game());
		return PAPAS_OK; // changeScene deleted us; touch nothing else
	}

	return PAPAS_OK;
}

// Centre the clip's 320x240 stage in the 400x240 top screen.
static const float INTRO_X = 200.0f;
static const float INTRO_Y = 120.0f;

// Draw one transformed layer without losing the active stereo view.
static void drawIntroLayer(C2D_SpriteSheet *sheets, const IntroDraw &d,
						   const IntroDraw *next, float t, float depth)
{
	float a = d.a, b = d.b, c = d.c, dm = d.d, tx = d.tx, ty = d.ty;
	float tint[4] = {(float)d.tint[0], (float)d.tint[1],
					 (float)d.tint[2], (float)d.tint[3]};
	if (next != nullptr)
	{
		a += (next->a - a) * t;   b += (next->b - b) * t;
		c += (next->c - c) * t;   dm += (next->d - dm) * t;
		tx += (next->tx - tx) * t; ty += (next->ty - ty) * t;
		for (int i = 0; i < 4; i++)
			tint[i] += ((float)next->tint[i] - tint[i]) * t;
	}

	C2D_Image img = C2D_SpriteSheetGetImage(sheets[d.sheet], d.index);

	Papas::Stereo::plane((float)d.plane / 100.0f);

	C3D_Mtx saved;
	C2D_ViewSave(&saved);
	C2D_Flush();

	C3D_Mtx m;
	memset(&m, 0, sizeof(m));
	m.r[0].x = a; m.r[0].y = c; m.r[0].w = tx + INTRO_X;
	m.r[1].x = b; m.r[1].y = dm; m.r[1].w = ty + INTRO_Y;
	m.r[2].z = 1;
	m.r[3].w = 1;
	C3D_Mtx composed;
	Mtx_Multiply(&composed, &saved, &m);
	C2D_ViewRestore(&composed);

	C2D_DrawParams p = {
		{0.0f, 0.0f, (float)img.subtex->width, (float)img.subtex->height},
		{0.0f, 0.0f},
		depth,
		0.0f
	};
	if (tint[0] < 254.5f || tint[1] < 254.5f || tint[2] < 254.5f || tint[3] < 254.5f)
	{
		C2D_ImageTint ti;
		C2D_PlainImageTint(&ti, C2D_Color32((u8)tint[0], (u8)tint[1],
											(u8)tint[2], (u8)tint[3]), 1.0f);
		C2D_DrawImage(img, &p, &ti);
	}
	else
	{
		C2D_DrawImage(img, &p, nullptr);
	}
	C2D_Flush();

	C2D_ViewRestore(&saved);
}

PapasError Papas::IntroCutscene::render_top()
{

	u64 elapsed = osGetTime() - startedAt;
	float srcFrame = (float)elapsed * INTRO_FPS / 1000.0f;

	static const int N_SAMPLES = (int)(sizeof(INTRO_FRAMES) / sizeof(INTRO_FRAMES[0]));
	int idx = (int)(srcFrame / INTRO_SAMPLE_STEP);
	if (idx > N_SAMPLES - 1) idx = N_SAMPLES - 1;
	const IntroFrame &f0 = INTRO_FRAMES[idx];
	const IntroFrame *f1 = idx + 1 < N_SAMPLES ? &INTRO_FRAMES[idx + 1] : nullptr;

	float t = 0.0f;
	if (f1 != nullptr)
	{
		t = (srcFrame - (float)f0.srcFrame) / (float)(f1->srcFrame - f0.srcFrame);
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;
	}

	for (u16 i = 0; i < f0.count; i++)
	{
		const IntroDraw &d = INTRO_DRAWS[f0.first + i];
		// Match the same placement in the next sample to lerp its motion
		const IntroDraw *nd = nullptr;
		if (f1 != nullptr)
		{
			for (u16 j = 0; j < f1->count; j++)
			{
				if (INTRO_DRAWS[f1->first + j].run == d.run)
				{
					nd = &INTRO_DRAWS[f1->first + j];
					break;
				}
			}
		}
		drawIntroLayer(introSheets, d, nd, t, 0.1f + (float)i * 0.002f);
	}

	// Mask oversized layers to the original 320px-wide Flash stage.
	{
		Papas::Stereo::plane(0.0f);

		C3D_Mtx saved;
		C2D_ViewSave(&saved);
		C2D_Flush();

		C3D_Mtx m;
		Mtx_Identity(&m);
		m.r[0].w = INTRO_X;
		m.r[1].w = INTRO_Y;
		C3D_Mtx composed;
		Mtx_Multiply(&composed, &saved, &m);
		C2D_ViewRestore(&composed);

		const u32 black = C2D_Color32(0x00, 0x00, 0x00, 0xff);
		C2D_DrawRectSolid(-200.0f, -120.0f, 0.9f, 45.0f, 280.0f, black); // left of stage
		C2D_DrawRectSolid(200.0f, -120.0f, 0.9f, -45.0f, 280.0f, black); // right of stage
		// Cover the bright antialiasing fringe at the stage edges.
		C2D_DrawRectSolid(-160.0f, -120.0f, 0.9f, 320.0f, 2.0f, black);
		C2D_DrawRectSolid(-160.0f, 118.0f, 0.9f, 320.0f, 2.0f, black);
		C2D_Flush();

		C2D_ViewRestore(&saved);
	}

	return PAPAS_OK;
}

PapasError Papas::IntroCutscene::render_bottom()
{
	C2D_DrawImageAt(skip_bg, 0, 0, 0, NULL, 1, 1);

	return PAPAS_OK;
}

void Papas::IntroCutscene::freeIntroSheets()
{
	for (int i = 0; i < INTRO_SHEET_COUNT; i++)
	{
		if (introSheets[i] != nullptr)
		{
			C2D_SpriteSheetFree(introSheets[i]);
			introSheets[i] = nullptr;
		}
	}
}

PapasError Papas::IntroCutscene::terminate()
{
	C2D_SetTintMode(C2D_TintSolid);
	// Just hush it; shutting the mixer now would bin the chunks Game::init wants
	Papas::ResourceManager::getInstance().stopMusic();

	freeIntroSheets();
	if (sheet_bg)
	{
		C2D_SpriteSheetFree(sheet_bg);
		sheet_bg = nullptr;
	}


	return PAPAS_OK;
}

// Time Roy's scribble by the clock so stereo's second draw can't speed it up.
static const float ROY_ORDER_FRAME_MS = 1000.0f / 60.0f;
// Idle between lines, as in the original (TakeOrderScreen.as lineIntervalSpeed)
static const float ROY_ORDER_PAUSE_MS = 2000.0f;

PapasError Papas::Game::init(Papas::SceneManager *sceneManager)
{


	Papas::ResourceManager::getInstance().initMusicPlayer();
	Papas::ResourceManager::getInstance().loadSfx("writepencil", "romfs:/sfx/writepencil.wav");
	
	takingOrder = false;
	s_stations = C2D_SpriteSheetLoad("romfs:/gfx/stations.t3x");

	ticketsStationImg = C2D_SpriteSheetGetImage(s_stations, 8);
	ticketsHolderImg = C2D_SpriteSheetGetImage(s_stations, 9);
	signOpenImg = C2D_SpriteSheetGetImage(s_stations, 10);
	signClosedImg = C2D_SpriteSheetGetImage(s_stations, 11);
	coalsDimImg = C2D_SpriteSheetGetImage(s_stations, 12);
	coalsBrightImg = C2D_SpriteSheetGetImage(s_stations, 13);
	// Filter this atlas for its scaled sign and coal tiles; 1:1 art stays crisp.
	C3D_TexSetFilter(signOpenImg.tex, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetFilter(signClosedImg.tex, GPU_LINEAR, GPU_LINEAR);

	
	orderStation = C2D_SpriteSheetLoad("romfs:/gfx/taking_order.t3x");
	to_counter = C2D_SpriteSheetGetImage(orderStation, 0);
	to_wallpaper = C2D_SpriteSheetGetImage(orderStation, 1);
	orderBubbleSheet = C2D_SpriteSheetLoad("romfs:/gfx/order_bubble.t3x");
	orderBubbleBase = C2D_SpriteSheetGetImage(orderBubbleSheet, 0);
	for (int i = 0; i < 7; i++) orderBubbleToppings[i] = C2D_SpriteSheetGetImage(orderBubbleSheet, i + 1);
	for (int i = 0; i < 16; i++) orderBubbleCoverage[i] = C2D_SpriteSheetGetImage(orderBubbleSheet, i + 8);
	for (int i = 0; i < 8; i++) orderBubbleClocks[i] = C2D_SpriteSheetGetImage(orderBubbleSheet, i + 24);
	for (int i = 0; i < 3; i++) orderBubbleCuts[i] = C2D_SpriteSheetGetImage(orderBubbleSheet, i + 32);
	for (int i = 0; i < 15; i++) orderBubbleOpening[i] = C2D_SpriteSheetGetImage(orderBubbleSheet, i + 35);
	for (size_t i = 0; i < C2D_SpriteSheetCount(orderBubbleSheet); i++)
		C3D_TexSetFilter(C2D_SpriteSheetGetImage(orderBubbleSheet, i).tex, GPU_LINEAR, GPU_LINEAR);

	v2 pos = {-24, 45};
	v2 scl = {0.8f, 0.8f};
	Roy.createAnim("romfs:/gfx/Roy_peeking.t3x", 0.0f, pos, scl, 35.0f);

	v2 pos_order = {-45, 45};
	v2 scl_order = {1.0f, 1.0f};
	Roy2.createAnim("romfs:/gfx/Roy_takingorder.t3x", ROY_ORDER_FRAME_MS, pos_order, scl_order, 0.0f);

	// Receipt system stuff to move later
	dokyo = C2D_FontLoad("romfs:/fonts/Dokyo.bcfnt");
	resultTextBuf = C2D_TextBufNew(256);
	orderBubbleTextBuf = C2D_TextBufNew(32);
	resultSheet = C2D_SpriteSheetLoad("romfs:/gfx/results.t3x");
	resultJar = C2D_SpriteSheetGetImage(resultSheet, 0);
	for (int i = 0; i < 10; i++) resultCoinPiles[i] = C2D_SpriteSheetGetImage(resultSheet, i + 1);
	for (int i = 0; i < 6; i++) resultCoinSpin[i] = C2D_SpriteSheetGetImage(resultSheet, i + 11);
	for (size_t i = 0; i < C2D_SpriteSheetCount(resultSheet); i++)
		C3D_TexSetFilter(C2D_SpriteSheetGetImage(resultSheet, i).tex, GPU_LINEAR, GPU_LINEAR);
	Papas::ResourceManager::getInstance().loadSfx("customer_decent", "romfs:/sfx/customer_decent.wav");
	Papas::ResourceManager::getInstance().loadSfx("customer_happy", "romfs:/sfx/customer_happy.wav");
	Papas::ResourceManager::getInstance().loadSfx("customer_overjoyed", "romfs:/sfx/customer_overjoyed.wav");
	Papas::ResourceManager::getInstance().loadSfx("customer_pissed", "romfs:/sfx/customer_pissed.wav");
	Papas::ResourceManager::getInstance().loadSfx("customer_upset", "romfs:/sfx/customer_upset.wav");
	Papas::ResourceManager::getInstance().loadSfx("customer_worried", "romfs:/sfx/customer_worried.wav");
	Papas::ResourceManager::getInstance().loadSfx("giveorder_drumroll", "romfs:/sfx/giveorder_drumroll.wav");
	Papas::ResourceManager::getInstance().loadSfx("singlecoin", "romfs:/sfx/singlecoin.wav");
	Papas::ResourceManager::getInstance().loadSfx("multicoin", "romfs:/sfx/multicoin.wav");
	Papas::ResourceManager::getInstance().loadSfx("talkbubble", "romfs:/sfx/talkbubble.wav");
	// Star-customer art + sound (star row on the take-order/result counters)
	starsSheet = C2D_SpriteSheetLoad("romfs:/gfx/stars.t3x");
	starEmptyImg = C2D_SpriteSheetGetImage(starsSheet, 0);
	starFilledImg = C2D_SpriteSheetGetImage(starsSheet, 1);
	starFlashImg = C2D_SpriteSheetGetImage(starsSheet, 2);
	sealImg = C2D_SpriteSheetGetImage(starsSheet, 3);
	for (size_t i = 0; i < C2D_SpriteSheetCount(starsSheet); i++)
		C3D_TexSetFilter(C2D_SpriteSheetGetImage(starsSheet, i).tex, GPU_LINEAR, GPU_LINEAR);
	Papas::ResourceManager::getInstance().loadSfx("getstar", "romfs:/sfx/getstar.wav");
	nameTextBuf = C2D_TextBufNew(32);
	resultCustomerType = 0;
	resultStarsBefore = 0;
	resultSealsBefore = 0;
	resultStarEarned = 0;
	resultStarsLost = false;
	resultSealEarned = false;
	resultStarSfxPlayed = false;

	showingResult = false;
	resultTouchHeld = false;
	totalScore = 0;
	resultCustomerNumber = 0;
	giveOrderLoaded = false;
	giveOrderPick = 0;
	for (int i = 0; i < GIVEORDER_SHEET_COUNT; i++) giveOrderSheets[i] = nullptr;

	r_manager.initManager(&dokyo);

	createReceipt.createButton(orderStation, 2, 2, 3, {34, 141});

	// Run the storefront intro before creating the customer manager.
	Papas::ResourceManager::getInstance().loadSfx("startofday", "romfs:/sfx/startofday.wav");
	// Fresh and existing slots can share this setup path.
	const SaveData &saved = SaveManager::getInstance().data;
	currentDay = saved.day;
	myRank = saved.rank;
	lastRankLimit = saved.lastRankLimit;
	totalTipsCents = saved.totalTipsCents;
	// Loaded with the rest of the day intro, below
	startOfDaySheet = nullptr;
	dayNumberSheet = nullptr;

	showingNewCustomer = false;
	newCustomerNoPapa = false;
	newCustomerType = 0;
	newCustomerSheet = nullptr;
	newCustomerNameScale = 1.0f;
	newCustomerTextBuf = C2D_TextBufNew(32);
	Papas::ResourceManager::getInstance().loadSfx("endofday", "romfs:/sfx/endofday.wav");
	Papas::ResourceManager::getInstance().loadSfx("buttonclick", "romfs:/sfx/buttonclick.wav");
	Papas::ResourceManager::getInstance().loadSfx("grabticket", "romfs:/sfx/grabticket.wav");
	Papas::ResourceManager::getInstance().loadSfx("dropticket", "romfs:/sfx/dropticket.wav");
	Papas::ResourceManager::getInstance().loadSfx("cash", "romfs:/sfx/cash.wav");
	Papas::ResourceManager::getInstance().loadSong("takeorder_music", "romfs:/music/takeorder_music.ogg");
	Papas::ResourceManager::getInstance().loadSong("justguitar_music", "romfs:/music/justguitar_music.ogg");

	// Pause overlays, the customer file, and the day's running tally
	overlay = OverlayNone;
	pauseIndex = 0;
	pauseSheet = nullptr;
	fileSelected = 0;
	uiTextBuf = C2D_TextBufNew(512);
	fileTextBuf = C2D_TextBufNew(512);
	helpBook.init(&dokyo);
	showingEndOfDay = false;
	endDayPhase = EndDayBoard;
	endDayRankUp = false;
	endDayTextBuf = C2D_TextBufNew(768);
	customersToday = 0;
	waitingToday = toppingToday = bakingToday = cuttingToday = 0;
	tipsTodayCents = 0;

	beginDayOrNewCustomer();

	SwitchStation(TicketStation);

	to_firstRun = false;
	to_currentAction = 0;
	to_bubbleKind = BubbleHidden;

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

// Dock 0.3% per second for a late order, and again for every second they queued too long.
int Papas::Game::scoreWaiting(const Pizza &pizza, const CustomerData &order) const
{
	(void)order;	// the notch lives on the ticket now, since the last customer's gets cut
	if (pizza.ticket == nullptr || pizza.ticket->orderStartedAt == 0) return 100;
	const Receipt &ticket = *pizza.ticket;

	// Fair turnaround: the bake itself, our prep window, plus slack per pickup already queued
	float bakeMs = ticket.cookNotch * 45.0f * MS_PER_COOK_DEGREE;
	float queueMs = ticket.ordersAheadOfMe * (COOK_TIME_MS * 0.5f) / 4.0f;
	float expectedMs = bakeMs + PREP_TIME_PER_ORDER_MS + queueMs;

	float elapsedMs = (float)(Papas::Clock::now() - ticket.orderStartedAt);
	float latePercent = std::max(0.0f, elapsedMs - expectedMs) / 1000.0f * 0.3f;
	// Standing in the queue counts against us too, once past the ideal wait
	float linePercent = std::max(0.0f, (float)ticket.lineWaitMs - IDEAL_LINE_WAIT_MS) / 1000.0f * 0.3f;

	return std::max(0, (int)std::floor(100.0f - latePercent - linePercent));
}

// How close the dial got to the ticket's notch, 90 degrees off = 0
int Papas::Game::scoreBaking(const Pizza &pizza, const CustomerData &order) const
{
	(void)order;
	int notch = pizza.ticket != nullptr ? pizza.ticket->cookNotch : order.time;
	float difference = std::abs(notch * 45.0f - pizza.cookDegrees);
	return std::max(0, (int)std::floor(100.0f - difference / 90.0f * 100.0f));
}

// Score topping quantity, coverage, spread, and unwanted items.
int Papas::Game::scoreToppings(const Pizza &pizza, const CustomerData &order) const
{
	// What the receipt wants, folded down per topping type
	int targetQuantity[7] = {};
	int targetCoverage[7][4] = {};
	for (size_t i = 0; i < order.items.size(); i++)
	{
		int type = order.items[i].Topping;
		targetQuantity[type] += order.items[i].Quantity;
		for (int q = 0; q < 4; q++)
			targetCoverage[type][q] = std::max(targetCoverage[type][q], order.items[i].Coverage[q]);
	}

	// What's actually on the pizza, counted per quadrant
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

	// Score each wanted topping, then average and knock off for extras
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

// Score cut count, angle spread, and whether each cut crosses the pizza.
int Papas::Game::scoreCutting(const Pizza &pizza, const CustomerData &order) const
{
	int expectedCuts = order.CutPizzaIn / 2;	// 4/6/8 slices = 2/3/4 full cuts
	if (expectedCuts <= 0) return pizza.cuts.empty() ? 100 : 0;
	std::vector<float> idealAngles;
	for (int i = 0; i < expectedCuts; i++)
		idealAngles.push_back(i * (180.0f / expectedCuts));

	// Greedily match each ideal angle to the closest unused cut
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

	// Short chords score badly, full-diameter cuts score 100
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

// Centre the baked day digits on the plate, drawing their shadow first.
void Papas::Game::renderDayNumber(float centreX, float inkCentreY)
{
	if (dayNumberSheet == nullptr) return;

	int digits[8];
	int count = 0;
	int value = currentDay > 0 ? currentDay : 1;
	while (value > 0 && count < 8)
	{
		digits[count++] = value % 10;
		value /= 10;
	}

	float total = 0.0f;
	for (int i = 0; i < count; i++) total += DAYNUM_ADVANCE[digits[i]];

	C2D_ImageTint shadowTint, inkTint;
	C2D_PlainImageTint(&shadowTint, C2D_Color32(178, 178, 172, 255), 1.0f);
	C2D_PlainImageTint(&inkTint, C2D_Color32(58, 58, 58, 255), 1.0f);

	// Pixel-snap these final-size digits so they stay crisp.
	float pen = roundf(centreX - total * 0.5f) - DAYNUM_PAD;
	float y = roundf(inkCentreY - DAYNUM_INK_H * 0.5f) - DAYNUM_PAD;
	for (int i = count; i-- > 0; )
	{
		C2D_Image digit = dayDigits[digits[i]];
		C2D_DrawImageAt(digit, roundf(pen) + 2.0f, y + 2.0f, 0.34f, &shadowTint);
		C2D_DrawImageAt(digit, roundf(pen), y, 0.35f, &inkTint);
		pen += DAYNUM_ADVANCE[digits[i]];
	}
}

void Papas::Game::renderDayIntro()
{
	static const float SOD_X = 38.0f; // centre the 323px scene on the top screen
	static const float SOD_W = 323.0f;
	// Keep the storefront deep and slide the day plate in front.
	static const float SOD_PLANE_SCENE = 1.0f;
	static const float SOD_PLANE_DOOR = 0.9f;
	static const float SOD_PLANE_PLATE = -0.2f;
	static const u32 SOD_BLACK = C2D_Color32(0, 0, 0, 255);

	// Fill behind the opaque clip so stereo shifts don't reveal the clear colour.
	Papas::Stereo::plane(0.0f);
	C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 400.0f, 240.0f, SOD_BLACK);
	Papas::Stereo::plane(SOD_PLANE_SCENE);
	s64 elapsed = (s64)(Papas::Clock::now() - dayIntroStartedAt);
	if (elapsed < 0) elapsed = 0;
	float srcFrame = 1.0f + (float)elapsed * STARTOFDAY_FPS / 1000.0f;
	if (srcFrame > (float)STARTOFDAY_SRC_FRAMES) srcFrame = (float)STARTOFDAY_SRC_FRAMES;

	C2D_DrawImageAt(C2D_SpriteSheetGetImage(startOfDaySheet, 0), SOD_X, 0.0f, 0.10f);
	while (dayIntroPick + 1 < STARTOFDAY_PATCH_COUNT &&
	       (float)STARTOFDAY_PATCHES[dayIntroPick + 1].srcFrame <= srcFrame)
		dayIntroPick++;
	const StartOfDayPatch &patch = STARTOFDAY_PATCHES[dayIntroPick];
	Papas::Stereo::plane(SOD_PLANE_DOOR);
	C2D_DrawImageAt(C2D_SpriteSheetGetImage(startOfDaySheet, patch.index),
		SOD_X + patch.ox, (float)patch.oy, 0.20f);

	// Day plate slides on its per-frame track; lerp between source frames
	int fi = (int)srcFrame;
	if (fi < 1) fi = 1;
	if (fi > STARTOFDAY_SRC_FRAMES) fi = STARTOFDAY_SRC_FRAMES;
	float y0 = STARTOFDAY_PLATE_TRACK[fi - 1];
	float y1 = fi < STARTOFDAY_SRC_FRAMES ? STARTOFDAY_PLATE_TRACK[fi] : y0;
	float plateY = y0 + (y1 - y0) * (srcFrame - (float)fi);
	C2D_Image plate = C2D_SpriteSheetGetImage(startOfDaySheet, 1);
	Papas::Stereo::plane(SOD_PLANE_PLATE);
	C2D_DrawImageAt(plate, SOD_X + STARTOFDAY_PLATE_X, plateY, 0.30f);

	// Put the real day number in the sample number's cleaned-out spot.
	static const float SOD_DIGIT_DROP = 3.0f;
	renderDayNumber(SOD_X + STARTOFDAY_PLATE_X + plate.subtex->width * 0.5f,
		plateY + STARTOFDAY_DIGIT_Y + SOD_DIGIT_DROP);

	// The eye shift slides the scene past its edges, so re-mask the borders
	Papas::Stereo::plane(0.0f);
	C2D_DrawRectSolid(0.0f, 0.0f, 0.5f, SOD_X, 240.0f, SOD_BLACK);
	C2D_DrawRectSolid(SOD_X + SOD_W, 0.0f, 0.5f, 400.0f - (SOD_X + SOD_W), 240.0f, SOD_BLACK);
}

void Papas::Game::beginDayIntro()
{
	if (startOfDaySheet == nullptr)
	{
		startOfDaySheet = C2D_SpriteSheetLoad("romfs:/gfx/startofday.t3x");
		for (size_t i = 0; i < C2D_SpriteSheetCount(startOfDaySheet); i++)
			C3D_TexSetFilter(C2D_SpriteSheetGetImage(startOfDaySheet, i).tex, GPU_LINEAR, GPU_LINEAR);
	}
	if (dayNumberSheet == nullptr)
	{
		// Drawn 1:1, so these keep the sheet's default nearest filtering
		dayNumberSheet = C2D_SpriteSheetLoad("romfs:/gfx/daynumber.t3x");
		for (int i = 0; i < 10; i++)
			dayDigits[i] = C2D_SpriteSheetGetImage(dayNumberSheet, i);
	}
	showingDayIntro = true;
	dayIntroStartedAt = Papas::Clock::now();
	dayIntroPick = 0;
	Papas::ResourceManager::getInstance().playSfx("startofday");
}

void Papas::Game::endDayIntro()
{
	showingDayIntro = false;
	if (startOfDaySheet != nullptr)
	{
		C2D_SpriteSheetFree(startOfDaySheet);
		startOfDaySheet = nullptr;
	}
	if (dayNumberSheet != nullptr)
	{
		C2D_SpriteSheetFree(dayNumberSheet);
		dayNumberSheet = nullptr;
	}
	Papas::ResourceManager::getInstance().switchMusic("orders_music");
	// Spawn today's customers (loads the rig + per-type atlases on demand)
	c_manager.initManager(myRank, currentDay);
}

// NEW CUSTOMER! splash, using the art and tracks from gen_newcustomer.py.

// Picks up the frame's entry from a track that holds its last value
static int newCustomerTrackIndex(float srcFrame, int frames)
{
	int index = (int)srcFrame - 1;
	if (index < 0) index = 0;
	if (index >= frames) index = frames - 1;
	return index;
}

bool Papas::Game::beginNewCustomer()
{
	newCustomerNoPapa = CustomerManager::papaBlocked(myRank);
	newCustomerType = newCustomerNoPapa ? 0 : CustomerManager::newCustomerToday(myRank);
	if (!newCustomerNoPapa && newCustomerType == 0) return false;

	newCustomerSheet = C2D_SpriteSheetLoad("romfs:/gfx/newcustomer.t3x");
	if (newCustomerSheet == nullptr) return false;
	for (size_t i = 0; i < C2D_SpriteSheetCount(newCustomerSheet); i++)
		C3D_TexSetFilter(C2D_SpriteSheetGetImage(newCustomerSheet, i).tex, GPU_LINEAR, GPU_LINEAR);

	if (!newCustomerNoPapa)
	{
		// The close-up limb art best matches this screen's 0.43 scale.
		CustomerRig::getInstance().load();
		CustomerRig::getInstance().loadType(newCustomerType, newCustomerAtlas);

		C2D_TextBufClear(newCustomerTextBuf);
		std::unordered_map<int, CustomerData>::const_iterator entry =
			map_customers.find(newCustomerType);
		C2D_TextFontParse(&newCustomerNameText, dokyo, newCustomerTextBuf,
			entry != map_customers.end() ? entry->second.name.c_str() : "");
		C2D_TextOptimize(&newCustomerNameText);
		// Match the original field's 48px type rather than guess a scale
		float naturalHeight = 0.0f;
		C2D_TextGetDimensions(&newCustomerNameText, 1.0f, 1.0f, nullptr, &naturalHeight);
		newCustomerNameScale = naturalHeight > 0.0f
			? NEWCUSTOMER_NAME_HEIGHT / naturalHeight : 1.0f;

		// Count them as introduced as soon as the splash is built.
		SaveManager &saveManager = SaveManager::getInstance();
		saveManager.data.customerMet[newCustomerType] = 1;
		saveManager.save();
	}

	Papas::ResourceManager::getInstance().stopMusic();
	Papas::ResourceManager::getInstance().playSfx(
		newCustomerNoPapa ? "endofday" : "customer_overjoyed");
	showingNewCustomer = true;
	newCustomerStartedAt = Papas::Clock::now();
	return true;
}

void Papas::Game::renderNewCustomer()
{
	s64 elapsed = (s64)(Papas::Clock::now() - newCustomerStartedAt);
	if (elapsed < 0) elapsed = 0;
	float srcFrame = 1.0f + (float)elapsed * NEWCUSTOMER_FPS / 1000.0f;

	Papas::Stereo::plane(1.0f);
	C2D_DrawImageAt(C2D_SpriteSheetGetImage(newCustomerSheet,
		newCustomerNoPapa ? NC_IMG_NOPAPA_BG : NC_IMG_BG), 0.0f, 0.0f, 0.0f);

	if (newCustomerNoPapa)
	{
		// Hide the seal and shadow until their entrance begins.
		int index = (int)srcFrame - NEWCUSTOMER_SEAL_START;
		if (index >= 0)
		{
			const NewCustomerBox &shadow = NEWCUSTOMER_NOPAPA_SHADOW[
				std::min(index, NEWCUSTOMER_NOPAPA_SHADOW_FRAMES - 1)];
			C2D_Image shadowImg = C2D_SpriteSheetGetImage(newCustomerSheet, NC_IMG_NOPAPA_SHADOW);
			Papas::Stereo::plane(0.85f);
			C2D_DrawImageAt(shadowImg, shadow.x, shadow.y, 0.1f, nullptr,
				shadow.w / shadowImg.subtex->width, shadow.h / shadowImg.subtex->height);

			const NewCustomerSealFrame &seal = NEWCUSTOMER_SEAL[
				std::min(index, NEWCUSTOMER_SEAL_FRAMES - 1)];
			// The seal is baked at maximum size, so only scale it down.
			Papas::Stereo::plane(0.4f);
			C2D_DrawImageAt(C2D_SpriteSheetGetImage(newCustomerSheet, NC_IMG_SEAL),
				seal.x, seal.y, 0.2f, nullptr, seal.scale, seal.scale);
		}
		return;
	}

	// The disc inflates behind the customer, who is on stage from the start
	const NewCustomerBox &disc = NEWCUSTOMER_DISC[
		newCustomerTrackIndex(srcFrame, NEWCUSTOMER_DISC_FRAMES)];
	Papas::Stereo::plane(0.8f);
	if (disc.w > 0.0f)
	{
		C2D_Image discImg = C2D_SpriteSheetGetImage(newCustomerSheet, NC_IMG_DISC);
		C2D_DrawImageAt(discImg, disc.x, disc.y, 0.1f, nullptr,
			disc.w / discImg.subtex->width, disc.h / discImg.subtex->height);
	}
	C2D_DrawImageAt(C2D_SpriteSheetGetImage(newCustomerSheet, NC_IMG_SHADOW),
		NEWCUSTOMER_SHADOW_X, NEWCUSTOMER_SHADOW_Y, 0.2f);

	// Keep the name on the disc and the customer in front of both.
	float nameWidth = 0.0f;
	C2D_TextGetDimensions(&newCustomerNameText, newCustomerNameScale, newCustomerNameScale,
		&nameWidth, nullptr);
	C2D_DrawText(&newCustomerNameText, C2D_WithColor, NEWCUSTOMER_NAME_CX - nameWidth * 0.5f,
		NEWCUSTOMER_NAME_Y, 0.3f, newCustomerNameScale, newCustomerNameScale,
		C2D_Color32(0, 0, 0, 255));

	Papas::Stereo::plane(0.6f);
	CustomerRig &rig = CustomerRig::getInstance();
	int segment = rig.segmentIndex("overjoyed");
	int frame = rig.frameForTime(segment, (float)elapsed / 1000.0f);
	rig.draw(newCustomerAtlas, newCustomerType, frame,
		NEWCUSTOMER_RIG_X, NEWCUSTOMER_RIG_Y,
		NEWCUSTOMER_RIG_SCALE, NEWCUSTOMER_RIG_SCALE, 0.4f);

	// The title reads as titling over the scene rather than part of it
	Papas::Stereo::plane(-0.1f);
	float titleY = NEWCUSTOMER_TITLE_Y[
		newCustomerTrackIndex(srcFrame, NEWCUSTOMER_TITLE_FRAMES)];
	if (titleY > -900.0f)
	{
		C2D_DrawImageAt(C2D_SpriteSheetGetImage(newCustomerSheet, NC_IMG_TITLE),
			NEWCUSTOMER_TITLE_X, titleY, 0.5f);
	}
}

void Papas::Game::endNewCustomer()
{
	showingNewCustomer = false;
	if (newCustomerSheet != nullptr)
	{
		C2D_SpriteSheetFree(newCustomerSheet);
		newCustomerSheet = nullptr;
	}
	CustomerRig::getInstance().freeType(newCustomerAtlas);
	// endAnimation(): straight into the day it was introducing
	beginDayIntro();
}

// Every day starts here: the splash first if one is due, the intro otherwise
void Papas::Game::beginDayOrNewCustomer()
{
	if (!beginNewCustomer()) beginDayIntro();
}

void Papas::Game::startNextDay()
{
	// The promotion already happened on the end-of-day board, so just turn the page
	showingEndOfDay = false;
	currentDay++;
	customersToday = 0;
	waitingToday = toppingToday = bakingToday = cuttingToday = 0;
	tipsTodayCents = 0;

	// End of day is the save checkpoint, same as the original games
	SaveManager &saveManager = SaveManager::getInstance();
	saveManager.data.day = currentDay;
	saveManager.data.rank = myRank;
	saveManager.data.lastRankLimit = lastRankLimit;
	saveManager.data.totalTipsCents = totalTipsCents;
	saveManager.save();

	Papas::ResourceManager::getInstance().stopMusic();
	beginDayOrNewCustomer();
}

void Papas::Game::loadGiveOrderSheets()
{
	static_assert(GIVEORDER_SHEET_COUNT <= 8, "grow Game::giveOrderSheets");
	if (giveOrderLoaded) return;
	bool ok = true;
	for (int i = 0; i < GIVEORDER_SHEET_COUNT; i++)
	{
		char path[48];
		std::snprintf(path, sizeof(path), "romfs:/gfx/giveorder_roy%d.t3x", i + 1);
		giveOrderSheets[i] = C2D_SpriteSheetLoad(path);
		if (giveOrderSheets[i] == nullptr) { ok = false; break; }
		for (size_t j = 0; j < C2D_SpriteSheetCount(giveOrderSheets[i]); j++)
			C3D_TexSetFilter(C2D_SpriteSheetGetImage(giveOrderSheets[i], j).tex, GPU_LINEAR, GPU_LINEAR);
	}
	if (!ok) freeGiveOrderSheets();
	else giveOrderLoaded = true;
}

void Papas::Game::freeGiveOrderSheets()
{
	for (int i = 0; i < GIVEORDER_SHEET_COUNT; i++)
	{
		if (giveOrderSheets[i] != nullptr) C2D_SpriteSheetFree(giveOrderSheets[i]);
		giveOrderSheets[i] = nullptr;
	}
	giveOrderLoaded = false;
}

void Papas::Game::renderGiveOrderRoy()
{
	// Placement of the scaled 339x249 clip canvas on the top screen
	static const float ROY_X = -55.0f;
	static const float ROY_Y = 20.0f;
	// Glide the steady pose in so the original's carry jiggle doesn't show.
	static const float SLIDE_FROM_X = 10.0f;
	static const float SLIDE_FROM_Y = 42.0f;
	static const float SLIDE_END_FRAME = 18.0f;
	// Guard wall-clock rollback so unsigned timing can't jump to the last frame.
	s64 elapsed = (s64)(Papas::Clock::now() - resultStartedAt);
	if (elapsed < 0) elapsed = 0;
	float srcFrame = 1.0f + (float)elapsed * GIVEORDER_FPS / 1000.0f;
	if (srcFrame > (float)GIVEORDER_SRC_FRAMES) srcFrame = (float)GIVEORDER_SRC_FRAMES;
	if (srcFrame < SLIDE_END_FRAME)
	{
		float t = (srcFrame - 1.0f) / (SLIDE_END_FRAME - 1.0f);
		float ease = 1.0f - (1.0f - t) * (1.0f - t);
		const GiveOrderFrame &hold = GIVEORDER_FRAMES[0];
		float x = SLIDE_FROM_X + ((float)hold.ox - SLIDE_FROM_X) * ease;
		float y = SLIDE_FROM_Y + ((float)hold.oy - SLIDE_FROM_Y) * ease;
		C2D_Image img = C2D_SpriteSheetGetImage(giveOrderSheets[hold.sheet], hold.index);
		C2D_DrawImageAt(img, ROY_X + x, ROY_Y + y, 0.70f);
		return;
	}
	while (giveOrderPick + 1 < GIVEORDER_FRAME_COUNT &&
	       (float)GIVEORDER_FRAMES[giveOrderPick + 1].srcFrame <= srcFrame)
		giveOrderPick++;
	const GiveOrderFrame &frame = GIVEORDER_FRAMES[giveOrderPick];
	C2D_Image img = C2D_SpriteSheetGetImage(giveOrderSheets[frame.sheet], frame.index);
	C2D_DrawImageAt(img, ROY_X + frame.ox, ROY_Y + frame.oy, 0.70f);
}

// Both live further down with the result screen they were written for
static void drawCenteredText(const C2D_Text &text, float y, float scale, u32 color);
static void drawBackdrop(C2D_Image img, float x, float y, float depth);

// Pause menu rows and the customer-file grid, shared by their draw and touch code
static const float PAUSE_MENU_Y = 60.0f;
static const float PAUSE_ROW_H = 30.0f;
// The 256x208 napkin board, centred on the top screen
static const float PAUSE_BOARD_X = 72.0f;
static const float PAUSE_BOARD_Y = 16.0f;
static const float FILE_GRID_X = 26.0f;
static const float FILE_GRID_Y = 22.0f;
static const float FILE_CELL_W = 45.0f;
static const float FILE_CELL_H = 30.0f;

// Rank ladder from GameData.as; anything past the end just stays Better Than Papa
static const char *RANK_TITLES[] = {
	"!", "Newbie", "Trainee", "Cashier", "Delivery Boy", "Part-Time Cook", "Line Cook",
	"Pizza Topper", "Head Cook", "Chef Trainee", "Assistant Chef", "Sous Chef", "Pizza Chef",
	"Head Chef", "Master Chef", "Pizza Master", "Pepperoni Lover", "Sausage Specialist",
	"Mushroom Master", "Pepper Pro", "Onion Wrangler", "Olive Expert", "Anchovy Flinger",
	"Dough Tosser", "Cheese Grater", "Oven Expert", "Slicer and Dicer", "Pizzeria Manager",
	"Pizza Commander", "Master of Pizzas", "Ultimate Chef", "Better Than Papa!"
};

static const char *rankTitle(int rank)
{
	static const int count = (int)(sizeof(RANK_TITLES) / sizeof(RANK_TITLES[0]));
	if (rank < 0) rank = 0;
	return RANK_TITLES[rank < count ? rank : count - 1];
}

// Cents to "$1.23", because everybody wants their tips written out properly
static void formatMoney(char *out, size_t size, int cents)
{
	if (cents < 0) cents = 0;
	std::snprintf(out, size, "$%d.%02d", cents / 100, cents % 100);
}

// The pause menu and the two screens it can open

// Only worth holding this art while the game's actually stopped
void Papas::Game::loadPauseSheet()
{
	if (pauseSheet != nullptr) return;
	pauseSheet = C2D_SpriteSheetLoad("romfs:/gfx/pause.t3x");
	if (pauseSheet == nullptr) return;
	pauseWoodImg = C2D_SpriteSheetGetImage(pauseSheet, 0);
	pauseBoardImg = C2D_SpriteSheetGetImage(pauseSheet, 1);
}

void Papas::Game::freePauseSheet()
{
	if (pauseSheet != nullptr)
	{
		C2D_SpriteSheetFree(pauseSheet);
		pauseSheet = nullptr;
	}
}

// Stack the wood band down the screen; its grain runs sideways so it tiles fine
void Papas::Game::renderPauseWood()
{
	if (pauseSheet == nullptr) return;
	Papas::Stereo::plane(1.0f);
	float band = (float)pauseWoodImg.subtex->height;
	for (float y = 0.0f; y < SCREEN_HEIGHT_TOP; y += band)
		C2D_DrawImageAt(pauseWoodImg, 0.0f, y, 0.05f);
}

void Papas::Game::togglePause()
{
	if (overlay == OverlayNone)
	{
		overlay = OverlayPause;
		pauseIndex = 0;
		loadPauseSheet();
		Papas::Clock::pause();
		Papas::ResourceManager::getInstance().pauseMusic();
	}
	else
	{
		// Whatever's on top, unpausing drops you straight back to work
		if (overlay == OverlayFile) closeFile();
		overlay = OverlayNone;
		freePauseSheet();
		Papas::Clock::resume();
		Papas::ResourceManager::getInstance().resumeMusic();
	}
}

// Back out of a sub-screen to the menu that opened it
void Papas::Game::leaveOverlay()
{
	if (overlay == OverlayFile) closeFile();
	overlay = OverlayPause;
}

void Papas::Game::updatePause(u32 kDown)
{
	ResourceManager &audio = ResourceManager::getInstance();

	if (overlay == OverlayHelp)
	{
		if (kDown & (KEY_B | KEY_START)) leaveOverlay();
		if (kDown & (KEY_R | KEY_DRIGHT | KEY_DDOWN | KEY_A)) helpBook.turnPage(1);
		if (kDown & (KEY_L | KEY_DLEFT | KEY_DUP)) helpBook.turnPage(-1);
		if (kDown & KEY_TOUCH)
		{
			int row = HelpBook::rowAt(touch);
			if (row >= 0) helpBook.setPage(row);
		}
		return;
	}

	if (overlay == OverlayFile)
	{
		if (kDown & (KEY_B | KEY_START)) { leaveOverlay(); return; }
		if (kDown & KEY_TOUCH)
		{
			// Six by six grid of everybody, tap a face to pull their card
			int col = (touch.px - FILE_GRID_X) / FILE_CELL_W;
			int row = (touch.py - FILE_GRID_Y) / FILE_CELL_H;
			if (touch.px >= FILE_GRID_X && touch.py >= FILE_GRID_Y && col >= 0 && col < 6 && row >= 0 && row < 6)
			{
				int type = row * 6 + col + 1;
				const SaveData &sv = SaveManager::getInstance().data;
				if (type <= 36 && sv.customerFirstDay[type] != 0)
				{
					audio.playSfx("buttonclick");
					selectFileCustomer(type);
				}
			}
		}
		return;
	}

	// The pause menu itself
	static const int ENTRY_COUNT = 5;
	if (kDown & KEY_DDOWN) pauseIndex = (pauseIndex + 1) % ENTRY_COUNT;
	if (kDown & KEY_DUP) pauseIndex = (pauseIndex + ENTRY_COUNT - 1) % ENTRY_COUNT;
	if (kDown & KEY_B) { togglePause(); return; }

	bool picked = (kDown & KEY_A) != 0;
	if (kDown & KEY_TOUCH)
	{
		int row = (touch.py - PAUSE_MENU_Y) / PAUSE_ROW_H;
		if (touch.px >= 60 && touch.px <= 260 && touch.py >= PAUSE_MENU_Y && row >= 0 && row < ENTRY_COUNT)
		{
			pauseIndex = row;
			picked = true;
		}
	}
	if (!picked) return;

	audio.playSfx("buttonclick");
	switch (pauseIndex)
	{
	case 0:
		togglePause();
		break;
	case 1:
		openFile();
		break;
	case 2:
		overlay = OverlayHelp;
		break;
	case 3:
		audio.toggleMute();
		break;
	case 4:
		// Save what we've got and hand back to the menu, same as quitGame()
		SaveManager::getInstance().save();
		Papas::Clock::resume();
		audio.stopMusic();
		p_sceneManager->changeScene(new Papas::MainMenu());
		break;
	}
}

void Papas::Game::renderPauseTop()
{
	// Napkin's clean writing area, in screen space once the board's placed
	static const float NAPKIN_CX = 212.0f;
	const u32 colInk = C2D_Color32(39, 42, 35, 255);
	const u32 colTitle = C2D_Color32(170, 63, 24, 255);

	Papas::Stereo::plane(-0.1f);
	if (pauseSheet != nullptr)
		C2D_DrawImageAt(pauseBoardImg, PAUSE_BOARD_X, PAUSE_BOARD_Y, 0.20f);

	C2D_TextBufClear(uiTextBuf);
	char line[64];
	char money[16];
	C2D_Text text;

	// The board art already says "pause", so this is just the day's standings
	std::snprintf(line, sizeof(line), "Day %d", currentDay);
	C2D_TextFontParse(&text, dokyo, uiTextBuf, line);
	C2D_TextOptimize(&text);
	drawTextCentered(&text, NAPKIN_CX, 100.0f, 0.94f, 0.78f, colTitle);

	std::snprintf(line, sizeof(line), "Rank %d", myRank);
	C2D_TextFontParse(&text, dokyo, uiTextBuf, line);
	C2D_TextOptimize(&text);
	drawTextCentered(&text, NAPKIN_CX, 134.0f, 0.94f, 0.6f, colInk);

	C2D_TextFontParse(&text, dokyo, uiTextBuf, rankTitle(myRank));
	C2D_TextOptimize(&text);
	drawTextCentered(&text, NAPKIN_CX, 158.0f, 0.94f, 0.5f, colInk);

	formatMoney(money, sizeof(money), totalTipsCents);
	std::snprintf(line, sizeof(line), "Tips %s", money);
	C2D_TextFontParse(&text, dokyo, uiTextBuf, line);
	C2D_TextOptimize(&text);
	drawTextCentered(&text, NAPKIN_CX, 184.0f, 0.94f, 0.56f, colInk);
}

void Papas::Game::renderPauseBottom()
{
	static const char *ENTRIES[] = {"Resume", "Customer File", "How to Play", "Sound", "Quit to Menu"};
	const u32 colPaper = C2D_Color32(244, 239, 218, 255);
	const u32 colInk = C2D_Color32(39, 42, 35, 255);
	const u32 colPick = C2D_Color32(236, 121, 42, 255);

	C2D_DrawRectSolid(50.0f, 30.0f, 0.90f, 220.0f, 176.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(55.0f, 35.0f, 0.91f, 210.0f, 166.0f, colPaper);

	C2D_TextBufClear(uiTextBuf);
	for (int i = 0; i < 5; i++)
	{
		char label[32];
		if (i == 3)
			std::snprintf(label, sizeof(label), "Sound: %s",
				ResourceManager::getInstance().isMuted() ? "Off" : "On");
		else
			std::snprintf(label, sizeof(label), "%s", ENTRIES[i]);

		C2D_Text text;
		C2D_TextFontParse(&text, dokyo, uiTextBuf, label);
		C2D_TextOptimize(&text);
		drawTextCentered(&text, SCREEN_WIDTH_BOTTOM * 0.5f, PAUSE_MENU_Y + i * PAUSE_ROW_H + 4.0f,
			0.94f, 0.5f, i == pauseIndex ? colPick : colInk);
	}
}

// Customer file: everyone we've met, and a card for whoever's picked

void Papas::Game::openFile()
{
	overlay = OverlayFile;
	// Pausing during the day intro gets here before the lobby has loaded the rig
	CustomerRig::getInstance().load();
	if (fileSelected == 0)
	{
		// Open on the first person we've actually served
		const SaveData &sv = SaveManager::getInstance().data;
		for (int type = 1; type <= 36; type++)
			if (sv.customerFirstDay[type] != 0) { selectFileCustomer(type); break; }
	}
}

void Papas::Game::closeFile()
{
	CustomerRig::getInstance().freeType(fileAtlas);
	fileSelected = 0;
}

void Papas::Game::selectFileCustomer(int type)
{
	CustomerRig::getInstance().freeType(fileAtlas);
	fileSelected = type;
	CustomerRig::getInstance().loadType(type, fileAtlas);

	const SaveData &sv = SaveManager::getInstance().data;
	C2D_TextBufClear(fileTextBuf);
	char lines[5][64];

	std::unordered_map<int, CustomerData>::const_iterator entry = map_customers.find(type);
	std::snprintf(lines[0], sizeof(lines[0]), "%s",
		entry != map_customers.end() ? entry->second.name.c_str() : "");

	std::unordered_map<int, std::string>::const_iterator fave = map_customerToppings.find(type);
	std::snprintf(lines[1], sizeof(lines[1]), "%s",
		fave != map_customerToppings.end() ? fave->second.c_str() : "");

	std::snprintf(lines[2], sizeof(lines[2]), "First came on day %d", (int)sv.customerFirstDay[type]);
	std::snprintf(lines[3], sizeof(lines[3]), "Pizzas ordered: %d", (int)sv.customerServed[type]);
	std::snprintf(lines[4], sizeof(lines[4]), "Gold seals: %d of 3", (int)sv.customerSeals[type]);

	for (int i = 0; i < 5; i++)
	{
		C2D_TextFontParse(&fileText[i], dokyo, fileTextBuf, lines[i]);
		C2D_TextOptimize(&fileText[i]);
	}
}

void Papas::Game::renderFileTop()
{
	// The rig hangs off its top-left corner and stands 322 units tall, so the
	// photo box gets centred around that rather than around the draw position.
	static const float RIG_W = 149.0f;
	static const float RIG_H = 323.0f;
	static const float PHOTO_X = 36.0f, PHOTO_Y = 32.0f, PHOTO_W = 128.0f, PHOTO_H = 148.0f;

	const u32 colPaper = C2D_Color32(244, 239, 218, 255);
	const u32 colTitle = C2D_Color32(170, 63, 24, 255);
	const u32 colInk = C2D_Color32(39, 42, 35, 255);

	Papas::Stereo::plane(0.6f);
	C2D_DrawRectSolid(14.0f, 10.0f, 0.90f, 372.0f, 220.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(19.0f, 15.0f, 0.91f, 362.0f, 210.0f, colPaper);

	if (fileSelected == 0) return;

	// Polaroid on the left, their details on the right
	C2D_DrawRectSolid(30.0f, 26.0f, 0.92f, 140.0f, 188.0f, C2D_Color32(255, 255, 255, 255));
	C2D_DrawRectSolid(PHOTO_X, PHOTO_Y, 0.93f, PHOTO_W, PHOTO_H, C2D_Color32(198, 224, 178, 255));

	Papas::Stereo::plane(0.35f);
	CustomerRig &rig = CustomerRig::getInstance();
	int segment = rig.segmentIndex(SaveManager::getInstance().data.customerSeals[fileSelected] > 0
		? "overjoyed" : "stand");
	float scale = (PHOTO_H - 10.0f) / RIG_H;
	rig.draw(fileAtlas, fileSelected, rig.frameForTime(segment, 0.0f),
		PHOTO_X + (PHOTO_W - RIG_W * scale) * 0.5f,
		PHOTO_Y + (PHOTO_H - RIG_H * scale) * 0.5f,
		scale, scale, 0.94f);

	Papas::Stereo::plane(0.3f);
	C2D_DrawText(&fileText[0], C2D_WithColor, 186.0f, 30.0f, 0.95f, 0.76f, 0.76f, colTitle);
	C2D_DrawText(&fileText[1], C2D_WithColor, 186.0f, 72.0f, 0.95f, 0.46f, 0.46f, colInk);
	for (int i = 2; i < 5; i++)
		C2D_DrawText(&fileText[i], C2D_WithColor, 186.0f, 104.0f + (i - 2) * 26.0f, 0.95f, 0.5f, 0.5f, colInk);

	renderStarRow(262.0f, 188.0f, SaveManager::getInstance().data.customerStars[fileSelected], 0, 0.95f);
}

void Papas::Game::renderFileBottom()
{
	const SaveData &sv = SaveManager::getInstance().data;
	const u32 colPaper = C2D_Color32(244, 239, 218, 255);
	const u32 colInk = C2D_Color32(39, 42, 35, 255);
	const u32 colSoft = C2D_Color32(178, 170, 150, 255);
	const u32 colPick = C2D_Color32(236, 121, 42, 255);

	C2D_DrawRectSolid(10.0f, 6.0f, 0.90f, 300.0f, 206.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(14.0f, 10.0f, 0.91f, 292.0f, 198.0f, colPaper);

	C2D_TextBufClear(uiTextBuf);
	for (int type = 1; type <= 36; type++)
	{
		int col = (type - 1) % 6;
		int row = (type - 1) / 6;
		float x = FILE_GRID_X + col * FILE_CELL_W;
		float y = FILE_GRID_Y + row * FILE_CELL_H;
		bool met = sv.customerFirstDay[type] != 0;

		C2D_DrawRectSolid(x + 2.0f, y + 2.0f, 0.92f, FILE_CELL_W - 5.0f, FILE_CELL_H - 5.0f,
			type == fileSelected ? colPick : (met ? C2D_Color32(214, 205, 178, 255) : colSoft));

		char label[8];
		std::snprintf(label, sizeof(label), met ? "%d" : "?", type);
		C2D_Text text;
		C2D_TextFontParse(&text, dokyo, uiTextBuf, label);
		C2D_TextOptimize(&text);
		drawTextCentered(&text, x + FILE_CELL_W * 0.5f - 1.0f, y + 3.0f, 0.94f, 0.52f, colInk);

		// A pip in the corner for anyone who's earned a seal
		if (met && sv.customerSeals[type] > 0)
			C2D_DrawRectSolid(x + FILE_CELL_W - 9.0f, y + 4.0f, 0.95f, 4.0f, 4.0f,
				C2D_Color32(232, 186, 48, 255));
	}

	C2D_Text hint;
	C2D_TextFontParse(&hint, dokyo, uiTextBuf, "Tap a customer    B Back");
	C2D_TextOptimize(&hint);
	drawTextCentered(&hint, SCREEN_WIDTH_BOTTOM * 0.5f, 214.0f, 0.94f, 0.46f,
		C2D_Color32(120, 105, 90, 255));
}

// End of day: how we did, what we made, and whether it earned a promotion

void Papas::Game::beginEndOfDay()
{
	showingEndOfDay = true;
	endDayPhase = EndDayBoard;
	endDayPhaseStarted = Papas::Clock::now();
	// showTips(): work out the promotion now, hand it over at the rank step
	endDayRankUp = totalTipsCents > lastRankLimit + (myRank + 1) * 500;
	prepareEndDayText();
	Papas::ResourceManager::getInstance().stopMusic();
	Papas::ResourceManager::getInstance().playSfx("endofday");
	Papas::ResourceManager::getInstance().switchMusic("justguitar_music");
}

void Papas::Game::prepareEndDayText()
{
	C2D_TextBufClear(endDayTextBuf);
	int served = std::max(1, customersToday);
	int waiting = waitingToday / served;
	int topping = toppingToday / served;
	int baking = bakingToday / served;
	int cutting = cuttingToday / served;
	int quality = (waiting + topping + baking + cutting) / 4;

	// Labels and their numbers are drawn separately so the numbers can be coloured
	char lines[13][64];
	char money[16];
	std::snprintf(lines[0], sizeof(lines[0]), "Day %d Done!", currentDay);
	std::snprintf(lines[1], sizeof(lines[1]), "Customers");
	std::snprintf(lines[2], sizeof(lines[2]), "Quality");
	std::snprintf(lines[3], sizeof(lines[3]), "Waiting");
	std::snprintf(lines[4], sizeof(lines[4]), "Toppings");
	std::snprintf(lines[5], sizeof(lines[5]), "Baking");
	std::snprintf(lines[6], sizeof(lines[6]), "Cutting");
	formatMoney(money, sizeof(money), tipsTodayCents);
	std::snprintf(lines[7], sizeof(lines[7]), "Tips Today   %s", money);
	formatMoney(money, sizeof(money), totalTipsCents);
	std::snprintf(lines[8], sizeof(lines[8]), "Tips Total   %s", money);
	// Rank line reads whatever myRank is now, so this gets rebuilt after a promotion
	std::snprintf(lines[9], sizeof(lines[9]), "Rank %d", myRank);
	std::snprintf(lines[10], sizeof(lines[10]), "%s", rankTitle(myRank));
	formatMoney(money, sizeof(money), std::max(0, lastRankLimit + (myRank + 1) * 500 - totalTipsCents));
	std::snprintf(lines[11], sizeof(lines[11]), "Next rank in %s", money);
	std::snprintf(lines[12], sizeof(lines[12]), "Continue");

	for (int i = 0; i < 13; i++)
	{
		C2D_TextFontParse(&endDayText[i], dokyo, endDayTextBuf, lines[i]);
		C2D_TextOptimize(&endDayText[i]);
	}

	char values[6][16];
	std::snprintf(values[0], sizeof(values[0]), "%d", customersToday);
	std::snprintf(values[1], sizeof(values[1]), "%d%%", quality);
	std::snprintf(values[2], sizeof(values[2]), "%d%%", waiting);
	std::snprintf(values[3], sizeof(values[3]), "%d%%", topping);
	std::snprintf(values[4], sizeof(values[4]), "%d%%", baking);
	std::snprintf(values[5], sizeof(values[5]), "%d%%", cutting);
	for (int i = 0; i < 6; i++)
	{
		C2D_TextFontParse(&endDayValue[i], dokyo, endDayTextBuf, values[i]);
		C2D_TextOptimize(&endDayValue[i]);
	}
}

void Papas::Game::updateEndOfDay()
{
	u64 elapsed = Papas::Clock::now() - endDayPhaseStarted;
	if (endDayPhase == EndDayTips && elapsed >= 2600)
	{
		// determineUpgrade(): the promotion lands here, with a cheer to match
		if (endDayRankUp)
		{
			myRank++;
			lastRankLimit += myRank * 500;
			Papas::ResourceManager::getInstance().playSfx("customer_overjoyed");
		}
		prepareEndDayText();
		endDayPhase = EndDayRank;
		endDayPhaseStarted = Papas::Clock::now();
	}
	else if (endDayPhase == EndDayRank && elapsed >= 1800)
	{
		endDayPhase = EndDayReady;
		endDayPhaseStarted = Papas::Clock::now();
	}
}

void Papas::Game::renderEndOfDayTop()
{
	// Same counter the results use, with the day's takings still sitting on it
	Papas::Stereo::plane(1.0f);
	drawBackdrop(to_wallpaper, 0, 0, 0);
	Papas::Stereo::plane(0.0f);
	C2D_DrawImageAt(to_counter, 0, 0, 0.65f);

	// Board centred on the top screen, labels left of the middle and figures right
	static const float BOARD_X = 62.0f, BOARD_W = 276.0f;
	static const float BOARD_CX = BOARD_X + BOARD_W * 0.5f;
	static const float LABEL_X = BOARD_X + 34.0f;
	static const float VALUE_R = BOARD_X + BOARD_W - 34.0f;
	const u32 colPaper = C2D_Color32(244, 239, 218, 255);
	const u32 colTitle = C2D_Color32(170, 63, 24, 255);
	const u32 colInk = C2D_Color32(39, 42, 35, 255);
	const u32 colScore = C2D_Color32(36, 105, 4, 255);

	Papas::Stereo::plane(-0.1f);
	C2D_DrawRectSolid(BOARD_X - 5.0f, 15.0f, 0.90f, BOARD_W + 10.0f, 210.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(BOARD_X, 20.0f, 0.91f, BOARD_W, 200.0f, colPaper);

	drawTextCentered(&endDayText[0], BOARD_CX, 28.0f, 0.94f, 0.78f, colTitle);
	if (endDayPhase == EndDayBoard)
	{
		for (int i = 1; i <= 6; i++)
		{
			float y = 74.0f + (i - 1) * 24.0f;
			C2D_DrawText(&endDayText[i], C2D_WithColor, LABEL_X, y, 0.94f, 0.54f, 0.54f, colInk);
			// Right-align the figures so the column of numbers lines up
			float width = 0.0f;
			C2D_TextGetDimensions(&endDayValue[i - 1], 0.54f, 0.54f, &width, nullptr);
			C2D_DrawText(&endDayValue[i - 1], C2D_WithColor, VALUE_R - width, y, 0.94f, 0.54f, 0.54f, colScore);
		}
	}
	else
	{
		drawTextCentered(&endDayText[7], BOARD_CX, 74.0f, 0.94f, 0.58f, colInk);
		drawTextCentered(&endDayText[8], BOARD_CX, 104.0f, 0.94f, 0.58f, colInk);
		if (endDayPhase != EndDayTips)
		{
			drawTextCentered(&endDayText[9], BOARD_CX, 144.0f, 0.94f, 0.72f, colTitle);
			drawTextCentered(&endDayText[10], BOARD_CX, 178.0f, 0.94f, 0.52f, colInk);
		}
	}

	// Jar keeps the day's tips on show through the whole wrap-up
	Papas::Stereo::plane(-0.08f);
	renderTipJar();
}

void Papas::Game::renderEndOfDayBottom()
{
	const u32 colPaper = C2D_Color32(244, 239, 218, 255);
	const u32 colInk = C2D_Color32(39, 42, 35, 255);
	const u32 colSoft = C2D_Color32(120, 105, 90, 255);

	C2D_DrawRectSolid(18.0f, 46.0f, 0.90f, 284.0f, 148.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(22.0f, 50.0f, 0.91f, 276.0f, 140.0f, colPaper);

	// Running total up here, and how far off the next promotion once it's settled
	drawCenteredText(endDayText[8], 70.0f, 0.62f, colInk);
	if (endDayPhase == EndDayReady)
		drawCenteredText(endDayText[11], 104.0f, 0.52f, colSoft);

	// Board waits for you to look it over, the rest of it plays itself out
	if (endDayPhase == EndDayBoard || endDayPhase == EndDayReady)
	{
		C2D_DrawRectSolid(100.0f, 148.0f, 0.92f, 120.0f, 30.0f, C2D_Color32(36, 105, 4, 255));
		C2D_DrawRectSolid(103.0f, 151.0f, 0.93f, 114.0f, 24.0f, C2D_Color32(76, 190, 16, 255));
		drawCenteredText(endDayText[12], 154.0f, 0.50f, C2D_Color32(11, 33, 0, 255));
	}
}

// A pizza got served: score it, pick the reaction, kick off the result screen
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

	// Scores 80+ earn a star, under 60 clears them, and five makes a seal.
	SaveData &sv = SaveManager::getInstance().data;
	resultCustomerType = ticket->customerType;
	bool starTracked = resultCustomerType > 0 && resultCustomerType < (int)sizeof(sv.customerStars);
	resultStarsBefore = starTracked ? sv.customerStars[resultCustomerType] : 0;
	resultSealsBefore = starTracked ? sv.customerSeals[resultCustomerType] : 0;
	resultStarEarned = 0;
	resultStarsLost = false;
	resultSealEarned = false;
	resultStarSfxPlayed = false;
	if (result.overall >= 80 && resultStarsBefore < 5)
		resultStarEarned = resultStarsBefore + 1;
	else if (result.overall < 60 && resultStarsBefore > 0)
		resultStarsLost = true;

	// Each seal adds $1 to the $3 max tip; the fifth star pays $9.
	int maxTipCents = 300 + resultSealsBefore * 100;
	result.tipCents = std::max(0, (int)std::round((result.overall * 2.0f - 100.0f) / 100.0f * maxTipCents));
	if (resultStarEarned == 5)
		result.tipCents = 900;

	if (starTracked)
	{
		if (resultStarEarned == 5)
		{
			// Row full: back to zero stars, seal awarded unless they have all 3
			if (resultSealsBefore < 3)
			{
				sv.customerSeals[resultCustomerType] = resultSealsBefore + 1;
				resultSealEarned = true;
			}
			sv.customerStars[resultCustomerType] = 0;
		}
		else if (resultStarEarned > 0)
			sv.customerStars[resultCustomerType] = resultStarEarned;
		else if (resultStarsLost)
			sv.customerStars[resultCustomerType] = 0;
	}

	// Day tally the end-of-day board averages out
	customersToday++;
	waitingToday += result.waiting;
	toppingToday += result.topping;
	bakingToday += result.baking;
	cuttingToday += result.cutting;
	tipsTodayCents += result.tipCents;

	result.customerName = order.name;
	if (result.overall >= 90) resultReaction = "overjoyed";
	else if (result.overall >= 80) resultReaction = "happy";
	else if (result.overall >= 70) resultReaction = "decent";
	else if (result.overall >= 60) resultReaction = "worried";
	else if (result.overall >= 50) resultReaction = "upset";
	else resultReaction = "pissed";
	totalScore += result.overall;
	totalTipsCents += result.tipCents;

	int customerNumber = ticket->customerNumber;
	resultCustomerNumber = customerNumber;
	resultPizzaId = pizza.id;
	pizza.ticket = nullptr;
	r_manager.removeReceipt(ticket);
	prepareResultText();
	Customer *customer = c_manager.getCustomer(resultCustomerNumber);
	if (customer != nullptr) customer->playPresentation("stand");
	loadGiveOrderSheets();
	giveOrderPick = 0;
	resultPhase = ResultDrumroll;
	resultPhaseStarted = Papas::Clock::now();
	resultStartedAt = Papas::Clock::now();
	Papas::ResourceManager::getInstance().playSfx("giveorder_drumroll");
	showingResult = true;
}

// Run the original result timing, including reactions and tip sounds.
void Papas::Game::updateResult()
{
	u64 elapsed = Papas::Clock::now() - resultPhaseStarted;
	Customer *customer = c_manager.getCustomer(resultCustomerNumber);
	if (resultPhase == ResultDrumroll && elapsed >= 1980)
	{
		if (customer != nullptr) customer->playPresentation("look");
		resultPhase = ResultLook;
		resultPhaseStarted = Papas::Clock::now();
	}
	else if (resultPhase == ResultLook && elapsed >= 1485)
	{
		if (customer != nullptr) customer->playPresentation(resultReaction.c_str());
		if (resultReaction == "overjoyed") Papas::ResourceManager::getInstance().playSfx("customer_overjoyed");
		else if (resultReaction == "happy") Papas::ResourceManager::getInstance().playSfx("customer_happy");
		else if (resultReaction == "decent") Papas::ResourceManager::getInstance().playSfx("customer_decent");
		else if (resultReaction == "worried") Papas::ResourceManager::getInstance().playSfx("customer_worried");
		else if (resultReaction == "upset") Papas::ResourceManager::getInstance().playSfx("customer_upset");
		else Papas::ResourceManager::getInstance().playSfx("customer_pissed");
		resultPhase = ResultReaction;
		resultPhaseStarted = Papas::Clock::now();
	}
	else if (resultPhase == ResultReaction && elapsed >= 1300)
	{
		if (result.tipCents > 0)
		{
			if (result.overall < 70) Papas::ResourceManager::getInstance().playSfx("singlecoin");
			else Papas::ResourceManager::getInstance().playSfx("multicoin");
		}
		resultPhase = ResultTip;
		resultPhaseStarted = Papas::Clock::now();
	}
	else if (resultPhase == ResultTip && elapsed >= 3300)
	{
		resultPhase = ResultReady;
		resultPhaseStarted = Papas::Clock::now();
	}

	// Light the earned star partway through the tip phase.
	if (resultPhase == ResultTip && resultStarEarned > 0 && !resultStarSfxPlayed
	    && Papas::Clock::now() - resultPhaseStarted >= 700)
	{
		Papas::ResourceManager::getInstance().playSfx("getstar");
		resultStarSfxPlayed = true;
	}
}

// Bake all the score panel lines into text once per serve
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

// The score panel on the bottom screen, Continue only once the show's over
void Papas::Game::renderResult()
{
	C2D_DrawRectSolid(18.0f, 12.0f, 0.90f, 284.0f, 220.0f, C2D_Color32(35, 42, 37, 245));
	C2D_DrawRectSolid(22.0f, 16.0f, 0.91f, 276.0f, 212.0f, C2D_Color32(244, 239, 218, 255));
	drawCenteredText(resultText[0], 24.0f, 0.72f, C2D_Color32(35, 61, 31, 255));
	drawCenteredText(resultText[1], 55.0f, 0.68f, C2D_Color32(170, 63, 24, 255));
	for (int i = 2; i <= 6; i++)
		drawCenteredText(resultText[i], 82.0f + (i - 2) * 22.0f, 0.52f, C2D_Color32(39, 42, 35, 255));
	if (resultPhase == ResultReady)
	{
		C2D_DrawRectSolid(100.0f, 198.0f, 0.92f, 120.0f, 30.0f, C2D_Color32(36, 105, 4, 255));
		C2D_DrawRectSolid(103.0f, 201.0f, 0.93f, 114.0f, 24.0f, C2D_Color32(76, 190, 16, 255));
		drawCenteredText(resultText[7], 204.0f, 0.50f, C2D_Color32(11, 33, 0, 255));
	}
}

// Tip jar on the top screen; the coin drops in and the fill level bumps up
void Papas::Game::renderTipJar()
{
	const float jarX = 304.0f;
	const float jarY = 137.0f;
	int shownTips = totalTipsCents;
	if (resultPhase < ResultTip || (resultPhase == ResultTip && Papas::Clock::now() - resultPhaseStarted < 900))
		shownTips -= result.tipCents;
	int jarFrame = 1;
	if (shownTips > 0)
	{
		jarFrame = (int)std::ceil(std::min(1.0f, shownTips / 3000.0f) * 10.0f);
		jarFrame = std::max(2, std::min(10, jarFrame));
	}
	C2D_DrawImageAt(resultCoinPiles[jarFrame - 1], jarX + 3.0f, jarY + 23.0f, 0.82f);
	C2D_DrawImageAt(resultJar, jarX, jarY, 0.84f);
	if (resultPhase == ResultTip && result.tipCents > 0)
	{
		float elapsed = (float)(Papas::Clock::now() - resultPhaseStarted);
		float progress = std::min(1.0f, elapsed / 950.0f);
		int frame = ((int)(elapsed / 70.0f)) % 6;
		C2D_Image coin = resultCoinSpin[frame];
		C2D_DrawImageAt(coin, jarX + 28.0f, 55.0f + progress * 92.0f, 0.86f, nullptr, 0.45f, 0.45f);
	}
}

// Draw the customer's five stars and their seal pins.
void Papas::Game::renderStarRow(float centerX, float y, int stars, int seals, float depth)
{
	static const float STAR_SCALE = 0.45f;
	static const float SEAL_SCALE = 0.5f;
	float stepX = starFilledImg.subtex->width * STAR_SCALE + 2.0f;
	float x = centerX - (5.0f * stepX - 2.0f) * 0.5f;
	for (int i = 0; i < 5; i++)
	{
		C2D_Image img = (i < stars) ? starFilledImg : starEmptyImg;
		C2D_DrawImageAt(img, x + i * stepX, y, depth, nullptr, STAR_SCALE, STAR_SCALE);
	}
	for (int s = 0; s < seals && s < 3; s++)
		C2D_DrawImageAt(sealImg, x - 26.0f - s * 12.0f, y - 2.0f, depth,
			nullptr, SEAL_SCALE, SEAL_SCALE);
}

// Keep old stars through the drumroll, then reveal and flash the earned one.
void Papas::Game::renderResultStars()
{
	static const float ROW_CX = 170.0f;
	static const float ROW_Y = 204.0f;
	static const float STAR_SCALE = 0.45f;
	u64 sincePhase = Papas::Clock::now() - resultPhaseStarted;
	bool revealed = resultPhase == ResultReady ||
		(resultPhase == ResultTip && sincePhase >= 700);

	int stars = resultStarsBefore;
	int seals = resultSealsBefore;
	if (revealed)
	{
		if (resultStarEarned > 0) stars = resultStarEarned;
		else if (resultStarsLost) stars = 0;
		if (resultSealEarned) seals++;
	}
	renderStarRow(ROW_CX, ROW_Y, stars, seals, 0.80f);

	// White flash popping over the freshly earned star
	if (revealed && resultStarEarned > 0 && resultPhase == ResultTip)
	{
		float t = std::min(1.0f, (float)(sincePhase - 700) / 600.0f);
		float w = starFilledImg.subtex->width * STAR_SCALE;
		float stepX = w + 2.0f;
		float rowX = ROW_CX - (5.0f * stepX - 2.0f) * 0.5f;
		float cx = rowX + (resultStarEarned - 1) * stepX + w * 0.5f;
		float cy = ROW_Y + starFilledImg.subtex->height * STAR_SCALE * 0.5f;
		float scale = STAR_SCALE * (1.0f + 1.1f * (1.0f - t));
		C2D_ImageTint tint;
		C2D_AlphaImageTint(&tint, 1.0f - t);
		C2D_DrawImageAt(starFlashImg,
			cx - starFlashImg.subtex->width * scale * 0.5f,
			cy - starFlashImg.subtex->height * scale * 0.5f,
			0.81f, &tint, scale, scale);
	}
}

// Name + star row on the take-order counter, like the original takeorder_fg
void Papas::Game::renderTakeOrderStars(int customerType)
{
	static const float ROW_CX = 180.0f;
	const SaveData &sv = SaveManager::getInstance().data;
	int stars = 0, seals = 0;
	if (customerType > 0 && customerType < (int)sizeof(sv.customerStars))
	{
		stars = sv.customerStars[customerType];
		seals = sv.customerSeals[customerType];
	}
	renderStarRow(ROW_CX, 166.0f, stars, seals, 0.66f);
	float tw = 0.0f;
	C2D_TextGetDimensions(&takeOrderNameText, 0.5f, 0.5f, &tw, nullptr);
	C2D_DrawText(&takeOrderNameText, C2D_WithColor, ROW_CX - tw * 0.5f, 188.0f, 0.66f,
		0.5f, 0.5f, C2D_Color32(244, 239, 218, 255));
}

// Transparent pixels write depth, so draw take-order layers back-to-front.
static const float TO_COUNTER_DEPTH = 0.002f;
static const float TO_WALL_DEPTH = 0.003f;

// Keep the ticket wall and its receipts on one stereo plane across both screens.
static const float TICKET_WALL_PLANE = -0.05f;

// Stretch the backdrop's edge texel so stereo shifts can't expose black.
static const float BACKDROP_BLEED = 6.0f;	// > STRENGTH * the furthest plane

static void drawBackdropEdge(C2D_Image img, float u, float x, float y, float depth)
{
	Tex3DS_SubTexture edge = *img.subtex;
	edge.width = 1;
	edge.left = u;
	edge.right = u;
	C2D_Image slice = { img.tex, &edge };
	C2D_DrawImageAt(slice, x, y, depth, nullptr, BACKDROP_BLEED, 1.0f);
}

// Drop-in for C2D_DrawImageAt on any backdrop that has to reach the screen edge.
static void drawBackdrop(C2D_Image img, float x, float y, float depth)
{
	const float texelU = 1.0f / (float)img.tex->width;
	drawBackdropEdge(img, img.subtex->left + 0.5f * texelU,
		x - BACKDROP_BLEED, y, depth);
	drawBackdropEdge(img, img.subtex->right - 0.5f * texelU,
		x + (float)img.subtex->width, y, depth);
	C2D_DrawImageAt(img, x, y, depth);
}

// Draw lobby layers in order; tiny customer offsets stay below the holder.
static const float LOBBY_STATION_DEPTH  = 0.001f;
static const float LOBBY_CUSTOMER_DEPTH = 0.0015f;
static const float LOBBY_HOLDER_DEPTH   = 0.002f;
static const float LOBBY_POPUP_DEPTH    = 0.003f;

PapasError Papas::Game::render_top()
{
	// Overlays sit on top of everything, so they get first say
	if (overlay != OverlayNone)
	{
		// Wood fills the screen behind all of them; the wallpaper is only 270 wide
		renderPauseWood();
		if (overlay == OverlayFile)
		{
			renderFileTop();
		}
		else if (overlay == OverlayHelp)
		{
			Papas::Stereo::plane(-0.1f);
			helpBook.renderTop();
		}
		else
		{
			renderPauseTop();
		}
		return PAPAS_OK;
	}

	if (showingEndOfDay)
	{
		renderEndOfDayTop();
		return PAPAS_OK;
	}

	if (showingNewCustomer)
	{
		renderNewCustomer();
		return PAPAS_OK;
	}

	if (showingDayIntro)
	{
		renderDayIntro();
		return PAPAS_OK;
	}

	if (showingResult)
	{
		Papas::Stereo::plane(1.0f);
		drawBackdrop(to_wallpaper, 0, 0, 0);
		Papas::Stereo::plane(0.5f);
		Customer *customer = c_manager.getCustomer(resultCustomerNumber);
		if (customer != nullptr) customer->renderOrdering(0.0f);
		Papas::Stereo::plane(0.0f);
		C2D_DrawImageAt(to_counter, 0, 0, 0.65f);
		renderResultStars();
		// Roy boxes the hidden pizza while the lid and glow face the viewer.
		if (giveOrderLoaded) renderGiveOrderRoy();
		else pz_manager.renderPizzaForResult(resultPizzaId, {211.0f, 125.0f}, 0.34f, 0.72f);
		Papas::Stereo::plane(-0.08f);
		renderTipJar();
		return PAPAS_OK;
	}

	if(!takingOrder){

		// Stay back-to-front because even transparent pixels write depth here.
		Papas::Stereo::plane(1.0f);
		drawBackdrop(ticketsStationImg, 0, 0, LOBBY_STATION_DEPTH);
		// Customers enter behind the holder, which doubles as the lobby door.
		Papas::Stereo::plane(0.75f);
		c_manager.renderLines(LOBBY_CUSTOMER_DEPTH);
		Papas::Stereo::plane(TICKET_WALL_PLANE);
		C2D_DrawImageAt(ticketsHolderImg, 260, 0, LOBBY_HOLDER_DEPTH);
		// Draw the transparent banner last so it can't hide customers' feet.
		Papas::Stereo::plane(0.0f);
		C2D_DrawImageAt(currentPopupImg, 0, 214, LOBBY_POPUP_DEPTH);
		// (the door sign lives in the corner of the bottom screen, render_bottom)
		Papas::Stereo::plane(TICKET_WALL_PLANE);
		r_manager.renderReceipt(true);
		Papas::Stereo::plane(0.1f);
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
		// Keep the receipt wall and its ticket in front of the counter.
		Papas::Stereo::plane(TICKET_WALL_PLANE);
		C2D_DrawImageAt(ticketsHolderImg, 260, 0, TO_WALL_DEPTH);
		r_manager.renderDockedReceipt(true);
	}
	

	return PAPAS_OK;
}

// Commiting a bullshittery here
void Papas::Game::setToppingBubble(const ItemOrder &item)
{
	to_bubbleKind = BubbleTopping;
	to_bubbleItem = item;
	C2D_TextBufClear(orderBubbleTextBuf);
	C2D_TextFontParse(&orderBubbleX, dokyo, orderBubbleTextBuf, "x");
	char quantity[8];
	std::snprintf(quantity, sizeof(quantity), "%d", item.Quantity);
	C2D_TextFontParse(&orderBubbleQuantity, dokyo, orderBubbleTextBuf, quantity);
	C2D_TextOptimize(&orderBubbleX);
	C2D_TextOptimize(&orderBubbleQuantity);
}

static void drawOrderBubbleImage(C2D_Image image, float x, float y, float scale, float depth)
{
	C2D_Sprite sprite;
	C2D_SpriteFromImage(&sprite, image);
	C2D_SpriteSetCenter(&sprite, 0.5f, 0.5f);
	C2D_SpriteSetPos(&sprite, x, y);
	C2D_SpriteSetScale(&sprite, scale, scale);
	C2D_SpriteSetDepth(&sprite, depth);
	C2D_DrawSprite(&sprite);
}

void Papas::Game::renderOrderBubble()
{
	static const float BUBBLE_X = 15.0f;
	static const float BUBBLE_Y = 28.0f;
	static const float BUBBLE_SCALE = 0.65f;
	// Balloon ellipse centre in unscaled bubble.png coordinates (tail excluded)
	static const float BUBBLE_CX = 71.5f;
	if (to_bubbleKind == BubbleHidden) return;
	if (to_bubbleKind == BubbleOpening)
	{
		// Bounce the balloon open, overshoot a little then settle
		float progress = std::min(1.0f, (float)(Papas::Clock::now() - to_orderStartedAt) / 700.0f);
		float bounce = progress < 0.7f ? progress / 0.7f * 1.08f : 1.08f - (progress - 0.7f) / 0.3f * 0.08f;
		float scale = BUBBLE_SCALE * bounce;
		drawOrderBubbleImage(orderBubbleBase,
			BUBBLE_X + orderBubbleBase.subtex->width * BUBBLE_SCALE * 0.5f,
			BUBBLE_Y + orderBubbleBase.subtex->height * BUBBLE_SCALE * 0.5f,
			scale, 0.80f);
		return;
	}

	C2D_DrawImageAt(orderBubbleBase, BUBBLE_X, BUBBLE_Y, 0.80f, nullptr, BUBBLE_SCALE, BUBBLE_SCALE);
	if (to_bubbleKind == BubbleTopping)
	{
		int coverage = 0;
		for (int q = 0; q < 4; q++) if (to_bubbleItem.Coverage[q]) coverage |= 1 << q;
		float coverageW = orderBubbleCoverage[coverage].subtex->width;
		C2D_DrawImageAt(orderBubbleCoverage[coverage], BUBBLE_X + (BUBBLE_CX - coverageW * 0.5f) * BUBBLE_SCALE,
			BUBBLE_Y + 12.0f * BUBBLE_SCALE, 0.82f, nullptr, BUBBLE_SCALE, BUBBLE_SCALE);
		// "icon xN" row: centre the whole group on the ellipse axis
		C2D_Image topping = orderBubbleToppings[to_bubbleItem.Topping];
		drawOrderBubbleImage(topping, BUBBLE_X + (BUBBLE_CX - 21.5f) * BUBBLE_SCALE,
			BUBBLE_Y + 90.0f * BUBBLE_SCALE, BUBBLE_SCALE, 0.82f);
		C2D_DrawText(&orderBubbleX, C2D_WithColor, BUBBLE_X + (BUBBLE_CX + 3.5f) * BUBBLE_SCALE,
			BUBBLE_Y + 76.0f * BUBBLE_SCALE, 0.83f, 0.65f * BUBBLE_SCALE,
			0.65f * BUBBLE_SCALE, C2D_Color32(130, 130, 130, 255));
		C2D_DrawText(&orderBubbleQuantity, C2D_WithColor, BUBBLE_X + (BUBBLE_CX + 28.5f) * BUBBLE_SCALE,
			BUBBLE_Y + 70.0f * BUBBLE_SCALE, 0.83f, 1.10f * BUBBLE_SCALE,
			1.10f * BUBBLE_SCALE, C2D_Color32(227, 42, 34, 255));
	}
	else if (to_bubbleKind == BubbleTime)
	{
		int frame = std::max(0, std::min(7, to_bubbleTime));
		float clockW = orderBubbleClocks[frame].subtex->width;
		C2D_DrawImageAt(orderBubbleClocks[frame], BUBBLE_X + (BUBBLE_CX - clockW * 0.5f) * BUBBLE_SCALE,
			BUBBLE_Y + 18.0f * BUBBLE_SCALE, 0.82f, nullptr, BUBBLE_SCALE, BUBBLE_SCALE);
	}
	else if (to_bubbleKind == BubbleCut)
	{
		int frame = to_bubbleCuts == 4 ? 0 : (to_bubbleCuts == 6 ? 1 : 2);
		float cutW = orderBubbleCuts[frame].subtex->width;
		C2D_DrawImageAt(orderBubbleCuts[frame], BUBBLE_X + (BUBBLE_CX - cutW * 0.5f) * BUBBLE_SCALE,
			BUBBLE_Y + 18.0f * BUBBLE_SCALE, 0.82f, nullptr, BUBBLE_SCALE, BUBBLE_SCALE);
	}
}

void Papas::Game::TakeOrder(Customer* customer)
{
	if (customer == nullptr)
	{
		// The customer left mid-order somehow; bail out of the flow.
		takingOrder = false;
		return;
	}
	int customerNum = customer->getType();

	// Draw wallpaper, customer, then counter; transparent pixels still write depth.
	Papas::Stereo::plane(1.0f);
	drawBackdrop(to_wallpaper, 0, 0, 0);
	Papas::Stereo::plane(0.5f);
	customer->renderOrdering(0.0f);
	//Roy2.renderAnimWithPauses(5, 2000);

	// On the first frame, bind the receipt and open the speech balloon.
	if(to_firstRun == false){
		to_n_actions = map_customers[customerNum].items.size();
		r_manager.getDockedReceipt(&to_tempReceipt);
		to_tempReceipt->customerType = customerNum;
		to_tempReceipt->customerNumber = customer->getNumber();
		to_orderStartedAt = Papas::Clock::now();
		to_bubbleKind = BubbleOpening;
		customer->playPresentation("stand");

		// drawCustomerOrdering(): another one on their tally for the file
		SaveData &sv = SaveManager::getInstance().data;
		if (customerNum > 0 && customerNum < (int)(sizeof(sv.customerServed) / sizeof(sv.customerServed[0])))
			sv.customerServed[customerNum]++;

		// Their name gets printed on the counter next to the star row
		C2D_TextBufClear(nameTextBuf);
		C2D_TextFontParse(&takeOrderNameText, dokyo, nameTextBuf, map_customers[customerNum].name.c_str());
		C2D_TextOptimize(&takeOrderNameText);

		to_firstRun = true;
	}

	if (to_bubbleKind == BubbleOpening && Papas::Clock::now() - to_orderStartedAt < 700)
	{
		Papas::Stereo::plane(0.0f);
		C2D_DrawImageAt(to_counter, 0, 0, TO_COUNTER_DEPTH);
		renderTakeOrderStars(customerNum);
		// Hold Roy's first frame while the balloon opens.
		Papas::Stereo::plane(0.05f);
		Roy2.renderCurrentFrame();
		Papas::Stereo::plane(-0.12f);
		renderOrderBubble();
		return;
	}
	if (to_bubbleKind == BubbleOpening)
	{
		to_bubbleKind = BubbleHidden;
		customer->playPresentation("takeorder");
	}

	Papas::Stereo::plane(0.0f);
	C2D_DrawImageAt(to_counter, 0, 0, TO_COUNTER_DEPTH);
	renderTakeOrderStars(customerNum);
	//Returns true for one frame on the first frame, and when the animation is paused
	Papas::Stereo::plane(0.05f);
	if (Roy2.renderAnimWithPauses(to_n_actions + 2, ROY_ORDER_PAUSE_MS))
	{
		if (to_currentAction < to_n_actions)
		{
			Papas::ResourceManager::getInstance().playSfx("writepencil");
			Papas::ResourceManager::getInstance().playSfx("talkbubble");
			setToppingBubble(map_customers[customerNum].items[to_currentAction]);
			to_tempReceipt->addItem(map_customers[customerNum].items[to_currentAction].Coverage,
									map_customers[customerNum].items[to_currentAction].Topping,
									map_customers[customerNum].items[to_currentAction].Quantity);
		}

		if (to_currentAction == to_n_actions)
		{
			Papas::ResourceManager::getInstance().playSfx("writepencil");
			Papas::ResourceManager::getInstance().playSfx("talkbubble");
			// Read the notch off the customer; the day's last one wants it quick
			to_bubbleKind = BubbleTime;
			to_bubbleTime = customer->getCookTime();
			to_tempReceipt->cookNotch = to_bubbleTime;
			to_tempReceipt->addTime(to_bubbleTime);
		}

		if (to_currentAction == to_n_actions + 1)
		{
			Papas::ResourceManager::getInstance().playSfx("writepencil");
			Papas::ResourceManager::getInstance().playSfx("talkbubble");
			to_bubbleKind = BubbleCut;
			to_bubbleCuts = map_customers[customerNum].CutPizzaIn;
			to_tempReceipt->addCut(map_customers[customerNum].CutPizzaIn);
		}

		if (to_currentAction == to_n_actions + 2)
		{
			to_bubbleKind = BubbleHidden;
			customer->playPresentation("stand");
			// finishTakingOrder(): the clock and the queue snapshot both start here
			to_tempReceipt->orderStartedAt = Papas::Clock::now();
			to_tempReceipt->lineWaitMs = Papas::Clock::now() - customer->getEnteredAt();
			to_tempReceipt->ordersAheadOfMe = c_manager.ordersAhead();
			to_firstRun = false;
			to_currentAction = 0;
			to_n_actions = 0;
			to_tempReceipt = nullptr;
			Roy2.resetAnim();
			takingOrder = false;
			Papas::ResourceManager::getInstance().switchMusic("orders_music");
			// Order's on the ticket: customer walks off to the wait line
			c_manager.orderTaken();
		}

		if(takingOrder == true){
			to_currentAction++;
		}
		
	}

	Papas::Stereo::plane(-0.12f);
	renderOrderBubble();

}

PapasError Papas::Game::render_bottom()
{

	// Cross-fade a 2x2 coal bed behind the grate so its cut-out slats reveal it.
	if (currentStation == BakingStation)
	{
		static const float COAL_X[4] = {14.0f, 156.0f, 14.0f, 156.0f};
		static const float COAL_Y[4] = {18.0f, 18.0f, 123.0f, 123.0f};
		// Stretch the compact 128x96 tiles back over their 142x105 area.
		static const float COAL_SX = 142.0f / 128.0f;
		static const float COAL_SY = 105.0f / 96.0f;
		// Slow the source twinkle into a gentler whole-bed glow.
		static const float COAL_PERIOD_MS = 1600.0f;

		float phase = (float)(Papas::Clock::now() % (u64)COAL_PERIOD_MS) / COAL_PERIOD_MS;
		C2D_ImageTint glow;
		C2D_AlphaImageTint(&glow, 0.5f - 0.5f * cosf(2.0f * M_PI * phase));
		for (int i = 0; i < 4; i++)
		{
			C2D_DrawImageAt(coalsDimImg, COAL_X[i], COAL_Y[i], 0.004f, nullptr, COAL_SX, COAL_SY);
			C2D_DrawImageAt(coalsBrightImg, COAL_X[i], COAL_Y[i], 0.005f, &glow, COAL_SX, COAL_SY);
		}
	}
	C2D_DrawImageAt(currentStationImg, 0, 0, 0.01f);

	// Same running order as up top: overlays first, then the day's set pieces
	if (overlay == OverlayHelp)
	{
		helpBook.renderBottom();
		return PAPAS_OK;
	}
	if (overlay == OverlayFile)
	{
		renderFileBottom();
		return PAPAS_OK;
	}
	if (overlay == OverlayPause)
	{
		renderPauseBottom();
		return PAPAS_OK;
	}

	if (showingEndOfDay)
	{
		renderEndOfDayBottom();
		return PAPAS_OK;
	}
	if (showingDayIntro || showingNewCustomer)
	{
		// Station art only; no buttons while the intro plays
		return PAPAS_OK;
	}
	if (showingResult)
	{
		renderResult();
		return PAPAS_OK;
	}

	if (currentStation == TicketStation){
		r_manager.renderReceipt(false);
		// Close the sharp 1:1 door sign after the day's last customer arrives.
		static const float SIGN_FOOT_X = 25.0f;
		static const float SIGN_FOOT_Y = 240.0f;
		C2D_DrawImageAt(c_manager.allSpawned() ? signClosedImg : signOpenImg,
			roundf(SIGN_FOOT_X - SIGN_FOOT_OX), roundf(SIGN_FOOT_Y - SIGN_FOOT_OY), 0.5f);
	}

	// The button only pops up once a customer has actually reached the counter
	if (currentStation == TicketStation && takingOrder == false &&
		c_manager.getOrderingCustomer() != nullptr)
	{
		if(createReceipt.showButton(touch)){
			takingOrder = true;
			// Its own little theme while they reel the order off
			Papas::ResourceManager::getInstance().switchMusic("takeorder_music");
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

	// Respond to user input
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	// Paused means paused: no timers, no walking, nothing but the menu
	if (overlay != OverlayNone)
	{
		updatePause(kDown);
		return PAPAS_OK; // updatePause may have changed scene; touch nothing else
	}

	pz_manager.updateTimers();

	if (kDown & KEY_START)
	{
		togglePause();
		return PAPAS_OK;
	}
	if (kDown & KEY_SELECT)
		return PAPAS_EXIT_REQUESTED;

	if (showingEndOfDay)
	{
		updateEndOfDay();
		bool touching = touch.px != 0 || touch.py != 0;
		bool onContinue = touch.px >= 100 && touch.px <= 220 && touch.py >= 148 && touch.py <= 182;
		bool ready = endDayPhase == EndDayBoard || endDayPhase == EndDayReady;
		if (ready && touching && onContinue) resultTouchHeld = true;
		if (ready && ((!touching && resultTouchHeld) || (kDown & KEY_A)))
		{
			resultTouchHeld = false;
			if (endDayPhase == EndDayBoard)
			{
				// moveScreen(): board's been read, now show what it earned
				endDayPhase = EndDayTips;
				endDayPhaseStarted = Papas::Clock::now();
				Papas::ResourceManager::getInstance().playSfx("cash");
			}
			else
			{
				startNextDay();
			}
		}
		return PAPAS_OK;
	}

	if (showingNewCustomer)
	{
		u64 elapsed = Papas::Clock::now() - newCustomerStartedAt;
		int frames = newCustomerNoPapa ? NEWCUSTOMER_NOPAPA_SRC_FRAMES : NEWCUSTOMER_SRC_FRAMES;
		u64 duration = (u64)((float)frames * 1000.0f / NEWCUSTOMER_FPS);
		bool skip = elapsed > 400 && ((kDown & KEY_A) || touch.px != 0 || touch.py != 0);
		if (elapsed >= duration || skip)
			endNewCustomer();
		return PAPAS_OK;
	}

	if (showingDayIntro)
	{
		u64 elapsed = Papas::Clock::now() - dayIntroStartedAt;
		u64 duration = (u64)((float)STARTOFDAY_SRC_FRAMES * 1000.0f / STARTOFDAY_FPS) + 400;
		bool skip = elapsed > 400 && ((kDown & KEY_A) || touch.px != 0 || touch.py != 0);
		if (elapsed >= duration || skip)
			endDayIntro();
		return PAPAS_OK;
	}

	if (showingResult)
	{
		updateResult();
		bool touching = touch.px != 0 || touch.py != 0;
		bool onContinue = touch.px >= 100 && touch.px <= 220 && touch.py >= 198 && touch.py <= 232;
		if (resultPhase == ResultReady && touching && onContinue)
			resultTouchHeld = true;
		if (resultPhase == ResultReady && ((!touching && resultTouchHeld) || (kDown & KEY_A)))
		{
			c_manager.completeOrder(resultCustomerNumber);
			resultCustomerNumber = 0;
			showingResult = false;
			resultTouchHeld = false;
			freeGiveOrderSheets();
			// Finish the day before switching stations so its music stays gated.
			if (c_manager.dayIsOver())
				beginEndOfDay();
			SwitchStation(TicketStation);
		}
		return PAPAS_OK;
	}

	// Wrap left from orders to cutting and right from cutting to orders.
	if (kDown & KEY_L)
	{
		if (currentStation == CuttingStation)
		{
			pz_manager.cancelCutting();
		}
		SwitchStation(currentStation == TicketStation
			? CuttingStation
			: (Stations)(currentStation - 1));
	}

	if (kDown & KEY_R)
	{
		if (currentStation == CuttingStation)
		{
			pz_manager.cancelCutting();
			SwitchStation(TicketStation);
		}
		else
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
		// During the day intro or the wrap-up the music is somebody else's problem
		if (!showingDayIntro && !showingNewCustomer && !showingEndOfDay)
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
	C2D_SpriteSheetFree(orderBubbleSheet);
	C2D_SpriteSheetFree(resultSheet);
	freeGiveOrderSheets();
	if (startOfDaySheet != nullptr)
	{
		C2D_SpriteSheetFree(startOfDaySheet);
		startOfDaySheet = nullptr;
	}
	if (dayNumberSheet != nullptr)
	{
		C2D_SpriteSheetFree(dayNumberSheet);
		dayNumberSheet = nullptr;
	}
	if (newCustomerSheet != nullptr)
	{
		C2D_SpriteSheetFree(newCustomerSheet);
		newCustomerSheet = nullptr;
	}
	CustomerRig::getInstance().freeType(newCustomerAtlas);
	CustomerRig::getInstance().freeType(fileAtlas);
	freePauseSheet();
	helpBook.terminate();
	C2D_SpriteSheetFree(starsSheet);
	C2D_TextBufDelete(endDayTextBuf);
	C2D_TextBufDelete(fileTextBuf);
	C2D_TextBufDelete(uiTextBuf);
	C2D_TextBufDelete(nameTextBuf);
	C2D_TextBufDelete(newCustomerTextBuf);
	C2D_TextBufDelete(orderBubbleTextBuf);
	C2D_TextBufDelete(resultTextBuf);
	C2D_FontFree(dokyo);

	return PAPAS_OK;
}
