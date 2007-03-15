#include "keyboard.h"
#include "allegro.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

#ifndef debug
#define debug cout<<"File: "<<__FILE__<<" Line: "<<__LINE__<<endl;
#endif

const int Keyboard::Key_A = KEY_A;
const int Keyboard::Key_S = KEY_S;
const int Keyboard::Key_D = KEY_D;

const int Keyboard::Key_LEFT = KEY_LEFT;
const int Keyboard::Key_RIGHT = KEY_RIGHT;
const int Keyboard::Key_UP = KEY_UP;
const int Keyboard::Key_DOWN = KEY_DOWN;
const int Keyboard::Key_ENTER = KEY_ENTER;
const int Keyboard::Key_SPACE = KEY_SPACE;
const int Keyboard::Key_F12 = KEY_F12;
const int Keyboard::Key_ESC = KEY_ESC;
	
const int Keyboard::Key_MINUS_PAD = KEY_MINUS_PAD;
const int Keyboard::Key_PLUS_PAD = KEY_PLUS_PAD;

Keyboard::Keyboard(){
}

/* KEY_MAX is defined in allegro at
 * allegro/include/allegro/keyboard.h
 */
void Keyboard::poll(){

	for ( int q = 0; q < KEY_MAX; q++ ){
		// my_keys[ q ] = key[ q ];
		if ( key[ q ] ){
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
