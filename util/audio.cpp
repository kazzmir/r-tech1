#include "audio.h"

namespace Util{

AudioConverter::AudioConverter(Encoding inputEncoding, int inputChannels, int inputFrequency,
               Encoding outputEncoding, int outputChannels, int outputFrequency){

#ifdef USE_SDL
    SDL_BuildAudioCVT(&conversion, inputEncoding, inputChannels, inputFrequency,
                      outputEncoding, outputChannels, outputFrequency);
#endif

}
    
AudioConverter::~AudioConverter(){
}
    
int AudioConverter::convertedLength(int length){
#ifdef USE_SDL
    return length * conversion.len_mult;
#else
    return length;
#endif
}

int AudioConverter::convert(void * input, int length){
#ifdef USE_SDL
    if (conversion.needed){
        conversion.buf = (Uint8*) input;
        conversion.len = length;
        /* then convert to whatever the real output wants */
        SDL_ConvertAudio(&conversion);
        return conversion.len_cvt;
    } else {
        return length;
    }
#else
    return length;
#endif
}

}
