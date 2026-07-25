#include "Papas_Pizza.h"
#include "Papas_ResourceManager.h"
#include <algorithm>
#include <cmath>

// Layout/tuning values, mostly traced off the station art
static const float PIZZA_SCALE = 0.7f;
static const Papas::v2 COUNTER_POS = {161.0f, 118.0f};	// pizza centre at the topping station
static const Papas::v2 BOARD_POS = {170.0f, 118.0f};	// pizza centre on the cutting board
static const float PIZZA_RADIUS = 137.0f;		// unscaled, for cut detection
static const float PIZZA_DROP_RADIUS = 88.0f;	// how close a topping drop has to be to count
static const float CUP_RADIUS = 24.0f;			// grab area of the topping cups
static const float GRAB_RADIUS = 16.0f;			// grab area of a topping already on the pizza
static const float SLIDE_STEPS = 3.0f;			// pizzas ease in/out by 1/3 of the distance per frame
static const float OVEN_SCALE = 0.35f;
static const float OVEN_TOUCH_RADIUS = 48.0f;
static const float MS_PER_DEGREE = 500.0f;		// oven timer speed, 360 degrees = 3 minutes
static const Papas::v2 OVEN_SLOTS[4] = {{105.0f, 72.0f}, {215.0f, 72.0f}, {105.0f, 178.0f}, {215.0f, 178.0f}};
static const Papas::v2 TIMER_POS[4] = {{19.0f, 76.0f}, {301.5f, 76.0f}, {19.0f, 182.0f}, {301.0f, 182.0f}};
// Each topping has cooked-look variants in the sheet; base index + how many
static const int LOOK_BASE[7] = {0, 4, 9, 14, 19, 24, 28};
static const int LOOK_COUNT[7] = {4, 5, 5, 5, 5, 4, 5};

static float distance(Papas::v2 a, Papas::v2 b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return std::sqrt(dx * dx + dy * dy);
}

static bool touchDown(const touchPosition &touch)
{
	return touch.px != 0 || touch.py != 0;
}

void Papas::PizzaManager::initManager()
{
	sheet = C2D_SpriteSheetLoad("romfs:/gfx/pizza.t3x");
	shellSheet = C2D_SpriteSheetLoad("romfs:/gfx/pizza_shells.t3x");

	for (int i = 0; i < 33; i++) {
		toppingImgs[i] = C2D_SpriteSheetGetImage(sheet, i);
		C3D_TexSetFilter(toppingImgs[i].tex, GPU_LINEAR, GPU_LINEAR);
	}
	for (int i = 0; i < 9; i++) {
		shellImgs[i] = C2D_SpriteSheetGetImage(shellSheet, i);
		C3D_TexSetFilter(shellImgs[i].tex, GPU_LINEAR, GPU_LINEAR);
	}
	needleImg = C2D_SpriteSheetGetImage(sheet, 33);
	dottedLineImg = C2D_SpriteSheetGetImage(sheet, 34);
	cutLineImg = C2D_SpriteSheetGetImage(sheet, 35);

	makePizzaBtn.createButton(sheet, 36, 36, 37, {102, 208});
	ovenBtn.createButton(sheet, 38, 38, 39, {112, 208});
	serveBtn.createButton(sheet, 40, 40, 41, {122, 207});

	ResourceManager::getInstance().loadSfx("grabtopping", "romfs:/sfx/grabtopping.wav");
	ResourceManager::getInstance().loadSfx("droptopping", "romfs:/sfx/droptopping.wav");
	ResourceManager::getInstance().loadSfx("pizzaslide", "romfs:/sfx/pizzaslide.wav");
	ResourceManager::getInstance().loadSfx("dottedline", "romfs:/sfx/dottedline.wav");
	ResourceManager::getInstance().loadSfx("cut", "romfs:/sfx/cut.wav");

	cups[0] = {Mushroom, {22.0f, 113.0f}};
	cups[1] = {Meat, {22.0f, 162.0f}};
	cups[2] = {Pepperoni, {23.0f, 215.0f}};
	cups[3] = {Pepper, {294.0f, 62.0f}};
	cups[4] = {Onion, {292.0f, 116.0f}};
	cups[5] = {Olive, {294.0f, 167.0f}};
	cups[6] = {Anochovie, {295.0f, 212.0f}};

	nextPizzaId = 1;
	servedPizzaId = -1;
	dragging = false;
	wasTouching = false;
	bakingWasTouching = false;
	cuttingWasTouching = false;
	cuttingDrag = false;
	dottedLineChannel = -1;
	droppedThisFrame = false;
	movingBack = false;
}

