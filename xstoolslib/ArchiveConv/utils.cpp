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

	©1997-2010 - X Engineering Software Systems Corp.
----------------------------------------------------------------------------------*/

///\unit

///\file
/// Miscellaneous subroutines.


#include <cstdlib>
#include <cassert>
#include <ctime>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#ifdef WIN32
#include <afxole.h>
#include <shlobj.h>
#include <winreg.h>
#include <afxwin.h>
#include <afxcmn.h>
#endif

#include "Markup.h"
#include "usbjtag.h"
#include "utils.h"
#include "xserror.h"
#include "xsallbrds.h"
#include "usbcmd.h"



/// Convert ASCII '0'-'F' to number between 0 and 15, inclusive.
unsigned int CharToHex(char c)
{
	if(c>='0' && c<='9')
		return c-'0';
	c = toupper(c);
	return c-'A'+10;
}


/// Convert a string to upper case.
///\return Upper-case string
string& ConvertToUpperCase(string& s)
{
	int i;
	for(i=0; i<s.length(); i++)
		s[i] = toupper(s[i]);
	return s;
}


/// Strip suffix from a file name.
///\return String with suffix removed
string& StripSuffix(string& fileName)
{
	int i;
	
	assert(fileName.empty()==false);
	for(i=fileName.length()-1; i>=0; i-- )
	{
		if(fileName[i]=='.') // found the last period that delimits the suffix
		{
			fileName.erase(i,fileName.length()-i); // cut off the suffix
			break;
		}
		else if(fileName[i]=='/' || fileName[i]=='\\') // found the file/directory delimiter
			break;        //  so there is no suffix
	}
	return fileName;  // return the suffix-stripped file name
}


/// Get suffix from a file name.
///\return File suffix in upper case
string GetSuffix(string& fileName)
{
	assert(fileName.empty()==false);
	for(int i=fileName.length()-1; i>=0; i--)
	{
		if(fileName[i]=='.') // found the last period that delimits the suffix
		{
			string suffix(fileName.substr(i+1,fileName.length()-(i+1)));
			return ConvertToUpperCase(suffix);
		}
		else if(fileName[i]=='/' || fileName[i]=='\\') // found the file/directory delimiter
			break;
	}
	return string(""); // no suffix. return empty string
}


/// Strip prefixed directory path from a file name.
///\return String with leading path removed
string& StripPrefix(string& fileName)
{
	assert(fileName.empty()==false);
	for(int i=fileName.length()-1; i>=0; i--)
		if(fileName[i]=='/' || fileName[i]=='\\') // found the last file/directory delimiter
		{
			// return only the last part of the file name
			fileName = fileName.substr(i+1,fileName.length()-(i+1));
			return fileName;
		}
	return fileName;  // no prefix, so return the original file name
}


/// Get prefixed directory path from a file name.
///\return Leading directory path to file
string GetPrefix(string& fileName)
{
	int i;
	
	assert(fileName.empty()==false);
	for(i=fileName.length()-1; i>=0; i--)
		if(fileName[i]=='/' || fileName[i]=='\\') // found the last file/directory delimiter
			break;
	return fileName.substr(0,i);	// return the directory path without the file name
}


/// Compute an integer from a vector of hex numbers in an FPGA bitstream.
///\return the integer retrieved from the bitstream
long unsigned int GetInteger(istream& is, int len)
{
	assert(len<=4); // can't handle lengths greater than 32 bits
	long unsigned int lui = 0;
	for(int i=0; i<len; i++)
	{ // process hex digits starting from MSDigit
		unsigned char digit;
		is.read((char*)&digit,1);
		assert(is.eof()==0);
		lui = (lui<<8) | (long unsigned int)digit;
	}
	return lui;
}


/// Get the field type indicator from an FPGA bitstream.
static int GetType(istream& is)
{
	unsigned char type;
	is.read((char*)&type,1);
	assert(is.eof()==0);
	return type;
}


/// Pass over fields in an FPGA bitstream until a certain field is found.
///\return true if the field type identifier was found, false otherwise
bool ScanForField(istream& is,	///< stream whose contents will be searched
			unsigned char searchType)	///< field type identifier to be searched for in stream
{
	while(true)
	{
		unsigned char type = GetType(is);
		if(type==searchType)
			return true;
		unsigned int length = GetInteger(is);
		is.ignore(length);
		if(is.eof()!=0)
			break;
	}
	return false;
}


#pragma optimize( "", off ) // turn off optimization so timing loops don't get removed

