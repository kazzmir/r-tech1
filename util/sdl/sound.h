struct Mix_Chunk;

struct SoundData{
    SoundData():
        chunk(NULL){
        }

    SoundData(const SoundData & copy):
        chunk(copy.chunk){}

    SoundData & operator=(const SoundData & copy){
        chunk = copy.chunk;
        return *this;
    }

    Mix_Chunk * chunk;
};
