#ifndef _paintown_allegro_joystick_h
#define _paintown_allegro_joystick_h

#include "../joystick.h"
#include <string>

class AllegroJoystick: public Joystick {
public:
    virtual void poll();
    virtual int getDeviceId() const;
    virtual std::string getName() const;
    virtual Key getKey(int button);
    virtual int getButton(Key key);
    
    virtual std::map<int, std::map<int, double> > getCurrentAxisValues() const;

    virtual ~AllegroJoystick();

    friend class Joystick;
protected:
    AllegroJoystick();
};

#endif
