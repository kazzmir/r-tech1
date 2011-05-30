#include "util/bitmap.h"
#include "util/funcs.h"
#include "widget.h"
#include <math.h>
#include "globals.h"
#include "util/token.h"
#include "coordinate.h"
#include "util/load_exception.h"
#include <sstream>

using namespace Gui;

static const double DEG_TO_RAD = 180.0/Util::pi;
static const double RAD_TO_DEG = Util::pi/180.0;

static void round_double (double dbl_to_round , int &rounded_num) {
    rounded_num = static_cast<int>(dbl_to_round);
    if ((dbl_to_round - static_cast<double>(rounded_num)) >= 0.5) {rounded_num++;}
}

//! min (borrowed from allegro)
static inline int Min(int x, int y){ return (((x) < (y)) ? (x) : (y)); }

//! max (borrowed from allegro)
static inline int Max(int x, int y){ return (((x) > (y)) ? (x) : (y)); }

//! mid (borrowed from allegro)
static inline int Mid(int x,int y,int z){ return (Max((x), Min((y), (z)))); }

ColorInfo::ColorInfo():
body(Graphics::makeColor(0, 0, 0)),
/* alpha 0 is invisible, 255 is opaque. set something in the middle as default */
bodyAlpha(128),
border(Graphics::makeColor(0, 0, 0)),
borderAlpha(128){
}

Transformations::Transformations():
radius(0){
}

Transformations::Transformations(const Transformations & transforms):
radius(transforms.radius){
}

Transformations::~Transformations(){
}

Transformations & Transformations::operator=(const Transformations & transforms){
    this->radius = transforms.radius;
    return *this;
}

Widget::Widget(){
	// Nothing yet
}
		
Widget::Widget( const Widget & w ){
    this->location = w.location;
    this->transforms = w.transforms;
}

Widget::~Widget(){
}

// copy
Widget &Widget::operator=( const Widget &copy){
    this->location = copy.location;
    this->transforms = copy.transforms;

    return *this;
}

void Widget::setCoordinates(const Token * token){
    if ( *token == "position" ){
        int x, y, width, height;
        token->view() >> x >> y >> width >> height;
        AbsolutePoint pos(x, y);
        AbsolutePoint dimensions(x + width, y + height);
        location.setPosition(pos);
        location.setPosition2(dimensions);
    } else if ( *token == "relative-position" ){
        double x1, y1, x2, y2;
        token->view() >> x1 >> y1 >> x2 >> y2;
        RelativePoint pos(x1,y1);
        RelativePoint dimensions(x2,y2);
        location = Coordinate(pos, dimensions);
    } else if ( *token == "coordinate" ){
        TokenView view = token->view();
        while (view.hasMore()){
            const Token * coordToken;
            view >> coordToken;
            if (*coordToken == "absolute"){
                int x, y, width, height;
                coordToken->view() >> x >> y >> width >> height;
                AbsolutePoint pos(x, y);
                AbsolutePoint dimensions(x + width, y + height);
                location = Coordinate(pos, dimensions);
            } else if (*coordToken == "relative"){
                double x1, y1, x2, y2;
                coordToken->view() >> x1 >> y1 >> x2 >> y2;
                RelativePoint pos(x1,y1);
                RelativePoint dimensions(x2,y2);
                location = Coordinate(pos, dimensions);
            } else if (*coordToken == "radius"){
		double radius = 0;
                coordToken->view() >> radius;
		transforms.setRadius(radius);
            } else if (*coordToken == "z"){
                double z;
                coordToken->view() >> z;
                location.setZ(z);
            }
        }
    }

    if (location.getWidth() < 0 || location.getHeight() < 0){
        std::ostringstream out;
        out << "Invalid location dimension (cannot have a negative size). Width " << location.getWidth() << " Height " << location.getHeight() << ". From token: ";
        token->toString(out, "");
        throw LoadException(__FILE__, __LINE__, out.str());
    }
}

void Widget::setColors(const Token * token){
    if ( *token == "position-body" ) {
        // This handles the body color of the widget
        int r,g,b;
        token->view() >> r >> g >> b >> colors.bodyAlpha;
        colors.body = Graphics::makeColor(r,g,b);
    } else if ( *token == "position-border" ) {
        // This handles the border color of the widget
        int r,g,b;
        token->view() >> r >> g >> b >> colors.borderAlpha;
        colors.border = Graphics::makeColor(r,g,b);
    } 
}
        
void Widget::render(const Graphics::Bitmap & bitmap, const Font & font){
    render(bitmap);
}

