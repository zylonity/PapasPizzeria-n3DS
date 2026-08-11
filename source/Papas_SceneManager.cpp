#pragma once
#include "Papas_SceneManager.h"

PapasError Papas::SceneManager::init() {
	PapasError ret;

	currentScene = nullptr;

	return PAPAS_OK;
}

PapasError Papas::SceneManager::update() {
	if (currentScene != nullptr) {
		return currentScene->update();
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::render_top() {
	PapasError ret;

	if (currentScene != nullptr) {
		ret = currentScene->render_top();
		ASSERT(ret == PAPAS_OK, "");
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::render_bottom() {
	PapasError ret;

	if (currentScene != nullptr) {
		ret = currentScene->render_bottom();
		ASSERT(ret == PAPAS_OK, "");
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::changeScene(Papas::Scene* scene) {
	PapasError ret;

	//Bin the old scene's art first, both lots at once is more than an o3DS's VRAM has
	if (currentScene) {
		currentScene->terminate();
		delete currentScene;
		currentScene = nullptr; // Ensure no dangling pointer
	}

	if (scene) {
		scene->init(this);
	}
	currentScene = scene;



	return PAPAS_OK;
}

PapasError Papas::SceneManager::terminate() {
	if (currentScene) {
		currentScene->terminate();
		delete currentScene;
		currentScene = nullptr;
	}

	return PAPAS_OK;
}
