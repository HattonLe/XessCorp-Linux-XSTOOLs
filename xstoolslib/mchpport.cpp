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

//
// MchpPort objects add Microchip ICSP capabilities to a parallel port object.
// A Microchip port can control the MCLR, PGM, PGC and PGD pins.
// 


#include <string>
#include <fstream>
#include <cassert>
#include <ctime>
#include <cstdarg>
using namespace std;

#include "utils.h"
#include "mchpport.h"

#define CMD_LENGTH		4
#define OPERAND_LENGTH	16
#define RESULT_LENGTH	8

enum
{
	CORE_INSTRUCTION			= 0x0,
	SHIFTOUT_TABLAT				= 0x2,
	TABLE_READ					= 0x8,
	TABLE_READ_POSTINC			= 0x9,
	TABLE_READ_POSTDEC			= 0xa,
	TABLE_READ_PREINC			= 0xb,
	TABLE_WRITE					= 0xc,
	TABLE_WRITE_POSTINC2		= 0xd,
	START_PROGRAMMING_POSTINC2	= 0xe,
	START_PROGRAMMING			= 0xf
};

#define TIME_P5		40,NANOSECONDS
#define TIME_P5A	40,NANOSECONDS
#define TIME_P9		1,MILLISECONDS
#define TIME_P10	100,MICROSECONDS
#define TIME_P11	5,MILLISECONDS
//#define TIME_P12	2,MICROSECONDS
//#define TIME_P15	2,MICROSECONDS
#define TIME_P12	10000,MICROSECONDS
#define TIME_P15	10000,MICROSECONDS
//#define TIME_P16	0,MICROSECONDS
//#define TIME_P18	0,MICROSECONDS
#define TIME_P16	10000,MICROSECONDS
#define TIME_P18	10000,MICROSECONDS

#define WRITE_BUFFER_SIZE	32

#define CONFIGBITS_ADDRESS	0x300000

static const unsigned int DEFAULT_INVMASK = 0x0B8000; // invert C0, C1, C2 and S7

// These store the bit positions of the Microchip programming signals
#if 1
const unsigned int posMCLR_N	= 16; // position of MCLR_N (C0)
const unsigned int posPGM		= 17; // position of PGM (C1)
const unsigned int posPGC		= 6;  // position of PGC (D6)
const unsigned int posPGD_O		= 2;  // position of PGD output (D2)
const unsigned int posPGD_I		= 11; // position of PGD input (S3)
const unsigned int inversionMask      = DEFAULT_INVMASK;
#else
const unsigned int posMCLR_N	= 5; // position of MCLR_N (D5)
const unsigned int posPGM		= 4; // position of PGM (D4)
const unsigned int posPGC		= 3;  // position of PGC (D3)
const unsigned int posPGD_O		= 2;  // position of PGD output (D2)
const unsigned int posPGD_I		= 11; // position of PGD input (S3)
const unsigned int inversionMask      = DEFAULT_INVMASK ^ 0x083C;
#endif


// begin methods and functions


// Create a JTAG controller port
MchpPort::MchpPort(void)
{
	;
}


// Create a parallel port-based JTAG controller port
MchpPort::MchpPort(
					XSError* e,	// pointer to error reporting object
					unsigned int portNum	// parallel port number
				   )
				   : PPort(e,portNum,inversionMask)
{
	Setup(e,portNum);
}


// Setup the parallel port-based JTAG controller port
bool MchpPort::Setup(
					XSError* e,	// pointer to error reporting object
					unsigned int portNum	// parallel port number
				 )
{
	SetTraceOnOff(false,cerr);	// don't trace TAP signals
	progressGauge = NULL;		// no gauge to show progress of operations at this time
	return PPort::Setup(e,portNum,inversionMask);	// return non-zero if error occurs
}


// enables/disables trace
void MchpPort::SetTraceOnOff(
					bool f,		// trace if true, no trace if false
					ostream& os	// trace info goes to this stream
					)
{
	traceFlag = f;
	osTrace = &os;  // trace messages go to this output stream
}


// Set the value of the MCLR_N bit
void MchpPort::SetMCLR_N(unsigned int b)
{
	mclr_nVal = b;
	Out(b,posMCLR_N,posMCLR_N);
}

// Get the value output on the MCLR_N bit
unsigned int MchpPort::GetMCLR_N(void)
{
	mclr_nVal = In(posMCLR_N,posMCLR_N);
	return mclr_nVal;
}


