struct SDL_Surface;

struct BitmapData{
    BitmapData():
        surface(0){}

    inline SDL_Surface * getSurface() const {
        return surface;
    }

    void setSurface(SDL_Surface * surface){
        this->surface = surface;
    }

    SDL_Surface * surface;
};
