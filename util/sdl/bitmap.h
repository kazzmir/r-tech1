struct SDL_Surface;

struct BitmapData{
    BitmapData():
        surface(0){}

    inline SDL_Surface * getSurface() const {
        return surface;
    }

    SDL_Surface * surface;
};
