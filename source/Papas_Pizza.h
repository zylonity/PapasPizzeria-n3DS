#pragma once

#include "Papas_Constants.h"
#include "Papas_Utils.h"
#include <3ds.h>
#include <citro2d.h>
#include <vector>

namespace Papas {

	// One topping sitting on a pizza, pos is relative to the pizza centre
	struct PlacedTopping {
		Toppings type;
		v2 pos;
		float rotDegrees;
	};

	// A finished cut across the pizza, animates in over 250ms
	struct PizzaCut {
		v2 start;
		v2 end;
		u64 startedAt;
	};

	struct Pizza {
		// Where the pizza is in its journey through the shop
		enum Location {
			SlidingIn,
			OnCounter,
			SlidingToSaved,		// parked off the counter for later
			Saved,
			SlidingOutToOven,
			InOven,
			SlidingOutOfOven,
			WaitingToCut,
			SlidingToBoard,
			OnBoard,
			Served
		};

		int id;						// stable id, safe to keep around after serving
		Receipt *ticket;			// the order this pizza is for
		Location loc;
		v2 pos;
		int ovenSlot = -1;			// which of the 4 ovens, -1 when not in one
		u64 ovenStartedAt = 0;
		float cookDegrees = 0.0f;	// timer dial position, caps at 360
		int cookStage = 0;			// doneness look, changes every 45 degrees
		std::vector<PlacedTopping> toppings;
		std::vector<PizzaCut> cuts;
	};

	// Owns each pizza from topping through baking, cutting, and scoring.
	class PizzaManager {
	public:
		PizzaManager() {};

		void initManager();
		void terminateManager();
		void update(touchPosition &touch, Receipt *dockedReceipt);	// topping station touches
		void updateTimers();				// every frame, ovens cook off-screen too
		void updateBaking(touchPosition &touch);
		void updateCutting(touchPosition &touch);
		void cancelCutting();				// leaving the station mid-drag
		void renderBottom(touchPosition &touch, Receipt *dockedReceipt);
		void renderBaking();
		void renderCutting(touchPosition &touch);
		Pizza *consumeServedPizza();		// one-shot, nullptr once taken
		void renderPizzaForResult(int pizzaId, v2 centre, float scale, float depth);

	private:
		// A topping cup around the edges of the topping station
		struct Cup {
			Toppings type;
			v2 centre;
		};

		Pizza *activePizza(Receipt *dockedReceipt);
		Pizza *pizzaForTicket(Receipt *ticket);
		Pizza *pizzaById(int id);
		Pizza *boardPizza();
		int freeOvenSlot();
		void movePizzas();
		void startNextCuttingPizza();
		void drawPizza(Pizza &p, v2 centre, float scale, float depth);
		void drawTopping(Toppings type, int cookStage, v2 screenPos, float rotDegrees, float scale, float depth);
		void drawLineImage(C2D_Image image, v2 start, v2 end, float scaleY, float depth);

		C2D_SpriteSheet sheet;
		C2D_SpriteSheet shellSheet;
		C2D_Image shellImgs[9];
		C2D_Image toppingImgs[33];
		C2D_Image needleImg;
		C2D_Image dottedLineImg;
		C2D_Image cutLineImg;

		Button makePizzaBtn;
		Button ovenBtn;
		Button saveBtn;
		Button serveBtn;

		std::vector<Pizza> v_pizzas;
		std::vector<int> cuttingQueue;		// pizza ids, first out of the oven cuts first
		Cup cups[7];
		int nextPizzaId;
		int servedPizzaId;					// set on Serve, cleared by consumeServedPizza

		// Topping drag state
		bool dragging;
		bool wasTouching;
		bool bakingWasTouching;
		bool cuttingWasTouching;
		bool cuttingDrag;
		int dottedLineChannel;				// looping cut sound whilst dragging
		bool droppedThisFrame;				// stops a drop also pressing a button
		PlacedTopping dragged;
		v2 dragPos;
		v2 cutStart;
		v2 cutEnd;
		bool fromPizza;						// picked off the pizza rather than a cup
		v2 origin;							// where a failed drop flies back to
		v2 originRel;
		bool movingBack;
	};
}
