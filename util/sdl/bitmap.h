struct SDL_Surface;

struct BitmapData{
    BitmapData():
        surface(0){}

    SDL_Surface * surface;
};
