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


#include <cctype>
#include <cstdarg>
#include <cassert>

#include <string.h>

#include "bitstrm.h"
#include "utils.h"

#define MIN(a,b) ((a)<(b) ? (a):(b))
// Fiddle the maths so that 0 bits requires 1 long so we don't have to worry about zero memory allocation
#define NUM_OF_LONGS(numBits)	((numBits - (numBits ? 1 : 0)) / bitsPerLong + 1)

static const unsigned int wordsPerLong		= sizeof(unsigned long) / (2 * sizeof(char));
static const unsigned int charsPerLong		= sizeof(unsigned long) / sizeof(char);
static const unsigned int nybblesPerLong	= 2 * charsPerLong;
static const unsigned int bitsPerLong		= 8 * charsPerLong;


/// Allocates a bitstream containing at least n bits.
Bitstream::Bitstream(unsigned int n) ///< length of bitstream
{
	numBits = n;		// remember number of bits in the bitstream
    unsigned int numLongs;
    unsigned int i;

    numLongs = NUM_OF_LONGS(numBits); // number of words to store it
    bits = new unsigned long[numLongs];	// allocate storage
    //  make sure storage was allocated
    if (NULL != bits)
    {
        for (i = 0; i < numLongs; i++)
        {
            bits[i] = 0;	// clear all bits in stream to 0
        }
    }
}


/// Allocates a bitstream and initializes it with a bit pattern
Bitstream::Bitstream(const string s)
{
	Bitstream(0);
	FromString(s);
}


/// Frees the storage used by a bitstream.
Bitstream::~Bitstream(void)
{
    delete [] bits;
	bits = NULL;
}


/// Returns the number of bits in the bitstream.
unsigned int Bitstream::GetLength(void) const
{
	return numBits;
}


/// Resizes a bitstream while keeping the current bitstream contents.
///\return true if successful, false if not.
bool Bitstream::Resize(unsigned int n) ///< new size of bitstream
{
    unsigned int numLongs = NUM_OF_LONGS(numBits);
    unsigned int newnumLongs = NUM_OF_LONGS(n);
    unsigned int i;
    unsigned long *newBits;

    newBits = new unsigned long[newnumLongs];

    for (i = 0; i < MIN(numLongs, newnumLongs); i++)
    {
		newBits[i] = bits[i];
    }
    for( ; i < newnumLongs; i++)
    {
		newBits[i] = 0;
    }
    delete [] bits;
	bits = newBits;

	numBits = n;
	return true;
}


/// Compares two bitstreams.
///\return true if bitstreams match, false if not.
bool Bitstream::operator==(Bitstream& b2) const
{
    if (b2.numBits != numBits)	// can't be equal unless same length
    {
		return false;
    }

    for (unsigned int i = NUM_OF_LONGS(numBits); i > 0; i--)
    {
        if (bits[i - 1] != b2.bits[i - 1])		// compare word-by-word
        {
			return false;	// failed - bits don't match
        }
    }
    return true;	// ok, they must be equal
}


/// Compares the end of 1st bitstream against a subfield of 2nd bitstream
///\return true if bitstreams match, false if not.
bool Bitstream::Subcompare(unsigned int pos,	///< position in 1st bitstream
						   Bitstream& b2,		///< 2nd bitstream
						   unsigned int b2Pos /**< position in 2nd bitstream */) const
{
    unsigned int i, j;

    assert(pos < numBits);       // field 1 goes from pos -> numBits
    assert(b2Pos < b2.numBits);   // field 2 goes from b2pos -> b2.numBits

    if (numBits - pos > b2.numBits - b2Pos)
    {
		return false;     // no match - field 1 longer than field 2
    }

    for (i = pos, j = b2Pos; i < numBits; i++,j++)
    {
        if ((*this)[i] != b2[j])
        {
			return false;	// no match - one or more bits don't match
        }
    }
    return true;	// got through the whole thing, so streams match
}


/// Copies contents of one bitstream into another (also adjusts size).
Bitstream& Bitstream::operator=(Bitstream& b2)
{
    unsigned int numLongs2;
    unsigned int i;

    numLongs2 = NUM_OF_LONGS(b2.numBits); // # words in b2
    if (NUM_OF_LONGS(numBits) != numLongs2)	// compare # words in b1 to b2
    {
        // change size of b1 bitstream to match b2
        delete [] bits;
        bits = new unsigned long[numLongs2];
        assert(bits != NULL);
	}
	numBits = b2.numBits;
    for(i = 0; i < numLongs2; i++)
    {
		bits[i] = b2.bits[i]; // copy b2 to b1 word-by-word
    }
	return *this;
}