/* draws a quarter arc */
void Widget::arc(const Graphics::Bitmap & work, int x, int y, double startAngle, int radius, Graphics::Color color){

    work.arc(x, y, startAngle, startAngle + Util::pi / 2, radius, color);

#if 0
    int q = 0;// for counters
    double d_q = 0.0;// for percentage of loop completed
    double d_q_plus_one = 0.0;

    const double d_angular_length_deg = 0.030;
    const double d_start_angle_rad = startAngle*DEG_TO_RAD;
    const double d_angular_length_rad = d_angular_length_deg*DEG_TO_RAD;
    const double d_arc_distance_between_points = 0.25*pow(2.0 , static_cast<double>(10) / 10.0);
    double d_angular_length_rad_per_segment = 0.0;

    double arc_length = radius*d_angular_length_rad;
    if (arc_length < 0.0) {arc_length *= -1.0;}
    const double d_num_segments = arc_length / d_arc_distance_between_points;

    int num_segments = 0;
    round_double(d_num_segments , num_segments);

    if (num_segments == 0) {num_segments += 1;} // need at least one segment (two points)
    const int num_points = num_segments + 1;
    const double d_num_points_minus_one = static_cast<double>(num_points - 1);

    int arc_point_x;//[num_points];
    int arc_point_y;//[num_points];
    int arc_point2_x;//[num_points];
    int arc_point2_y;//[num_points];
    double d_arc_point_x = 0.0;
    double d_arc_point_y = 0.0;
    double d_arc_point2_x = 0.0;
    double d_arc_point2_y = 0.0;

    double current_angle_rad = 0.0;
    double current_angle2_rad = 0.0;

    if (d_arc_distance_between_points <= 1.0) {
        for (q = 0 ; q < num_points ; q++) {
            d_q = static_cast<double>(q);
            current_angle_rad = d_start_angle_rad + (d_q / d_num_points_minus_one)*d_angular_length_rad;
            d_arc_point_x = x + radius*cos(current_angle_rad);
            d_arc_point_y = y + radius*sin(current_angle_rad);

            round_double(d_arc_point_x , arc_point_x);
            round_double(d_arc_point_y , arc_point_y);

            work.putPixel(arc_point_x, arc_point_y, color);
        }
    }
    if (d_arc_distance_between_points > 1.0) {

        d_angular_length_rad_per_segment = d_angular_length_rad / d_num_points_minus_one;
        for (q = 0 ; q < num_segments ; q++) {
            d_q = static_cast<double>(q);
            d_q_plus_one = static_cast<double>(q + 1);

            current_angle_rad = d_start_angle_rad + d_q*d_angular_length_rad_per_segment;
            current_angle2_rad = d_start_angle_rad + d_q_plus_one*d_angular_length_rad_per_segment;

            d_arc_point_x = x + radius*cos(current_angle_rad);
            d_arc_point_y = y + radius*sin(current_angle_rad);

            round_double(d_arc_point_x , arc_point_x);
            round_double(d_arc_point_y , arc_point_y);

            d_arc_point2_x = x + radius*cos(current_angle2_rad);
            d_arc_point2_y = y + radius*sin(current_angle2_rad);

            round_double(d_arc_point2_x , arc_point2_x);
            round_double(d_arc_point2_y , arc_point2_y);

            work.line(arc_point_x,arc_point_y, arc_point2_x, arc_point2_y,color);
        }
    }
#endif
}

