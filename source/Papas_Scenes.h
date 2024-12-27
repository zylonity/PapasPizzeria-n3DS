#pragma once
#include "Papas_Constants.h"
#include "Papas_SceneManager.h"
#include "Papas_Utils.h"
#include <3ds.h>
#include <citro2d.h>
#include <vector>
#include <chrono>

namespace Papas {

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

		GuyPeeking guy;

		C2D_Image ticketsStationImg;
		C2D_Image ticketsHolderImg;

		C2D_Image currentStationImg;
		C2D_Image currentPopupImg;

		void SwitchStation(Stations station);

		u64 lastInputTime = 0;		   // Last time input was processed
		const u64 inputCooldown = 200; // 200ms cooldown

		Papas::SceneManager* p_sceneManager;

		// Receipt system stuff to move later
		C2D_Font dokyo;

		ReceiptManager rep;
	};
}