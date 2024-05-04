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


#ifndef XC95KPORT_H
#define XC95KPORT_H

#include "lptjtag.h"


/**
 Object for manipulating an XS95 Board.
 
 This object specializes the JTAGPort object by adding a method that 
reads SVF files and sends the configuration commands they contain to the 
XC9500 CPLD through the parallel port. Methods are also provided for 
reading the contents of the device ID and USERCODE signature registers 
in the XC9500 CPLD. 

*/
class XC95KPort : public LPTJTAG
{
	public:

	XC95KPort(void);

	void SetErr(XSError* e) { JTAGPort::SetErr(e); }

	XSError& GetErr(void) { return JTAGPort::GetErr(); }

	XC95KPort(XSError* err,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int posTCK,
		unsigned int posTMS,
		unsigned int posTDI,
		unsigned int posTDO);

	bool Setup(XSError* err,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int posTCK,
		unsigned int posTMS,
		unsigned int posTDI,
		unsigned int posTDO);

	string GetChipID();

	string GetUSERCODE();

	void InitConfigureCPLD(void);

	void ConfigureCPLD(unsigned char b);

	bool ConfigureCPLD(istream& is, string& fileName);

	bool ConfigureCPLD(string& fileName);

	
	private:

	string chipType;		///< type of chip (e.g. XC95108)

};

#endif
