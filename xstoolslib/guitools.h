#ifndef GUITOOLS_H
#define GUITOOLS_H

#include <QtWidgets>

#include "xsboard.h"

class GuiTools
{
public:
    GuiTools();

    /// Subroutine for enabling/disabling prompts to user.
    void EnableBatch(bool b); ///< true if user prompts should be disabled for batch processing; false if user prompts are enabled

    static void SetPortList(QComboBox *cb);
    static void SetBoardList(QComboBox *cb);
    static bool GetPortTypeAndNumber(QComboBox *cb, PortType *portType, int *portNum);

    static void TellUser(const char *Msg);

    /// Subroutine for prompting the user for a response.
    ///\return RESPONSE_CONTINUE if user selects OK; RESPONSE_CANCEL if user selects CANCEL
    static int PromptUser(string& msg, int action);

};

#endif // GUITOOLS_H
