
#include <list>
#include <string>

#include "guitools.h"
#include "transports.h"
#include "utils.h"
#include "parameters.h"

extern Transports AppTransports;



static bool batch = false;	// stops prompts to user when true


GuiTools::GuiTools()
{
}

/// Subroutine for enabling/disabling prompts to user.
void GuiTools::EnableBatch(bool b) ///< true if user prompts should be disabled for batch processing; false if user prompts are enabled
{
    batch = b;
}

void GuiTools::TellUser(const char *Msg)
{
    QMessageBox::critical(NULL, "Information", Msg, QMessageBox::Ok);
}

/// Subroutine for prompting the user for a response.
///\return RESPONSE_CONTINUE if user selects OK; RESPONSE_CANCEL if user selects CANCEL
int GuiTools::PromptUser(string& msg, int action)
{
    if(batch)
        return RESPONSE_CONTINUE;

    switch(action)
    {
        case PROMPT_OKCANCEL:
            QMessageBox::StandardButton Reply;
            Reply = QMessageBox::information(NULL, "Information", msg.c_str(), QMessageBox::Ok | QMessageBox::Cancel);
            return Reply == QMessageBox::Cancel ? RESPONSE_CANCEL : RESPONSE_CONTINUE;

        case PROMPT_OK:
        default:
            QMessageBox::information(NULL, "Information", msg.c_str(), QMessageBox::Ok);
            return RESPONSE_CONTINUE;
    }
}



/// Populate the list of active parallel and USB ports.
void GuiTools::SetPortList(QComboBox *cb) ///< combo-box list to hold the active port names
{
    int numPorts;
    QStringList list;
    std::list<string> Names;

    cb->clear();	// remove all the ports from the list

    // setup the list of parallel ports in the pulldown list
    XSError err(cerr);

    numPorts = Transports::Singleton->ScanPorts(&err);

    // setup the list of all ports in the pulldown list
    if (numPorts > 0)
    {
        string portName;

        Names = Transports::Singleton->GetTransportNames();

        for (string Name : Names)
        {
            list += Name.c_str();
        }
        cb->addItems(list);
    }
}

/// Populate the list of XS boards.
void GuiTools::SetBoardList(QComboBox *cb) ///< combo-box list to hold the board model names
{
    // read-in the board information
    XSBoardInfo* brdInfo;
    int numBoards;

    numBoards = GetXSBoardInfo(&brdInfo);

    // populate the list with the board names
    cb->clear();
    for (int i = 0; i < numBoards; i++)
    {
        cb->addItem(brdInfo[i].brdModel);
    }
}

/// Return the port type and index number for the selected port in the list.
bool GuiTools::GetPortTypeAndNumber(QComboBox *cb,		///< combo-box list that holds the port names
                          PortType *portType,	///< return the type of port (USB or LPT) here
                          int *portNum)			///< return the port index here
{
    int curSel;
    if ((curSel=cb->currentIndex()) == -1)
    {
        *portType = PORTTYPE_INVALID;
        *portNum = 0;
        return false;	// failure
    }

    string portName;

    portName = cb->itemText(curSel).toStdString();

    if(portName.substr(0,3) == "LPT")
    {
        int n = sscanf(portName.c_str()+strlen("LPT"),"%d",portNum);
        assert(n != 0);
        *portType = PORTTYPE_LPT;
    }
    else if(portName.substr(0,3) == "USB")
    {
        int numPorts;

        // refresh the pointers to USB ports in case a
        // USB board has been unplugged and then re-plugged
        XSError err(cerr);
        numPorts = Transports::Singleton->ScanPorts(&err);

        int n = sscanf(portName.c_str()+strlen("USB"),"%d",portNum);
        assert(n != 0);
        *portType = PORTTYPE_USBJTAG;
    }
    else
    {
        *portType = PORTTYPE_INVALID;
        *portNum = 0;
        return false;	// failure
    }
    return true;	// success
}

