#include "Papas_Framework.h"
#include <3ds.h>
#include <citro2d.h>
#include "spine-sfml.h"
#include <string>
//===============================================================================
// Framework is constructed as Global
Papas::Framework g_framework;
//===============================================================================

spine::SkeletonData* readSkeletonJsonData(const spine::String &filename, spine::Atlas *atlas, float scale) {
	spine::SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonDataFile(filename);
	if (!skeletonData) {
		printf("%s\n", json.getError().buffer());
		exit(0);
	}
	return skeletonData;
}

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

	//C2D_SpriteSheet top_bg = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	//C2D_Image top_bg_img = C2D_SpriteSheetGetImage(top_bg, 1);




	std::string json = "romfs:celestial-circus-pro.json";
	std::string binaryName = "romfs:celestial-circus-pro.skel";
	std::string atlasName = "romfs:celestial-circus-pma.atlas";
	
	spine::SFMLTextureLoader textureLoader;

	spine::Atlas* atlas = new spine::Atlas(atlasName.c_str(), &textureLoader);
	float scale = 1.0f;
	//std::cout << atlas->getPages()[0]->texturePath.buffer() << '\n';
	spine::SkeletonData* skeletonData = readSkeletonJsonData(json.c_str(), atlas, scale);

	spine::SkeletonDrawable drawable;
	drawable.timeScale = 1;
	drawable.setUsePremultipliedAlpha(true);

	spine::Skeleton* skeleton = drawable.skeleton;
	skeleton->setPosition(320, 480);
	skeleton->setScaleX(0.2);
	skeleton->setScaleY(0.2);
	skeleton->updateWorldTransform(spine::Physics_Update);

drawable.state->setAnimation(0, "swing", true);

	while (aptMainLoop()) {
		//ret = g_framework.update();
		//ASSERT(ret == PAPAS_OK, "");

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		//C2D_TargetClear(topScreen, C2D_Color32(0x00, 0x00, 0x00, 0xff));
		//C2D_SceneBegin(topScreen);

		C3D_FrameDrawOn(topScreen);
		//C2D_DrawImageAt(top_bg_img, 0, 0, 0, NULL, 1, 1);
		//drawable.update(33.33f);
		drawable.draw();
		

		C3D_FrameEnd(0);
		gspWaitForVBlank();
	}
	
	//ret = g_framework.terminate();
	//ASSERT(ret == PAPAS_OK, "");

	return 0;
}