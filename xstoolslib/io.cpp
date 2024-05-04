/* Linux Parallel Port+ v0.2 - Shivendra Jairam
 * Contact: linuxfreak87@gmail.com
 * Updated: 11/10/07
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <string.h>

#include "io.h"

int g_baseaddr;	//  base address

PortRegisters LastRead;                  // Shadow of the last read of the registers
PortRegisters PendingWrite;              // A copy of the above with pending bits for writing.

const PinDef PinDefinitions[17] =
{
   {2, 0x01, 0, 1}, // Pin 1
   {0, 0x01, 0, 0}, // Pin 2
   {0, 0x02, 1, 0}, // Pin 3
   {0, 0x04, 2, 0}, // Pin 4
   {0, 0x08, 3, 0}, // Pin 5
   {0, 0x10, 4, 0}, // Pin 6
   {0, 0x20, 5, 0}, // Pin 7
   {0, 0x40, 6, 0}, // Pin 8
   {0, 0x80, 7, 0}, // Pin 9
   {1, 0x40, 6, 0}, // Pin 10
   {1, 0x80, 7, 1}, // Pin 11
   {1, 0x20, 5, 0}, // Pin 12
   {1, 0x10, 4, 0}, // Pin 13
   {2, 0x02, 1, 1}, // Pin 14
   {1, 0x08, 3, 0}, // Pin 15
   {2, 0x04, 2, 0}, // Pin 16
   {2, 0x08, 3, 1}  // Pin 17
};

IOPort::IOPort(int Address, bool writable)
{
    Addr = Address;
    CanWrite = writable;
}

void IOPort::write(unsigned char Value)
{
    if (CanWrite)
    {
        ioperm(Addr, RegisterCount, 1);
        outb(Value, Addr);
        ioperm(Addr, RegisterCount, 0);
    }
}

unsigned char IOPort::readChar()
{
    unsigned char Value;

    ioperm(Addr, RegisterCount, 1);
    Value = inb(Addr);
    ioperm(Addr, RegisterCount, 0);
    return Value;
}


// Returns the bit value for the given Pin from the last parallel port register read.
int PortReadPin(int PinNumber)
{
   const PinDef *Ctrl;
   int Val;

   Ctrl = &PinDefinitions[PinNumber - 1];
 
   Val = (LastRead.Data[Ctrl->Addr] >> Ctrl->Shift) & 0x01;
   
   // Handle port bits that are harware inverted
   Val ^= Ctrl->Invert;
   return Val;
}

// Assigns a bit value for the given Pin
void PortWritePin(int PinNumber, int Value)
{
   const PinDef *Ctrl;

   Ctrl = &PinDefinitions[PinNumber - 1];

   // Handle port bits that are harware inverted
   Value ^= Ctrl->Invert;

   PendingWrite.Data[Ctrl->Addr] &= ~Ctrl->Mask;
   if (1 == Value)
   {
      PendingWrite.Data[Ctrl->Addr] |= Ctrl->Mask;
   }
}

int PortReadData()
{
   return LastRead.Data[0];
}

void PortWriteData(int Val)
{
   PendingWrite.Data[0] = Val;
}

// Sends writes to the parallel port registers.
void PortRegisterSend()
{
   int i;
   
   //printf("Sent:");
   ioperm(g_baseaddr, RegisterCount, 1);
   for (i = 0; i < RegisterCount; i++)
   {
      outb(PendingWrite.Data[i], g_baseaddr + i);
      //printf("0x%02X, ", PendingWrite.Data[i]);
   }
   ioperm(g_baseaddr, RegisterCount, 0);
   //printf("\n");
}

void PortRegisterRead()
{
   int i;

   // If the user has any pending bit changes to send to the port, we send them before overwriting with a new read.
   if (0 != memcmp(&PendingWrite, &LastRead, sizeof(PortRegisters)))
   {
      PortRegisterSend();
   }
   // In theory updates to pending write bits could occur from between here and the end of this method.
   
   //printf("Read:");
   ioperm(g_baseaddr, RegisterCount, 1);
   for (i = 0; i < RegisterCount; i++)
   {
      LastRead.Data[i] = inb(g_baseaddr + i);
      //printf("0x%02X, ", LastRead.Data[i]);
   }
   ioperm(g_baseaddr, RegisterCount, 0);
   
   // Reset the pending write buffer
   PendingWrite = LastRead;
   //printf("\n");
}

// Function to detect the address of the parallel port
// Callback function for button "Detect"
char *PortFind()
{
   FILE *output_file; // define stream
   static char c[50];     
   int user_num=0;
   
   strcpy(c, "");
   if ((output_file = fopen("/proc/ioports", "r")) == NULL)
   {
      printf("Oops, can't find it /proc. You need to enter it manually\n");
      strcpy(c, "Oops, can't find it /proc");
   }
   else 
   {
      while (fgets(c, 50, output_file) != NULL)
      { 
         if (strstr(c, "parport") != NULL)
            // because two instances of parport may exist. Its usually the first one.
            if (user_num == 0)
            {
               ++user_num;
               break;
               printf("The address range seems to be %s\n",c);
            }
      }
      // closes the stream, output_file upon ternimation of the while loop.
      fclose(output_file);
      printf("Proceed to enter the base address\n");
    }
   return c;
}


// This function echoes back (to the update label widget (main UI) the address 
// entered and also sets global baseaddr
// Callback function for button "Address"
int PortSetAddress(int baseaddr)
{
   g_baseaddr = baseaddr;
   printf("\nPort Address set to 0x%04X\n", g_baseaddr);
   
   // Initialise the shadow register buffers
   PortRegisterRead();
   
   return 0;
}

int PortDeactivate()
{
   printf("\nSending 0x00 to port and giving up access permissions\n");

   PortWriteData(0x00);
   PortRegisterSend();
   return 0;
}
