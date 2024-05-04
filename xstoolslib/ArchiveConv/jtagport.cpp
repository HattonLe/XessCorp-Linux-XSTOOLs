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


#include <string>
#include <fstream>
#include <cassert>
#include <ctime>
#include <cstdarg>
using namespace std;

#include "utils.h"
#include "jtagport.h"

#define	CONFIG_SETTLE_TIME	30,MILLISECONDS

static const int maxNumDevices = 50;	// maximum number of devices in a JTAG chain


// The following array stores the transition table for the TAP
// controller.  It tells us what the next state will be by placing the
// current state code in the first index and the value of
// the TMS input in the second index.
static TAPState nextTAPState[16][2] =
{
	// TMS=0                 TMS=1            CURRENT TAP STATE
	{ RunTestIdle,        TestLogicReset },     // TestLogicReset
	{ RunTestIdle,        SelectDRScan   },     // RunTestIdle
	{ CaptureDR,          SelectIRScan   },     // SelectDRScan
	{ CaptureIR,          TestLogicReset },     // SelectIRScan
	{ ShiftDR,            Exit1DR        },     // CaptureDR
	{ ShiftIR,            Exit1IR        },     // CaptureIR
	{ ShiftDR,            Exit1DR        },     // ShiftDR
	{ ShiftIR,            Exit1IR        },     // ShiftIR
	{ PauseDR,            UpdateDR       },     // Exit1DR
	{ PauseIR,            UpdateIR       },     // Exit1IR
	{ PauseDR,            Exit2DR        },     // PauseDR
	{ PauseIR,            Exit2IR        },     // PauseIR
	{ ShiftDR,            UpdateDR       },     // Exit2DR
	{ ShiftIR,            UpdateIR       },     // Exit2IR
	{ RunTestIdle,        SelectDRScan   },     // UpdateDR
	{ RunTestIdle,        SelectDRScan   },     // UpdateIR
};


