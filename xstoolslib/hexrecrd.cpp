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
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include <string.h>

#include "hex.h"
#include "utils.h"
#include "hexrecrd.h"


/// Construct and initialize an empty hex record.
HexRecord::HexRecord(void)
{
	Setup(NULL);
}


/// Delete a hex record.
HexRecord::~HexRecord(void)
{
	delete data;
}


/// Setup the entries in a hex record.
void HexRecord::Setup(const char* format)  ///< string that indicates hex file format (e.g., "XESS-16", "EXO-24", "MCS", etc.)
{
	if(format==NULL)
	{
		SetFileFormat(UnknownFormat);		// set file format for hex data
		SetRecordType(InvalidRecord);	// record invalid until it's loaded with something
	}
	else
	{
		if(!strcmp(format,"XESS-16"))
		{
			SetFileFormat(XESSFormat);
			SetRecordType(XESSDataWith16BitAddressRecord);
		}
		else if(!strcmp(format,"XESS-24"))
		{
			SetFileFormat(XESSFormat);
			SetRecordType(XESSDataWith24BitAddressRecord);
		}
		else if(!strcmp(format,"XESS-32"))
		{
			SetFileFormat(XESSFormat);
			SetRecordType(XESSDataWith32BitAddressRecord);
		}
		else if(!strcmp(format,"EXO-16"))
		{
			SetFileFormat(MotorolaFormat);
			SetRecordType(MotoDataWith16BitAddressRecord);
		}
		else if(!strcmp(format,"EXO-24"))
		{
			SetFileFormat(MotorolaFormat);
			SetRecordType(MotoDataWith24BitAddressRecord);
		}
		else if(!strcmp(format,"EXO-32"))
		{
			SetFileFormat(MotorolaFormat);
			SetRecordType(MotoDataWith32BitAddressRecord);
		}
		else if(!strcmp(format,"HEX"))
		{
			SetFileFormat(IntelFormat);
			SetRecordType(IntelDataRecord);
		}
		else if(!strcmp(format,"MCS"))
		{
			SetFileFormat(IntelFormat);
			SetRecordType(IntelDataRecord);
		}
		else if(!strcmp(format,"XILINX"))
		{
			SetFileFormat(XilinxFormat);
			SetRecordType(XilinxDataRecord);
		}
		else
		{
			assert(1==0);	// should never get here
		}
	}

	data = NULL;					// no hex data at this time
	segmented = false;				// no Intel segmented addressing
	SetLength(0);					// length of hex data is zero
	SetAddress(0);					// hex data is not destined for any address
	SetBaseAddress(0);				// make sure base address is zero
	SetError(NoHexRecordError);		// no errors yet
	CalcCheckSum();					// calculate the checksum for this trivial record
}


/// Set the type of file format for the hex record.
void HexRecord::SetFileFormat(HexFileFormat f) ///< type of file format
{
	fileFormat = f;
}


/// Get the type of file format for the hex record.
///\return the hex file format
HexFileFormat HexRecord::GetFileFormat(void) const
{
	return fileFormat;
}


/// Set the type tag of the hex record.
void HexRecord::SetRecordType(HexRecordType t) ///< type of hex record
{
	recordType = t;

	// set the address mask according to the type of hex record
	switch(recordType)
	{
	case MotoDataWith16BitAddressRecord:
	case MotoEndWith16BitAddressRecord:
		segmented = false;
		mask = 0x0000FFFF;
		break;
	case MotoDataWith24BitAddressRecord:
	case MotoEndWith24BitAddressRecord:
		segmented = false;
		mask = 0x00FFFFFF;
		break;
	case MotoStartRecord:
	case MotoDataWith32BitAddressRecord:
	case MotoEndWith32BitAddressRecord:
		segmented = false;
		mask = 0xFFFFFFFF;
		break;
	case XESSDataWith16BitAddressRecord:
		segmented = false;
		mask = 0x0000FFFF;
		break;
	case XESSDataWith24BitAddressRecord:
		segmented = false;
		mask = 0x00FFFFFF;
		break;
	case XESSDataWith32BitAddressRecord:
		segmented = false;
		mask = 0xFFFFFFFF;
		break;
	case IntelExtSegAddressRecord:
		segmented = true;
		mask = 0x0000FFFF;
		break;
	case IntelExtLinAddressRecord:
		segmented = false;
		mask = 0x0000FFFF;
		break;
	case IntelDataRecord:
	case IntelEOFRecord:
	case IntelStartSegAddressRecord:
	case IntelStartLinAddressRecord:
		// segmented flag should be set by another record		
		mask = 0x0000FFFF;
		break;
	case XilinxDataRecord:
		segmented = false;
		mask = 0xFFFFFFFF;
		break;
	default:
		mask = 0xFFFFFFFF;
		break;
	}
}


