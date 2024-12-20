#pragma once
#include "Papas_Constants.h"
#include "Papas_SceneManager.h"
#include <3ds.h>
#include <citro2d.h>

namespace Papas {

	class Renderer
	{
	public:
		PapasError init(Papas::SceneManager* sceneManager);
		PapasError update(Papas::SceneManager* sceneManager);
		PapasError render(Papas::SceneManager* sceneManager);
		PapasError terminate();
	private:
		
		//bool firstPass;

		C3D_RenderTarget* topRenderTarget;
		C3D_RenderTarget* bottomRenderTarget;

	};
}