#include "progress.h"
#include "utils.h"

#include <unistd.h>

/*----------------------------------------------------------------------------------
    The ProgressThread displays the progress made on a long-duration operation.
    It runs in a thread so the progress will show activity even if the main thread
    is stuck performing some long-duration operation.
   ----------------------------------------------------------------------------------*/

Worker::Worker()
    : QObject()
{
    //qDebug() << "ProgressWorker() this:" << this;
}

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif
void Worker::qSleep(int ms)
{
    assert(ms > 0);

#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

void Worker::DoWork()
{
    //qDebug() << "DoWork():";

    ForceAbort = false;

    //qDebug() << "QTimer()...";
    t = new QTimer(this);
    if (NULL != t)
    {
        connect(t, &QTimer::timeout, this, &Worker::UpdatePercentage);
        t->start(500);
    }

    //qDebug() << "Before loop...";
    Tracker->percentDone = 0;
    while (!ForceAbort)
    {
        qSleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    //qDebug() << "After loop...";

    // If abort requested externally
    if (ForceAbort)
    {
        //qDebug() << "Abort Actioned...";
    }

    Shutdown();

    //qDebug() << "Thread Finished";
    emit WorkFinished();
 //   QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

///\return true if the setup was accomplished, false if not
///< pointer to Progress friend-object that has all the information
bool Worker::Setup(Progress *progress)
{
    Tracker = progress;

    Tracker->percentDone = 0;
    return true;
}

void Worker::UpdatePercentage()
{
//    qDebug() << "Tick...";
    if (NULL != Tracker)
    {
        if (!ForceAbort)
        {
            if (Tracker->freeWheel)
            {
//                Tracker->percentDone = ( (int) Tracker->percentDone + 10 ) % 100;
            }
            else
            {
                //... perform one percent of the operation
//                Tracker->percentDone++;
            }
//            qDebug() << "Tok(" << Tracker->percentDone << "%)";
            emit PercentageComplete(Tracker->percentDone);
        }
    }
}

//... cleanup
void Worker::Shutdown(void)
{
    //qDebug() << "Shutdown...";
    if (NULL != t)
    {
        t->stop();

        delete t;
        t = NULL;
    }
}

void Worker::AbortWork()
{
    //qDebug() << "Aborting...";

    ForceAbort = true;
}

/// Destroy a progress thread object.
Worker::~Worker(void)
{
    //qDebug() << "destructor";
    AbortWork();
}

/// Create a progress indicator object.
Progress::Progress(QObject *parent, XSError *e) :
    QObject()
{
    ParentWindow = parent;
    errorChannel = e;

    // progress indicator is not yet visible
    pd = NULL;
}

/// Create a progress indicator object.
Progress::Progress(QObject *parent,
                    XSError *e, ///< error reporting channel
                    string &maintaskDesc, ///< description of main task on which progress is being reported
                    string &subtaskDesc, ///< description of subtask within main task on which progress is currently being displayed
                    float lo, ///< lower boundary of progress
                    float hi, ///< upper boundary of progress
                    bool freeWheel) :  ///< when true, progress indicator just recirculates to show progress is being made
    Progress(parent, e)
{
    Setup(maintaskDesc, subtaskDesc, lo, hi, freeWheel);
}

/// Destroy the progress indicator object.
Progress::~Progress(void)
{
    CloseWorker();
}

void Progress::CloseWorker()
{
    //qDebug() << "Progress Closing";

    if (NULL != worker)
    {
        //Tell the thread to abort
        //qDebug() << "Flagging Abort...";
        emit AbortWorkRequest();

        // We use this as a indicator we have told worker to go.
        worker = NULL;
        //qDebug() << "Progress Close done";
    }
}


// https://stackoverflow.com/questions/34553960/minimal-example-to-use-qt-threads
// https://doc.qt.io/qt-6/qprogressdialog.html

/// Initialize the object.
///\return true if the setup was accomplished, false if not
bool Progress::Setup(string &maintaskDesc, ///< description of main task on which progress is being reported
                     string &subtaskDesc, ///< description of subtask within main task on which progress is currently being displayed
                     float lo, ///< lower boundary of progress
                     float hi, ///< upper boundary of progress
                     bool freeWheel ) ///< when true, progress indicator just recirculates to show progress is being made
{
    // Only allow setup when the progress indicator is not yet visible.
    // We use pd as a flag to determine if the progress dialog is already onscreen or not.
    if (NULL == pd)
    {
        maintaskDescription = maintaskDesc;
        subtaskDescription  = subtaskDesc;
        loBound             = lo;
        hiBound             = hi;
        percentDone         = 0;
        this->freeWheel     = false; //freeWheel;

        worker = new Worker();
        if (NULL != worker)
        {
            IndicatorThread = new QThread();
            if (NULL != IndicatorThread)
            {
                string Desc;

                worker->Setup(this);

                Desc = maintaskDescription + "\n" + subtaskDescription;

                //qDebug() << "QProgressDialog()...";
                pd = new QProgressDialog(Desc.c_str(), "Cancel", 0, 100);
                if (NULL != pd)
                {
                    //pd->setAttribute(Qt::WA_DeleteOnClose, true);

                    // Prevent overrunning the progress bar limit causing the dialog to reset & show itself again.
                    pd->setAutoReset(false);

                    worker->moveToThread(IndicatorThread);

                    // Either Cancel button clicked, or the close dialog window X clicked.
                    connect(pd, &QProgressDialog::canceled, this, &Progress::AllDone);
                    connect(pd, &QProgressDialog::destroyed, this, &Progress::NoPD);

                    //connect(worker, SIGNAL(PercentageComplete(int)), pd, SLOT(setValue(int)));
                    connect(worker, SIGNAL(PercentageComplete(int)), this, SLOT(WorkTick(int)));
                    connect(worker, SIGNAL(WorkFinished()), this, SLOT(WorkTerminated()));

                    connect(this, SIGNAL(AbortWorkRequest()), worker, SLOT(AbortWork()));

                    connect(IndicatorThread, SIGNAL(started()), worker, SLOT(DoWork()));
                    connect(IndicatorThread, SIGNAL(finished()), worker, SLOT(deleteLater()));

                    connect(IndicatorThread, SIGNAL(finished()), IndicatorThread, SLOT(deleteLater()));

                    pd->show();
                }
                IndicatorThread->start();
            }
        }

        return true;    // setup successful
    }
    else
    {
        return false;   // setup failed because progress indicator is already visible
    }
}

/// returns true if the user has Cancelled the long activity; i.e. has ended the progress dialog.
bool Progress::UserHasCancelled()
{
    return (NULL == pd);
}

/// Display the current progress and returns true if the user has Cancelled the long activity.
///< a value between the high and low levels of the progress indicator.
void Progress::Report(float x)
{
    float percent = 100 * ( x - loBound ) / ( hiBound - loBound );

    if (!freeWheel && percent > percentDone + 3)
    {
        percentDone = percent;
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

// Use this at end of the long activity to remove the progress dialog from display.
Progress *Progress::EndProgress()
{
    //qDebug() << "Ending Progress";

    // Force a 100% display
    percentDone = 100;
    emit AbortWorkRequest();

    if (NULL != pd)
    {
        pd->close();
    }

    return NULL;
}

void Progress::WorkTick(int val)
{
    //qDebug() << "WorkTick(" << val << ")";
    // During closure a tick may be received, so ignore it
    if ((NULL != pd) && (NULL != worker))
    {
        pd->setValue(val);
    }
}

void Progress::WorkTerminated()
{
    //qDebug() << "WorkTerminated()...";
}

void Progress::AllDone()
{
    //qDebug() << "AllDone()...";

    CloseWorker();
    if (NULL != pd)
    {
        pd->deleteLater();

        // Must allow Setup() to succeed immediately for the next progress indicator be initialised
        pd = NULL;
    }
}

// You get this callback sometime after AllDone() if you specify pd->setAttribute(Qt::WA_DeleteOnClose, true);
// or... if you had called pd->deleteLater()
void Progress::NoPD()
{
    //qDebug() << "NoPD()...";

    // Can't do this here because this is called at a later time when pd var might be in use again.
    //pd = NULL;
}