/// Return the tag of the hex record.
HexRecordType HexRecord::GetRecordType(void) const
{
	return recordType;
}


/// Query the hex record to see if it contains data.
///\return true if the hex record contains data, false otherwise
bool HexRecord::IsData(void) const
{
	switch(recordType)
	{
	case MotoDataWith16BitAddressRecord:
	case MotoDataWith24BitAddressRecord:
	case MotoDataWith32BitAddressRecord:
	case IntelDataRecord:
	case XESSDataWith16BitAddressRecord:
	case XESSDataWith24BitAddressRecord:
	case XESSDataWith32BitAddressRecord:
	case XilinxDataRecord:
		return true;
	default:
		return false;
	}
}


/// Query the hex record to see if it is valid.
///\return true if the hex record is valid, false otherwise
bool HexRecord::IsValid(void) const
{
	return recordType!=InvalidRecord;
}


/// Set the address mask. 
void HexRecord::SetAddressMask(unsigned int addrMask)
{
	mask = addrMask;
}


/// Get the address mask.
unsigned int HexRecord::GetAddressMask(void) const
{
	return mask;
}


/// Set the address at which the hex record data should be loaded.
void HexRecord::SetAddress(unsigned int addr)
{
	switch(recordType)
	{
	case MotoDataWith16BitAddressRecord:
	case MotoEndWith16BitAddressRecord:
	case MotoDataWith24BitAddressRecord:
	case MotoEndWith24BitAddressRecord:
	case MotoStartRecord:
	case MotoDataWith32BitAddressRecord:
	case MotoEndWith32BitAddressRecord:
	case XESSDataWith16BitAddressRecord:
	case XESSDataWith24BitAddressRecord:
	case XESSDataWith32BitAddressRecord:
	case XilinxDataRecord:
		offset = addr;
		base = 0;
		break;
	case IntelExtSegAddressRecord:
		offset = 0;
		break;
	case IntelExtLinAddressRecord:
		offset = 0;
		break;
	case IntelDataRecord:
		offset = addr;
		break;
	case IntelEOFRecord:
	case IntelStartSegAddressRecord:
	case IntelStartLinAddressRecord:
	default:
		// we don't even use these
		offset = 0;
		break;
	}
}


/// Get address at which hex record data will be loaded.
unsigned int HexRecord::GetAddress(void) const
{
	return base + offset;
	switch(recordType)
	{
	case MotoDataWith16BitAddressRecord:
	case MotoEndWith16BitAddressRecord:
	case MotoDataWith24BitAddressRecord:
	case MotoEndWith24BitAddressRecord:
	case MotoStartRecord:
	case MotoDataWith32BitAddressRecord:
	case MotoEndWith32BitAddressRecord:
	case XESSDataWith16BitAddressRecord:
	case XESSDataWith24BitAddressRecord:
	case XESSDataWith32BitAddressRecord:
	case XilinxDataRecord:
		return offset;
		break;
	case IntelDataRecord:
		if(segmented)
			return (base + (offset & 0xFFFF)) & 0xFFFFFFFF;
		else
			return (base + offset) & 0xFFFFFFFF;
		break;
	case IntelExtSegAddressRecord:
	case IntelExtLinAddressRecord:
		return base;
		break;
	case IntelEOFRecord:
	case IntelStartSegAddressRecord:
	case IntelStartLinAddressRecord:
		// we don't even use these
		return 0;
		break;
	default:
		return 0;
		break;
	}
}


/// Set the base address for the hex record.
void HexRecord::SetBaseAddress(unsigned int baseAddr)
{
	base = baseAddr;
}


/// Get the base address for the hex record.
unsigned int HexRecord::GetBaseAddress(void) const
{
	return base;
}


/// Set the offset address for the hex record.
void HexRecord::SetOffsetAddress(unsigned int offsetAddr)
{
	offset = offsetAddr;
}


/// Get the offset address for the hex record.
unsigned int HexRecord::GetOffsetAddress(void) const
{
	return offset;
}


