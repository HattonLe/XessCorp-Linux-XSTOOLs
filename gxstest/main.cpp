#include "gxstestdlg.h"

#include <QApplication>

//#include "../xstoolslib/process.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    Progress x;
//    x.move(800, 400);
//    x.show();

    gxsTestDlg w;
    w.show();
    return a.exec();
}
