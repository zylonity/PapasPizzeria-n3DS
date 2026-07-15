#include "Papas_Framework.h"
#include "Papas_Renderer.h"
#include "Papas_SceneManager.h"
#include "Papas_ResourceManager.h"


PapasError Papas::Framework::init() {
	PapasError ret;

	Papas::ResourceManager::getInstance().init();

	// Initialize our scene manager
	m_pSceneManager = new Papas::SceneManager;
	ASSERT(m_pSceneManager != nullptr, "");							// Always checking with any new if we have successfully allocated memory for it
	ret = m_pSceneManager->init();
	ASSERT(ret == PAPAS_OK, "");									// Always checking if we have a valid return code

	// Initialize our Renderer
	m_pRenderer = new Papas::Renderer;
	ASSERT(m_pRenderer != nullptr, "");							// Always checking with any new if we have successfully allocated memory for it
	ret = m_pRenderer->init(m_pSceneManager);
	ASSERT(ret == PAPAS_OK, "");									// Always checking if we have a valid return code

	//Papas::ResourceManager::getInstance().init();

	return PAPAS_OK;
}

PapasError Papas::Framework::update() {
	return m_pRenderer->update(m_pSceneManager);
}

PapasError Papas::Framework::terminate() {
	PapasError ret;

	// Scenes own Citro2D resources, so release them while the renderer is
	// still alive.
	ret = m_pSceneManager->terminate();
	ASSERT(ret == PAPAS_OK, "");
	delete m_pSceneManager;
	m_pSceneManager = nullptr;
	ASSERT(m_pSceneManager == nullptr, "");

	ret = m_pRenderer->terminate();
	ASSERT(ret == PAPAS_OK, "");
	delete m_pRenderer;
	m_pRenderer = nullptr;
	ASSERT(m_pRenderer == nullptr, "");

	Papas::ResourceManager::getInstance().terminate();

	return PAPAS_OK;
}
