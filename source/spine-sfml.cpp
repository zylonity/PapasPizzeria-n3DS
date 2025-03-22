/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated July 28, 2023. Replaces all prior versions.
 *
 * Copyright (c) 2013-2023, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software or
 * otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THE
 * SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "spine-sfml.h"
#include <3ds.h>
#include <citro3d.h>

#ifndef SPINE_MESH_VERTEX_COUNT_MAX
#define SPINE_MESH_VERTEX_COUNT_MAX 1000
#endif

using namespace spine;

// sf::BlendMode blendModes[] = {
// 		sf::BlendMode(sf::BlendMode::SrcAlpha, sf::BlendMode::OneMinusSrcAlpha),
// 		sf::BlendMode(sf::BlendMode::SrcAlpha, sf::BlendMode::One),
// 		sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::OneMinusSrcAlpha),
// 		sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor)};

// sf::BlendMode blendModesPma[] = {
// 		sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha),
// 		sf::BlendMode(sf::BlendMode::One, sf::BlendMode::One),
// 		sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::OneMinusSrcAlpha),
// 		sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor),
// };

SkeletonRenderer *skeletonRenderer = nullptr;

SkeletonDrawable::SkeletonDrawable(SkeletonData *skeletonData, AnimationStateData *stateData) : timeScale(1),
																								usePremultipliedAlpha(false),
																								vertexArray(new std::vector<Papas::Vertex>(skeletonData->getBones().size() * 4)) {
	Bone::setYDown(true);
	skeleton = new (__FILE__, __LINE__) Skeleton(skeletonData);
	ownsAnimationStateData = stateData == 0;
	if (ownsAnimationStateData) stateData = new (__FILE__, __LINE__) AnimationStateData(skeletonData);
	state = new (__FILE__, __LINE__) AnimationState(stateData);
}

SkeletonDrawable::~SkeletonDrawable() {
	delete vertexArray;
	if (ownsAnimationStateData) delete state->getData();
	delete state;
	delete skeleton;
}

void SkeletonDrawable::update(float deltaTime, Physics physics) {
	state->update(deltaTime * timeScale);
	state->apply(*skeleton);
	skeleton->update(deltaTime * timeScale);
	skeleton->updateWorldTransform(physics);
}

// inline void toSFMLColor(uint32_t color, sf::Color *sfmlColor) {
// 	sfmlColor->a = (color >> 24) & 0xFF;
// 	sfmlColor->r = (color >> 16) & 0xFF;
// 	sfmlColor->g = (color >> 8) & 0xFF;
// 	sfmlColor->b = color & 0xFF;
// }

void SkeletonDrawable::draw() const {
	//states.texture = NULL;
	vertexArray->clear();

	if (!skeletonRenderer) skeletonRenderer = new (__FILE__, __LINE__) SkeletonRenderer();
	RenderCommand *command = skeletonRenderer->render(*skeleton);
	while (command) {
		Papas::Vertex vertex;
		float *positions = command->positions;
		float *uvs = command->uvs;
		uint32_t *colors = command->colors;
		uint16_t *indices = command->indices;
		C2D_Image *texture = (C2D_Image*) command->texture;
		glm::vec2 size = {texture->subtex->width, texture->subtex->height};
		for (int i = 0, n = command->numIndices; i < n; ++i) {
			int ii = indices[i];
			int index = ii << 1;
			vertex.position.x = positions[index];
			vertex.position.y = positions[index + 1];
			vertex.texCoords.x = uvs[index] * size.x;
			vertex.texCoords.y = uvs[index + 1] * size.y;
			vertex.colour = colors[ii];
			//toSFMLColor(colors[ii], &vertex.color);
			vertexArray->push_back(vertex);
		}
		BlendMode blendMode = command->blendMode;
		
		//C3D_AlphaBlend();

		//states.blendMode = usePremultipliedAlpha ? blendModesPma[blendMode] : blendModes[blendMode];
		//states.texture = texture;
		//target.draw(*vertexArray, states);

		// C3D_BufInfo* bufInfo = C3D_GetBufInfo();
        // BufInfo_Init(bufInfo);
        // BufInfo_Add(bufInfo, &vertices, sizeof(m3d::Vertex), 2, 0x10);
        // C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);

		// vertexArray->clear();

		 // -- BIND THE TEXTURE --
        // Citro2D’s C2D_Image has an underlying C3D_Tex in texture->tex
        // so we bind that to texture unit 0:
        C3D_TexBind(0, texture->tex);

        // -- SET UP THE GPU ENV FOR TEXTURING + COLOR --
        // e.g. multiply texture color by vertex color (GPU_MODULATE).
        // C3D_TexEnv* env = C3D_GetTexEnv(0);
        // C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
        // C3D_TexEnvOpRgb(env, GPU_REPLACE, 0, 0);
		// C3D_TexEnvOpAlpha(env, GPU_REPLACE, 0, 0);
        // C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

        // -- HANDLE BLENDING IF NEEDED (command->blendMode) --
        // For example:
        // C3D_AlphaBlend(GPU_BLEND_SRC_ALPHA, GPU_BLEND_ONE_MINUS_SRC_ALPHA, ...);

        // -- SET UP ATTRIBUTES (position, UV, color) --
        C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
        AttrInfo_Init(attrInfo);
        // location=0 => x,y
        AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 2);
        // location=1 => u,v
        AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2);
        // location=2 => color
        AttrInfo_AddLoader(attrInfo, 2, GPU_UNSIGNED_BYTE, 4);

        // -- SET UP BUFFER INFO --
        C3D_BufInfo* bufInfo = C3D_GetBufInfo();
        BufInfo_Init(bufInfo);
        // The 3rd parameter is how many attributes you have (3).
        // The last param is a bitmask specifying which attribute slots are used (0x210 is typical for 3).
        // In practice, you may need to experiment or check devkitPro docs.
        BufInfo_Add(bufInfo, vertexArray->data(), sizeof(Papas::Vertex), 3, 0x210);

        // -- DRAW THE VERTICES --
        // Spine's data is typically triangles, so we use GPU_TRIANGLES.
        // The number of vertices is indexCount (since each index is one vertex).
        C3D_DrawArrays(GPU_TRIANGLES, 0, command->numIndices);

        // Clean up
        //delete[] vertices;
		vertexArray->clear();

		command = command->next;
	}
}

void SFMLTextureLoader::load(AtlasPage &page, const String &path) {

	C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoad(path.buffer());
	C2D_Image texture = C2D_SpriteSheetGetImage(spriteSheet, 0);

	// top_bg = C2D_SpriteSheetGetImage(sheet_bg, 1);

	// Texture *texture = new Texture();
	// if (!texture->loadFromFile(path.buffer())) return;

	if (page.magFilter == TextureFilter_Linear) {
		C3D_TexSetFilter(texture.tex, GPU_LINEAR, GPU_LINEAR);
	}
	if (page.uWrap == TextureWrap_Repeat && page.vWrap == TextureWrap_Repeat){
		C3D_TexSetWrap(texture.tex, GPU_REPEAT, GPU_REPEAT);
	} 

	page.texture = &texture;
	glm::vec2 size = {texture.subtex->width, texture.subtex->height};
	page.width = size.x;
	page.height = size.y;
}

void SFMLTextureLoader::unload(void *texture) {
	// C3D_Load
	// delete (Texture *) texture;
}

SpineExtension *spine::getDefaultExtension() {
	return new DefaultSpineExtension();
}
