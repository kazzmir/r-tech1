#ifndef _paintown_gradient_h
#define _paintown_gradient_h

namespace Effects{

class Gradient{
public:
    Gradient(int size, int startColor, int endColor);

    /* move to next color. update is an alias for `forward' */
    void update();
    void forward();
    void backward();

    /* start at startColor */
    void reset();

    /* get current color */
    int current();
    int current(int offset);

    virtual ~Gradient();

protected:
    int * colors;
    unsigned int size;
    unsigned int index;
};

}

#endif
