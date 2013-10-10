#include "convert.h"
#include "SDL_mixer.h"
#include "util/sound/audio.h"

Util::Encoding encoding(int format){
    switch (format){
        case AUDIO_U8: return Util::Unsigned8;
        case AUDIO_S16: return Util::Signed16;
        case AUDIO_U16: return Util::Unsigned16;
#if SDL_VERSION_ATLEAST(1, 3, 0)
        case AUDIO_F32LSB:
        case AUDIO_F32MSB: return Util::Float32;
#endif
    }
    return Util::Signed16;
}

/* use SDL to convert between formats */
int convertFormat(SDL_AudioSpec * wav, int format, Mix_Chunk * chunk){
    SDL_AudioCVT wavecvt;
    int samplesize;
    if (SDL_BuildAudioCVT(&wavecvt,
                          wav->format, wav->channels, wav->freq,
                          format, wav->channels, wav->freq) < 0){
        SDL_FreeWAV(chunk->abuf);
        free(chunk);
        return 1;
    }
    samplesize = ((wav->format & 0xFF)/8)*wav->channels;
    wavecvt.len = chunk->alen & ~(samplesize-1);
    wavecvt.buf = (Uint8 *)malloc(wavecvt.len*wavecvt.len_mult);
    if ( wavecvt.buf == NULL ) {
        SDL_SetError("Out of memory");
        SDL_FreeWAV(chunk->abuf);
        free(chunk);
        return 1;
    }
    memcpy(wavecvt.buf, chunk->abuf, chunk->alen);
    SDL_FreeWAV(chunk->abuf);

    if ( SDL_ConvertAudio(&wavecvt) < 0 ) {
        free(wavecvt.buf);
        free(chunk);
        return 1;
    }
    chunk->abuf = wavecvt.buf;
    chunk->alen = wavecvt.len_cvt;
    return 0;
}

extern "C" int convertAudio(SDL_AudioSpec * wav, SDL_AudioSpec * mixer, Mix_Chunk *chunk){
    // printf("Convert format %d, channels %d, frequency %d to format %d, channels %d, frequency %d\n", wav->format, wav->channels, wav->freq, mixer->format, mixer->channels, mixer->freq);
    if (convertFormat(wav, mixer->format, chunk)){
        printf("Could not convert format!\n");
        return 1;
    }

    // printf("Mixer format %d to encoding %d\n", mixer->format, encoding(mixer->format));
    /* format is now what the mixer wants */
    Util::AudioConverter convert(encoding(mixer->format), wav->channels, wav->freq,
                                 encoding(mixer->format), mixer->channels, mixer->freq);
    unsigned int size = convert.convertedLength(chunk->alen);
    unsigned char * data = (unsigned char *) malloc(size > chunk->alen ? size : chunk->alen);
    memcpy(data, chunk->abuf, chunk->alen);
    convert.convert(data, chunk->alen); 
    SDL_FreeWAV(chunk->abuf);
    
    chunk->abuf = data;
    chunk->alen = size;

    return 0;
}
