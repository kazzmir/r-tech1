namespace Graphics{

/* saves some state and restores it RAII style */
class RestoreState{
public:
    RestoreState();
    ~RestoreState();
};

}
