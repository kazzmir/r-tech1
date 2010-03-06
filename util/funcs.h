#ifndef _paintown_funcs_h
#define _paintown_funcs_h

#include <stdlib.h>
#include <vector>
#include <string>

#include "regex.h"
#include "file-system.h"

namespace Util{

    extern const double pi;


/* returns a number between 0 and q-1. you will never get `q'. if
 * you wanted to get `q' then pass in q+1
 */
inline int rnd( int q ){
    if (q <= 0){
        return 0;
    }

    return (int)(rand() % q);
}

std::vector< std::string > getFiles(const Filesystem::AbsolutePath & dataPath, const std::string & find );

double radians(double degree);

Filesystem::AbsolutePath getDataPath2();

void setDataPath( const std::string & str );

bool exists( const std::string & file );

/* check that `version' is compatible with this program, mostly used
 * for network clients.
 */
bool checkVersion(int version);

/* return a random number + some range between min/max */
int rnd( int q, int min, int max );

int max(int a, int b);
int min(int a, int b);

/* return a number between min/max */
int rnd( int min, int max );

void blend_palette( int * pal, int mp, int sc, int ec );

void rest( int x );

std::string trim(const std::string & str);
std::string upcase(std::string str);

int levenshtein(const std::string & str1, const std::string & str2);

int getPipe(int files[2]);

}

#endif
