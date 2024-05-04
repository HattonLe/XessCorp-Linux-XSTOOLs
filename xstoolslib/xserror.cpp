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


#include <cassert>

#include "guitools.h"

#include "xserror.h"


/// Create an error object with output going to a stream.
XSError::XSError(ostream& s)
{
	Setup(s);
}


/// Initialize an error object.
void XSError::Setup(ostream& s)
{
	for(XSErrorSeverity i=XSErrorMin; i<XSErrorMax; i=(XSErrorSeverity)((int)i+1))
		SetNumErrors(i,0);
	os = &s;
	state = XSErrorInitial;
	severity = XSErrorNone;
	header = "";
	storedMsg = "";
	batch = false;
}


/// Destroy error object.
XSError::~XSError(void)
{
	;
}


/// Assign contents of one source error object to another.
///\return reference to the object whose contents were overwritten.
XSError& XSError::operator= (XSError& src)	///< overwrite the error object with the contents of this one
{
	for(XSErrorSeverity i=XSErrorMin; i<XSErrorMax; i=(XSErrorSeverity)((int)i+1))
		SetNumErrors(i,src.GetNumErrors(i));
	os = src.os;
	state = src.state;
	severity = src.severity;
	header = src.header;
	storedMsg = src.storedMsg;
	batch = src.batch;
	return *this;
}


/// Get number of errors of a certain severity that have occurred.
///\return the number of errors of severity s
unsigned int XSError::GetNumErrors(XSErrorSeverity s /**< severity */ ) const
{
	assert(s>=XSErrorMin && s<XSErrorMax);
	return numErrors[s];
}


/// Set number of errors of a certain type that have occurred.
void XSError::SetNumErrors(XSErrorSeverity s,	///< error severity level
					unsigned int n)		///< set number of errors of severity s to this
{
	assert(s>=XSErrorMin && s<XSErrorMax);
	numErrors[s] = n;
}


/// Returns true if any errors were recorded by this error object.
///\return true if errors were reported by this object, false otherwise
bool XSError::IsError(void) const
{
	for(XSErrorSeverity i=XSErrorMin; i<XSErrorMax; i=(XSErrorSeverity)((int)i+1))
		if(GetNumErrors(i)) return true; // yes, there were errors
	return false; // no errors were recorded
}


/// Return the severity of the current error message.
///\return severity of current error message
XSErrorSeverity XSError::GetSeverity(void) const
{
	return severity;
}


/// Set severity of next error message.
void XSError::SetSeverity(XSErrorSeverity s)	///< error severity level
{
	severity = s;
	switch(severity)
	{
	case XSErrorFatal:
        *os << GetHeader().c_str() << " FATAL: ";
		SetState(XSErrorInMessage);
		break;

	case XSErrorMajor:
        *os << GetHeader().c_str() << " MAJOR: ";
		os->flush();
		SetState(XSErrorInMessage);
		break;

	case XSErrorMinor:
        *os << GetHeader().c_str() << " MINOR: ";
		os->flush();
		SetState(XSErrorInMessage);
		break;

    case XSErrorDebug:
        *os << GetHeader().c_str() << " DEBUG: ";
        os->flush();
        SetState(XSErrorInMessage);
        break;

    case XSErrorNone:
        *os << GetHeader().c_str() << " INFO: ";
        os->flush();
        SetState(XSErrorInMessage);
        break;

	default:
		SetSeverity(XSErrorMinor);
		*os << "\nerror severity was incorrectly set!\n";
		os->flush();
		EndMsg();
		break;
	}
}


/// Get the current state of the error object.
///\return the current state of the error object
XSErrorState XSError::GetState(void) const
{
	return state;
}


/// Set the state of the error object
void XSError::SetState(XSErrorState s )	///< desired state of error object
{
	state = s;
}


/// Set header string for each error message.
void XSError::SetHeader(string& h)	///< string containing error message header
{
	header = h;
}


/// Get the current error message header.
///\return reference to error message header string
string& XSError::GetHeader(void)
{
	return header;
}


