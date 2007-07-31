#ifndef _load_exception_h
#define _load_exception_h

/* Generic load exception class. Thrown whenever a structure is being created
 * and an error occurs.
 */

#include <exception>
#include <string>

using namespace std;

class LoadException: public exception {
public:
	LoadException();
	LoadException( const string & reason );

	inline const string & getReason() const {
		return reason;
	}

	virtual ~LoadException() throw();

protected:
	string reason;
};

#endif