// Every pizza belongs to a receipt; served ones don't count anymore
Papas::Pizza *Papas::PizzaManager::pizzaForTicket(Receipt *ticket)
{
	for (size_t i = 0; i < v_pizzas.size(); i++)
		if (v_pizzas[i].ticket == ticket && v_pizzas[i].loc != Pizza::Served) return &v_pizzas[i];
	return nullptr;
}

Papas::Pizza *Papas::PizzaManager::pizzaById(int id)
{
	for (size_t i = 0; i < v_pizzas.size(); i++)
		if (v_pizzas[i].id == id) return &v_pizzas[i];
	return nullptr;
}

// The pizza the docked receipt is working on at the topping station
Papas::Pizza *Papas::PizzaManager::activePizza(Receipt *ticket)
{
	Pizza *p = ticket == nullptr ? nullptr : pizzaForTicket(ticket);
	return p != nullptr && (p->loc == Pizza::SlidingIn || p->loc == Pizza::OnCounter || p->loc == Pizza::SlidingOutToOven) ? p : nullptr;
}

// Front of the cutting queue, cut pizzas in the order they left the oven
Papas::Pizza *Papas::PizzaManager::boardPizza()
{
	if (cuttingQueue.empty()) return nullptr;
	return pizzaById(cuttingQueue.front());
}

void Papas::PizzaManager::startNextCuttingPizza()
{
	Pizza *p = boardPizza();
	if (p != nullptr && p->loc == Pizza::WaitingToCut) {
		p->loc = Pizza::SlidingToBoard;
		p->pos = {450.0f, BOARD_POS.y};
		ResourceManager::getInstance().playSfx("pizzaslide");
	}
}

// Slide animations for every pizza in transit (counter, ovens, board)
void Papas::PizzaManager::movePizzas()
{
	for (size_t i = 0; i < v_pizzas.size(); i++) {
		Pizza &p = v_pizzas[i];
		if (p.loc == Pizza::SlidingIn) {
			float dx = (COUNTER_POS.x - p.pos.x) / SLIDE_STEPS;
			if (std::abs(dx) < 1.0f) { p.pos = COUNTER_POS; p.loc = Pizza::OnCounter; }
			else p.pos.x += dx;
		} else if (p.loc == Pizza::SlidingOutToOven) {
			float dx = (SCREEN_WIDTH_BOTTOM + 150.0f - p.pos.x) / SLIDE_STEPS;
			if (std::abs(dx) < 1.0f) { p.loc = Pizza::InOven; p.ovenStartedAt = osGetTime(); }
			else p.pos.x += dx;
		} else if (p.loc == Pizza::SlidingOutOfOven) {
			float target = (p.ovenSlot == 0 || p.ovenSlot == 2) ? -150.0f : SCREEN_WIDTH_BOTTOM + 150.0f;
			float dx = (target - p.pos.x) / SLIDE_STEPS;
			if (std::abs(dx) < 1.0f) {
				p.loc = Pizza::WaitingToCut;
				p.ovenSlot = -1;
				cuttingQueue.push_back(p.id);
				startNextCuttingPizza();
			} else p.pos.x += dx;
		} else if (p.loc == Pizza::SlidingToBoard) {
			float dx = (BOARD_POS.x - p.pos.x) / SLIDE_STEPS;
			if (std::abs(dx) < 1.0f) { p.pos = BOARD_POS; p.loc = Pizza::OnBoard; }
			else p.pos.x += dx;
		}
	}
}

