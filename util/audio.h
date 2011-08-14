#ifndef _paintown_audio_h
#define _paintown_audio_h

#ifdef USE_SDL
#include <SDL.h>
#endif

/* Deals with audio conversion between any source format and any destination format.
 * A format consists of
 *   byte encoding (8/16/32 bit, signed/unsigned, floating point/integer)
 *   number of channels (mono/stereo)
 *   frequency (22050hz, 44100hz, arbitrary hz)
 */

namespace Util{

typedef int Encoding;

class AudioConverter{
public:
    AudioConverter(Encoding inputEncoding, int inputChannels, int inputFrequency,
                   Encoding outputEncoding, int outputChannels, int outputFrequency);
    
    /* given some input length, return how long the converted output will be */
    int convertedLength(int length);

    /* convert the audio, put the output in the same buffer passed in -- 'input'
     * and returns the number of converted samples.
     */
    int convert(void * input, int length);

    virtual ~AudioConverter();

protected:
#ifdef USE_SDL
    SDL_AudioCVT conversion;
#endif
};

}

#endif
