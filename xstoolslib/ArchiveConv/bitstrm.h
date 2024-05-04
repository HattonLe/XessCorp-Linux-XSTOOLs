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


#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <iostream>
#include <string>
using namespace std;


/** 
Stores and operates upon arbitrary-length strings of binary bits.

This object allows the creation of arbitrary-length binary strings and 
permits some basic operations on them. It is used primarily for handling 
the strings of instruction and data register bits that go through the 
JTAG port. 

*/
class Bitstream
{
	public:

	Bitstream(unsigned int n);

	Bitstream(const string s);

	~Bitstream(void);

	unsigned int GetLength(void) const;

	bool Resize(unsigned int n);

	bool operator==(Bitstream& b2) const;

	bool Subcompare(unsigned int pos, Bitstream& b2, unsigned int b2Pos) const;

	Bitstream& operator=(Bitstream& b2);

	Bitstream* Copy(void);

	unsigned int operator[](unsigned int bitIndex) const;

	void Clear(void);

	void SetBit(unsigned int bitIndex, unsigned int val);

	void SetBits(unsigned int bitIndex,	int firstBit, ... );

	Bitstream& operator^(Bitstream& b2) const;

	Bitstream& operator+(Bitstream& b2) const;

	void ShiftRight(unsigned int rightShift);

	void Reverse();

	ostream& PrintBits(ostream& os) const;

	unsigned char* ToHexString(void) const;

	bool FromHexString(unsigned int nBits, const char *hexString);

	bool FromHexStream(unsigned int nBits, istream& is, bool reverseBits);

	unsigned char* ToCharString(unsigned char *charString = NULL) const;

	bool FromCharString(unsigned int nBits, const unsigned char *charString);

	unsigned int* ToWordString(unsigned int *wordString = NULL) const;

	bool FromWordString(unsigned int nBits, const unsigned int *wordString);

	bool FromCharStream(unsigned int nBits, istream& is, bool reverseBits);

	string ToString(void) const;

	void FromString(const string s);

	private:

	unsigned int numBits;	///< number of bits in bitstream
	unsigned long* bits;	///< storage for bitstream
};

ostream& operator<<(ostream& os, Bitstream& b);

#endif