/// Enable batch processing (i.e., disable error messages).
void XSError::EnableBatch(bool b)	///< if true, messages are disabled for batch processing
{
	batch = b;	// disable user prompts (i.e. enable batch processing) when true
}


/// End the current error message and clean-up for the next one.
void XSError::EndMsg(void)
{
	cerr << storedMsg;
	switch(GetSeverity())
	{
	case XSErrorFatal:
		SetNumErrors(XSErrorFatal,GetNumErrors(XSErrorFatal)+1);
		*os <<("Abnormal termination of program\n");

        if(!batch) GuiTools::TellUser(storedMsg.c_str());

		exit(1);
		break;

	case XSErrorMajor:
		SetNumErrors(XSErrorMajor,GetNumErrors(XSErrorMajor)+1);

        if(!batch) GuiTools::TellUser(storedMsg.c_str());

		storedMsg = "";
		SetState(XSErrorInitial);
		break;

	case XSErrorMinor:
		SetNumErrors(XSErrorMinor,GetNumErrors(XSErrorMinor)+1);

        if(!batch) GuiTools::TellUser(storedMsg.c_str());

		storedMsg = "";
		SetState(XSErrorInitial);
		break;

    case XSErrorNone:
        SetNumErrors(XSErrorNone,GetNumErrors(XSErrorNone)+1);

        // Silent logging for something not normally shown to the user at runtime
        //if(!batch) GuiTools::TellUser(storedMsg.c_str());

        storedMsg = "";
        SetState(XSErrorInitial);
        break;

    case XSErrorDebug:
        SetNumErrors(XSErrorNone,GetNumErrors(XSErrorNone)+1);

        // Use this error mode for design\development debug logging.
        // Not normally displayed to the user at run time.
        //if(!batch) GuiTools::TellUser(storedMsg.c_str());

        storedMsg = "";
        SetState(XSErrorInitial);
        break;

	default:
		SetSeverity(XSErrorMinor);
		*os << "\nerror severity was not set!\n";
		os->flush();

        if(!batch) GuiTools::TellUser(storedMsg.c_str());

		storedMsg = "";
		SetState(XSErrorInitial);
		break;
	}
}


/// A simple, one-step method for sending an error message.
void XSError::SimpleMsg(XSErrorSeverity s,	///< severity of this error
                    string& msg)	///< error message
{
	SetSeverity(s);
    storedMsg = storedMsg + msg;
	EndMsg();
}


/// A simple, one-step method for sending an error message.
void XSError::SimpleMsg(XSErrorSeverity s,	///< severity of this error
                    const char* msg)	///< error message
{
    SetSeverity(s);
    storedMsg = storedMsg + (string)msg;
    EndMsg();
}


/// Overload << operator to concatenate an integer to an error message.
///\return reference to the error object
XSError& XSError::operator<<(long n)	///< integer to concatenate to error message
{
	char num[30];
	sprintf(num,"%ld",n);
	storedMsg = storedMsg + (string)num;
	return *this;
}


/// Overload << operator to concatenate a float to an error message.
///\return reference to the error object
XSError& XSError::operator<<(float f)	///< float to concatenate to error message
{
	char num[30];
	sprintf(num,"%f",f);
	storedMsg = storedMsg + (string)num;
	return *this;
}


/// Overload << operator to concatenate a character string to an error message.
///\return reference to the error object
XSError& XSError::operator<<(char* msg)	///< char. string to concatenate to error message
{
	storedMsg = storedMsg + (string)msg;
	return *this;
}

/*
/// Overload << operator to concatenate a string to an error message.
///\return reference to the error object
XSError& XSError::operator<<(string& msg)	///< string to concatenate to error message
{
	storedMsg = storedMsg + msg;
	return *this;
}
*/

/// Overload << operator to concatenate a string to an error message.
///\return reference to the error object
XSError& XSError::operator<<(string msg)	///< string to concatenate to error message
{
	storedMsg = storedMsg + msg;
	return *this;
}


/// Overload << operator to set the output channel for the error object.
///\return reference to the error object
//XSError& XSError::operator<<(ostream& s)	///< output stream where error messages will be directed
//{
//	return *this;
//}
