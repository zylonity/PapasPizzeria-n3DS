#pragma once

#include "Papas_Constants.h"
#include "Papas_Utils.h"
#include <3ds.h>
#include <citro2d.h>
#include <vector>

namespace Papas {

	struct PlacedTopping {
		Toppings type;
		v2 pos;
		float rotDegrees;
	};

	struct PizzaCut {
		v2 start;
		v2 end;
		u64 startedAt;
	};

	struct Pizza {
		enum Location {
			SlidingIn,
			OnCounter,
			SlidingOutToOven,
			InOven,
			SlidingOutOfOven,
			WaitingToCut,
			SlidingToBoard,
			OnBoard,
			Served
		};

		int id;
		Receipt *ticket;
		Location loc;
		v2 pos;
		int ovenSlot = -1;
		u64 ovenStartedAt = 0;
		float cookDegrees = 0.0f;
		int cookStage = 0;
		std::vector<PlacedTopping> toppings;
		std::vector<PizzaCut> cuts;
	};

	class PizzaManager {
	public:
		PizzaManager() {};

		void initManager();
		void terminateManager();
		void update(touchPosition &touch, Receipt *dockedReceipt);
		void updateTimers();
		void updateBaking(touchPosition &touch);
		void updateCutting(touchPosition &touch);
		void cancelCutting();
		void renderBottom(touchPosition &touch, Receipt *dockedReceipt);
		void renderBaking();
		void renderCutting(touchPosition &touch);
		Pizza *consumeServedPizza();
		void renderPizzaForResult(int pizzaId, v2 centre, float scale, float depth);

	private:
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
		Button serveBtn;

		std::vector<Pizza> v_pizzas;
		std::vector<int> cuttingQueue;
		Cup cups[7];
		int nextPizzaId;
		int servedPizzaId;

		bool dragging;
		bool wasTouching;
		bool bakingWasTouching;
		bool cuttingWasTouching;
		bool cuttingDrag;
		int dottedLineChannel;
		bool droppedThisFrame;
		PlacedTopping dragged;
		v2 dragPos;
		v2 cutStart;
		v2 cutEnd;
		bool fromPizza;
		v2 origin;
		v2 originRel;
		bool movingBack;
	};
}
