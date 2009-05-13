/*
--------------
About
--------------
A Freetype wrapper for use with allegro
Feel free to do whatever you like with this, by all means enjoy!
Just add it to your project

--------------
Linking
--------------
on linux:
g++ `freetype-config --cflags` ftalleg.cpp myfiles.cpp `freetype-config --libs` `allegro-config --libs`

on windows (you may need to include the freetype dir location, ie -Ic:/mingw/include):
g++ ftalleg.cpp myfiles.cpp -lfreetype -lalleg

--------------
Disclaimer
--------------
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE 
SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FT_FONT_H
#define FT_FONT_H

#ifdef _WIN32
#define BITMAP dummyBITMAP
#include <windows.h>
#undef BITMAP
#endif

// #include <allegro.h>
#include <map>
#include <string>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
	
#define GLYPH_PI	3.14159265358979323846
#define GLYPH_SQRT2	1.41421356237309504880

struct BITMAP;
namespace ftalleg
{
	//!  Internal class for freetype to use
	/*!  
	 * This holds necessary information regarding a character \n
	 * from Freetype.
	 */
	class character
	{
		public:
			//! Constructor
			character();
			
			//! Destructor
			~character();
			
			//! Unicode representation of character
			signed long unicode;
			
			//! Width of character
			int width;
			
			//! Height of character
			int height;
			
			//! Space on the left of a character (assists on positioning the character)
			int left;
			
			//! Space on top of the character (assists on positioning the character)
			int top;
			
			//! Space on the right of a character (assists on positioning the character)
			int right;
			
			//! Pitch of a character (assists on positioning the character)
			int pitch;
			
			//! Amount of shades of grays the FT_Bitmap holds
			int grays;
			
			//! Entire rows of the FT_Bitmap
			int rows;
			
			//! Entire length of the character with spacing and all
			int length;
			
			//! FT_Bitmap raw data
			unsigned char *line;
	};
	
	//!  Internal class for freetype to use
	/*!  
	 * Properties used for indexing a given font according to its size and matrix \n
	 */
	class fontSize
	{
		public:
			fontSize();
			~fontSize();
			FT_UInt width;
			FT_UInt height;
			int italics;
			int angle;
			
			bool operator<(const fontSize &fs) const;
			
			const int createKey() const;
	};
	
	//!  Freetype based font system
	/*!  
	 * Allows us to use freetype in allegro
	 */
	class freetype
	{
		private:
			//! Filename
			std::string currentFilename;
			
			//! Is the face loaded
			bool faceLoaded;
			
			//! Does the face have kerning
			bool kerning;
			
			//! Current index default 0
			int currentIndex;
			
			//! for internal use disregard
			bool internalFix;

			int maximumHeight;
			
			//! Font size
			fontSize size;
			
			//! Current Set system name
			std::string systemName;
			
			//! Face
			FT_Face face;
			
			//! Face Name
			std::string faceName;
			
			//! Current character
			character *currentChar;
			
			//! Lookup Table by size
			std::map<int, std::map<signed long, character> >fontTable;
			
			//! Extract glyph
			character extractGlyph(signed long unicode);

			int calculateMaximumHeight();
			
			//! Create single index
			void createIndex();
			
			//! Render a character from the lookup table (utilizing the workBitmap)
			void drawCharacter(signed long unicode, int &x1, int &y1, BITMAP *bitmap, const int & color);


			const int height( long code ) const;
			const int calculateHeight( const std::string & str ) const;
			
		public:
			//! Constructor
			freetype( const std::string & str, const int x, const int y );
			
			//! Destructor
			~freetype();
			
			//! Enum list of alignments
			enum ftAlign
			{
				ftLeft = 0,
    				ftCenter = 1,
				ftRight = 2
			};
			
			//! Load font from memory
			bool load(const unsigned char *memoryFont, unsigned int length, int index, unsigned int width, unsigned int height);
			
			//! Load font from file
			bool load(const std::string & filename, int index, unsigned int width, unsigned int height);
			
			//! Get text length
			int getLength(const std::string & text);
			
			//! Render font to a bitmap
			void render(int x, int y, const int & color, BITMAP *bmp, ftAlign alignment, const std::string & text, int marker, ...);
			
			//! Set size
			void setSize( unsigned int w, unsigned int h);
			
			//! Set italics
			void setItalics(int i);

                        /* get the size attributes (close to width/height) */
                        void getSize(int * w, int * h) const;
			
			//! Get Width
			const int getWidth() const;
			
			//! Get Height
			const int getHeight( const std::string & str ) const;
			
			//! Get Italics
			int getItalics();
	};
}

#endif /* FT_FONT_H */
