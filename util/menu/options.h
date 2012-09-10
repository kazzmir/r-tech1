#ifndef _paintown_options_h
#define _paintown_options_h

#include "menu_option.h"
#include "util/gui/animation.h"
#include "util/input/input-map.h"
#include "util/file-system.h"
#include "util/pointer.h"
#include "font-info.h"

class Token;

namespace Menu{
    class OptionFactory;
}

/*! Handles key reconfiguration */
class OptionCredits: public MenuOption {
public:
    enum CreditKey{
        Exit
    };
    
    class Block{
    public:
        Block(const std::string &);
        Block(const Token *);
        Block(const Block &);
        ~Block();
        
        const Block & operator=(const Block &);
        void addCredit(const std::string &);
        
        void act();
        
        enum Justification{
            Left,
            Center,
            Right,
        };
        
        int print(int x, int y, Graphics::Color defaultTitleColor, Graphics::Color defaultColor, const Font &, const Graphics::Bitmap &, const Justification &) const;
        
        const int size(const Font &) const;
        
        inline const bool empty() const {
            return title.empty() && credits.empty();
        }
        
    protected:
        std::string title;
        std::vector<std::string> credits;
        bool titleColorOverride;
        Graphics::Color titleColor;
        bool colorOverride;
        Graphics::Color color;
        // Before title
        Util::ReferenceCount<Gui::Animation> topAnimation;
        int topWidth, topHeight;
        // After last credit
        Util::ReferenceCount<Gui::Animation> bottomAnimation;
        int bottomWidth, bottomHeight;
    };
    
    class Sequence {
    public:
        Sequence(Token *);
        Sequence(const Sequence &);
        ~Sequence();
        
        const Sequence & operator=(const Sequence &);
        
        void act();
        void draw(const Graphics::Bitmap &);
        
        enum Type{
            Primary,
            Roll,
        };
        
    protected:
        Type type;
        int start;
        int end;
        double speed;
        double distance;
        Block::Justification justification;
        std::vector<Block> credits;
    };

    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    OptionCredits(const Gui::ContextBox & parent, const Token *token);

    virtual ~OptionCredits();
//private:
    Util::ReferenceCount<Menu::Context> creditsContext;
    std::vector<Block> creditsPrimary;
    int primaryStart;
    int primaryEnd;
    double primarySpeed;
    double primaryAlphaSpeed;
    std::vector<Block> creditsRoll;
    double rollSpeed;
    double rollOffset;
    Block::Justification rollJustification;
    std::string music;
    Graphics::Color color, title;
    InputMap<CreditKey> input;
    // Clear background (default black)
    Graphics::Color clearColor;
};

/*! Dummy option, to allow place fillers in menus */
class OptionDummy: public MenuOption{
public:
	OptionDummy(const Gui::ContextBox & parent, const Token *token);
	OptionDummy(const Gui::ContextBox & parent, const std::string &name);

	// Do logic before run part
	virtual void logic();

	// Finally it has been selected, this is what shall run 
	// endGame will be set true if it is a terminating option
	virtual void run(const Menu::Context &);

	virtual ~OptionDummy();
};

/*! Handles key reconfiguration */
class OptionFullscreen: public MenuOption{
public:
    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);
    
    virtual std::string getText() const;

    // This is to pass paramaters to an option ie a bar or something
    virtual bool leftKey();
    virtual bool rightKey();

    OptionFullscreen(const Gui::ContextBox & parent, const Token *token);

    virtual ~OptionFullscreen();
private:

    int lblue, lgreen;
    int rblue, rgreen;
};

/*! Handles key reconfiguration */
class OptionInvincible : public MenuOption
{
	public:
		// Do logic before run part
		virtual void logic();
		
		// Finally it has been selected, this is what shall run 
		// endGame will be set true if it is a terminating option
		virtual void run(const Menu::Context &);
		
		// This is to pass paramaters to an option ie a bar or something
		virtual bool leftKey();
		virtual bool rightKey();
    
        virtual std::string getText() const;
		
		OptionInvincible(const Gui::ContextBox & parent, const Token *token);
	
		virtual ~OptionInvincible();
	private:
		int lblue, lgreen;
		int rblue, rgreen;
};

/*! Handles joystick reconfiguration */
class OptionJoystick: public MenuOption {
public:
    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    OptionJoystick(const Gui::ContextBox & parent, const Token *token);

    virtual ~OptionJoystick();

