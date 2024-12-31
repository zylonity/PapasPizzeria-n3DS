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

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::initMusicPlayer()
{

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    return PAPAS_OK;
}

PapasError Papas::ResourceManager::loadSong(const char *name, const char *ogg_file)
{
    songs.insert({name, Mix_LoadMUS(ogg_file)});

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::endMusicPlayer()
{

    Mix_CloseAudio();
    return PAPAS_OK;
}

PapasError Papas::ResourceManager::playMusic(const char *name)
{

    // music = Mix_LoadMUS(ogg_file);
    // if (music == NULL)
    // {
    //     Mix_CloseAudio();
    //     SDL_Quit();
    //     return PAPAS_NOT_OK;
    // }

    Mix_PlayMusic(songs[name], -1);

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

PapasError Papas::ResourceManager::switchMusic(const char *name)
{
    // if (Mix_PlayingMusic())
    // {
    //     Mix_HaltMusic();
    // }

    // if (music != NULL)
    // {
    //     Mix_FreeMusic(music);
    // }

    // Load the new music file
    //music = Mix_LoadMUS(ogg_file);
    //Mix_FadeOutMusic(1000);
    Mix_FadeInMusic(songs[name], -1, 1000);
    //Mix_PlayMusic(songs[name], -1);

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::terminate()
{
    for (auto &it : songs)
    {
        Mix_FreeMusic(it.second);
    }
    Mix_CloseAudio();
    SDL_Quit();

    return PAPAS_OK;
}
