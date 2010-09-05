#ifdef USE_ALLEGRO
#include <allegro.h>
#endif
#ifdef USE_SDL
#include <SDL.h>
#endif
#include "funcs.h"
#include "globals.h"
#include <vector>
#include <string>
#include <string.h>
#include <algorithm>
#include "file-system.h"
#include "bitmap.h"

#ifndef USE_ALLEGRO
/* FIXME: move this to the filesystem module */
#include "sfl/sfl.h"
#include "sfl/sflfile.h"
#endif

#ifndef WINDOWS
#include <unistd.h>
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

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

/*
int Util::min(int a, int b){
    if (a<b){
        return a;
    }
    return b;
}
*/

int Util::clamp(int value, int min, int max){
    return Util::min(Util::max(value, min), max);
}

int Util::rnd( int q, int min, int range ){
	return q - min + rnd( range );
}

int Util::rnd( int min, int max ){
	return rnd( max - min ) + min;
}

/* sleep for `x' milliseconds */
void Util::rest( int x ){
#ifdef USE_ALLEGRO
    ::rest(x);
#endif
#ifdef USE_SDL
    SDL_Delay(x);
#endif
}

void Util::restSeconds(double x){
    Util::rest((int)(x * 1000));
}

bool Util::checkVersion(int version){
    if (version == Global::getVersion()){
        return true;
    }

    /* when an incompatible version is made, add a check here, like
     *  version < getVersion(3, 5)
     * would mean any client below version 3.5 is incompatible.
     *
     * assume versions of client's greater than ourself is compatible, but
     * this may not be true. There is no way to check this.
     */
    if (version < 0){
        return false;
    }

    return true;
}

void Util::setDataPath( const string & str ){
    dataPath = str;	
}

Filesystem::AbsolutePath Util::getDataPath2(){
    return Filesystem::AbsolutePath(dataPath + "/");
}

bool Util::exists( const string & file ){
#ifdef USE_ALLEGRO
    return ::exists(file.c_str()) != 0;
#else
    return file_exists(file.c_str());
#endif
}

/*
vector<Filesystem::AbsolutePath> Util::getFiles(const Filesystem::AbsolutePath & dataPath, const string & find){
    return Filesystem::getFiles(dataPath, find);
}
*/

void Util::blend_palette(int * pal, int mp, int startColor, int endColor ) {
    /*
    ASSERT(pal);
    ASSERT(mp != 0);
    */

    int sc_r = Bitmap::getRed(startColor);
    int sc_g = Bitmap::getGreen(startColor);
    int sc_b = Bitmap::getBlue(startColor);

    int ec_r = Bitmap::getRed(endColor);
    int ec_g = Bitmap::getGreen(endColor);
    int ec_b = Bitmap::getBlue(endColor);

    for ( int q = 0; q < mp; q++ ) {
        float j = (float)( q + 1 ) / (float)( mp );
        int f_r = (int)( 0.5 + (float)( sc_r ) + (float)( ec_r-sc_r ) * j );
        int f_g = (int)( 0.5 + (float)( sc_g ) + (float)( ec_g-sc_g ) * j );
        int f_b = (int)( 0.5 + (float)( sc_b ) + (float)( ec_b-sc_b ) * j );
        pal[q] = Bitmap::makeColor( f_r, f_g, f_b );
    }
}

string Util::trim(const std::string & str){
    string s;
    size_t startpos = str.find_first_not_of(" \t");
    size_t endpos = str.find_last_not_of(" \t");
    // if all spaces or empty return an empty string  
    if ((string::npos == startpos) ||
        (string::npos == endpos)){
        return "";
    } else {
        return str.substr(startpos, endpos-startpos+1);
    }
    return str;
}

/* makes the first letter of a string upper case */
string Util::upcase(std::string str){
    if ( str.length() > 0 && (str[0] >= 'a' && str[0] <= 'z') ){
        str[0] = str[0] - 'a' + 'A';
    }
    return str;
}

static int lowerCase(int c){
    return tolower(c);
}

static int upperCase(int c){
    return toupper(c);
}

string Util::upperCaseAll(std::string str){
    std::transform(str.begin(), str.end(), str.begin(), upperCase);
    return str;
}

string Util::lowerCaseAll(std::string str){
    std::transform(str.begin(), str.end(), str.begin(), lowerCase);
    /*
    for (unsigned int i = 0; i < str.length(); i++){
        if (str[0] >= 'A' && str[0] <= 'Z'){
            str[0] = str[0] - 'A' + 'a';
        }
    }
    */
    return str;
}

/*Gets the minimum of three values*/
static int minimum(int a,int b,int c){
  int min=a;
  if(b<min)
    min=b;
  if(c<min)
    min=c;
  return min;
}

/* Compute levenshtein distance between s and t
 * from: http://www.merriampark.com/ldc.htm
 */
static int levenshtein_distance(const char *s, const char *t){
  //Step 1
  int k,i,j,n,m,cost,distance;
  int * d;
  n=strlen(s); 
  m=strlen(t);
  if(n!=0&&m!=0){
    d = (int*) malloc((sizeof(int))*(m+1)*(n+1));
    m++;
    n++;
    //Step 2	
    for(k=0;k<n;k++)
	d[k]=k;
    for(k=0;k<m;k++)
      d[k*n]=k;
    //Step 3 and 4	
    for(i=1;i<n;i++)
      for(j=1;j<m;j++)
	{
        //Step 5
        if(s[i-1]==t[j-1])
          cost=0;
        else
          cost=1;
        //Step 6			 
        d[j*n+i]=minimum(d[(j-1)*n+i]+1,d[j*n+i-1]+1,d[(j-1)*n+i-1]+cost);
      }
    distance=d[n*m-1];
    free(d);
    return distance;
  } else {
    return -1; //a negative return value means that one or both strings are empty.
  }
}

int Util::levenshtein(const std::string & str1, const std::string & str2){
    return levenshtein_distance(str1.c_str(), str2.c_str());
}

void Util::limitPrintf(char * buffer, int size, const char * format, va_list args){
#ifdef USE_ALLEGRO
    uvszprintf(buffer, size, format, args);
#else
    vsnprintf(buffer, size, format, args);
#endif
}

#ifndef WINDOWS
int Util::getPipe(int files[2]){
    return pipe(files);
}
#endif