/// Makes a new copy of a bitstream.
///\return Pointer to copy of bitstream.
Bitstream* Bitstream::Copy(void)
{
	Bitstream* bcopy = new Bitstream(numBits);	// make a new bitstream of the same size
	*bcopy = *this;		// copy contents of original to new bitstream
	return bcopy;
}


/// Gets the value of a bit from a bitstream.
unsigned int Bitstream::operator[](unsigned int bitIndex /**< index into bitstream (0 is index of first bit) */) const 
{
	unsigned int wordIndex = bitIndex / bitsPerLong;	// word containing the bit
	unsigned int shift     = bitIndex % bitsPerLong;	// bit position within word
	
	// now get the word and shift it so the requested bit is in the LSB.
	// Then mask off the rest and return just a 1 or a 0.
    return ((bits[wordIndex] >> shift) & 1L);
}


/// Clear a bitstream to all zeroes.
void Bitstream::Clear(void)
{
    unsigned int i;

    // count from numLongs down to 1
    for (i = NUM_OF_LONGS(numBits); i > 0; )
    {
        bits[--i] = 0; // decrement before access since index is numLongs-1 to 0
    }
}


/// Sets the value of a bit in a bitstream.
void Bitstream::SetBit( unsigned int bitIndex,	// position to set
					   unsigned int val			// value to set bit
					   )
{
	assert(bitIndex<numBits);

	unsigned int wordIndex = bitIndex / bitsPerLong;  // index of word containing bit
	unsigned long bitMask  = 1L << (bitIndex % bitsPerLong);
	
	bits[wordIndex] &= ~bitMask;  // force selected bit to zero
    if (val)
    {
		bits[wordIndex] |= bitMask; // then set it if the input value is 1
    }
}


/// Sets the value of multiple bits in a bitstream.  End the string of bit values with a number that is neither 0 or 1.
void Bitstream::SetBits( unsigned int bitIndex,	///< position to start changing bits
						int firstBit, ...)		///< list of bit values
{
	int bit;

	// progress through the states until a -1 is seen (this ends the string)
	va_list ap;
    for (va_start(ap, firstBit), bit = firstBit;  bit == 0 || bit == 1; bit = va_arg(ap, int))
    {
        SetBit(bitIndex++, (unsigned int) bit);
    }
	va_end(ap);
}


/// XOR two bitstreams.
///\return the resulting bitstream.
Bitstream& Bitstream::operator^(Bitstream& b2) const
{
    unsigned int numLongs;
    unsigned int i;

    assert(numBits == b2.numBits); // operands must have the same size
	Bitstream& result = *(new Bitstream(numBits));  // result is same size as operands
    numLongs = NUM_OF_LONGS(numBits);
	
	// store a 1 in the result anywhere the two bitstreams differ
    for (i = 0; i < numLongs; i++)
    {
		result.bits[i] = bits[i] ^ b2.bits[i];	// XOR word-by-word
    }
	return result;
}


/// Concatenate two bitstreams.
///\return the concatenated bitstream.
Bitstream& Bitstream::operator+(Bitstream& b2) const
{
	Bitstream& sum = *(new Bitstream(numBits + b2.numBits));
    int i;
    int j;

    for (i = sum.numBits - 1, j = numBits - 1; j >= 0; i--, j--)
    {
        sum.SetBit(i, (*this)[j]);
    }
    assert(j == -1);

    for(j = b2.numBits - 1; j >= 0; i--, j--)
    {
        sum.SetBit(i, b2[j]);
    }
	assert(i== -1);
	assert(j== -1);
	return sum;
}


