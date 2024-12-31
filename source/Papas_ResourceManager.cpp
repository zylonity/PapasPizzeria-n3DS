//===============================================================================
// name: Papas_ResourceManager.cpp
// desc: Manages all resources. Is a SINGLETON
// auth: Khaleel Brewesh
//===============================================================================

#include "Papas_ResourceManager.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

PapasError Papas::ResourceManager::init()
{

    SDL_Init(SDL_INIT_AUDIO);

    //

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::initMusicPlayer()
{

    //SDL_Init(SDL_INIT_AUDIO);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::endMusicPlayer()
{

    // SDL_Init(SDL_INIT_AUDIO);
    Mix_CloseAudio();
    //Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::playMusic(const char *ogg_file)
{

    music = Mix_LoadMUS(ogg_file);
    if (music == NULL)
    {
        Mix_CloseAudio();
        SDL_Quit();
        return PAPAS_NOT_OK;
    }

    Mix_PlayMusic(music, -1);

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::pauseMusic()
{
    Mix_PauseMusic();
    return PAPAS_OK;
}


PapasError Papas::ResourceManager::stopMusic()
{
    Mix_HaltMusic();
    return PAPAS_OK;
}

PapasError Papas::ResourceManager::switchMusic(const char *ogg_file)
{
    if (Mix_PlayingMusic())
    {
        Mix_HaltMusic();
    }

    if (music != NULL)
    {
        Mix_FreeMusic(music);
    }

    // Load the new music file
    music = Mix_LoadMUS(ogg_file);

    Mix_PlayMusic(music, -1);

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::terminate()
{
    if(music == NULL){
        Mix_FreeMusic(music);
    }
    Mix_CloseAudio();
    SDL_Quit();

    return PAPAS_OK;
}