    //! keys
    enum JoystickType{
        Up = 0,
        Down,
        Left,
        Right,
        Jump,
        Attack1,
        Attack2,
        Attack3,
        Attack4,
        Attack5,
        Attack6,
        Invalidkey
    };

private:
    //! name
    std::string name;
    int player;

    JoystickType type;
    int keyCode;
};

/*! Handles key reconfiguration */
class OptionKey: public MenuOption{
    public:
        // Do logic before run part
        virtual void logic();

        // Finally it has been selected, this is what shall run 
        // endGame will be set true if it is a terminating option
        virtual void run(const Menu::Context &);

        OptionKey(const Gui::ContextBox & parent, const Token *token);

        virtual ~OptionKey();

        //! keys
        enum keyType
        {
            up=0,
            down,
            left,
            right,
            jump,
            attack1,
            attack2,
            attack3,
            attack4,
            attack5,
            attack6,
            invalidkey
        };

    private:
        //! name
        std::string name;
        int player;

        keyType type;
        int keyCode;
};

/*! Handles key reconfiguration */
class OptionLevel: public MenuOption {
public:
	OptionLevel(const Gui::ContextBox & parent, const Token *token, int * set, int value);

	// Do logic before run part
	virtual void logic();

	// Finally it has been selected, this is what shall run 
	// endGame will be set true if it is a terminating option
	virtual void run(const Menu::Context &);

	virtual ~OptionLevel();

protected:
    /* integer to set if this option is chosen */
    int * set;
    int value;
};


/*! Handles key reconfiguration */
class OptionLives: public MenuOption{
public:
    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    // This is to pass paramaters to an option ie a bar or something
    virtual bool leftKey();
    virtual bool rightKey();
    
    virtual std::string getText() const;

    OptionLives(const Gui::ContextBox & parent, const Token *token);

    virtual ~OptionLives();
private:
    int lblue, lgreen;
    int rblue, rgreen;
};

namespace Menu {
    class Menu;
    class Context;
}

/*! Handles sub menus */
class OptionMenu: public MenuOption {
    public:
        // Do logic before run part
        virtual void logic();

        // Finally it has been selected, this is what shall run 
        // endGame will be set true if it is a terminating option
        virtual void run(const Menu::Context &);

        OptionMenu(const Gui::ContextBox & parent, const Token *token, const Menu::OptionFactory & factory);

        virtual ~OptionMenu();

    private:
        //Menu *menu;
        Menu::Menu *menu;
};

/* FIXME: should be moved to the paintown directory */
class OptionNpcBuddies: public MenuOption {
public:
	OptionNpcBuddies(const Gui::ContextBox & parent, const Token *token );

	// Do logic before run part
	virtual void logic();
	
	// Finally it has been selected, this is what shall run 
	// endGame will be set true if it is a terminating option
	virtual void run(const Menu::Context &);
    
        virtual std::string getText() const;
	
	// This is to pass paramaters to an option ie a bar or something
	virtual bool leftKey();
	virtual bool rightKey();
	
	virtual ~OptionNpcBuddies();
private:
	int lblue, lgreen;
	int rblue, rgreen;
};

/* FIXME: move to the paintown directory */
class OptionPlayMode: public MenuOption {
public:
    OptionPlayMode(const Gui::ContextBox & parent, const Token *token);

    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);
    
    virtual std::string getText() const;

    // This is to pass paramaters to an option ie a bar or something
    virtual bool leftKey();
    virtual bool rightKey();

    virtual ~OptionPlayMode();

protected:
    virtual void changeMode();

private:

    int lblue, lgreen;
    int rblue, rgreen;
};

class OptionQuit : public MenuOption {
public:
	OptionQuit(const Gui::ContextBox & parent, const Token *token);
	OptionQuit(const Gui::ContextBox & parent, const std::string &name);

	// Do logic before run part
	virtual void logic();

	// Finally it has been selected, this is what shall run 
	// endGame will be set true if it is a terminating option
	virtual void run(const Menu::Context &);

	virtual ~OptionQuit();
};

/* return to previous menu */
class OptionReturn: public MenuOption {
public:
    OptionReturn(const Gui::ContextBox & parent, const Token * token);
    virtual void logic();
    virtual void run(const Menu::Context &);
    virtual ~OptionReturn();
};

/* Sets the quality filter used to stretch the screen (xbr/hqx) */
class OptionQualityFilter: public MenuOption {
public:
    OptionQualityFilter(const Gui::ContextBox & parent, const Token * token);
    virtual void logic();
    virtual void run(const Menu::Context &);
    std::string getText() const;
    virtual bool leftKey();
    virtual bool rightKey();
    virtual ~OptionQualityFilter();

protected:
    std::string filter;
};

