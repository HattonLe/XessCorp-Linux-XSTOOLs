/* Linux Parallel Port+ v0.2 - Shivendra Jairam
 * Contact: linuxfreak87@gmail.com
 * Updated: 11/10/07
 */

#ifndef IO_H
#define IO_H

// Parallel Port registers
#define RegisterCount 3

typedef struct 
{
   int Data[RegisterCount];         // The 3 parallel port registers
} PortRegisters;

typedef struct 
{
   int Addr;         // Port register offset (0 = base address, 1 = Ctrl register)
   int Mask;
   int Shift;
   int Invert;
} PinDef;


class IOPort
{
private:
    int Addr;
    bool CanWrite;

public:
    IOPort(int Address, bool writable);

public:
    void write(unsigned char Value);
    unsigned char readChar();
};


// Returns the bit value for the given Pin from the last parallel port register read.
int PortReadPin(int PinNumber);

// Assigns a bit value for the given Pin
void PortWritePin(int PinNumber, int Value);

int PortReadData();

void PortWriteData(int Val);

// Sends writes to the parallel port registers.
void PortRegisterSend();

void PortRegisterRead();

// Function to detect the address of the parallel port
// Callback function for button "Detect"
char *PortFind();


// This function echoes back (to the update label widget (main UI) the address 
// entered and also sets global baseaddr
// Callback function for button "Address"
int PortSetAddress(int baseaddr);


int PortDeactivate();


#endif
