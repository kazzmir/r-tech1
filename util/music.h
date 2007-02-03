#ifndef _music_class_h
#define _music_class_h

#include <string>
#include <vector>

/*
#ifdef WINDOWS
#include <allegro.h>
#include <winalleg.h>
#endif

#include <pthread.h>
*/

using namespace std;

struct AL_DUH_PLAYER;
struct DUH;

/* The music class. Dont be late or youll get an F!
 */
class Music{
public:
	Music();

	/*
	Music( const char * song );
	Music( const string & song );
	*/

	~Music();

	static bool loadSong( const char * song );
	static bool loadSong( const string & song );

	/* load one of the songs in 'songs' */
	static void loadSong( const vector< string > & songs );

	void doPlay();

	/*
	void _loadSong( const char * song );
	void _loadSong( const string & song );
	*/

	void _play();
	void _pause();
	void _soften();
	void _louden();

	static void pause();
	static void play();
	static void soften();
	static void louden();

	/*
	void _pause();
	void _resume();
	*/

	/*
	void _play();
	int _soften();
	int _louden();
	void _maxVolume();
	*/

	static void setVolume( double v );
	static double getVolume();

		/*
		return (int)( volume * 100 );
	}
	*/

	static void mute();
	static void unmute();

protected:

	void _setVolume( double vol );

	bool playing;

	bool internal_loadSong( const char * path );

	AL_DUH_PLAYER * player;
	DUH * music_file;
};

#endif
