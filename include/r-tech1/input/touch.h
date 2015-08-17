#ifndef _rtech1_touch
#define _rtech1_touch

#include "../pointer.h"

/* Manages devices that respond to touch input, such as android/ios */

namespace DeviceInput{

class Touch{
public:
    Touch();
    virtual void poll() = 0;
    virtual ~Touch();
};

Util::ReferenceCount<Touch> getTouchDevice();

}

#endif
