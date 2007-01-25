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
		return my_keys[ i ];
	}

	/* keypressed:
	 * Returns true if a key is pressed
	 */
	bool keypressed() const;
	
	/* readKeys:
	 * Store all pressed keys in a user supplied vector
	 */
	void readKeys( vector< int > & all_keys ) const;

	static const int Key_A;
	static const int Key_S;
	static const int Key_D;

	static const int Key_LEFT;
	static const int Key_RIGHT;
	static const int Key_UP;
	static const int Key_DOWN;
	static const int Key_SPACE;

protected:

	map<int,bool> my_keys;

};

#endif
