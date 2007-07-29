#include "keyboard.h"
#include "allegro.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

#ifndef debug
#define debug cout<<"File: "<<__FILE__<<" Line: "<<__LINE__<<endl;
#endif

const int Keyboard::Key_A = ::KEY_A;
const int Keyboard::Key_B = ::KEY_B;
const int Keyboard::Key_C = ::KEY_C;
const int Keyboard::Key_D = ::KEY_D;
const int Keyboard::Key_E = ::KEY_E;
const int Keyboard::Key_F = ::KEY_F;
const int Keyboard::Key_G = ::KEY_G;
const int Keyboard::Key_H = ::KEY_H;
const int Keyboard::Key_I = ::KEY_I;
const int Keyboard::Key_J = ::KEY_J;
const int Keyboard::Key_K = ::KEY_K;
const int Keyboard::Key_L = ::KEY_L;
const int Keyboard::Key_M = ::KEY_M;
const int Keyboard::Key_N = ::KEY_N;
const int Keyboard::Key_O = ::KEY_O;
const int Keyboard::Key_P = ::KEY_P;
const int Keyboard::Key_Q = ::KEY_Q;
const int Keyboard::Key_R = ::KEY_R;
const int Keyboard::Key_S = ::KEY_S;
const int Keyboard::Key_T = ::KEY_T;
const int Keyboard::Key_U = ::KEY_U;
const int Keyboard::Key_V = ::KEY_V;
const int Keyboard::Key_W = ::KEY_W;
const int Keyboard::Key_X = ::KEY_X;
const int Keyboard::Key_Y = ::KEY_Y;
const int Keyboard::Key_Z = ::KEY_Z;
const int Keyboard::Key_0 = ::KEY_0;
const int Keyboard::Key_1 = ::KEY_1;
const int Keyboard::Key_2 = ::KEY_2;
const int Keyboard::Key_3 = ::KEY_3;
const int Keyboard::Key_4 = ::KEY_4;
const int Keyboard::Key_5 = ::KEY_5;
const int Keyboard::Key_6 = ::KEY_6;
const int Keyboard::Key_7 = ::KEY_7;
const int Keyboard::Key_8 = ::KEY_8;
const int Keyboard::Key_9 = ::KEY_9;
const int Keyboard::Key_0_PAD = ::KEY_0_PAD;
const int Keyboard::Key_1_PAD = ::KEY_1_PAD;
const int Keyboard::Key_2_PAD = ::KEY_2_PAD;
const int Keyboard::Key_3_PAD = ::KEY_3_PAD;
const int Keyboard::Key_4_PAD = ::KEY_4_PAD;
const int Keyboard::Key_5_PAD = ::KEY_5_PAD;
const int Keyboard::Key_6_PAD = ::KEY_6_PAD;
const int Keyboard::Key_7_PAD = ::KEY_7_PAD;
const int Keyboard::Key_8_PAD = ::KEY_8_PAD;
const int Keyboard::Key_9_PAD = ::KEY_9_PAD;
const int Keyboard::Key_F1 = ::KEY_F1;
const int Keyboard::Key_F2 = ::KEY_F2;
const int Keyboard::Key_F3 = ::KEY_F3;
const int Keyboard::Key_F4 = ::KEY_F4;
const int Keyboard::Key_F5 = ::KEY_F5;
const int Keyboard::Key_F6 = ::KEY_F6;
const int Keyboard::Key_F7 = ::KEY_F7;
const int Keyboard::Key_F8 = ::KEY_F8;
const int Keyboard::Key_F9 = ::KEY_F9;
const int Keyboard::Key_F10 = ::KEY_F10;
const int Keyboard::Key_F11 = ::KEY_F11;
const int Keyboard::Key_F12 = ::KEY_F12;
const int Keyboard::Key_ESC = ::KEY_ESC;
const int Keyboard::Key_TILDE = ::KEY_TILDE;
const int Keyboard::Key_MINUS = ::KEY_MINUS;
const int Keyboard::Key_EQUALS = ::KEY_EQUALS;
const int Keyboard::Key_BACKSPACE = ::KEY_BACKSPACE;
const int Keyboard::Key_TAB = ::KEY_TAB;
const int Keyboard::Key_OPENBRACE = ::KEY_OPENBRACE;
const int Keyboard::Key_CLOSEBRACE = ::KEY_CLOSEBRACE;
const int Keyboard::Key_ENTER = ::KEY_ENTER;
const int Keyboard::Key_COLON = ::KEY_COLON;
const int Keyboard::Key_QUOTE = ::KEY_QUOTE;
const int Keyboard::Key_BACKSLASH = ::KEY_BACKSLASH;
const int Keyboard::Key_BACKSLASH2 = ::KEY_BACKSLASH2;
const int Keyboard::Key_COMMA = ::KEY_COMMA;
const int Keyboard::Key_STOP = ::KEY_STOP;
const int Keyboard::Key_SLASH = ::KEY_SLASH;
const int Keyboard::Key_SPACE = ::KEY_SPACE;
const int Keyboard::Key_INSERT = ::KEY_INSERT;
const int Keyboard::Key_DEL = ::KEY_DEL;
const int Keyboard::Key_HOME = ::KEY_HOME;
const int Keyboard::Key_END = ::KEY_END;
const int Keyboard::Key_PGUP = ::KEY_PGUP;
const int Keyboard::Key_PGDN = ::KEY_PGDN;
const int Keyboard::Key_LEFT = ::KEY_LEFT;
const int Keyboard::Key_RIGHT = ::KEY_RIGHT;
const int Keyboard::Key_UP = ::KEY_UP;
const int Keyboard::Key_DOWN = ::KEY_DOWN;
const int Keyboard::Key_SLASH_PAD = ::KEY_SLASH_PAD;
const int Keyboard::Key_ASTERISK = ::KEY_ASTERISK;
const int Keyboard::Key_MINUS_PAD = ::KEY_MINUS_PAD;
const int Keyboard::Key_PLUS_PAD = ::KEY_PLUS_PAD;
const int Keyboard::Key_DEL_PAD = ::KEY_DEL_PAD;
const int Keyboard::Key_ENTER_PAD = ::KEY_ENTER_PAD;
const int Keyboard::Key_PRTSCR = ::KEY_PRTSCR;
const int Keyboard::Key_PAUSE = ::KEY_PAUSE;
const int Keyboard::Key_ABNT_C1 = ::KEY_ABNT_C1;
const int Keyboard::Key_YEN = ::KEY_YEN;
const int Keyboard::Key_KANA = ::KEY_KANA;
const int Keyboard::Key_CONVERT = ::KEY_CONVERT;
const int Keyboard::Key_NOCONVERT = ::KEY_NOCONVERT;
const int Keyboard::Key_AT = ::KEY_AT;
const int Keyboard::Key_CIRCUMFLEX = ::KEY_CIRCUMFLEX;
const int Keyboard::Key_COLON2 = ::KEY_COLON2;
const int Keyboard::Key_KANJI = ::KEY_KANJI;
const int Keyboard::Key_EQUALS_PAD = ::KEY_EQUALS_PAD;
const int Keyboard::Key_BACKQUOTE = ::KEY_BACKQUOTE;
const int Keyboard::Key_SEMICOLON = ::KEY_SEMICOLON;
const int Keyboard::Key_COMMAND = ::KEY_COMMAND;
const int Keyboard::Key_UNKNOWN1 = ::KEY_UNKNOWN1;
const int Keyboard::Key_UNKNOWN2 = ::KEY_UNKNOWN2;
const int Keyboard::Key_UNKNOWN3 = ::KEY_UNKNOWN3;
const int Keyboard::Key_UNKNOWN4 = ::KEY_UNKNOWN4;
const int Keyboard::Key_UNKNOWN5 = ::KEY_UNKNOWN5;
const int Keyboard::Key_UNKNOWN6 = ::KEY_UNKNOWN6;
const int Keyboard::Key_UNKNOWN7 = ::KEY_UNKNOWN7;
const int Keyboard::Key_UNKNOWN8 = ::KEY_UNKNOWN8;
const int Keyboard::Key_MODIFIERS = ::KEY_MODIFIERS;
const int Keyboard::Key_LSHIFT = ::KEY_LSHIFT;
const int Keyboard::Key_RSHIFT = ::KEY_RSHIFT;
const int Keyboard::Key_LCONTROL = ::KEY_LCONTROL;
const int Keyboard::Key_RCONTROL = ::KEY_RCONTROL;
const int Keyboard::Key_ALT = ::KEY_ALT;
const int Keyboard::Key_ALTGR = ::KEY_ALTGR;
const int Keyboard::Key_LWIN = ::KEY_LWIN;
const int Keyboard::Key_RWIN = ::KEY_RWIN;
const int Keyboard::Key_MENU = ::KEY_MENU;
const int Keyboard::Key_SCRLOCK = ::KEY_SCRLOCK;
const int Keyboard::Key_NUMLOCK = ::KEY_NUMLOCK;
const int Keyboard::Key_CAPSLOCK = ::KEY_CAPSLOCK;

