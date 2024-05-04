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


#ifndef XC3SPORT_H
#define XC3SPORT_H


#include "progress.h"
#include "cnfgport.h"


/**
Download configuration bitstreams into a Spartan3 FPGA.

This object provides methods for downloading a configuration bitstream 
into a Spartan3 FPGA. The bitstream file is opened and the field 
containing the configuration bits is extracted and passed byte-by-byte 
into the FPGA. The download operates in one of two modes: fast mode 
where each byte is downloaded through the parallel port and into the 
Spartan3 FPGA in SelectMap configuration mode, or slow mode where each 
byte is serially transmitted through a single pin of the parallel port 
into the FPGA in slave-serial configuration mode.

*/
class XC3SPort : public CnfgPort
{
	public:

	XC3SPort(void);

	XC3SPort(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int posCCLK, unsigned int posPROG, unsigned int posDlo,
		unsigned int posDhi, unsigned int posDONE);

	bool Setup(XSError* e, unsigned int portNum, unsigned int invMask,
		unsigned int posCCLK, unsigned int posPROG, unsigned int posDlo,
		unsigned int posDhi, unsigned int posDONE);

	string GetChipType(istream& is);

    string GetChipType(const char *bitfileName);

	void InitConfigureFPGA(void);

	void EnableFastDownload(bool t);

	void ConfigureFPGA(unsigned char b);

    bool ConfigureFPGA(istream& is, bool *UserCancelled);

    bool ConfigureFPGA(const char *fileName, bool *UserCancelled);


	protected:

	Progress *progressGauge;	///< indicates progress of operations
	unsigned int posDLO;		///< lower bit position in parallel port of config. data pins
	unsigned int posDHI;		///< upper bit position in parallel port of config. data pins


	private:

	string chipType;		///< type of chip (e.g. 4005XLPC84)
	bool fastDownload;		///< enable fast downloading of Spartan3 bitstreams
};

#endif