// Set the value of the PGM bit
void MchpPort::SetPGM(unsigned int b)
{
	pgmVal = b;
	Out(b,posPGM,posPGM);
}

// Get the value output on the PGM bit
unsigned int MchpPort::GetPGM(void)
{
	pgmVal = In(posPGM,posPGM);
	return pgmVal;
}


// Set the value of the PGC bit
void MchpPort::SetPGC(unsigned int b)
{
	pgcVal = b;
	Out(b,posPGC,posPGC);
//	printf("%d %d %d\n",pgcVal,pgd_oVal,pgd_iVal);
}

// Get the value output on the PGC bit
unsigned int MchpPort::GetPGC(void)
{
	pgcVal = In(posPGC,posPGC);
	return pgcVal;
}


// Set the value of the PGD bit
void MchpPort::SetPGD(unsigned int b)
{
	pgd_oVal = b;
	Out(b,posPGD_O,posPGD_O);
}

// Get the value on the PGD bit
unsigned int MchpPort::GetPGD(void)
{
	pgd_iVal = In(posPGD_I,posPGD_I);
	return pgd_iVal;
}


// Enter the Microchip in-circuit programming state
void MchpPort::EnterICSP(void)
{
	// Set all the ICSP inputs to 0
	SetPGC(0);
	SetPGD(0);
	SetMCLR_N(0);
	SetPGM(0);

	// Raise PGM and then MCLR_N to start ICSP
	SetPGM(1);
	InsertDelay(TIME_P15);
	SetMCLR_N(1);
	InsertDelay(TIME_P12);
//	InsertDelay(200,MILLISECONDS);
}


// Exit the Microchip in-circuit programming state
void MchpPort::ExitICSP(void)
{
	SetPGC(0);		// lower the PGC and PGD pins
	SetPGD(0);
	InsertDelay(TIME_P16);
	SetMCLR_N(0);	// lower the MCLR_N pin
	InsertDelay(TIME_P18);
//	InsertDelay(200,MILLISECONDS);
	SetPGM(0);		// lower the PGM pin to exit ICSP
}


// Output a command+operand to the Microchip device and return the 8-bit result
unsigned int MchpPort::ICSPIO(
				unsigned int cmd,
				unsigned int operand
				)
{
	assert(mclr_nVal==1);
	assert(pgmVal==1);
	assert(pgcVal==0);
	assert(pgd_oVal==0);

//	printf("ICSPIO: %01x %04x\n",cmd,operand);

	int bitCnt;
	for(bitCnt=0; bitCnt<CMD_LENGTH; bitCnt++)
	{
		SetPGD(cmd & 1);
		SetPGC(1);
		SetPGC(0);
		cmd >>= 1;
	}
	InsertDelay(TIME_P5);

	unsigned int result = 0;
	for(bitCnt=0; bitCnt<OPERAND_LENGTH; bitCnt++)
	{
		SetPGD(operand & 1);
		SetPGC(1);
		SetPGC(0);
		// only record the final 8 bits output from the Microchip device
		result = (GetPGD() ? (1<<(RESULT_LENGTH-1)):0) | ((result>>1) & ((1<<(RESULT_LENGTH-1))-1));
		operand >>= 1;
	}
	SetPGD(0);
	InsertDelay(TIME_P5A);

	return result;
}


// Output NOP instruction but hold PGC high for programming to complete
void MchpPort::WaitWhileProgramming(void)
{
	assert(mclr_nVal==1);
	assert(pgmVal==1);
	assert(pgcVal==0);
	assert(pgd_oVal==0);

//	printf("WaitWhileProgramming:\n");

	SetPGD(0); // PGD is 0 for all four command bits so as to issue a CORE_INSTRUCTION command
	SetPGC(1); // first command bit
	SetPGC(0);
	SetPGC(1); // second command bit
	SetPGC(0);
	SetPGC(1); // third command bit
	SetPGC(0);
	SetPGC(1); // hold PGC high on 4th command bit while programming occurs
	InsertDelay(TIME_P9);
	SetPGC(0); // lower PGC after programming is done
	InsertDelay(TIME_P10); // let memory array discharge after programming

	// output NOP instruction (0x0000) to the core
	for(int bitCnt=0; bitCnt<OPERAND_LENGTH; bitCnt++)
	{
		SetPGC(1);
		SetPGC(0);
	}
	InsertDelay(TIME_P5A);
}


