#ifndef _paintown_fire_93c6678306f3542737be4288dc09cfa9
#define _paintown_fire_93c6678306f3542737be4288dc09cfa9

namespace Graphics{
class Bitmap;
}

namespace Paintown{

struct Wisp{
    double x, y;
    int life;
    int angle;
};

class Fire{
public:
    Fire();

    void update();
    void draw(const Graphics::Bitmap & work);

    virtual ~Fire();

protected:
    void updateHotspots();
    void updateWisps();

protected:
    unsigned char ** data;
    /* enough to fill an unsigned char */
    static const int MAX_COLORS = 256;
    int colors[MAX_COLORS];
    static const int MAX_HOTSPOTS = 10;
    double hotspots[MAX_HOTSPOTS];
    double directions[MAX_HOTSPOTS];
    static const int MAX_WISPS = 30;
    Wisp wisps[MAX_WISPS];
};

}

#endif