/// Remove padding bits on the right-end of a bitstream.
void Bitstream::ShiftRight(unsigned int rightShift)
{
    unsigned int i, j;

	// exit if nothing to do
    if (rightShift == 0)
    {
		return;
}
	// first, remove chunks of bits on the right
    unsigned int numLongs = NUM_OF_LONGS(numBits);
	unsigned int numPaddingWords = rightShift / bitsPerLong;
    if (numPaddingWords > 0)
	{
        for (i = 0, j = numPaddingWords; j < numLongs; i++, j++)
		{
			bits[i] = bits[j];
			bits[j] = 0;
		}
	}

	// then right shift bits between chunks to get the final result
	rightShift %= bitsPerLong;
	unsigned long a;
	unsigned int leftShift = bitsPerLong - rightShift;

    for (i = 0; i < numLongs - 1; i++)
	{
		a = bits[i+1];
        bits[i] = (bits[i] >> rightShift) | (a << leftShift);
	}

	// right shift the last chunk
	bits[i] >>= rightShift;
}


// Reverse bits in a long.
static unsigned long ReverseLongBits(unsigned long l)
{
	// Reverse a long byte-by-byte while reversing the bits within each byte.
	unsigned char *p1 = (unsigned char*)&l;						// ptr to first byte of long
	unsigned char *p2 = (unsigned char*)&l + charsPerLong - 1;	// ptr to last byte of long
	unsigned char a;
    unsigned int i, j;

    for (i = 0, j = charsPerLong - 1; i <= j; i++, j--, p1++, p2--)
	{
		// swap bytes and reverse bits within each byte
		a = *p1;
		*p1 = reverseByteBits[*p2];
		*p2 = reverseByteBits[a];
	}
	return l;
}


/// Reverse bit order in a bitstream.
void Bitstream::Reverse(void)
{
    unsigned int i, j;

    if (numBits==0)
    {
		return;
    }
	// Reverse the bitstream chunk-by-chunk while reversing the bits within each chunk.
    unsigned int numLongs = NUM_OF_LONGS(numBits);
	unsigned long a;
    for (i = 0, j = numLongs - 1; i <= j; i++, j--)
	{
		a = bits[i];
		bits[i] = ReverseLongBits(bits[j]);
		bits[j] = ReverseLongBits(a);
	}
	// Remove the extra bits at the end of the original bitstream which are now
	// at the beginning due to the reversal of bit order.
	unsigned int numPaddingBits = bitsPerLong - (numBits % bitsPerLong);

    if (numPaddingBits != 0 && numPaddingBits < bitsPerLong)
    {
		ShiftRight(numPaddingBits);
    }
}


