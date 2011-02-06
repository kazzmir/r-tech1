#include "../keyboard.h"
#include <allegro5/keycodes.h>

const int Keyboard::Key_A = ALLEGRO_KEY_A;
const int Keyboard::Key_B = ALLEGRO_KEY_B;
const int Keyboard::Key_C = ALLEGRO_KEY_C;
const int Keyboard::Key_D = ALLEGRO_KEY_D;
const int Keyboard::Key_E = ALLEGRO_KEY_E;
const int Keyboard::Key_F = ALLEGRO_KEY_F;
const int Keyboard::Key_G = ALLEGRO_KEY_G;
const int Keyboard::Key_H = ALLEGRO_KEY_H;
const int Keyboard::Key_I = ALLEGRO_KEY_I;
const int Keyboard::Key_J = ALLEGRO_KEY_J;
const int Keyboard::Key_K = ALLEGRO_KEY_K;
const int Keyboard::Key_L = ALLEGRO_KEY_L;
const int Keyboard::Key_M = ALLEGRO_KEY_M;
const int Keyboard::Key_N = ALLEGRO_KEY_N;
const int Keyboard::Key_O = ALLEGRO_KEY_O;
const int Keyboard::Key_P = ALLEGRO_KEY_P;
const int Keyboard::Key_Q = ALLEGRO_KEY_Q;
const int Keyboard::Key_R = ALLEGRO_KEY_R;
const int Keyboard::Key_S = ALLEGRO_KEY_S;
const int Keyboard::Key_T = ALLEGRO_KEY_T;
const int Keyboard::Key_U = ALLEGRO_KEY_U;
const int Keyboard::Key_V = ALLEGRO_KEY_V;
const int Keyboard::Key_W = ALLEGRO_KEY_W;
const int Keyboard::Key_X = ALLEGRO_KEY_X;
const int Keyboard::Key_Y = ALLEGRO_KEY_Y;
const int Keyboard::Key_Z = ALLEGRO_KEY_Z;
const int Keyboard::Key_0 = ALLEGRO_KEY_0;
const int Keyboard::Key_1 = ALLEGRO_KEY_1;
const int Keyboard::Key_2 = ALLEGRO_KEY_2;
const int Keyboard::Key_3 = ALLEGRO_KEY_3;
const int Keyboard::Key_4 = ALLEGRO_KEY_4;
const int Keyboard::Key_5 = ALLEGRO_KEY_5;
const int Keyboard::Key_6 = ALLEGRO_KEY_6;
const int Keyboard::Key_7 = ALLEGRO_KEY_7;
const int Keyboard::Key_8 = ALLEGRO_KEY_8;
const int Keyboard::Key_9 = ALLEGRO_KEY_9;
const int Keyboard::Key_0_PAD = ALLEGRO_KEY_PAD_0;
const int Keyboard::Key_1_PAD = ALLEGRO_KEY_PAD_1;
const int Keyboard::Key_2_PAD = ALLEGRO_KEY_PAD_2;
const int Keyboard::Key_3_PAD = ALLEGRO_KEY_PAD_3;
const int Keyboard::Key_4_PAD = ALLEGRO_KEY_PAD_4;
const int Keyboard::Key_5_PAD = ALLEGRO_KEY_PAD_5;
const int Keyboard::Key_6_PAD = ALLEGRO_KEY_PAD_6;
const int Keyboard::Key_7_PAD = ALLEGRO_KEY_PAD_7;
const int Keyboard::Key_8_PAD = ALLEGRO_KEY_PAD_8;
const int Keyboard::Key_9_PAD = ALLEGRO_KEY_PAD_9;
const int Keyboard::Key_F1 = ALLEGRO_KEY_F1;
const int Keyboard::Key_F2 = ALLEGRO_KEY_F2;
const int Keyboard::Key_F3 = ALLEGRO_KEY_F3;
const int Keyboard::Key_F4 = ALLEGRO_KEY_F4;
const int Keyboard::Key_F5 = ALLEGRO_KEY_F5;
const int Keyboard::Key_F6 = ALLEGRO_KEY_F6;
const int Keyboard::Key_F7 = ALLEGRO_KEY_F7;
const int Keyboard::Key_F8 = ALLEGRO_KEY_F8;
const int Keyboard::Key_F9 = ALLEGRO_KEY_F9;
const int Keyboard::Key_F10 = ALLEGRO_KEY_F10;
const int Keyboard::Key_F11 = ALLEGRO_KEY_F11;
const int Keyboard::Key_F12 = ALLEGRO_KEY_F12;
const int Keyboard::Key_ESC = ALLEGRO_KEY_ESCAPE;
const int Keyboard::Key_TILDE = ALLEGRO_KEY_TILDE;
const int Keyboard::Key_MINUS = ALLEGRO_KEY_MINUS;
const int Keyboard::Key_EQUALS = ALLEGRO_KEY_EQUALS;
const int Keyboard::Key_BACKSPACE = ALLEGRO_KEY_BACKSPACE;
const int Keyboard::Key_TAB = ALLEGRO_KEY_TAB;
const int Keyboard::Key_OPENBRACE = ALLEGRO_KEY_OPENBRACE;
const int Keyboard::Key_CLOSEBRACE = ALLEGRO_KEY_CLOSEBRACE;
const int Keyboard::Key_ENTER = ALLEGRO_KEY_ENTER;
const int Keyboard::Key_COLON = ALLEGRO_KEY_SEMICOLON2;
const int Keyboard::Key_QUOTE = ALLEGRO_KEY_QUOTE;
const int Keyboard::Key_BACKSLASH = ALLEGRO_KEY_BACKSLASH;
const int Keyboard::Key_BACKSLASH2 = ALLEGRO_KEY_BACKSLASH2;
const int Keyboard::Key_COMMA = ALLEGRO_KEY_COMMA;
const int Keyboard::Key_STOP = ALLEGRO_KEY_FULLSTOP;
const int Keyboard::Key_SLASH = ALLEGRO_KEY_SLASH;
const int Keyboard::Key_SPACE = ALLEGRO_KEY_SPACE;
const int Keyboard::Key_INSERT = ALLEGRO_KEY_INSERT;
const int Keyboard::Key_DEL = ALLEGRO_KEY_DELETE;
const int Keyboard::Key_HOME = ALLEGRO_KEY_HOME;
const int Keyboard::Key_END = ALLEGRO_KEY_END;
const int Keyboard::Key_PGUP = ALLEGRO_KEY_PGUP;
const int Keyboard::Key_PGDN = ALLEGRO_KEY_PGDN;
const int Keyboard::Key_LEFT = ALLEGRO_KEY_LEFT;
const int Keyboard::Key_RIGHT = ALLEGRO_KEY_RIGHT;
const int Keyboard::Key_UP = ALLEGRO_KEY_UP;
const int Keyboard::Key_DOWN = ALLEGRO_KEY_DOWN;
const int Keyboard::Key_SLASH_PAD = ALLEGRO_KEY_PAD_SLASH;
const int Keyboard::Key_ASTERISK = ALLEGRO_KEY_PAD_ASTERISK;
const int Keyboard::Key_MINUS_PAD = ALLEGRO_KEY_PAD_MINUS;
const int Keyboard::Key_PLUS_PAD = ALLEGRO_KEY_PAD_PLUS;
const int Keyboard::Key_DEL_PAD = ALLEGRO_KEY_PAD_DELETE;
const int Keyboard::Key_ENTER_PAD = ALLEGRO_KEY_PAD_ENTER;
const int Keyboard::Key_PRTSCR = ALLEGRO_KEY_PRINTSCREEN;
const int Keyboard::Key_PAUSE = ALLEGRO_KEY_PAUSE;
const int Keyboard::Key_ABNT_C1 = ALLEGRO_KEY_ABNT_C1;
const int Keyboard::Key_YEN = ALLEGRO_KEY_YEN;
const int Keyboard::Key_KANA = ALLEGRO_KEY_KANA;
const int Keyboard::Key_CONVERT = ALLEGRO_KEY_CONVERT;
const int Keyboard::Key_NOCONVERT = ALLEGRO_KEY_NOCONVERT;
const int Keyboard::Key_AT = ALLEGRO_KEY_AT;
const int Keyboard::Key_CIRCUMFLEX = ALLEGRO_KEY_CIRCUMFLEX;
const int Keyboard::Key_COLON2 = ALLEGRO_KEY_COLON2;
const int Keyboard::Key_KANJI = ALLEGRO_KEY_KANJI;
const int Keyboard::Key_EQUALS_PAD = ALLEGRO_KEY_PAD_EQUALS;
const int Keyboard::Key_BACKQUOTE = ALLEGRO_KEY_BACKQUOTE;
const int Keyboard::Key_SEMICOLON = ALLEGRO_KEY_SEMICOLON;
const int Keyboard::Key_COMMAND = ALLEGRO_KEY_COMMAND;