void Widget::roundRect(const Graphics::Bitmap & work, int radius, int x1, int y1, int x2, int y2, Graphics::Color color){
    const int width = x2 - x1;
    const int height = y2 - y1;
    radius = Mid(0, radius, Min((x1+width - x1)/2, (y1+height - y1)/2));
    work.line(x1+radius, y1, x1+width-radius, y1, color);
    work.line(x1+radius, y1+height, x1+width-radius,y1+height, color);
    work.line(x1, y1+radius,x1, y1+height-radius, color);
    work.line(x1+width, y1+radius,x1+width, y1+height-radius, color);

#if 0
    /* upper left. draw from 180 to 270 */
    arc(work, x1+radius, y1+radius, S_PI / 2, radius, color);
    /* upper right. draw from 90 to 0 */
    arc(work, x1+radius + (width - radius *2), y1+radius, S_PI, radius, color);
    /* lower right. draw from 0 to 270 */
    arc(work, x1+width-radius, y1+height-radius, 3 * S_PI / 2, radius ,color);
    /* lower left. draw from 180 to 270 */
    arc(work, x1+radius, y1+height-radius, 0, radius, color);
#endif

    double quarterTurn = Util::pi / 2;
    double quadrant1 = 0;
    /* signs are flipped because the coordinate system is reflected over the y-axis */
    double quadrant2 = -Util::pi / 2;
    double quadrant3 = Util::pi;
    double quadrant4 = -3 * Util::pi / 2; 

    /* upper right. draw from 90 to 0 */
    work.arc(x1+radius + (width - radius *2), y1 + radius, quadrant1, quadrant1 + quarterTurn, radius, color);
    /* upper left. draw from 180 to 270 */
    work.arc(x1 + radius, y1 + radius, quadrant2, quadrant2 + quarterTurn, radius, color);
    /* lower left. draw from 180 to 270 */
    work.arc(x1 + radius, y1 + height - radius, quadrant3, quadrant3 + quarterTurn, radius, color);
    /* lower right. draw from 0 to 270 */
    work.arc(x1+width-radius, y1+height-radius, quadrant4, quadrant4 + quarterTurn, radius, color);


#if 0
    arc(work, x1+radius, y1+radius, S_PI-1.115, radius, color);
    /* upper right */
    arc(work, x1+radius + (width - radius *2), y1+radius, -S_PI/2 +0.116, radius, color);
    /* lower right */
    arc(work, x1+width-radius, y1+height-radius, -0.110, radius ,color);
    /* lower left */
    arc(work, x1+radius, y1+height-radius, S_PI/2-0.119, radius, color);
#endif
}

void Widget::roundRectFill(const Graphics::Bitmap & work, int radius, int x1, int y1, int x2, int y2, Graphics::Color color){
    const int width = x2 - x1;
    const int height = y2 - y1;
    radius = Mid(0, radius, Min((x1+width - x1)/2, (y1+height - y1)/2));
    /*
    work.circleFill(x1+radius, y1+radius, radius, color);
    work.circleFill((x1+width)-radius, y1+radius, radius, color);
    work.circleFill(x1+radius, (y1+height)-radius, radius, color);
    work.circleFill((x1+width)-radius, (y1+height)-radius, radius, color);
    */

    double quarterTurn = Util::pi / 2;
    double quadrant1 = 0;
    /* signs are flipped because the coordinate system is reflected over the y-axis */
    double quadrant2 = -Util::pi / 2;
    double quadrant3 = Util::pi;
    double quadrant4 = -3 * Util::pi / 2; 

    /* upper right. draw from 90 to 0 */
    work.arcFilled(x1+radius + (width - radius *2), y1 + radius, quadrant1, quadrant1 + quarterTurn, radius, color);
    /* upper left. draw from 180 to 270 */
    work.arcFilled(x1 + radius, y1 + radius, quadrant2, quadrant2 + quarterTurn, radius, color);
    /* lower left. draw from 180 to 270 */
    work.arcFilled(x1 + radius, y1 + height - radius, quadrant3, quadrant3 + quarterTurn, radius, color);
    /* lower right. draw from 0 to 270 */
    work.arcFilled(x1+width-radius, y1+height-radius, quadrant4, quadrant4 + quarterTurn, radius, color);

    work.rectangleFill(x1+radius + 1, y1, x2-radius - 1, y1+radius - 1, color);
    work.rectangleFill(x1, y1+radius, x2, y2-radius, color);
    work.rectangleFill(x1+radius + 1, y2-radius + 1, x2-radius - 1, y2, color);
}

static void foobar(int a, int b){
    a = a + b;
}

Util::ReferenceCount<Graphics::Bitmap> Widget::checkWorkArea(const Graphics::Bitmap & parent){
    if (location.getWidth() <= 0 || location.getHeight() <= 0){
        return NULL;
    }

    return new Graphics::Bitmap(parent, location.getX(), location.getY(), location.getWidth(), location.getHeight());

    /*
    if (! workArea){
        workArea = new Graphics::Bitmap(location.getWidth(), location.getHeight());
    } else if (location.getWidth() < workArea->getWidth() || location.getHeight() < workArea->getHeight()){
        delete workArea;
        workArea = new Graphics::Bitmap(location.getWidth(), location.getHeight());
    } else if (location.getWidth() > workArea->getWidth() || location.getHeight() > workArea->getHeight()){
        delete workArea;
        workArea = new Graphics::Bitmap(location.getWidth(), location.getHeight());
    }

    if (workArea){
        workArea->fill(Graphics::MaskColor());
    }
    */
}
