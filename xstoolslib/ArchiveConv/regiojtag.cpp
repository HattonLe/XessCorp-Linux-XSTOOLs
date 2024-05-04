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

#include "utils.h"
#include "jtaginstr.h"
#include "regiojtag.h"


/// Create a port for accessing registers via JTAG.
RegioJTAG::RegioJTAG(void)
{
	;
}


/// Create a port for accessing registers via JTAG.
RegioJTAG::RegioJTAG(XSError* e,	///< error reporting channel 
	unsigned int portNum,			///< USB port number
	unsigned int endptNum,			///< USB endpoint number
	string brdModel					///< type of XESS Board attached thru USB
	): USBJTAG(e,portNum,endptNum)
{
	Setup(e,portNum,endptNum,brdModel);
}


/// Initialize the members of the object.
///\return true if the setup is successful.
bool RegioJTAG::Setup(XSError* e,	///< error reporting channel 
	unsigned int portNum,			///< USB port number
	unsigned int endptNum,			///< USB endpoint number
	string brdModel)				///< type of XESS Board attached thru USB
{

	// get information about all the models of XS boards
	XSBoardInfo *brdInfo;
	int numBoards = GetXSBoardInfo(&brdInfo);

	// use the board model name to find the corresponding index into the list of boards
	int brdIndex;
	for(brdIndex=0; brdIndex<numBoards; brdIndex++)
	{
		if(strcmp(brdModel.c_str(),brdInfo[brdIndex].brdModel) == 0)
			break;
	}
	if(brdIndex>=numBoards)
	{
		GetErr().SimpleMsg(XSErrorMajor,"unknown type of XS Board!\n");
		return false;
	}
	userInstruction = brdInfo[brdIndex].port[PORTTYPE_USBJTAG].userInstruction;

	Bitstream null(0), instr(0), data(0);

	// load the USER instruction
	InitTAP();
	GotoTAPState(ShiftIR);
	instr.FromString(userInstruction);
	SendRcvBitstream(instr,null);

	// go to the SHIFT-DR state where the RAM interface circuitry can be controlled
	GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);

	// download the instruction that gets the interface capabilities from the FPGA
	instr.FromString(INSTR_CAPABILITIES);
	SendRcvBitstream(instr,null);

	// readback the capabilities of the interface
	data.Resize(TDO_LENGTH);
	GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	SendRcvBitstream(null,data);

	// check the capabilities to see if RAM writes are supported
	bool regIntfcAlreadyLoaded = hasCapability(data,CAPABLE_RAM_WRITE_BIT);
	if(!regIntfcAlreadyLoaded)
	{
		GetErr().SimpleMsg(XSErrorMajor,"register interface is missing!!\n");
		return false;
	}

	// go to the SHIFT-DR state where the reg interface circuitry can be controlled
	GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);

	// download the instruction that gets the reg organization from the FPGA
	instr.FromString(INSTR_RAM_SIZE);
	SendRcvBitstream(instr,null);

	// readback the sizes of the register address and data buses
	data.Resize(TDO_LENGTH);
	GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	SendRcvBitstream(null,data);
	unsigned char *sizes = data.ToCharString();
	dataWidth   = sizes[0];
	addrWidth   = sizes[1];
	delete [] sizes;

	DEBUG_STMT(" ")
	DEBUG_STMT("dataWidth   = " << dataWidth)
	DEBUG_STMT("addrWidth   = " << addrWidth)
	return true;
}


