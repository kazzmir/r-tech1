#include "select-list.h"

#include "util/bitmap.h"
#include "util/font.h"
#include "util/debug.h"

using namespace Gui;

SelectItem::SelectItem(){
}
SelectItem::~SelectItem(){
}

SelectListInterface::SelectListInterface():
accessEmpty(true),
allowWrap(true){
}

SelectListInterface::~SelectListInterface(){
}

SimpleSelect::SimpleSelect(){
}
SimpleSelect::~SimpleSelect(){
}
void SimpleSelect::act(){
}
void SimpleSelect::render(const Graphics::Bitmap &, const Font &) const{
}
void SimpleSelect::addItem(const Util::ReferenceCount<SelectItem> &){
}
void SimpleSelect::addItems(const std::vector<Util::ReferenceCount<SelectItem> > &){
}
const std::vector<Util::ReferenceCount<SelectItem> > & SimpleSelect::getItems() const{
    return items;
}
void SimpleSelect::clearItems(){
}
void SimpleSelect::setCellDimensions(int width){
}
void SimpleSelect::setCellSpacing(int x, int y){
}
void SimpleSelect::setCursors(int total){
}
int SimpleSelect::totalCursors() const{
    return cursors.size();
}
void SimpleSelect::setCurrentIndex(int cursor, unsigned int location){
}
unsigned int SimpleSelect::getCurrentIndex(int cursor) const{
    return cursors[cursor];
}
bool SimpleSelect::up(int cursor){
    return false;
}
bool SimpleSelect::down(int cursor){
    return false;
}
bool SimpleSelect::left(int cursor){
    return false;
}
bool SimpleSelect::right(int cursor){
    return false;
}

GridSelect::GridSelect(){
}
GridSelect::~GridSelect(){
}
void GridSelect::act(){
}
void GridSelect::render(const Graphics::Bitmap &, const Font &) const{
}
void GridSelect::addItem(const Util::ReferenceCount<SelectItem> &){
}
void GridSelect::addItems(const std::vector<Util::ReferenceCount<SelectItem> > &){
}
const std::vector<Util::ReferenceCount<SelectItem> > & GridSelect::getItems() const{
    return items;
}
void GridSelect::clearItems(){
}
void GridSelect::setCellDimensions(int width){
}
void GridSelect::setCellSpacing(int x, int y){
}
void GridSelect::setCursors(int total){
}
int GridSelect::totalCursors() const{
    return cursors.size();
}
void GridSelect::setCurrentIndex(int cursor, unsigned int location){
}
unsigned int GridSelect::getCurrentIndex(int cursor) const{
    return cursors[cursor];
}
bool GridSelect::up(int cursor){
    return false;
}
bool GridSelect::down(int cursor){
    return false;
}
bool GridSelect::left(int cursor){
    return false;
}
bool GridSelect::right(int cursor){
    return false;
}