//===============================================================================
// name: Papas_Customers.cpp
// desc: Live customer/line system, ported from Customer.as/CustomerManager.as.
//===============================================================================

#include "Papas_Customers.h"
#include "Papas_ResourceManager.h"
#include <algorithm>
#include <cmath>

//===============================================================================
// Lobby layout, ported from Customer.as. The original stage is 600x450 and the
// top screen is 400x240, so positions map by x*0.667, y*0.533 (the rig origin
// is at the top of the figure; a full-size figure is ~322px tall on the
// original stage). All of these are hand-tunable.
//===============================================================================
#define ENTER_OFFSCREEN_X   420.0f	// spawn/walk-in x, fully off the right edge
#define LEAVE_OFFSCREEN_X   430.0f	// walk-off x before rejoining the wait line

#define ORDERLINE_X         73.0f	// 110 * 0.667
#define ORDERLINE_Y         111.0f	// 208 * 0.533
#define ORDERLINE_OFFSET    57.0f	// 86 * 0.667
#define ORDERLINE_SCALE     0.32f	// 0.6 (Flash sizepercent) * 0.533

#define WAITLINE_X          138.0f	// 207 * 0.667
#define WAITLINE_Y          52.0f	// 97 * 0.533
#define WAITLINE_OFFSET     44.0f	// 66 * 0.667
#define WAITLINE_SCALE      0.27f	// 0.5 * 0.533

#define LEAVELINE_X         183.0f	// 274 * 0.667
#define LEAVELINE_Y         77.0f	// 145 * 0.533
#define LEAVELINE_SCALE     0.32f	// 0.6 * 0.533

#define TAKEORDER_X         150.0f	// stands behind the counter, right of Roy
#define TAKEORDER_Y         -8.0f	// lower body hidden by the countertop
#define TAKEORDER_SCALE     0.65f

#define WALK_SPEED          81.0f	// px/s: 4px per 33ms on the original stage * 0.667

#define CUSTOMERS_PER_DAY   4		// numberOfCustomers[rank 0] in CustomerManager.as
#define TIME_PER_DAY        90.0f	// seconds; spawnSpeed = day / (customers - 1)

//===============================================================================
// Customer
//===============================================================================

void Papas::Customer::spawnCustomer(int typeId, int num, int lineIndex)
{
	type = typeId;
	number = num;

	// VRAM win: only customers that are actually in the lobby hold their
	// type's limb atlas (and only the lobby-sized variant; the bigger
	// take-order art loads on demand in renderOrdering).
	Papas::CustomerRig::getInstance().loadType(type, atlasLine, "_line");

	// startCustomerEntering(): walk in from the right to the order carpet
	state = EnteringOrderLine;
	x = ENTER_OFFSCREEN_X;
	y = ORDERLINE_Y;
	targetX = ORDERLINE_X + lineIndex * ORDERLINE_OFFSET;
	walkDir = -1;
	walking = true;
	flipped = false;
	startSegment("walk");
}

void Papas::Customer::despawnCustomer()
{
	Papas::CustomerRig::getInstance().freeType(atlasLine);
	Papas::CustomerRig::getInstance().freeType(atlasOrder);
}

void Papas::Customer::startSegment(const char* name)
{
	currentSeg = Papas::CustomerRig::getInstance().segmentIndex(name);
	segStart = std::chrono::steady_clock::now();
}

float Papas::Customer::animSeconds() const
{
	return std::chrono::duration<float>(std::chrono::steady_clock::now() - segStart).count();
}

void Papas::Customer::update(float deltaSeconds)
{
	if (!walking)
		return;

	// walkCustomer(): step toward the target, stand when we get there
	x += walkDir * WALK_SPEED * deltaSeconds;

	if (walkDir == -1 && x <= targetX)
	{
		x = targetX;
		walking = false;
		startSegment("stand");
		if (state == EnteringOrderLine)
			state = AtOrderLine;
		else if (state == EnteringWaitLine)
			state = AtWaitLine;
	}
	else if (walkDir == 1 && x >= targetX)
	{
		// Leaving customers finish offscreen; the manager moves them to the
		// wait line from there.
		x = targetX;
		walking = false;
	}
}

