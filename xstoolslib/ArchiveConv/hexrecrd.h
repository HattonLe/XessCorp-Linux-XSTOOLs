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


#ifndef HEXRECORD_H
#define HEXRECORD_H

#include <iostream>


/**
Supported hex file formats.

This object implements a generic hexadecimal data record with a buffer 
containing data bytes and a base address that records the starting 
address in memory for the data. This object has methods for entering 
data, reading the data back, determining the type of the data and 
setting/querying the starting address. Operators are also provided that 
allow storing/loading of HexRecord objects to/from Intel, Motorola and 
XESS HEX files. 

*/
typedef enum
{
	MotorolaFormat	= 0x00,	///< Motorola EXO format
	IntelFormat		= 0x10,	///< Intel MCS or HEX format
	XESSFormat		= 0x20,	///< XESS XES format
	XilinxFormat    = 0x30, ///< .bit format, not really a hex format
	UnknownFormat   = 0xF0,
} HexFileFormat;


/// Types of records found in a hex file.
typedef enum
{
	MotoStartRecord					= 0x00,	// first record in file
	MotoDataWith16BitAddressRecord	= 0x01,	// actual address and data
	MotoDataWith24BitAddressRecord	= 0x02,	// actual address and data
	MotoDataWith32BitAddressRecord	= 0x03,	// actual address and data
	MotoEndWith16BitAddressRecord	= 0x09,	// actual address and data
	MotoEndWith24BitAddressRecord	= 0x08,	// actual address and data
	MotoEndWith32BitAddressRecord	= 0x07,	// actual address and data
	IntelDataRecord					= 0x10,	// actual address offset and data
	IntelEOFRecord					= 0x11,	// end of file record
	IntelExtSegAddressRecord		= 0x12,	// specifies bits 4-19 of 20-bit address
	IntelExtLinAddressRecord		= 0x14,	// specifies upper 16 bits of 32-bit address
	IntelStartSegAddressRecord		= 0x13,	// specifies 20-bit execution start address
	IntelStartLinAddressRecord		= 0x15,	// specifies 32-bit execution start address
	XESSDataWith16BitAddressRecord	= 0x20,	// actual address and data
	XESSDataWith24BitAddressRecord	= 0x21,	// actual address and data
	XESSDataWith32BitAddressRecord	= 0x22,	// actual address and data
	XilinxDataRecord                = 0x30, // bits in bitstream
	InvalidRecord					= 0xFF,	// erroneous record
} HexRecordType;


/// Types of errors that can occur when handling hex records.
typedef enum
{
	NoHexRecordError,
	StartHexRecordError,
	LengthHexRecordError,
	AddressHexRecordError,
	TypeHexRecordError,
	DataHexRecordError,
	CheckSumHexRecordError
} HexRecordError;


/// Extracts hexadecimal records from hex files in Intel, Motorola and XESS formats.
class HexRecord
{
	public:

	HexRecord(void);

	~HexRecord(void);

	void Setup(const char* format);

	void SetFileFormat(HexFileFormat f);

	HexFileFormat GetFileFormat(void) const;

	void SetRecordType(HexRecordType t);

	HexRecordType GetRecordType(void) const;

	bool IsData(void) const;

	bool IsValid(void) const;

	void SetAddress(unsigned int addr);

	unsigned int GetAddress(void) const;

	void SetBaseAddress(unsigned int baseAddr);

	unsigned int GetBaseAddress(void) const;

	void SetOffsetAddress(unsigned int off);

	unsigned int GetOffsetAddress(void) const;

	unsigned int GetAddressLength(void);

	void SetAddressMask(unsigned int mask);

	unsigned int GetAddressMask(void) const;

	void SetLength(unsigned int l);

	unsigned int GetLength(void) const;

	unsigned char& operator[](unsigned int index);

	void CalcCheckSum(void);

	unsigned char GetCheckSum(void) const;

	void SetError(HexRecordError e);

	HexRecordError GetError(void) const;

	bool IsError(void) const;

	const char* GetErrMsg(void) const;


	private:

	HexRecordError err;			///< hex record error
	HexFileFormat fileFormat;	///< hex file format (XESS, Intel, Motorola)
	HexRecordType recordType;	///< type of data stored in hex record
	unsigned int length;		///< number of data bytes in the hex record
	unsigned int offset;		///< destination address for the hex data
	unsigned int base;			///< base address added to offset to create final address
	unsigned int mask;			///< masks off upper bits for address lengths less than 32-bits
	bool segmented;				///< true if record is for Intel 20-bit segmented data
	unsigned char* data;		///< data values stored in the hex record
	unsigned char checkSum;		///< checksum for the entire hex record
};


const char* ErrMsg(HexRecordError e);

extern istream& operator>> (istream& is, HexRecord& hx);

extern ostream& operator<< (ostream& is, HexRecord& hx);


#endif
