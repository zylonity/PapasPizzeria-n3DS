#pragma once
//===============================================================================
// name: Papas_Framework.h
// desc: Simple framework
// auth: Khaleel Brewesh Mora
//===============================================================================
#include "Papas_Constants.h"

namespace Papas {
	//===============================================================================
	// Forward Declarations
	class Renderer;
	class SceneManager;
	//===============================================================================


	class Framework
	{
	public:
		PapasError init();
		PapasError update();
		PapasError terminate();
	private:
		Papas::Renderer* m_pRenderer = nullptr;
		Papas::SceneManager* m_pSceneManager = nullptr;
	};

}