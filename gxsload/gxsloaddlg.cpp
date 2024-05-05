
#include <QtWidgets>

#include <cassert>
#include <cstdlib>
#include <strstream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

#include "gxsloaddlg.h"
#include "ui_gxsloaddlg.h"

#include "../xstoolslib/utils.h"
#include "../xstoolslib/pport.h"
#include "../xstoolslib/guitools.h"
#include "../xstoolslib/utils.h"
#include "../xstoolslib/parameters.h"

#include "../xstoolslib/guitools.h"
#include "../xstoolslib/transports.h"

#define ENDIAN_DEFAULTS true

enum {RAMSOURCE, FLASHSOURCE};

Transports *Transports::Singleton = new Transports();

GxsloadDlg::GxsloadDlg(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GxsloadDlg)
{
    ui->setupUi(this);

    qApp->installEventFilter(this);

    this->setFocusPolicy(Qt::StrongFocus);

    connect(ui->m_lstFPLD, SIGNAL(SelectionUpdated(const QString&)), this, SLOT(SelUpdatedFPLD(const QString&)));
    connect(ui->m_lstNONVOL, SIGNAL(SelectionUpdated(const QString&)), this, SLOT(SelUpdatedNON(const QString&)));
    connect(ui->m_lstRAM, SIGNAL(SelectionUpdated(const QString&)), this, SLOT(SelUpdatedRAM(const QString&)));

    connect(ui->m_lstNONVOL, SIGNAL(FileUploadRequested(const QString&)), this, SLOT(UploadData(const QString&)));
    connect(ui->m_lstRAM, SIGNAL(FileUploadRequested(const QString&)), this, SLOT(UploadData(const QString&)));
    SelFPLD = 0;
    SelRAM = 0;
    SelNON = 0;

    UpdateLoadButton();

    connect(ui->ButtonExit, SIGNAL(clicked()), SLOT(ExitButton_clicked()));
    connect(ui->m_btnLoad, SIGNAL(clicked()), SLOT(OnLoad()));
    connect(ui->m_cmbLpt, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnSelchangeComboLpt(const QString&)));
    connect(ui->m_cmbBoard, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnSelchangeCmbBoard(const QString&)));
    connect(ui->m_flashFormat, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnSelchangeFlashformat(const QString&)));
    connect(ui->m_ramFormat, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnSelchangeRamformat(const QString&)));
    connect(ui->m_flashIntfcDownload, SIGNAL(clicked()), SLOT(DownloadSelectedFiles()));
    connect(ui->m_ramIntfcDownload, SIGNAL(clicked()), SLOT(DownloadSelectedFiles()));

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
            string flashFmt;
            string ramFmt;
            string brdType;
            string portName;

            // get the last settings
            portName = Parameters::GetXSTOOLSParameter("PORT");
            if ("" == portName)
            {
                 // look for old-style LPT parameter if PORT parameter is empty
                portName = Parameters::GetXSTOOLSParameter("LPT");
            }

            ramFmt = Parameters::GetXSTOOLSParameter("RAMFormat");
            flashFmt = Parameters::GetXSTOOLSParameter("FlashFormat");
            brdType = Parameters::GetXSTOOLSParameter("BoardType");

            // stored in the XSTOOLS parameter file
            GuiTools::SetPortList(ui->m_cmbLpt);
            GuiTools::SetBoardList(ui->m_cmbBoard);

            // setup the list of Flash file formats in the pulldown list
            ui->m_flashFormat->addItem("HEX");
            ui->m_flashFormat->addItem("MCS");
            ui->m_flashFormat->addItem("EXO-16");
            ui->m_flashFormat->addItem("EXO-24");
            ui->m_flashFormat->addItem("EXO-32");
            ui->m_flashFormat->addItem("XESS-16");
            ui->m_flashFormat->addItem("XESS-24");
            ui->m_flashFormat->addItem("XESS-32");

            // setup the list of Flash file formats in the pulldown list
            ui->m_ramFormat->addItem("HEX");
            ui->m_ramFormat->addItem("MCS");
            ui->m_ramFormat->addItem("EXO-16");
            ui->m_ramFormat->addItem("EXO-24");
            ui->m_ramFormat->addItem("EXO-32");
            ui->m_ramFormat->addItem("XESS-16");
            ui->m_ramFormat->addItem("XESS-24");
            ui->m_ramFormat->addItem("XESS-32");

            ui->m_flashIntfcDownload->setChecked(true);

            ui->m_ramIntfcDownload->setChecked(true);

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

            if ("" == flashFmt)
            {
                // set the default value if no previous value exists
                Parameters::SetXSTOOLSParameter("FlashFormat", "HEX");
                flashFmt = Parameters::GetXSTOOLSParameter("FlashFormat");
            }

            if ("" == ramFmt)
            {
                // set the default value if no previous value exists
                Parameters::SetXSTOOLSParameter("RAMFormat", "HEX");
                ramFmt = Parameters::GetXSTOOLSParameter("RAMFormat");
            }

            ui->m_cmbLpt->setCurrentIndex(ui->m_cmbLpt->findText(portName.c_str()));
            ui->m_cmbBoard->setCurrentIndex(ui->m_cmbBoard->findText(brdType.c_str()));
            ui->m_flashFormat->setCurrentIndex(ui->m_flashFormat->findText(flashFmt.c_str()));
            ui->m_ramFormat->setCurrentIndex(ui->m_ramFormat->findText(ramFmt.c_str()));

            ExtractScreenSettings();
        }
    }
}

