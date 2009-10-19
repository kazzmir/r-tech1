#ifndef _paintown_gradient_glow_h
#define _paintown_gradient_glow_h

namespace Effects{

class GradientGlow{
public:
    GradientGlow(int size, int startColor, int endColor);

    /* move to next color */
    void update();

    /* get current color */
    int current();

    virtual ~GradientGlow();

protected:
    int * colors;
    unsigned int size;
    unsigned int index;
};

}

#endif
