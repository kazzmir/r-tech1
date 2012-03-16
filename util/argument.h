#ifndef _paintown_util_argument_h
#define _paintown_util_argument_h

#include <vector>
#include <string>
#include "pointer.h"

class ArgumentAction{
public:
    virtual void act() = 0;
    virtual ~ArgumentAction();
};

typedef std::vector<Util::ReferenceCount<ArgumentAction> > ActionRefs;

class Argument{
public:
    /* Keywords on the command line that should invoke this argument */
    virtual std::vector<std::string> keywords() const = 0;
    
    /* Parse more strings from the command line. Any actions that should take place
     * after the command line has been parsed should be put into 'actions'
     */
    virtual std::vector<std::string>::iterator parse(std::vector<std::string>::iterator current, std::vector<std::string>::iterator end, ActionRefs & actions) = 0;

    /* Description of what this argument does */
    virtual std::string description() const = 0;

    bool isArg(const std::string & what) const;

    virtual ~Argument();
};

#endif
