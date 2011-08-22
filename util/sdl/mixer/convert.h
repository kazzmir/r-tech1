#ifndef _paintown_sdl_mixer_convert_h
#define _paintown_sdl_mixer_convert_h

struct SDL_AudioSpec;
struct Mix_Chunk;

#ifdef __cplusplus
extern "C" {
#endif
void convertAudio(SDL_AudioSpec * wav, SDL_AudioSpec * mixer, Mix_Chunk *chunk);
#ifdef __cplusplus
}
#endif

#endif
