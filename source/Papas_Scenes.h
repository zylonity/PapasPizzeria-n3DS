#pragma once
#include "Papas_Constants.h"
#include "Papas_SceneManager.h"
#include "Papas_Utils.h"
#include <3ds.h>
#include <citro2d.h>
#include <vector>

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

		//icons
		C2D_SpriteSheet sheet_icons;
		C2D_Image logo;

		touchPosition touch;

		//buttons
		int buttonIndex;
		bool aPressed = false;
		C2D_SpriteSheet sheet_buttons;
		std::vector<Papas::Button> v_buttons;

	};

	class IntroVideo : public Scene {
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

		//icons
		C2D_SpriteSheet sheet_icons;
		C2D_Image logo;

		touchPosition touch;

		//buttons
		int buttonIndex;
		bool aPressed = false;
		C2D_SpriteSheet sheet_buttons;
		std::vector<Papas::Button> v_buttons;

	};


}