/// Delay for a given number of microseconds or milliseconds.
void InsertDelay(unsigned long d, ///< number of microseconds or milliseconds to delay
				unsigned int time_units) ///< time units, either MICROSECONDS or MILLISECONDS
{
	static long loopsPerMillisecond = -1;
	
	if(loopsPerMillisecond < 0)
	{ // determine the timing of the computer if we haven't already done so
		long unsigned iterations = 10000000;
		clock_t start, finish;
		do
		{
		start = clock();	// get starting time
		assert(start>=0);
		for(long i = iterations; i>0L; i--)
			;	// do a whole bunch of empty loops
		finish = clock();	// get ending time
		assert(finish>0);
		iterations *= 2;	// double iterations for next try if needed
		} while ((finish-start)<10);	// keep going until we have enough precision
		// compute the number of empty loops per millisecond
		loopsPerMillisecond = ((iterations/(2*1000))*CLOCKS_PER_SEC)/(finish-start);
		assert(loopsPerMillisecond>1000);	// we need enough precision
		DEBUG_STMT("finish = " << finish << "   start = " << start)
		DEBUG_STMT("loops per ms = " << loopsPerMillisecond)
	}
	
	if(time_units==MICROSECONDS && d>10000)
	{
		time_units = MILLISECONDS;
		d /= 1000;
	}
	if(time_units == MILLISECONDS)
	{	// doing millisecond timing here
		long s = clock();
		assert(s>0);	// clock() returns -1 if there is an error
		long t = s + (d * CLOCKS_PER_SEC)/ 1000;
		if(t==s) t++;	// must wait at least 1 clock tick
		while(clock()<t)
			;
	}
	else if(time_units == MICROSECONDS)
	{	// doing microsecond timing here
		for(long loops = (loopsPerMillisecond*d)/1000; loops>0; loops--)
			;
	}
	else
		;
}

#pragma optimize( "", on )


static bool batch = false;	// stops prompts to user when true

/// Subroutine for enabling/disabling prompts to user.
void EnableBatch(bool b) ///< true if user prompts should be disabled for batch processing; false if user prompts are enabled
{
	batch = b;
}


/// Subroutine for prompting the user for a response.
///\return RESPONSE_CONTINUE if user selects OK; RESPONSE_CANCEL if user selects CANCEL
int PromptUser(string& msg, ///< prompt string displayed to user 
				int action) ///< allowable user actions
{
	if(batch)
		return RESPONSE_CONTINUE;

#ifdef _WINDOWS
	int response;
	switch(action)
	{
	case PROMPT_OKCANCEL:
		response = AfxMessageBox(msg.c_str(),MB_ICONINFORMATION | MB_OKCANCEL);
		return response==IDCANCEL ? RESPONSE_CANCEL : RESPONSE_CONTINUE;
	case PROMPT_OK:
	default:
		AfxMessageBox(msg.c_str(),MB_ICONINFORMATION);
		return RESPONSE_CONTINUE;
	}
#else
	cerr << msg.c_str();
	char c;
	switch(action)
	{
	case PROMPT_OKCANCEL:
		cerr << "\nContinue (Y/N)?...";
		c = toupper(getchar());
		fflush(stdin);
		cerr << "\n";
		if(c == 'Y')
			return RESPONSE_CONTINUE;
		else
			return RESPONSE_CANCEL;
	case PROMPT_OK:
	default:
		cerr << "\nPress any key to continue...";
		getchar();
		fflush(stdin);
		cerr << "\n";
		return RESPONSE_CONTINUE;
	}
#endif
}


/// Get the directory where the XSTOOLs store temporary data files.
///\return path to XSTOOLs data directory if found; NULL otherwise
const char* FindXSTOOLSDataDir(void)
{
	XSError err(cerr);

#ifdef _WINDOWS
	// check the registry
	HKEY key;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\XESS\\XSTOOLS",0,KEY_READ,&key) == ERROR_SUCCESS)
	{
		// get the installation path
		static char path[200];
		DWORD length, valType;
		length = 200;
		if(RegQueryValueEx(key,"DataDir",0,&valType,(unsigned char*)path,&length) == ERROR_SUCCESS)
		{
			assert(valType==REG_SZ);
			return path;
		}
	}
#endif
	// check the environment variable if the registry didn't have it
	char* p = getenv("XSTOOLS_DATA");
	if(p != NULL)
		return p;
	p = getenv("XSTOOLS_DATA_DIR");
	if(p != NULL)
		return p;
	string msg("Neither XSTOOLS_DATA or XSTOOLS_DATA_DIR environment variable is set!!\n");
	err.SimpleMsg(XSErrorFatal,msg);
	return NULL;
}