// Runs every frame no matter the station, so ovens keep cooking off-screen
void Papas::PizzaManager::updateTimers()
{
	movePizzas();
	u64 now = osGetTime();
	for (size_t i = 0; i < v_pizzas.size(); i++) {
		Pizza &p = v_pizzas[i];
		if (p.loc != Pizza::InOven || p.ovenStartedAt == 0) continue;
		p.cookDegrees = std::min(360.0f, (float)(now - p.ovenStartedAt) / MS_PER_DEGREE);
		p.cookStage = std::min(8, (int)(p.cookDegrees / 45.0f));
	}
}

// Topping station touch handling: grab from cups, drag, drop on the pizza
void Papas::PizzaManager::update(touchPosition &touch, Receipt *dockedReceipt)
{
	droppedThisFrame = false;
	Pizza *active = activePizza(dockedReceipt);
	// A failed drop flies back to wherever it came from
	if (movingBack) {
		float dx = (origin.x - dragPos.x) / 2.0f, dy = (origin.y - dragPos.y) / 2.0f;
		if (std::abs(dx) < 1.0f && std::abs(dy) < 1.0f) {
			if (fromPizza && active != nullptr) { dragged.pos = originRel; active->toppings.push_back(dragged); ResourceManager::getInstance().playSfx("droptopping"); }
			else ResourceManager::getInstance().playSfx("grabtopping");
			movingBack = false; dragging = false;
		} else { dragPos.x += dx; dragPos.y += dy; }
		wasTouching = touchDown(touch);
		return;
	}

	bool touching = touchDown(touch);
	if (touching && !wasTouching && !dragging && active != nullptr && active->loc == Pizza::OnCounter) {
		v2 t = {(float)touch.px, (float)touch.py};
		// Toppings already on the pizza grab first, newest on top
		for (size_t i = active->toppings.size(); i-- > 0;) {
			v2 at = {active->pos.x + active->toppings[i].pos.x * PIZZA_SCALE, active->pos.y + active->toppings[i].pos.y * PIZZA_SCALE};
			if (distance(t, at) < GRAB_RADIUS) {
				dragged = active->toppings[i]; active->toppings.erase(active->toppings.begin() + i);
				fromPizza = true; origin = at; originRel = dragged.pos; dragPos = t; dragging = true;
				ResourceManager::getInstance().playSfx("grabtopping"); break;
			}
		}
		// Otherwise try the cups for a fresh topping
		if (!dragging) for (size_t i = 0; i < 7; i++) if (distance(t, cups[i].centre) < CUP_RADIUS) {
			dragged.type = cups[i].type; dragged.rotDegrees = (float)ResourceManager::getInstance().randomNumber(0, 359);
			fromPizza = false; origin = cups[i].centre; dragPos = t; dragging = true;
			ResourceManager::getInstance().playSfx("grabtopping"); break;
		}
	} else if (touching && dragging) dragPos = {(float)touch.px, (float)touch.py};
	else if (!touching && wasTouching && dragging) {
		// Released: land on the pizza, discard into a matching cup, or fly back
		droppedThisFrame = true;
		if (active != nullptr && active->loc == Pizza::OnCounter && distance(dragPos, active->pos) < PIZZA_DROP_RADIUS) {
			dragged.pos = {(dragPos.x - active->pos.x) / PIZZA_SCALE, (dragPos.y - active->pos.y) / PIZZA_SCALE};
			active->toppings.push_back(dragged); dragging = false; ResourceManager::getInstance().playSfx("droptopping");
		} else {
			bool inCup = false;
			for (size_t i = 0; i < 7; i++) if (cups[i].type == dragged.type && distance(dragPos, cups[i].centre) < CUP_RADIUS * 1.5f) inCup = true;
			if (inCup) { dragging = false; ResourceManager::getInstance().playSfx("grabtopping"); }
			else movingBack = true;
		}
	}
	wasTouching = touching;
}

