/********************************************************************************
** Form generated from reading UI file 'gxsloaddlg.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GXSLOADDLG_H
#define UI_GXSLOADDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "dragwidget.h"
#include "mylist.h"

QT_BEGIN_NAMESPACE

class Ui_GxsloadDlg
{
public:
    QWidget *centralwidget;
    QComboBox *m_cmbLpt;
    QComboBox *m_cmbBoard;
    QPushButton *ButtonExit;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QPushButton *m_btnLoad;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *m_ramHiAddr;
    QLineEdit *m_flashHiAddr;
    QLineEdit *m_ramLoAddr;
    QLineEdit *m_flashLoAddr;
    QComboBox *m_ramFormat;
    QComboBox *m_flashFormat;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QCheckBox *m_ramIntfcDownload;
    QCheckBox *m_flashIntfcDownload;
    MyList *m_lstFPLD;
    MyList *m_lstRAM;
    MyList *m_lstNONVOL;
    DragWidget *m_NONUpload;
    DragWidget *m_RAMUpload;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *GxsloadDlg)
    {
        if (GxsloadDlg->objectName().isEmpty())
            GxsloadDlg->setObjectName(QString::fromUtf8("GxsloadDlg"));
        GxsloadDlg->resize(730, 643);
        GxsloadDlg->setAcceptDrops(true);
        centralwidget = new QWidget(GxsloadDlg);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        centralwidget->setAcceptDrops(true);
        m_cmbLpt = new QComboBox(centralwidget);
        m_cmbLpt->setObjectName(QString::fromUtf8("m_cmbLpt"));
        m_cmbLpt->setGeometry(QRect(120, 60, 86, 25));
        m_cmbBoard = new QComboBox(centralwidget);
        m_cmbBoard->setObjectName(QString::fromUtf8("m_cmbBoard"));
        m_cmbBoard->setGeometry(QRect(120, 20, 171, 25));
        ButtonExit = new QPushButton(centralwidget);
        ButtonExit->setObjectName(QString::fromUtf8("ButtonExit"));
        ButtonExit->setGeometry(QRect(580, 80, 89, 25));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 91, 17));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(40, 60, 67, 17));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(60, 130, 101, 17));
        m_btnLoad = new QPushButton(centralwidget);
        m_btnLoad->setObjectName(QString::fromUtf8("m_btnLoad"));
        m_btnLoad->setGeometry(QRect(580, 40, 89, 25));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(320, 130, 41, 17));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(520, 130, 121, 17));
        m_ramHiAddr = new QLineEdit(centralwidget);
        m_ramHiAddr->setObjectName(QString::fromUtf8("m_ramHiAddr"));
        m_ramHiAddr->setGeometry(QRect(240, 370, 181, 25));
        m_flashHiAddr = new QLineEdit(centralwidget);
        m_flashHiAddr->setObjectName(QString::fromUtf8("m_flashHiAddr"));
        m_flashHiAddr->setGeometry(QRect(460, 370, 191, 25));
        m_ramLoAddr = new QLineEdit(centralwidget);
        m_ramLoAddr->setObjectName(QString::fromUtf8("m_ramLoAddr"));
        m_ramLoAddr->setGeometry(QRect(240, 410, 181, 25));
        m_flashLoAddr = new QLineEdit(centralwidget);
        m_flashLoAddr->setObjectName(QString::fromUtf8("m_flashLoAddr"));
        m_flashLoAddr->setGeometry(QRect(460, 410, 191, 25));
        m_ramFormat = new QComboBox(centralwidget);
        m_ramFormat->setObjectName(QString::fromUtf8("m_ramFormat"));
        m_ramFormat->setGeometry(QRect(240, 450, 141, 25));
        m_flashFormat = new QComboBox(centralwidget);
        m_flashFormat->setObjectName(QString::fromUtf8("m_flashFormat"));
        m_flashFormat->setGeometry(QRect(460, 450, 131, 25));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(110, 450, 121, 21));
        label_7 = new QLabel(centralwidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(110, 410, 101, 21));
        label_8 = new QLabel(centralwidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(110, 370, 101, 21));
        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(120, 500, 91, 71));
        label_9->setWordWrap(true);
        m_ramIntfcDownload = new QCheckBox(centralwidget);
        m_ramIntfcDownload->setObjectName(QString::fromUtf8("m_ramIntfcDownload"));
        m_ramIntfcDownload->setGeometry(QRect(260, 520, 92, 23));
        m_flashIntfcDownload = new QCheckBox(centralwidget);
        m_flashIntfcDownload->setObjectName(QString::fromUtf8("m_flashIntfcDownload"));
        m_flashIntfcDownload->setGeometry(QRect(480, 520, 92, 23));
        m_lstFPLD = new MyList(centralwidget);
        m_lstFPLD->setObjectName(QString::fromUtf8("m_lstFPLD"));
        m_lstFPLD->setGeometry(QRect(10, 150, 256, 192));
        m_lstFPLD->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_lstRAM = new MyList(centralwidget);
        m_lstRAM->setObjectName(QString::fromUtf8("m_lstRAM"));
        m_lstRAM->setGeometry(QRect(270, 150, 211, 192));
        m_lstRAM->setSelectionMode(QAbstractItemView::MultiSelection);
        m_lstNONVOL = new MyList(centralwidget);
        m_lstNONVOL->setObjectName(QString::fromUtf8("m_lstNONVOL"));
        m_lstNONVOL->setGeometry(QRect(515, 150, 211, 192));
        m_lstNONVOL->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_NONUpload = new DragWidget(centralwidget);
        m_NONUpload->setObjectName(QString::fromUtf8("m_NONUpload"));
        m_NONUpload->setGeometry(QRect(600, 450, 31, 31));
        m_RAMUpload = new DragWidget(centralwidget);
        m_RAMUpload->setObjectName(QString::fromUtf8("m_RAMUpload"));
        m_RAMUpload->setGeometry(QRect(390, 450, 31, 31));
        GxsloadDlg->setCentralWidget(centralwidget);
        menubar = new QMenuBar(GxsloadDlg);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 730, 22));
        GxsloadDlg->setMenuBar(menubar);
        statusbar = new QStatusBar(GxsloadDlg);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        GxsloadDlg->setStatusBar(statusbar);

        retranslateUi(GxsloadDlg);

        QMetaObject::connectSlotsByName(GxsloadDlg);
    } // setupUi

    void retranslateUi(QMainWindow *GxsloadDlg)
    {
        GxsloadDlg->setWindowTitle(QCoreApplication::translate("GxsloadDlg", "gxsload", nullptr));
        ButtonExit->setText(QCoreApplication::translate("GxsloadDlg", "Exit", nullptr));
        label->setText(QCoreApplication::translate("GxsloadDlg", "Board Type", nullptr));
        label_2->setText(QCoreApplication::translate("GxsloadDlg", "Port", nullptr));
        label_3->setText(QCoreApplication::translate("GxsloadDlg", "FPGA / CPLD", nullptr));
        m_btnLoad->setText(QCoreApplication::translate("GxsloadDlg", "Load", nullptr));
        label_4->setText(QCoreApplication::translate("GxsloadDlg", "RAM", nullptr));
        label_5->setText(QCoreApplication::translate("GxsloadDlg", "Flash / EEPROM", nullptr));
        label_6->setText(QCoreApplication::translate("GxsloadDlg", "Upload Format", nullptr));
        label_7->setText(QCoreApplication::translate("GxsloadDlg", "Low Address", nullptr));
        label_8->setText(QCoreApplication::translate("GxsloadDlg", "High Address", nullptr));
        label_9->setText(QCoreApplication::translate("GxsloadDlg", "Download Flash/Ram Interface", nullptr));
        m_ramIntfcDownload->setText(QString());
        m_flashIntfcDownload->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class GxsloadDlg: public Ui_GxsloadDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GXSLOADDLG_H
