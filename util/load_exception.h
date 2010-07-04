#ifndef _load_exception_h
#define _load_exception_h

#include "exceptions/exception.h"

/* Generic load exception class. Thrown whenever a structure is being created
 * and an error occurs.
 */

#include <string>

/* FIXME: put this in the Exception namespace */
class LoadException: public Exception::Base {
public:
    LoadException(const std::string & file, int line, const std::string & reason);
    LoadException(const std::string & file, int line, const Exception::Base & nested, const std::string & reason);
    LoadException(const LoadException & copy);

    virtual ~LoadException() throw();

protected:
    virtual const std::string getReason() const;

    virtual Exception::Base * copy() const;

    std::string reason;
};

#endif