// Tap a cooking pizza to pull it out of the oven
void Papas::PizzaManager::updateBaking(touchPosition &touch)
{
	bool touching = touchDown(touch);
	if (touching && !bakingWasTouching) {
		v2 at = {(float)touch.px, (float)touch.py};
		for (size_t i = 0; i < v_pizzas.size(); i++) {
			Pizza &p = v_pizzas[i];
			if (p.loc == Pizza::InOven && p.ovenSlot >= 0 && distance(at, OVEN_SLOTS[p.ovenSlot]) < OVEN_TOUCH_RADIUS) {
				p.loc = Pizza::SlidingOutOfOven; p.pos = OVEN_SLOTS[p.ovenSlot];
				ResourceManager::getInstance().playSfx("pizzaslide"); break;
			}
		}
	}
	bakingWasTouching = touching;
}

// Cutting station: drag a line across the pizza, full crossings become cuts
void Papas::PizzaManager::updateCutting(touchPosition &touch)
{
	Pizza *p = boardPizza();
	bool touching = touchDown(touch);
	if (p != nullptr && p->loc == Pizza::OnBoard) {
		v2 at = {(float)touch.px, (float)touch.py};
		bool onServeButton = at.x >= 122.0f && at.x <= 218.0f && at.y >= 207.0f && at.y <= 235.0f;
		if (touching && !cuttingWasTouching && !onServeButton) {
			cuttingDrag = true; cutStart = at; cutEnd = at;
			dottedLineChannel = ResourceManager::getInstance().playSfxLoop("dottedline");
		} else if (touching && cuttingDrag) cutEnd = at;
		else if (!touching && cuttingWasTouching && cuttingDrag) {
			ResourceManager::getInstance().stopSfxChannel(dottedLineChannel);
			dottedLineChannel = -1;
			// Count a cut only when both circle crossings fit inside the drag.
			v2 d = {cutEnd.x - cutStart.x, cutEnd.y - cutStart.y};
			v2 f = {cutStart.x - BOARD_POS.x, cutStart.y - BOARD_POS.y};
			float radius = PIZZA_RADIUS * PIZZA_SCALE;
			float a = d.x * d.x + d.y * d.y;
			float b = 2.0f * (f.x * d.x + f.y * d.y);
			float c = f.x * f.x + f.y * f.y - radius * radius;
			float disc = b * b - 4.0f * a * c;
			if (a > 0.001f && disc >= 0.0f) {
				float root = std::sqrt(disc);
				float t1 = (-b - root) / (2.0f * a), t2 = (-b + root) / (2.0f * a);
				if (t1 > 0.0f && t2 < 1.0f) {
					PizzaCut cut;
					cut.start = {(cutStart.x + d.x * t1 - BOARD_POS.x) / PIZZA_SCALE, (cutStart.y + d.y * t1 - BOARD_POS.y) / PIZZA_SCALE};
					cut.end = {(cutStart.x + d.x * t2 - BOARD_POS.x) / PIZZA_SCALE, (cutStart.y + d.y * t2 - BOARD_POS.y) / PIZZA_SCALE};
					cut.startedAt = osGetTime(); p->cuts.push_back(cut); ResourceManager::getInstance().playSfx("cut");
				}
			}
			cuttingDrag = false;
		}
	}
	cuttingWasTouching = touching;
}

// Kill the drag + its looping sound when leaving the station mid-cut
void Papas::PizzaManager::cancelCutting()
{
	ResourceManager::getInstance().stopSfxChannel(dottedLineChannel);
	dottedLineChannel = -1;
	cuttingDrag = false;
	cuttingWasTouching = false;
}

// Hand the just-served pizza to the game once; returns nullptr after that
Papas::Pizza *Papas::PizzaManager::consumeServedPizza()
{
	if (servedPizzaId < 0) return nullptr;
	Pizza *served = pizzaById(servedPizzaId);
	servedPizzaId = -1;
	return served;
}

