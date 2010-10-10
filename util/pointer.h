#ifndef _paintown_util_pointer_h
#define _paintown_util_pointer_h

namespace Util{

/* Some helpful pointer classes, probably equivalent to stuff in boost
 */

template <class Data>
class ReferenceCount{
public:
    ReferenceCount(Data * what = NULL):
    count(NULL),
    data(what){
        count = new int;
        *count = 1;
    }

    ReferenceCount(const ReferenceCount<Data> & him){
        data = him.data;
        count = him.count;
        *count += 1;
    }

    ReferenceCount & operator=(const ReferenceCount<Data> & him){
        release();
        data = him.data;
        count = him.count;
        *count += 1;
        return *this;
    }

    Data * operator->() const {
        return data;
    }
    
    Data & operator*() const {
        return *data;
    }

    bool operator==(const ReferenceCount<Data> & him) const {
        return data == him.data;
    }

    bool operator!=(const ReferenceCount<Data> & him) const {
        return !(*this == him);
    }

    bool operator==(const void * what) const {
        return data == what;
    }

    bool operator!=(const void * what) const {
        return !(*this == what);
    }

    virtual ~ReferenceCount(){
    }

protected:

    void release(){
        *count -= 1;
        if (*count == 0){
            delete data;
            delete count;
            data = NULL;
            count = NULL;
        }
    }

    int * count;
    Data * data;
};

/* Initializes its pointer to NULL and deletes the data in the destructor */
template <class Data>
class ClassPointer{
public:
    ClassPointer():
        data(NULL){
        }

    ClassPointer & operator=(Data * him){
        if (data != NULL){
            delete data;
        }

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
