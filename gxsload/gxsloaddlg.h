#ifndef GXSLOADDLG_H
#define GXSLOADDLG_H

#include <QMainWindow>

#include "mylist.h"
#include "../xstoolslib/xserror.h"
#include "../xstoolslib/xsboard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class GxsloadDlg; }
QT_END_NAMESPACE

class GxsloadDlg : public QMainWindow
{
    Q_OBJECT

private slots:
    void ExitButton_clicked();
    void OnSelchangeComboLpt(const QString&);
    void OnSelchangeCmbBoard(const QString&);
    void OnSelchangeFlashformat(const QString&);
    void OnSelchangeRamformat(const QString&);
    void OnLoad();

    void SelUpdatedFPLD(const QString&);
    void SelUpdatedRAM(const QString&);
    void SelUpdatedNON(const QString&);
    void UploadData(const QString&);
    void DownloadSelectedFiles();

public:
    GxsloadDlg(QWidget *parent = nullptr);
    ~GxsloadDlg();

private:
    int SelFPLD;
    int SelRAM;
    int SelNON;
    Ui::GxsloadDlg *ui;

    XSError *errMsg_ptr;

    XSBoard* brdPtr;
    PortType portType;
    int portNum;

private:
    bool eventFilter(QObject *obj, QEvent *event);

    void keyPressEvent(QKeyEvent *event);

    bool ExtractScreenSettings();

    void UpdateLoadButton();

    bool UploadFile(int Source, string UploadPath);
};
#endif // GXSLOADDLG_H

