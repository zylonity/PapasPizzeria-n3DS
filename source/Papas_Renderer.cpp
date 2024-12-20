#include "Papas_Renderer.h"

#include "Papas_Scenes.h"

PapasError Papas::Renderer::init(Papas::SceneManager* sceneManager) {
	PapasError ret;

	// Init various things
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	
//Debugging console
#ifdef DEBUGGING_TOP
	consoleInit(GFX_TOP, NULL);
#endif // DEBUGGING_TOP

#ifdef DEBUGGING_BOTTOM
	consoleInit(GFX_BOTTOM, NULL);
#endif // DEBUGGING_TOP

	
#ifndef DEBUGGING_TOP
	// Create a C3D render target
	topRenderTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
#endif

#ifndef DEBUGGING_BOTTOM
	bottomRenderTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
#endif
	

	//Init the first scene
	sceneManager->changeScene(new Papas::MainMenu());


	return PAPAS_OK;
}

PapasError Papas::Renderer::update(Papas::SceneManager* sceneManager) {
	PapasError ret;

	//hidScanInput();

	//// Respond to user input
	//u32 kDown = hidKeysDown();
	//if (kDown & KEY_START)
	//	return PAPAS_NOT_OK; // break in order to return to hbmenu

	////sceneManager->changeScene();

	ret = sceneManager->update();
	ASSERT(ret == PAPAS_OK, "");

	ret = render(sceneManager);
	ASSERT(ret == PAPAS_OK, "");

	return PAPAS_OK;
}

PapasError Papas::Renderer::render(Papas::SceneManager* sceneManager) {
	PapasError ret;

#ifndef DEBUGGING_TOP
	// Render the scene
	C2D_TargetClear(topRenderTarget, C2D_Color32(0x00, 0x00, 0x00, 0xff));
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_SceneBegin(topRenderTarget);

	//Render the scene's top screen
	sceneManager->render_top();


	C3D_FrameEnd(0);
#endif
	
#ifndef DEBUGGING_BOTTOM
	// Render the scene
	C2D_TargetClear(bottomRenderTarget, C2D_Color32(0x00, 0x00, 0x00, 0xff));
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_SceneBegin(bottomRenderTarget);

	//Render the scene's bottom screen
	sceneManager->render_bottom();

	C3D_FrameEnd(0);
#endif
	


	return PAPAS_OK;
}

PapasError Papas::Renderer::terminate() {
	PapasError ret;

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();

	return PAPAS_OK;
}