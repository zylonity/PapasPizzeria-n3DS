#include "papas_utils.h"

#include <stdio.h>
#include <stdlib.h>

PapasError Papas::Button::createButton(C2D_SpriteSheet& spriteSheet, int unpressed, int pressed, v2 position) {

	// Load the sprites
	img_Unpressed = C2D_SpriteSheetGetImage(spriteSheet, unpressed);
	img_Pressed = C2D_SpriteSheetGetImage(spriteSheet, pressed);

	pos = position;

	return PAPAS_OK;
}

bool Papas::Button::showButton(touchPosition& touch) {

	hitBox.left = pos.x;
	hitBox.top = pos.y;
	hitBox.height = img_Pressed.subtex->height;
	hitBox.width = img_Pressed.subtex->width;

	//Detect touch
	//If between left and between right and between top and between bottom
	if (touch.px > hitBox.left && touch.px < hitBox.left + hitBox.width && touch.py > hitBox.top && touch.py < hitBox.top + hitBox.height) {
		C2D_DrawImageAt(img_Pressed, pos.x, pos.y, 1, NULL, 1, 1);
		return true;
	}
	else {
		C2D_DrawImageAt(img_Unpressed, pos.x, pos.y, 1, NULL, 1, 1);
		return false;
	}

}