//-------------------------------------------------------------------------
/*! \file ACIA_sysdep.cc
    \brief Routines to simulate the system dependant part of an ACIA device.
   
    Routines to simulate interrupts when the input data register is full
    and when the output data register is empty and to execute the associated 
    handler if working mode is Interrupt mode.
    If current mode is Busy Waiting mode, data and state registers are also
    modified but handlers aren't call.    

    DO NOT CHANGE -- part of the machine emulation

    Copyright (c) 1999-2000 INSA de Rennes.
    All rights reserved.  
    See copyright_insa.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
*/
//------------------------------------------------------------------------- 
#include <strings.h>
#include "machine/interrupt.h"
#include "utility/stats.h"
#include "drivers/drvACIA.h"
#include "machine/ACIA.h"
#include "machine/ACIA_sysdep.h"

//! Dummy functions because C++ is weird about pointers to member function
static void DummyInterruptRec (int64_t arg)
{
  ACIA_sysdep *ACIA_s = (ACIA_sysdep *)arg;
  ACIA_s->InterruptRec();
}

static void DummyInterruptEm (int64_t arg)
{
  ACIA_sysdep *ACIA_s = (ACIA_sysdep *)arg;
  ACIA_s->InterruptEm();
}

//------------------------------------------------------------------------
/** Initializes a system dependent part of the ACIA.
 * \param interface: the non-system dependent part of the Acia simulation (ACIA)
 * \param machine: the MIPS machine
 */
//------------------------------------------------------------------------
ACIA_sysdep::ACIA_sysdep(ACIA *iface, Machine *m)
{
  // 'interface' is a pointer to the associated ACIA object.
  interface = iface;

  // Open a socket and assign a name to it.
  sock = OpenSocket();
  AssignNameToSocket(g_cfg->TargetMachineName,sock);
  
  bcopy(g_cfg->TargetMachineName,sockName,strlen(g_cfg->TargetMachineName)+1);

  // Start checking for incoming char.
  m->interrupt->Schedule(DummyInterruptRec,(int64_t)this,
      nano_to_cycles(CHECK_TIME,g_cfg->ProcessorFrequency),ACIA_RECEIVE_INT);
};

//------------------------------------------------------------------------
/** Deallocates it and close the socket. */
//------------------------------------------------------------------------
ACIA_sysdep::~ACIA_sysdep()
{
  CloseSocket(sock);
};

//------------------------------------------------------------------------
/** Check if there is an incoming char. 
 * Schedule the interrupt to execute itself again in a while.
 * Check if a char had came through the socket. If there is one, 
 * input register's value and state are modified and 
 * in Interrupt mode, execute the reception handler.
 * The data reception register of the ACIA object is overwritten
 * in all the cases.
 */
//------------------------------------------------------------------------
void 
ACIA_sysdep::InterruptRec()
{
  int received;

  // Schedule a interrupt for next polling.
  g_machine->interrupt->Schedule(DummyInterruptRec,(int64_t)this,
     nano_to_cycles(CHECK_TIME,g_cfg->ProcessorFrequency),ACIA_RECEIVE_INT);

  // Check if a char had been threw through the socket
  // Try to read a char from the socket.
  received = ReadFromSocket(sock,&(interface->inputRegister),1);
  

  // If this operation successed... 
  if (received!=-1)
    {
      // Input register becomes FULL.
      interface->inputStateRegister = FULL;

      // In interrupt mode and reception interrups are allowed, execute the reception handler.
      if (((interface->mode) & REC_INTERRUPT) != 0)
	g_acia_driver->InterruptReceive();
    }
}; 

//------------------------------------------------------------------------
/**  Send a char through the socket and drain the output register.  In
 * Interrupt mode, execute the emission handler.
 */ 
//------------------------------------------------------------------------
void 
ACIA_sysdep::InterruptEm()
{
  // Send the char.
  SendToSocket(sock,&(interface->outputRegister),1,sockName);
  // Drain the output register.
  interface->outputRegister = 0;
  interface->outputStateRegister = EMPTY;
  
  // If send interrupts ara allowed, execute the send interrupt handler
  if (((interface->mode) & SEND_INTERRUPT) != 0)
    g_acia_driver->InterruptSend();
};		


//------------------------------------------------------------------------
/** Schedules an interrupt to simulate 
 * the output register dumping.
 */
//------------------------------------------------------------------------
void
ACIA_sysdep::SendChar()
{
  interface->outputStateRegister = FULL;
  g_machine->interrupt->Schedule(DummyInterruptEm,(int64_t)this,
    nano_to_cycles(SEND_TIME,g_cfg->ProcessorFrequency),ACIA_SEND_INT);
};


//------------------------------------------------------------------------
/** Simulate the input register draining because it must be clear just after
 * a read operation.
 */
//------------------------------------------------------------------------
void 
ACIA_sysdep::Drain()
{
  interface->inputRegister = 0;
  interface->inputStateRegister = EMPTY;
};