/// Get number of bytes in the address.
unsigned int HexRecord::GetAddressLength(void)
{
	switch(recordType)
	{
	case MotoDataWith16BitAddressRecord:	return 2;
	case MotoDataWith24BitAddressRecord:	return 3;
	case MotoDataWith32BitAddressRecord:	return 4;
	case MotoEndWith16BitAddressRecord:		return 2;
	case MotoEndWith24BitAddressRecord:		return 3;
	case MotoEndWith32BitAddressRecord:		return 4;
	case IntelDataRecord:					return 2;
	case IntelEOFRecord:					return 2;
	case IntelExtSegAddressRecord:			return 2;
	case IntelExtLinAddressRecord:			return 2;
	case IntelStartSegAddressRecord:		return 2;
	case IntelStartLinAddressRecord:		return 2;
	case XESSDataWith16BitAddressRecord:	return 2;
	case XESSDataWith24BitAddressRecord:	return 3;
	case XESSDataWith32BitAddressRecord:	return 4;
	case XilinxDataRecord:					return 4;
	default:
		assert(1==0);
		return 0;
	}
}


/// Set the size of the data area in the hex record.  Delete existing data.
void HexRecord::SetLength(unsigned int l)
{
	if(length != 0)
		delete data;
	length = l;
	data = new unsigned char[length];
}


/// Get the number of data bytes in the hex record.
unsigned int HexRecord::GetLength(void) const
{
	return length;
}


/// Index into the hex record to get a single byte.
unsigned char& HexRecord::operator[](unsigned int index)
{
	assert(index<length);
	return data[index];
}


/// Calculate the 8-bit checksum of the hex data and store it in the hex record.
void HexRecord::CalcCheckSum(void)
{
	int i;
	unsigned int addr = GetOffsetAddress();
	
	switch(fileFormat)
	{
	case MotorolaFormat:
		// add length to checksum.  Length of Motorola record is actually the
		// number of data bytes + number of address bytes + 1 (for the checksum byte)
		checkSum = length + GetAddressLength() + 1;
		for(i=GetAddressLength(); i>0; i--)
		{
			checkSum += (addr & 0xFF);
			addr >>= 8;
		}
		for(i=length-1; i>=0; i--)
			checkSum += data[i];
		checkSum = (~checkSum) & 0xFF;
		break;
	case IntelFormat:
		checkSum  = length;
		checkSum += addr & 0xFF;
		checkSum += ((addr>>8) & 0xFF);
		checkSum += (recordType & 0xF);
		for(i=length-1; i>=0; i--)
			checkSum += data[i];
		checkSum = (-checkSum) & 0xFF;
		break;
	case XESSFormat:
	case XilinxFormat:
	case UnknownFormat:
		checkSum = 0;
		break;
	default:
		assert(1==0);
	}
}


/// Get the calculated checksum of the hex record.
unsigned char HexRecord::GetCheckSum(void) const
{
	return checkSum;
}


/// Set the error flag in the hex record.
void HexRecord::SetError(HexRecordError e)
{
	err = e;
}


/// Get the error flag from the hex record.
HexRecordError HexRecord::GetError(void) const
{
	return err;
}


/// Query the hex record to see if there are any errors.
///\return true if the hex record has errors, false if the hex record has no errors
bool HexRecord::IsError(void) const
{
	return err != NoHexRecordError;
}


/// Return an error message for the hex record.
///\return string containing the error message
const char* HexRecord::GetErrMsg(void) const
{
	return ErrMsg(GetError());
}


/// Return an error message for the given type of error.
///\return string containing the error message
const char* ErrMsg(HexRecordError e)
{
	switch(e)
	{
	case NoHexRecordError:
		return "no error";
	case StartHexRecordError:
		return "should start with \'S\' or \':\' or \'-\' or \'=\' or \'+\'";
	case LengthHexRecordError:
		return "invalid length";
	case AddressHexRecordError:
		return "invalid address";
	case TypeHexRecordError:
		return "invalid type";
	case DataHexRecordError:
		return "invalid hex data";
	case CheckSumHexRecordError:
		return "invalid checksum";
	default:
		return "unknown error";
	}
}


