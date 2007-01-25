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
const int Keyboard::Key_SPACE = KEY_SPACE;

Keyboard::Keyboard(){
}

/* KEY_MAX is defined in allegro at
 * allegro/include/allegro/keyboard.h
 */
void Keyboard::poll(){

	for ( int q = 0; q < KEY_MAX; q++ ){
		my_keys[ q ] = key[ q ];
	}

}

void Keyboard::readKeys( vector< int > & all_keys ) const{
	for ( map<int,bool>::const_iterator it = my_keys.begin(); it != my_keys.end(); it++ ){
		if ( (*it).second ){
			all_keys.push_back( (*it).first );
		}
	}
}

bool Keyboard::keypressed() const{
	for ( map<int,bool>::const_iterator it = my_keys.begin(); it != my_keys.end(); it++ ){
		const bool & n = (*it).second;
		if ( n ) return true;
	}
	return false;
}
