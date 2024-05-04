#ifndef TRANSPORT_H
#define TRANSPORT_H


#include <list>
#include <string>

#include "xserror.h"

class Transport
{
public:
    Transport();

    /// Load the DLL containing API subroutines that interface to the Microchip USB driver.
    ///\return USB_SUCCESS or USB_FAILURE
    virtual int InitTransport() = 0;

    static int ScanForPorts(XSError *err, unsigned int num_trials = 1);

private:
    // return the number of ports found
    static int ScanPorts(XSError *err,	list<string> *TransportNames);

};

#endif // TRANSPORT_H
