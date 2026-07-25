#pragma once
// Customer orders and live queues drawn with the shared rig.

#include "Papas_Constants.h"
#include "Papas_Utils.h"
#include "Papas_CustomerRig.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace Papas {

	struct CustomerData
	{
		std::string name;
		std::vector<ItemOrder> items;
		int time;
		int CutPizzaIn;
	};

	// Index -> order data for all 36 customer types (defined in Papas_Customers.cpp)
	extern std::unordered_map<int, CustomerData> map_customers;

	// A lobby customer; the manager owns it and moves it between lines.
	class Customer
	{
	public:
		enum State
		{
			EnteringOrderLine,	// walking in from the right to the order carpet
			AtOrderLine,		// standing in the order line
			Leaving,			// order taken: walking off to the right (flipped)
			EnteringWaitLine,	// walking back in to the pick-up carpet
			AtWaitLine			// standing in the wait line
		};

		void spawnCustomer(int typeId, int number, int lineIndex); // loads the type atlas
		void despawnCustomer();                                    // frees the type atlas

		void update(float deltaSeconds);
		void render(float depth);         // draw in the lobby (order/wait line)
		void renderOrdering(float depth); // draw large on the take-order screen
		void playPresentation(const char* segment);

		void moveOrderDone();                  // -> Leaving
		void moveToWaitLine(int lineIndex);    // -> EnteringWaitLine
		void shiftOrderLine(int lineIndex);    // someone ahead left; new target
		void shiftWaitLine(int lineIndex);

		State getState() const { return state; }
		int   getType() const { return type; }
		int   getNumber() const { return number; }
		bool  isWalking() const { return walking; }

	private:
		void startSegment(const char* name);
		float animSeconds() const;

		RigTypeAtlas atlasLine;   // lobby-sized art, held while spawned
		RigTypeAtlas atlasOrder;  // take-order-sized art, held only while ordering
		int type = 1;
		int number = 0;

		State state = EnteringOrderLine;
		float x = 0;
		float y = 0;
		float targetX = 0;
		int   walkDir = -1;      // -1 = walking left (entering), 1 = right (leaving)
		bool  walking = false;
		bool  flipped = false;   // leaving customers face right (negative scaleX)

		int currentSeg = -1;     // rig segment being played
		std::chrono::steady_clock::time_point segStart;
		int presentationSeg = -1;
		std::chrono::steady_clock::time_point presentationStart;
	};

	// Spawns the day's lineup and runs the order/wait lines (CustomerManager.as)
	class CustomerManager
	{
	public:
		void initManager(int rank);
		void terminateManager();

		void update();               // spawn timer + walking
		void renderLines(float depth); // lobby customers on the top screen

		// Take-order flow: the customer at the front of the order line.
		Customer* getOrderingCustomer();     // nullptr if none has arrived yet
		Customer* getCustomer(int customerNumber);
		void orderTaken();                   // front customer leaves -> wait line
		void completeOrder(int customerNumber); // remove a served customer from the wait line

		bool dayIsOver() const;
		// True once the day's whole lineup has walked in (flips the door sign)
		bool allSpawned() const { return totalCustomers >= (int)customerLineup.size(); }

		// Keep unlock rules static so the pre-day splash can use them.
		// Six starters, then one per rank; Papa is type 36.
		static int  unlockedCount(int rank);
		// PAPA LOUIE!: rank 31+ with 3 gold seals on all 35 other customers
		static bool papaEarned();
		// Rank 31+ but not there yet - shows the "no papa" splash instead
		static bool papaBlocked(int rank);
		// Find today's unseen lead unlock, or return 0.
		static int  newCustomerToday(int rank);

	private:
		void decideLineup(int rank);
		void spawnNext();

		std::vector<int> customerLineup;      // types, in spawn order
		std::vector<Customer*> v_customers;   // owns every spawned customer
		std::vector<Customer*> orderline;
		std::vector<Customer*> waitline;

		int totalCustomers = 0;
		float spawnSpeed = 0;                 // seconds between spawns
		std::chrono::steady_clock::time_point lastSpawnTime;
		std::chrono::steady_clock::time_point lastUpdate;
	};
}
