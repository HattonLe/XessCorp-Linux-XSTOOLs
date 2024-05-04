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


#ifndef XC4KPORT_H
#define XC4KPORT_H


#include "progress.h"
#include "cnfgport.h"


/**
Download configuration bitstreams into an XC4000 FPGA.

This object provides methods for downloading a configuration bitstream 
into an XC4000 FPGA. The bitstream file is opened and the field 
containing the configuration bits is extracted and passed byte-by-byte 
into the FPGA. Each byte is serially transmitted through a single pin of 
the parallel port into the FPGA in slave-serial configuration mode. 

*/
class XC4KPort : public CnfgPort
{
	public:

	XC4KPort(void);

	XC4KPort(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int posCCLK, unsigned int posPROG, unsigned int posDIN,
		unsigned int posDONE);

	bool Setup(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int posCCLK, unsigned int posPROG, unsigned int posDIN,
		unsigned int posDONE);

	string GetChipType(istream& is);

	string GetChipType(string& bitfileName);

	void InitConfigureFPGA(void);

	void ConfigureFPGA(unsigned char b);

	bool ConfigureFPGA(istream& is);

	bool ConfigureFPGA(string& fileName);


	protected:

	Progress *progressGauge;	///< indicates progress of operations
	

	private:

	string chipType;		///< type of chip (e.g. 4005XLPC84)
};

#endif