void Papas::PizzaManager::renderPizzaForResult(int pizzaId, v2 centre, float scale, float depth)
{
	Pizza *pizza = pizzaById(pizzaId);
	if (pizza != nullptr) drawPizza(*pizza, centre, scale, depth);
}

void Papas::PizzaManager::drawTopping(Toppings type, int stage, v2 pos, float rotation, float scale, float depth)
{
	// Pick the cooked-look variant for how far along the pizza is
	int look = LOOK_BASE[type] + std::min(stage, LOOK_COUNT[type] - 1);
	C2D_Sprite spr; C2D_SpriteFromImage(&spr, toppingImgs[look]); C2D_SpriteSetCenter(&spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&spr, pos.x, pos.y); C2D_SpriteSetScale(&spr, scale, scale);
	C2D_SpriteSetRotationDegrees(&spr, rotation); C2D_SpriteSetDepth(&spr, depth); C2D_DrawSprite(&spr);
}

// Shell + every topping, toppings stacked in the order they were placed
void Papas::PizzaManager::drawPizza(Pizza &p, v2 centre, float scale, float depth)
{
	C2D_Image shell = shellImgs[std::min(8, p.cookStage)];
	C2D_DrawImageAt(shell, centre.x - shell.subtex->width * scale * 0.5f, centre.y - shell.subtex->height * scale * 0.5f, depth, NULL, scale, scale);
	for (size_t i = 0; i < p.toppings.size(); i++) {
		v2 pos = {centre.x + p.toppings[i].pos.x * scale, centre.y + p.toppings[i].pos.y * scale};
		drawTopping(p.toppings[i].type, p.cookStage, pos, p.toppings[i].rotDegrees, scale, depth + 0.01f + i * 0.0005f);
	}
}

// Stretch + rotate a 1px-ish line texture between two points
void Papas::PizzaManager::drawLineImage(C2D_Image image, v2 start, v2 end, float scaleY, float depth)
{
	float dx = end.x - start.x, dy = end.y - start.y;
	float length = std::sqrt(dx * dx + dy * dy);
	if (length < 1.0f) return;
	C2D_Sprite spr; C2D_SpriteFromImage(&spr, image); C2D_SpriteSetCenter(&spr, 0.0f, 0.5f);
	C2D_SpriteSetPos(&spr, start.x, start.y); C2D_SpriteSetScale(&spr, length / image.subtex->width, scaleY);
	C2D_SpriteSetRotation(&spr, std::atan2(dy, dx)); C2D_SpriteSetDepth(&spr, depth); C2D_DrawSprite(&spr);
}

// First oven slot nobody's using, -1 when all four are busy
int Papas::PizzaManager::freeOvenSlot()
{
	for (int slot = 0; slot < 4; slot++) {
		bool taken = false;
		for (size_t i = 0; i < v_pizzas.size(); i++)
			if (v_pizzas[i].ovenSlot == slot && (v_pizzas[i].loc == Pizza::InOven || v_pizzas[i].loc == Pizza::SlidingOutToOven)) taken = true;
		if (!taken) return slot;
	}
	return -1;
}

void Papas::PizzaManager::renderBaking()
{
	for (size_t i = 0; i < v_pizzas.size(); i++) {
		Pizza &p = v_pizzas[i];
		if ((p.loc == Pizza::InOven || p.loc == Pizza::SlidingOutOfOven) && p.ovenSlot >= 0) drawPizza(p, p.pos.x == OVEN_SLOTS[p.ovenSlot].x || p.loc == Pizza::InOven ? OVEN_SLOTS[p.ovenSlot] : p.pos, OVEN_SCALE, 0.02f);
	}
	// Timer needles, one dial per oven slot
	for (int slot = 0; slot < 4; slot++) {
		float degrees = 0.0f;
		for (size_t i = 0; i < v_pizzas.size(); i++) if (v_pizzas[i].ovenSlot == slot && v_pizzas[i].loc == Pizza::InOven) degrees = v_pizzas[i].cookDegrees;
		C2D_Sprite spr; C2D_SpriteFromImage(&spr, needleImg); C2D_SpriteSetCenterRaw(&spr, 3.0f, 19.0f);
		C2D_SpriteSetPos(&spr, TIMER_POS[slot].x, TIMER_POS[slot].y); C2D_SpriteSetScale(&spr, 0.85f, 0.85f);
		C2D_SpriteSetRotationDegrees(&spr, degrees); C2D_SpriteSetDepth(&spr, 0.8f); C2D_DrawSprite(&spr);
	}
}

