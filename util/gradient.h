#ifndef _paintown_gradient_h
#define _paintown_gradient_h

#include "util/bitmap.h"

namespace Effects{

class Gradient{
public:
    Gradient(int size, Graphics::Color startColor, Graphics::Color endColor);
    Gradient(const Gradient & copy);

    Gradient & operator=(const Gradient & copy);

    /* move to next color. update is an alias for `forward' */
    void update();
    void forward();
    void backward();

    /* start at startColor */
    void reset();

    /* get current color */
    Graphics::Color current() const;
    Graphics::Color current(int offset) const;

    virtual ~Gradient();

protected:
    Graphics::Color * colors;
    unsigned int size;
    unsigned int index;
};

}

#endif
