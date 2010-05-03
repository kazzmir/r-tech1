struct SDL_Surface;

struct BitmapData{
    BitmapData():
        surface(0),
        clip_left(0),
        clip_right(0),
        clip_top(0),
        clip_bottom(0){}

    inline SDL_Surface * getSurface() const {
        return surface;
    }

    inline void setClip(int left, int top, int right, int bottom) const {
        clip_left = left;
        clip_right = right;
        clip_top = top;
        clip_bottom = bottom;
    }

    bool isClipped(int x, int y) const {
        return (x < clip_left ||
                y < clip_top ||
                x > clip_right ||
                y > clip_bottom);
    }

    void setSurface(SDL_Surface * surface);

    SDL_Surface * surface;
    mutable int clip_left, clip_right;
    mutable int clip_top, clip_bottom;
};
