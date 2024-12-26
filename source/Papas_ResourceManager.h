#pragma once
//===============================================================================
// name: Papas_ResourceManager.h
//===============================================================================

#include "Papas_Constants.h"
#include "Papas_Renderer.h"

//Forward declarations
typedef struct _Mix_Music Mix_Music;

namespace Papas
{

	class ResourceManager
	{
	public:
		PapasError init();

		PapasError terminate();

		PapasError playMusic(const char* ogg_file);
		PapasError stopMusic();
		PapasError pauseMusic();

		//===============================================================================
		// Singleton Implementations
		static ResourceManager& getInstance()
		{
			static ResourceManager	instance; // Guaranteed to be destroyed.
			return instance;
		}
		// Make deleted functions public for nicer error messages (~ Scott Myers)
		ResourceManager(ResourceManager const&)	= delete;	// Copy constructor
		void operator=(ResourceManager const&)			= delete;	// Assignment Operator
		//===============================================================================
	private:
		Mix_Music *music;

		//===============================================================================
		// Singleton Implementations (Banned functions to prevent a new instance)
		ResourceManager()
		{
		} // Default Constructor private so can only be called from within
		  //===============================================================================
	};
}
//===============================================================================

