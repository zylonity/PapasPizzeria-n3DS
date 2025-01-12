#include "papas_animations.h"


Papas::AnimatedSprite::AnimatedSprite(const char *spriteSheet, float time, v2 position, v2 scale, float defaultRotation)
{

	createAnim(spriteSheet, time, position, scale, defaultRotation);
}

void Papas::AnimatedSprite::createAnim(const char *spriteSheet, float time, v2 position, v2 scale, float defaultRotation)
{

	s_spriteSheet = C2D_SpriteSheetLoad(spriteSheet);

	numOfSprites = C2D_SpriteSheetCount(s_spriteSheet);
	animTime = time;
	pos = position;
	size = scale;
	rot = defaultRotation;



	for (size_t i = 0; i < numOfSprites; i++)
	{
		C2D_Sprite spr;
		C2D_SpriteFromSheet(&spr, s_spriteSheet, i);
		C2D_SpriteSetPos(&spr, pos.x, pos.y);
		C2D_SpriteSetScale(&spr, size.x, size.y);
		C2D_SpriteSetRotationDegrees(&spr, rot);
		C2D_SpriteSetDepth(&spr, 0.7f);
		C3D_TexSetFilter(spr.image.tex, GPU_LINEAR, GPU_LINEAR); //Adds bilinear filtering, the sprite looks a bit pixelated otherwise
		
		each_sprite.push_back(spr);
	}
	start = osGetTime();
	currentSprite = 0;
}

void Papas::AnimatedSprite::renderAnim(bool loop)
{

	end = osGetTime();

	if (end - start >= animTime)
	{
		if(currentSprite < numOfSprites - 1){
			currentSprite++;
		}
		else if (currentSprite == numOfSprites - 1)
		{
			if(loop){
				currentSprite = 0;
			}
			else{
				finished = true;
			}
			
		}

		start = osGetTime();
	}

	if (!finished)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
}

void Papas::AnimatedSprite::destroyAnim()
{

	C2D_SpriteSheetFree(s_spriteSheet);
}

void Papas::RoyPeeking::renderAnim(bool loop)
{
	end = osGetTime();

	if (end - start >= animTime)
	{
		if(ranOnce == false){
			if (currentSprite < numOfSprites - 1)
			{
				currentSprite++;
			}
			else if (currentSprite == numOfSprites - 1) 
			{
				// We slow down the last few frames, since I had to remove them to load them onto the sprite sheet
				// Don't need to worry about copy cos C2D_Sprite holds a pointer to the texture anyways
				for (size_t i = 0; i < 45; i++)
				{
					each_sprite.push_back(each_sprite[28]);
				}
				numOfSprites += 45;
				ranOnce = true;
			}
		}
		else{
			if (currentSprite < numOfSprites - 1)
			{
				currentSprite++;
			}
			else if (currentSprite == numOfSprites - 1)
			{
				if(loop){
					currentSprite = 16;
				}
				else{
					finished = true;
				}
				
			}
		}
		

		start = osGetTime();
	}

	if (!finished)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
}

void Papas::RoyPeeking::renderAnimBackwards(bool loop)
{

	if(finished == false){
		if (currentSprite == 0)
		{
			currentSprite = 15;
		}
	}


	end = osGetTime();

	if (end - start >= animTime)
	{

		if (currentSprite > 1)
		{
			currentSprite--;
		}
		else if (currentSprite == 1)
		{
			finished = true;
		}

		start = osGetTime();
	}

	if (finished == false)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
}

void Papas::RoyPeeking::resetAnim()
{
	finished = false;
	currentSprite = 0;
}

bool Papas::RoyTakingOrder::renderAnimWithPauses(int pauses, float pauseTime)
{
	end = osGetTime();
	pauseTriggered = false;

	if (paused == false && end - start >= animTime)
	{
		if(currentSprite == 1){
			pauseTriggered = true;
		}
		if (currentSprite < numOfSprites - 1)
		{
			currentSprite++;
		}
		else if (currentSprite == numOfSprites - 1)
		{
			if (currentPauses < pauses)
			{
				
				paused = true;
				currentSprite = 0;
			}
			else
			{
				finished = true;
			}
		}

		start = osGetTime();
	}

	if (!finished && !paused)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}

	if (paused)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
		size_t currentWait = end - start;
		if (currentWait >= pauseTime)
		{
			
			paused = false;
			currentPauses++;
			start = osGetTime();
		}
	}

	if (finished)
	{
		C2D_DrawSprite(&each_sprite[currentSprite]);
	}
	if (pauseTriggered)
	{
		pauseTriggered = false; // Reset the flag immediately
		return true;
	}

	return false;
}

void Papas::RoyTakingOrder::resetAnim(){
	finished = false;
	pauseTriggered = false;
	paused = false;
	currentSprite = 0;
	currentPauses = 0;
}