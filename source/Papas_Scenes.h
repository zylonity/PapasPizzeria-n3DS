#pragma once
#include "Papas_Constants.h"
#include "Papas_SceneManager.h"
#include "Papas_Utils.h"
#include "Papas_Customers.h"
#include "Papas_Pizza.h"
#include <3ds.h>
#include <citro2d.h>
#include <vector>
#include <chrono>


namespace Papas {

	// Layered intro cutscene; B skips it.
	class IntroCutscene : public Scene
	{
	public:
		PapasError init(Papas::SceneManager *sceneManager) override;
		PapasError update() override;
		PapasError render_top() override;
		PapasError render_bottom() override;
		PapasError terminate() override;

	private:
		Papas::SceneManager *p_sceneManager;
		C2D_SpriteSheet sheet_bg;
		C2D_Image skip_bg;
		C2D_SpriteSheet introSheets[4]; // INTRO_SHEET_COUNT checked in the .cpp
		u64 startedAt;
	};

	class MainMenu : public Scene {
	public:
		PapasError init(Papas::SceneManager* sceneManager) override;
		PapasError update() override;
		PapasError render_top() override;
		PapasError render_bottom() override;
		PapasError terminate() override;
	private:
		Papas::SceneManager* p_sceneManager;
		//backgrounds
		C2D_SpriteSheet sheet_bg;
		C2D_Image top_bg;
		C2D_Image bottom_bg;
		C2D_Image logo;
		//icons
		

		touchPosition touch;

		//buttons
		int buttonIndex;
		bool aPressed = false;
		C2D_SpriteSheet sheet_buttons;
		std::vector<Papas::Button> v_buttons;

	};

	// Three-slot picker: B backs out, and pressing X twice deletes a slot.
	class SaveSelect : public Scene
	{
	public:
		PapasError init(Papas::SceneManager *sceneManager) override;
		PapasError update() override;
		PapasError render_top() override;
		PapasError render_bottom() override;
		PapasError terminate() override;

	private:
		void refreshSlotText();
		PapasError activateSlot(int slot); // changeScene: caller must return immediately

		Papas::SceneManager *p_sceneManager;
		C2D_SpriteSheet sheet_bg;
		C2D_Image top_bg;
		C2D_Image bottom_bg;
		C2D_Image logo;

		// One receipt per slot on the top screen; the selected one pops up
		C2D_SpriteSheet sheet_receipt;
		C2D_Image receiptImg;
		float popAmount[3];	// 0 = tucked behind the counter, 1 = popped up

		C2D_Font dokyo;
		C2D_TextBuf textBuf;
		C2D_Text headerText;
		C2D_Text hintText;
		C2D_Text deleteText;
		C2D_Text slotTitleText[3];
		C2D_Text slotInfoText[3];
		C2D_Text receiptDayText[3];		// "Day N", or "New Game" on a blank ticket
		C2D_Text receiptRankText[3];	// "Rank N", used slots only

		bool slotUsed[3];
		int selected;
		int deleteArmed;	// slot waiting for the confirming X press, -1 = none
		touchPosition touch;
	};

	// How to play, off the main menu; the pause overlay draws the same book
	class HelpScreen : public Scene
	{
	public:
		PapasError init(Papas::SceneManager *sceneManager) override;
		PapasError update() override;
		PapasError render_top() override;
		PapasError render_bottom() override;
		PapasError terminate() override;

	private:
		Papas::SceneManager *p_sceneManager;
		C2D_SpriteSheet sheet_bg;
		C2D_Image top_bg;
		C2D_Image bottom_bg;
		C2D_Font dokyo;
		HelpBook book;
		touchPosition touch;
	};

	// Who made the thing, off the main menu
	class CreditsScreen : public Scene
	{
	public:
		PapasError init(Papas::SceneManager *sceneManager) override;
		PapasError update() override;
		PapasError render_top() override;
		PapasError render_bottom() override;
		PapasError terminate() override;

	private:
		Papas::SceneManager *p_sceneManager;
		C2D_SpriteSheet sheet_bg;
		C2D_Image top_bg;
		C2D_Image bottom_bg;
		C2D_Image logo;
		C2D_Font dokyo;
		C2D_TextBuf textBuf;
		C2D_Text lines[10];
		int lineCount;
		C2D_Text hintText;
	};

