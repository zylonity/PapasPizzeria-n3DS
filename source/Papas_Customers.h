#pragma once
//===============================================================================
// name: Papas_Customers.h
// desc: Customer data (orders) + the live customer/line system, ported from
//       the original game's Customer.as / CustomerManager.as. Customers are
//       drawn with the shared skeletal rig (Papas_CustomerRig.*); each live
//       customer lazy-loads its own type's limb atlas so only on-screen
//       types cost VRAM.
//===============================================================================

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

	// A live customer somewhere in the lobby (Customer.as). The manager owns
	// these and moves them between lines; the customer just walks/stands/draws.
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
		void initManager();
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

	private:
		void decideLineup();
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
//===============================================================================
