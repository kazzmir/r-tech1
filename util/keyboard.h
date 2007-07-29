#ifndef _keyboard_h
#define _keyboard_h

#include <map>
#include <vector>

using namespace std;

/* handles allegro key[] array better than keypressed()
 * and readkey()
 */
class Keyboard{
public:

	Keyboard();

	/* poll:
	 * Put the keys in Allegro's key[] array into our map of int -> bool
	 */
	void poll();

	/* []:
	 * Extract a boolean value given a key number
	 */
	inline const bool operator[] ( const int i ){

		/* if the key has been pressed for the first time return true */
		if ( my_keys[ i ] < 0 ){
			my_keys[ i ] = 1;
			return true;
		}

		bool b = my_keys[ i ] > key_delay[ i ];
		if ( b ){
			my_keys[ i ] = 1;
		}
		return b;
	}

	/* keypressed:
	 * Returns true if a key is pressed
	 */
	const bool keypressed();
	
	/* readKeys:
	 * Store all pressed keys in a user supplied vector
	 */
	void readKeys( vector< int > & all_keys );

	void setDelay( const int key, const int delay );

	static const int Key_A;
	static const int Key_B;
	static const int Key_C;
	static const int Key_D;
	static const int Key_E;
	static const int Key_F;
	static const int Key_G;
	static const int Key_H;
	static const int Key_I;
	static const int Key_J;
	static const int Key_K;
	static const int Key_L;
	static const int Key_M;
	static const int Key_N;
	static const int Key_O;
	static const int Key_P;
	static const int Key_Q;
	static const int Key_R;
	static const int Key_S;
	static const int Key_T;
	static const int Key_U;
	static const int Key_V;
	static const int Key_W;
	static const int Key_X;
	static const int Key_Y;
	static const int Key_Z;
	static const int Key_0;
	static const int Key_1;
	static const int Key_2;
	static const int Key_3;
	static const int Key_4;
	static const int Key_5;
	static const int Key_6;
	static const int Key_7;
	static const int Key_8;
	static const int Key_9;
	static const int Key_0_PAD;
	static const int Key_1_PAD;
	static const int Key_2_PAD;
	static const int Key_3_PAD;
	static const int Key_4_PAD;
	static const int Key_5_PAD;
	static const int Key_6_PAD;
	static const int Key_7_PAD;
	static const int Key_8_PAD;
	static const int Key_9_PAD;
	static const int Key_F1;
	static const int Key_F2;
	static const int Key_F3;
	static const int Key_F4;
	static const int Key_F5;
	static const int Key_F6;
	static const int Key_F7;
	static const int Key_F8;
	static const int Key_F9;
	static const int Key_F10;
	static const int Key_F11;
	static const int Key_F12;
	static const int Key_ESC;
	static const int Key_TILDE;
	static const int Key_MINUS;
	static const int Key_EQUALS;
	static const int Key_BACKSPACE;
	static const int Key_TAB;
	static const int Key_OPENBRACE;
	static const int Key_CLOSEBRACE;
	static const int Key_ENTER;
	static const int Key_COLON;
	static const int Key_QUOTE;
	static const int Key_BACKSLASH;
	static const int Key_BACKSLASH2;
	static const int Key_COMMA;
	static const int Key_STOP;
	static const int Key_SLASH;
	static const int Key_SPACE;
	static const int Key_INSERT;
	static const int Key_DEL;
	static const int Key_HOME;
	static const int Key_END;
	static const int Key_PGUP;
	static const int Key_PGDN;
	static const int Key_LEFT;
	static const int Key_RIGHT;
	static const int Key_UP;
	static const int Key_DOWN;
	static const int Key_SLASH_PAD;
	static const int Key_ASTERISK;
	static const int Key_MINUS_PAD;
	static const int Key_PLUS_PAD;
	static const int Key_DEL_PAD;
	static const int Key_ENTER_PAD;
	static const int Key_PRTSCR;
	static const int Key_PAUSE;
	static const int Key_ABNT_C1;
	static const int Key_YEN;
	static const int Key_KANA;
	static const int Key_CONVERT;
	static const int Key_NOCONVERT;
	static const int Key_AT;
	static const int Key_CIRCUMFLEX;
	static const int Key_COLON2;
	static const int Key_KANJI;
	static const int Key_EQUALS_PAD;
	static const int Key_BACKQUOTE;
	static const int Key_SEMICOLON;
	static const int Key_COMMAND;
	static const int Key_UNKNOWN1;
	static const int Key_UNKNOWN2;
	static const int Key_UNKNOWN3;
	static const int Key_UNKNOWN4;
	static const int Key_UNKNOWN5;
	static const int Key_UNKNOWN6;
	static const int Key_UNKNOWN7;
	static const int Key_UNKNOWN8;
	static const int Key_MODIFIERS;
	static const int Key_LSHIFT;
	static const int Key_RSHIFT;
	static const int Key_LCONTROL;
	static const int Key_RCONTROL;
	static const int Key_ALT;
	static const int Key_ALTGR;
	static const int Key_LWIN;
	static const int Key_RWIN;
	static const int Key_MENU;
	static const int Key_SCRLOCK;
	static const int Key_NUMLOCK;
	static const int Key_CAPSLOCK;

protected:

	map<int,int> my_keys;
	map<int,int> key_delay;

};

#endif
