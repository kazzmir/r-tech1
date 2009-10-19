#ifndef _paintown_gradient_h
#define _paintown_gradient_h

namespace Effects{

class Gradient{
public:
    Gradient(int size, int startColor, int endColor);

    /* move to next color */
    void update();

    /* get current color */
    int current();

    virtual ~Gradient();

protected:
    int * colors;
    unsigned int size;
    unsigned int index;
};

}

#endif