void Papas::Customer::moveOrderDone()
{
	// Done ordering: the big take-order art won't be needed again
	Papas::CustomerRig::getInstance().freeType(atlasOrder);

	// moveCustomerOrderDone(): walk off to the right, mirrored
	state = Leaving;
	x = LEAVELINE_X;
	y = LEAVELINE_Y;
	targetX = LEAVE_OFFSCREEN_X;
	walkDir = 1;
	walking = true;
	flipped = true;
	startSegment("walk");
}

void Papas::Customer::moveToWaitLine(int lineIndex)
{
	// moveCustomerWaitLine(): walk back in to the pick-up carpet
	state = EnteringWaitLine;
	x = ENTER_OFFSCREEN_X;
	y = WAITLINE_Y;
	targetX = WAITLINE_X + lineIndex * WAITLINE_OFFSET;
	walkDir = -1;
	walking = true;
	flipped = false;
	startSegment("walk");
}

void Papas::Customer::shiftOrderLine(int lineIndex)
{
	targetX = ORDERLINE_X + lineIndex * ORDERLINE_OFFSET;
	if (!walking)
	{
		// shiftOrderLine(): standing customers just snap to their new spot
		x = targetX;
	}
}

void Papas::Customer::shiftWaitLine(int lineIndex)
{
	targetX = WAITLINE_X + lineIndex * WAITLINE_OFFSET;
	if (!walking)
	{
		x = targetX;
	}
}

void Papas::Customer::render(float depth)
{
	float scale;
	switch (state)
	{
	case Leaving:
		scale = LEAVELINE_SCALE;
		break;
	case EnteringWaitLine:
	case AtWaitLine:
		scale = WAITLINE_SCALE;
		break;
	default:
		scale = ORDERLINE_SCALE;
		break;
	}

	int frame = Papas::CustomerRig::getInstance().frameForTime(currentSeg, animSeconds());
	float scaleX = flipped ? -scale : scale; // Flash mirrors about x=0
	// Draw at whole pixels: a continuously-sweeping subpixel position makes
	// straight edges shimmer/crawl under bilinear filtering while walking.
	Papas::CustomerRig::getInstance().draw(atlasLine, type, frame,
										   roundf(x), roundf(y), scaleX, scale, depth);
}

void Papas::Customer::renderOrdering(float depth)
{
	// Lazy-load the take-order-sized art; at most one customer orders at a time
	if (!atlasOrder.sheet)
	{
		Papas::CustomerRig::getInstance().loadType(type, atlasOrder);
	}

	// drawCustomerOrdering(): stand large on the take-order screen
	int segStand = Papas::CustomerRig::getInstance().segmentIndex("stand");
	int frame = Papas::CustomerRig::getInstance().frameForTime(segStand, animSeconds());
	Papas::CustomerRig::getInstance().draw(atlasOrder, type, frame,
										   TAKEORDER_X, TAKEORDER_Y,
										   TAKEORDER_SCALE, TAKEORDER_SCALE, depth);
}

//===============================================================================
// CustomerManager
//===============================================================================

void Papas::CustomerManager::initManager()
{
	Papas::CustomerRig::getInstance().load();

	totalCustomers = 0;
	decideLineup();

	// setupSpawn(): first customer right away, the rest spread over the day
	spawnSpeed = TIME_PER_DAY / (customerLineup.size() - 1);
	lastUpdate = std::chrono::steady_clock::now();
	spawnNext();
}

void Papas::CustomerManager::terminateManager()
{
	for (size_t i = 0; i < v_customers.size(); i++)
	{
		v_customers[i]->despawnCustomer();
		delete v_customers[i];
	}
	v_customers.clear();
	orderline.clear();
	waitline.clear();
	customerLineup.clear();
}

void Papas::CustomerManager::decideLineup()
{
	// decideLineup(), minus the rank/unlock system for now: random distinct types
	customerLineup.clear();
	std::vector<int> pool;
	for (int t = 1; t <= 36; t++)
	{
		pool.push_back(t);
	}
	for (int i = 0; i < CUSTOMERS_PER_DAY && !pool.empty(); i++)
	{
		int pick = Papas::ResourceManager::getInstance().randomNumber(0, pool.size() - 1);
		customerLineup.push_back(pool[pick]);
		pool.erase(pool.begin() + pick);
	}
}

void Papas::CustomerManager::spawnNext()
{
	if (totalCustomers >= (int)customerLineup.size())
		return;

	Customer* c = new Customer();
	c->spawnCustomer(customerLineup[totalCustomers], totalCustomers + 1, orderline.size());
	totalCustomers++;
	v_customers.push_back(c);
	orderline.push_back(c);
	lastSpawnTime = std::chrono::steady_clock::now();
}

