#ifndef _paintown_fire_93c6678306f3542737be4288dc09cfa9
#define _paintown_fire_93c6678306f3542737be4288dc09cfa9

#include <vector>

class Bitmap;

namespace Paintown{

class Fire{
public:
    Fire();

    void update();
    void draw(const Bitmap & work);

    virtual ~Fire();

protected:
    std::vector<int> hotspots;
    unsigned char ** data;
    static const int MAX_COLORS = 100;
    int colors[MAX_COLORS];
};

}

#endif
