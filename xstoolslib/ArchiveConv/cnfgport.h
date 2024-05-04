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


#ifndef CNFGPORT_H
#define CNFGPORT_H

#include <cassert>
#include <cstdarg>

#include "pport.h"


/**
Controls the configuration pins of an FPGA.

This object provides methods for setting and querying the pins of the 
parallel port connected to the CCLK, DIN, PROG and DONE pins of the FPGA 
on an XS Board. The association between each FPGA configuration pin and 
its bit position in the 24-bit parallel port register is made when the 
CnfgPort object is instantiated.
*/ 

class CnfgPort : public PPort
{
	public:

	CnfgPort(void);

	CnfgPort(XSError* e,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int pos_cclk,
		unsigned int pos_prog,
		unsigned int pos_din,
		unsigned int pos_done);

	bool Setup(XSError* e,
		unsigned int portNum,
		unsigned int invMask,
		unsigned int pos_cclk,
		unsigned int pos_prog,
		unsigned int pos_din,
		unsigned int pos_done);

	void SetCCLK(unsigned int b);

	unsigned int GetCCLK(void);

	void PulseCCLK(void);

	void SetPROG(unsigned int b);

	unsigned int GetPROG(void);

	void PulsePROG(void);

	void SetDIN(unsigned int b);

	unsigned int GetDIN(void);

	unsigned int GetDONE(void);

	
	private:

	unsigned int posCCLK;	///< position of configuration clock pin
	unsigned int posPROG;	///< position of configuration initiation pin 
	unsigned int posDIN;	///< position of configuration data pin
	unsigned int posDONE;	///< position of configuration done pin

};

#endif
