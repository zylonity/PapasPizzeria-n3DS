// Runs the live customer queues from Customer.as and CustomerManager.as.

#include "Papas_Customers.h"
#include "Papas_ResourceManager.h"
#include "Papas_Save.h"
#include <algorithm>
#include <cmath>

// The 600x450 lobby maps into its 294x240 room, not the whole top screen.
#define LOBBY_X(stage)      ((stage) * 0.49f)	// stage x -> lobby x

// Send flipped leavers far enough to hide behind the ticket-holder door.
#define ENTER_OFFSCREEN_X   LOBBY_X(630.0f)
#define LEAVE_OFFSCREEN_X   320.0f	// past LOBBY_X(645), clear of the holder edge

// Nudge each queue around this port's deeper counter.
#define ORDERLINE_COUNTER   18.0f
#define WAITLINE_COUNTER    24.0f

// Give the queue more room without moving its front customer.
#define LINE_SPACING        1.19f

#define ORDERLINE_X         (LOBBY_X(110.0f) + ORDERLINE_COUNTER)
#define ORDERLINE_Y         111.0f	// 208 * 0.533
#define ORDERLINE_OFFSET    (LOBBY_X(86.0f) * LINE_SPACING)
#define ORDERLINE_SCALE     0.32f	// 0.6 (Flash sizepercent) * 0.533

#define WAITLINE_X          (LOBBY_X(207.0f) + WAITLINE_COUNTER)
#define WAITLINE_Y          52.0f	// 97 * 0.533
#define WAITLINE_OFFSET     (LOBBY_X(66.0f) * LINE_SPACING)
#define WAITLINE_SCALE      0.27f	// 0.5 * 0.533

#define LEAVELINE_X         LOBBY_X(274.0f)
#define LEAVELINE_Y         77.0f	// 145 * 0.533
#define LEAVELINE_SCALE     0.32f	// 0.6 * 0.533

#define TAKEORDER_X         115.0f	// stands behind the counter, right of Roy
#define TAKEORDER_Y         18.0f	// lower body hidden by the countertop
#define TAKEORDER_SCALE     0.55f

#define WALK_SPEED          81.0f	// px/s: 4px per 33ms on the original stage * 0.667

#define TIME_PER_DAY        90.0f	// seconds; spawnSpeed = day / (customers - 1)

// OrderScreen.forceCustomerTime: how long an empty order line is allowed to sulk
#define IDLE_LINE_NUDGE_MS  9000

// Customer

void Papas::Customer::spawnCustomer(int typeId, int num, int lineIndex)
{
	type = typeId;
	number = num;
	enteredAt = Papas::Clock::now();

	// Take our own copy of the notch so the last customer's can be shortened
	std::unordered_map<int, CustomerData>::const_iterator order = map_customers.find(type);
	cookTime = order != map_customers.end() ? order->second.time : 1;

	// Lobby customers hold only their lobby-sized limb atlas.
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
	segStart = Papas::Clock::now();
}

float Papas::Customer::animSeconds() const
{
	return (float)(Papas::Clock::now() - segStart) / 1000.0f;
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
		// Once they're offscreen, the manager moves them to the wait line.
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
	// Pixel-snap walking customers to prevent filtered edges shimmering.
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

	int segment = presentationSeg;
	float seconds;
	if (segment >= 0)
		seconds = (float)(Papas::Clock::now() - presentationStart) / 1000.0f;
	else
	{
		segment = Papas::CustomerRig::getInstance().segmentIndex("stand");
		seconds = animSeconds();
	}
	int frame = Papas::CustomerRig::getInstance().frameForTime(segment, seconds);
	Papas::CustomerRig::getInstance().draw(atlasOrder, type, frame,
										   TAKEORDER_X, TAKEORDER_Y,
										   TAKEORDER_SCALE, TAKEORDER_SCALE, depth);
}

void Papas::Customer::playPresentation(const char* segment)
{
	presentationSeg = Papas::CustomerRig::getInstance().segmentIndex(segment);
	presentationStart = Papas::Clock::now();
}

// CustomerManager