// The following array stores the TAP state path to get from the
// first state to the final state.  The first state is entered as the first index
// to this array, the final state is entered as the second index to the array,
// and the third index is used to step through the intermediate states until
// a -1 is found indicating the end of the sequence.
static int TAPPath[16][16][17] =
{
	// From TestLogicReset ...
	{
		// ... to TestLogicReset
		{ -1 },
		// ... to RunTestIdle
		{ RunTestIdle, -1 },
		// ... to SelectDRScan
		{ RunTestIdle, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ RunTestIdle, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ RunTestIdle, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ RunTestIdle, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ RunTestIdle, SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ RunTestIdle, SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ RunTestIdle, SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ RunTestIdle, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ RunTestIdle, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ RunTestIdle, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ RunTestIdle, SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ RunTestIdle, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ RunTestIdle, SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ RunTestIdle, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From RunTestIdle ...
	{
		// ... to TestLogicReset
		{ SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ -1 },
		// ... to SelectDRScan
		{ SelectDRScan, -1 },
		// ... to SelectIRScan
		{ SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From SelectDRScan ...
	{
		// ... to TestLogicReset
		{ SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ SelectIRScan, TestLogicReset, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ -1 },
		// ... to SelectIRScan
		{ SelectIRScan, -1 },
		// ... to CaptureDR
		{ CaptureDR, -1 },
		// ... to CaptureIR
		{ SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From SelectIRScan ...
	{
		// ... to TestLogicReset
		{ TestLogicReset, -1 },
		// ... to RunTestIdle
		{ TestLogicReset, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ TestLogicReset, RunTestIdle, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ -1 },
		// ... to CaptureDR
		{ TestLogicReset, RunTestIdle, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ CaptureIR, -1 },
		// ... to ShiftDR
		{ TestLogicReset, RunTestIdle, SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ TestLogicReset, RunTestIdle, SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ TestLogicReset, RunTestIdle, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ TestLogicReset, RunTestIdle, SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ TestLogicReset, RunTestIdle, SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From CaptureDR ...
	{
		// ... to TestLogicReset
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ Exit1DR, UpdateDR, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ Exit1DR, UpdateDR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ -1 },
		// ... to CaptureIR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ ShiftDR, -1 },
		// ... to ShiftIR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ Exit1DR, -1 },
		// ... to Exit1IR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From CaptureIR ...
	{
		// ... to TestLogicReset
		{ Exit1IR, UpdateIR, SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ Exit1IR, UpdateIR, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ Exit1IR, UpdateIR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ Exit1IR, UpdateIR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ -1 },
		// ... to ShiftDR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ ShiftIR, -1 },
		// ... to Exit1DR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ Exit1IR, -1 },
		// ... to PauseDR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ Exit1IR, UpdateIR, -1 },
	},
	// From ShiftDR ...
	{
		// ... to TestLogicReset
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ Exit1DR, UpdateDR, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ Exit1DR, UpdateDR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ Exit1DR, UpdateDR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ -1 },
		// ... to ShiftIR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ Exit1DR, -1 },
		// ... to Exit1IR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ Exit1DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From ShiftIR ...
	{
		// ... to TestLogicReset
		{ Exit1IR, UpdateIR, SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ Exit1IR, UpdateIR, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ Exit1IR, UpdateIR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ Exit1IR, UpdateIR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ Exit1IR, UpdateIR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ -1 },
		// ... to Exit1DR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ Exit1IR, -1 },
		// ... to PauseDR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ Exit1IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ Exit1IR, UpdateIR, -1 },
	},
	// From Exit1DR ...
	{
		// ... to TestLogicReset
		{ UpdateDR, SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ UpdateDR, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ UpdateDR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ UpdateDR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ UpdateDR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ UpdateDR, SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ -1 },
		// ... to Exit1IR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ PauseDR, -1 },
		// ... to PauseIR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ UpdateDR, -1 },
		// ... to UpdateIR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From Exit1IR ...
	{
		// ... to TestLogicReset
		{ UpdateIR, SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ UpdateIR, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ UpdateIR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ UpdateIR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ UpdateIR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ UpdateIR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ UpdateIR, SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ UpdateIR, SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ -1 },
		// ... to PauseDR
		{ UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ PauseIR, -1 },
		// ... to Exit2DR
		{ UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ UpdateIR, SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ UpdateIR, -1 },
	},
	// From PauseDR ...
	{
		// ... to TestLogicReset
		{ Exit2DR, UpdateDR, SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ Exit2DR, UpdateDR, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ Exit2DR, UpdateDR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ Exit2DR, UpdateDR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ Exit2DR, UpdateDR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ Exit2DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ Exit2DR, ShiftDR, -1 },
		// ... to ShiftIR
		{ Exit2DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ Exit2DR, ShiftDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ Exit2DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ Exit2DR, UpdateDR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ Exit2DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ Exit2DR, -1 },
		// ... to Exit2IR
		{ Exit2DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ Exit2DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ Exit2DR, UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From PauseIR ...
	{
		// ... to TestLogicReset
		{ Exit2IR, UpdateIR, SelectDRScan, SelectIRScan, TestLogicReset, -1},
		// ... to RunTestIdle
		{ Exit2IR, UpdateIR, RunTestIdle, -1},
		// ... to SelectDRScan
		{ Exit2IR, UpdateIR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ Exit2IR, UpdateIR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ Exit2IR, UpdateIR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ Exit2IR, UpdateIR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ Exit2IR, UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ Exit2IR, ShiftIR, -1},
		// ... to Exit1DR
		{ Exit2IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ Exit2IR, ShiftIR, Exit1IR, -1 },
		// ... to PauseDR
		{ Exit2IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ Exit2IR, UpdateIR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ Exit2IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ Exit2IR, -1},
		// ... to UpdateDR
		{ Exit2IR, UpdateIR, SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ Exit2IR, UpdateIR, -1 },
	},
	// From Exit2DR ...
	{
		// ... to TestLogicReset
		{ UpdateDR, SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ UpdateDR, RunTestIdle, -1 },
		// ... to SelectDRScan
		{ UpdateDR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ UpdateDR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ UpdateDR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ ShiftDR, -1 },
		// ... to ShiftIR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ ShiftDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ UpdateDR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ -1 },
		// ... to Exit2IR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ UpdateDR, -1 },
		// ... to UpdateIR
		{ UpdateDR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From Exit2IR ...
	{
		// ... to TestLogicReset
		{ UpdateIR, SelectDRScan, SelectIRScan, TestLogicReset, -1},
		// ... to RunTestIdle
		{ UpdateIR, RunTestIdle, -1},
		// ... to SelectDRScan
		{ UpdateIR, SelectDRScan, -1 },
		// ... to SelectIRScan
		{ UpdateIR, SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ UpdateIR, SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ UpdateIR, SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ UpdateIR, SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ ShiftIR, -1},
		// ... to Exit1DR
		{ UpdateIR, SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ ShiftIR, Exit1IR, -1 },
		// ... to PauseDR
		{ UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ UpdateIR, SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ UpdateIR, SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ -1},
		// ... to UpdateDR
		{ UpdateIR, SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ UpdateIR, -1 },
	},
	// From UpdateDR ...
	{
		// ... to TestLogicReset
		{ SelectDRScan, SelectIRScan, TestLogicReset, -1 },
		// ... to RunTestIdle
		{ RunTestIdle, -1 },
		// ... to SelectDRScan
		{ SelectDRScan, -1 },
		// ... to SelectIRScan
		{ SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1 },
		// ... to Exit1DR
		{ SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, -1 },
		// ... to PauseDR
		{ SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1 },
		// ... to UpdateDR
		{ -1 },
		// ... to UpdateIR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, UpdateIR, -1 },
	},
	// From UpdateIR ...
	{
		// ... to TestLogicReset
		{ SelectDRScan, SelectIRScan, TestLogicReset, -1},
		// ... to RunTestIdle
		{ RunTestIdle, -1},
		// ... to SelectDRScan
		{ SelectDRScan, -1 },
		// ... to SelectIRScan
		{ SelectDRScan, SelectIRScan, -1 },
		// ... to CaptureDR
		{ SelectDRScan, CaptureDR, -1 },
		// ... to CaptureIR
		{ SelectDRScan, SelectIRScan, CaptureIR, -1 },
		// ... to ShiftDR
		{ SelectDRScan, CaptureDR, ShiftDR, -1 },
		// ... to ShiftIR
		{ SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, -1},
		// ... to Exit1DR
		{ SelectDRScan, CaptureDR, Exit1DR, -1 },
		// ... to Exit1IR
		{ SelectDRScan, SelectIRScan, CaptureIR, ShiftIR, Exit1IR, -1 },
		// ... to PauseDR
		{ SelectDRScan, CaptureDR, Exit1DR, PauseDR, -1 },
		// ... to PauseIR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, -1 },
		// ... to Exit2DR
		{ SelectDRScan, CaptureDR, Exit1DR, PauseDR, Exit2DR, -1 },
		// ... to Exit2IR
		{ SelectDRScan, SelectIRScan, CaptureIR, Exit1IR, PauseIR, Exit2IR, -1},
		// ... to UpdateDR
		{ SelectDRScan, CaptureDR, Exit1DR, UpdateDR, -1 },
		// ... to UpdateIR
		{ -1 },
	},
};


/// Create a JTAG controller port.
JTAGPort::JTAGPort(void)
{
	progressGauge = NULL;
	Setup((XSError*)NULL,false,cerr);
}


/// Create a JTAG controller port.
JTAGPort::JTAGPort(XSError* e)
{
	progressGauge = NULL;
	Setup(e,false,cerr);
}


/// Initialize the object.
bool JTAGPort::Setup(XSError *e,	///< error reporting channel 
			bool traceOnFlag, 		///< if true, output trace information on JTAG operations
			ostream& traceOstream)	///< send trace messages to this output stream
{
	currentTAPState = InvalidTAPState;
	SetErr(e);
	SetTraceOnOff(traceOnFlag,traceOstream);
	if(progressGauge != NULL)
		delete progressGauge;
	progressGauge = NULL;
	numDevices = -1;
	return true;
}


/// Sets the error reporting channel.
void JTAGPort::SetErr(XSError* e)	///< error reporting channel
{
	err = e;
}


/// Provides access to the error reporting channel.
XSError& JTAGPort::GetErr(void)
{
	return *err;
}


/// Returns the text label associated with the given TAP state.
string JTAGPort::GetTAPStateLabel(TAPState s)
{
	switch(s)
	{
	case TestLogicReset:  return string("Test-Logic-Reset");
	case RunTestIdle:     return string("Run-Test/Idle");
	case SelectDRScan:    return string("Select_DR-Scan");
	case SelectIRScan:    return string("Select_IR-Scan");
	case CaptureDR:       return string("Capture-DR");
	case CaptureIR:       return string("Capture-IR");
	case ShiftDR:         return string("Shift-DR");
	case ShiftIR:         return string("Shift-IR");
	case Exit1DR:         return string("Exit1-DR");
	case Exit1IR:         return string("Exit1-IR");
	case PauseDR:         return string("Pause-DR");
	case PauseIR:         return string("Pause-IR");
	case Exit2DR:         return string("Exit2-DR");
	case Exit2IR:         return string("Exit2-IR");
	case UpdateDR:        return string("Update-DR");
	case UpdateIR:        return string("Update-IR");
	case InvalidTAPState:
	default:
		assert(true==false);  // should never get here!!
	}
	return string("Unknown TAP state");
}


/// Returns the TAP state for the given text label.
TAPState JTAGPort::LabelToTAPState(string label)
{
	if(label == "Test-Logic-Reset" || label == "RESET")
		return TestLogicReset;
	if(label == "Run-Test/Idle"    || label == "IDLE")
		return RunTestIdle;
	if(label == "Select_DR-Scan"   || label == "DRSELECT")
		return SelectDRScan;
	if(label == "Select_IR-Scan"   || label == "IRSELECT")
		return SelectIRScan;
	if(label == "Capture-DR"       || label == "DRCAPTURE")
		return CaptureDR;
	if(label == "Capture-IR"       || label == "IRCAPTURE")
		return CaptureIR;
	if(label == "Shift-DR"         || label == "DRSHIFT")
		return ShiftDR;
	if(label == "Shift-IR"         || label == "IRSHIFT")
		return ShiftIR;
	if(label == "Exit1-DR"         || label == "DREXIT1")
		return Exit1DR;
	if(label == "Exit1-IR"         || label == "IREXIT1")
		return Exit1IR;
	if(label == "Pause-DR"         || label == "DRPAUSE")
		return PauseDR;
	if(label == "Pause-IR"         || label == "IRPAUSE")
		return PauseIR;
	if(label == "Exit2-DR"         || label == "DREXIT2")
		return Exit2DR;
	if(label == "Exit2-IR"         || label == "IREXIT2")
		return Exit2IR;
	if(label == "Update-DR"        || label == "DRUPDATE")
		return UpdateDR;
	if(label == "Update-IR"        || label == "IRUPDATE")
		return UpdateIR;
	assert(1==0);
	return (TAPState) -1;
}


/// Initialize the TAP state controller and the attached JTAG device.
void JTAGPort::InitTAP(void)
{
	currentTAPState = TestLogicReset; // this is the state the TAP should start in
	SetTCK(0);		// set clock low; TAP changes state on rising edge
	SetTMS(1);		// set TMS high to reset TAP state machine
	SetTDI(0);
	PulseTCK(5);	// pulse TCK 5 times to reset TAP state machine
	currentTAPState = TestLogicReset; // this is state TAP should be in now
}


/// Enables/disables trace of JTAG signals.
void JTAGPort::SetTraceOnOff(bool f,		///< if true, output trace of JTAG operations
							 ostream& os)	///< send trace messages to this output stream
{
	traceFlag = f;
	osTrace = &os;  // trace messages go to this output stream
}


/// Update the TAP state to what it will be after the given number of clock pulses.
/// No clock pulses are actually created!
void JTAGPort::UpdateTAPState(unsigned int numTCKPulses) ///< number of clock pulses
{
	assert(currentTAPState>=TestLogicReset && currentTAPState<=UpdateIR);
	unsigned int tms = GetTMS();
	for(unsigned int i=numTCKPulses; i>0; i--)
		currentTAPState = nextTAPState[currentTAPState][tms];
}


/// Set the TMS level and pulse TCK to move from the current state to the given adjacent state. 
/// If the new state of the TAP controller doesn't match the requested state, the subroutine will cause the program to abort.
void JTAGPort::GotoNextTAPState(TAPState nextState) ///< TAP state that is adjacent to the current state
{
	// determine the correct TMS value to move to the desired state
	assert(currentTAPState>=TestLogicReset && currentTAPState<=UpdateIR);
	assert( nextTAPState[currentTAPState][0]==nextState || nextTAPState[currentTAPState][1]==nextState);
	SetTMS( nextTAPState[currentTAPState][1]==nextState ? 1:0 );
	PulseTCK();  // pulse TCK to move to the next state
	assert( currentTAPState==nextState );  // ensure new state matches desired state
}


/// Moves through a sequence of TAP controller states.
void JTAGPort::GoThruTAPSequence(TAPState nextState,	///< TAP state that is adjacent to the current state
								...)	///< sequence of adjacent state to move thru until a -1 terminates the sequence
{
	// progress through the states until a -1 is seen
	va_list ap;
	for(va_start(ap,nextState); nextState != -1; nextState=(TAPState)va_arg(ap,int))
		GotoNextTAPState(nextState);
	va_end(ap);
}


/// Move from the current TAP state to a specific, non-adjacent TAP state.
void JTAGPort::GotoTAPState(TAPState finalState)	///< move to this state (which does not have to be adjacent to the current state)
{
	int* path = TAPPath[currentTAPState][finalState];
	for(int i=0; i<17 && path[i]>=0; i++)
		GotoNextTAPState((TAPState)path[i]);
}


/// Check table of TAP state paths to make sure there are no invalid transitions.
///\return true if the table of state paths is valid, false if invalid.
bool JTAGPort::TAPPathsOK(void)
{
	for(int start=0; start<16; start++)
	{
		for(int final=0; final<16; final++)
		{
			int* path = TAPPath[start][final];
			int s = start;
			for(int p=0; p<17 && path[p]>=0; p++)
			{
				if(nextTAPState[s][0]!=path[p] && nextTAPState[s][1]!=path[p])
				{
					XSError err = GetErr();
					err.SetSeverity(XSErrorMinor);
					err << "TAP path error: " << GetTAPStateLabel((TAPState)start);
					err << " --- ("           << GetTAPStateLabel((TAPState)path[p]);
					err << ") --- "           << GetTAPStateLabel((TAPState)final);
					err << "\n";
					err.EndMsg();
					break;
				}
				s = path[p];
			}
		}
	}
	return GetErr().IsError() ? false : true;
}


/// Fetch the bit from TDO, send a bit to TDI, and pulse TCLK.
/// This subroutine assumes that the TCK is 0 and the TAP controller state
/// is Shift-DR or Shift-IR upon entry.  That's why we pick up the value
/// coming in through TDO right away.
///\return the TDO bit
unsigned int JTAGPort::SendRcvBit(unsigned int sendBit) ///< send this bit through TDI
{
	// Make sure we are in the Shift-IR or Shift-DR state.  Gathering
	// and shifting out data isn't legal otherwise.
	assert( currentTAPState==ShiftIR || currentTAPState==ShiftDR );

	// now output the specified value to the TDI input of the external device
	SetTDI(sendBit & 1);
	
	// pulse TCK to move the bit into the BSIR or BSDR
	PulseTCK(1); //clock goes high then low

	return tdoVal; // return the value on TDO pin before the clock pulse
}


/// Load the BSIR with an instruction, execute the instruction, and then capture and reload the BSDR.
/// This subroutine assumes the TAP controller is in the Test-Logic-Reset
/// or Run-Test/Idle state upon entry.  The TAP controller is returned to the
/// Run-Test/Idle state upon exit.
void JTAGPort::LoadBSIRthenBSDR(Bitstream& instruction,	///< instruction to be loaded into BSIR
								Bitstream& send, 	///< data to send into the BSDR
								Bitstream& recv )	///< store data received from the BSDR in here
{
	// Make sure we are in the appropriate state.
	assert( currentTAPState==RunTestIdle || currentTAPState==TestLogicReset ||
		currentTAPState==PauseDR || currentTAPState==PauseIR);
	
	// if the BSIR length is non-zero, then select the BSIR by moving
	// into the Shift-IR state so that the instruction can be shifted in.
	// Then activate the instruction by moving into the Update-IR state.
	if(instruction.GetLength() > 0)
	{
		// move the TAP controller to the Shift-IR state
		GotoTAPState(ShiftIR);
		
		// load the instruction opcode into the BSIR
		Bitstream nullBitstream(0);	//   
		SendRcvBitstream(instruction, nullBitstream);
		
		// now activate the instruction by entering the Update-IR state.
		GotoTAPState(UpdateIR);
	}
	
	// if the BSDR length is non-zero, then select the BSDR by moving
	// into the Capture-DR state so that the new pin data is
	// parallel-loaded into the BSDR.  Then move into the
	// Shift-DR state so that the captured data can be shifted out
	// while a new pattern is shifted in. Finally, output the
	// newly loaded data onto the parallel output of the BSDR by moving
	// into the Update-DR state.
	if(send.GetLength() > 0 || recv.GetLength() > 0)
	{
		// move into the Capture-DR state so that the new pin data is
		// parallel loaded into the BSDR.  Then move into the Shift-DR state.
		GotoTAPState(ShiftDR);
		
		// shift out the captured data while new data is shifted in
		SendRcvBitstream(send, recv);
		
		// output the freshly loaded data onto the pins of the external device
		GotoTAPState(UpdateDR);
	}

	// return to the idle state
	GotoTAPState(RunTestIdle);
}


/// Determine the number of devices in the JTAG chain.
///\return the number of devices found in the chain, -1 if error
int JTAGPort::GetNumberOfDevices(void)
{
	int i;

	// place all the devices in Test-Logic-Reset state
	InitTAP();

	// concatenate bypass instructions
	Bitstream bypassInstr(0), ir(0), dr(maxNumDevices), nullBitstream(0);
	bypassInstr.FromString("11111111");	// bypass instr. is always all 1's
	for(i=maxNumDevices; i>0; i--)
		ir = ir + bypassInstr;

	// load bypass instruction into each device on the chain
	GotoTAPState(ShiftIR);
	SendRcvBitstream(ir,nullBitstream); // all devices should now be bypassed

	// set the single-bit bypass data register of all devices
	for(i=maxNumDevices-1; i>=0; i--)
		dr.SetBit(i,1);
	GotoTAPState(ShiftDR);
	SendRcvBitstream(dr,nullBitstream);

	// go back to the ShiftDR state without capturing external data into DR.
	GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);

	// read the contents of the concatenated bypass data registers
	// as 0's are shifted in.
	//                   devices all loaded                  
	//                        with 1's                       
	//                  +---+   +---+   +---+                
	// shift in 0's --->| 1 |-->| 1 |-->| 1 |---> output goes
	//                  +---+   +---+   +---+     into dr    
	//                  dr[2]   dr[1]   dr[0]                
	SendRcvBitstream(nullBitstream,dr);

	// restore the devices in the chain to the Test-Logic-Reset state
	InitTAP();

	// Look for the 1st zero in dr.  That's the # of devices in the chain.
	for(i=0; i<maxNumDevices; i++)
		if(dr[i] == 0)
		{
			numDevices = i;
			return numDevices;
		}

	numDevices = -1;
	return -1; // no 0 was found in dr so an error occurred.
}


/// Get the ID codes from the devices on the JTAG chain.
///\return the number of devices found in the chain.
int JTAGPort::GetIDCODEs(Bitstream ***idcodes) ///< pointer to array of ID code bitstreams
{
	if(GetNumberOfDevices() <= 0)
	{
		*idcodes = NULL;
		return 0;
	}

	Bitstream **id = new Bitstream* [numDevices];
	Bitstream dr(32);
	dr.FromHexString(32,"FFFFFFFF"); // 32-bits
	InitTAP();
	GotoTAPState(ShiftDR);
	for(int i=numDevices-1; i>=0; i--)
	{
		id[i] = new Bitstream(32);
		SendRcvBitstream(dr,*id[i]);
		GoThruTAPSequence(PauseDR,Exit2DR,ShiftDR,-1);
	}
	*idcodes = id;
	return numDevices;
}


// Read and entire SVF file into memory.
static char* LoadFile(istream& is)
{
	long loadfile_start = clock();

	// determine the size of the file
	is.seekg(0,ios::end);		// move pointer to end of file
	int length = is.tellg();	// pointer position = position of end of file
	is.seekg(0,ios::beg);		// return pointer to beginning of file

	char *buffer = new char[length+1];
	if(buffer==NULL)
	{
		XSError err(cerr);
		err.SimpleMsg(XSErrorFatal,"Unable to allocate enough memory to hold file.");
	}
	for(int i=0; i<=length; i++)
		buffer[i] = 0;
	is.read(buffer, length);

	long loadfile_finish = clock();
	DEBUG_STMT("Time to load SVF file = " << (loadfile_finish-loadfile_start)/(CLOCKS_PER_SEC))

	return buffer;
}


// Fetch the next line from the SVF file stored in memory
static char* GetLineFromBuffer(char** buffer) // buffer holding SVF file
{
	char* beginBuffer = *buffer;
	char* newBuffer = *buffer;
	if(*newBuffer==0)
		return NULL;
	for(char b=*newBuffer; *newBuffer!=0x0a && *newBuffer!=0x0c && *newBuffer!=0; newBuffer++)
		;
	for( ; *newBuffer==0x0c || *newBuffer==0x0a; newBuffer++)
		*newBuffer = 0;
	*buffer = newBuffer;
	return beginBuffer;
}


// Fetch the next SVF command from the SVF file stored in memory
static char* GetSVFCmdFromBuffer(char** buffer)
{
	char* beginBuffer = *buffer;
	char* newBuffer = *buffer;
	if(*newBuffer==0)
		return NULL;
	while(*newBuffer!=0)
	{
		switch(*newBuffer)
		{
		case 0x0a:
		case 0x0c:
			*newBuffer++ = ' ';
			break;
		case '/':
			if(*(newBuffer+1) != '/')
			{
				newBuffer++;
				break;
			}
			*newBuffer++ = ' ';
		case '!':
			for( ; (*newBuffer!=0x0a) && (*newBuffer!=0x0c) && (*newBuffer!=0); newBuffer++)
				*newBuffer = ' ';
			break;
		case ';':
			*newBuffer = 0;
			*buffer = newBuffer+1;
			return beginBuffer;
		default:
			newBuffer++;
			break;
		}
	}
	*buffer = newBuffer;
	return beginBuffer;
}


// Fetch the next word from an SVF command string
static string NextWord(string& s)
{
	int pos = s.find_first_not_of(" \t\n\r");	// find beginning of word
	if(pos==string::npos)
	{	// no non-whitespace chars found, so string has no words
		s = "";
		return "";
	}
	s = s.substr(pos,s.length()-pos);	// remove leading whitespace
	if(*(s.c_str()) == '(')
	{
		pos = s.find_first_of(")");			// find end of parenthesized hex
		pos++;
	}
	else
		pos = s.find_first_of(" \t\n\r");	// find end of first keyword
	string word;
	if(pos==string::npos)
	{	// no more whitespace, so entire string is the last word
		word = s;
		s = "";
	}
	else
	{	// extract the word from the string
		word = s.substr(0,pos);	// get first keyword from command
		s = s.substr(pos,s.length()-pos);	// remove keyword from command
	}
	return word;
}


// Fetch the next word from an SVF command received through an input stream
static string NextWord(istream& is)
{
	char c, waste[257];
	string word = "";
	while(!is.fail())
	{
		is.read(&c,1);
		if(is.fail())
			break;
		switch(c)
		{
		case '/':
			if(is.peek()=='/')
			{
				is.getline(waste,256);
				if(word.length() != 0)
					return word;
			}
			else
				word = word + c;
			break;

		case '!':
			is.getline(waste,256);
			if(word.length() != 0)
				return word;
			break;

		case '\n':
			break;

		case ' ':
		case '\t':
			if(word.length() != 0)
				return word;
			break;

		case ';':
			is.putback(c);
			return word;

		default:
			word = word + c;
			break;
		}
	}
	return word;
}


// Fetch the next SVF command from an input stream
static void NextCmd(istream& is)
{
	char c, waste[257];
	while(!is.fail())
	{
		is.read(&c,1);
		if(is.fail())
			break;
		switch(c)
		{
		case '/':
			if(is.peek()=='/')
				is.getline(waste,256);
			break;

		case '!':
			is.getline(waste,256);
			break;

		case ';':
			return;

		default:
			break;
		}
	}
}


/// Process SVF file and send results to device attached to JTAG port.
///\return true if no errors were encountered, false otherwise.
bool JTAGPort::DownloadSVF(istream& is,		///< receive SVF through this input stream
						string& fileName)	///< name of the file that was opened to create the input stream (used only for progress indicator)
{
	TAPState endir=RunTestIdle, enddr=RunTestIdle;

	long start = clock();

	// determine the size of the SVF file
	is.seekg(0,ios::end);	// move pointer to end of file
	long svfLength = is.tellg();	// pointer position = position of end of file
	is.seekg(0,ios::beg);	// return pointer to beginning of file

	char* svfStart = LoadFile(is); // is.tellg() stops working correctly after file is loaded into internal memory
	if(*svfStart == 0)
	{
		// file is empty, so there is nothing to do
		delete svfStart;
		return false;
	}
	char* svf = svfStart;

	// create a downloading progress gauge
	if(progressGauge == NULL && !IsBufferOn())
	{
		string desc("Download SVF"), subdesc("Downloading "+StripPrefix(fileName));
		progressGauge = new Progress(&GetErr(),desc,subdesc,0,svfLength);
	}

	InitTAP();	// initialize the JTAG state machine
	GoThruTAPSequence(RunTestIdle,-1); // home state for configuration
	
	int lineCnt = 0;
	unsigned long elapsedTime = 0;

	// Progress is misleading if buffering is on, so somebody else has to report it, not us.
	if(!IsBufferOn())
	{
		progressGauge->Report(svf-svfStart); // start progress at 0 which is the beginning of the SVF string
	//	progressGauge->Report(is.tellg()); // start progress at 0 which is the beginning of the SVF string
	}

	unsigned int currentSecond = clock()/CLOCKS_PER_SEC;

	static Bitstream hdr(0), hir(0), tdr(0), tir(0), sdr(0), sir(0);
	while(true)
	{
		// update progress to the user no more than once per second
		if(!IsBufferOn() && (unsigned int)(clock()/CLOCKS_PER_SEC)!=currentSecond)
		{
			currentSecond = clock()/CLOCKS_PER_SEC;
			progressGauge->Report(svf-svfStart);  // SVF string pointer position indicates the current progress
//			progressGauge->Report(is.tellg());  // position in stream indicates the current progress
		}

		// collect one or more lines into a single SVF command
		char *s = GetSVFCmdFromBuffer(&svf);
		if(s == NULL)
			break;

		string svfCmd = s;

#ifdef DEBUG_SVF
		if(svfCmd.length()<1000)
		{
			cout << "SVFCmd: --" << svfCmd << "--\n";
			cout.flush();
		}
#endif

		// remove the first keyword from the command
		string keywd(NextWord(svfCmd));
//		string keywd = NextWord(svfCmd);
//		string keywd = NextWord(is);
//		if(is.fail())
//			break;	// exit if EOF
		if(keywd == "")
			continue;	// must be a blank line
#ifdef DEBUG_SVF
		cout << "Keywd: --" << keywd << "--\n"; cout.flush();
#endif

		int status;
		unsigned long len;

		if(keywd == "TRST"
			|| keywd == "PIO"
			|| keywd == "PIOMAP"
			|| keywd == "FREQUENCY"
			)
		{ // these commands have nothing to do with configuration so ignore them
//			NextCmd(is);
			continue;
		}

		else if(keywd == "ENDIR")
		{
			string nw = NextWord(svfCmd);
//			string nw = NextWord(is);
			assert(nw != "");
			endir = LabelToTAPState(nw);
#ifdef DEBUG_SVF
			cout << GetTAPStateLabel(endir);
			cout.flush();
#endif
//			NextCmd(is);
			continue;
		}

		else if(keywd == "ENDDR")
		{
			string nw = NextWord(svfCmd);
//			string nw = NextWord(is);
			assert(nw != "");
			enddr = LabelToTAPState(nw);
#ifdef DEBUG_SVF
			cout << GetTAPStateLabel(enddr);
			cout.flush();
#endif
//			NextCmd(is);
			continue;
		}

		else if(keywd == "STATE")
		{
			string s;
			for(s=NextWord(svfCmd); s!=""; s=NextWord(svfCmd))
//			for(s=NextWord(is); s!=""; s=NextWord(is))
			{
#ifdef DEBUG_SVF
				cout<< " --> " << s;
				cout.flush();
#endif
				GotoTAPState(LabelToTAPState(s));
			}
#ifdef DEBUG_SVF
			cout<< "\n";
			cout.flush();
#endif
//			NextCmd(is);
			continue;
		}
		
		else if(keywd == "HIR")
		{ // initialize the header bits for loading into the IR
			// get the number of bits in the header
			status = sscanf(NextWord(svfCmd).c_str(),"%ld",&len);
//			status = sscanf(NextWord(is).c_str(),"%ld",&len);
			assert(status!=0);
			hir.Resize(len);
			if(len != 0)
			{
				// otherwise look for the TDI bitfield
				string word;
				for(word=NextWord(svfCmd); word!="TDI" && word!=""; word=NextWord(svfCmd))
//				for(word=NextWord(is); word!="TDI" && word!=""; word=NextWord(is))
					;
				if(word=="TDI")
				{
					// replace the TDI bits only if the SVF command has a TDI bitfield
					word = NextWord(svfCmd); // get the TDI bits
					assert(word[0]=='(');
					assert(word[word.length()-1]==')');
					hir.FromHexString(len,word.c_str()+1);
//					hir.FromHexStream(len,is,false);
				}
			}
#ifdef DEBUG_SVF
			if(hir.GetLength()<1000)
				cout << hir << "\n";
			else
				cout << "[... " << hir.GetLength() << " ...]\n";
			cout.flush();
#endif
//			NextCmd(is);
			continue;
		}
		
		else if(keywd == "HDR")
		{ // initialize the header bits for loading into the DR
			// get the number of bits in the header
			status = sscanf(NextWord(svfCmd).c_str(),"%ld",&len);
//			status = sscanf(NextWord(is).c_str(),"%ld",&len);
			assert(status!=0);
			hdr.Resize(len);
			if(len != 0)
			{
				// otherwise look for the TDI bitfield
				string word;
				for(word=NextWord(svfCmd); word!="TDI" && word!=""; word=NextWord(svfCmd))
//				for(word=NextWord(is); word!="TDI" && word!=""; word=NextWord(is))
					;
				if(word=="TDI")
				{
					// replace the TDI bits only if the SVF command has a TDI bitfield
					word = NextWord(svfCmd); // get the TDI bits
					assert(word[0]=='(');
					assert(word[word.length()-1]==')');
					hdr.FromHexString(len,word.c_str()+1);
//					hdr.FromHexStream(len,is,false);
				}
			}
#ifdef DEBUG_SVF
			if(hdr.GetLength()<1000)
				cout << hdr << "\n";
			else
				cout << "[... " << hdr.GetLength() << " ...]\n";
			cout.flush();
#endif
//			NextCmd(is);
			continue;
		}
		
		else if(keywd == "TIR")
		{ // initialize the trailer bits for loading into the IR
			// get the number of bits in the trailer
			status = sscanf(NextWord(svfCmd).c_str(),"%ld",&len);
//			status = sscanf(NextWord(is).c_str(),"%ld",&len);
			assert(status!=0);
			tir.Resize(len);
			if(len != 0)
			{
				// otherwise look for the TDI bitfield
				string word;
				for(word=NextWord(svfCmd); word!="TDI" && word!=""; word=NextWord(svfCmd))
//				for(word=NextWord(is); word!="TDI" && word!=""; word=NextWord(is))
					;
				if(word=="TDI")
				{
					// replace the TDI bits only if the SVF command has a TDI bitfield
					word = NextWord(svfCmd); // get the TDI bits
					assert(word[0]=='(');
					assert(word[word.length()-1]==')');
					tir.FromHexString(len,word.c_str()+1);
//					tir.FromHexStream(len,is,false);
				}
			}
#ifdef DEBUG_SVF
			if(tir.GetLength()<1000)
				cout << tir << "\n";
			else
				cout << "[... " << tir.GetLength() << " ...]\n";
			cout.flush();
#endif
//			NextCmd(is);
			continue;
		}
		
		else if(keywd == "TDR")
		{ // initialize the trailer bits for loading into the DR
			// get the number of bits in the trailer
			status = sscanf(NextWord(svfCmd).c_str(),"%ld",&len);
//			status = sscanf(NextWord(is).c_str(),"%ld",&len);
			assert(status!=0);
			tdr.Resize(len);
			if(len != 0)
			{
				// otherwise look for the TDI bitfield
				string word;
				for(word=NextWord(svfCmd); word!="TDI" && word!=""; word=NextWord(svfCmd))
//				for(word=NextWord(is); word!="TDI" && word!=""; word=NextWord(is))
					;
				if(word=="TDI")
				{
					// replace the TDI bits only if the SVF command has a TDI bitfield
					word = NextWord(svfCmd); // get the TDI bits
					assert(word[0]=='(');
					assert(word[word.length()-1]==')');
					tdr.FromHexString(len,word.c_str()+1);
//					tdr.FromHexStream(len,is,false);
				}
			}
#ifdef DEBUG_SVF
			if(tdr.GetLength()<1000)
				cout << tdr << "\n";
			else
				cout << "[... " << tdr.GetLength() << " ...]\n";
			cout.flush();
#endif
//			NextCmd(is);
			continue;
		}
		
		else if(keywd == "SIR")
		{ // send a bitstream into the instruction register
			// get the number of bits in the instruction register
			status = sscanf(NextWord(svfCmd).c_str(),"%ld",&len);
//			status = sscanf(NextWord(is).c_str(),"%ld",&len);
			assert(status!=0);
			assert(len>0);
			Bitstream bs(len); // load TDI bits into this bit stream
			string word;
			for(word=NextWord(svfCmd); word!="TDI" && word!=""; word=NextWord(svfCmd))
//			for(word=NextWord(is); word!="TDI" && word!=""; word=NextWord(is))
				;
			if(word=="TDI")
			{
				word = NextWord(svfCmd); // get the TDI bits
				assert(word[0]=='(');
				assert(word[word.length()-1]==')');
				sir.FromHexString(len,word.c_str()+1);
//				sir.FromHexStream(len,is,false);
			}

			// concatenate header, instruction, and trailer bitstreams
			bs = hir + sir + tir;

#ifdef DEBUG_SVF
			if(bs.GetLength()<1000)
				cout << "SIR " << len << " (" << bs.ToHexString() << ")" << endl;
			else
				cout << "SIR " << len << " (... " << bs.GetLength() << " ...)" << endl;
			cout.flush();
#endif
			// send bitstream into IR and return to idle state
			GotoTAPState(ShiftIR);
			Bitstream null(0);	// zero-length bitstream
			SendRcvBitstream(bs,null);
			GotoTAPState(endir);
//			NextCmd(is);
			continue;
		}
		
		else if(keywd == "SDR")
		{ // send a bitstream into the data register
			// get the number of bits in the data register
			status = sscanf(NextWord(svfCmd).c_str(),"%ld",&len);
//			status = sscanf(NextWord(is).c_str(),"%ld",&len);
			assert(status!=0);
			assert(len>0);
			Bitstream bs(0); // load TDI bits into this bit stream
			string word;
			for(word=NextWord(svfCmd); word!="TDI" && word!=""; word=NextWord(svfCmd))
//			for(word=NextWord(is); word!="TDI" && word!=""; word=NextWord(is))
				;
			if(word=="TDI")
			{
				word = NextWord(svfCmd); // get the TDI bits
				assert(word[0]=='(');
				assert(word[word.length()-1]==')');
				sdr.FromHexString(len,word.c_str()+1);
//				sdr.FromHexStream(len,is,false);
			}

			// concatenate header, data, and trailer bitstreams
			bs = hdr + sdr + tdr;

#ifdef DEBUG_SVF
			if(bs.GetLength()<1000)
				cout << "SDR " << len << " (" << bs.ToHexString() << ")" << endl;
			else
				cout << "SDR " << len << " (... " << bs.GetLength() << " ...)" << endl;
			cout.flush();
#endif
			// send bitstream into DR and return to Run-Test-Idle state
			long sdr_start = clock();
			GotoTAPState(ShiftDR);
			Bitstream null(0);	// zero-length bitstream
			SendRcvBitstream(bs,null);
			GotoTAPState(enddr);
			long sdr_finish = clock();
			DEBUG_STMT("SDR download elapsed time = " << (sdr_finish-sdr_start)/(CLOCKS_PER_SEC))
//			NextCmd(is);
			continue;
		}
		
		else if(keywd == "RUNTEST")
		{ // insert delay while programming takes place
			// get the number of microseconds to pause while programming the CPLD 
			status = sscanf(NextWord(svfCmd).c_str(),"%ld",&len);
//			status = sscanf(NextWord(is).c_str(),"%ld",&len);
			assert(status!=0);
			elapsedTime += len;
			assert(NextWord(svfCmd) == "TCK");
//			assert(NextWord(is) == "TCK");
			if(currentTAPState == TestLogicReset)
				SetTMS(1); // TMS value to stay in RESET state
			else
				SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
			PulseTCK(len);
			DEBUG_STMT("delay = " << len << " us")
//			NextCmd(is);
			continue;
		}
		
		// if we get here, then it's some strange junk we don't understand
		XSError err = GetErr();
		err.SetSeverity(XSErrorMajor);
		err << (string)"unknown SVF command: " << keywd << (string)"\n";
		err.EndMsg();
		InitTAP(); // reset TAP and idle the JTAG logic
		if(progressGauge != NULL)
			delete progressGauge;
		progressGauge = NULL;
		delete svfStart;
		return false;
	}
	
	if(!IsBufferOn())
	{
		progressGauge->Report(svf-svfStart); // should set gauge to 100%
	//	progressGauge->Report(is.tellg());	// should set gauge to 100%
	}
	
	long finish = clock();
	DEBUG_STMT("elapsed time = " << (finish-start)/(CLOCKS_PER_SEC))
	DEBUG_STMT("elapsed time = " << elapsedTime << " us")
	
	InitTAP(); // done so reset TAP and idle the JTAG logic

	InsertDelay(CONFIG_SETTLE_TIME);

	if(progressGauge != NULL)
		delete progressGauge;				// remove the progress display once download is over
	progressGauge = NULL;

	delete svfStart;

	return true;
}
	
	
/// Download the contents of an SVF file to a device attached to the JTAG port.
///\return true if no errors were encountered, false otherwise.
bool JTAGPort::DownloadSVF(string& fileName)	///< file containing SVF commands
{
	XSError& err = GetErr();

	if(fileName.length()==0)
		return false;  // stop if no SVF file was given
	
	ifstream is(fileName.c_str(), ios::binary);  // otherwise open SVF file
	if(!is || is.fail() || is.eof())
	{ // error - couldn't open file
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << fileName.c_str() << "\n";
		err.EndMsg();
		return false;
	}

	bool status = DownloadSVF(is, fileName);
	
	is.close();  // close-up the SVF file
	
	return status;
}


static const int DeviceFieldType    = 0x62; // field type for the FPGA device identifier
static const int BitstreamFieldType = 0x65; // field type for the bitstream data

/// Process a configuration bitstream and send it to a device attached to the JTAG port.
///\return true if the operation succeeded, false otherwise.
bool JTAGPort::DownloadBitstream(istream& is)	///< input stream that delivers the config. bitstream
{
	XSError& err = GetErr();
	
	if(is.eof()!=0)
		return false; // exit if no BIT stream exists

	if(progressGauge == NULL)
	{
		is.seekg(0,ios::end);	// move pointer to end of file
		streampos streamEndPos = is.tellg();	// pointer position = position of end of file
		is.seekg(0,ios::beg);	// return pointer to beginning of file
		string desc("Download Bitstream"), subdesc("Downloading Bitstream");
		progressGauge = new Progress(&GetErr(),desc,subdesc,0,streamEndPos,true);
	}

	// show the progress indicator
//	progressGauge->Report(0);
	
	// jump over the first field of the BIT file
	long unsigned int fieldLength = GetInteger(is);
	assert(fieldLength!=0);
	is.ignore(fieldLength);
	
	// process the second field
	fieldLength = GetInteger(is);
	assert(fieldLength==1);
	
	// now look for the field with the chip identifier
	bool status = ScanForField(is,DeviceFieldType);
	assert(status==true);
	fieldLength = GetInteger(is);
	assert(fieldLength>0 && fieldLength<100);
	string chipType;
	unsigned int i;
	for(i=0; i<fieldLength; i++)
	{
		char c;
		is >> c;
		chipType.append(1,c);
	}
	chipType.append(1,0);		// terminate string
	
	// now look for the field with the bitstream data
	status = ScanForField(is,BitstreamFieldType);
	assert(status==true);

	// now we are at the start of the configuration bits
	fieldLength = GetInteger(is,4) * 8; // get the length of the bit stream
	assert(fieldLength>0);
	DEBUG_STMT("bit stream field length = " << fieldLength)

	Bitstream bsir(0);				// BSIR for sending configuration instructions
	Bitstream bsdr(fieldLength);	// BSDR for holding bitstream
	Bitstream null(0);				// zero-length bitstream

	if(chipType.substr(0,5) == "3s50a" || chipType.substr(0,6) == "3s200a" || chipType.substr(0,6) == "3s700a" || chipType.substr(0,7) == "3s1400a")
	{
		InitTAP();	// initialize the JTAG state machine

		// Loading device with a `jprogram` instruction. 
		// SIR 6 TDI (0b) ;
		bsir.FromHexString(6,"0b");
		LoadBSIRthenBSDR(bsir,null,null);
		// Loading device with a `cfg_in` instruction. 
		// SIR 6 TDI (05) ;
		bsir.FromHexString(6,"05");
		LoadBSIRthenBSDR(bsir,null,null);
		// RUNTEST 10000 TCK;
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(10000);
		// SDR 96 TDI (00000000e000850c9955ffff) SMASK (ffffffffffffffffffffffff) ;
		bsdr.FromHexString(96,"00000000e000850c9955ffff");
		LoadBSIRthenBSDR(null,bsdr,null);
		// Loading device with a `jshutdown` instruction. 
		// SIR 6 TDI (0d) ;
		// RUNTEST 12 TCK;
		bsir.FromHexString(6,"0d");
		LoadBSIRthenBSDR(bsir,null,null);
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(12);
		// Loading device with a `cfg_in` instruction. 
		// SIR 6 TDI (05) ;
		// SDR 32 TDI (00000000) SMASK (ffffffff) ;
		bsir.FromHexString(6,"05");
		bsdr.FromHexString(64,"000000001000850c"); // extra 8 bytes "1000850c" were extracted from TDI stream in SVF file
		LoadBSIRthenBSDR(bsir,bsdr,null);
		// SIR 6 TDI (05) TDO (00) MASK (00) ;
		// load the bitstream into the FPGA
		bsir.FromHexString(6,"05"); // CFG_IN instruction
		bsdr.FromCharStream(fieldLength, is, true);
		LoadBSIRthenBSDR( bsir, bsdr, null );

		bsir.FromHexString(6,"0c");	// JSTART instruction
		LoadBSIRthenBSDR( bsir, null, null );
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(16);
		bsir.FromHexString(6,"3f");
		LoadBSIRthenBSDR(bsir,null,null);
		LoadBSIRthenBSDR(bsir,null,null);
		LoadBSIRthenBSDR(bsir,null,null);

		GotoTAPState(TestLogicReset);
	}
	else if(chipType.substr(0,2) == "3s")
	{
#define S3_BYPASS		"111111"
#define S3_JPROGRAM		"001011"
#define S3_CFG_IN		"000101"
#define S3_JSTART		"001100"
#define S3_JSHUTDOWN	"001101"
#define S3_NUM_PULSES	12

		InitTAP();	// initialize the JTAG state machine
#if 0		
		// Loading device with a `jprogram` instruction. 
		//SIR 6 TDI (0b) ;
		//RUNTEST 10000 TCK;
		bsir.FromString(S3_JPROGRAM);
		LoadBSIRthenBSDR(bsir,null,null);
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(14000);
		
		// Loading device with a `jshutdown` instruction. 
		//SIR 6 TDI (0d) ;
		//RUNTEST 12 TCK;
		bsir.FromString(S3_JSHUTDOWN);
		LoadBSIRthenBSDR(bsir,null,null);
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(S3_NUM_PULSES);
		
		// load the bitstream into the FPGA
		bsir.FromString(S3_CFG_IN); // CFG_IN instruction
		bsdr.FromCharStream(fieldLength, is, true);
		LoadBSIRthenBSDR( bsir, bsdr, null );
		
		bsir.FromString(S3_JSTART);	// JSTART instruction
		LoadBSIRthenBSDR( bsir, null, null );
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(S3_NUM_PULSES);
		
		bsir.FromString(S3_JSTART);	// JSTART instruction
		bsdr.FromString("0000000000000000000000");
		LoadBSIRthenBSDR( bsir, bsdr, null );

		GotoTAPState(TestLogicReset);
#else
		// Loading device with a `jprogram` instruction. 
		//SIR 6 TDI (0b) ;
		//RUNTEST 10000 TCK;
		bsir.FromHexString(6,"0b");
		LoadBSIRthenBSDR(bsir,null,null);
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(10000);

		bsir.FromHexString(6,"05");
		bsdr.FromHexString(192,"0000000000000000e00000008001000c66aa9955ffffffff");
		LoadBSIRthenBSDR(bsir,bsdr,null);

		// Loading device with a `jshutdown` instruction. 
		//SIR 6 TDI (0d) ;
		//RUNTEST 12 TCK;
		bsir.FromHexString(6,"0d");
		LoadBSIRthenBSDR(bsir,null,null);
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(12);
		
		// load the bitstream into the FPGA
		GotoTAPState(TestLogicReset);
		bsir.FromHexString(6,"05");
		bsdr.FromHexString(96,"000000000000000066aa9955");
		LoadBSIRthenBSDR(bsir,bsdr,null);
		bsir.FromHexString(6,"05"); // CFG_IN instruction
		bsdr.FromCharStream(fieldLength, is, true);
		LoadBSIRthenBSDR( bsir, bsdr, null );

		GotoTAPState(TestLogicReset);
		GotoTAPState(RunTestIdle);
		GotoTAPState(TestLogicReset);
		GotoTAPState(RunTestIdle);
		bsir.FromHexString(6,"0c");	// JSTART instruction
		LoadBSIRthenBSDR( bsir, null, null );
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(12);

		bsir.FromHexString(6,"3f");
		LoadBSIRthenBSDR(bsir,null,null);
		LoadBSIRthenBSDR(bsir,null,null);

		GotoTAPState(TestLogicReset);
		GotoTAPState(RunTestIdle);
		GotoTAPState(TestLogicReset);
		GotoTAPState(RunTestIdle);
		bsir.FromHexString(6,"0c");	// JSTART instruction
		LoadBSIRthenBSDR( bsir, null, null );
		SetTMS(0); // TMS value to stay in IDLE, DRPAUSE or IRPAUSE state
		PulseTCK(12);

		bsir.FromHexString(6,"3f");
		LoadBSIRthenBSDR(bsir,null,null);
		LoadBSIRthenBSDR(bsir,null,null);

		GotoTAPState(TestLogicReset);
#endif
	}
	else if(chipType.substr(0,2) == "2s")
	{
#define S2_BYPASS		"11111"
#define S2_CFG_IN		"00101"
#define S2_JSTART		"01100"
#define S2_NUM_PULSES	13
		
		InitTAP();	// initialize the JTAG state machine

		// Loading device with a `cfg_in` instruction. 
		bsir.FromString(S2_CFG_IN);
		bsdr.FromHexString(288,"00000000e00000008001000ca00000008001000cbcfd05008004800c66aa995500000000");
		LoadBSIRthenBSDR( bsir, bsdr, null );
		GotoTAPState(TestLogicReset);
		
		bsir.FromString(S2_JSTART);	// JSTART instruction
		bsdr.FromString("0000000000000");
		LoadBSIRthenBSDR( bsir, bsdr, null );
		GotoTAPState(TestLogicReset);

		// Loading device with a `cfg_in` instruction. 
		bsir.FromString(S2_CFG_IN);
		bsdr.FromHexString(352,"00000000e00000008001000ca00000008001000cfffc05008004800c100000008001000c66aa995500000000");
		LoadBSIRthenBSDR( bsir, bsdr, null );
		GotoTAPState(TestLogicReset);

		bsir.FromString(S2_JSTART);	// JSTART instruction
		bsdr.FromString("0000000000000");
		LoadBSIRthenBSDR( bsir, bsdr, null );
		GotoTAPState(TestLogicReset);

		// Loading device with a `cfg_in` instruction. 
		bsir.FromString(S2_CFG_IN);
		bsdr.FromHexString(288,"00000000e00000008001000ca00000008001000cb4fd05008004800c66aa995500000000");
		LoadBSIRthenBSDR( bsir, bsdr, null );
		GotoTAPState(TestLogicReset);
		
		bsir.FromString(S2_JSTART);	// JSTART instruction
		bsdr.FromString("0000000000000");
		LoadBSIRthenBSDR( bsir, bsdr, null );
		GotoTAPState(TestLogicReset);
		
		// load the bitstream into the FPGA
		bsir.FromString(S2_CFG_IN); // CFG_IN instruction
		bsdr.FromCharStream(fieldLength, is, true);
		LoadBSIRthenBSDR( bsir, bsdr, null );
		GotoTAPState(TestLogicReset);
		
		bsir.FromString(S2_JSTART);	// JSTART instruction
		bsdr.FromString("0000000000000");
		LoadBSIRthenBSDR( bsir, bsdr, null );
		GotoTAPState(TestLogicReset);
	}
	else
	{
		err.SetSeverity(XSErrorMajor);
		err << "unknown type of Xilinx chip: " << chipType.c_str() << "\n";
		err.EndMsg();
		return false;
	}

	assert( currentTAPState==TestLogicReset );

	progressGauge->Report(is.tellg());	// this should set gauge to 100%

	InsertDelay(CONFIG_SETTLE_TIME);	// insert a delay to let the chip initialize

	delete progressGauge;				// remove the progress display once download is over
	progressGauge = NULL;
	
	DEBUG_STMT("exiting DownloadBitstream()")
	
	return true;
}


/// Process a configuration bitstream and send it to a device attached to the JTAG port.
///\return true if the operation succeeded, false otherwise.
bool JTAGPort::DownloadBitstream(string& fileName)	///< file containing the config. bitstream
{
	XSError& err = GetErr();
	
	if(fileName.length()==0)
		return false;  // stop if no SVF or BIT file was given
	
	ifstream is(fileName.c_str(),ios::binary);  // otherwise open BIT or SVF file
	if(!is || is.fail() || is.eof())
	{ // error - couldn't open file
		err.SetSeverity(XSErrorMajor);
		err << "could not open " << fileName.c_str() << "\n";
		err.EndMsg();
		return false;
	}
	
	// determine the size of the file
	is.seekg(0,ios::end);	// move pointer to end of file
	streampos streamEndPos = is.tellg();	// pointer position = position of end of file
	is.seekg(0,ios::beg);	// return pointer to beginning of file

	string desc("Configure FPGA"), subdesc("Downloading "+StripPrefix(fileName));
	progressGauge = new Progress(&err,desc,subdesc,0,streamEndPos,true);
	progressGauge->Report(0);
	
	bool status = DownloadBitstream(is);
	
	is.close();  // close-up the hex file
	
	return status;
}


/// Download a configuration bitstream or SVF file to a device attached to the JTAG port
///\return true if the operation succeeded, false otherwise.
bool JTAGPort::DownloadConfigFile(string& fileName)
{
	XSError& err = GetErr();
	
	if(fileName.length()==0)
		return false;  // stop if no SVF or BIT file was given
	
	string suffix = GetSuffix(fileName);
	if(suffix.length()==0)
		return false;  // stop if unable to determine type of configuration file

	if(suffix=="BIT")
		return DownloadBitstream(fileName);
	else if(suffix=="SVF")
		return DownloadSVF(fileName);
	else
		return false;  // unknown type of configuration file
}
