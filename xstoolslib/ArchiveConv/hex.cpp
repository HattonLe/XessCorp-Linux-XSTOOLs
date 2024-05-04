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


#include <cassert>
#include <ctype.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include "hex.h"
#include "utils.h"


/// Construct a hex number with a given number of hex digits.
Hex::Hex(unsigned int l) ///< Number of hex digits in the data
{
	SetLength(l);	// set number of hex digits in hex number
	SetHex(0);		// initialize hex number to 0
}


/// Set the number of hex digits in the hex number.
void Hex::SetLength(unsigned int l) ///< Number of hex digits in the data
{
	assert(sizeof(unsigned int) * 2 >= l);
	length = l;
}


/// Return the number of hex digits in the hex number.
unsigned int Hex::GetLength(void)
{
	return length;
}


/// Assign a value to the hex number.
void Hex::SetHex(unsigned int h) ///< value to assign to hex number
{
	hex = h;
}


/// Assign a value to the hex number.
///\return a reference to the Hex object whose value was updated
Hex& Hex::operator=(unsigned int h) ///< value to assign to hex number
{
	hex = h;
	return *this;
}


/// Return the value of the hex number.
unsigned int Hex::GetHex(void)
{
	return hex;
}


/// Helper function which gets a hex number from an input stream.
///\return a reference to the input stream
istream& operator>> (istream& is,	///< stream through which a hex number is received
					Hex& n)			///< Hex object that will receive the data from the stream
{
	char buf[100];
	is >> setw(n.GetLength()+1) >> buf;
	n.SetHex(0);
	bool started = false;
	for(char *p=buf; *p!='\0'; p++)
	{
		if(isspace(*p))
		{
			if(started) break; // stop once numerals end
			else continue;     // keep looking for start of numerals
		}
		started = true;
		n.SetHex(n.GetHex()*16 + CharToHex(*p));
	}
	return is;
}


/// Helper function which outputs a hex number to an output stream.
///\return a reference to the output stream
ostream& operator<< (ostream& os,	///< stream through which a hex number is sent
					Hex& n)			///< Hex object whose value is sent through the stream
{
	os << n.GetHex();
	return os;
}
