#include <allegro.h>
#ifdef WINDOWS
#include <winalleg.h>
#endif
#include "../keyboard.h"
#include "util/thread.h"
#include "util/funcs.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

const Keyboard::KeyType Keyboard::Key_A = ::KEY_A;
const Keyboard::KeyType Keyboard::Key_B = ::KEY_B;
const Keyboard::KeyType Keyboard::Key_C = ::KEY_C;
const Keyboard::KeyType Keyboard::Key_D = ::KEY_D;
const Keyboard::KeyType Keyboard::Key_E = ::KEY_E;
const Keyboard::KeyType Keyboard::Key_F = ::KEY_F;
const Keyboard::KeyType Keyboard::Key_G = ::KEY_G;
const Keyboard::KeyType Keyboard::Key_H = ::KEY_H;
const Keyboard::KeyType Keyboard::Key_I = ::KEY_I;
const Keyboard::KeyType Keyboard::Key_J = ::KEY_J;
const Keyboard::KeyType Keyboard::Key_K = ::KEY_K;
const Keyboard::KeyType Keyboard::Key_L = ::KEY_L;
const Keyboard::KeyType Keyboard::Key_M = ::KEY_M;
const Keyboard::KeyType Keyboard::Key_N = ::KEY_N;
const Keyboard::KeyType Keyboard::Key_O = ::KEY_O;
const Keyboard::KeyType Keyboard::Key_P = ::KEY_P;
const Keyboard::KeyType Keyboard::Key_Q = ::KEY_Q;
const Keyboard::KeyType Keyboard::Key_R = ::KEY_R;
const Keyboard::KeyType Keyboard::Key_S = ::KEY_S;
const Keyboard::KeyType Keyboard::Key_T = ::KEY_T;
const Keyboard::KeyType Keyboard::Key_U = ::KEY_U;
const Keyboard::KeyType Keyboard::Key_V = ::KEY_V;
const Keyboard::KeyType Keyboard::Key_W = ::KEY_W;
const Keyboard::KeyType Keyboard::Key_X = ::KEY_X;
const Keyboard::KeyType Keyboard::Key_Y = ::KEY_Y;
const Keyboard::KeyType Keyboard::Key_Z = ::KEY_Z;
const Keyboard::KeyType Keyboard::Key_0 = ::KEY_0;
const Keyboard::KeyType Keyboard::Key_1 = ::KEY_1;
const Keyboard::KeyType Keyboard::Key_2 = ::KEY_2;
const Keyboard::KeyType Keyboard::Key_3 = ::KEY_3;
const Keyboard::KeyType Keyboard::Key_4 = ::KEY_4;
const Keyboard::KeyType Keyboard::Key_5 = ::KEY_5;
const Keyboard::KeyType Keyboard::Key_6 = ::KEY_6;
const Keyboard::KeyType Keyboard::Key_7 = ::KEY_7;
const Keyboard::KeyType Keyboard::Key_8 = ::KEY_8;
const Keyboard::KeyType Keyboard::Key_9 = ::KEY_9;
const Keyboard::KeyType Keyboard::Key_0_PAD = ::KEY_0_PAD;
const Keyboard::KeyType Keyboard::Key_1_PAD = ::KEY_1_PAD;
const Keyboard::KeyType Keyboard::Key_2_PAD = ::KEY_2_PAD;
const Keyboard::KeyType Keyboard::Key_3_PAD = ::KEY_3_PAD;
const Keyboard::KeyType Keyboard::Key_4_PAD = ::KEY_4_PAD;
const Keyboard::KeyType Keyboard::Key_5_PAD = ::KEY_5_PAD;
const Keyboard::KeyType Keyboard::Key_6_PAD = ::KEY_6_PAD;
const Keyboard::KeyType Keyboard::Key_7_PAD = ::KEY_7_PAD;
const Keyboard::KeyType Keyboard::Key_8_PAD = ::KEY_8_PAD;
const Keyboard::KeyType Keyboard::Key_9_PAD = ::KEY_9_PAD;
const Keyboard::KeyType Keyboard::Key_F1 = ::KEY_F1;
const Keyboard::KeyType Keyboard::Key_F2 = ::KEY_F2;
const Keyboard::KeyType Keyboard::Key_F3 = ::KEY_F3;
const Keyboard::KeyType Keyboard::Key_F4 = ::KEY_F4;
const Keyboard::KeyType Keyboard::Key_F5 = ::KEY_F5;
const Keyboard::KeyType Keyboard::Key_F6 = ::KEY_F6;
const Keyboard::KeyType Keyboard::Key_F7 = ::KEY_F7;
const Keyboard::KeyType Keyboard::Key_F8 = ::KEY_F8;
const Keyboard::KeyType Keyboard::Key_F9 = ::KEY_F9;
const Keyboard::KeyType Keyboard::Key_F10 = ::KEY_F10;
const Keyboard::KeyType Keyboard::Key_F11 = ::KEY_F11;
const Keyboard::KeyType Keyboard::Key_F12 = ::KEY_F12;
const Keyboard::KeyType Keyboard::Key_ESC = ::KEY_ESC;
const Keyboard::KeyType Keyboard::Key_TILDE = ::KEY_TILDE;
const Keyboard::KeyType Keyboard::Key_MINUS = ::KEY_MINUS;
const Keyboard::KeyType Keyboard::Key_EQUALS = ::KEY_EQUALS;
const Keyboard::KeyType Keyboard::Key_BACKSPACE = ::KEY_BACKSPACE;
const Keyboard::KeyType Keyboard::Key_TAB = ::KEY_TAB;
const Keyboard::KeyType Keyboard::Key_OPENBRACE = ::KEY_OPENBRACE;
const Keyboard::KeyType Keyboard::Key_CLOSEBRACE = ::KEY_CLOSEBRACE;
const Keyboard::KeyType Keyboard::Key_ENTER = ::KEY_ENTER;
const Keyboard::KeyType Keyboard::Key_COLON = ::KEY_COLON;
const Keyboard::KeyType Keyboard::Key_QUOTE = ::KEY_QUOTE;
const Keyboard::KeyType Keyboard::Key_BACKSLASH = ::KEY_BACKSLASH;
const Keyboard::KeyType Keyboard::Key_BACKSLASH2 = ::KEY_BACKSLASH2;
const Keyboard::KeyType Keyboard::Key_COMMA = ::KEY_COMMA;
const Keyboard::KeyType Keyboard::Key_STOP = ::KEY_STOP;
const Keyboard::KeyType Keyboard::Key_SLASH = ::KEY_SLASH;
const Keyboard::KeyType Keyboard::Key_SPACE = ::KEY_SPACE;
const Keyboard::KeyType Keyboard::Key_INSERT = ::KEY_INSERT;
const Keyboard::KeyType Keyboard::Key_DEL = ::KEY_DEL;
const Keyboard::KeyType Keyboard::Key_HOME = ::KEY_HOME;
const Keyboard::KeyType Keyboard::Key_END = ::KEY_END;
const Keyboard::KeyType Keyboard::Key_PGUP = ::KEY_PGUP;
const Keyboard::KeyType Keyboard::Key_PGDN = ::KEY_PGDN;
const Keyboard::KeyType Keyboard::Key_LEFT = ::KEY_LEFT;
const Keyboard::KeyType Keyboard::Key_RIGHT = ::KEY_RIGHT;
const Keyboard::KeyType Keyboard::Key_UP = ::KEY_UP;
const Keyboard::KeyType Keyboard::Key_DOWN = ::KEY_DOWN;
const Keyboard::KeyType Keyboard::Key_SLASH_PAD = ::KEY_SLASH_PAD;
const Keyboard::KeyType Keyboard::Key_ASTERISK = ::KEY_ASTERISK;
const Keyboard::KeyType Keyboard::Key_MINUS_PAD = ::KEY_MINUS_PAD;
const Keyboard::KeyType Keyboard::Key_PLUS_PAD = ::KEY_PLUS_PAD;
const Keyboard::KeyType Keyboard::Key_DEL_PAD = ::KEY_DEL_PAD;
const Keyboard::KeyType Keyboard::Key_ENTER_PAD = ::KEY_ENTER_PAD;
const Keyboard::KeyType Keyboard::Key_PRTSCR = ::KEY_PRTSCR;
const Keyboard::KeyType Keyboard::Key_PAUSE = ::KEY_PAUSE;
const Keyboard::KeyType Keyboard::Key_ABNT_C1 = ::KEY_ABNT_C1;
const Keyboard::KeyType Keyboard::Key_YEN = ::KEY_YEN;
const Keyboard::KeyType Keyboard::Key_KANA = ::KEY_KANA;
const Keyboard::KeyType Keyboard::Key_CONVERT = ::KEY_CONVERT;
const Keyboard::KeyType Keyboard::Key_NOCONVERT = ::KEY_NOCONVERT;
const Keyboard::KeyType Keyboard::Key_AT = ::KEY_AT;
const Keyboard::KeyType Keyboard::Key_CIRCUMFLEX = ::KEY_CIRCUMFLEX;
const Keyboard::KeyType Keyboard::Key_COLON2 = ::KEY_COLON2;
const Keyboard::KeyType Keyboard::Key_KANJI = ::KEY_KANJI;
const Keyboard::KeyType Keyboard::Key_EQUALS_PAD = ::KEY_EQUALS_PAD;
const Keyboard::KeyType Keyboard::Key_BACKQUOTE = ::KEY_BACKQUOTE;
const Keyboard::KeyType Keyboard::Key_SEMICOLON = ::KEY_SEMICOLON;
const Keyboard::KeyType Keyboard::Key_COMMAND = ::KEY_COMMAND;

