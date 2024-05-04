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


#ifndef XSBOARD_H
#define XSBOARD_H

#include <string>
using namespace std;

#include "xserror.h"
#include "jtagport.h"


///< type of port connected to XESS Board
typedef enum
{	
	PORTTYPE_INVALID,
	PORTTYPE_LPT,
	PORTTYPE_USB,
	PORTTYPE_LPTJTAG,
	PORTTYPE_USBJTAG,
	PORTTYPE_END						///< total number of port types
} PortType;

/// Structure to hold information about XESS boards.
typedef struct brdModel_struct
{
    char* brdModel;                             ///< XS Board model identifier
    char* chipType;                             ///< identifier for main programmable chip on board
	struct{
        bool  allowed;                          ///< true if the FPGA board allows this type of interface
        int   invMask;                          ///< mask for inverting signals going through the parallel port
        const char* dwnldIntfcBitstreamFile;	///< standard interface for downloading/uploading bitstreams and RAM data
        const char* ramBitstreamFile;			///< RAM upload/download interface bistream file
        const char* oscBitstreamFile;			///< programmable oscillator interface bitstream file
        bool  programmableOsc;                  ///< true if the oscillator is programmable
        const char* eraseBitstreamFile;         ///< erase file for interface CPLD on board
        const char* flashBitstreamFile;         ///< Flash/SEEPROM upload/download interface bitstream file
        const char* flashConfigBitstreamFile;	///< configure FPGA from flash bitstream file
        const char* testIntfcBitstreamFile;     ///< test interface bitstream file
        const char* testBitstreamFile;          ///< board test bitstream file
        const char* testObjFile;				///< board test program object file
        const char* userInstruction;            ///< bitstring for a JTAG USER instruction
	} port[PORTTYPE_END];
} XSBoardInfo; 


/// A virtual object that contains pointers to the base methods provided by all the XS Board objects.
///
/// This is a virtual class from which the object classes for all the XESS Board classes are derived. 
/// This class provides virtual methods that all the derived classes must support.
class XSBoard
{
	public:

	/// Destroy an XESS board object.
	virtual ~XSBoard(void) {};

	/// Setup an object for an XESS board.
	/// This method initializes the various objects contained in the XESS Board object as indicated by the string that identifies the board model. 
	/// All communication with the XESS Board will occur through the given parallel port. 
	/// Any errors that occur during setup will be reported through the given error-reporting channel. 
	///\return true if the operation was successful, false otherwise
	virtual bool Setup(
		XSError* err,			///< error reporting object
		const char* brdModel,	///< model of XESS Board
		unsigned int lptNum)	///< number of parallel port connected to board
		= 0;

	/// Set values for flags in the XESS board object.
	virtual void SetFlags(unsigned long f)	///< flag value
		= 0;

	/// Get flag values from the XESS board object.
	///\return flag value
	virtual unsigned long GetFlags(void)
		= 0;

	/// Load a configuration bitstream into the FPGA.
	/// This method downloads the configuration bitstream in the given file into the main programmable device on the XESS Board. 
	///\return true if the operation was successful, false otherwise
    virtual bool Configure(const char *fileName, bool *UserCancelled)	///< FPGA config. bitstream file
		= 0;

	/// Load a configuration bitstream into the interface CPLD.
	/// This method downloads the configuration bitstream in the given file into the programmable device that manages the parallel port interface on the XESS Board. 
	///\return true if the operation was successful, false otherwise
    virtual bool ConfigureInterface(const char *fileName, bool *UserCancelled)	///< FPGA config. bitstream file
		= 0;

	/// Download hex file into RAM on the XESS board.
	/// This method downloads a HEX data file in Intel, Motorola or XESS format into the RAM on the XESS Board. 
	/// Because multiple HEX files may be downloaded, boolean parameters are provided to indicate if the given file is the 
	/// first or last in the sequence so that initialization and shutdown procedures can be activated accordingly. 
	///\return true if the operation was successful, false otherwise
	virtual bool DownloadRAM(
            const char *fileName,		///< EXO, MCS or XES hex data file
			bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
			bool bigEndianBits,		///< if true, data is stored in RAM with most-significant bit in position 0
			bool doStart,			///< if true, perform startup operations for RAM downloads
            bool doEnd,				///< if true, perform terminating operations for RAM downloads
            bool *UserCancelled)
		= 0;

	/// Upload XESS board RAM from loAddr to hiAddr into a hex file.
	/// This method uploads data between the upper and lower addresses (inclusive) from the XESS Board RAM and stores it in a 
	/// HEX file in Intel, Motorola or XESS format.
	///\return true if the operation was successful, false otherwise
	virtual bool UploadRAM(
            const char *fileName,		///< store uploaded data in this EXO, MCS or XES hex file
			const char* format,		///< select hex file format - Intel mcs, Moto exo, or XESS xes
			unsigned int loAddr,	///< lower address of RAM data (inclusive)
			unsigned int hiAddr,	///< upper address of RAM data (inclusive)
			bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
			bool bigEndianBits,		///< if true, data is stored in RAM with most-significant bit in position 0
			bool doStart,			///< Perform startup operations for RAM uploads
            bool doEnd,
            bool *UserCancelled)				///< Perform terminating operations for RAM uploads
		= 0;

	/// Read contents of a single location in RAM.
	///\return true if the operation was successful, false otherwise
	virtual bool ReadRAM(
			unsigned int addr,		///< address of RAM location
			unsigned int* data,		///< loaded with data returned from RAM location
			bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
            bool bigEndianBits,
            bool *UserCancelled)		///< if true, data is stored in RAM with most-significant bit in position 0
		= 0;

