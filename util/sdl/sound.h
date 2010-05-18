struct Mix_Chunk;

struct SoundData{
    SoundData():
        chunk(NULL),
        channel(-1){
        }

    SoundData(const SoundData & copy):
        chunk(copy.chunk),
        channel(copy.channel){}

    SoundData & operator=(const SoundData & copy){
        chunk = copy.chunk;
        channel = copy.channel;
        return *this;
    }

    Mix_Chunk * chunk;
    int channel;
};
