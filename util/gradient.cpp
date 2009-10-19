#include "gradient-glow.h"
#include "funcs.h"

namespace Effects{

GradientGlow::GradientGlow(int size, int startColor, int endColor):
colors(0),
size(size),
index(0){
    colors = new int[size];
    Util::blend_palette(colors, size / 2, startColor, endColor);
    Util::blend_palette(colors + size / 2, size / 2, endColor, startColor);
}

void GradientGlow::update(){
    index = (index + 1) % size;
}

int GradientGlow::current(){
    return colors[index];
}

GradientGlow::~GradientGlow(){
    delete[] colors;
}

}
