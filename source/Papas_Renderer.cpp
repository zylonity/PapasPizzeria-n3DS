#include "Papas_Renderer.h"
#include "Papas_Scenes.h"
#include "Papas_Stereo.h"

PapasError Papas::Renderer::init(Papas::SceneManager* sceneManager) {
	//PapasError ret;

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
	// Give each eye a target; draw the right one only when 3D is on.
	gfxSet3D(true);
	topRenderTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	topRightRenderTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
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

	ret = sceneManager->update();
	if (ret != PAPAS_OK)
		return ret;

	ret = render(sceneManager);
	ASSERT(ret == PAPAS_OK, "");

	return PAPAS_OK;
}

PapasError Papas::Renderer::render(Papas::SceneManager* sceneManager) {
	//PapasError ret;

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

#ifndef DEBUGGING_TOP
	float slider = osGet3DSliderState();

	// Render the scene
	C2D_TargetClear(topRenderTarget, C2D_Color32(0x00, 0x00, 0x00, 0xff));
	C2D_SceneBegin(topRenderTarget);
	Papas::Stereo::beginEye(Papas::Stereo::EyeLeft, slider);

	//Render the scene's top screen
	sceneManager->render_top();

	// Skip the right-eye pass when 3D is off, but clear stale emulator output.
	C2D_TargetClear(topRightRenderTarget, C2D_Color32(0x00, 0x00, 0x00, 0xff));
	if (slider > 0.0f)
	{
		C2D_SceneBegin(topRightRenderTarget);
		Papas::Stereo::beginEye(Papas::Stereo::EyeRight, slider);
		sceneManager->render_top();
	}
	Papas::Stereo::endEye();
#endif
	
#ifndef DEBUGGING_BOTTOM
	// Render the scene
	C2D_TargetClear(bottomRenderTarget, C2D_Color32(0x00, 0x00, 0x00, 0xff));
	//C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_SceneBegin(bottomRenderTarget);

	//Render the scene's bottom screen
	sceneManager->render_bottom();

#endif

	C3D_FrameEnd(0);
	gspWaitForVBlank();
	return PAPAS_OK;
}

PapasError Papas::Renderer::terminate() {
	//PapasError ret;

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();

	return PAPAS_OK;
}
