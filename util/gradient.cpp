#include "gradient.h"
#include "funcs.h"
#include "token.h"

namespace Effects{

/* this class does virtually no error checking. great job */
Gradient::Gradient(int size, Graphics::Color startColor, Graphics::Color endColor):
colors(0),
size(size),
index(0){
    size = Util::clamp(size, 1, 10000);
    colors = new Graphics::Color[size];
    Graphics::blend_palette(colors, size / 2, startColor, endColor);
    Graphics::blend_palette(colors + size / 2, size / 2, endColor, startColor);
}

Gradient::Gradient(const Token * token):
colors(NULL),
size(10),
index(0){
    int lowRed = 0, lowGreen = 0, lowBlue = 0;
    int highRed = 255, highGreen = 255, highBlue = 255;
    token->match("low", lowRed, lowGreen, lowBlue);
    token->match("high", highRed, highGreen, highBlue);
    token->match("distance", size);
    token->match("size", size);

    size = Util::clamp(size, 1, 10000);

    Graphics::Color startColor = Graphics::makeColor(lowRed, lowGreen, lowBlue);
    Graphics::Color endColor = Graphics::makeColor(highRed, highGreen, highBlue);

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
    return *this;
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
