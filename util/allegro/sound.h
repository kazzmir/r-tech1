struct SAMPLE;

struct SoundData{
    SoundData():
        sample(NULL),
        voice(-1){
        }

    SoundData(const SoundData & copy):
        sample(copy.sample),
        voice(copy.voice){}

    SoundData & operator=(const SoundData & copy){
        sample = copy.sample;
        voice = copy.voice;
        return *this;
    }

    SAMPLE * sample;
    int voice;
};