void Papas::CustomerManager::update()
{
	auto now = std::chrono::steady_clock::now();
	float deltaSeconds = std::chrono::duration<float>(now - lastUpdate).count();
	lastUpdate = now;

	// spawn timer
	if (totalCustomers < (int)customerLineup.size() &&
		std::chrono::duration<float>(now - lastSpawnTime).count() >= spawnSpeed)
	{
		spawnNext();
	}

	for (size_t i = 0; i < v_customers.size(); i++)
	{
		Customer* c = v_customers[i];
		c->update(deltaSeconds);

		// A leaving customer that walked offscreen rejoins on the wait line.
		if (c->getState() == Customer::Leaving && !c->isWalking())
		{
			c->moveToWaitLine(waitline.size());
			waitline.push_back(c);
		}
	}
}

void Papas::CustomerManager::renderLines(float depth)
{
	// Spawn order decides who draws on top (Flash used depth 300 + number).
	// Steps stay small so a full day's lineup fits under the ticket holder's
	// 0.002 depth.
	for (size_t i = 0; i < v_customers.size(); i++)
	{
		v_customers[i]->render(depth + v_customers[i]->getNumber() * 0.00004f);
	}
}

Papas::Customer* Papas::CustomerManager::getOrderingCustomer()
{
	if (orderline.empty())
		return nullptr;
	if (orderline[0]->getState() != Customer::AtOrderLine)
		return nullptr; // still walking up
	return orderline[0];
}

void Papas::CustomerManager::orderTaken()
{
	if (orderline.empty())
		return;

	Customer* front = orderline[0];
	orderline.erase(orderline.begin());
	front->moveOrderDone();

	// tellOthersToShift()
	for (size_t i = 0; i < orderline.size(); i++)
	{
		orderline[i]->shiftOrderLine(i);
	}
}

bool Papas::CustomerManager::dayIsOver() const
{
	return totalCustomers == (int)customerLineup.size() &&
		   orderline.empty() && waitline.empty();
}