/// Get the directory where the XSTOOLs utilities are stored.
///\return path to XSTOOLs utilities if found; NULL otherwise
const char* FindXSTOOLSBinDir(void)
{
	XSError err(cerr);

#ifdef _WINDOWS
	// check the registry
	HKEY key;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\XESS\\XSTOOLS",0,KEY_READ,&key) == ERROR_SUCCESS)
	{
		// get the installation path
		static char path[200];
		DWORD length, valType;
		length = 200;
		if(RegQueryValueEx(key,"InstallDir",0,&valType,(unsigned char*)path,&length) == ERROR_SUCCESS)
		{
			assert(valType==REG_SZ);
			return path;
		}
	}
#endif
	// check the environment variable if the registry didn't have it
	char* p = getenv("XSTOOLS");
	if(p != NULL)
		return p;
	p = getenv("XSTOOLS_BIN_DIR");
	if(p != NULL)
		return p;
	string msg("Neither XSTOOLS or XSTOOLS_BIN_DIR environment variable is set!!\n");
	err.SimpleMsg(XSErrorFatal,msg);
	return NULL;
}


/// Set XSTOOLs parameter value.
///\return true if successful; false otherwise
bool SetXSTOOLSParameter(char *name, ///< name of parameter 
					const char *value)	///< value to be stored under parameter name
{
	string n(name), v(value);
	return SetXSTOOLSParameter(n,v);
}


