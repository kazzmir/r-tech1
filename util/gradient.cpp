#include "gradient.h"
#include "funcs.h"

namespace Effects{

/* this class does virtually no error checking. great job */
Gradient::Gradient(int size, Graphics::Color startColor, Graphics::Color endColor):
colors(0),
size(size),
index(0){
    colors = new Graphics::Color[size];
    Graphics::blend_palette(colors, size / 2, startColor, endColor);
    Graphics::blend_palette(colors + size / 2, size / 2, endColor, startColor);
}
    
Gradient::Gradient(const Gradient & copy):
colors(NULL),
size(copy.size),
index(copy.index){
    colors = new Graphics::Color[size];
    for (unsigned int i = 0; i < size; i++){
        colors[i] = copy.colors[i];
    }
}
    
Gradient & Gradient::operator=(const Gradient & copy){
    delete[] colors;
    size = copy.size;
    index = copy.index;
    colors = new Graphics::Color[size];
    for (unsigned int i = 0; i < size; i++){
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

Graphics::Color Gradient::current() const {
    return colors[index];
}
    
Graphics::Color Gradient::current(int offset) const {
    return colors[(index + offset + size) % size];
}

Gradient::~Gradient(){
    delete[] colors;
}

}
