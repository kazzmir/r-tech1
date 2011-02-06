#include <sstream>
#include <allegro5/allegro_memfile.h>

static const int rgb_r_shift_16 = 0;
static const int rgb_g_shift_16 = 5;
static const int rgb_b_shift_16 = 11;

static inline int pack565(unsigned char red, unsigned char green, unsigned char blue){
    return (((red >> 3) << rgb_r_shift_16) |
            ((green >> 2) << rgb_g_shift_16) |
            ((blue >> 3) << rgb_b_shift_16));
}

/* from allegro4 */
static inline void unpack565(int color, unsigned char * red, unsigned char * green, unsigned char * blue){
    static int _rgb_scale_5[32] = {
        0,   8,   16,  24,  33,  41,  49,  57,
        66,  74,  82,  90,  99,  107, 115, 123,
        132, 140, 148, 156, 165, 173, 181, 189,
        198, 206, 214, 222, 231, 239, 247, 255
    };

    static int _rgb_scale_6[64] = {
        0,   4,   8,   12,  16,  20,  24,  28,
        32,  36,  40,  44,  48,  52,  56,  60,
        65,  69,  73,  77,  81,  85,  89,  93,
        97,  101, 105, 109, 113, 117, 121, 125,
        130, 134, 138, 142, 146, 150, 154, 158,
        162, 166, 170, 174, 178, 182, 186, 190,
        195, 199, 203, 207, 211, 215, 219, 223,
        227, 231, 235, 239, 243, 247, 251, 255
    };

    *red = _rgb_scale_5[(color >> rgb_r_shift_16) & 0x1F];
    *green = _rgb_scale_6[(color >> rgb_g_shift_16) & 0x3F];
    *blue = _rgb_scale_5[(color >> rgb_b_shift_16) & 0x1F];
}

Bitmap::Bitmap(int width, int height):
own(NULL),
mustResize(false),
bit8MaskColor(0){
    ALLEGRO_BITMAP * bitmap = al_create_bitmap(width, height);
    if (bitmap == NULL){
        std::ostringstream out;
        out << "Could not create bitmap with dimensions " << width << ", " << height;
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    getData().setBitmap(bitmap);
    own = new int;
    *own = 1;
}

enum Format{
    PNG,
    GIF,
};

static ALLEGRO_BITMAP * memoryGIF(const char * data, int length){
    ALLEGRO_FILE * memory = al_open_memfile((void *) data, length, "r");
    al_fclose(memory);

/* FIXME: get gif addon for a5 */

#if 0
    RGB * palette = NULL;
    /* algif will close the packfile for us in both error and success cases */
    BITMAP * gif = load_gif_packfile(pack, palette);
    if (!gif){
        al_fclose(memory);
        // pack_fclose(pack);
        ostringstream out;
        out <<"Could not load gif from memory: " << (void*) data << " length " << length;
        throw LoadException(__FILE__, __LINE__, out.str());
    }

    BITMAP * out = create_bitmap(gif->w, gif->h);
    blit(gif, out, 0, 0, 0, 0, gif->w, gif->h);
    destroy_bitmap(gif);
    // pack_fclose(pack);
#endif

    ALLEGRO_BITMAP * out = NULL;

    return out;
}

static ALLEGRO_BITMAP * load_bitmap_from_memory(const char * data, int length, Format type){
    switch (type){
        case PNG : {
            break;
        }
        case GIF : {
            return memoryGIF(data, length);
            break;
        }
    }
    throw Exception::Base(__FILE__, __LINE__);
}

Bitmap::Bitmap(const char * data, int length):
own(NULL),
mustResize(false),
bit8MaskColor(0){
    Format type = GIF;
    getData().setBitmap(load_bitmap_from_memory(data, length, type));
    if (getData().getBitmap() == NULL){
        std::ostringstream out;
        out << "Could not create bitmap from memory";
        throw BitmapException(__FILE__, __LINE__, out.str());
    }
    own = new int;
    *own = 1;
}

Bitmap::Bitmap( const Bitmap & copy, int x, int y, int width, int height ):
own(NULL),
mustResize(false),
bit8MaskColor(copy.bit8MaskColor){
    path = copy.getPath();
    ALLEGRO_BITMAP * his = copy.getData().getBitmap();
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (width + x > al_get_bitmap_width(his)){
        width = al_get_bitmap_width(his) - x;
    }
    if (height + y > al_get_bitmap_height(his)){
        height = al_get_bitmap_height(his) - y;
    }

    ALLEGRO_BITMAP * sub = al_create_sub_bitmap(his, x, y, width, height);
    getData().setBitmap(sub);
    
    own = new int;
    *own = 1;
}

int Bitmap::getWidth() const {
    if (getData().getBitmap() != NULL){
        return al_get_bitmap_width(getData().getBitmap());
    }

    return 0;
}

int Bitmap::getRed(int color){
    unsigned char red, green, blue;
    unpack565(color, &red, &green, &blue);
    return red;
}

int Bitmap::getGreen(int color){
    unsigned char red, green, blue;
    unpack565(color, &red, &green, &blue);
    return green;
}

int Bitmap::getBlue(int color){
    unsigned char red, green, blue;
    unpack565(color, &red, &green, &blue);
    return blue;
}

int Bitmap::makeColor(int red, int blue, int green){
    return pack565(red, blue, green);
}

int Bitmap::getHeight() const {
    if (getData().getBitmap() != NULL){
        return al_get_bitmap_height(getData().getBitmap());
    }

    return 0;
}

int Bitmap::setGraphicsMode(int mode, int width, int height){
    al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_RGB_565);
    return 0;
}

int Bitmap::getPixel(const int x, const int y) const {
    ALLEGRO_COLOR color = al_get_pixel(getData().getBitmap(), x, y);
    return pack565(color.r, color.g, color.b);
}
