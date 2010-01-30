#include "bitmap.h"
#include <vector>
#include "fire.h"
#include <string.h>
#include "funcs.h"
#include <math.h>

class Bitmap;

namespace Paintown{

static int MAX_X = 320;
static int MAX_Y = 240;

Fire::Fire(){
    Util::blend_palette(colors, 64, Bitmap::makeColor(64, 0, 0), Bitmap::makeColor(255, 0, 0));
    Util::blend_palette(colors + 64, 64, Bitmap::makeColor(255, 0, 0), Bitmap::makeColor(255, 255, 0));
    Util::blend_palette(colors + 64 + 64, 96, Bitmap::makeColor(255, 255, 0), Bitmap::makeColor(255, 255, 255));
    Util::blend_palette(colors + 64 + 64 + 64 + 32, 32, Bitmap::makeColor(255, 255, 255), Bitmap::makeColor(255, 255, 255));

    data = new unsigned char*[MAX_Y];
    for (int i = 0; i < MAX_Y; i++){
        data[i] = new unsigned char[MAX_X];
        memset(data[i], 0, MAX_X);
    }

    for (int i = 0; i < MAX_HOTSPOTS; i++){
        hotspots[i] = Util::rnd(MAX_X);
        directions[i] = 0;
    }
}

void Fire::updateHotspots(){
    for (int i = 0; i < MAX_HOTSPOTS; i++){
        // hotspots[i] = (hotspots[i] + Util::rnd(5) - 2) % MAX_X;
        hotspots[i] += directions[i];
        if (hotspots[i] >= MAX_X){
            hotspots[i] -= MAX_X;
        }
        if (hotspots[i] < 0){
            hotspots[i] += MAX_X;
        }
        directions[i] += (double) (Util::rnd(7) - 3) / 5.0;
        if (directions[i] < -4){
            directions[i] = -4;
        }
        if (directions[i] > 4){
            directions[i] = 4;
        }
    }

    int hotspot_length = 40;

    for (int i = 0; i < MAX_HOTSPOTS; i++){
        for (int x = -hotspot_length/2; x < hotspot_length/2; x++){
            int position = ((int) hotspots[i] + x + MAX_X) % MAX_X;
            int more = (int) data[MAX_Y-1][position] + (hotspot_length / 2 - fabs(x));
            if (more >= MAX_COLORS){
                more = MAX_COLORS - 1;
            }
            data[MAX_Y-1][position] = more;
        }
    }
}

void Fire::update(){
    updateHotspots();

    int decay = 1;

    for (int y = MAX_Y-1; y > 0; y--){
        for (int x = 0; x < MAX_X; x++){
            if (y < MAX_Y-1){
                int lx = x-1;
                if (lx < 0){
                    lx += MAX_X;
                }
                int rx = x+1;
                if (rx >= MAX_X){
                    rx -= MAX_X;
                }
                // int less = (int)((double) data[y+1][x] * 0.6 + (double) data[y+1][lx] * 0.3 + (double) data[y+1][rx] * 0.3);
                unsigned char * down = data[y+1];
                // int less = (int)(((double) down[x]) * 0.9 + ((double) down[lx]) * 0.05 + ((double) down[rx]) * 0.05);
                int less = (double) down[lx] * 0.20;
                less += (double) down[rx] * 0.20;
                less += (double) down[x] * 0.62;
                less -= Util::rnd(9);
                if (less < 0){
                    less = 0;
                }
                if (less >= MAX_COLORS){
                    less = MAX_COLORS - 1;
                }
                data[y][x] = (unsigned char) less;
            } else {
                int less = (int)((double) data[y][x] * 0.95);
                if (less < 0){
                    less = 0;
                }
                data[y][x] = (unsigned char) less;
            }
        }
    }
}

void Fire::draw(const Bitmap & work){
    // Bitmap::drawingMode(Bitmap::MODE_TRANS);
    for (int y = 0; y < MAX_Y; y++){
        for (int x = 0; x < MAX_X; x++){
            // work.putPixel(x, y, colors[data[y][x]]);
            if (data[y][x] > 0){
                work.rectangleFill(x*2, y*2, x*2+1, y*2+1, colors[data[y][x]]);
            }
        }
    }
    // Bitmap::drawingMode(Bitmap::MODE_SOLID);
}

Fire::~Fire(){
    for (int y = 0; y < MAX_Y; y++){
        delete[] data[y];
    }
    delete[] data;
}

}
