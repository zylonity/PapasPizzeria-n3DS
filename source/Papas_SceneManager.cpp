#pragma once
#include "Papas_SceneManager.h"

PapasError Papas::SceneManager::init() {
	PapasError ret;

	currentScene = nullptr;

	return PAPAS_OK;
}

PapasError Papas::SceneManager::update() {
	PapasError ret;

	if (currentScene != nullptr) {
		ret = currentScene->update();
		ASSERT(ret == PAPAS_OK, "Updating scene failed");
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::render_top() {
	PapasError ret;

	if (currentScene != nullptr) {
		ret = currentScene->render_top();
		//ASSERT(ret == PAPAS_OK);
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::render_bottom() {
	PapasError ret;

	if (currentScene != nullptr) {
		ret = currentScene->render_bottom();
		//ASSERT(ret == PAPAS_OK);
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::changeScene(Papas::Scene* scene) {
	PapasError ret;

	if (currentScene) {
		currentScene->terminate();
		delete currentScene;
	}
	currentScene = scene;

	if (currentScene) {
		currentScene->init();
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::terminate() {
	PapasError ret;

	currentScene->terminate();
	delete currentScene;
	currentScene = nullptr;

	return PAPAS_OK;
}
