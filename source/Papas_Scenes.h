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

	// The intro cutscene (car ride -> pizzeria -> Papa's note), played back
	// as layered animation from the original clip's timeline (see
	// tools/gen_intro.py). Replaces the old prerecorded ready.ogv. B skips.
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

	// Save-slot picker between the main menu and the game: three files on
	// the bottom screen. An empty slot starts a new game (intro cutscene
	// first); a used slot loads straight into the day. B backs out, X twice
	// deletes the selected file.
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
		// Rank progression (EndDayScreen.as): rank-up when total tips cross
		// lastRankLimit + (rank+1)*500 cents at the end of a day
		int myRank;
		int lastRankLimit;
		void beginDayIntro();
		void startNextDay();
		C2D_SpriteSheet startOfDaySheet;
		C2D_TextBuf dayTextBuf;
		C2D_Text dayNumText;
		C2D_Image signOpenImg;
		C2D_Image signClosedImg;
		void renderDayIntro();
		void endDayIntro();

		// NEW CUSTOMER! splash (NewCustomerScreen.as), shown ahead of the day
		// intro the first time a newly unlocked customer is due. Its no-papa
		// sibling takes over at rank 31+ until every customer is fully sealed.
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
		// Roy give-order clip: ~5 MB of atlases, loaded only while a result
		// shows (capacity checked against GIVEORDER_SHEET_COUNT in the .cpp)
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
		// Star-customer system (GiveOrderScreen.as): each customer type earns
		// a star for a >=80 order, loses them all under 60; five stars mint a
		// gold seal (max 3) that raises their max tip by $1 each
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
