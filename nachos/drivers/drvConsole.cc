/*! \file drvConsole.cc 
//  \brief Routines to synchronously access the console
//
//      The console is an asynchronous device (requests return 
//      immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the console providing a synchronous interface
//      (requests wait until the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the console can only
//	handle one read operation and one write operation at a time,
//      use two locks to enforce mutual exclusion.
//
//  Copyright (c) 1992-1993 The Regents of the University of California.
//  All rights reserved.  See copyright.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
*/

#include "machine/interrupt.h"
#include "drivers/drvConsole.h"

//----------------------------------------------------------------------
// static void ConsoleGet
/*! 	Console write interrupt handler. Needs this to be a C routine, 
//      because C++ can't handle pointers to member functions.
*/
//----------------------------------------------------------------------
void ConsoleGet() {
  g_console_driver->GetAChar();
}

//----------------------------------------------------------------------
// static void ConsolePut
/*! 	Console read interrupt handler.  Needs this to be a C routine, 
//      because C++ can't handle pointers to member functions.
*/
//----------------------------------------------------------------------
void ConsolePut() {
  g_console_driver->PutAChar();
}

//-----------------------------------------------------------------
// DriverConsole::DriverConsole
/*!     Constructor. 
//      Initialize the console driver (lock and semaphore creation)
*/
//-----------------------------------------------------------------
DriverConsole::DriverConsole(){
  get = new Semaphore((char*)"get",0);
  put = new Semaphore((char*)"put",0);
  mutexget = new Lock((char*)"mutex get");
  mutexput = new Lock((char*)"mutex put");
}


//-----------------------------------------------------------------
// DriverConsole::~DriverConsole
/*!     Destructor.
//      De-allocate data structures needed by the console driver
//      (semaphores, locks).
*/
//-----------------------------------------------------------------
DriverConsole::~DriverConsole() {
  delete mutexget;
  delete mutexput;
  delete get;
  delete put;
}


//-----------------------------------------------------------------
// DriverConsole::PutAChar
/*!     Operate a V on the "write" semaphore to signal a char is written.
//      The method is called by the interrupt handler ConsolePut.
*/
//-----------------------------------------------------------------
void DriverConsole::PutAChar(){
  
  IntStatus oldLevel = g_machine->interrupt->SetStatus(INTERRUPTS_OFF);
  put->V();
  (void) g_machine->interrupt->SetStatus(oldLevel);
}


//-----------------------------------------------------------------
// DriverConsole::PutString
/*!     Send a string to the console device using a lock to insure
//      mutual exclusion. The method returns when all characters 
//      has been sent.
// 
//      \param buffer contains the data to send
//      \param nbcar is the number of chars to send
*/
//-----------------------------------------------------------------
void DriverConsole::PutString(char *buffer,int nbcar) {
  
  mutexput->Acquire();

  for (int i=0;i<nbcar;i++) {
    g_current_thread->GetProcessOwner()->stat->incrNumCharWritten();
    g_machine->console->PutChar(buffer[i]);
    put->P();
  }
  
  mutexput->Release();
}

//-----------------------------------------------------------------
// DriverConsole::GetAChar
/*!     Operate a V on the "read" semaphore to signal a char is read.
//      The method is called by the interrupt handler ConsoleGet.
*/
//-----------------------------------------------------------------
void DriverConsole::GetAChar(){
  
  IntStatus oldLevel = g_machine->interrupt->SetStatus(INTERRUPTS_OFF);
  get->V();
  (void) g_machine->interrupt->SetStatus(oldLevel);

}


//-----------------------------------------------------------------
// DriverConsole::GetString
/*!     Receive a string from the console device using a lock to 
//      prevent from concurrent accesses. The method returns when
//      all characters has been received.
//
//      \param buffer is the structure to fill
/       \param size is the number max of char to be received
*/
//-----------------------------------------------------------------
void DriverConsole::GetString(char *buffer,int nbcar) {
  
  char c = 0;
  int i;

  mutexget->Acquire();
  g_machine->console->EnableInterrupt();
  
  for (i=0;((i<nbcar) && (c!='\n'));i++) {
    g_current_thread->GetProcessOwner()->stat->incrNumCharRead();
    get->P();
    c =  g_machine->console->GetChar();
    buffer[i] = c;
  }
  buffer[i] = 0;

  g_machine->console->DisableInterrupt();
  mutexget->Release();

}
