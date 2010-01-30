#ifndef _paintown_fire_93c6678306f3542737be4288dc09cfa9
#define _paintown_fire_93c6678306f3542737be4288dc09cfa9

class Bitmap;

namespace Paintown{

class Fire{
public:
    Fire();

    void update();
    void draw(const Bitmap & work);

    virtual ~Fire();

protected:
    void updateHotspots();

protected:
    unsigned char ** data;
    /* enough to fill an unsigned char */
    static const int MAX_COLORS = 256;
    int colors[MAX_COLORS];
    static const int MAX_HOTSPOTS = 10;
    double hotspots[MAX_HOTSPOTS];
    double directions[MAX_HOTSPOTS];
};

}

#endif
