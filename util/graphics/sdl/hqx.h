#ifndef _paintown_hqx_h
#define _paintown_hqx_h

struct SDL_Surface;

namespace hq2x{
    void hq2x(SDL_Surface * input, SDL_Surface * output);
}

namespace hqx{
    void hq3x(SDL_Surface * input, SDL_Surface * output);
    void hq4x(SDL_Surface * input, SDL_Surface * output);
}

#endif