static char hexChar[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/// Converts a bitstream to hexadecimal characters.
///\return pointer to character array of hex characters
unsigned char* Bitstream::ToHexString(void) const
{
    unsigned int i, j;
    int k;

    if (numBits == 0)
    {
		return (unsigned char*)"";
    }

    unsigned int numLongs = NUM_OF_LONGS(numBits);
    unsigned int nChars = ((numBits - 1) / 4 + 1) + 1;  // extra char to hold '0' string terminator
	unsigned char *hexString = new unsigned char[nChars];

    hexString[nChars - 1] = 0; // string terminator
    for (i = 0, k = nChars - 2; i < numLongs; i++)
	{
        unsigned long b;

        b = bits[i];
        for (j = 0; j < 2 * charsPerLong && k >= 0; j++, b >>= 4, k--)
        {
			hexString[k] = hexChar[b & 0xF];
        }
        assert( (i < numLongs - 1 && j == 2 * charsPerLong) || (i == numLongs - 1 && j > 0) );
        assert( (i < numLongs - 1 && k >= 0) || (i == numLongs - 1 && k >= -1));
	}
	return hexString;
}


/// Converts a string of hexadecimal characters into a bitstream.
///< convert hex string into a bitstream with this many bits
///< string of hex digits to be converted into a bitstream
///\return the resulting bitstream
bool Bitstream::FromHexString(unsigned int nBits, const char *hexString)
{
    Resize(nBits);  // sets numBits == nBits

    if (numBits != 0)
    {
        unsigned int nChars = strlen(hexString);
        unsigned int numLongs = NUM_OF_LONGS(numBits);
        unsigned int i, j;
        int k;

        for (i = 0, k = nChars - 1; i < numLongs; i++)
        {
            bits[i] = 0;
            for (j = 0; j < 2 * charsPerLong && k >= 0; k--)
            {
                char c;

                c = toupper(hexString[k]);

                // skip non-hex characters
                if (c >= '0' && c <= 'F')
                {
                    // This must be a long because its used in a 64 bit shift below
                    unsigned long b;

                    // ASCII Hex digit to 4 bit value
                    b = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
                    bits[i] |= b << (j * 4);
                    j++;
                }
            }
            assert( (i < numLongs - 1 && j == 2 * charsPerLong) || (i == numLongs - 1 && j > 0) );
            assert( (i < numLongs - 1 && k >= 0) || (i == numLongs - 1 && k >= -1));
        }
    }
	return true;
}


/// Converts hexadecimal characters from a stream into a bitstream.
///\return true if successful, false if not.
bool Bitstream::FromHexStream(unsigned int nBits, ///< convert hex string into a bitstream with this many bits
					istream& is, ///< stream through which hex characters are received
					bool reverseBits) ///< true if the entire bitstream should be returned with its bit-order reversed
{
	Resize(nBits);  // sets numBits == nBits

	// exit if nothing to do
    if (numBits != 0)
    {
        // read hex characters and store them in reversed-bit format so that most-significant bit of bitstream ends-up in bit position 0
        int nNybbles = (numBits + 3) / 4; // 0 bits=>0 nybbles; 1..4 bits=> 1 nybble; 5..8 bits=>2 nybbles
        int numLongs = NUM_OF_LONGS(numBits);
        int i, j, k;

        j = (nNybbles - 1) % nybblesPerLong;
        for (i = numLongs - 1, k = nNybbles - 1; i >= 0; i--)
        {
            bits[i] = 0;
            while (j >= 0 && k >= 0)
            {
                char c;

                is.read(&c, 1);

                c = toupper(c);

                // skip non-hex characters
                if (c >= '0' && c <= 'F')
                {
                    // This must be a long because its used in a 64 bit shift below
                    unsigned long b;

                    b = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
                    bits[i] |= b << (j * 4);

                    j--;
                    k--;
                }
            }
            j = nybblesPerLong - 1;
//          assert( (i<numLongs-1 && j==nybblesPerLong) || (i==numLongs-1 && j>0) );
//      	assert( (i<numLongs-1 && k>=0) || (i==numLongs-1 && k>=-1));
        }

        if (reverseBits)
        {
            Reverse();
        }
    }
	return true;
}


/// Converts a bitstream to unsigned, bit-packed characters.
///\return pointer to array of bit-packed characters
unsigned char* Bitstream::ToCharString() const
{
    unsigned char *charString;
    unsigned int i;

    charString = NULL;
    if (numBits != 0)
    {
        unsigned int numLongs = NUM_OF_LONGS(numBits);
        unsigned int nChars = (numBits - 1) / 8 + 1;

        charString = new unsigned char[nChars + 1]; // one more for the terminator
        charString[nChars] = 0; // string terminator

        for (i = 0; i < numLongs; i++)
        {
            unsigned long b;
            unsigned int j, k;

            b = bits[i];
            k = i * charsPerLong;
            for (j = 0; (j < charsPerLong) && ((k + j) < nChars); j++, b >>= 8)
            {
                charString[k + j] = b & 0xFF;
            }
            assert( ((i < (numLongs - 1)) && (j == charsPerLong)) || ((i == (numLongs - 1)) && (j > 0)) );
        }
    }
    return charString;
}


/// Converts unsigned, bit-packed characters to a bitstream.
///\return true if successful, false if not.
bool Bitstream::FromCharString(unsigned int nBits,  ///< convert bit-packed character array into a bitstream with this many bits
					const unsigned char *charString) ///< pointer to array of bit-packed characters
{
	Resize(nBits);  // sets numBits == nBits

    if (numBits != 0)
    {
        unsigned int nChars = (numBits-1) / 8 + 1;
        unsigned int numLongs = NUM_OF_LONGS(numBits);
        unsigned int i;

        for (i = 0; i < numLongs; i++)
        {
            unsigned int j, k;

            bits[i] = 0;
            k = i * charsPerLong;
            for (j = 0; (j < charsPerLong) && ((k + j) < nChars); j++)
            {
                unsigned long Value;

                Value = charString[k + j];
                bits[i] |= Value << (j * 8);
            }
        }
	}
	return true;
}


/// Converts a bitstream to unsigned, bit-packed words.
///\return pointer to array of bit-packed words
unsigned int* Bitstream::ToWordString(unsigned int *wordString) const
{
    unsigned int i;

    if (numBits == 0)
    {
		return NULL;   
    }

    unsigned int numLongs = NUM_OF_LONGS(numBits);
	unsigned int nWords = (numBits-1) / 16 + 1;
    if (wordString == NULL)
    {
		wordString = new unsigned [nWords];
    }

    for (i = 0; i < numLongs; i++)
	{
        unsigned long b;
        unsigned int j, k;

        b = bits[i];
        k = i * wordsPerLong;
        for (j = 0; (j < wordsPerLong) && ((k + j) < nWords); j++, b >>= 16)
        {
            wordString[k + j] = b & 0xFFFF;
        }
        assert( ((i < (numLongs - 1)) && (j == wordsPerLong)) || ((i == (numLongs - 1)) && (j > 0)) );
	}
	return wordString;
}

/// Converts unsigned, bit-packed words to a bitstream.
///\return true if successful, false if not.
bool Bitstream::FromWordString(unsigned int nBits,  ///< convert bit-packed word array into a bitstream with this many bits
					const unsigned int *wordString) ///< pointer to array of bit-packed words
{
	Resize(nBits);  // sets numBits == nBits

    if (numBits != 0)
    {
        unsigned int nWords = (numBits-1) / 16 + 1;
        unsigned int numLongs = NUM_OF_LONGS(numBits);
        unsigned int i;

        for (i = 0; i < numLongs; i++)
        {
            unsigned int j, k;

            bits[i] = 0;
            k = i * wordsPerLong;
            for (j = 0; (j < wordsPerLong) && ((k + j) < nWords); j++)
            {
                unsigned long Value;

                Value = wordString[k + j];
                bits[i] |= Value << (j * 16);
            }
        }
    }
	return true;
}


/// Converts unsigned, bit-packed characters from a stream into a bitstream.
///\return true if successful, false if not.
bool Bitstream::FromCharStream(unsigned int nBits, ///< convert bit-packed character stream into a bitstream with this many bits
					istream& is, ///< stream over which bit-packed characters will arrive
					bool reverseBits) ///< true if the entire bitstream should be returned with its bit-order reversed
{
	Resize(nBits);  // sets numBits == nBits

	// exit if nothing to do
    if (numBits != 0)
    {
        // read bit-packed characters and store them in reversed-bit format with MSb in bit position 0
        unsigned int nChars = (numBits - 1) / 8 + 1;
        unsigned int numLongs = NUM_OF_LONGS(numBits);
        unsigned char byte;
        unsigned int i;

        for (i = 0; i < numLongs; i++)
        {
            unsigned int j, k;

            bits[i] = 0;

            k = i * charsPerLong;
            for(j = 0; (j < charsPerLong) && ((k + j) < nChars); j++)
            {
                unsigned long Value;

                is.read((char*)&byte, 1);
                Value = reverseByteBits[byte];
                bits[i] |= Value << (j * 8);
            }
        }

        // since bits are already stored in reversed order, do another reversal if you want them in
        // the unreversed order
        if(!reverseBits)
        {
            Reverse();
        }
    }
	return true;
}


/// Converts a bitstream to a string of 1 and 0 characters.
///\return a string consisting of 1 and 0 characters.
string Bitstream::ToString(void) const
{
    unsigned int i;

	// start printing with the MSB which is stored at the
	// end of the bitstream.  Then work our way to the beginning.
	string s = "";
    for (i = numBits; i > 0; )
    {
        s += ((*this)[--i] == 1 ? "1" : "0");
    }
	return s;
}


/// Converts a string of 1s and 0s to a bitstream.
void Bitstream::FromString(const string s)
{
    unsigned int i, j;

	Resize(s.length());

	// the last char in the string is the LSB of the bitstream
    for (i = numBits - 1, j = 0; i >= 0; i--)
	{
        // skip whitespace
        if (!isspace(s[i]))
        {
            this->SetBit(j++, s[i]=='0' ? 0 : 1);
        }
	}
}


/// Prints a bitstream to an output stream.
///\return reference to the output stream
ostream& Bitstream::PrintBits(ostream& os) const
{
	os << ToString().c_str() << endl;
	return os;
}


/// Helper function which prints a bitstream to an output stream.
///\return reference to the output stream
ostream& operator<<(ostream& os, Bitstream& b)
{
	return b.PrintBits(os);
}