// Erase entire Microchip device
void MchpPort::BulkErase(unsigned int area)
{
//	printf("BulkErase:\n");
	ICSPIO(CORE_INSTRUCTION,0x0e3c); // MOVLW 0x3c
	ICSPIO(CORE_INSTRUCTION,0x6ef8); // MOVWF TBLPTRU
	ICSPIO(CORE_INSTRUCTION,0x0e00); // MOVLW 0x00
	ICSPIO(CORE_INSTRUCTION,0x6ef7); // MOVWF TBLPTRH
	ICSPIO(CORE_INSTRUCTION,0x0e05); // MOVLW 0x05
	ICSPIO(CORE_INSTRUCTION,0x6ef6); // MOVWF TBLPTRL
	ICSPIO(TABLE_WRITE,(area&0xFF00)|((area>>8)&0x00FF));	// write to 0x3c0005
	ICSPIO(CORE_INSTRUCTION,0x0e3c); // MOVLW 0x3c
	ICSPIO(CORE_INSTRUCTION,0x6ef8); // MOVWF TBLPTRU
	ICSPIO(CORE_INSTRUCTION,0x0e00); // MOVLW 0x00
	ICSPIO(CORE_INSTRUCTION,0x6ef7); // MOVWF TBLPTRH
	ICSPIO(CORE_INSTRUCTION,0x0e04); // MOVLW 0x04
	ICSPIO(CORE_INSTRUCTION,0x6ef6); // MOVWF TBLPTRL
	ICSPIO(TABLE_WRITE,((area<<8)&0xFF00)|(area&0x00FF));	// write to 0x3c0004
	ICSPIO(CORE_INSTRUCTION,0x0000); // NOP
	int i;
	for(i=0; i<CMD_LENGTH; i++)
	{
		SetPGC(1);
		SetPGC(0);
	}
	InsertDelay(TIME_P11);			 // wait for bulk erase to complete
	InsertDelay(TIME_P10);
	for(i=0; i<OPERAND_LENGTH; i++)
	{
		SetPGC(1);
		SetPGC(0);
	}

}


// Erase rows of program memory in the Microchip device
void MchpPort::RowErase(unsigned int loAddr, unsigned int hiAddr)
{
	for(unsigned int addr=loAddr&0xffffffc0; addr<=hiAddr; addr+=64)
	{
		// Flash prog. spec says you only need to issue these three instructions at the start,
		// but you actually need to issue them for every row erase.
		ICSPIO(CORE_INSTRUCTION,0x8ea6);	// BSF EECON1, EEPGD
		ICSPIO(CORE_INSTRUCTION,0x9ca6);	// BCF EECON1, CFGS
		ICSPIO(CORE_INSTRUCTION,0x84a6);	// BSF EECON1, WREN
		ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | ((addr>>16)&0xff)); // MOVLW address[21:16]
		ICSPIO(CORE_INSTRUCTION,0x6ef8);						// MOVWF TBLPTRU
		ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | ((addr>>8)&0xff));  // MOVLW address[15:8]
		ICSPIO(CORE_INSTRUCTION,0x6ef7);						// MOVWF TBLPTRH
		ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | (addr&0xff));       // MOVLW address[7:0]
		ICSPIO(CORE_INSTRUCTION,0x6ef6);						// MOVWF TBLPTRL
		ICSPIO(CORE_INSTRUCTION,0x88a6);						// BSF EECON1, FREE
		ICSPIO(CORE_INSTRUCTION,0x82a6);						// BSF EECON1, WR
		WaitWhileProgramming();
	}
}


