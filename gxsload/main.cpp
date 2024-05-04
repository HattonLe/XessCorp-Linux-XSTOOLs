#include "gxsloaddlg.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GxsloadDlg w;
    w.show();
    return a.exec();
}
