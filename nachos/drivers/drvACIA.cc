/* \file drvACIA.cc
   \brief Routines of the ACIA device driver
//
//      The ACIA is an asynchronous device (requests return
//      immediately, and an interrupt happens later on).
//      This is a layer on top of the ACIA.
//      Two working modes are to be implemented in assignment 2:
//      a Busy Waiting mode and an Interrupt mode. The Busy Waiting
//      mode implements a synchronous IO whereas IOs are asynchronous
//      IOs are implemented in the Interrupt mode (see the Nachos
//      roadmap for further details).
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.
//  See copyright_insa.h for copyright notice and limitation
//  of liability and disclaimer of warranty provisions.
//
*/

/* Includes */

#include "kernel/system.h"         // for the ACIA object
#include "kernel/synch.h"
#include "machine/ACIA.h"
#include "drivers/drvACIA.h"

//-------------------------------------------------------------------------
// DriverACIA::DriverACIA()
/*! Constructor.
  Initialize the ACIA driver. In the ACIA Interrupt mode,
  initialize the reception index and semaphores and allow
  reception and emission interrupts.
  In the ACIA Busy Waiting mode, simply inittialize the ACIA
  working mode and create the semaphore.
  */
//-------------------------------------------------------------------------

DriverACIA::DriverACIA()
{
#ifdef ETUDIANTS_TP
  send_sema    = new Semaphore((char*)"send_sema driver", 0);
  receive_sema = new Semaphore((char*)"receive_sema driver", BUFFER_SIZE);
  ind_send     = 0;
  ind_rec      = 0;
  //g_machine->acia->SetWorkingMode(REC_INTERRUPT | SEND_INTERRUPT);
  g_machine->acia->SetWorkingMode(BUSY_WAITING);
#else
  printf("**** Warning: contructor of the ACIA driver not implemented yet\n");
  exit(-1);
#endif
}

//-------------------------------------------------------------------------
// DriverACIA::TtySend(char* buff)
/*! Routine to send a message through the ACIA (Busy Waiting or Interrupt mode)
  */
//-------------------------------------------------------------------------

int DriverACIA::TtySend(char* buff)
{
#ifdef ETUDIANTS_TP
  send_sema->P();

  int i = 0;
  while (1) {
    send_buffer[ind_send] = buff[i];

    if (buff[i] == '\0')
      break;

    i++;
    ind_send = (ind_send + 1) % BUFFER_SIZE;
  }

  send_sema->V();
#else
  printf("**** Warning: method Tty_Send of the ACIA driver not implemented yet\n");
  exit(-1);
  return 0;
#endif
}

//-------------------------------------------------------------------------
// DriverACIA::TtyReceive(char* buff,int length)
/*! Routine to reveive a message through the ACIA
//  (Busy Waiting and Interrupt mode).
  */
//-------------------------------------------------------------------------

int DriverACIA::TtyReceive(char* buff,int lg)
{
#ifdef ETUDIANTS_TP
  receive_sema->P();

  int i = 0;
  while (1) {
    buff[i] = receive_buffer[ind_send];

    if (i >= lg)
      break;

    i++;
    ind_rec = (ind_rec + 1) % BUFFER_SIZE;
  }

  receive_sema->V();
#
#else
  printf("**** Warning: method Tty_Receive of the ACIA driver not implemented yet\n");
  exit(-1);
  return 0;
#endif
}


//-------------------------------------------------------------------------
// DriverACIA::InterruptSend()
/*! Emission interrupt handler.
  Used in the ACIA Interrupt mode only.
  Detects when it's the end of the message (if so, releases the send_sema semaphore), else sends the next character according to index ind_send.
  */
//-------------------------------------------------------------------------

void DriverACIA::InterruptSend()
{
#ifdef ETUDIANTS_TP
#else
  printf("**** Warning: send interrupt handler not implemented yet\n");
  exit(-1);
#endif
}

//-------------------------------------------------------------------------
// DriverACIA::Interrupt_receive()
/*! Reception interrupt handler.
  Used in the ACIA Interrupt mode only. Reveices a character through the ACIA.
  Releases the receive_sema semaphore and disables reception
  interrupts when the last character of the message is received
  (character '\0').
  */
//-------------------------------------------------------------------------

void DriverACIA::InterruptReceive()
{
#ifdef ETUDIANTS_TP
#else
  printf("**** Warning: receive interrupt handler not implemented yet\n");
  exit(-1);
#endif
}
