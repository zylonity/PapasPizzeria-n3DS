#pragma once
#include "Papas_Constants.h"
#include "Papas_SceneManager.h"
#include <3ds.h>
#include <citro2d.h>

namespace Papas {

	class MainMenu : public Scene {
	public:
		PapasError init() override;
		PapasError update() override;
		PapasError render_top() override;
		PapasError render_bottom() override;
		PapasError terminate() override;
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