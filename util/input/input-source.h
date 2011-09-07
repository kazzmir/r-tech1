#ifndef paintown_input_source_h
#define paintown_input_source_h

/* this class should abstract over actual devices (keyboard, joystick)
 * and return a list of events on request.
 */

class InputSource{
public:
    InputSource();
    virtual ~InputSource();

    virtual int getJoystick() const;
};

#endif
