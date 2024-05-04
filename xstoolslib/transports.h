#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include <list>
#include <string>

#include "jtagport.h"

#include "xsboard.h"
#include "xserror.h"

#define MAX_NUM_DEV	4
static JTAGPort *primaryJTAG[MAX_NUM_DEV] = {NULL,};
static JTAGPort *secondaryJTAG[MAX_NUM_DEV] = {NULL,};

class Transports
{
public:
    Transports();

    /// Scan LPT and USB ports for XESS Boards.
    ///\return the number of XESS Boards found
    int ScanPorts(XSError *err);

    list<string> GetTransportNames() { return TransportNames; };

    /// Get a pointer to an active LPT or USB JTAG port object.
    ///< type of port, either LPTJTAG or USBJTAG
    ///< port index between 0 and # of active ports
    ///< endpoint: 1 for primary endpoint, 2 for secondary endpoint
    ///\return a pointer to the JTAG port object
    JTAGPort* GetPort(int portNum, int endptNum);

public:
    static Transports *Singleton;

private:
    static list<string> TransportNames;

private:
    static int numActivePorts;
};

#endif // TRANSPORTS_H
