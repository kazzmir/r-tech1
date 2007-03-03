#ifndef _funcs_h
#define _funcs_h

#include <stdlib.h>
#include <vector>
#include <string>

namespace Util{

// int rnd( int q );
inline int rnd( int q ){
	if ( q <= 0 ) return 0;
	return (int)( rand() % q );
}

std::vector< std::string > getFiles( std::string dataPath, std::string find );

/* return a random number + some range between min/max */
int rnd( int q, int min, int max );

/* return a number between min/max */
int rnd( int min, int max );

void blend_palette( int * pal, int mp, int sc, int ec );

}

#endif
