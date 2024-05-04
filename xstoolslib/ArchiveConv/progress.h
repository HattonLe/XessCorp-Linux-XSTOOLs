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


#ifndef PROGRESS_H
#define PROGRESS_H

#ifdef _WINDOWS
#include <afxwin.h>
#include <afxcmn.h>
#else
#include <windows.h>
#endif

#include "xserror.h"


/**
Report the progress of an operation.

This object provides feedback to the user concerning the percentage of a 
task that has been completed. A progress bar is displayed in a GUI 
environment while a rolling percentage indicator is used in a 
command-line environment. 

*/

#define	WM_PROGRESS_REPORT		WM_USER+1

class Progress;

#ifdef _WINDOWS
class ProgressThread : public CWinThread
#else
class ProgressThread
#endif
{
	public:

	ProgressThread(void);

	ProgressThread(Progress *progress);

	~ProgressThread(void);

	bool Setup(Progress *progress);

#ifdef _WINDOWS
	BOOL InitInstance(void);

	LRESULT Report(WPARAM w, LPARAM l);

	BOOL ExitInstance(void);

    DECLARE_DYNCREATE(ProgressThread)
#else
	DWORD WINAPI Report(LPVOID l);
#endif


	private:

	class Progress *progress;	///< pointer to friend-class that has all the information

#ifdef _WINDOWS
	CWnd prgCWnd;
	CProgressCtrl ctlProgress;
	CStatic description;
	CFont font;
	protected:
	DECLARE_MESSAGE_MAP()
#endif
};



class Progress
{
	friend class ProgressThread;	///< friend needs to look at member values in this class
	friend DWORD WINAPI ProgressReport( LPVOID l );


	public:

	Progress();

	Progress(XSError* e, string& maintaskDesc, string& subtaskDesc, float lo, float hi, bool freeWheel=false);

	~Progress(void);

	bool Progress::Setup(XSError* e, string& maintaskDesc, string& subtaskDesc, float lo, float hi, bool freeWheel=false);

	void Report(float x);

	
	private:

	XSError* errorChannel;		///< error reporting channel
	string maintaskDescription;	///< description of main task on which progress is being reported
	string subtaskDescription;	///< description of subtask within main task on which progress is currently being displayed
	float loBound;				///< lower boundary of progress
	float hiBound;				///< upper boundary of progress
	float percentDone;			///< current percentage of the task that is done (range 0..100)
	bool freeWheel;				///< when true, progress indicator just recirculates to show progress is being made
	bool visible;				///< when true, progress indicator is visible
	bool running;				///< when true, progress is being displayed and updated
#ifdef _WINDOWS
	ProgressThread *prgThread;	///< progress indicator is updated inside this thread
#else
	HANDLE prgThreadHandle[1];	///< HANDLE for progress thread
#endif
};

#endif
