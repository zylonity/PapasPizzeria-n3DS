#pragma once
#include "Papas_Constants.h"
#include "Papas_Utils.h"
#include <tremor/ivorbisfile.h>
#include <tremor/ivorbiscodec.h>
#include <3ds.h>

namespace Papas
{

    class SoundPlayer
    {
    public:
        PapasError init();
        PapasError update();
        PapasError render_top();
        PapasError render_bottom();
        PapasError terminate();

    private:

        // --- AUDIO START DEFINITIONS ---

        const char *PATH = "romfs:/music/justguitar_music.ogg"; // Path to Ogg Vorbis file to play

        const int THREAD_AFFINITY = -1;        // Execute thread on any core
        const int THREAD_STACK_SZ = 32 * 1024; // 32kB stack for audio thread

        // ---- END DEFINITIONS ----

        ndspWaveBuf s_waveBufs[3];
        int16_t *s_audioBuffer = NULL;

        LightEvent s_event;
        volatile bool s_quit = false; // Quit flag

        const char *vorbisStrError(int error);
    };

}