/// Set XSTOOLs parameter value.
///\return true if successful; false otherwise
bool SetXSTOOLSParameter(string& name, ///< name of parameter 
					string& value)	///< value to be stored under parameter name
{
	XSError err(cerr);
	string msg;
	
#ifdef _WINDOWS
	// check the registry for the parameter value and set it if found
	HKEY appKey;
	DWORD keyDisp;
	
	if(RegCreateKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\XESS",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&appKey,&keyDisp) == ERROR_SUCCESS)
	{
		RegSetValueEx(appKey,name.c_str(),0,REG_SZ,(const unsigned char*)(value.c_str()),value.length());
		RegCloseKey(appKey);
	}
#endif
	// remove the parameter value from the XSPARAM file and replace it at the end of the file with a new value
	char* lines[1000];
	string XSTOOLSParameterFilename = (string)FindXSTOOLSBinDir() + (string)"/XSPARAM.TXT";
	FILE* fp = fopen(XSTOOLSParameterFilename.c_str(),"r");
	int numLines=0;
	if(fp != NULL)
	{
		char line[512], val[512];
		for(int i=0; fgets(line,511,fp) != NULL; i++)
		{
			char nm[512];
			int n = sscanf(line,"%s %s",nm,val);
			if(n==0 || n==EOF)
				continue;	// skip blank lines
			if(n != 2)
			{
				msg = (string)"Corrupted record encountered while reading XSTOOLS parameter file (" + XSTOOLSParameterFilename + (string)"):\n";
				msg += (string)line;
				err.SimpleMsg(XSErrorMajor,msg);
				fclose(fp);
				return false;
			}
			if((string)nm != name)  // store lines that don't match line to be replaced
				lines[numLines++] = strdup(line);
		}
		fclose(fp);
	}
	fp = fopen(XSTOOLSParameterFilename.c_str(),"w");
	if(fp == NULL)
	{
		msg = "XSTOOLS parameter file (" + XSTOOLSParameterFilename + (string)") could not be opened!!\n";
		err.SimpleMsg(XSErrorMajor,msg);
		return false;
	}
	for(int j=0; j<numLines; j++)
	{
		fprintf(fp,"%s",lines[j]);
		free(lines[j]);
	}
	fprintf(fp,"%s %s\n",name.c_str(),value.c_str());
	fclose(fp);
	return true;
}


/// Get XSTOOLs parameter value from registry or file in XSTOOLs directory.
///\return value of the parameter
string GetXSTOOLSParameter(char *name) ///< name of parameter whose value is returned
{
	string n(name);
	return GetXSTOOLSParameter(n);
}


/// Get XSTOOLs parameter value from registry or file in XSTOOLs directory.
///\return value of the parameter
string GetXSTOOLSParameter(string& name) ///< name of parameter whose value is returned
{
	XSError err(cerr);
	string msg;
	char fileValString[512];
	char regValString[512];
	DWORD regValLength = 0;
	
#ifdef _WINDOWS
	// check the registry for the parameter
	HKEY appKey;
	DWORD keyDisp;
	DWORD valType;
	if(RegCreateKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\XESS",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&appKey,&keyDisp) == ERROR_SUCCESS)
	{
		regValLength = 512;
		if(RegQueryValueEx(appKey,name.c_str(),0,&valType,(unsigned char*)regValString,&regValLength) == ERROR_SUCCESS)
		{
			assert(valType==REG_SZ);
			regValString[regValLength] = 0;	// terminate string read from registry
		}
		else
			regValLength = 0;
		RegCloseKey(appKey);
	}
#endif
	// check the XS parameter file
	fileValString[0] = 0;	// start off by clearing the value string from the XS parameter file
	string XSTOOLSParameterFilename = (string)FindXSTOOLSBinDir() + (string)"/XSPARAM.TXT";
	FILE* fp = fopen(XSTOOLSParameterFilename.c_str(),"r");
	if(fp == NULL)
	{
		msg = "XSTOOLS parameter file (" + XSTOOLSParameterFilename + (string)") could not be opened!!\n";
		err.SimpleMsg(XSErrorMajor,msg);
	}
	else
	{
		char line[512];
		while(fgets(line,511,fp) != NULL)
		{
			// got a line of text.  now see if it starts with the name we are searching for...
			char nm[512];
			int n = sscanf(line,"%s %s",nm,fileValString);
			if(n==0 || n==EOF)
				continue;	// skip blank lines
			if(n != 2)
			{
				msg = (string)"Corrupted record encountered while reading XSTOOLS parameter file (" + XSTOOLSParameterFilename + (string)"):\n";
				msg += (string)line;
				err.SimpleMsg(XSErrorMajor,msg);
				break;
			}
			if((string)nm == name)
				break;	// found the parameter name.  now return the parameter value...
			// otherwise keep searching through the parameter text file...
			fileValString[0] = 0; // clear any crap from the fileValString
		}
		fclose(fp);
	}
	if(regValLength==0) return fileValString;	// return value from file if registry value is null
	else return regValString;					// else return value from registry
}


/// Get the information about the XS Boards from an XML file
///\return the number of boards in the array
int GetXSBoardInfo(XSBoardInfo** bInfo) ///< returns with a pointer to the array of XESS board information
{
	XSError err(cerr);
	static int numBoards=0;
	static XSBoardInfo* brdInfo=NULL;

	// return board info already read from file on a previous call to this subroutine
	if(numBoards > 0)
	{
		*bInfo = brdInfo;
		return numBoards;
	}

	// otherwise, read board info from XML file and store it in data structure
	string msg;
	string XSBoardInfoFilename = (string)FindXSTOOLSBinDir() + (string)"/XSBRDINF.xml";
	FILE* fp = fopen(XSBoardInfoFilename.c_str(),"r");
	if(fp == NULL)
	{
		msg = "XS Board information file (" + XSBoardInfoFilename + (string)") could not be opened!!\n";
		err.SimpleMsg(XSErrorMajor,msg);
		brdInfo = NULL;
		numBoards = 0;
		*bInfo = brdInfo;
		return numBoards;
	}

	string xmlString;
	char line[512];
	while(!feof(fp))
	{
		if(fgets(line,512,fp) == NULL)
		{
			if(feof(fp))
				break;
			msg = (string)"File error encountered while reading XS Board information file (" + XSBoardInfoFilename + (string)")\n";
			err.SimpleMsg(XSErrorMajor,msg);
			numBoards = 0;
			if(brdInfo!=NULL)
				free(brdInfo);
			brdInfo = NULL;
			fclose(fp);
			*bInfo = brdInfo;
			return numBoards;
		}
		xmlString = xmlString + line;
	}
	fclose(fp);

	char* port_id[PORTTYPE_END] = {"invalid", "lpt", "usb", "lptjtag", "usbjtag"};

	CMarkup xml;
	bool status;
	xml.SetDoc(xmlString.c_str());
	status = xml.FindChildElem("boards");
	assert(status==true);
	xml.IntoElem(); // boards
	while(xml.FindChildElem("board"))
	{
		xml.IntoElem(); // board

		if((brdInfo = (XSBoardInfo*)realloc(brdInfo, (numBoards+1) * sizeof(XSBoardInfo))) == NULL)
		{
			msg = (string)"Ran out of memory while reading records from XS Board information file (" + XSBoardInfoFilename + (string)")\n";
			err.SimpleMsg(XSErrorMajor,msg);
			numBoards = 0;
			if(brdInfo!=NULL)
				free(brdInfo);
			brdInfo = NULL;
			fclose(fp);
			*bInfo = brdInfo;
			return numBoards;
		}

		status = xml.FindChildElem("model");
		assert(status==true);
		brdInfo[numBoards].brdModel = strdup(xml.GetChildData());
		xml.ResetChildPos();

		status = xml.FindChildElem("chip_type");
		assert(status==true);
		brdInfo[numBoards].chipType = strdup(xml.GetChildData());
		xml.ResetChildPos();

		int port_index;
		for(port_index=0; port_index<sizeof(port_id)/sizeof(char*); port_index++)
		{
			if(!xml.FindChildElem(port_id[port_index]))
			{
				brdInfo[numBoards].port[port_index].allowed = false;
				continue;
			}
	
			brdInfo[numBoards].port[port_index].allowed = true;
			xml.IntoElem();	// port

			status = xml.FindChildElem("root");
			assert(status==true);
			const char* xstoolsBinDir = FindXSTOOLSBinDir();
			string root = string(xstoolsBinDir) + string("/") + string(xml.GetChildData());
			xml.ResetChildPos();

			status = xml.FindChildElem("inv_mask");
			assert(status==true);
			sscanf(xml.GetChildData(),"%x",&(brdInfo[numBoards].port[port_index].invMask));
			xml.ResetChildPos();

			status = xml.FindChildElem("default_interface");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].dwnldIntfcBitstreamFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].dwnldIntfcBitstreamFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("ram_interface");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].ramBitstreamFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].ramBitstreamFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("osc_interface");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].oscBitstreamFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].oscBitstreamFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("programmable_osc");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
			{
				if(ConvertToUpperCase((string)xml.GetChildData()) == "YES")
					brdInfo[numBoards].port[port_index].programmableOsc = true;
				else
					brdInfo[numBoards].port[port_index].programmableOsc = false;
			}
			else
				brdInfo[numBoards].port[port_index].programmableOsc = false;
			xml.ResetChildPos();

			status = xml.FindChildElem("erase_interface");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].eraseBitstreamFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].eraseBitstreamFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("flash_interface");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].flashBitstreamFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].flashBitstreamFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("flash_config_interface");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].flashConfigBitstreamFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].flashConfigBitstreamFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("test_interface");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].testIntfcBitstreamFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].testIntfcBitstreamFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("test_bitstream");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].testBitstreamFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].testBitstreamFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("test_object_file");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].testObjFile = strdup((root + (string)xml.GetChildData()).c_str());
			else
				brdInfo[numBoards].port[port_index].testObjFile = 0;
			xml.ResetChildPos();

			status = xml.FindChildElem("user_instruction");
			assert(status==true);
			if(strlen(xml.GetChildData())!=0)
				brdInfo[numBoards].port[port_index].userInstruction = strdup(xml.GetChildData());
			else
				brdInfo[numBoards].port[port_index].userInstruction = 0;
			xml.ResetChildPos();

			xml.OutOfElem();	// port
		}

		xml.OutOfElem(); // board

		numBoards++;
	}

	fclose(fp);
	*bInfo = brdInfo;
	return numBoards;
}