/// Write a register with data via JTAG.
void RegioJTAG::Write(unsigned long addr,	///< address of register to write
	unsigned long data,						///< data to write to register
	bool startup,
	bool teardown)
{
	int i;
	unsigned char oprnds[20];			// general storage for various operands
	Bitstream null(0), instr(0), cnt(0), reg_data(0), reg_addr(0), payload(0);

	if(startup)
	{
		// load the USER instruction
		InitTAP();
		GotoTAPState(ShiftIR);
		instr.FromString(userInstruction);
		SendRcvBitstream(instr,null);

		// go to the SHIFT-DR state where the register interface circuitry can be controlled
		GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);
	}

	instr.FromString(INSTR_RAM_WRITE);	// prepare instruction for downloading data to registers

	// prepare the number of words that will be downloaded to the register file
	int numWords = 1;
	for(i=0; i<(addrWidth+7)/8; i++,numWords>>=8)
		oprnds[i] = numWords & 0xFF;
	cnt.FromCharString(addrWidth,oprnds);

	// prepare the address of the register
	unsigned raddr = addr;
	for(i=0; i<(addrWidth+7)/8; i++,raddr>>=8)
		oprnds[i] = raddr & 0xFF;
	reg_addr.FromCharString(addrWidth,oprnds);

	// prepare the data that will be downloaded to the register
	unsigned rdata = data;
	for(i=0; i<(dataWidth+7)/8; i++,rdata>>=8)
		oprnds[i] = rdata & 0xFF;
	reg_data.FromCharString(dataWidth,oprnds);

	// create the instruction for downloading the data to the selected register
	payload = reg_data + cnt + reg_addr + instr;

	// send the instruction and the data
	SendRcvBitstream(payload,null);

	if(teardown)
	{
		// Go back to the test-logic-reset state
		GoThruTAPSequence(PauseDR,Exit2DR,UpdateDR,SelectDRScan,SelectIRScan,TestLogicReset,-1);
	}
}


/// Read data from a register via JTAG.
///\return data read from register
unsigned long RegioJTAG::Read(unsigned long addr,	///< address of register to read
	bool startup,
	bool teardown)
{
	int i;
	unsigned char oprnds[20];			// general storage for various operands
	Bitstream null(0), instr(0), cnt(0), reg_data(0), reg_addr(0), payload(0);

	if(startup)
	{
		// load the USER instruction
		InitTAP();
		GotoTAPState(ShiftIR);
		instr.FromString(userInstruction);
		SendRcvBitstream(instr,null);

		// go to the SHIFT-DR state where the register interface circuitry can be controlled
		GoThruTAPSequence(UpdateIR,SelectDRScan,CaptureDR,ShiftDR,-1);
	}

	instr.FromString(INSTR_RAM_READ);	// prepare instruction for uploading data from registers

	// prepare the number of words that will be uploaded from the register file
	int numWords = 1;
	for(i=0; i<(addrWidth+7)/8; i++,numWords>>=8)
		oprnds[i] = numWords & 0xFF;
	cnt.FromCharString(addrWidth,oprnds);

	// prepare the address of the register
	unsigned raddr = addr;
	for(i=0; i<(addrWidth+7)/8; i++,raddr>>=8)
		oprnds[i] = raddr & 0xFF;
	reg_addr.FromCharString(addrWidth,oprnds);

	// create the instruction for uploading the data from the selected register
	payload = cnt + reg_addr + instr;

	// Prepare a bitstream to hold the data from the register.
	// The data will be in the last dataWidth bits of the bitstream.
	reg_data.Resize(instr.GetLength()+addrWidth+addrWidth+dataWidth);

	// send the instruction and get the data from the register
	SendRcvBitstream(payload,reg_data);

	if(teardown)
	{
		// Go back to the test-logic-reset state
		GoThruTAPSequence(PauseDR,Exit2DR,UpdateDR,SelectDRScan,SelectIRScan,TestLogicReset,-1);
	}

	// shift the register data to the least-significant bits of the bitstream
	reg_data.ShiftRight(instr.GetLength()+addrWidth+addrWidth);

	// the register data is in the lower bits of the bitstream
	unsigned char *d = reg_data.ToCharString();
	unsigned long data = 0L;
	for(i=(dataWidth+7)/8 - 1; i>=0; i--)
		data = (data<<8) | d[i];
	unsigned long mask = (1L << dataWidth)-1;
	data &= mask;
	return data;
}

