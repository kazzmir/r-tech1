#include "funcs.h"
#include <allegro.h>
#include <vector>
#include <string>

using namespace std;

/* remove this once cmake and scons properly set DATA_PATH */
#ifndef DATA_PATH
#define DATA_PATH "data"
#endif

/* the build system should define DATA_PATH */
static string dataPath = DATA_PATH;
    
const double Util::pi = 3.14159265;

double Util::radians(double degree){
    return degree * pi / 180.0;
}

/*
inline int rnd( int q ){
	if ( q <= 0 ) return 0;
	return (int)( rand() % q );
}
*/

int Util::max(int a, int b){
    if (a>b){
        return a;
    }
    return b;
}

int Util::min(int a, int b){
    if (a<b){
        return a;
    }
    return b;
}

int Util::rnd( int q, int min, int range ){
	return q - min + rnd( range );
}

int Util::rnd( int min, int max ){
	return rnd( max - min ) + min;
}

void Util::rest( int x ){
	::rest( x );
}

void Util::setDataPath( const string & str ){
	dataPath = str;	
}

string Util::getDataPath(){
	return dataPath + "/";
}

bool Util::exists( const string & file ){
	return ::exists( file.c_str() ) != 0;
}

vector< string > Util::getFiles( string dataPath, string find ){
	struct al_ffblk info;
	vector< string > files;

	if ( al_findfirst( (dataPath + find).c_str(), &info, FA_ALL ) != 0 ){
		return files;
	}
	files.push_back( dataPath + string( info.name ) );
	while ( al_findnext( &info ) == 0 ){
		files.push_back( dataPath + string( info.name ) );
	}
	al_findclose( &info );

	return files;
}

void Util::blend_palette( int * pal, int mp, int sc, int ec ) {

	ASSERT( pal );
	ASSERT( mp != 0 );

	int sc_r = getr( sc );
	int sc_g = getg( sc );
	int sc_b = getb( sc );

	int ec_r = getr( ec );
	int ec_g = getg( ec );
	int ec_b = getb( ec );

	for ( int q = 0; q < mp; q++ ) {
		float j = (float)( q ) / (float)( mp );
		int f_r = (int)( 0.5 + (float)( sc_r ) + (float)( ec_r-sc_r ) * j );
		int f_g = (int)( 0.5 + (float)( sc_g ) + (float)( ec_g-sc_g ) * j );
		int f_b = (int)( 0.5 + (float)( sc_b ) + (float)( ec_b-sc_b ) * j );
		pal[q] = makecol( f_r, f_g, f_b );
	}

}

string Util::trim(const std::string & str){
    string s;
    size_t startpos = str.find_first_not_of(" \t");
    size_t endpos = str.find_last_not_of(" \t");
    // if all spaces or empty return an empty string  
    if ((string::npos == startpos ) ||
        (string::npos == endpos)){
        return "";
    } else {
        return str.substr(startpos, endpos-startpos+1);
    }
}

/* makes the first letter of a string upper case */
string Util::upcase(std::string str){
    if ( str.length() > 0 && (str[0] >= 'a' && str[0] <= 'z') ){
        str[0] = str[0] - 'a' + 'A';
    }
    return str;
}
