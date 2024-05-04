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


#include <string>
using namespace std;

#include "utils.h"
#include "cnfgport.h"


// Create a config controller port.
CnfgPort::CnfgPort(void)
{
	;
}


// Create a config controller port.
CnfgPort::CnfgPort(XSError* e,	///< pointer to error reporting object
				   unsigned int portNum, ///< parallel port number
				   unsigned int invMask, ///< inversion mask for the parallel port
				   unsigned int pos_cclk, ///< bit position in parallel port of CCLK pin
				   unsigned int pos_prog, ///< bit position in parallel port of PROGRAM pin
				   unsigned int pos_din, ///< bit position in parallel port of DIN pin
				   unsigned int pos_done) ///< bit position in parallel port of DONE pin
{
	Setup(e,portNum,invMask,pos_cclk,pos_prog,pos_din,pos_done);
}


// Setup a config controller port
bool CnfgPort::Setup(XSError* e, ///< pointer to error reporting object
				   unsigned int portNum, ///< parallel port number
				   unsigned int invMask, ///< inversion mask for the parallel port
				   unsigned int pos_cclk, ///< bit position in parallel port of CCLK pin
				   unsigned int pos_prog, ///< bit position in parallel port of PROGRAM pin
				   unsigned int pos_din, ///< bit position in parallel port of DIN pin
				   unsigned int pos_done) ///< bit position in parallel port of DONE pin
{
	posCCLK = pos_cclk;
	posPROG = pos_prog;
	posDIN = pos_din;
	posDONE = pos_done;
	return PPort::Setup(e,portNum,invMask);	// return false if error occurs
}


// Set the value of the CCLK pin.
void CnfgPort::SetCCLK(unsigned int b)
{
	Out(b,posCCLK,posCCLK);
}


// Get the value output on the CCLK pin.
unsigned int CnfgPort::GetCCLK(void)
{
	return In(posCCLK,posCCLK);
}


// Toggle the CCLK pin twice (return it to its original level).
void CnfgPort::PulseCCLK(void)
{
	assert(GetCCLK()==0);  // quiescent state of CCLK should be zero
	SetCCLK(1);  // toggle TCK output
	SetCCLK(0);	// toggle it again
}


// Set the value of the PROGRAM pin.
void CnfgPort::SetPROG(unsigned int b)
{
	Out(b,posPROG,posPROG);
}


// Get the value output on the PROGRAM pin.
unsigned int CnfgPort::GetPROG(void)
{
	return In(posPROG,posPROG);
}


// Toggle the configuration initiation pin (return it to its original level).
void CnfgPort::PulsePROG(void)
{
	assert(GetPROG()==1);	// quiescent state of /PROGRAM should be logic high level
	SetPROG(0);				// toggle /PROGRAM output
	InsertDelay(10,MILLISECONDS);
	SetPROG(1);				// toggle it again
	InsertDelay(10,MILLISECONDS);
}


// Set the value of the DIN pin.
void CnfgPort::SetDIN(unsigned int b)
{
	Out(b,posDIN,posDIN);
}


// Get the value output on the DIN pin.
unsigned int CnfgPort::GetDIN(void)
{
	return In(posDIN,posDIN);
}


// Get the value of the DONE pin.
unsigned int CnfgPort::GetDONE(void)
{
	return In(posDONE,posDONE);
}
