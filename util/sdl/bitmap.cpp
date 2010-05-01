#include "../bitmap.h"
#include "../lit_bitmap.h"
#include <SDL.h>

static const int FULLSCREEN = 0;
/* bits per pixel */
static int SCREEN_DEPTH = 16;
static SDL_Surface * screen;

/* TODO: fix MaskColor */
const int Bitmap::MaskColor = 0;

Bitmap * Bitmap::Screen = NULL;
static Bitmap * Scaler = NULL;
static Bitmap * Buffer = NULL;

Bitmap::Bitmap(){
    /* TODO */
}

Bitmap::Bitmap(SDL_Surface * who, bool deep_copy){
    /* TODO */
}

Bitmap::Bitmap( int x, int y ){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file ){
    /* TODO */
}

Bitmap::Bitmap( const std::string & load_file ){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file, int sx, int sy ){
    /* TODO */
}

Bitmap::Bitmap( const char * load_file, int sx, int sy, double accuracy ){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, bool deep_copy){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy ){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, int sx, int sy, double accuracy ){
    /* TODO */
}

Bitmap::Bitmap( const Bitmap & copy, int x, int y, int width, int height ){
    /* TODO */
}

int Bitmap::getWidth() const {
    /* TODO */
    return 0;
}

int Bitmap::getHeight() const {
    /* TODO */
    return 0;
}

int Bitmap::getRed(int c){
    /* TODO */
    return 0;
}

int Bitmap::getBlue(int c){
    /* TODO */
    return 0;
}

int Bitmap::getGreen(int c){
    /* TODO */
    return 0;
}

int Bitmap::makeColor(int red, int blue, int green){
    /* TODO */
    return 0;
}
	
int Bitmap::setGraphicsMode(int mode, int width, int height){
    switch (mode){
        default: {
        // case WINDOWED : {
            screen = SDL_SetVideoMode(width, height, SCREEN_DEPTH, SDL_HWSURFACE);
            if (!screen){
                return 1;
            }
            break;
        }
    }

    if (SCALE_X == 0){
        SCALE_X = width;
    }
    if (SCALE_Y == 0){
        SCALE_Y = height;
    }
    if ( Screen != NULL ){
        delete Screen;
        Screen = NULL;
    }

    if ( Scaler != NULL ){
        delete Scaler;
        Scaler = NULL;
    }

    if ( Buffer != NULL ){
        delete Buffer;
        Buffer = NULL;
    }

    if (width != 0 && height != 0){
        Screen = new Bitmap(screen);
        if ( width != 0 && height != 0 && (width != SCALE_X || height != SCALE_Y) ){
            Scaler = new Bitmap(width, height);
            Buffer = new Bitmap(SCALE_X, SCALE_Y);
        }
    }

    return 0;
}
	
void Bitmap::addBlender( int r, int g, int b, int a ){
    /* TODO */
}
	
void Bitmap::multiplyBlender( int r, int g, int b, int a ){
    /* TODO */
}
	
void Bitmap::differenceBlender( int r, int g, int b, int a ){
    /* TODO */
}
	
Bitmap & Bitmap::operator=(const Bitmap &){
    /* TODO */
    return *this;
}
        
int Bitmap::setGfxModeText(){
    /* TODO */
    return 0;
}

int Bitmap::setGfxModeFullscreen(int x, int y){
    return setGraphicsMode(FULLSCREEN, x, y);
}
	
void Bitmap::drawingMode(int type){
    /* TODO */
}
	
void Bitmap::transBlender( int r, int g, int b, int a ){
    /* TODO */
}
	
void Bitmap::setClipRect( int x1, int y1, int x2, int y2 ) const{
    /* TODO */
}

void Bitmap::destroyPrivateData(){
    SDL_FreeSurface(getData().surface);
}

void Bitmap::putPixel( int x, int y, int col ) const {
    /* TODO */
}
	
void Bitmap::putPixelNormal(int x, int y, int col) const {
    /* TODO */
}
	
bool Bitmap::getError(){
    /* TODO */
    return false;
}

void Bitmap::border( int min, int max, int color ) const {
    /* TODO */
}

void Bitmap::rectangle( int x1, int y1, int x2, int y2, int color ) const {
    /* TODO */
}

void Bitmap::rectangleFill( int x1, int y1, int x2, int y2, int color ) const {
    /* TODO */
}

void Bitmap::circleFill( int x, int y, int radius, int color ) const {
    /* TODO */
}

void Bitmap::circle( int x, int y, int radius, int color ) const {
    /* TODO */
}

void Bitmap::line( const int x1, const int y1, const int x2, const int y2, const int color ) const {
    /* TODO */
}


