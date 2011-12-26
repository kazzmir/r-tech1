#ifndef _hqx_h
#define _hqx_h

#include <stdint.h>

namespace hq2x{
    void filter_render_565(uint16_t *output, unsigned outpitch,
                           const uint16_t *input, unsigned pitch, unsigned width, unsigned height);
}

#endif
