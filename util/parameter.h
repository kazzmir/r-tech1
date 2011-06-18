#ifndef _paintown_parameter_h
#define _paintown_parameter_h

/* parameters are to be used to control the dynamic extent of values. values
 * can be pushed onto the current extent and are automatically popped off
 * upon function return.
 * this is similar to 'parameters' in Racket.
 *
 * This class is not thread safe.
 */

#include <map>
#include <vector>
#include <string>

namespace Util{

/* the static variable `stack' has to be defined somewhere. use this syntax to define it
 *  template <class Value> typename Util::Parameter<Value>::container Util::Parameter<Value>::stacks;
 */
template <class Value>
class Parameter{
public:
    /* push a new value on the stack */
    Parameter(const Value & what, const std::string & name):
    items(0),
    name(name){
        push(what);
    }

    Parameter(const std::string & name):
    items(0),
    name(name){
    }

    /* pop last value */
    ~Parameter(){
        std::vector<Value> & stack = stacks[name];
        for (int i = 0; i < items; i++){
            if (stack.size() > 0){
                stack.pop_back();
            }
        }
    }

    void push(const Value & what){
        std::vector<Value> & stack = stacks[name];
        items += 1;
        stack.push_back(what);
    }

    /* get the current value */
    static Value current(const std::string & name){
        std::vector<Value> & stack = stacks[name];
        if (stack.size() > 0){
            return stack.back();
        }
        return Value();
    }

    typedef std::map<const std::string, std::vector<Value> > container;
    static container stacks;

protected:
    /* number of things pushed onto the stack by this object. note this is
     * not the number of items on the stack because if there are multiple
     * parameter objects then each one could have pushed 1 object on. thus
     * the stack will have 2 things in it but each parameter will have an
     * 'items' count of 1.
     */
    int items;
    std::string name;
};

}

#endif
