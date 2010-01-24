#include <allegro.h>
#include "compress.h"
#include "memory.h"
#include "globals.h"

namespace Compress{

void testCompression(unsigned char * input, int length){
    unsigned char * data = new unsigned char[length*2];
    PACKFILE_VTABLE table = Memory::makeTable();
    Memory::memory memory(data, length*2);
    PACKFILE * pack = pack_fopen_vtable(&table, &memory);

    LZSS_PACK_DATA * lzss = create_lzss_pack_data();
    int r = lzss_write(pack, lzss, length, input, 1);
    if (r != 0){
        // printf("lzss error %d: %s!\n", r, ustrerror(errno));
        exit(-1);
    }
    free_lzss_pack_data(lzss);
    pack_fclose(pack);

    Global::debug(0) << "Compressed " << length << " to " << memory.getSize() << " ratio is " << ((double)memory.getSize() / length) << std::endl;

    delete[] data;
}

}
