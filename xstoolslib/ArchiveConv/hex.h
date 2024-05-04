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


#ifndef HEX_H
#define HEX_H

#include <iostream>
using namespace std;


/**
Stores hexadecimal data with an arbitrary number of digits.

This object stores a hexadecimal number consisting of eight hex digits or less. It is mainly used when reading a data record from an Intel, Motorola or XESS HEX file.
*/
class Hex
{
	public:

	Hex(unsigned int l=4);

	void SetLength(unsigned int l);

	unsigned int GetLength(void);

	void SetHex(unsigned int h);

	Hex& operator=(unsigned int h);

	unsigned int GetHex(void);

	
	private:

	unsigned int hex;		///< storage for data
	unsigned int length;	///< number of hex digits in the data
};


unsigned int CharToHex(char c);

istream& operator>> (istream& is, Hex& n);

ostream& operator<< (ostream& os, Hex& n);

#endif