/* Change the fps */
class OptionFps: public MenuOption {
public:
    OptionFps(const Gui::ContextBox & parent, const Token * token);
    virtual void logic();
    virtual void run(const Menu::Context &);
    std::string getText() const;
    virtual bool leftKey();
    virtual bool rightKey();
    virtual ~OptionFps();
};

/* continue the game */
class OptionContinue: public MenuOption {
public:
    OptionContinue(const Gui::ContextBox & parent, const Token * token);
    virtual void logic();
    virtual void run(const Menu::Context &);
    virtual ~OptionContinue();
};

struct ScreenSize{
    ScreenSize(int w, int h):w(w), h(h){}
    int w, h;
};

/*! Handles key reconfiguration */
class OptionScreenSize : public MenuOption {
public:
    OptionScreenSize(const Gui::ContextBox & parent, const Token *token);

    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    // This is to pass paramaters to an option ie a bar or something
    virtual bool leftKey();
    virtual bool rightKey();

    virtual ~OptionScreenSize();

protected:

    void setMode(int width, int height);
    int findMode(int width, int height);

private:
    // name
    std::string name;
    std::vector<ScreenSize> modes;

    int lblue, lgreen;
    int rblue, rgreen;
};

/*! Handles font selection */
class OptionSelectFont: public MenuOption {
public:
    OptionSelectFont(const Gui::ContextBox & parent, const Token *token);

    virtual void open();
    virtual void close();

    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    // This is to pass paramaters to an option ie a bar or something
    virtual bool leftKey();
    virtual bool rightKey();

    virtual ~OptionSelectFont();

protected:

private:
    // Type of selector
    enum Adjust{
      fontName=0,
      fontHeight,
      fontWidth
    };
    // Current type of menu to adjust
    Adjust typeAdjust;
    
    int lblue, lgreen;
    int rblue, rgreen;
    
    std::vector<Util::ReferenceCount<Menu::FontInfo> > fonts;
    
    void nextIndex(bool forward);

};

/*! Handles key reconfiguration */
class OptionSpeed: public MenuOption {
	public:
		// Do logic before run part
		virtual void logic();
		
		// Finally it has been selected, this is what shall run 
		// endGame will be set true if it is a terminating option
		virtual void run(const Menu::Context &);
		
		// This is to pass paramaters to an option ie a bar or something
		virtual bool leftKey();
		virtual bool rightKey();

        virtual std::string getText() const;
		
		OptionSpeed(const Gui::ContextBox & parent, const Token *token);
	
		virtual ~OptionSpeed();
	private:
		// name
		std::string name;
		
		int lblue, lgreen;
		int rblue, rgreen;
};

/*! Handles sub menus */
class OptionTabMenu: public MenuOption {
public:
    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    OptionTabMenu(const Gui::ContextBox & parent, const Token *token, const Menu::OptionFactory & factory);

    virtual ~OptionTabMenu();

private:
    Menu::Menu *menu;
};

class OptionSound: public MenuOption {
public:
    OptionSound(const Gui::ContextBox & parent, const Token *token);

    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    // This is to pass parameters to an option ie a bar or something
    virtual bool leftKey();
    virtual bool rightKey();

    virtual ~OptionSound();

protected:
    void changeSound(int much);

private:
    int lblue, lgreen;
    int rblue, rgreen;
    std::string originalName;
};

class OptionMusic: public MenuOption {
public:
    OptionMusic(const Gui::ContextBox & parent, const Token *token);

    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    // This is to pass parameters to an option ie a bar or something
    virtual bool leftKey();
    virtual bool rightKey();

    virtual ~OptionMusic();

protected:
    void changeMusic(int much);

private:
    int lblue, lgreen;
    int rblue, rgreen;
    std::string originalName;
};

class OptionLanguage: public MenuOption {
public:
    OptionLanguage(const Gui::ContextBox & parent, const Token * token);
    virtual void logic();
    virtual void run(const Menu::Context &);
};

/* FIXME: move this option to the paintown engine somehow */
class OptionGibs: public MenuOption {
public:
    OptionGibs(const Gui::ContextBox & parent, const Token *token);

    // Do logic before run part
    virtual void logic();

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &);

    // This is to pass parameters to an option ie a bar or something
    virtual bool leftKey();
    virtual bool rightKey();

    virtual ~OptionGibs();
protected:
    void changeGibs(int much);
private:
    std::string originalName;
};

#endif
