/********************************************************************************
** Form generated from reading UI file 'gxstestdlg.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GXSTESTDLG_H
#define UI_GXSTESTDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_gxsTestDlg
{
public:
    QWidget *centralwidget;
    QDialogButtonBox *buttonBox;
    QComboBox *m_cmbLpt;
    QComboBox *m_cmbBoard;
    QPushButton *ButtonTest;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *gxsTestDlg)
    {
        if (gxsTestDlg->objectName().isEmpty())
            gxsTestDlg->setObjectName(QString::fromUtf8("gxsTestDlg"));
        gxsTestDlg->resize(432, 163);
        centralwidget = new QWidget(gxsTestDlg);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        buttonBox = new QDialogButtonBox(centralwidget);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(240, 60, 166, 25));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
        m_cmbLpt = new QComboBox(centralwidget);
        m_cmbLpt->setObjectName(QString::fromUtf8("m_cmbLpt"));
        m_cmbLpt->setGeometry(QRect(120, 60, 86, 25));
        m_cmbBoard = new QComboBox(centralwidget);
        m_cmbBoard->setObjectName(QString::fromUtf8("m_cmbBoard"));
        m_cmbBoard->setGeometry(QRect(120, 20, 171, 25));
        ButtonTest = new QPushButton(centralwidget);
        ButtonTest->setObjectName(QString::fromUtf8("ButtonTest"));
        ButtonTest->setGeometry(QRect(320, 20, 89, 25));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 91, 17));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(40, 60, 67, 17));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(50, 100, 311, 17));
        gxsTestDlg->setCentralWidget(centralwidget);
        menubar = new QMenuBar(gxsTestDlg);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 432, 22));
        gxsTestDlg->setMenuBar(menubar);
        statusbar = new QStatusBar(gxsTestDlg);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        gxsTestDlg->setStatusBar(statusbar);

        retranslateUi(gxsTestDlg);

        QMetaObject::connectSlotsByName(gxsTestDlg);
    } // setupUi

    void retranslateUi(QMainWindow *gxsTestDlg)
    {
        gxsTestDlg->setWindowTitle(QCoreApplication::translate("gxsTestDlg", "XS Board Test Utility", nullptr));
        ButtonTest->setText(QCoreApplication::translate("gxsTestDlg", "TEST", nullptr));
        label->setText(QCoreApplication::translate("gxsTestDlg", "Board Type", nullptr));
        label_2->setText(QCoreApplication::translate("gxsTestDlg", "Port", nullptr));
        label_3->setText(QCoreApplication::translate("gxsTestDlg", "Select your XS Board and click on TEST", nullptr));
    } // retranslateUi

};

namespace Ui {
    class gxsTestDlg: public Ui_gxsTestDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GXSTESTDLG_H
