#include "gxstestdlg.h"

#include <QApplication>

//#include "../xstoolslib/process.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    gxsTestDlg w;
    w.show();
    return a.exec();
}