/// Return a pointer to a new object created for a given XS Board model.  
/// \return NULL if not successful; pointer to board object otherwise.
XSBoard* NewXSBoard(const char* boardModelName, ///< model name of XESS board
					PortType portType) ///< type of port connected to board
{
	XSBoard* brdPtr;

	string ucBoardModelName = ConvertToUpperCase(string(boardModelName));
	
	if(strncmp(ucBoardModelName.c_str(),"XS95",strlen("XS95"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = new XS95Board();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XS40",strlen("XS40"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = new XS40Board();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XSP",strlen("XSP"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = new XS40Board();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XSA-3S",strlen("XSA-3S"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = new XSA3SBoard();
		else if(portType == PORTTYPE_USBJTAG)
			brdPtr = new XSJTAGBoard();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XULA",strlen("XULA"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = NULL;
		else if(portType == PORTTYPE_USBJTAG)
			brdPtr = new XuLABoard();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XSA-200",strlen("XSA-200"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = new XSA200Board();
		else if(portType == PORTTYPE_USBJTAG)
			brdPtr = new XSJTAGBoard();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XSA",strlen("XSA"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = new XSABoard();
		else if(portType == PORTTYPE_USBJTAG)
			brdPtr = new XSJTAGBoard();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XSB",strlen("XSB"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = new XSBBoard();
		else if(portType == PORTTYPE_USBJTAG)
			brdPtr = new XSJTAGBoard();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XSC",strlen("XSC"))==0)
	{
		// No LPT port for XSC Boards!!
		if(portType == PORTTYPE_USBJTAG)
			brdPtr = new XSCBoard();
		else
			brdPtr = NULL;
	}
	else if(strncmp(ucBoardModelName.c_str(),"XSV",strlen("XSV"))==0)
	{
		if(portType == PORTTYPE_LPT)
			brdPtr = new XSVBoard();
		else
			brdPtr = NULL;
	}
	else
		brdPtr = NULL;

	return brdPtr;
}


/// Table used to reverse bits within a byte.  reverseBits[d] = d with bits in reverse order.
const unsigned char reverseByteBits[] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0, 
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8, 
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4, 
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc, 
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2, 
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa, 
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6, 
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1, 
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9, 
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5, 
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd, 
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3, 
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb, 
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7, 
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff, 
};

/// Combine hex data bytes to form a word of data depending upon big/little-byte/bit ordering.
///\return the hex data as an unsigned int
unsigned int Hex2Data(HexRecord& hx, ///< hex record containing hex data bytes 
				int start, ///< index of starting position within the hex record
				int numBytes, ///< number of bytes in hex representation for data
				bool bigEndianBytes, ///< true if data should be stored with most-significant byte at lower address
				bool bigEndianBits) ///< true if data should be stored with most-significant bit in bit position 0 (right-most)
{
	// If translating big-endian hex into data, start at lower address and work toward higher address.
	// Do the opposite if hex is stored in little-endian byte order.
	int j     = bigEndianBytes ? 0        : numBytes-1; // starting byte position within hex record
	int j_end = bigEndianBytes ? numBytes : -1; // ending position within hex record
	int j_inc = bigEndianBytes ? 1        : -1; // direction from start to end
	unsigned int data = 0;
	for( ; j!=j_end; j+=j_inc)
	{
		// process hex from most-significant byte to least-significant byte
		unsigned int hx_data = hx[start+j] & 0xFF;
		// reverse the bits within each hex byte if it is in big-endian bit order
		if(bigEndianBits)
			hx_data = reverseByteBits[hx_data];
		data = (data<<8) | hx_data;
	}
	return data;
}


/// Split-apart a word of data into hex data bytes depending upon big/little-byte/bit ordering.
void Data2Hex(unsigned int data,  ///< data to be rearranged (assumes bytes are stored big-endian and bits are stored little-endian)
				HexRecord& hx, ///< hex record to store hex representation of data
				int start, ///< index of starting position within the hex record
				int numBytes, ///< number of bytes in hex representation for data
				bool bigEndianBytes,  ///< true if data should be stored with most-significant byte at lower address
				bool bigEndianBits) ///< true if data should be stored with most-significant bit in bit position 0 (right-most)
{
	// If translating data into hex with big-endian byte order, start at higher address and work toward lower address.
	// Do the opposite if hex is stored in little-endian byte order.
	int j     = bigEndianBytes ? numBytes-1 : 0; // starting byte position within hex record
	int j_end = bigEndianBytes ? -1         : numBytes; // ending position within hex record
	int j_inc = bigEndianBytes ? -1         : 1; // direction from start to end
	for( ; j!=j_end; j+=j_inc)
	{
		// process data from least-significant byte to most-significant byte and store into hex record
		unsigned int hx_data = data & 0xFF;
		data >>= 8;
		// reverse the bits within each byte if the data is to be stored in big-endian bit order
		if(bigEndianBits)
			hx_data = reverseByteBits[hx_data];
		hx[start+j] = hx_data;
	}
}

/// Rearrange data word depending upon big/little-byte/bit ordering.
///\return the rearranged data
unsigned int RearrangeData(unsigned int data, ///< data to be rearranged (assumes bytes are stored big-endian and bits are stored little-endian)
				unsigned int numBits, ///< width of data
				bool bigEndianBytes, ///< true if data should be stored with most-significant byte at lower address
				bool bigEndianBits) ///< true if data should be stored with most-significant bit in bit position 0 (right-most)
{
	unsigned int numBytes = (numBits+7) / 8;	// 0 bits=>0 bytes; 1..8 bits=>1 byte; 9..17 bits=>2 bytes...
	HexRecord hx;
	hx.SetLength(numBytes);
	Data2Hex(data,hx,0,numBytes,BIG_ENDIAN_BYTES,LITTLE_ENDIAN_BITS); // convert data to hex
	return Hex2Data(hx,0,numBytes,bigEndianBytes,bigEndianBits); // convert back into data with the desired bit and byte order
}


#ifdef _WINDOWS
/// Populate the list of active parallel and USB ports.
void SetPortList(CComboBox *cb) ///< combo-box list to hold the active port names
{
	cb->ResetContent();	// remove all the ports from the list

	// setup the list of parallel ports in the pulldown list
	XSError err(cerr);
	bool portNumExists[5];
	int numLPTPorts = ScanPPort(&err,portNumExists);
	for(int i=1; i<sizeof(portNumExists)/sizeof(bool); i++)
	{
		char s[7];
		if(portNumExists[i])
		{
			sprintf(s,"LPT%d",i);
			cb->AddString(s);
		}
	}

	// setup the list of USB ports in the pulldown list
	int numUSBJTAGPorts = ScanUSBJTAG(&err,10);
	for(i=0; i<numUSBJTAGPorts; i++)
	{
		char s[7];
		sprintf(s,"USB%d",i);
		cb->AddString(s);
	}

	if(cb->GetCount()==0)
		return;	// no ports in the list!

	// get the last setting for the port
	int portNum = -1;
	string portName = GetXSTOOLSParameter("PORT");
	if(portName == "")
		portName = GetXSTOOLSParameter("LPT"); // look for old-style LPT parameter if PORT parameter is empty

	// select the stored port if it is in the list...
	if(cb->SelectString(-1,(const char*)(portName.c_str())) == CB_ERR)
		cb->SetCurSel(0); // otherwise, select the first item in the list
}
#endif

#ifdef _WINDOWS
/// Populate the list of XS boards.
void SetBoardList(CComboBox *cb) ///< combo-box list to hold the board model names
{
	cb->ResetContent();	// remove all the boards from the list

	// read-in the board information
	XSBoardInfo* brdInfo;
	int numBoards;
	numBoards = GetXSBoardInfo(&brdInfo);

	// populate the list with the board names
	for(int i=0; i<numBoards; i++)
		cb->AddString(brdInfo[i].brdModel);

	// get the last setting for the board type
	string brdType;
	if((brdType=GetXSTOOLSParameter((string)"BoardType")) == "")
	{ // set the default value if no previous value exists
		if(SetXSTOOLSParameter((string)"BoardType",(string)brdInfo[0].brdModel) == false)
		{
			AfxMessageBox("BoardType value was not set",MB_ICONSTOP);
			exit(0);
		}
		brdType = GetXSTOOLSParameter((string)"BoardType");
		assert(brdType != "");
	}
	cb->SelectString(-1,(const char*)(brdType.c_str()));
}
#endif

#ifdef _WINDOWS
/// Return the port type and index number for the selected port in the list.
bool GetPortTypeAndNumber(CComboBox *cb,		///< combo-box list that holds the port names
						  PortType *portType,	///< return the type of port (USB or LPT) here
						  int *portNum)			///< return the port index here
{
	int curSel;
	if((curSel=cb->GetCurSel()) == CB_ERR)
	{
		*portType = PORTTYPE_INVALID;
		*portNum = 0;
		return false;	// failure
	}

	CString prtName;
	cb->GetLBText(curSel,prtName);
	string portName = (string)LPCTSTR(prtName);

	if(portName.substr(0,3) == "LPT")
	{
		int n = sscanf(portName.c_str()+strlen("LPT"),"%d",portNum);
		assert(n != 0);
		*portType = PORTTYPE_LPT;
	}
	else if(portName.substr(0,3) == "USB")
	{
		// refresh the pointers to USB ports in case a
		// USB board has been unplugged and then re-plugged
		XSError err(cerr);
		int numUSBJTAGPorts = ScanUSBJTAG(&err);
		int n = sscanf(portName.c_str()+strlen("USB"),"%d",portNum);
		assert(n != 0);
		*portType = PORTTYPE_USBJTAG;
	}
	else
	{
		*portType = PORTTYPE_INVALID;
		*portNum = 0;
		return false;	// failure
	}
	return true;	// success
}
#endif


/// Indicate if a given capability is present.
/// This subroutine checks the capability bitstream returned by the XESS Board to see if the configuration supports a given capability.
///\return true if the XESS board has the capability, false otherwise
bool hasCapability(Bitstream& b, ///< bitstream containing the capability indicator bits
                   unsigned int bit_index) ///< index of capability bit to check
{
	unsigned char *capabilities = b.ToCharString();
	if(capabilities[0] == 0xA5 && capabilities[3] == 0xA5) // upper and lower bytes must match this pattern
	{
		unsigned int cap = (capabilities[3]<<24) | (capabilities[2]<<16) | (capabilities[1]<<8) | capabilities[0];
		if(cap & (1<<bit_index))
		{
			delete [] capabilities;
			return true;
		}
	}
	delete [] capabilities;
	return false;
}


/// Scan LPT and USB ports for XESS Boards.
///\return the number of XESS Boards found
int ScanPorts(XSError *err)  ///< pointer to error reporting object
{
	int numActiveLPTJTAGPorts = ScanLPTJTAG(err);
	int numActiveUSBJTAGPorts = ScanUSBJTAG(err);
	return numActiveLPTJTAGPorts + numActiveUSBJTAGPorts;
}


/// Get a pointer to an active LPT or USB JTAG port object.
///\return a pointer to the JTAG port object
JTAGPort* GetPort(PortType type, ///< type of port, either LPTJTAG or USBJTAG
				int portNum, ///< port index between 0 and # of active ports
				int endptNum) ///< endpoint: 1 for primary endpoint, 2 for secondary endpoint
{
	switch(type)
	{
	case PORTTYPE_LPTJTAG: return GetLPTJTAG(portNum,endptNum);
	case PORTTYPE_USBJTAG: return GetUSBJTAG(portNum,endptNum);
	default:  return NULL;
	}
	return NULL;
}


/// Convert the USERCODE stored in the CPLD into its canonical form.
///\return a pointer to the converted string
char* ConvertUSERCODE(char* code)
{
	// Reverse the order of the four characters in the string.
	char tmp = code[0];
	code[0] = code[3];
	code[3] = tmp;
	tmp = code[1];
	code[1] = code[2];
	code[2] = tmp;
	// convert any (), [], {} or AB character pairs into <>
	if( (code[0]=='(' && code[2]==')') ||
		(code[0]=='[' && code[2]==']') ||
		(code[0]=='{' && code[2]=='}') ||
		(code[0]=='A' && code[2]=='B') )
	{
		code[0] = '<';
		code[2] = '>';
	}
	code[3] = '!'; // force the last character to an exclamation point
	return code;
}


/// Read a byte value from the EEPROM of the XESS board's microcontroller.
///\return a byte value from the EEPROM
unsigned int ReadEeprom(USBPort *usbPort,	///< pointer to USB port object that communicates with XSUSB
				 unsigned int address) ///< EEPROM address to be written
{
	unsigned char txBuf[64], rxBuf[64];	// Buffers for data to and from the board.
	unsigned long txLen, rxLen;			// Amount of data in each buffer.

	txBuf[0] = READ_EEDATA_CMD;
	txBuf[1] = 1;
	txBuf[2] = address & 0xff;
	txBuf[3] = (address>>8) & 0xff;
	txBuf[4] = (address>>16) & 0xff;
	txLen = 5;
	rxLen = 1+5;
	if(usbPort->SendRcvPacket(txBuf,txLen,rxBuf,&rxLen,true) != USB_SUCCESS)
	{
		return 0;
	}
	return rxBuf[5];
}


/// Write a byte value to the EEPROM of the XESS board's microcontroller.
///\return true if the operation succeeded, false otherwise
bool WriteEeprom(USBPort *usbPort,	///< pointer to USB port object that communicates with XSUSB
				 unsigned int address, ///< EEPROM address to be written
				 unsigned int data) ///< data to write to EEPROM
{
	unsigned char txBuf[64], rxBuf[64];	// Buffers for data to and from the board.
	unsigned long txLen, rxLen;			// Amount of data in each buffer.

	txBuf[0] = WRITE_EEDATA_CMD;
	txBuf[1] = 1;
	txBuf[2] = address & 0xff;
	txBuf[3] = (address>>8) & 0xff;
	txBuf[4] = (address>>16) & 0xff;
	txBuf[5] = data & 0xff;
	txLen = 6;
	rxLen = 1;
	if(usbPort->SendRcvPacket(txBuf,txLen,rxBuf,&rxLen,true) != USB_SUCCESS)
	{
		return false;
	}
	return true;
}

