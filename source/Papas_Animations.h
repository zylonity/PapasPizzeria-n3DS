#pragma once

#include "Papas_Constants.h"
#include "Papas_Utils.h"
#include <3ds.h>
#include <citro2d.h>
#include <vector>
#include <chrono>

namespace Papas {

	class AnimatedSprite {
	public:
		AnimatedSprite() {};
		AnimatedSprite(const char *spriteSheet, float time, v2 position = v2(0), v2 scale = v2(0), float rotation = 0);
		void createAnim(const char *spriteSheet, float time, v2 position = v2(0), v2 scale = v2(0), float rotation = 0);
		void destroyAnim();

		virtual void renderAnim(bool loop);

		void setPosition(v2 postoSet);
		void setRotation(v2 postoSet);

	protected:
		
		std::vector<C2D_Sprite> each_sprite;
		C2D_SpriteSheet s_spriteSheet;

		v2 pos;
		v2 size;
		float rot;

		u64 start;
		u64 end;
		int numOfSprites;
		float animTime;

		int currentSprite;

		bool finished;

	};

	class RoyPeeking : public AnimatedSprite
	{
	public:
		using AnimatedSprite::AnimatedSprite;

		void renderAnim(bool loop) override;
		void resetAnim();
		void renderAnimBackwards(bool loop);

	private:
		bool ranOnce = false;
	};

	class RoyTakingOrder : public AnimatedSprite
	{
	public:
		using AnimatedSprite::AnimatedSprite;

		bool renderAnimWithPauses(int pauses, float pauseTime);
		void resetAnim();

	private:
		bool pauseTriggered = false;
		bool paused = false;
		int currentPauses = 0;
	};

	class Customer_CutOutAnimation{
		public:
		void init();
		private:
		int temp;
	};
}