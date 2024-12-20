#pragma once
#include "Papas_Constants.h"
#include "Papas_SceneManager.h"
#include <3ds.h>
#include <citro2d.h>

namespace Papas {

	class Renderer
	{
	public:
		PapasError init();
		PapasError update(Papas::SceneManager* sceneManager);
		PapasError render(Papas::SceneManager* sceneManager);
		PapasError terminate();
	private:
		
		C3D_RenderTarget* topRenderTarget;
		C3D_RenderTarget* bottomRenderTarget;

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