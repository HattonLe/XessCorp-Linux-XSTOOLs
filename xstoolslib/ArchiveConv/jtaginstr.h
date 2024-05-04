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

///\unit

#ifndef JTAGINSTR_H
#define JTAGINSTR_H

/**
Definitions of commands sent after a USER JTAG instruction.
*/
#define	INSTR_NOP					 "00000000" // no operation
#define	INSTR_RUN_DIAG				"000000011" // run board diagnostic; must be one bit longer than normal
#define	INSTR_RAM_WRITE				 "00000101" // write data to RAM
#define	INSTR_RAM_READ				 "00000111" // read data from RAM
#define	INSTR_RAM_SIZE				"000001001" // get RAM organization; must be one bit longer than normal
#define	INSTR_FLASH_ERASE			"000001011" // erase entire Flash chip; must be one bit longer than normal 
#define	INSTR_FLASH_PGM				 "00001101" // program downloaded block of data into Flash
#define	INSTR_FLASH_BLK_PGM			 "00001111" // program downloaded block of data into Flash
#define	INSTR_FLASH_READ			 "00010001" // read data from Flash
#define	INSTR_FLASH_SIZE			"000010011" // get Flash organization; must be one bit longer than normal
#define	INSTR_REG_WRITE				 "00010101" // write data to registers
#define	INSTR_REG_READ				 "00010111" // read data from registers
#define	INSTR_REG_SIZE				"000011001" // get register organization; must be one bit longer than normal
#define INSTR_CAPABILITIES			"011111111" // get capabilities of instruction execution unit; must be one bit longer than normal

// The length of the TDO register used to return information to the PC
#define TDO_LENGTH					32

// Possible capabilities for the instruction execution unit.
// The lower and upper bytes are mirrors of each other, and bits are set
// in the middle two bytes to indicate if a given capability is present.
#define NO_CAPABILITIES				0xA50000A5
#define CAPABLE_RUN_DIAG_BIT		8
#define CAPABLE_RUN_DIAG_MASK		(1<<CAPABLE_RUN_DIAG_BIT)
#define CAPABLE_RAM_WRITE_BIT		9
#define CAPABLE_RAM_WRITE_MASK		(1<<CAPABLE_RAM_WRITE_BIT)
#define CAPABLE_RAM_READ_BIT		10
#define CAPABLE_RAM_READ_MASK		(1<<CAPABLE_RAM_READ_BIT)
#define CAPABLE_FLASH_WRITE_BIT		11
#define CAPABLE_FLASH_WRITE_MASK	(1<<CAPABLE_FLASH_WRITE_BIT)
#define CAPABLE_FLASH_READ_BIT		12
#define CAPABLE_FLASH_READ_MASK		(1<<CAPABLE_FLASH_READ_BIT)
#define CAPABLE_REG_WRITE_BIT		13
#define CAPABLE_REG_WRITE_MASK		(1<<CAPABLE_REG_WRITE_BIT)
#define CAPABLE_REG_READ_BIT		14
#define CAPABLE_REG_READ_MASK		(1<<CAPABLE_REG_READ_BIT)

// Status codes from the instruction execution unit.
#define OP_INPROGRESS				"01230123"
#define OP_PASSED					"45674567"
#define OP_FAILED					"89AB89AB"

#endif