Keyboard::Keyboard(){
}

/* KEY_MAX is defined in allegro at
 * allegro/include/allegro/keyboard.h
 */
void Keyboard::poll(){

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

}

void Keyboard::readKeys( vector< int > & all_keys ){
	for ( map<int,int>::const_iterator it = my_keys.begin(); it != my_keys.end(); it++ ){
		const int & key = it->first;
		const int & delay = it->second;
		if ( delay < 0 || delay > key_delay[ key ] ){
			all_keys.push_back( key );
		}
	}
}
	
const int Keyboard::readKey(){
	return ::readkey() >> 8;
}
	
void Keyboard::setDelay( const int key, const int delay ){
	key_delay[ key ] = delay;	
}

const bool Keyboard::keypressed(){
	for ( map<int,int>::const_iterator it = my_keys.begin(); it != my_keys.end(); it++ ){
		const int & n = (*it).second;
		if ( n < 0 || n > key_delay[ it->first ] ){
			return true;
		}
	}
	return false;
}

void Keyboard::clear(){
	::clear_keybuf();
	my_keys.clear();
}

const char * Keyboard::keyToName( int key ){
	switch ( key ){
		case Keyboard::Key_A : return "A";
		case Keyboard::Key_B : return "B";
		case Keyboard::Key_C : return "C";
		case Keyboard::Key_D : return "D";
		case Keyboard::Key_E : return "E";
		case Keyboard::Key_F : return "F";
		case Keyboard::Key_G : return "G";
		case Keyboard::Key_H : return "H";
		case Keyboard::Key_I : return "I";
		case Keyboard::Key_J : return "J";
		case Keyboard::Key_K : return "K";
		case Keyboard::Key_L : return "L";
		case Keyboard::Key_M : return "M";
		case Keyboard::Key_N : return "N";
		case Keyboard::Key_O : return "O";
		case Keyboard::Key_P : return "P";
		case Keyboard::Key_Q : return "Q";
		case Keyboard::Key_R : return "R";
		case Keyboard::Key_S : return "S";
		case Keyboard::Key_T : return "T";
		case Keyboard::Key_U : return "U";
		case Keyboard::Key_V : return "V";
		case Keyboard::Key_W : return "W";
		case Keyboard::Key_X : return "X";
		case Keyboard::Key_Y : return "Y";
		case Keyboard::Key_Z : return "Z";
		case Keyboard::Key_0 : return "0";
		case Keyboard::Key_1 : return "1";
		case Keyboard::Key_2 : return "2";
		case Keyboard::Key_3 : return "3";
		case Keyboard::Key_4 : return "4";
		case Keyboard::Key_5 : return "5";
		case Keyboard::Key_6 : return "6";
		case Keyboard::Key_7 : return "7";
		case Keyboard::Key_8 : return "8";
		case Keyboard::Key_9 : return "9";
		case Keyboard::Key_0_PAD : return "0_PAD";
		case Keyboard::Key_1_PAD : return "1_PAD";
		case Keyboard::Key_2_PAD : return "2_PAD";
		case Keyboard::Key_3_PAD : return "3_PAD";
		case Keyboard::Key_4_PAD : return "4_PAD";
		case Keyboard::Key_5_PAD : return "5_PAD";
		case Keyboard::Key_6_PAD : return "6_PAD";
		case Keyboard::Key_7_PAD : return "7_PAD";
		case Keyboard::Key_8_PAD : return "8_PAD";
		case Keyboard::Key_9_PAD : return "9_PAD";
		case Keyboard::Key_F1 : return "F1";
		case Keyboard::Key_F2 : return "F2";
		case Keyboard::Key_F3 : return "F3";
		case Keyboard::Key_F4 : return "F4";
		case Keyboard::Key_F5 : return "F5";
		case Keyboard::Key_F6 : return "F6";
		case Keyboard::Key_F7 : return "F7";
		case Keyboard::Key_F8 : return "F8";
		case Keyboard::Key_F9 : return "F9";
		case Keyboard::Key_F10 : return "F10";
		case Keyboard::Key_F11 : return "F11";
		case Keyboard::Key_F12 : return "F12";
		case Keyboard::Key_ESC : return "ESC";
		case Keyboard::Key_TILDE : return "TILDE";
		case Keyboard::Key_MINUS : return "MINUS";
		case Keyboard::Key_EQUALS : return "EQUALS";
		case Keyboard::Key_BACKSPACE : return "BACKSPACE";
		case Keyboard::Key_TAB : return "TAB";
		case Keyboard::Key_OPENBRACE : return "OPENBRACE";
		case Keyboard::Key_CLOSEBRACE : return "CLOSEBRACE";
		case Keyboard::Key_ENTER : return "ENTER";
		case Keyboard::Key_COLON : return "COLON";
		case Keyboard::Key_QUOTE : return "QUOTE";
		case Keyboard::Key_BACKSLASH : return "BACKSLASH";
		case Keyboard::Key_BACKSLASH2 : return "BACKSLASH2";
		case Keyboard::Key_COMMA : return "COMMA";
		case Keyboard::Key_STOP : return "STOP";
		case Keyboard::Key_SLASH : return "SLASH";
		case Keyboard::Key_SPACE : return "SPACE";
		case Keyboard::Key_INSERT : return "INSERT";
		case Keyboard::Key_DEL : return "DEL";
		case Keyboard::Key_HOME : return "HOME";
		case Keyboard::Key_END : return "END";
		case Keyboard::Key_PGUP : return "PGUP";
		case Keyboard::Key_PGDN : return "PGDN";
		case Keyboard::Key_LEFT : return "LEFT";
		case Keyboard::Key_RIGHT : return "RIGHT";
		case Keyboard::Key_UP : return "UP";
		case Keyboard::Key_DOWN : return "DOWN";
		case Keyboard::Key_SLASH_PAD : return "SLASH_PAD";
		case Keyboard::Key_ASTERISK : return "ASTERISK";
		case Keyboard::Key_MINUS_PAD : return "MINUS_PAD";
		case Keyboard::Key_PLUS_PAD : return "PLUS_PAD";
		case Keyboard::Key_DEL_PAD : return "DEL_PAD";
		case Keyboard::Key_ENTER_PAD : return "ENTER_PAD";
		case Keyboard::Key_PRTSCR : return "PRTSCR";
		case Keyboard::Key_PAUSE : return "PAUSE";
		case Keyboard::Key_ABNT_C1 : return "ABNT_C1";
		case Keyboard::Key_YEN : return "YEN";
		case Keyboard::Key_KANA : return "KANA";
		case Keyboard::Key_CONVERT : return "CONVERT";
		case Keyboard::Key_NOCONVERT : return "NOCONVERT";
		case Keyboard::Key_AT : return "AT";
		case Keyboard::Key_CIRCUMFLEX : return "CIRCUMFLEX";
		case Keyboard::Key_COLON2 : return "COLON2";
		case Keyboard::Key_KANJI : return "KANJI";
		case Keyboard::Key_EQUALS_PAD : return "EQUALS_PAD";
		case Keyboard::Key_BACKQUOTE : return "BACKQUOTE";
		case Keyboard::Key_SEMICOLON : return "SEMICOLON";
		case Keyboard::Key_COMMAND : return "COMMAND";
		case Keyboard::Key_UNKNOWN1 : return "UNKNOWN1";
		case Keyboard::Key_UNKNOWN2 : return "UNKNOWN2";
		case Keyboard::Key_UNKNOWN3 : return "UNKNOWN3";
		case Keyboard::Key_UNKNOWN4 : return "UNKNOWN4";
		case Keyboard::Key_UNKNOWN5 : return "UNKNOWN5";
		case Keyboard::Key_UNKNOWN6 : return "UNKNOWN6";
		case Keyboard::Key_UNKNOWN7 : return "UNKNOWN7";
		case Keyboard::Key_UNKNOWN8 : return "UNKNOWN8";
		// case Keyboard::Key_MODIFIERS : return "MODIFIERS";
		case Keyboard::Key_LSHIFT : return "LSHIFT";
		case Keyboard::Key_RSHIFT : return "RSHIFT";
		case Keyboard::Key_LCONTROL : return "LCONTROL";
		case Keyboard::Key_RCONTROL : return "RCONTROL";
		case Keyboard::Key_ALT : return "ALT";
		case Keyboard::Key_ALTGR : return "ALTGR";
		case Keyboard::Key_LWIN : return "LWIN";
		case Keyboard::Key_RWIN : return "RWIN";
		case Keyboard::Key_MENU : return "MENU";
		case Keyboard::Key_SCRLOCK : return "SCRLOCK";
		case Keyboard::Key_NUMLOCK : return "NUMLOCK";
		case Keyboard::Key_CAPSLOCK : return "CAPSLOCK";
		default : return "Unknown key";
	}
}
