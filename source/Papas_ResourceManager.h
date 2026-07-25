#pragma once

#include "Papas_Constants.h"
#include <unordered_map>
#include <random>
#include <string>

//Forward declarations
typedef struct _Mix_Music Mix_Music;
typedef struct Mix_Chunk;

namespace Papas
{

	class ResourceManager
	{
	public:
		PapasError init();
		PapasError initMusicPlayer();
		PapasError endMusicPlayer();

		PapasError terminate();

		PapasError loadSong(const char *name, const char *ogg_file);
		PapasError loadSfx(const char *name, const char *wav_file);
		PapasError playMusic(const char *name);
		PapasError playMusicOnce(const char *name);
		PapasError playSfx(const char *name);
		int playSfxLoop(const char *name);
		PapasError stopSfxChannel(int channel);
		PapasError stopMusic();
		PapasError pauseMusic();
		int randomNumber(int small, int big);
		PapasError switchMusic(const char *name);

		// Shared instance.
		static ResourceManager& getInstance()
		{
			static ResourceManager	instance; // Guaranteed to be destroyed.
			return instance;
		}
		// Make deleted functions public for nicer error messages (~ Scott Myers)
		ResourceManager(ResourceManager const&)	= delete;	// Copy constructor
		void operator=(ResourceManager const&)			= delete;	// Assignment Operator
	private:
		std::unordered_map<std::string, Mix_Music*> songs;
		std::unordered_map<std::string, Mix_Chunk*> sfx;

		typedef std::mt19937 rng_type;
		rng_type rng;

		// Don't allow extra instances.
		ResourceManager()
		{
		} // Default Constructor private so can only be called from within
	};
}
