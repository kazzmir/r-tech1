#ifndef paintown_input_source_h
#define paintown_input_source_h

/* this class should abstract over actual devices (keyboard, joystick)
 * and return a list of events on request.
 */

class InputSource{
public:
    InputSource();
    InputSource(bool keyboard, int joystick);
    InputSource(const InputSource & copy);
    virtual ~InputSource();

    InputSource & operator=(const InputSource &);

    virtual bool useKeyboard() const;
    virtual int getJoystick() const;

private:
    bool keyboard;
    int joystick;
};

#endif
