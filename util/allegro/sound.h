struct SAMPLE;

struct SoundData{
    SoundData():
        sample(NULL){
        }

    SoundData(const SoundData & copy):
        sample(copy.sample){}

    SoundData & operator=(const SoundData & copy){
        sample = copy.sample;
        return *this;
    }

    SAMPLE * sample;
};
