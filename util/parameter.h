#ifndef _paintown_parameter_h
#define _paintown_parameter_h

/* parameters are to be used to control the dynamic extent of values. values
 * can be pushed onto the current extent and are automatically popped off
 * upon function return.
 * this is similar to 'parameters' in Racket.
 *
 * This class is not thread safe.
 */

#include <vector>

namespace Util{

/* the static variable `stack' has to be defined somewhere. use this syntax to define it
 *  template <> vector<int> Parameter<int>::stack;
 */
template <class Value>
class Parameter{
public:
    /* push a new value on the stack */
    Parameter(const Value & what):
    items(0){
        push(what);
    }

    Parameter():
    items(0){
    }

    /* pop last value */
    ~Parameter(){
        for (int i = 0; i < items; i++){
            if (stack.size() > 0){
                stack.pop_back();
            }
        }
    }

    void push(const Value & what){
        items += 1;
        stack.push_back(what);
    }

    /* get the current value */
    static Value current(){
        if (stack.size() > 0){
            return stack.back();
        }
        return Value();
    }

    static std::vector<Value> stack;

protected:
    /* number of things pushed onto the stack by this object */
    int items;
};

}

#endif
