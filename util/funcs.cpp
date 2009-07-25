#include "funcs.h"
#include <allegro.h>
#include <vector>
#include <string>

#ifndef WINDOWS
#include <unistd.h>
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

vector< string > Util::getFiles( const string & dataPath, const string & find ){
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

#ifndef ALLEGRO_WINDOWS
int Util::getPipe(int files[2]){
    return pipe(files);
}
#endif
