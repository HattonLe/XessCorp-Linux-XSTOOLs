#include "gxsportdlg.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    gxsportDlg w;
    w.show();
    return a.exec();
}
