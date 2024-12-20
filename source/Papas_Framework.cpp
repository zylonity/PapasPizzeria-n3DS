#include "Papas_Framework.h"
#include "Papas_Renderer.h"
#include "Papas_SceneManager.h"



PapasError Papas::Framework::init() {
	PapasError ret;

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



	return PAPAS_OK;
}

PapasError Papas::Framework::update() {
	PapasError ret;

	ret = m_pRenderer->update(m_pSceneManager);
	ASSERT(ret == PAPAS_OK, "");



	return PAPAS_OK;
}

PapasError Papas::Framework::terminate() {
	PapasError ret;

	ret = m_pRenderer->terminate();
	ASSERT(ret == PAPAS_OK, "");
	delete m_pRenderer;
	m_pRenderer = nullptr;
	ASSERT(m_pRenderer == nullptr, "");

	ret = m_pSceneManager->terminate();
	ASSERT(ret == PAPAS_OK, "");
	delete m_pSceneManager;
	m_pSceneManager = nullptr;
	ASSERT(m_pSceneManager == nullptr, "");


	return PAPAS_OK;
}