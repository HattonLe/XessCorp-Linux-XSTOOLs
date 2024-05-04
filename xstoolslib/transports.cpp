#include "transports.h"

#include "usbjtag.h"
#include "lptjtag.h"

int Transports::numActivePorts = 0;
list<string> Transports::TransportNames;

Transports::Transports()
{

}

/// Scan LPT and USB ports for XESS Boards
///< pointer to error reporting object
///< try this number of times to open each device
///\return the number of XESS Boards found
int Transports::ScanPorts(XSError *err)
{
    int i;
    int numJTAG;
    PortType type;

    numActivePorts = 0;

    // remove any objects whose USB connections are no longer working
    for (i = 0; i < MAX_NUM_DEV; i++)
    {
        if(primaryJTAG[i] != NULL)
        {
            primaryJTAG[i]->Close();
            if(!primaryJTAG[i]->Open(i))
            {
                delete primaryJTAG[i];
                primaryJTAG[i] = NULL;
            }
            else
            {
                numActivePorts++;
                primaryJTAG[i]->Close(); // don't leave USB ports open when not actively in use
            }
        }
        if(secondaryJTAG[i] != NULL)
        {
            secondaryJTAG[i]->Close();
            if(!secondaryJTAG[i]->Open(i))
            {
                delete secondaryJTAG[i];
                secondaryJTAG[i] = NULL;
            }
            else
            {
                secondaryJTAG[i]->Close(); // don't leave USB ports open when not actively in use
            }
        }
    }

    // scan for and add any new connections
    type = PORTTYPE_LPTJTAG;
    switch(type)
    {
        case PORTTYPE_LPTJTAG:
            Transports::TransportNames = PPort::ScanHardware(err);
        break;

        case PORTTYPE_USBJTAG:
            //Transports::TransportNames = USBPort::ScanHardware(err);
        break;

        default:
        Transports::TransportNames.clear();
        break;
    }
    return Transports::TransportNames.size();

    for (i = 0; i < MAX_NUM_DEV && numJTAG > numActivePorts; i++)
    {
        if (primaryJTAG[i] == NULL)
        {
            switch(type)
            {
                case PORTTYPE_LPTJTAG:
                    primaryJTAG[i] = new LPTJTAG(err, i+1, 0x0b8000,1,2,0,12,7);
                break;

                case PORTTYPE_USBJTAG:
                    //primaryJTAG[i] = new USBJTAG(err, i, 1);
                break;

                default:
                    break;
            }

            if (!primaryJTAG[i]->Open(i))
            {
                delete primaryJTAG[i];
                primaryJTAG[i] = NULL;
            }
            else
            {
                // primary USB port to FPGA was found, so we have an active USB port
                primaryJTAG[i]->Close();
                numActivePorts++;

                switch(type)
                {
                    case PORTTYPE_LPTJTAG:
                        secondaryJTAG[i] = new LPTJTAG(err,i+1,0x0b8000,17,18,19,15,0);
                    break;

                    // look for the secondary port to the CPLD (doesn't exist if this is XuLA board)
                    case PORTTYPE_USBJTAG:
                        //secondaryJTAG[i] = new USBJTAG(err, i, 2);
                    break;

                    default:
                        break;
                }

                if (!secondaryJTAG[i]->Open(i))
                {
                    delete secondaryJTAG[i];
                    secondaryJTAG[i] = NULL;
                    delete primaryJTAG[i];
                    primaryJTAG[i] = NULL;
                }
                else
                {
                    secondaryJTAG[i]->Close();
                }
            }
        }
    }
//    DEBUG_STMT("Number of active LPT ports =  " << numActivePorts)

    return numActivePorts;
}

/// Get a pointer to an active LPT or USB JTAG port object.
///< port index between 1 and # of active ports
///< port index between 0 and # of active ports-1
///< endpoint: 1 for primary endpoint, 2 for secondary endpoint
/// return a pointer to the JTAG Port
JTAGPort* Transports::GetPort(int portNum, int endptNum)
{
    cerr << "portNum = " << portNum << "\nnumActivePorts = " << numActivePorts << endl;
    //DEBUG_STMT("portNum = " << portNum << "\nnumActivePorts = " << numActivePorts << endl)
    if(portNum>=numActivePorts)
    {
        cerr << "port number too large!!\n";
        return NULL;
    }

    int i,j;
    for(i=0,j=0; i<MAX_NUM_DEV; i++)
    {
        if(primaryJTAG[i] != NULL)
        {
            if(j == portNum)
            {
                switch(endptNum)
                {
                case 1: return primaryJTAG[i];
                case 2: return secondaryJTAG[i];
                default: return NULL;
                }
            }
            j++;
        }
    }

    //DEBUG_STMT("Couldn't find an active USBJTAG port!!\n")
    return NULL;
}