/*
const int Keyboard::Key_UNKNOWN1 = ALLEGRO_KEY_UNKNOWN1;
const int Keyboard::Key_UNKNOWN2 = ALLEGRO_KEY_UNKNOWN2;
const int Keyboard::Key_UNKNOWN3 = ALLEGRO_KEY_UNKNOWN3;
const int Keyboard::Key_UNKNOWN4 = ALLEGRO_KEY_UNKNOWN4;
const int Keyboard::Key_UNKNOWN5 = ALLEGRO_KEY_UNKNOWN5;
const int Keyboard::Key_UNKNOWN6 = ALLEGRO_KEY_UNKNOWN6;
const int Keyboard::Key_UNKNOWN7 = ALLEGRO_KEY_UNKNOWN7;
const int Keyboard::Key_UNKNOWN8 = ALLEGRO_KEY_UNKNOWN8;
*/

const int Keyboard::Key_MODIFIERS = ALLEGRO_KEY_MODIFIERS;
const int Keyboard::Key_LSHIFT = ALLEGRO_KEY_LSHIFT;
const int Keyboard::Key_RSHIFT = ALLEGRO_KEY_RSHIFT;
const int Keyboard::Key_LCONTROL = ALLEGRO_KEY_LCTRL;
const int Keyboard::Key_RCONTROL = ALLEGRO_KEY_RCTRL;
const int Keyboard::Key_ALT = ALLEGRO_KEY_ALT;
const int Keyboard::Key_ALTGR = ALLEGRO_KEY_ALTGR;
const int Keyboard::Key_LWIN = ALLEGRO_KEY_LWIN;
const int Keyboard::Key_RWIN = ALLEGRO_KEY_RWIN;
const int Keyboard::Key_MENU = ALLEGRO_KEY_MENU;
const int Keyboard::Key_SCRLOCK = ALLEGRO_KEY_SCROLLLOCK;
const int Keyboard::Key_NUMLOCK = ALLEGRO_KEY_NUMLOCK;
const int Keyboard::Key_CAPSLOCK = ALLEGRO_KEY_CAPSLOCK;
    
Keyboard::Keyboard():
enableBuffer(false){
}

void Keyboard::disableKeyRepeat(){
    /* TODO */
}

void Keyboard::enableKeyRepeat(){
    /* TODO */
}

void Keyboard::readKeys(std::vector<int> & all_keys){
    /* TODO */
}

bool Keyboard::keypressed(){
    /* TODO */
    return false;
}

void Keyboard::clear(){
    /* TODO */
}
