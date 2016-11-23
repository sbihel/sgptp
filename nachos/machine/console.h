/*! \file console.h 
    \brief Data structures to simulate the behavior of a terminal
   
  	I/O device.  A terminal has two parts -- a keyboard input,
  	and a display output, each of which produces/accepts 
  	characters sequentially.
  
  	The console hardware device is asynchronous.  When a character is
  	written to the device, the routine returns immediately, and an 
  	interrupt handler is called later when the I/O completes.
  	For reads, an interrupt handler is called when a character arrives. 
  
  	The user of the device can specify the routines to be called when 
  	the read/write interrupts occur.  There is a separate interrupt
  	for read and write, and the device is "duplex" -- a character
  	can be outgoing and incoming at the same time.

    DO NOT CHANGE -- part of the machine emulation
  
 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
 of liability and disclaimer of warranty provisions.
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include "kernel/copyright.h"
#include "kernel/synch.h"
#include "utility/utility.h"

/*! \brief Defines a hardware console device.
//
// Input and output to the device is simulated by reading 
// and writing to UNIX files ("readFile" and "writeFile").
//
// Since the device is asynchronous, the console "read" interrupt handler
// is called when a character has arrived, ready to be read in.
// The console "write" interrupt handler is called when an output character 
// has been "put", so that the next character can be written.
*/
class Console {
  public:

  /** 	Constructor. Initialize the simulation of a hardware console device.
   //
   //	\param readFile UNIX file simulating the keyboard (NULL -> use stdin)
   //	\param writeFile UNIX file simulating the display (NULL -> use stdout)
   // 	\param readAvail is the interrupt handler called when a character 
   //              arrives from the keyboard
   // 	\param writeDone is the interrupt handler called when a character has
   //		been output, so that it is ok to request the next char be
   //		output
   */
    Console(char *readFile, char *writeFile, VoidNoArgFunctionPtr readAvail, 
	VoidNoArgFunctionPtr writeDone);
	
    //! Destructor. Clean up console emulation
    ~Console();	

    // External interface -- Nachos kernel code can call these

    /*! 	
    // Write a character to the simulated display, schedule an interrupt 
    // to occur in the future, and return.
    */
    void PutChar(char ch);

    /*! 	
    // Read a character from the input buffer, if there is any there.
    // Either return the character, or EOF if none buffered.
    // \return read character
    */
    char GetChar();

    /*! Enable the console interrupt 
     */
    void EnableInterrupt();
    
    /*! Disable the console interrupt
     */ 
    void DisableInterrupt();

    // Internal emulation routines -- DO NOT call these. 

    /*! Internal routine called when it is time to invoke the interrupt
    //	handler to tell the Nachos kernel that the output character has
    //	completed.
    */
    void WriteDone();

    /*! 	
    // Periodically called to check if a character is available for
    // input from the simulated keyboard (eg, has it been typed?).
    //
    // Only read it in if there is buffer space for it (if the previous
    // character has been grabbed out of the buffer by the Nachos kernel).
    // Invoke the "read" interrupt handler, once the character has been 
    // put into the buffer. 
    */
    void CheckCharAvail();

  private:
    bool intState;                      //!< Interrupt status
    
    int readFileNo;			//!< UNIX file emulating the keyboard 
    int writeFileNo;			//!< UNIX file emulating the display
    VoidNoArgFunctionPtr writeHandler; 	/*!< Interrupt handler to call when the PutChar I/O completes
					*/

    VoidNoArgFunctionPtr readHandler; 	/*!< Interrupt handler to call when a character arrives from the keyboard
					*/

    bool putBusy;    			/*!< Is a PutChar operation in progress? If so, you can't do another one!
					*/
    char incoming;    			/*!< Contains the character to be read,
					  if there is one available. Otherwise contains EOF.
					*/
};

#endif // CONSOLE_H