	// The actual game: all four stations, customers, pizzas, the whole day
	class Game : public Scene
	{
	public:
		PapasError init(Papas::SceneManager* sceneManager) override;
		PapasError update() override;
		PapasError render_top() override;
		PapasError render_bottom() override;
		PapasError terminate() override;
		void TakeOrder(Customer* customer);

	private:
		// The bottom screen workstations, L/R shoulder buttons move between them
		enum Stations
		{
			TicketStation,
			ToppingStation,
			BakingStation,
			CuttingStation,
			StationMAX
		};

		Stations currentStation;
		C2D_SpriteSheet s_stations;

		// Roy peeking over the till / writing down orders
		RoyPeeking Roy;
		RoyTakingOrder Roy2;

		C2D_Image ticketsStationImg;
		C2D_Image ticketsHolderImg;
		// Cross-faded coal bed behind the oven grate.
		C2D_Image coalsDimImg;
		C2D_Image coalsBrightImg;
		Button createReceipt;

		// Take-order top screen art (wallpaper behind, counter in front)
		C2D_SpriteSheet orderStation;
		bool takingOrder;
		C2D_Image to_wallpaper;
		C2D_Image to_counter;

		C2D_Image currentStationImg;
		C2D_Image currentPopupImg;

		void SwitchStation(Stations station);

		Papas::SceneManager* p_sceneManager;

		// Receipt system stuff to move later
		C2D_Font dokyo;

		ReceiptManager r_manager;

		touchPosition touch;

		//Taking order stuff
		bool to_firstRun;
		int to_currentAction;		// which line of the order we're up to
		int to_n_actions;
		Receipt *to_tempReceipt;
		// The speech balloon over the customer's head, one kind per order line
		enum OrderBubbleKind { BubbleOpening, BubbleTopping, BubbleTime, BubbleCut, BubbleHidden };
		OrderBubbleKind to_bubbleKind;
		u64 to_orderStartedAt;
		ItemOrder to_bubbleItem;
		int to_bubbleTime;
		int to_bubbleCuts;
		C2D_SpriteSheet orderBubbleSheet;
		C2D_Image orderBubbleBase;
		C2D_Image orderBubbleToppings[7];
		C2D_Image orderBubbleCoverage[16];	// all 16 quadrant combos
		C2D_Image orderBubbleClocks[8];
		C2D_Image orderBubbleCuts[3];		// 4/6/8 slice diagrams
		C2D_Image orderBubbleOpening[15];	// unused, procedural bounce instead
		C2D_TextBuf orderBubbleTextBuf;
		C2D_Text orderBubbleX;
		C2D_Text orderBubbleQuantity;
		void setToppingBubble(const ItemOrder &item);
		void renderOrderBubble();

		// Customers in the lobby (order/wait lines, take-order screen)
		CustomerManager c_manager;

		// Pizzas being topped at the topping station
		PizzaManager pz_manager;
		Receipt *dockedReceipt();

		// The four category scores plus tip for a served order
		struct OrderResult
		{
			int waiting;
			int topping;
			int baking;
			int cutting;
			int overall;
			int tipCents;
			std::string customerName;
		};

		// Start-of-day intro (storefront cutscene) + door sign state
		bool showingDayIntro;
		u64 dayIntroStartedAt;
		int dayIntroPick;
		int currentDay;
		// Rank up when tips pass lastRankLimit + (rank + 1) * $5.
		int myRank;
		int lastRankLimit;
		void beginDayIntro();
		void startNextDay();
		C2D_SpriteSheet startOfDaySheet;
		// Use baked plate digits because the game font is too small.
		C2D_SpriteSheet dayNumberSheet;
		C2D_Image dayDigits[10];
		void renderDayNumber(float centreX, float inkCentreY);
		C2D_Image signOpenImg;
		C2D_Image signClosedImg;
		void renderDayIntro();
		void endDayIntro();

		// Show NEW CUSTOMER! before the day; rank 31+ can use the no-Papa version.
		bool showingNewCustomer;
		bool newCustomerNoPapa;
		int newCustomerType;
		u64 newCustomerStartedAt;
		C2D_SpriteSheet newCustomerSheet;
		RigTypeAtlas newCustomerAtlas;
		C2D_TextBuf newCustomerTextBuf;
		C2D_Text newCustomerNameText;
		float newCustomerNameScale;
		// Returns false if there is nothing to introduce today
		bool beginNewCustomer();
		void renderNewCustomer();
		void endNewCustomer();
		// Both splashes lead into the day intro; this picks between them
		void beginDayOrNewCustomer();

