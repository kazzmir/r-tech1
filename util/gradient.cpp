#include "gradient.h"
#include "funcs.h"

namespace Effects{

/* this class does virtually no error checking. great job */
Gradient::Gradient(int size, int startColor, int endColor):
colors(0),
size(size),
index(0){
    colors = new int[size];
    Util::blend_palette(colors, size / 2, startColor, endColor);
    Util::blend_palette(colors + size / 2, size / 2, endColor, startColor);
}
    
Gradient::Gradient(const Gradient & copy):
colors(NULL),
size(copy.size),
index(copy.index){
    colors = new int[size];
    for (int i = 0; i < size; i++){
        colors[i] = copy.colors[i];
    }
}

void Gradient::forward(){
    index = (index + 1) % size;
}

void Gradient::backward(){
    index = (index - 1 + size) % size;
}

void Gradient::update(){
    forward();
}

void Gradient::reset(){
    index = 0;
}

int Gradient::current() const {
    return colors[index];
}
    
int Gradient::current(int offset) const {
    return colors[(index + offset + size) % size];
}

Gradient::~Gradient(){
    delete[] colors;
}

}
