
#include <QtWidgets>

#include <cassert>
#include <cstdlib>
#include <strstream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

#include "gxsportdlg.h"
#include "ui_gxsportdlg.h"

#include "../xstoolslib/utils.h"
#include "../xstoolslib/pport.h"
#include "../xstoolslib/guitools.h"
#include "../xstoolslib/utils.h"
#include "../xstoolslib/parameters.h"

#include "../xstoolslib/guitools.h"
#include "../xstoolslib/transports.h"

Transports *Transports::Singleton = new Transports();

gxsportDlg::gxsportDlg(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::gxsportDlg)
{
    ui->setupUi(this);

    connect(ui->pushButtonExit, SIGNAL(clicked()), SLOT(ExitButton_clicked()));
    connect(ui->m_btnStrobe, SIGNAL(clicked()), SLOT(StrobeButton_clicked()));
    connect(ui->m_cmbLpt, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnSelchangeComboLpt(const QString&)));
    connect(ui->m_chkCount, SIGNAL(clicked()), SLOT(OnChkCount()));
    connect(ui->m_btnData0, SIGNAL(clicked()), SLOT(OnBtnData0()));
    connect(ui->m_btnData1, SIGNAL(clicked()), SLOT(OnBtnData1()));
    connect(ui->m_btnData2, SIGNAL(clicked()), SLOT(OnBtnData2()));
    connect(ui->m_btnData3, SIGNAL(clicked()), SLOT(OnBtnData3()));
    connect(ui->m_btnData4, SIGNAL(clicked()), SLOT(OnBtnData4()));
    connect(ui->m_btnData5, SIGNAL(clicked()), SLOT(OnBtnData5()));
    connect(ui->m_btnData6, SIGNAL(clicked()), SLOT(OnBtnData6()));
    connect(ui->m_btnData7, SIGNAL(clicked()), SLOT(OnBtnData7()));

    // initialize array of button pointers to the parallel port data buttons
    dataBtn[0] = ui->m_btnData0;
    dataBtn[1] = ui->m_btnData1;
    dataBtn[2] = ui->m_btnData2;
    dataBtn[3] = ui->m_btnData3;
    dataBtn[4] = ui->m_btnData4;
    dataBtn[5] = ui->m_btnData5;
    dataBtn[6] = ui->m_btnData6;
    dataBtn[7] = ui->m_btnData7;

    brdPtr = NULL;

    // for testing only
    setenv("XSTOOLS", "/home/main/Documents/MyXSTOOLs/XessData", 1);
    setenv("XSTOOLS_BIN_DIR", "/home/main/Documents/MyXSTOOLs/XessData", 1);

    errMsg_ptr = new XSError(cerr);

    if (getuid())
    {
        string Msg;

        Msg = "You need to run this as root for access to Parallel Ports";
        QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
    }
    else
    {
        string portName;
        string brdType;

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

        ExtractScreenSettings();
    }
}

gxsportDlg::~gxsportDlg()
{
    delete ui;
}


bool gxsportDlg::ExtractScreenSettings()
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


void gxsportDlg::OnSelchangeComboLpt(const QString& index)
{
    // store the port information
    string port;

    port = ui->m_cmbLpt->itemText(ui->m_cmbLpt->currentIndex()).toStdString();

    // Prevent using LPT1 during GUI init
    if (Parameters::SetXSTOOLSParameter("PORT", port.c_str()))
    {
        if (ExtractScreenSettings())
        {
            // display the port data on the buttons
            newPortDataVal = brdPtr->GetTestVector();
            DisplayBtnData(newPortDataVal);
        }
    }
}

// initialize the button text to reflect the data value
void gxsportDlg::DisplayBtnData(unsigned int data)
{
    assert(data>=0 && data<=255);

    int i;

    for (i = 0; i < 8; i++, data >>=1)
    {
        if ((data & 1) == 0)
        {
            dataBtn[i]->setText("0");
        }
        else
        {
            dataBtn[i]->setText("1");
        }
    }

}


void gxsportDlg::OnBtnData(int bitPos)
{
    assert(bitPos>=0 && bitPos<=7);

    // toggle the data bit and show the new value on the button
    if(newPortDataVal & (1<<bitPos))
    {
        // data bit is 1 so reset it to 0
        newPortDataVal &= (~(1<<bitPos));
        dataBtn[bitPos]->setText("0");
    }
    else
    {
        // data bit is 0 so set it to 1
        newPortDataVal |= (1<<bitPos);
        dataBtn[bitPos]->setText("1");
    }

    HandleStrobe();
}

void gxsportDlg::OnBtnData0()
{
    OnBtnData(0);
}

void gxsportDlg::OnBtnData1()
{
    OnBtnData(1);
}

void gxsportDlg::OnBtnData2()
{
    OnBtnData(2);
}

void gxsportDlg::OnBtnData3()
{
    OnBtnData(3);
}

void gxsportDlg::OnBtnData4()
{
    OnBtnData(4);
}

void gxsportDlg::OnBtnData5()
{
    OnBtnData(5);
}

void gxsportDlg::OnBtnData6()
{
    OnBtnData(6);
}

void gxsportDlg::OnBtnData7()
{
    OnBtnData(7);
}

void gxsportDlg::HandleStrobe()
{
    // enable strobe button when counting or if the new data value
    // is different from the current port data
    if (!ui->m_chkCount->isChecked())
    {
        // just setting data value with buttons
        if(newPortDataVal==brdPtr->GetTestVector())
            ui->m_btnStrobe->setEnabled(false);
        else
            ui->m_btnStrobe->setEnabled(true);
    }
    else
    { // here we are counting up the data value on the port
        ui->m_btnStrobe->setEnabled(true);
    }
}

void gxsportDlg::OnBtnStrobe()
{
    if (!ui->m_chkCount->isChecked())
    {
        // place the new data value on the parallel port when strobe is clicked
        brdPtr->ApplyTestVectors(newPortDataVal,0xFF);
        // then display the strobe button appropriately
        HandleStrobe();

        // now the written value should match the value that is out there
        if (newPortDataVal != brdPtr->GetTestVector())
        {
            string Msg;

            Msg = "Parallel Port Not responding!";
            QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
        }
    }
    else
    {
        // incrementing data port value
        brdPtr->ApplyTestVectors(newPortDataVal+1,0xFF);
        newPortDataVal = brdPtr->GetTestVector();
        DisplayBtnData(newPortDataVal);
    }
}


void gxsportDlg::ExitButton_clicked()
{
    close();
}

void gxsportDlg::OnChkCount()
{
    HandleStrobe();
}

void gxsportDlg::StrobeButton_clicked()
{
    if (!ui->m_chkCount->isChecked())
    {
        // place the new data value on the parallel port when strobe is clicked
        brdPtr->ApplyTestVectors(newPortDataVal,0xFF);
        // then display the strobe button appropriately
        HandleStrobe();

        // now the written value should match the value that is out there
        if (newPortDataVal != brdPtr->GetTestVector())
        {
            string Msg;

            Msg = "Parallel Port Not responding!";
            QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
        }
    }
    else
    {
        // incrementing data port value
        brdPtr->ApplyTestVectors(newPortDataVal+1,0xFF);
        newPortDataVal = brdPtr->GetTestVector();
        DisplayBtnData(newPortDataVal);
    }
}
