#include "Papas_Framework.h"
#include "Papas_Renderer.h"
#include <assert.h>

PapasError Papas::Framework::init() {
	PapasError ret;

	// Initialize our Renderer
	m_pRenderer = new Papas::Renderer;
	assert(m_pRenderer != nullptr);							// Always checking with any new if we have successfully allocated memory for it
	ret = m_pRenderer->init();
	assert(ret == PAPAS_OK);									// Always checking if we have a valid return code

	return PAPAS_OK;
}

PapasError Papas::Framework::update() {
	PapasError ret;

	ret = m_pRenderer->update();
	assert(ret == PAPAS_OK);

	return PAPAS_OK;
}

PapasError Papas::Framework::terminate() {
	PapasError ret;

	ret = m_pRenderer->terminate();
	assert(ret == PAPAS_OK);
	delete m_pRenderer;
	m_pRenderer = nullptr;
	assert(m_pRenderer == nullptr);


	return PAPAS_OK;
}