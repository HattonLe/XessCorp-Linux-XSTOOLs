/*----------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.

	1997-2010 - X Engineering Software Systems Corp.
----------------------------------------------------------------------------------*/


#ifndef XSERROR_H
#define XSERROR_H

#include <iostream>
#include <string>
using namespace std;

#define ERRLOC	__FILE__(__LINE__)


/// Severity of errors.
typedef enum
{
	XSErrorMin,		///< minimum error index
    XSErrorNone,    ///< no error (not initialized)
    XSErrorDebug,   ///< for bug\development issues
    XSErrorMinor,   ///< minor error (no abort)
	XSErrorMajor,	///< major error (no abort)
	XSErrorFatal,	///< fatal error (causes abort)
	XSErrorMax		///< maximum error index
} XSErrorSeverity;


/// States of the error object.
typedef enum
{
	XSErrorInitial,   ///< Just starting error message
	XSErrorInMessage, ///< Currently printing error message
} XSErrorState;


/**
 Provides a channel for reporting errors with a consistent format.

This object extends stream objects like cerr so that they report errors 
with a consistent format. Each error message starts with a header that 
typically reports the name of the program or method where the error 
occurred. The severity of the error is also indicated. If the severity 
is high enough, the object will terminate the entire program. Otherwise, 
the object will record the number of each type of error that occurred. 
Later, the calling program can query whether an error occurred and 
decide what action to take. 

This object also stores error messages and displays them in a Windows 
message window or as text in a command-line environment. 

\example
	XSError err(cerr);
	err.setHeader("XESS ");
	err.setSeverity(XSErrorMinor);
	err << "this is a minor error" << endl;
	err.endMsg();
	err.simpleMsg(XSErrorMinor,"this is another minor error");
\endexample
*/
class XSError
{
	public:

	XSError(ostream& s = cerr);

	void Setup(ostream& s);

	~XSError(void);

	XSError& operator=(XSError& src);

	unsigned int GetNumErrors(XSErrorSeverity s) const;

	void SetNumErrors(XSErrorSeverity s, unsigned int n);

	bool IsError(void) const;

	void SetSeverity(XSErrorSeverity s);

	void SetHeader(string& h);

	void EnableBatch(bool b);

	void EndMsg(void);

    void SimpleMsg(XSErrorSeverity s, string& msg);

    void SimpleMsg(XSErrorSeverity s, const char *msg);

	XSError& operator<<(long n);

	XSError& operator<<(float f);

	XSError& operator<<(char*  msg);

//	XSError& operator<<(string& msg);

	XSError& operator<<(string msg);

//	XSError& operator<<(ostream& s);


	private:

	XSErrorSeverity GetSeverity(void) const;

	void SetState(XSErrorState s );

	XSErrorState GetState(void) const;

	string& GetHeader(void);

	ostream *os;							///< output stream for error messages
	unsigned int numErrors[XSErrorMax];		///< counters for various grades of errors
	XSErrorState state;						///< records state of the error reporting process
	XSErrorSeverity severity;				///< severity of current error report
	string header;							///< header for each error message
	string storedMsg;						///< stored error message for display in window
	bool batch;								///< disables messages to user when true
};

#endif
