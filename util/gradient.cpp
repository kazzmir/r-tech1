#include "gradient.h"
#include "funcs.h"

namespace Effects{

Gradient::Gradient(int size, int startColor, int endColor):
colors(0),
size(size),
index(0){
    colors = new int[size];
    Util::blend_palette(colors, size / 2, startColor, endColor);
    Util::blend_palette(colors + size / 2, size / 2, endColor, startColor);
}

void Gradient::update(){
    index = (index + 1) % size;
}

int Gradient::current(){
    return colors[index];
}

Gradient::~Gradient(){
    delete[] colors;
}

}
