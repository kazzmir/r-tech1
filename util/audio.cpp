#include <string.h>

#include "audio.h"
#include "debug.h"

namespace Util{

#ifdef USE_SDL1
AudioConverter::AudioConverter(Encoding inputEncoding, int inputChannels, int inputFrequency,
               Encoding outputEncoding, int outputChannels, int outputFrequency){

    SDL_BuildAudioCVT(&conversion, inputEncoding, inputChannels, inputFrequency,
                      outputEncoding, outputChannels, outputFrequency);
}
    
AudioConverter::~AudioConverter(){
}
    
int AudioConverter::convertedLength(int length){
    return length;
}

int AudioConverter::convert(void * input, int length){
    if (conversion.needed){
        conversion.buf = (Uint8*) input;
        conversion.len = length;
        /* then convert to whatever the real output wants */
        SDL_ConvertAudio(&conversion);
        return conversion.len_cvt;
    } else {
        return length;
    }
}
#else
AudioConverter::AudioConverter(Encoding inputEncoding, int inputChannels, int inputFrequency,
               Encoding outputEncoding, int outputChannels, int outputFrequency){

    input.bytes = inputEncoding;
    input.channels = inputChannels;
    input.frequency = inputFrequency;

    output.bytes = outputEncoding;
    output.channels = outputChannels;
    output.frequency = outputFrequency;

    sizeRatio = (double) byteSize(output) * output.frequency / ((double) byteSize(input) * input.frequency);
}
    
int AudioConverter::byteSize(const Format & what){
    return encodingBytes(what.bytes) * what.channels;
}
    
/* how many bytes an encoding takes up */
int AudioConverter::encodingBytes(Encoding what){
    switch (what){
        case Signed16: return 2;
        case Float32: return 4;
    }

    return 1;
}
 
int AudioConverter::convertedLength(int length){
    return length * sizeRatio;
}

double CubicInterpolate(double y0,double y1,
                        double y2,double y3,
                        double mu){
    double a0,a1,a2,a3,mu2;

    mu2 = mu*mu;
    a0 = y3 - y2 - y0 + y1;
    a1 = y0 - y1 - a0;
    a2 = y2 - y0;
    a3 = y1;

    return (a0*mu*mu2+a1*mu2+a2*mu+a3);
}

template <class Size>
Size clamp(double input){
    Size top = (1 << (sizeof(Size) * 8 - 1)) - 1;
    Size bottom = -(1 << (sizeof(Size) * 8 - 1));
    if (input > top){
        return top;
    }
    if (input < bottom){
        return bottom;
    }
    return input;
}

template <>
float clamp(double input){
    return input;
}

template <class Size>
void doConvertRate(Size * input, Size * buffer, int length, int outputLength, double ratio, int channels){
    for (int sample = 0; sample < outputLength; sample += 1){
        double inputSample = sample / ratio;

        for (int channel = 0; channel < channels; channel += 1){

            int sample0 = ((int) inputSample - 1) * channels + channel;
            int sample1 = ((int) inputSample + 0) * channels + channel;
            int sample2 = ((int) inputSample + 1) * channels + channel;
            int sample3 = ((int) inputSample + 2) * channels + channel;

            if (sample0 < 0){
                sample0 = sample1;
            }

            if (sample2 >= length * channels){
                sample2 = sample1;
            }

            if (sample3 >= length * channels){
                sample3 = sample2;
            }

            buffer[sample * channels + channel] = clamp<Size>(CubicInterpolate(input[sample0], input[sample1], input[sample2], input[sample3], inputSample - (int) inputSample));

            // Global::debug(0) << "Input[" << sample << "] " << channel << ": " << input[sample1] << " Output: " << buffer[sample * 2 + channel] << std::endl;
        }
    }
}

int AudioConverter::convert(void * input, int length){
    /* no conversion needed */
    if (this->input == this->output){
        return length;
    }

    int total = convertedLength(length);
    /* make sure we get an even number of samples */
    if (total % byteSize(output) != 0){
        total -= total % byteSize(output);
    }

    char * buffer = new char[total];

    if (this->input.channels == output.channels &&
        this->input.bytes == output.bytes){
        switch (this->input.bytes){
            case Signed16: doConvertRate<signed short>((signed short*) input, (signed short*) buffer, length / sizeof(signed short) / this->input.channels, total / sizeof(signed short) / output.channels, sizeRatio, output.channels); break;
            case Float32: doConvertRate<float>((float*) input, (float*) buffer, length / sizeof(float) / this->input.channels, total / sizeof(float) / output.channels, sizeRatio, output.channels);
        }
    }
    
    memcpy(input, buffer, total);
    delete[] buffer;

    return total;
}

AudioConverter::~AudioConverter(){
}
        
bool AudioConverter::Format::operator==(const AudioConverter::Format & him) const {
    return this->bytes == him.bytes &&
           this->channels == him.channels &&
           this->frequency == him.frequency;
}

#endif

}
