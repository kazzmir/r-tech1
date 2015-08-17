#ifndef _rtech1_touch
#define _rtech1_touch

#include "../pointer.h"

/* Manages devices that respond to touch input, such as android/ios */

namespace Input{

class Touch{
public:
    Touch();
    virtual ~Touch();
};

Util::ReferenceCount<Touch> getTouchDevice();

}

#endif
