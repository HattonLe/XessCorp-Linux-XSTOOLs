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

	�1997-2010 - X Engineering Software Systems Corp.
----------------------------------------------------------------------------------*/


#include <cassert>
#include <ctime>
#include <fstream>
#include <string>
using namespace std;

#include "utils.h"
#include "progress.h"
#include "jtaginstr.h"
#include "lptjtag.h"
#include "usbjtag.h"
#include "xsboard.h"

#define	CONFIG_SETTLE_TIME	30,MILLISECONDS
#define	DWNLD_BUFFER_SIZE	0x80000


static XSBoardInfo* brdInfo;
static int numBoards;
