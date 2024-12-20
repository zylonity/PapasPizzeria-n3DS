#pragma once
#include "Papas_Constants.h"

namespace Papas {

	class Scene {
	public:
		virtual ~Scene() = default;

		virtual void init() = 0;           // Initialize resources for the scene
		virtual void update() = 0;         // Handle input and game logic
		virtual void render_top() = 0;         // Draw the scene
		virtual void render_bottom() = 0;         // Draw the scene
		virtual void terminate() = 0;      // Free resources
	};

	class SceneManager
	{
	public:
		PapasError init();
		PapasError update();
		PapasError render_top();
		PapasError render_bottom();
		PapasError terminate();
		PapasError changeScene(Papas::Scene* scene);

		//Accessors
		const Papas::Scene* getCurrentScene() const { return currentScene; };

	private:

		Papas::Scene* currentScene;


	};


}