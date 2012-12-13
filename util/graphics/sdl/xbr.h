#ifndef _paintown_xbr_h
#define _paintown_xbr_h

struct SDL_Surface;

namespace xbr{
    void xbr2x(SDL_Surface * input, SDL_Surface * output);
    void xbr3x(SDL_Surface * input, SDL_Surface * output);
    void xbr4x(SDL_Surface * input, SDL_Surface * output);
}

#endif