void Papas::PizzaManager::renderCutting(touchPosition &touch)
{
	Pizza *p = boardPizza();
	if (p == nullptr) return;
	if (p->loc == Pizza::SlidingToBoard || p->loc == Pizza::OnBoard) drawPizza(*p, p->pos, PIZZA_SCALE, 0.02f);
	if (p->loc != Pizza::OnBoard) return;
	u64 now = osGetTime();
	for (size_t i = 0; i < p->cuts.size(); i++) {
		v2 start = {BOARD_POS.x + p->cuts[i].start.x * PIZZA_SCALE, BOARD_POS.y + p->cuts[i].start.y * PIZZA_SCALE};
		v2 finish = {BOARD_POS.x + p->cuts[i].end.x * PIZZA_SCALE, BOARD_POS.y + p->cuts[i].end.y * PIZZA_SCALE};
		float progress = std::min(1.0f, (float)(now - p->cuts[i].startedAt) / 250.0f);
		v2 end = {start.x + (finish.x - start.x) * progress, start.y + (finish.y - start.y) * progress};
		drawLineImage(cutLineImg, start, end, 1.0f, 0.85f);
	}
	if (cuttingDrag) drawLineImage(dottedLineImg, cutStart, cutEnd, 1.0f, 0.9f);
	// Serve hands this pizza over for scoring and pulls in the next one
	if (!cuttingDrag && serveBtn.showButton(touch)) {
		p->loc = Pizza::Served; servedPizzaId = p->id;
		cuttingQueue.erase(cuttingQueue.begin()); startNextCuttingPizza();
	}
}

// Draw the topping pizza, dragged item, and the button for its current step.
void Papas::PizzaManager::renderBottom(touchPosition &touch, Receipt *ticket)
{
	Pizza *active = activePizza(ticket);
	if (active != nullptr) drawPizza(*active, active->pos, PIZZA_SCALE, 0.02f);
	if (dragging || movingBack) drawTopping(dragged.type, 0, dragPos, dragged.rotDegrees, PIZZA_SCALE, 0.9f);
	if (ticket != nullptr && !dragging && !movingBack && !droppedThisFrame) {
		Pizza *existing = pizzaForTicket(ticket);
		if (existing == nullptr && makePizzaBtn.showButton(touch)) {
			Pizza p = {}; p.id = nextPizzaId++; p.ticket = ticket; p.loc = Pizza::SlidingIn; p.pos = {-150.0f, COUNTER_POS.y}; p.ovenSlot = -1;
			v_pizzas.push_back(p); ResourceManager::getInstance().playSfx("pizzaslide");
		} else if (active != nullptr && active->loc == Pizza::OnCounter && ovenBtn.showButton(touch)) {
			int slot = freeOvenSlot();
			if (slot != -1) { active->ovenSlot = slot; active->loc = Pizza::SlidingOutToOven; ResourceManager::getInstance().playSfx("pizzaslide"); }
		}
	}
}

void Papas::PizzaManager::terminateManager()
{
	cancelCutting();
	if (sheet) { C2D_SpriteSheetFree(sheet); sheet = nullptr; }
	if (shellSheet) { C2D_SpriteSheetFree(shellSheet); shellSheet = nullptr; }
	v_pizzas.clear(); cuttingQueue.clear();
}
