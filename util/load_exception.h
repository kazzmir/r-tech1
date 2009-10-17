#ifndef _load_exception_h
#define _load_exception_h

/* Generic load exception class. Thrown whenever a structure is being created
 * and an error occurs.
 */

#include <exception>
#include <string>

class LoadException: public std::exception {
public:
	LoadException();
	LoadException( const std::string reason );

	inline const std::string & getReason() const {
		return reason;
	}

	virtual ~LoadException() throw();

protected:
        std::string reason;
};

#endif