		// Serving results: drumroll -> customer looks -> reacts -> tip -> Continue
		bool showingResult;
		bool resultTouchHeld;
		enum ResultPhase { ResultDrumroll, ResultLook, ResultReaction, ResultTip, ResultReady };
		ResultPhase resultPhase;
		u64 resultPhaseStarted;		// resets on every phase change
		u64 resultStartedAt;		// fixed, drives Roy's animation
		// Load Roy's large give-order atlases only while results are visible.
		C2D_SpriteSheet giveOrderSheets[8];
		bool giveOrderLoaded;
		int giveOrderPick;
		OrderResult result;
		int totalScore;
		int totalTipsCents;			// whole session, also drives the rank-ups
		int resultCustomerNumber;
		int resultPizzaId;
		std::string resultReaction;	// which rig segment/sound plays
		C2D_SpriteSheet resultSheet;
		C2D_Image resultJar;
		C2D_Image resultCoinPiles[10];	// jar fill levels
		C2D_Image resultCoinSpin[6];	// the coin flipping into the jar
		C2D_TextBuf resultTextBuf;
		C2D_Text resultText[8];
		// Scores earn or clear stars; five stars make one of up to three seals.
		C2D_SpriteSheet starsSheet;
		C2D_Image starEmptyImg;
		C2D_Image starFilledImg;
		C2D_Image starFlashImg;
		C2D_Image sealImg;
		int resultCustomerType;
		int resultStarsBefore;
		int resultSealsBefore;
		int resultStarEarned;	// 1..5 = which star lights up, 0 = none
		bool resultStarsLost;
		bool resultSealEarned;
		bool resultStarSfxPlayed;
		C2D_TextBuf nameTextBuf;
		C2D_Text takeOrderNameText;
		void renderStarRow(float centerX, float y, int stars, int seals, float depth);
		void renderResultStars();
		void renderTakeOrderStars(int customerType);

		// Everything the pause button can put on top of the game
		enum Overlay { OverlayNone, OverlayPause, OverlayFile, OverlayHelp };
		Overlay overlay;
		int pauseIndex;
		C2D_TextBuf uiTextBuf;		// scratch for menu labels rebuilt every frame
		void togglePause();
		void leaveOverlay();		// back to the pause menu, or back to work
		void updatePause(u32 kDown);
		void renderPauseTop();
		void renderPauseBottom();

		// Customer file: a grid of everyone we've met plus the selected profile
		int fileSelected;			// customer type on the card, 0 = nobody yet
		RigTypeAtlas fileAtlas;		// portrait art, only for whoever's showing
		C2D_TextBuf fileTextBuf;
		C2D_Text fileText[5];
		void openFile();
		void closeFile();
		void selectFileCustomer(int type);
		void renderFileTop();
		void renderFileBottom();

		// The same help book the main menu shows
		HelpBook helpBook;

		// End-of-day wrap-up: board, tips, rank, then off to bed
		bool showingEndOfDay;
		enum EndDayPhase { EndDayBoard, EndDayTips, EndDayRank, EndDayReady };
		EndDayPhase endDayPhase;
		u64 endDayPhaseStarted;
		bool endDayRankUp;			// decided when the tips land, applied at the rank step
		int customersToday;
		int waitingToday, toppingToday, bakingToday, cuttingToday;
		int tipsTodayCents;
		C2D_TextBuf endDayTextBuf;
		C2D_Text endDayText[13];
		void beginEndOfDay();
		void prepareEndDayText();
		void updateEndOfDay();
		void renderEndOfDayTop();
		void renderEndOfDayBottom();

		void completeServedPizza(Pizza &pizza);
		void prepareResultText();
		void renderResult();
		void updateResult();
		void renderTipJar();
		void loadGiveOrderSheets();
		void freeGiveOrderSheets();
		void renderGiveOrderRoy();
		int scoreWaiting(const Pizza &pizza, const CustomerData &order) const;
		int scoreToppings(const Pizza &pizza, const CustomerData &order) const;
		int scoreBaking(const Pizza &pizza, const CustomerData &order) const;
		int scoreCutting(const Pizza &pizza, const CustomerData &order) const;
	};
}
