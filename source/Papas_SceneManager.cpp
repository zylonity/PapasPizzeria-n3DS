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
		currentScene->update();
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::render_top() {
	PapasError ret;

	if (currentScene != nullptr) {
		currentScene->render_top();
	}

	return PAPAS_OK;
}

PapasError Papas::SceneManager::render_bottom() {
	PapasError ret;

	if (currentScene != nullptr) {
		currentScene->render_bottom();
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


	return PAPAS_OK;
}

PapasError Papas::Scene::init() {
	PapasError ret;


	return PAPAS_OK;
}

PapasError Papas::Scene::update() {
	PapasError ret;


	return PAPAS_OK;
}

PapasError Papas::Scene::terminate() {
	PapasError ret;


	return PAPAS_OK;
}