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

#include "progress.h"
#include "utils.h"


#ifdef _WINDOWS

IMPLEMENT_DYNCREATE( ProgressThread, CWinThread )

BEGIN_MESSAGE_MAP( ProgressThread, CWinThread )
ON_THREAD_MESSAGE( WM_PROGRESS_REPORT, Report )
END_MESSAGE_MAP()

#endif


/*----------------------------------------------------------------------------------
    The ProgressThread displays the progress made on a long-duration operation.
    It runs in a thread so the progress will show activity even if the main thread
    is stuck performing some long-duration operation.
   ----------------------------------------------------------------------------------*/

/// Create a progress thread object.
ProgressThread::ProgressThread( void )
{
    ;
}



/// Create a progress thread object.
ProgressThread::ProgressThread( Progress *progress ) ///< pointer to Progress friend-object that has all the information
{
    Setup( progress );
}



/// Destroy a progress thread object.
ProgressThread::~ProgressThread( void )
{
    ;
}



/// Initialize the object.
///\return true if the setup was accomplished, false if not
bool ProgressThread::Setup( Progress *progress ) ///< pointer to Progress friend-object that has all the information
{
#ifdef _WINDOWS
    m_bAutoDelete  = true;  // Deallocate thread storage when this thread is terminated.
#endif
    this->progress = progress;
    return true;    // setup successful
}



#ifdef _WINDOWS
BOOL ProgressThread::InitInstance( void )
{
    CRect mainRect, progRect( 0, 0, 250, 80 );
    AfxGetMainWnd()->GetWindowRect( &mainRect );
    progRect.OffsetRect( mainRect.TopLeft() );
    progRect.OffsetRect( 0, mainRect.Height() + 20 );
    CString strWndClass = AfxRegisterWndClass( NULL, 0, (HBRUSH)( COLOR_3DFACE + 1 ), 0 );
    prgCWnd.CreateEx( 0, strWndClass, progress->maintaskDescription.c_str(),
                      WS_POPUP | WS_CAPTION | WS_VISIBLE, progRect, NULL, 0, NULL );

    description.Create( progress->subtaskDescription.c_str(),
                        WS_CHILD | WS_VISIBLE | SS_LEFT, CRect( 10, 10, 235, 25 ), &prgCWnd, 0 );
    font.CreatePointFont( 80, "MS Sans Serif" );
    description.SetFont( &font );

    ctlProgress.Create( WS_CHILD | WS_VISIBLE | WS_BORDER, CRect( 10, 30, 235, 45 ), &prgCWnd, 0 );
    ctlProgress.SetPos( 0 );
    prgCWnd.UpdateWindow();

    return TRUE;
}



#endif


#ifdef _WINDOWS
LRESULT ProgressThread::Report( WPARAM w, LPARAM l )
{
    if ( progress->freeWheel )
        progress->percentDone = ( (int)progress->percentDone + 10 ) % 100; // else
     // percentDone = (int)(((LPARAMStruct*)l)->percentDone * 100);

    description.SetFont( &font );
    ctlProgress.SetPos( progress->percentDone );
    prgCWnd.Invalidate();
    prgCWnd.RedrawWindow();

    if ( progress->freeWheel )
    {
        PostThreadMessage( WM_PROGRESS_REPORT, true, 0 );
        Sleep( 500 );
    }
    return 0;
}



#endif


#ifdef _WINDOWS
BOOL ProgressThread::ExitInstance( void )
{
    prgCWnd.DestroyWindow();    // Get rid of progress window.
    CWinThread::ExitInstance(); // Call base class method to deallocate thread storage.
    return TRUE;
}



#endif



#ifndef _WINDOWS

DWORD WINAPI ProgressReport( LPVOID l )
{
    Progress *progress = (Progress *)l;
    int prcntDone      = 0;
    cerr << "\n" << progress->maintaskDescription.c_str() << ":\n";
    cerr << progress->subtaskDescription.c_str() << ":";
    while ( progress->running )
    {
        if ( progress->freeWheel )
            progress->percentDone = ( (int)progress->percentDone + 10 ) % 100; // else
        char msg1[20], msg2[20];
        sprintf( msg1, "%d%%", (int)progress->percentDone );
        sprintf( msg2, "%4.4s\010\010\010\010", msg1 );
        cerr << msg2;
        Sleep( 200 );
    }
    cerr << "100%\n";
    flush( cerr );
    return 0;
}



#endif



/// Create a progress indicator object.
Progress::Progress( void )
{
    visible = false;    // progress indicator is not yet visible
}



/// Create a progress indicator object.
Progress::Progress( XSError *e, ///< error reporting channel
                    string &maintaskDesc, ///< description of main task on which progress is being reported
                    string &subtaskDesc, ///< description of subtask within main task on which progress is currently being displayed
                    float lo, ///< lower boundary of progress
                    float hi, ///< upper boundary of progress
                    bool freeWheel ) ///< when true, progress indicator just recirculates to show progress is being made
{
    visible = false;    // progress indicator is not yet visible
    Setup( e, maintaskDesc, subtaskDesc, lo, hi, freeWheel );
}



/// Destroy the progress indicator object.
Progress::~Progress( void )
{
#ifdef _WINDOWS
    if ( prgThread != NULL )
        prgThread->PostThreadMessage( WM_QUIT, 0, 0 );
#else
    if ( prgThreadHandle[0] != NULL )
    {
        running = false;
        WaitForSingleObject( prgThreadHandle[0], INFINITE );
    }
#endif
}



/// Initialize the object.
///\return true if the setup was accomplished, false if not
bool Progress::Setup( XSError *e, ///< error reporting channel
                      string &maintaskDesc, ///< description of main task on which progress is being reported
                      string &subtaskDesc, ///< description of subtask within main task on which progress is currently being displayed
                      float lo, ///< lower boundary of progress
                      float hi, ///< upper boundary of progress
                      bool freeWheel ) ///< when true, progress indicator just recirculates to show progress is being made
{
    // Only allow setup when the progress indicator is not yet visible.
    if ( !visible )
    {
#ifdef _WINDOWS
        prgThread           = NULL;
#endif
        errorChannel        = e;
        maintaskDescription = maintaskDesc;
        subtaskDescription  = subtaskDesc;
        loBound             = lo;
        hiBound             = hi;
        percentDone         = 0;
        this->freeWheel     = freeWheel;
        return true;    // setup successful
    }
    else
    {
        return false;   // setup failed because progress indicator is already visible
    }
}



/// Display the current progress.
void Progress::Report( float x )  ///< a value between the high and low levels of the progress indicator.
{
    float percent = 100 * ( x - loBound ) / ( hiBound - loBound );
    if ( !visible )
    {
#ifdef _WINDOWS
                                ( ProgressThread * ) prgThread = (ProgressThread *)AfxBeginThread( RUNTIME_CLASS( ProgressThread ), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL );
        prgThread->Setup( this );
        prgThread->ResumeThread();
        prgThread->PostThreadMessage( WM_PROGRESS_REPORT, NULL, NULL );
#else
        prgThreadHandle[0]                                     = CreateThread( NULL, 0, ProgressReport, ( LPVOID ) this, 0, NULL );
#endif
        running                                                = true;
        visible                                                = true;
    }
    else if ( !freeWheel && percent > percentDone + 3 )
    {
#ifdef _WINDOWS
        prgThread->PostThreadMessage( WM_PROGRESS_REPORT, NULL, NULL );
#endif
        percentDone = percent;
    }
}
