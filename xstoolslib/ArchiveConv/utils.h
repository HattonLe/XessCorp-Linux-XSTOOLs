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

#ifndef UTILS_H
#define UTILS_H

#include <string>
using namespace std;

#include "hexrecrd.h"
#include "usbport.h"
#include "xsboard.h"

const unsigned int MILLISECONDS = 1000;
const unsigned int MICROSECONDS = 1;
const unsigned int NANOSECONDS  = 0;

enum{PROMPT_OK,PROMPT_OKCANCEL};
enum{RESPONSE_CONTINUE,RESPONSE_CANCEL};

extern const unsigned char reverseByteBits[];

unsigned int CharToHex(char c);
string& ConvertToUpperCase(string& s);
string& StripSuffix(string& fileName);
string GetSuffix(string& fileName);
string& StripPrefix(string& fileName);
string GetPrefix(string& fileName);
long unsigned int GetInteger(istream& is, int len=2);
bool ScanForField(istream& is, unsigned char searchType);
void InsertDelay(unsigned long d, unsigned int time_units);
void EnableBatch(bool b);
int PromptUser(string& msg, int action);
const char* FindXSTOOLSDataDir(void);
const char* FindXSTOOLSBinDir(void);
bool SetXSTOOLSParameter(char *name, const char *value);
bool SetXSTOOLSParameter(string& name, string& value);
string GetXSTOOLSParameter(char *name);
string GetXSTOOLSParameter(string& name);
int GetXSBoardInfo(XSBoardInfo** bInfo);
XSBoard* NewXSBoard(const char* boardModelName, PortType portType);
unsigned int Hex2Data(HexRecord& hx, int start, int numBytes, bool bigEndianBytes, bool bigEndianBits);
void Data2Hex(unsigned int data, HexRecord& hx, int start, int numBytes, bool bigEndianBytes, bool bigEndianBits);
unsigned int RearrangeData(unsigned int data, unsigned int numBits, bool bigEndianBytes, bool bigEndianBits);
#ifdef _WINDOWS
void SetPortList(CComboBox *cb);
void SetBoardList(CComboBox *cb);
bool GetPortTypeAndNumber(CComboBox *cb, PortType *portType, int *portNum);
#endif
bool hasCapability(Bitstream& b, unsigned int bit_index);
int ScanPorts(XSError *err);
JTAGPort* GetPort(PortType type, int portNum, int endptNum);
char* ConvertUSERCODE(char* code);
unsigned int ReadEeprom(USBPort *usbPort, unsigned int address);
bool WriteEeprom(USBPort *usbPort, unsigned int address, unsigned int data);

#ifdef _DEBUG
#define DEBUG_STMT(stmt)	{cout << StripPrefix((string)__FILE__) << "(" << __LINE__ << "): " << stmt << "\n";}
#else
#define DEBUG_STMT(stmt)
#endif

#endif
