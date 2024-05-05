#include <QtWidgets>

#include <cassert>
#include <cstdlib>
#include <strstream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

#include "gxstestdlg.h"
#include "ui_gxstestdlg.h"

#include "../xstoolslib/utils.h"
#include "../xstoolslib/pport.h"
#include "../xstoolslib/guitools.h"
#include "../xstoolslib/utils.h"
#include "../xstoolslib/parameters.h"

#include "../xstoolslib/guitools.h"
#include "../xstoolslib/transports.h"

Transports *Transports::Singleton = new Transports();

gxsTestDlg::gxsTestDlg(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::gxsTestDlg)
{
    ui->setupUi(this);

    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(CancelButton_clicked()));
    connect(ui->ButtonTest, SIGNAL(clicked()), SLOT(OnTest()));
    connect(ui->m_cmbLpt, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnSelchangeComboLpt(const QString&)));
    connect(ui->m_cmbBoard, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnSelchangeCmbBoard(const QString&)));

    errMsg_ptr = new XSError(cerr);

    if (getuid())
    {
        string Msg;

        Msg = "You need to run this as root for access to Parallel Ports";
        QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
    }
    else
    {
        int ArgC;
        string User;

        // Get the user name from the only and only command argument
        ArgC = QApplication::arguments().count();
        if (2 == ArgC)
        {
            User = QApplication::arguments().value(1).toStdString();
        }

        // If we can access the XSTOOLs parameters file
        if (Parameters::FindParameterFile(User.c_str()))
        {
            string brdType;
            string portName;

            // get the last settings
            portName = Parameters::GetXSTOOLSParameter("PORT");
            if ("" == portName)
            {
                 // look for old-style LPT parameter if PORT parameter is empty
                portName = Parameters::GetXSTOOLSParameter("LPT");
            }

            brdType = Parameters::GetXSTOOLSParameter("BoardType");

            // stored in the XSTOOLS parameter file
            GuiTools::SetPortList(ui->m_cmbLpt);
            GuiTools::SetBoardList(ui->m_cmbBoard);

            Parameters::InitialisationDone();

            if ("" == portName)
            {
                Parameters::SetXSTOOLSParameter("PORT", "LPT1");
                portName = Parameters::GetXSTOOLSParameter("PORT");
            }

            if ("" == brdType)
            {
                // set the default value if no previous value exists
                Parameters::SetXSTOOLSParameter("BoardType", "XSA-3S1000");
                brdType = Parameters::GetXSTOOLSParameter("BoardType");
            }

            ui->m_cmbLpt->setCurrentIndex(ui->m_cmbLpt->findText(portName.c_str()));
            ui->m_cmbBoard->setCurrentIndex(ui->m_cmbBoard->findText(brdType.c_str()));

            ExtractScreenSettings();
        }
    }
}

gxsTestDlg::~gxsTestDlg()
{
    delete ui;
}

bool gxsTestDlg::ExtractScreenSettings()
{
    bool Valid;

    Valid = false;

    if (brdPtr != NULL)
    {
        delete brdPtr;
        brdPtr = NULL;
    }

    // get the port type and number
    GuiTools::GetPortTypeAndNumber(ui->m_cmbLpt, &portType, &portNum);

    // determine the type of XS Board and set the pointer to the board object
    string brdModel= Parameters::GetXSTOOLSParameter("BoardType");
    if ("" != brdModel)
    {
        brdPtr = NewXSBoard(brdModel.c_str(), portType);
        if (NULL == brdPtr)
        {
            QMessageBox::critical(this, "Error", "Unknown type of XS Board!", QMessageBox::Ok);
        }
        else
        {
            Valid = brdPtr->Setup(errMsg_ptr, brdModel.c_str(), portNum);
            if (!Valid)
            {
                string Msg;

                Msg = "Invalid port selected!";
                QMessageBox::critical(this, "Error", Msg.c_str(), QMessageBox::Ok);

                delete brdPtr;
                brdPtr = NULL;
            }
        }
    }
    return Valid;
}


void gxsTestDlg::OnSelchangeComboLpt(const QString& index)
{
    // store the port information
    string port;

    port = ui->m_cmbLpt->itemText(ui->m_cmbLpt->currentIndex()).toStdString();
    Parameters::SetXSTOOLSParameter("PORT", port.c_str());

    // Prevent using LPT1 during GUI init
    if (Parameters::SetXSTOOLSParameter("PORT", port.c_str()))
    {
        ExtractScreenSettings();
    }
}

void gxsTestDlg::OnSelchangeCmbBoard(const QString& index)
{
    string brdModel;

    brdModel = ui->m_cmbBoard->itemText(ui->m_cmbBoard->currentIndex()).toStdString();
    Parameters::SetXSTOOLSParameter("BoardType",brdModel.c_str());
}


void gxsTestDlg::CancelButton_clicked()
{
    close();
}

void gxsTestDlg::OnTest()
{
    ExtractScreenSettings();

    if (NULL != brdPtr)
    {
        brdPtr->Test();

        delete brdPtr;
        brdPtr = NULL;
    }
}