// Program Microchip device with data from hex record
bool MchpPort::Program(HexRecord& hx)
{
	unsigned long address = hx.GetAddress();
	unsigned long length = hx.GetLength();

//	printf("Program:\n");

	if(length==0)
		return true; // nothing to do

	if(length > WRITE_BUFFER_SIZE)
	{
		char s[100];
		sprintf(s,"Hex record at %06x is too large (%d bytes)",address,length);
		string msg(s);
		GetErr().SimpleMsg(XSErrorMajor,msg);
		return false;
	}

	if(address >= CONFIGBITS_ADDRESS)
	{
		// programming configuration bits
		ICSPIO(CORE_INSTRUCTION,0x8ea6); // BSF EECON1,EEPGD
		ICSPIO(CORE_INSTRUCTION,0x8ca6); // BSF EECON1,CFGS
		for(int l=0; l<length-1; l++,address++)
		{
			ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | ((address>>16)&0xff)); // MOVLW address[21:16]
			ICSPIO(CORE_INSTRUCTION,0x6ef8); // MOVWF TBLPTRU
			ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | ((address>>8)&0xff));  // MOVLW address[15:8]
			ICSPIO(CORE_INSTRUCTION,0x6ef7); // MOVWF TBLPTRH
			ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | (address&0xff));       // MOVLW address[7:0]
			ICSPIO(CORE_INSTRUCTION,0x6ef6); // MOVWF TBLPTRL
			if(address%2) // odd address requires config data in the MSB
				ICSPIO(START_PROGRAMMING,hx[l]<<8);
			else // even address requires config data in the LSB
				ICSPIO(START_PROGRAMMING,hx[l]);
			WaitWhileProgramming();
		}
	}
	else
	{
		// programming code space or ID bits
		if(length%2)
		{
			char s[100];
			sprintf(s,"Hex record at %06x has an odd length (%d bytes)",address,length);
			string msg(s);
			GetErr().SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		if(address%2)
		{
			char s[100];
			sprintf(s,"Hex record at %06x starts at an odd address",address);
			string msg(s);
			GetErr().SimpleMsg(XSErrorMajor,msg);
			return false;
		}
		
		ICSPIO(CORE_INSTRUCTION,0x8ea6); // BSF EECON1,EEPGD
		ICSPIO(CORE_INSTRUCTION,0x9ca6); // BCF EECON1,CFGS
		ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | ((address>>16)&0xff)); // MOVLW address[21:16]
		ICSPIO(CORE_INSTRUCTION,0x6ef8); // MOVWF TBLPTRU
		ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | ((address>>8)&0xff));  // MOVLW address[15:8]
		ICSPIO(CORE_INSTRUCTION,0x6ef7); // MOVWF TBLPTRH
		ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | (address&0xff));       // MOVLW address[7:0]
		ICSPIO(CORE_INSTRUCTION,0x6ef6); // MOVWF TBLPTRL
		int l;
		for(l=0; l<length-2; l+=2)
			ICSPIO(TABLE_WRITE_POSTINC2,(hx[l+1]<<8)|hx[l]);
		ICSPIO(START_PROGRAMMING,(hx[l+1]<<8)|hx[l]);
		WaitWhileProgramming();
	}

	return true;
}


// Read contents of Microchip device into a hex record
bool MchpPort::Read(
				HexRecord& hx,			// hex record to store data in
				unsigned long loAddr,	// begin reading at this address
				unsigned long hiAddr	// end reading at this address
				)
{
	XSError& err = GetErr();

//	printf("Read:\n");

	hx.SetAddress(loAddr);	// set beginning and ending addresses for the hex record
	unsigned long length = hiAddr - loAddr + 1;
	hx.SetLength(length);

	if(length==0)
		return true; // nothing to do

//	fprintf(stderr,"Reading %08x to %08x\n",loAddr,hiAddr);

	// table pointer loaded with starting address
	ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | ((loAddr>>16)&0xff)); // MOVLW address[21:16]
	ICSPIO(CORE_INSTRUCTION,0x6ef8); // MOVWF TBLPTRU
	ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | ((loAddr>>8)&0xff));  // MOVLW address[15:8]
	ICSPIO(CORE_INSTRUCTION,0x6ef7); // MOVWF TBLPTRH
	ICSPIO(CORE_INSTRUCTION,(0x0e<<8) | (loAddr&0xff));       // MOVLW address[7:0]
	ICSPIO(CORE_INSTRUCTION,0x6ef6); // MOVWF TBLPTRL

	for(unsigned long l=0; l<length; l++)
		hx[l] = ICSPIO(TABLE_READ_POSTINC,0x0);

	hx.CalcCheckSum();	// put the checksum into the hex record
	return true;
}


