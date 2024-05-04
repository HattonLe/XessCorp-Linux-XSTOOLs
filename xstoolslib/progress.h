#ifndef PROGRESS_H
#define PROGRESS_H

#include <QtWidgets>
#include <pthread.h>

#include "../TempFixes/FixLH.h"
#include "../TempFixes/wtypes.h"

#include "xserror.h"

/**
Report the progress of an operation.

This object provides feedback to the user concerning the percentage of a 
task that has been completed. A progress bar is displayed in a GUI 
environment while a rolling percentage indicator is used in a 
command-line environment. 

*/

class Progress;

class Worker : public QObject
{
    Q_OBJECT

public slots:
    void DoWork();
    void UpdatePercentage();
    void AbortWork();

signals:
    void PercentageComplete(int);
    void WorkFinished();

public:
    Worker();
    ~Worker(void);

    bool Setup(Progress *progress);

private:
    void qSleep(int ms);
    void Shutdown();

private:
    class Progress *Tracker;	///< pointer to friend-class that has all the information
    QTimer *t;
    bool ForceAbort;

	protected:
};

class Progress : public QObject
{
    Q_OBJECT

signals:
    void AbortWorkRequest();
    void PercentageComplete(int val);

public slots:
    void WorkTerminated();
    void AllDone();
    void NoPD();
    void WorkTick(int val);

    friend class Worker;	///< friend needs to look at member values in this class
    friend void *ProgressReport(void *data);

public:
    Progress(QObject *parent, XSError *e);
    Progress(QObject *parent, XSError* e, string& maintaskDesc, string& subtaskDesc, float lo, float hi, bool freeWheel=false);
    ~Progress(void);
    bool Setup(string& maintaskDesc, string& subtaskDesc, float lo, float hi, bool freeWheel=false);
    void Report(float x);
    bool UserHasCancelled();
    Progress *EndProgress();

private:
    void CloseWorker();

private:
    QObject *ParentWindow;
	XSError* errorChannel;		///< error reporting channel

	string maintaskDescription;	///< description of main task on which progress is being reported
	string subtaskDescription;	///< description of subtask within main task on which progress is currently being displayed
	float loBound;				///< lower boundary of progress
	float hiBound;				///< upper boundary of progress
    volatile float percentDone;			///< current percentage of the task that is done (range 0..100)
	bool freeWheel;				///< when true, progress indicator just recirculates to show progress is being made

    QThread *IndicatorThread;
    QProgressDialog *pd;
    Worker *worker;
};

#endif
