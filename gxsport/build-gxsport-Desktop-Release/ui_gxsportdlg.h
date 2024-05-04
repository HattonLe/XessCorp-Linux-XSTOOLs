/********************************************************************************
** Form generated from reading UI file 'gxsportdlg.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GXSPORTDLG_H
#define UI_GXSPORTDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_gxsportDlg
{
public:
    QWidget *centralwidget;
    QCheckBox *m_chkCount;
    QComboBox *m_cmbLpt;
    QPushButton *pushButtonExit;
    QLabel *label;
    QLabel *label_2;
    QPushButton *m_btnStrobe;
    QPushButton *m_btnData7;
    QPushButton *m_btnData6;
    QPushButton *m_btnData5;
    QPushButton *m_btnData4;
    QPushButton *m_btnData3;
    QPushButton *m_btnData2;
    QPushButton *m_btnData1;
    QPushButton *m_btnData0;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *gxsportDlg)
    {
        if (gxsportDlg->objectName().isEmpty())
            gxsportDlg->setObjectName(QString::fromUtf8("gxsportDlg"));
        gxsportDlg->resize(442, 134);
        centralwidget = new QWidget(gxsportDlg);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        m_chkCount = new QCheckBox(centralwidget);
        m_chkCount->setObjectName(QString::fromUtf8("m_chkCount"));
        m_chkCount->setGeometry(QRect(120, 60, 71, 23));
        m_cmbLpt = new QComboBox(centralwidget);
        m_cmbLpt->setObjectName(QString::fromUtf8("m_cmbLpt"));
        m_cmbLpt->setGeometry(QRect(260, 60, 86, 25));
        pushButtonExit = new QPushButton(centralwidget);
        pushButtonExit->setObjectName(QString::fromUtf8("pushButtonExit"));
        pushButtonExit->setGeometry(QRect(360, 20, 71, 25));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(210, 60, 41, 21));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(40, 30, 21, 17));
        m_btnStrobe = new QPushButton(centralwidget);
        m_btnStrobe->setObjectName(QString::fromUtf8("m_btnStrobe"));
        m_btnStrobe->setGeometry(QRect(30, 60, 71, 25));
        m_btnData7 = new QPushButton(centralwidget);
        m_btnData7->setObjectName(QString::fromUtf8("m_btnData7"));
        m_btnData7->setGeometry(QRect(40, 0, 21, 25));
        m_btnData6 = new QPushButton(centralwidget);
        m_btnData6->setObjectName(QString::fromUtf8("m_btnData6"));
        m_btnData6->setGeometry(QRect(70, 0, 21, 25));
        m_btnData5 = new QPushButton(centralwidget);
        m_btnData5->setObjectName(QString::fromUtf8("m_btnData5"));
        m_btnData5->setGeometry(QRect(100, 0, 21, 25));
        m_btnData4 = new QPushButton(centralwidget);
        m_btnData4->setObjectName(QString::fromUtf8("m_btnData4"));
        m_btnData4->setGeometry(QRect(130, 0, 21, 25));
        m_btnData3 = new QPushButton(centralwidget);
        m_btnData3->setObjectName(QString::fromUtf8("m_btnData3"));
        m_btnData3->setGeometry(QRect(160, 0, 21, 25));
        m_btnData2 = new QPushButton(centralwidget);
        m_btnData2->setObjectName(QString::fromUtf8("m_btnData2"));
        m_btnData2->setGeometry(QRect(190, 0, 21, 25));
        m_btnData1 = new QPushButton(centralwidget);
        m_btnData1->setObjectName(QString::fromUtf8("m_btnData1"));
        m_btnData1->setGeometry(QRect(220, 0, 21, 25));
        m_btnData0 = new QPushButton(centralwidget);
        m_btnData0->setObjectName(QString::fromUtf8("m_btnData0"));
        m_btnData0->setGeometry(QRect(250, 0, 21, 25));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(70, 30, 21, 17));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(100, 30, 21, 17));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(130, 30, 21, 17));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(160, 30, 21, 17));
        label_7 = new QLabel(centralwidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(190, 30, 21, 17));
        label_8 = new QLabel(centralwidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(220, 30, 21, 17));
        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(250, 30, 21, 17));
        gxsportDlg->setCentralWidget(centralwidget);
        menubar = new QMenuBar(gxsportDlg);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 442, 22));
        gxsportDlg->setMenuBar(menubar);
        statusbar = new QStatusBar(gxsportDlg);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        gxsportDlg->setStatusBar(statusbar);

        retranslateUi(gxsportDlg);

        QMetaObject::connectSlotsByName(gxsportDlg);
    } // setupUi

    void retranslateUi(QMainWindow *gxsportDlg)
    {
        gxsportDlg->setWindowTitle(QCoreApplication::translate("gxsportDlg", "gxsport", nullptr));
        m_chkCount->setText(QCoreApplication::translate("gxsportDlg", "Count", nullptr));
        pushButtonExit->setText(QCoreApplication::translate("gxsportDlg", "Exit", nullptr));
        label->setText(QCoreApplication::translate("gxsportDlg", "Port", nullptr));
        label_2->setText(QCoreApplication::translate("gxsportDlg", "D7", nullptr));
        m_btnStrobe->setText(QCoreApplication::translate("gxsportDlg", "Strobe", nullptr));
        m_btnData7->setText(QCoreApplication::translate("gxsportDlg", "0", nullptr));
        m_btnData6->setText(QCoreApplication::translate("gxsportDlg", "0", nullptr));
        m_btnData5->setText(QCoreApplication::translate("gxsportDlg", "0", nullptr));
        m_btnData4->setText(QCoreApplication::translate("gxsportDlg", "0", nullptr));
        m_btnData3->setText(QCoreApplication::translate("gxsportDlg", "0", nullptr));
        m_btnData2->setText(QCoreApplication::translate("gxsportDlg", "0", nullptr));
        m_btnData1->setText(QCoreApplication::translate("gxsportDlg", "0", nullptr));
        m_btnData0->setText(QCoreApplication::translate("gxsportDlg", "0", nullptr));
        label_3->setText(QCoreApplication::translate("gxsportDlg", "D6", nullptr));
        label_4->setText(QCoreApplication::translate("gxsportDlg", "D5", nullptr));
        label_5->setText(QCoreApplication::translate("gxsportDlg", "D4", nullptr));
        label_6->setText(QCoreApplication::translate("gxsportDlg", "D3", nullptr));
        label_7->setText(QCoreApplication::translate("gxsportDlg", "D2", nullptr));
        label_8->setText(QCoreApplication::translate("gxsportDlg", "D1", nullptr));
        label_9->setText(QCoreApplication::translate("gxsportDlg", "D0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class gxsportDlg: public Ui_gxsportDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GXSPORTDLG_H
