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

#ifndef AT17PORT_H
#define AT17PORT_H

#include "progress.h"
#include "hexrecrd.h"
#include "pport.h"


/**
Downloading of configuration bitstreams into an AT17CXXX serial EEPROM on the XS40 Board.

This object provides methods for downloading a configuration bitstream 
to an Atmel AT17CXXX serial EEPROM. The bitstream file is opened and the 
field containing the configuration bits is extracted and partitioned 
into 64-byte page. Each page of data is transmitted serially through the 
parallel port where the FPGA connects the clock and data streams to the 
programming pins of the AT17CXXX chip. 

*/
class AT17Port : public PPort
{
	public:

	 AT17Port(void);

	 AT17Port(
			XSError* e,	
			unsigned int portNum,	
			unsigned int invMask,	
			unsigned int pos_clk,	
			unsigned int pos_eece,	
			unsigned int pos_eeoe,	
			unsigned int pos_din		
			);

	int Setup(
			XSError* e,	
			unsigned int portNum,	
			unsigned int invMask,	
			unsigned int pos_clk,	
			unsigned int pos_eece,	
			unsigned int pos_eeoe,	
			unsigned int pos_din		
			);

	bool ProgramEEPROM(string& bitfileName);

	bool ProgramEEPROM(istream& is);

	void SendEEPROMByte(unsigned char byte);

	void ProgramEEPROMPage(
			unsigned char* buf,	
			unsigned int pageAddr,	
			unsigned int len		
			);


	protected:

	Progress *progressGauge;	///< indicates progress of operations

	
	private:

	unsigned int posCLK;	///< bit position in parallel port of EEPROM clock pin
	unsigned int posEECE;	///< bit position in parallel port of EEPROM chip-enable
	unsigned int posEEOE;	///< bit position in parallel port of EEPROM output-enable
	unsigned int posDIN;	///< bit position in parallel port of EEPROM serial data input
};

#endif