/*
const int Keyboard::Key_UNKNOWN1 = ::KEY_UNKNOWN1;
const int Keyboard::Key_UNKNOWN2 = ::KEY_UNKNOWN2;
const int Keyboard::Key_UNKNOWN3 = ::KEY_UNKNOWN3;
const int Keyboard::Key_UNKNOWN4 = ::KEY_UNKNOWN4;
const int Keyboard::Key_UNKNOWN5 = ::KEY_UNKNOWN5;
const int Keyboard::Key_UNKNOWN6 = ::KEY_UNKNOWN6;
const int Keyboard::Key_UNKNOWN7 = ::KEY_UNKNOWN7;
const int Keyboard::Key_UNKNOWN8 = ::KEY_UNKNOWN8;
*/

const Keyboard::KeyType Keyboard::Key_MODIFIERS = ::KEY_MODIFIERS;
const Keyboard::KeyType Keyboard::Key_LSHIFT = ::KEY_LSHIFT;
const Keyboard::KeyType Keyboard::Key_RSHIFT = ::KEY_RSHIFT;
const Keyboard::KeyType Keyboard::Key_LCONTROL = ::KEY_LCONTROL;
const Keyboard::KeyType Keyboard::Key_RCONTROL = ::KEY_RCONTROL;
const Keyboard::KeyType Keyboard::Key_ALT = ::KEY_ALT;
const Keyboard::KeyType Keyboard::Key_ALTGR = ::KEY_ALTGR;
const Keyboard::KeyType Keyboard::Key_LWIN = ::KEY_LWIN;
const Keyboard::KeyType Keyboard::Key_RWIN = ::KEY_RWIN;
const Keyboard::KeyType Keyboard::Key_MENU = ::KEY_MENU;
const Keyboard::KeyType Keyboard::Key_SCRLOCK = ::KEY_SCRLOCK;
const Keyboard::KeyType Keyboard::Key_NUMLOCK = ::KEY_NUMLOCK;
const Keyboard::KeyType Keyboard::Key_CAPSLOCK = ::KEY_CAPSLOCK;

