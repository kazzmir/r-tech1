#ifndef _paintown_util_pointer_h
#define _paintown_util_pointer_h

namespace Util{

/* Some helpful pointer classes, probably equivalent to stuff in boost */

/* Initializes its pointer to NULL and deletes the data in the destructor */
template <class Data>
class ClassPointer{
public:
    ClassPointer():
        data(NULL){
        }

    ClassPointer & operator=(Data * him){
        data = him;
        return *this;
    }

    Data & operator*() const {
        return *data;
    }

    bool operator==(const void * what) const {
        return data == what;
    }

    bool operator!=(const void * what) const {
        return !(*this == what);
    }

    Data* operator->() const {
        return data;
    }

    virtual ~ClassPointer(){
        delete data;
    }

private:
    Data* data;
};

}

#endif
