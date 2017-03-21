/*! \file drvConsole.h 
    \brief Data structures to export a synchronous interface to the console
  	   device.
  
    Copyright (c) 1992-1993 The Regents of the University of California.
    All rights reserved.  See copyright.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
*/

#ifndef SYNCHCONS_H
#define SYNCHCONS_H

class DriverConsole;

#include "machine/console.h"
#include "utility/utility.h"
#include "kernel/synch.h"
#include "kernel/system.h"


/*! \brief Defines a "synchronous" console abstraction.
//
// As with other I/O devices, the console is an asynchronous device.
// This class provides methods in order to preserve mutual exclusion
// on the console device. Two operations can be called : the first one
// writes a string to the console and the second one reads a string from
// the console. They return only when the read or write operation is 
// completed.
*/
class DriverConsole {
 public:
  DriverConsole();            // Constructor. Initialize the console driver
  ~DriverConsole();           // Destructor. Data de-allocation
  void PutString(char *buffer,int nbcar); 
                             // Write a buffer on the console
  void GetString(char *buffer,int nbcar);
                             // Read characters from the console

  void GetAChar();           // Send a char to the console device
  void PutAChar();           // Receive e char from the console

private:
  Lock *mutexget;            //!< Lock on read operations
  Lock *mutexput;            //!< Lock on write operations
  Semaphore *get, *put;      //!< Semaphores to wait for interrupts
};
    
void ConsoleGet();
void ConsolePut();

#endif // SYNCHCONS_H