void Bitmap::draw(const int x, const int y, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::draw(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::drawHFlip(const int x, const int y, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::drawHFlip(const int x, const int y, const int startWidth, const int startHeight, const int width, const int height, const Bitmap & where) const {
    /* TODO */
}

void Bitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawTrans( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawTransHFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawTransVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawTransHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawMask( const int x, const int y, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawStretched( const int x, const int y, const int new_width, const int new_height, const Bitmap & who ){
    /* TODO */
}

void Bitmap::Blit( const std::string & xpath ) const {
    /* TODO */
}

void Bitmap::Blit( const Bitmap & where ) const {
    Blit(0, 0, where);
}

void Bitmap::Blit( const int x, const int y, const Bitmap & where ) const {
    Blit(x, y, 0, 0, where);
}

void Bitmap::Blit( const int mx, const int my, const int wx, const int wy, const Bitmap & where ) const {
    Blit(mx, my, getWidth(), getHeight(), wx, wy, where);
}

void Bitmap::Blit( const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where ) const {
    SDL_Rect source;
    SDL_Rect destination;
    source.w = width;
    source.h = height;
    source.x = mx;
    source.y = my;

    destination.w = width;
    destination.h = height;
    destination.x = wx;
    destination.y = wy;

    SDL_BlitSurface(getData().getSurface(), &source, where.getData().getSurface(), &destination);
}

void Bitmap::BlitMasked( const int mx, const int my, const int width, const int height, const int wx, const int wy, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::BlitToScreen(const int upper_left_x, const int upper_left_y) const {
    if ( Scaler == NULL ){
        this->Blit( upper_left_x, upper_left_y, *Screen );
    } else {
        this->Blit( upper_left_x, upper_left_y, *Buffer );
        Buffer->Stretch(*Scaler);
        Scaler->Blit(0, 0, 0, 0, *Screen);
    }

    SDL_Flip(Screen->getData().getSurface());
}

void Bitmap::BlitAreaToScreen(const int upper_left_x, const int upper_left_y) const {
    /* TODO */
}

void Bitmap::BlitFromScreen(const int x, const int y) const {
    /* TODO */
}

void Bitmap::Stretch( const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::Stretch( const Bitmap & where, const int sourceX, const int sourceY, const int sourceWidth, const int sourceHeight, const int destX, const int destY, const int destWidth, const int destHeight ) const {
    /* TODO */
}
	
void Bitmap::save( const std::string & str ){
    /* TODO */
}
	
void Bitmap::load( const std::string & str ){
    /* TODO */
}

void Bitmap::triangle( int x1, int y1, int x2, int y2, int x3, int y3, int color ) const {
    /* TODO */
}

void Bitmap::ellipse( int x, int y, int rx, int ry, int color ) const {
    /* TODO */
}

void Bitmap::ellipseFill( int x, int y, int rx, int ry, int color ) const {
    /* TODO */
}

void Bitmap::light(int x, int y, int width, int height, int start_y, int focus_alpha, int edge_alpha, int focus_color, int edge_color) const {
    /* TODO */
}

void Bitmap::applyTrans(const int color){
    /* TODO */
}
	
void Bitmap::floodfill( const int x, const int y, const int color ) const {
    /* TODO */
}
	
void Bitmap::horizontalLine( const int x1, const int y, const int x2, const int color ) const {
    /* TODO */
}

void Bitmap::hLine( const int x1, const int y, const int x2, const int color ) const {
    /* TODO */
}

void Bitmap::vLine( const int y1, const int x, const int y2, const int color ) const {
    /* TODO */
}
	
void Bitmap::polygon( const int * verts, const int nverts, const int color ) const {
    /* TODO */
}
	
void Bitmap::arc(const int x, const int y, const double ang1, const double ang2, const int radius, const int color ) const {
    /* TODO */
}
	
void Bitmap::fill( int color ) const {
    /* TODO */
}
	
int Bitmap::darken( int color, double factor ){
    /* TODO */
    return color;
}
        
Bitmap Bitmap::greyScale(){
    /* TODO */
    return *this;
}
	
void Bitmap::drawCharacter( const int x, const int y, const int color, const int background, const Bitmap & where ) const {
    /* TODO */
}

void Bitmap::drawRotate( const int x, const int y, const int angle, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const Bitmap & where ){
    /* TODO */
}

void Bitmap::drawPivot( const int centerX, const int centerY, const int x, const int y, const int angle, const double scale, const Bitmap & where ){
    /* TODO */
}
        
Bitmap Bitmap::memoryPCX(unsigned char * const data, const int length, const bool mask){
    /* TODO */
    return Bitmap();
}
	
int Bitmap::getPixel( const int x, const int y ) const {
    /* TODO */
    return 0;
}
	
void Bitmap::readLine( std::vector< int > & vec, int y ){
    /* TODO */
}

void Bitmap::StretchBy2( const Bitmap & where ){
    /* TODO */
}

void Bitmap::StretchBy4( const Bitmap & where ){
    /* TODO */
}

LitBitmap::LitBitmap( const Bitmap & b ){
    /* TODO */
}

LitBitmap::LitBitmap(){
    /* TODO */
}

LitBitmap::~LitBitmap(){
    /* TODO */
}
	
void LitBitmap::draw( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawHFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}

void LitBitmap::drawHVFlip( const int x, const int y, const Bitmap & where ) const {
    /* TODO */
}
