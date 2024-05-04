#ifndef GXSTESTDLG_H
#define GXSTESTDLG_H

#include <QMainWindow>

#include "../xstoolslib/xserror.h"
#include "../xstoolslib/xsboard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class gxsTestDlg; }
QT_END_NAMESPACE

class gxsTestDlg : public QMainWindow
{
    Q_OBJECT

private slots:
    void CancelButton_clicked();
    void OnSelchangeComboLpt(const QString& index);
    void OnSelchangeCmbBoard(const QString& index);
    void OnTest();

public:
    gxsTestDlg(QWidget *parent = nullptr);
    ~gxsTestDlg();

private:
    Ui::gxsTestDlg *ui;

    XSError *errMsg_ptr;

    XSBoard* brdPtr;
    PortType portType;
    int portNum;

private:
    bool ExtractScreenSettings();

};
#endif // GXSTESTDLG_H