GxsloadDlg::~GxsloadDlg()
{
    delete ui;
}

bool GxsloadDlg::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        //qDebug() << "key " << keyEvent->key() << "from" << obj;

        // Pass key events to the list box.
        if (0 == (strcmp("MyList", obj->metaObject()->className())))
        {
            ((MyList *) obj)->HandleKeyEvent(keyEvent);

        }
    }
    return QObject::eventFilter(obj, event);
}

void GxsloadDlg::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "event->key(): " << event->key();

    QWidget::keyPressEvent(event);
}

void GxsloadDlg::UpdateLoadButton()
{
    ui->m_btnLoad->setEnabled(0 < (SelFPLD + SelNON + SelRAM));
}

void GxsloadDlg::SelUpdatedFPLD(const QString& Count)
{
//    qDebug() << "SelUpdatedFPLD " << Count.toInt();

    SelFPLD = Count.toInt();
    UpdateLoadButton();
}

void GxsloadDlg::SelUpdatedNON(const QString& Count)
{
//    qDebug() << "SelUpdatedNON " << Count.toInt();

    SelNON = Count.toInt();
    UpdateLoadButton();
}

void GxsloadDlg::SelUpdatedRAM(const QString& Count)
{
//    qDebug() << "SelUpdatedRAM " << Count.toInt();

    SelRAM = Count.toInt();
    UpdateLoadButton();
}

bool GxsloadDlg::ExtractScreenSettings()
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


void GxsloadDlg::OnSelchangeComboLpt(const QString& index)
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

void GxsloadDlg::OnSelchangeCmbBoard(const QString& index)
{
    string brdModel;

    brdModel = ui->m_cmbBoard->itemText(ui->m_cmbBoard->currentIndex()).toStdString();
    Parameters::SetXSTOOLSParameter("BoardType",brdModel.c_str());
}

void GxsloadDlg::OnSelchangeFlashformat(const QString& index)
{
   string fmt;

   fmt = ui->m_flashFormat->itemText(ui->m_flashFormat->currentIndex()).toStdString();
   Parameters::SetXSTOOLSParameter("FlashFormat", fmt.c_str());
}

void GxsloadDlg::OnSelchangeRamformat(const QString& index)
{
   string fmt;

   fmt = ui->m_ramFormat->itemText(ui->m_ramFormat->currentIndex()).toStdString();
   Parameters::SetXSTOOLSParameter("RAMFormat", fmt.c_str());
}

void GxsloadDlg::ExitButton_clicked()
{
    close();
}

void GxsloadDlg::OnLoad()
{
    DownloadSelectedFiles();
}

