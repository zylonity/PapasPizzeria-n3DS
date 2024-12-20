#pragma once
#include "Papas_Constants.h"
#include "Papas_SceneManager.h"
#include <3ds.h>
#include <citro2d.h>

namespace Papas {

	class MainMenu : public Scene {
	public:
		void init() override;
		void update() override;
		void render_top() override;
		void render_bottom() override;
		void terminate() override;
	private:

		//backgrounds
		C2D_SpriteSheet sheet_bg;
		C2D_Image top_bg;
		C2D_Image bottom_bg;

		//icons
		C2D_SpriteSheet sheet_icons;
		C2D_Image logo;

		//buttons
		C2D_SpriteSheet sheet_buttons;
		C2D_Image start;
		C2D_Image help;
		C2D_Image credits;

	};


}