	/// Write contents of a single location in RAM.
	///\return true if the operation was successful, false otherwise
	virtual bool WriteRAM(
			unsigned int addr,		///< address of RAM location
			unsigned int data,		///< data to be written into RAM location
			bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
            bool bigEndianBits,
            bool *UserCancelled)		///< if true, data is stored in RAM with most-significant bit in position 0
		= 0;

	/// Download hex file into Flash on the XESS board.
	/// This method downloads a HEX data file in Intel, Motorola or XESS format into the Flash memory on the XESS Board. 
	/// Because multiple HEX files may be downloaded, boolean parameters are provided to indicate if the given file is the first 
	/// or last in the sequence so that initialization and shutdown procedures can be activated accordingly. 
	///\return true if the operation was successful, false otherwise
	virtual bool DownloadFlash(
            const char *fileName,		///< EXO, MCS or XES hex data file
			bool bigEndianBytes,	///< if true, data is stored in Flash with most-significant byte at lower address
			bool bigEndianBits,		///< if true, data is stored in Flash with most-significant bit in position 0
			bool doStart,			///< perform startup operations for Flash downloads
            bool doEnd,
            bool *UserCancelled)				///< perform terminating operations for Flash downloads
		= 0;

	/// Upload XESS board Flash from loAddr to hiAddr into a hex file.
	/// This method uploads data between the upper and lower addresses (inclusive) from the XESS Board Flash and stores it in a 
	/// HEX file in Intel, Motorola or XESS format. 
	///\return true if the operation was successful, false otherwise
	virtual bool UploadFlash(
        const char *fileName,		///< store uploaded data in this EXO, MCS or XES hex file
		const char* format,		///< select hex file format - Intel mcs, Moto exo, or XESS xes
		unsigned int loAddr,	///< lower address of RAM data (inclusive)
		unsigned int hiAddr,	///< upper address of RAM data (inclusive)
		bool bigEndianBytes,	///< if true, data is stored in RAM with most-significant byte at lower address
		bool bigEndianBits,		///< if true, data is stored in RAM with most-significant bit in position 0
		bool doStart,			///< perform startup operations for Flash uploads
        bool doEnd,
        bool *UserCancelled)				///< perform terminating operations for Flash uploads
		= 0;

	/// Program the oscillator frequency.
	/// This method sets the divisor of the programmable oscillator on the XESS Board. 
	/// The internal master oscillator is divided by this parameter to arrive at the clock frequency that is output to the rest of the  XESS Board circuitry. 
	/// An external clock source is used as the master oscillator if the extOscPresent parameter is true. 
	///\return true if the operation was successful, false otherwise
	virtual bool SetFreq(
		int div,				///< divisor for master frequency
		bool extOscPresent)		///< if true, master frequency arrives from external source, otherwise use internal oscillator
		= 0;

	/// Program the control registers of the audio codec.
	///\return true if the operation was successful, false otherwise
	virtual bool SetupAudio(int *reg)	///< array of values to load into registers
		= 0;

	/// Setup the registers of the video input decoder.
	///\return true if the operation was successful, false otherwise
	virtual bool SetupVideoIn(string& fileName) ///< file containing values to load into video decoder registers
		= 0;

	/// Run diagnostic on XESS board.
	/// This method executes a diagnostic test routine on the XESS Board and returns true if the test passes and false if the test detects a problem.
	///\return true if the XESS board passed the diagnostic, false otherwise
	virtual bool Test(void)
		= 0;

	/// Apply test vectors and collect the responses.
	/// This method sends byte-wide test vectors to the port and collects the responses.
	///\return the response to a single test vector (if numVectors==1).
	virtual unsigned char ApplyTestVectors(
		unsigned char singleVector,		///< single, byte-wide test vector (used if numVectors==1)
		unsigned char mask,				///< mask for output bits
		unsigned char *vector = NULL,	///< array of byte-wide test vectors (used if numVectors>1)
		unsigned char *response = NULL,	///< array of responses to test vectors (used if numVectors>1)
		unsigned int numVectors = 1)	///< number of test vectors to apply through parallel port
		= 0;

	/// Get current test vector being applied to the FPGA.
	///\return the value of the current test vector.
	virtual unsigned char GetTestVector(void) = 0;

	/// Download unsigned integer values from an array to the RAM on the XS Board.
	///\return true if the operation was successful, false otherwise
	virtual bool DownloadRAMFromIntArray(
		unsigned *intArray,		///< array of data to download to XS Board RAM
		unsigned address,		///< starting address in XS Board RAM
		unsigned numInts)		///< number of data values to download from array
		= 0;

	/// Upload unsigned integer values the RAM on the XS Board to an array.
	///\return true if the operation was successful, false otherwise
	virtual bool UploadRAMToIntArray(
		unsigned *intArray,		///< array to be loaded with data from the XS Board RAM
		unsigned address,		///< starting address of data in XS Board RAM
		unsigned numInts)		///< number of data values to upload from the XS Board RAM
		= 0;			

	
	protected:

	XSBoardInfo *brdInfo;
	XSError *brdErr;		///< global error object for entire board
	int brdIndex;			///< index into global table of information about XESS Board models
	char* brdModel;			///< board model
	unsigned long flags;	///< general-purpose flags
	PortType portType;		///< TYpe of port, either LPT or USB
	unsigned int portNum;	///< port number
};


#endif