void GxsloadDlg::DownloadSelectedFiles(void)
{
    XSError errMsg(cerr); // setup error channel
    QStringList *SelectedFPLD;
    QStringList *SelectedRAM;
    QStringList *SelectedNONVOL;

    SelectedFPLD = ui->m_lstFPLD->SelectedItems();
    SelectedRAM = ui->m_lstRAM->SelectedItems();
    SelectedNONVOL = ui->m_lstNONVOL->SelectedItems();

    // exit if no files selected for downloading
    if (0 != SelectedFPLD->count() || 0 != SelectedRAM->count() || 0 != SelectedNONVOL->count())
    {
        // get the port type and number
        if (ExtractScreenSettings())
        {
            // if any files in the RAM list are selected, then download them into RAM
            if (0 != SelectedRAM->count())
            {
                bool ramIntfcDownload;
                bool UserCancelled;
                bool FirstFile;
                int LastFile;
                int i;
                string FormatName;

                FormatName = ui->m_ramFormat->itemText(ui->m_ramFormat->currentIndex()).toStdString();
                ramIntfcDownload = ui->m_ramIntfcDownload->isChecked();

                FirstFile = true;
                LastFile = SelectedRAM->count() - 1;
                i = 0;
                UserCancelled = false;
                while (!UserCancelled && (i < SelectedRAM->count()))
                {
                    string dirAndFileName;

                    dirAndFileName = SelectedRAM->value(i).toStdString();

                    if (!brdPtr->DownloadRAM(dirAndFileName.c_str(), ENDIAN_DEFAULTS, ENDIAN_DEFAULTS, FirstFile && ramIntfcDownload, i == LastFile, &UserCancelled))
                    {
                        string instructions = "An error occurred while downloading to the RAM\n\nContinue?";
                        if (GuiTools::PromptUser(instructions, PROMPT_OKCANCEL) == RESPONSE_CANCEL)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if(errMsg.IsError())
                        {
                            string instructions = "An error occurred while downloading to the RAM\n\nContinue?";
                            if (GuiTools::PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
                            {
                                break;
                            }
                        }
                    }
                    FirstFile = false;
                    i++;
                }
            }

            // if any files in the FlashEEPROM list are selected, then download them into Flash/EEPROM
            if (0 != SelectedNONVOL->count())
            {
                bool flashIntfcDownload;
                bool UserCancelled;
                bool FirstFile;
                int LastFile;
                int i;
                string FormatName;

                FormatName = ui->m_flashFormat->itemText(ui->m_flashFormat->currentIndex()).toStdString();
                flashIntfcDownload = ui->m_flashIntfcDownload->isChecked();

                FirstFile = true;
                LastFile = SelectedNONVOL->count() - 1;
                UserCancelled = false;
                i = 0;
                while (!UserCancelled && (i < SelectedNONVOL->count()))
                {
                    string dirAndFileName;

                    dirAndFileName = SelectedNONVOL->value(i).toStdString();

                    if (!brdPtr->DownloadFlash(dirAndFileName.c_str(), ENDIAN_DEFAULTS, ENDIAN_DEFAULTS, FirstFile && flashIntfcDownload, i == LastFile, &UserCancelled))
                    {
                        string instructions = "An error occurred while downloading to the Flash\n\nContinue?";
                        if (GuiTools::PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if (errMsg.IsError())
                        {
                            string instructions = "An error occurred while downloading to the Flash\n\nContinue?";
                            if (GuiTools::PromptUser(instructions,PROMPT_OKCANCEL) == RESPONSE_CANCEL)
                            {
                                break;
                            }
                        }
                    }
                    FirstFile = false;
                    i++;
                }
            }

            // if any file in the FPGA/CPLD list is selected, then download it into RAM
            if (0 != SelectedFPLD->count() )
            {
                int numFiles;

                numFiles = SelectedFPLD->count();
                if (1 == numFiles)
                {
                    bool UserCancelled;
                    int i;

                    UserCancelled = false;
                    i = 0;
                    while (!UserCancelled && (i < SelectedFPLD->count()))
                    {
                        string dirAndFileName;

                        dirAndFileName = SelectedFPLD->value(i).toStdString();

                        if (!brdPtr->Configure(dirAndFileName.c_str(), &UserCancelled))
                        {
                            break;
                        }
                        else
                        {
                            if (errMsg.IsError())
                            {
                                break;
                            }
                        }
                        i++;
                    }
                }
            }
        }
        delete brdPtr;
        brdPtr = NULL;
    }
   delete SelectedFPLD;
   delete SelectedRAM;
   delete SelectedNONVOL;
}

void GxsloadDlg::UploadData(const QString& Name)
{
    string UploadPath;

//    qDebug() << "GxsloadDlg::UploadData()" << Name;

    UploadPath = FindXSTOOLSDataDir();
    if ("" != UploadPath)
    {
        if ("m_NONUpload" == Name)
        {
            UploadFile(FLASHSOURCE, UploadPath);
        }
        if ("m_RAMUpload" == Name)
        {
            UploadFile(RAMSOURCE, UploadPath);
        }
    }
}

// This method is used when dragging from the two upload icons... a mechanism for uploading data from the Xess Board to the PC.
bool GxsloadDlg::UploadFile(int Source, string UploadPath)
{    
    bool status;
    string Msg;
    bool UserCancelled;
    string formatName;
    string UploadFilename;
    int hiAddress, loAddress;
    bool IntfcDownload;
    string s;
    char tmp[100];
    int sLen;

    XSError errMsg(cerr); // setup error channel
    CString prtName;

//    qDebug("GxsloadDlg::UploadFile()");

    status = false;

    if (RAMSOURCE == Source)
    {
        // Assume we have good addresses so far
        status = true;

        IntfcDownload = (ui->m_ramIntfcDownload->isChecked());

        formatName = ui->m_ramFormat->currentText().toStdString();
        UploadFilename = UploadPath;
        UploadFilename += "/ramupld.";
        UploadFilename += formatName;

        s = ui->m_ramHiAddr->text().toStdString();
        if (0 >= s.length())
        {
            Msg = "You must specify an upper address for the RAM upload operation!";
            QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
            status = false;
        }
        else
        {
            if (sscanf(s.c_str(), "%x%s", &hiAddress, &tmp) != 1)
            {
                Msg = "You must specify a single upper address for RAM upload operations!";
                QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
                status = false;
            }
        }

        s = ui->m_ramLoAddr->text().toStdString();
        if (0 >= s.length())
        {
            Msg = "You must specify a lower address for the RAM upload operation!";
            QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
            status = false;
        }
        else
        {
            if (sscanf(s.c_str(), "%x%s", &loAddress, &tmp) != 1)
            {
                Msg = "You must specify a single lower address for RAM upload operations!";
                QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
                status = false;
            }
        }

        if (status)
        {
            if (NULL != brdPtr)
            {
                status = brdPtr->UploadRAM(UploadFilename.c_str(), formatName.c_str(), loAddress, hiAddress, ENDIAN_DEFAULTS, ENDIAN_DEFAULTS, IntfcDownload, true, &UserCancelled);
            }
        }
    }

    if (FLASHSOURCE == Source)
    {
        // Assume we have good addresses so far
        status = true;

        IntfcDownload = (ui->m_flashIntfcDownload->isChecked());

        formatName = ui->m_flashFormat->currentText().toStdString();
        UploadFilename = UploadPath;
        UploadFilename += "/flshupld.";
        UploadFilename += formatName;

        s = ui->m_flashHiAddr->text().toStdString();
        if (0 >= s.length())
        {
            Msg = "You must specify an upper address for the Flash upload operation!";
            QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
            status = false;
        }
        else
        {
            if (sscanf(s.c_str(), "%x%s", &hiAddress, &tmp) != 1)
            {
                Msg = "You must specify a single upper address for Flash upload operations!";
                QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
                status = false;
            }
        }

        s = ui->m_flashLoAddr->text().toStdString();
        if (0 >= s.length())
        {
            Msg = "You must specify a lower address for the Flash upload operation!";
            QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
            status = false;
        }
        else
        {
            if (sscanf(s.c_str(), "%x%s", &loAddress, &tmp) != 1)
            {
                Msg = "You must specify a single lower address for Flash upload operations!";
                QMessageBox::critical(this, "Information", Msg.c_str(), QMessageBox::Ok);
                status = false;
            }
        }

        if (status)
        {
            if (NULL != brdPtr)
            {
                status = brdPtr->UploadFlash(UploadFilename.c_str(), formatName.c_str(), loAddress, hiAddress, ENDIAN_DEFAULTS, ENDIAN_DEFAULTS, IntfcDownload, true, &UserCancelled);
            }
        }
    }
    return status;
}


