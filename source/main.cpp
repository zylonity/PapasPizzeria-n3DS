#include "Papas_Framework.h"
#include <3ds.h>
#include <citro2d.h>
//===============================================================================
// Framework is constructed as Global
Papas::Framework g_framework;
//===============================================================================


int main(int argc, char* argv[]) {
    
	PapasError ret;

	// Initialize our framework. This encapsulates all systems.
	ret = g_framework.init();
	ASSERT(ret == PAPAS_OK, "");
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	C3D_RenderTarget* topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	while (aptMainLoop()) {
		//ret = g_framework.update();
		//ASSERT(ret == PAPAS_OK, "");

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		C2D_TargetClear(topScreen, C2D_Color32(0x00, 0x00, 0x00, 0xff));
		C2D_SceneBegin(topScreen);


		

		C3D_FrameEnd(0);
		gspWaitForVBlank();
	}
	
	//ret = g_framework.terminate();
	//ASSERT(ret == PAPAS_OK, "");

	return 0;
}