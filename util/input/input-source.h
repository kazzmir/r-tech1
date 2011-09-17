#ifndef paintown_input_source_h
#define paintown_input_source_h

/* this class should abstract over actual devices (keyboard, joystick)
 * and return a list of events on request.
 */

class InputSource{
public:
    InputSource();
    /* keyboard specifies the configuration to use.
     * joystick specifies the configuration and which physical joystick to use.
     * -1 for keyboard/joystick means don't use it
     */
    InputSource(int keyboard, int joystick);
    InputSource(const InputSource & copy);
    virtual ~InputSource();

    InputSource & operator=(const InputSource &);

    virtual bool useKeyboard() const;
    virtual int getKeyboard() const;
    virtual bool useJoystick() const;
    virtual int getJoystick() const;

private:
    int keyboard;
    int joystick;
};

#endif