//===============================================================================
// Customer order data (index, name, items, notch time, slices)
//===============================================================================
std::unordered_map<int, Papas::CustomerData> Papas::map_customers = {
	{1,                               // Index
	 {"Cooper",                       // Name
	  {{{0, 0, 1, 1}, Pepperoni, 4}}, // Vector/array of items the customer wants (Coverage, Topping, Ammount)
	  1,                              // Time (notch timer)
	  4}},                            // Slices
	{2, {"Wally", {{{1, 1, 1, 1}, Anochovie, 8}}, 2, 8}},
	{3, {"Rita", {{{0, 1, 1, 0}, Mushroom, 6}}, 4, 4}},
	{4, {"Marty", {{{1, 1, 0, 0}, Olive, 6}}, 3, 4}},
	{5, {"Kingsley", {{{0, 0, 1, 1}, Pepperoni, 8}}, 4, 4}},
	{6, {"Timm", {{{0, 0, 1, 1}, Pepper, 4}}, 4, 6}},
	{7, {"Big Pauly", {{{1, 1, 0, 0}, Meat, 4}, {{0, 0, 1, 1}, Onion, 4}}, 3, 8}},
	{8, {"Penny", {{{1, 1, 1, 1}, Meat, 8}, {{1, 0, 0, 0}, Mushroom, 2}}, 2, 6}},
	{9, {"Maggie", {{{0, 0, 1, 1}, Pepper, 4}, {{1, 1, 0, 0}, Olive, 6}}, 2, 4}},
	{10, {"Taylor", {{{1, 1, 0, 0}, Pepper, 2}, {{1, 1, 0, 0}, Onion, 6}}, 3, 4}},
	{11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
	{12, {"Allan", {{{1, 0, 0, 1}, Pepperoni, 4}, {{0, 1, 1, 0}, Meat, 4}}, 4, 6}},
	{13, {"Mindy", {{{1, 0, 0, 0}, Mushroom, 4}, {{1, 1, 0, 0}, Anochovie, 6}}, 5, 8}},
	{14, {"Chuck", {{{1, 1, 1, 1}, Pepperoni, 8}, {{0, 1, 1, 0}, Meat, 4}}, 2, 6}},
	{15, {"Greg", {{{0, 1, 1, 1}, Pepperoni, 6}, {{0, 0, 1, 0}, Mushroom, 4}}, 4, 4}},
	{16, {"Robby", {{{0, 1, 1, 1}, Mushroom, 6}, {{0, 0, 1, 1}, Pepper, 6}}, 4, 6}},
	{17, {"Mary", {{{1, 1, 1, 1}, Pepperoni, 8}}, 2, 4}},
	{18, {"Mitch", {{{1, 1, 0, 0}, Pepperoni, 4}, {{1, 0, 0, 0}, Olive, 2}, {{1, 1, 0, 0}, Anochovie, 4}}, 2, 4}},
	{19, {"Prudence", {{{1, 0, 0, 0}, Mushroom, 5}, {{0, 1, 0, 0}, Onion, 3}}, 2, 6}},
	{20, {"James", {{{1, 1, 0, 0}, Meat, 4}, {{0, 1, 1, 0}, Olive, 8}}, 2, 4}},
	{21, {"Cecilia", {{{1, 1, 1, 6}, Mushroom, 4}, {{0, 1, 1, 1}, Pepper, 3}, {{1, 1, 0, 1}, Onion, 3}}, 2, 8}},
	{22, {"Mandi", {{{1, 1, 0, 0}, Pepperoni, 4}, {{1, 0, 1, 1}, Mushroom, 6}}, 4, 8}},
	{23, {"Sasha", {{{0, 1, 0, 0}, Pepper, 4}, {{1, 1, 1, 1}, Olive, 8}}, 4, 8}},
	{24, {"Olga", {{{1, 1, 1, 0}, Meat, 6}, {{0, 0, 1, 0}, Mushroom, 4}, {{0, 0, 1, 0}, Pepper, 2}}, 6, 4}},
	{25, {"Franco", {{{1, 1, 1, 1}, Pepperoni, 8}, {{1, 0, 1, 1}, Olive, 3}}, 4, 8}},
	{26, {"Tohru", {{{0, 0, 1, 1}, Mushroom, 6}, {{1, 1, 1, 1}, Anochovie, 8}}, 2, 8}},
	{27, {"Clair", {{{1, 1, 1, 1}, Pepperoni, 4}, {{0, 0, 1, 1}, Mushroom, 6}, {{0, 1, 0, 0}, Pepper, 4}}, 4, 4}},
	{28, {"Clover", {{{1, 1, 1, 1}, Pepperoni, 8}}, 4, 8}},
	{29, {"Hugo", {{{1, 1, 0, 0}, Meat, 4}, {{0, 1, 0, 0}, Pepper, 4}}, 4, 6}},
	{30, {"Peggy", {{{1, 1, 1, 1}, Onion, 4}, {{1, 1, 1, 0}, Olive, 6}}, 3, 8}},
	{31, {"Carlo Romano", {{{1, 1, 0, 0}, Meat, 4}, {{0, 0, 1, 1}, Mushroom, 6}, {{0, 1, 0, 0}, Pepper, 4}}, 4, 8}},
	{32, {"Little Edoardo", {{{1, 1, 1, 1}, Onion, 4}, {{1, 1, 1, 1}, Olive, 4}, {{1, 1, 1, 1}, Anochovie, 4}}, 5, 8}},
	{33, {"Gino Romano", {{{1, 1, 1, 1}, Pepperoni, 8}, {{1, 1, 1, 1}, Onion, 4}, {{1, 1, 1, 1}, Olive, 4}}, 4, 8}},
	{34, {"Bruna Romano", {{{1, 0, 0, 0}, Pepperoni, 2}, {{1, 1, 1, 1}, Meat, 4}, {{1, 1, 1, 1}, Olive, 4}}, 5, 4}},
	{35, {"SargeFan!", {{{1, 1, 1, 1}, Onion, 12}}, 5, 6}},
	// Fat fuck
	{36, {"PAPA LOUIE!", {{{1, 0, 0, 0}, Pepperoni, 2},
						  {{0, 1, 0, 0}, Meat, 2},
						  {{0, 0, 1, 0}, Mushroom, 2},
						  {{0, 0, 0, 1}, Pepper, 2},
						  {{1, 0, 0, 0}, Onion, 2},
						  {{0, 1, 0, 0}, Olive, 2},
						  {{0, 0, 1, 0}, Anochovie, 2}}, 4, 4}}
};
//===============================================================================
