#ifndef GXSPORTDLG_H
#define GXSPORTDLG_H

#include <QtWidgets>
#include <QMainWindow>

#include "../xstoolslib/xserror.h"
#include "../xstoolslib/xsboard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class gxsportDlg; }
QT_END_NAMESPACE

class gxsportDlg : public QMainWindow
{
    Q_OBJECT

private slots:
    void ExitButton_clicked();
    void StrobeButton_clicked();
    void OnBtnData0();
    void OnBtnData1();
    void OnBtnData2();
    void OnBtnData3();
    void OnBtnData4();
    void OnBtnData5();
    void OnBtnData6();
    void OnBtnData7();
    void OnBtnStrobe();
    void OnChkCount();
    void OnSelchangeComboLpt(const QString& index);

public:
    gxsportDlg(QWidget *parent = nullptr);
    ~gxsportDlg();

private:
    Ui::gxsportDlg *ui;

    XSError *errMsg_ptr;

    XSBoard* brdPtr;
    PortType portType;
    int portNum;

//	PPort* parPort;
    unsigned int newPortDataVal;

    QPushButton *dataBtn[8];

private:
    // initialize the button text to reflect the data value
    void DisplayBtnData(unsigned int data);
    void HandleStrobe();
    void OnBtnData(int bitPos);

    bool ExtractScreenSettings();

};
#endif // GXSPORTDLG_H