static Util::Thread::Lock keyboardData;

static vector<Keyboard::KeyData> stolenKeys;
static int steal_keys(int unicode, int * scancode){
    // Global::debug(0) << "Steal key " << unicode << " " << *scancode << endl;
    Util::Thread::acquireLock(&keyboardData);
    stolenKeys.push_back(Keyboard::KeyData(*scancode, unicode, true));
    Util::Thread::releaseLock(&keyboardData);
    return unicode;
}

static map<int, bool> stolenPresses;
static void steal_scancode(int scancode){
    Util::Thread::acquireLock(&keyboardData);
    if (scancode & 0x80){
        stolenPresses[scancode & 0x7f] = false;
    } else {
        stolenPresses[scancode & 0x7f] = true;
    }
    Util::Thread::releaseLock(&keyboardData);
}

Keyboard::Keyboard():
enableBuffer(false){
    /*
	for ( int q = 0; q < KEY_MAX; q++ ){
		my_keys[ q ] = 0;
	}
        */
	setAllDelay(0);

        Util::Thread::initializeLock(&keyboardData);
        ::keyboard_ucallback = steal_keys;
        ::keyboard_lowlevel_callback = steal_scancode;
}

/* KEY_MAX is defined in allegro at
 * allegro/include/allegro/keyboard.h
 */
