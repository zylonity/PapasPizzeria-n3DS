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

	class IntroVid : public Scene
	{
	public:
		PapasError init(Papas::SceneManager *sceneManager) override;
		PapasError update() override;
		PapasError render_top() override;
		PapasError render_bottom() override;
		PapasError terminate() override;

	private:
		bool startedPlaying;
		Papas::SceneManager *p_sceneManager;
		C2D_SpriteSheet sheet_bg;
		C2D_Image skip_bg;
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

		RoyPeeking Roy;
		RoyTakingOrder Roy2;

		C2D_Image ticketsStationImg;
		C2D_Image ticketsHolderImg;
		Button createReceipt;
		
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
		int to_currentAction;
		int to_n_actions;
		Receipt *to_tempReceipt;
		enum OrderBubbleKind { BubbleOpening, BubbleTopping, BubbleTime, BubbleCut, BubbleHidden };
		OrderBubbleKind to_bubbleKind;
		u64 to_orderStartedAt;
		ItemOrder to_bubbleItem;
		int to_bubbleTime;
		int to_bubbleCuts;
		C2D_SpriteSheet orderBubbleSheet;
		C2D_Image orderBubbleBase;
		C2D_Image orderBubbleToppings[7];
		C2D_Image orderBubbleCoverage[16];
		C2D_Image orderBubbleClocks[8];
		C2D_Image orderBubbleCuts[3];
		C2D_Image orderBubbleOpening[15];
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

		bool showingResult;
		bool resultTouchHeld;
		enum ResultPhase { ResultDrumroll, ResultLook, ResultReaction, ResultTip, ResultReady };
		ResultPhase resultPhase;
		u64 resultPhaseStarted;
		u64 resultStartedAt;
		// Roy give-order clip: ~5 MB of atlases, loaded only while a result
		// shows (capacity checked against GIVEORDER_SHEET_COUNT in the .cpp)
		C2D_SpriteSheet giveOrderSheets[8];
		bool giveOrderLoaded;
		int giveOrderPick;
		OrderResult result;
		int totalScore;
		int totalTipsCents;
		int resultCustomerNumber;
		int resultPizzaId;
		std::string resultReaction;
		C2D_SpriteSheet resultSheet;
		C2D_Image resultJar;
		C2D_Image resultCoinPiles[10];
		C2D_Image resultCoinSpin[6];
		C2D_TextBuf resultTextBuf;
		C2D_Text resultText[8];
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
