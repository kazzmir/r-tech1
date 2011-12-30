#ifndef _hqx_h
#define _hqx_h

#include <stdint.h>

struct SDL_Surface;

namespace hq2x{
    void filter_render_565(SDL_Surface * input, SDL_Surface * output);
}

namespace hq4x{
    void filter_render_565(SDL_Surface * input, SDL_Surface * output);
}

#endif