// download the Microchip device with the contents of a HEX file
bool MchpPort::Download(string& hexfileName)
{
    bool status ;

    status = false;
	XSError& err = GetErr();
	
	if(hexfileName.length()==0)
		return false;  // stop if no hex file was given
	
	ifstream is(hexfileName.c_str(), ios::binary);  // otherwise open hex file
	if(!is || is.fail() || is.eof()!=0)
	{ // error - couldn't open hex file
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << hexfileName.c_str() << "\n";
		err.EndMsg();
		return false;
	}
	
	// determine the size of the file
	is.seekg(0,ios::end);	// move pointer to end of file
	streampos streamEndPos = is.tellg();	// pointer position = position of end of file
	is.seekg(0,ios::beg);	// return pointer to beginning of file

	string desc("Microchip Download"), subdesc("Downloading "+StripPrefix(hexfileName));
    progressGauge = new Progress(NULL, &err);
    if (NULL != progressGauge)
    {
        progressGauge->Setup(desc,subdesc,0,streamEndPos);

        status = Download(is);
	
        delete progressGauge;
        progressGauge = NULL;
    }
	
	is.close();  // close-up the hex file
	
	return status;
}


// download the Microchip device with the contents arriving through a stream
bool MchpPort::Download(istream& is)
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);	// start progress indicator at zero

	// read hex records from file and place data in the Microchip device
	HexRecord hx;
	while(is.eof()==0)
	{
		is >> hx;
		if(is.eof()!=0)
			break; // terminate loading loop when stream goes dry
		if(err.IsError())
			break;
		if(is.fail())
		{
			err.SetSeverity(XSErrorMajor);
			err << "error reading stream\n";
			err.EndMsg();
			break;
		}
		if(hx.IsError())
		{ // some error in the hex record itself
			err.SetSeverity(XSErrorMajor);
			err << "hex record: " << hx.GetErrMsg() << "\n";
			err.EndMsg();
			break;
		}
		else if(!hx.IsData())  // skip non-data records (like address offsets in Intel format)
			;
		else
		{ // send out an indication for each hex record that's loaded
			progressGauge->Report(is.tellg());
			if(Program(hx) == false) // download hex record
				return false;	// an error occurred while downloading the hex record
		}
	}
	progressGauge->Report(is.tellg());	// should set gauge to 100%
	
	return err.IsError() ? false:true;
}


// upload the Microchip device to a hex file
bool MchpPort::Upload(
				string& hexfileName,	// dump uploaded data to this file
				const char* format,		// hex file format
				unsigned long loAddr,	// start fetching data from this address
				unsigned long hiAddr	// stop at this address
				)
{
    bool status;

    status = false;
	XSError& err = GetErr();
	
	if(hexfileName.length()==0)
		return false;  // stop if no hex file was given
	
	ofstream os(hexfileName.c_str());  // otherwise open hex file
	if(os.fail() || os.eof()!=0)
	{ // error - couldn't open hex file
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << hexfileName.c_str() << "\n";
		err.EndMsg();
		return false;
	}

	string desc("Microchip Upload"), subdesc("Uploading "+StripPrefix(hexfileName));
    progressGauge = new Progress(NULL, &err);
    if (NULL != progressGauge)
    {
        progressGauge->Setup(desc,subdesc,loAddr,hiAddr);

        status = Upload(os,format,loAddr,hiAddr);

        delete progressGauge;
        progressGauge = NULL;
    }

	os.close();  // close-up the hex file

	return status;
}


// upload the Microchip device to a stream
bool MchpPort::Upload(
				ostream& os,			// dump uploaded data to this stream
				const char* format,		// hex file format
				unsigned long loAddr,	// start fetching data from this address
				unsigned long hiAddr	// stop at this address
				)
{
	XSError& err = GetErr();

	assert(progressGauge != NULL);	//	make sure progress indicator is initialized
	progressGauge->Report(0);	// start progress indicator at zero
	
	// read hex records from Microchip device and place data in the hex file
	HexRecord hx;
	hx.Setup(format);
	unsigned long addr;
	for(addr=loAddr; (addr|0xF)<=hiAddr; addr=(addr+16)&~0xF)
	{
		if(Read(hx,addr,addr|0xF) == false) // get 16 bytes from device
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
		progressGauge->Report(addr);	// give feedback on progress
	}
	if(addr<=hiAddr)
	{ // handle the last few bytes of an upload
		if(Read(hx,addr,hiAddr) == false) // get the last few bytes
			return false;	// an error occurred while uploading the hex record
		os << hx;		// send hex record to output stream
	}
	progressGauge->Report(hiAddr);	// should set gauge to 100%
	
	return true;
}


// Erase contents of Microchip device
bool MchpPort::Erase(void)
{
	BulkErase(0x0f87);
	return true;
}