void Keyboard::poll(){
    buffer.clear();

    // keyState.clear();
    Util::Thread::acquireLock(&keyboardData);
    for (vector<KeyData>::iterator it = stolenKeys.begin(); it != stolenKeys.end(); it++){
        const KeyData & data = *it;
        keyState[data.key] = data;
    }
    stolenKeys.clear();

    for (map<int, bool>::iterator it = stolenPresses.begin(); it != stolenPresses.end(); it++){
        int key = it->first;
        bool pressed = it->second;
        keyState[key].enabled = pressed;

        if (pressed){
            press(key, keyState[key].unicode);
        } else {
            release(key);
        }
    }
    stolenPresses.clear();
    Util::Thread::releaseLock(&keyboardData);

    /*
    ::poll_keyboard();
    keyState.clear();
    while (::keypressed()){
        int unicode;
        int key = ::ureadkey(&unicode);
        keyState[key] = KeyData(key, unicode, true);
    }
    */

#if 0
	for ( int q = 0; q < KEY_MAX; q++ ){
		// my_keys[ q ] = key[ q ];
		if ( key[ q ] ){
			/* my_keys[ q ] becomes negative so that the key
			 * can be pressed the first time without having
			 * to wait for a delay
			 */
			if ( my_keys[ q ] <= 0 ){
				my_keys[ q ] -= 1;
			} else {
				my_keys[ q ] += 1;
			}
			// printf( "Key %d = %d\n", q, my_keys[ q ] );
		} else {
			my_keys[ q ] = 0;
		}
	}
#endif
}

#if 0
std::vector<Keyboard::KeyData> Keyboard::readData(){
    std::vector<Keyboard::KeyData> out;
    /* TODO */
    return out;
}
    
std::vector<Keyboard::unicode_t> Keyboard::readText(){
    std::vector<Keyboard::unicode_t> out;
    /* TODO */
    return out;
}
#endif

void Keyboard::enableKeyRepeat(){
    /* these numbers come from the allergo source, src/keyboard.c */
    set_keyboard_rate(250, 33);
}

void Keyboard::disableKeyRepeat(){
    set_keyboard_rate(0, 0);
}

void Keyboard::readKeys( vector< int > & all_keys ){

    for (std::map<KeyType, KeyData>::iterator it = keyState.begin(); it != keyState.end(); it++){
        KeyType key = it->first;
        const KeyData & data = it->second;
        if (data.enabled){
            all_keys.push_back(key);
        }
    }

/*
	for ( map<int,int>::const_iterator it = my_keys.begin(); it != my_keys.end(); it++ ){
		const int & key = it->first;
		const int & delay = it->second;
		if ( delay != 0 ){
			Global::debug( 6 ) << "Key " << key << " delay " << delay << " key_delay " << key_delay[ key ] << endl;
		}
		if ( delay < 0 ){
			all_keys.push_back( key );
			my_keys[ key ] = 1;
		} else if ( delay > key_delay[ key ] ){
			all_keys.push_back( key );
			my_keys[ key ] = 1;
		}
	}
        */
}
	
/*
int Keyboard::readKey(){
    return ::readkey() >> 8;
}
*/

void Keyboard::wait(){
	clear();
	poll();
	while ( keypressed() ){
		poll();
		Util::rest( 1 );
	}
}

bool Keyboard::keypressed(){
    return ::keypressed();
    /*
	for ( map<int,int>::const_iterator it = my_keys.begin(); it != my_keys.end(); it++ ){
		const int & n = (*it).second;
		if ( n < 0 || n > key_delay[ it->first ] ){
			return true;
		}
	}
	return false;
        */
}

void Keyboard::clear(){
    ::clear_keybuf();
    buffer.clear();
    keyState.clear();
    // my_keys.clear();
}
