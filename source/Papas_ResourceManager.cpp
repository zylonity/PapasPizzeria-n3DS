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

    rng_type::result_type const seedval = osGetTime();
    rng.seed(seedval);

    return PAPAS_OK;
}

// Random int between small and big, both ends included
int Papas::ResourceManager::randomNumber(int small, int big)
{



    std::uniform_int_distribution<rng_type::result_type> udist(small, big);
    rng_type::result_type random_number = udist(rng);

    return random_number;
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

PapasError Papas::ResourceManager::loadSfx(const char *name, const char *wav_file)
{
    sfx.insert({name, Mix_LoadWAV(wav_file)});

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::endMusicPlayer()
{

    Mix_CloseAudio();
    return PAPAS_OK;
}

PapasError Papas::ResourceManager::playSfx(const char *name)
{

    Mix_PlayChannel(-1, sfx[name], 0);
    return PAPAS_OK;
}

// Loops forever until you stop the channel it hands back
int Papas::ResourceManager::playSfxLoop(const char *name)
{
    return Mix_PlayChannel(-1, sfx[name], -1);
}

PapasError Papas::ResourceManager::stopSfxChannel(int channel)
{
    if (channel >= 0)
    {
        Mix_HaltChannel(channel);
    }
    return PAPAS_OK;
}

PapasError Papas::ResourceManager::playMusic(const char *name)
{

    Mix_PlayMusic(songs[name], -1);


    return PAPAS_OK;
}

PapasError Papas::ResourceManager::playMusicOnce(const char *name)
{
    Mix_PlayMusic(songs[name], 1);

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

// Fades the new song in over a second, used when changing stations
PapasError Papas::ResourceManager::switchMusic(const char *name)
{

    Mix_FadeInMusic(songs[name], -1, 1000);

    return PAPAS_OK;
}

PapasError Papas::ResourceManager::terminate()
{
    for (auto &it : songs)
    {
        Mix_FreeMusic(it.second);
    }

    for (auto &it : sfx)
    {
        Mix_FreeChunk(it.second);
    }

    Mix_CloseAudio();
    SDL_Quit();

    return PAPAS_OK;
}