void Papas::CustomerManager::initManager(int rank, int day)
{
	Papas::CustomerRig::getInstance().load();

	totalCustomers = 0;
	today = day;
	decideLineup(rank);

	// setupSpawn(): first customer right away, the rest spread over the day
	spawnSpeed = TIME_PER_DAY / (customerLineup.size() - 1);
	lastUpdate = Papas::Clock::now();
	emptyLineSince = 0;
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

// Start with six, add one per rank, then unlock Papa after 35 triple seals.
int Papas::CustomerManager::unlockedCount(int rank)
{
	int unlocked = 6 + (rank - 1);
	if (unlocked > 35) unlocked = 35;
	if (rank >= 31 && papaEarned()) unlocked++;
	return unlocked;
}

bool Papas::CustomerManager::papaEarned()
{
	const SaveData &save = SaveManager::getInstance().data;
	for (int type = 1; type <= 35; type++)
	{
		if (save.customerSeals[type] < 3) return false;
	}
	return true;
}

bool Papas::CustomerManager::papaBlocked(int rank)
{
	return rank >= 31 && !papaEarned();
}

int Papas::CustomerManager::newCustomerToday(int rank)
{
	// Below rank 2 the lineup is all random, so nobody is guaranteed to show
	if (rank < 2) return 0;
	int newest = unlockedCount(rank);
	return SaveManager::getInstance().data.customerMet[newest] ? 0 : newest;
}

void Papas::CustomerManager::decideLineup(int rank)
{
	// Put the newest unlock first, then pick the rest without duplicates.
	static const int customersPerRank[11] = {4, 4, 5, 5, 6, 7, 7, 8, 8, 9, 10};
	int count = customersPerRank[rank < 10 ? rank : 10];
	int unlocked = unlockedCount(rank);

	customerLineup.clear();
	std::vector<int> pool;
	for (int t = 1; t <= unlocked; t++)
	{
		pool.push_back(t);
	}
	if (rank >= 2)
	{
		customerLineup.push_back(unlocked);
		pool.erase(pool.begin() + (unlocked - 1));
	}
	while ((int)customerLineup.size() < count && !pool.empty())
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

	int type = customerLineup[totalCustomers];
	// startCustomerEntering(): first visit stamps the day on their file
	SaveData &sv = SaveManager::getInstance().data;
	if (type > 0 && type < (int)(sizeof(sv.customerFirstDay) / sizeof(sv.customerFirstDay[0]))
		&& sv.customerFirstDay[type] == 0)
		sv.customerFirstDay[type] = (u16)today;

	Customer* c = new Customer();
	c->spawnCustomer(type, totalCustomers + 1, orderline.size());
	totalCustomers++;
	// Last one through the door orders something quick so the day can end
	if (totalCustomers == (int)customerLineup.size())
		c->shortenWaitTime();
	v_customers.push_back(c);
	orderline.push_back(c);
	lastSpawnTime = Papas::Clock::now();
	emptyLineSince = 0;
}

// Skip the wait when the counter's been dead for a while, like the original did
void Papas::CustomerManager::nudgeIdleLine()
{
	if (totalCustomers >= (int)customerLineup.size())
		return;

	u64 now = Papas::Clock::now();
	if (!orderline.empty())
	{
		emptyLineSince = 0;
		return;
	}
	if (emptyLineSince == 0)
	{
		emptyLineSince = now;
		return;
	}
	// Only worth it if the next scheduled spawn is still ages away
	if (now - emptyLineSince >= IDLE_LINE_NUDGE_MS && now - lastSpawnTime >= IDLE_LINE_NUDGE_MS)
		spawnNext();
}

void Papas::CustomerManager::update()
{
	u64 now = Papas::Clock::now();
	float deltaSeconds = (float)(now - lastUpdate) / 1000.0f;
	lastUpdate = now;

	// spawn timer
	if (totalCustomers < (int)customerLineup.size() &&
		(float)(now - lastSpawnTime) / 1000.0f >= spawnSpeed)
	{
		spawnNext();
	}
	nudgeIdleLine();

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
	// Spawn order sets depth; keep the whole line inside the available gap.
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

Papas::Customer* Papas::CustomerManager::getCustomer(int customerNumber)
{
	for (size_t i = 0; i < v_customers.size(); i++)
		if (v_customers[i]->getNumber() == customerNumber) return v_customers[i];
	return nullptr;
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

void Papas::CustomerManager::completeOrder(int customerNumber)
{
	Customer* completed = nullptr;
	for (size_t i = 0; i < v_customers.size(); i++)
	{
		if (v_customers[i]->getNumber() == customerNumber)
		{
			completed = v_customers[i];
			break;
		}
	}
	if (completed == nullptr)
		return;

	waitline.erase(std::remove(waitline.begin(), waitline.end(), completed), waitline.end());
	orderline.erase(std::remove(orderline.begin(), orderline.end(), completed), orderline.end());
	for (size_t i = 0; i < waitline.size(); i++)
		waitline[i]->shiftWaitLine(i);
	for (size_t i = 0; i < orderline.size(); i++)
		orderline[i]->shiftOrderLine(i);

	for (size_t i = 0; i < v_customers.size(); i++)
	{
		if (v_customers[i] == completed)
		{
			completed->despawnCustomer();
			delete completed;
			v_customers.erase(v_customers.begin() + i);
			break;
		}
	}
}

bool Papas::CustomerManager::dayIsOver() const
{
	return totalCustomers == (int)customerLineup.size() &&
		   orderline.empty() && waitline.empty();
}

// Customer order data (index, name, items, notch time, slices)
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
	{21, {"Cecilia", {{{1, 1, 1, 0}, Mushroom, 6}, {{0, 1, 1, 1}, Pepper, 3}, {{1, 1, 0, 1}, Onion, 3}}, 2, 8}},
	{22, {"Mandi", {{{1, 1, 0, 0}, Pepperoni, 4}, {{1, 0, 1, 1}, Mushroom, 6}}, 4, 8}},
	{23, {"Sasha", {{{0, 1, 0, 0}, Pepper, 4}, {{1, 1, 1, 1}, Olive, 8}}, 4, 8}},
	{24, {"Olga", {{{1, 1, 1, 0}, Meat, 6}, {{0, 0, 1, 0}, Mushroom, 4}, {{0, 0, 1, 0}, Pepper, 2}}, 6, 4}},
	{25, {"Franco", {{{1, 1, 1, 1}, Pepperoni, 8}, {{1, 0, 1, 1}, Olive, 3}}, 4, 8}},
	{26, {"Tohru", {{{0, 0, 1, 1}, Mushroom, 6}, {{1, 0, 0, 0}, Pepper, 2}, {{1, 1, 1, 1}, Anochovie, 8}}, 2, 8}},
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

// What each customer is known for, printed on their file card
std::unordered_map<int, std::string> Papas::map_customerToppings = {
	{1, "Pepperoni"},
	{2, "Anchovies"},
	{3, "Mushrooms"},
	{4, "Olives"},
	{5, "Pepperoni"},
	{6, "Peppers"},
	{7, "Sausage and Onions"},
	{8, "Sausage and Mushrooms"},
	{9, "Peppers and Olives"},
	{10, "Peppers and Onions"},
	{11, "Pepperoni and Mushrooms"},
	{12, "Pepperoni and Sausage"},
	{13, "Mushroom and Anchovies"},
	{14, "Pepperoni and Sausage"},
	{15, "Pepperoni and Mushrooms"},
	{16, "Mushroom and Peppers"},
	{17, "Pepperoni"},
	{18, "Pepperoni, Olive, Anchovies"},
	{19, "Mushroom and Onions"},
	{20, "Sausage and Olives"},
	{21, "Mushroom, Peppers, Onions"},
	{22, "Pepperoni and Mushrooms"},
	{23, "Peppers and Olives"},
	{24, "Sausage, Mushroom, Onions"},
	{25, "Pepperoni and Olives"},
	{26, "Mushroom, Peppers, Anchovies"},
	{27, "Pepperoni, Mushroom, Peppers"},
	{28, "Pepperoni"},
	{29, "Sausage and Peppers"},
	{30, "Onions and Olives"},
	{31, "Sausage, Mushrooms, Peppers"},
	{32, "Onion, Olives, Anchovies"},
	{33, "Pepperoni, Onions, Olives"},
	{34, "Pepperoni, Sausage, Olives"},
	{35, "Onions Only!"},
	{36, "The Works!"}
};
