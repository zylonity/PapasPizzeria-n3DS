#include "Papas_Sound.h"

PapasError Papas::SoundPlayer::init()
{
    PapasError ret;

    return PAPAS_OK;
}

const char *Papas::SoundPlayer::vorbisStrError(int error)
{
    switch (error)
    {
    case OV_FALSE:
        return "OV_FALSE: A request did not succeed.";
    case OV_HOLE:
        return "OV_HOLE: There was a hole in the page sequence numbers.";
    case OV_EREAD:
        return "OV_EREAD: An underlying read, seek or tell operation "
               "failed.";
    case OV_EFAULT:
        return "OV_EFAULT: A NULL pointer was passed where none was "
               "expected, or an internal library error was encountered.";
    case OV_EIMPL:
        return "OV_EIMPL: The stream used a feature which is not "
               "implemented.";
    case OV_EINVAL:
        return "OV_EINVAL: One or more parameters to a function were "
               "invalid.";
    case OV_ENOTVORBIS:
        return "OV_ENOTVORBIS: This is not a valid Ogg Vorbis stream.";
    case OV_EBADHEADER:
        return "OV_EBADHEADER: A required header packet was not properly "
               "formatted.";
    case OV_EVERSION:
        return "OV_EVERSION: The ID header contained an unrecognised "
               "version number.";
    case OV_EBADPACKET:
        return "OV_EBADPACKET: An audio packet failed to decode properly.";
    case OV_EBADLINK:
        return "OV_EBADLINK: We failed to find data we had seen before or "
               "the stream was sufficiently corrupt that seeking is "
               "impossible.";
    case OV_ENOSEEK:
        return "OV_ENOSEEK: An operation that requires seeking was "
               "requested on an unseekable stream.";
    default:
        return "Unknown error.";
    }

    return PAPAS_OK;
}

PapasError Papas::SoundPlayer::render_top()
{

    return PAPAS_OK;
}

PapasError Papas::SoundPlayer::update()
{

    hidScanInput();

    // Respond to user input
    u32 kDown = hidKeysDown();
    if (kDown & KEY_START)
        return PAPAS_NOT_OK; // break in order to return to hbmenu

    return PAPAS_OK;
}

PapasError Papas::SoundPlayer::render_bottom()
{

    return PAPAS_OK;
}

PapasError Papas::SoundPlayer::terminate()
{

    return PAPAS_OK;
}