/// Helper function to load a hex record from a stream.
///\return reference to the stream that delivers the hex record data
istream& operator>> (istream& is, 	///< stream that delivers hex records
					HexRecord& hx)	///< reference to hex record that will store data received from the stream
{
	unsigned char c;	// character from input stream
	Hex length(2);		// two-digit length for hex data record
	Hex address(0);		// offset address for hex data
	Hex recordType(2);	// two-digit data record tag
	Hex data(2);		// two-digit data item
	Hex checkSum(2);	// two-digit checksum
	unsigned int i = 0;	// counter variable

	if(hx.GetFileFormat() != XilinxFormat)
	{
		// get the initial character of a hex record to see what type it is
		is >> c;
		if(is.eof())
		{ // no hex record error
			hx.SetError(NoHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		if(is.fail())
		{ // error starting hex record
			hx.SetError(StartHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}

		switch(c)
		{
		case ':':
			hx.SetFileFormat(IntelFormat);
			break;
		case 'S':
			hx.SetFileFormat(MotorolaFormat);
			break;
		case '-':
			hx.SetFileFormat(XESSFormat);
			hx.SetRecordType(XESSDataWith16BitAddressRecord);
			break;
		case '=':
			hx.SetFileFormat(XESSFormat);
			hx.SetRecordType(XESSDataWith24BitAddressRecord);
			break;
		case '+':
			hx.SetFileFormat(XESSFormat);
			hx.SetRecordType(XESSDataWith32BitAddressRecord);
			break;
		default:
			// hx.SetError(StartHexRecordError);
			// nothing recognizable, so let's assume it is a Xilinx bit file
			hx.SetFileFormat(XilinxFormat);
			hx.SetRecordType(XilinxDataRecord);
			hx.SetAddress(0);
			is.seekg(0,ios::beg);	// return pointer to beginning of file
			break;
		}
	}

	switch(hx.GetFileFormat())
	{
	case MotorolaFormat:
		is >> c;
		if(is.eof())
		{ // no hex record error
			hx.SetError(StartHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		if(is.fail())
		{ // error starting S-record
			hx.SetError(StartHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		switch(c)
		{
		case '3':	// data record with 4-byte address
			hx.SetRecordType(MotoDataWith32BitAddressRecord);
			break;
		case '2':	// data record with 3-byte address
			hx.SetRecordType(MotoDataWith24BitAddressRecord);
			break;
		case '1':	// data record with 2-byte address
			hx.SetRecordType(MotoDataWith16BitAddressRecord);
			break;
		case '7':	// starting execution address in 4-byte address
			hx.SetRecordType(MotoEndWith32BitAddressRecord);
			break;
		case '8':	// starting execution address in 3-byte address
			hx.SetRecordType(MotoEndWith24BitAddressRecord);
			break;
		case '9':	// starting execution address in 2-byte address
			hx.SetRecordType(MotoEndWith16BitAddressRecord);
			break;
		case '0':	// name, version, revision info record with 2-byte address
			hx.SetRecordType(MotoStartRecord);
			break;
		default:
			hx.SetError(StartHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		
		// now get the number of bytes in the S-record
		is >> length;
		if(is.fail())
		{ // no length given for the S-record
			hx.SetError(LengthHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		// reduce the length by the number of address bytes and the checksum byte
		hx.SetLength(length.GetHex()-hx.GetAddressLength()-1);
		
		// now get the destination address for the data bytes
		address.SetLength(2 * hx.GetAddressLength());	// set # of hex digits in address
		is >> address;
		if(is.fail())
		{ // no starting address for the S-record data
			hx.SetError(AddressHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		hx.SetAddress(address.GetHex());
		
		// read in the number of data bytes given in the length
		for(i=0; i<hx.GetLength(); i++)
		{
			is >> data;
			hx[i] = data.GetHex();
			if(is.fail())
			{ // error if data ended too soon
				hx.SetError(DataHexRecordError);
				hx.SetRecordType(InvalidRecord);
				return is;
			}
		}
		
		// calculate the checksum for the data that has been read
		is >> checkSum; // get the checksum for the S-record
		if(is.fail())
		{ // error if no checksum when there should be one
			hx.SetError(CheckSumHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		hx.CalcCheckSum();	// now calculate the checksum for the data
		if(checkSum.GetHex()!=hx.GetCheckSum())
		{ // error if checksums do not match
			hx.SetError(CheckSumHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		
		// got all the way to the end, so there are no errors in the Motorola S-record
		hx.SetError(NoHexRecordError);
		break;

	case IntelFormat:
		// get the number of bytes in the hex record
		is >> length;
		if(is.fail())
		{ // no length given for the hex record
			hx.SetError(LengthHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		hx.SetLength(length.GetHex());
		
		// now get the destination offset address for the data bytes
		address.SetLength(4);	// set # of hex digits in offset address
		is >> address;
		if(is.fail())
		{ // no offset address for the hex record data
			hx.SetError(AddressHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}

		// get the Intel record type
		is >> recordType;
		if(is.fail())
		{ // no record type error
			hx.SetError(TypeHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		hx.SetRecordType( (HexRecordType) (recordType.GetHex() | (unsigned int)(hx.GetFileFormat())) ); // record the type of record
		
		// read in the number of data bytes given in the length
		for(i=0; i<hx.GetLength(); i++)
		{
			is >> data;
			hx[i] = data.GetHex();
			if(is.fail())
			{ // error if data ended too soon
				hx.SetError(DataHexRecordError);
				hx.SetRecordType(InvalidRecord);
				return is;
			}
		}

		// don't set the record address until the record type is determined
		// update the base address if this is an extended address record
		switch(hx.GetRecordType())
		{
		case IntelExtSegAddressRecord:
			hx.SetBaseAddress(((hx[0]<<8) + hx[1]) << 4);
			hx.SetAddress(0);
			break;
		case IntelExtLinAddressRecord:
			hx.SetBaseAddress(((hx[0]<<8) + hx[1]) << 16);
			hx.SetAddress(0);
			break;
		default:
			hx.SetAddress(address.GetHex());
		}
		
		// calculate the checksum for the data that has been read
		// and compare it with the calculated value
		is >> checkSum; // get the checksum for the hex record
		if(is.fail())
		{ // error if no checksum when there should be one
			hx.SetError(CheckSumHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		hx.CalcCheckSum();	// now calculate the checksum for the data
		if(checkSum.GetHex()!=hx.GetCheckSum())
		{ // error if checksums do not match
			hx.SetError(CheckSumHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		
		// got all the way to the end, so there are no errors in the hex record
		hx.SetError(NoHexRecordError);
		break;

	case XESSFormat:
		// get the number of bytes in the hex record
		is >> length;
		if(is.fail())
		{ // no length given for the hex record
			hx.SetError(LengthHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		hx.SetLength(length.GetHex());
		
		// now get the destination offset address for the data bytes
		address.SetLength(2 * hx.GetAddressLength());	// set # of hex digits in address
		is >> address;
		if(is.fail())
		{ // no offset address for the hex record data
			hx.SetError(AddressHexRecordError);
			hx.SetRecordType(InvalidRecord);
			return is;
		}
		hx.SetAddress(address.GetHex());
		
		// read in the number of data bytes given in the length
		for(i=0; i<hx.GetLength(); i++)
		{
			is >> data;
			hx[i] = data.GetHex();
			if(is.fail())
			{ // error if data ended too soon
				hx.SetError(DataHexRecordError);
				hx.SetRecordType(InvalidRecord);
				return is;
			}
		}
		
		// got all the way to the end, so there are no errors in the hex record
		hx.SetError(NoHexRecordError);
		break;

	case XilinxFormat:
		if(hx.GetAddress() == 0 && hx.GetLength() == 0)
		{
			// jump over the first field of the BIT file
			long unsigned int fieldLength = GetInteger(is);
			assert(fieldLength!=0);
			is.ignore(fieldLength);
			
			// process the second field
			fieldLength = GetInteger(is);
			assert(fieldLength==1);
			
			// now look for the field with the bitstream data
			const int BitstreamFieldType = 0x65; // field type for the bitstream data
			bool status = ScanForField(is,BitstreamFieldType);
			assert(status==true);

			// now we are at the start of the configuration bits
			fieldLength = GetInteger(is,4); // get the length of the bit stream in bytes
			assert(fieldLength>0);

			const int preambleLength = 16;
			hx.SetLength(preambleLength);
			for(int i=0; i<preambleLength; i++)
				hx[i] = 0xff;
		}
		else
		{
			const int frameLength = 16;
			hx.SetAddress(hx.GetAddress() + hx.GetLength());
			hx.SetLength(frameLength);
			for(int i=0; i<frameLength; i++)
			{
				char c;
				is.read((char*)&c,1);
				if(is.eof())
					break;
				hx[i] = c;
			}
			if(i < frameLength)
			{
				if(i == 0)
				{
					hx.SetLength(0);
					hx.SetRecordType(InvalidRecord);
				}
				else
				{
					HexRecord tmp;
                    int j;

					tmp.SetLength(i);
					for(int j=0; j<i; j++)
						tmp[j] = hx[j];
					hx.SetLength(i);
					for(j=0; j<i; j++)
						hx[j] = tmp[j];
				}
			}
		}
		break;

	default:
		assert(1==0);
		break;
	}
	
	return is;
}


/// Helper function to output the hex record to a stream in Motorola, Intel, or XESS format.
///\return the output stream for the hex record data
ostream& operator<< (ostream& os,	///< stream to which hex record is sent
					HexRecord& hx)	///< hex record that is output through the stream
{
	if(!hx.IsValid())
		return os;	// abort if hex record is not valid

	unsigned int i;

	switch(hx.GetFileFormat())
	{
	case MotorolaFormat:
		os << "S";
		switch(hx.GetRecordType())
		{
			case MotoDataWith16BitAddressRecord:
				os << '1';
				os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength()+3;
				os << setiosflags(ios::uppercase) << setfill('0') << setw(4) << hex << hx.GetAddress();
				break;
			case MotoDataWith24BitAddressRecord:
				os << '2';
				os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength()+4;
				os << setiosflags(ios::uppercase) << setfill('0') << setw(6) << hex << hx.GetAddress();
				break;
			case MotoDataWith32BitAddressRecord:
				os << '3';
				os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength()+5;
				os << setiosflags(ios::uppercase) << setfill('0') << setw(8) << hex << hx.GetAddress();
				break;
			case MotoEndWith16BitAddressRecord:
				os << '9';
				os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength()+3;
				os << setiosflags(ios::uppercase) << setfill('0') << setw(4) << hex << hx.GetAddress();
				break;
			case MotoEndWith24BitAddressRecord:
				os << '8';
				os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength()+4;
				os << setiosflags(ios::uppercase) << setfill('0') << setw(6) << hex << hx.GetAddress();
				break;
			case MotoEndWith32BitAddressRecord:
				os << '7';
				os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength()+5;
				os << setiosflags(ios::uppercase) << setfill('0') << setw(8) << hex << hx.GetAddress();
				break;
			case MotoStartRecord:
				os << '0';
				os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength()+3;
				os << setiosflags(ios::uppercase) << setfill('0') << setw(4) << hex << hx.GetAddress();
				break;
			default:
				assert(1==0);
				break;
		}
		for(i=0; i<hx.GetLength(); i++)
			os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << (int)hx[i];
		os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << (int)hx.GetCheckSum();
		os << endl;
		break;

	case IntelFormat:
		if(hx.IsData())
		{
			unsigned int base, offset;
			base = hx.GetBaseAddress();
			offset = hx.GetOffsetAddress();
			if(base != (offset&~0xFFFF))
			{
				base = offset & ~0xFFFF;
				HexRecord h;
				h.Setup("HEX");
				h.SetRecordType(IntelExtLinAddressRecord);
				h.SetLength(2);
				h.SetOffsetAddress(0);
				h[0] = (base>>24) & 0xFF;
				h[1] = (base>>16) & 0xFF;
				h.CalcCheckSum();
				os << h;
			}
			hx.SetBaseAddress(offset & ~0xFFFF);
			hx.SetOffsetAddress(offset & 0xFFFF);
		}
		os << ":";
		os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength();
		os << setiosflags(ios::uppercase) << setfill('0') << setw(4) << hex << hx.GetOffsetAddress();
		os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << (hx.GetRecordType() & 0x0F);
		for(i=0; i<hx.GetLength(); i++)
			os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << (int)hx[i];
		os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << (int)hx.GetCheckSum();
		os << endl;
		break;

	case XESSFormat:
		switch(hx.GetRecordType())
		{
		case XESSDataWith16BitAddressRecord:
			os << "- ";
			os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength();
			os << " ";
			os << setiosflags(ios::uppercase) << setfill('0') << setw(4) << hex << hx.GetAddress();
			break;
		case XESSDataWith24BitAddressRecord:
			os << "= ";
			os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength();
			os << " ";
			os << setiosflags(ios::uppercase) << setfill('0') << setw(6) << hex << hx.GetAddress();
			break;
		case XESSDataWith32BitAddressRecord:
			os << "+ ";
			os << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << hx.GetLength();
			os << " ";
			os << setiosflags(ios::uppercase) << setfill('0') << setw(8) << hex << hx.GetAddress();
			break;
		}
		for(i=0; i<hx.GetLength(); i++)
			os << " " << setiosflags(ios::uppercase) << setfill('0') << setw(2) << hex << (int)hx[i];
		os << endl;
		break;

	default:
		assert(1==0);
	}
	
	return os;
}
