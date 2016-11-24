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
  send_sema = new Semaphore((char*)"send_sema driver", 1);
  if ( (g_cfg->ACIA) == ACIA_BUSY_WAITING ){
    DEBUG('d', (char*)"ACIA_BUSY_WAITING mode\n");
    receive_sema = new Semaphore((char*)"receive_sema driver", 1);
    g_machine->acia->SetWorkingMode(BUSY_WAITING);
  }
  else if( (g_cfg->ACIA) == ACIA_INTERRUPT ){
    DEBUG('d', (char*)"ACIA_INTERRUPT mode\n");
    receive_sema = new Semaphore((char*)"receive_sema driver", 0);
    g_machine->acia->SetWorkingMode(SEND_INTERRUPT|REC_INTERRUPT);
    ind_rec = 0;
  }
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
  int i;
  DEBUG('d', (char*)"TtySend(%s)\n", buff);
  send_sema->P();
  i = 0;
  if(g_machine->acia->GetWorkingMode() == BUSY_WAITING){
    do{
      DEBUG('d', (char*)"[busy][send] %c\n", buff[i]);
      while(g_machine->acia->GetOutputStateReg() != EMPTY){
      }
      g_machine->acia->PutChar(buff[i]);
      i++;
    }while(buff[i-1] != '\0');
    send_sema->V();
    return i;
  }
  else if(g_machine->acia->GetWorkingMode()==(REC_INTERRUPT | SEND_INTERRUPT)){
    DEBUG('d', (char*)"[passive][send] %c\n", buff[i]);
    ind_send = 0;
    do{
      send_buffer[i] = buff[i];
      i++;
    }while(buff[i-1] != '\0' && i<BUFFER_SIZE);
    send_buffer[i-1] = '\0';
    g_machine->acia->PutChar(send_buffer[ind_send]);
    ind_send++;
    return i;
  }
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
  if(lg <= 0){
    return -1;
  }
  int i;
  receive_sema->P();
  i = 0;
  if(g_machine->acia->GetWorkingMode() == BUSY_WAITING){
    do{
      while(g_machine->acia->GetInputStateReg() == EMPTY);
      buff[i] = g_machine->acia->GetChar();
      DEBUG('d', (char*)"[busy][receive] '%c'\n", buff[i]);
      i++;
    }while( (buff[i-1] != '\0') && (i < lg) );
    buff[i-1] = '\0';
    receive_sema->V();
    return i;
  }
  else if (g_machine->acia->GetWorkingMode() == SEND_INTERRUPT){
    DEBUG('d', (char*)"[passive][receive]\n");
    do{
      buff[i] = receive_buffer[i];
      i++;
    } while(receive_buffer[i-1] != '\0' && i<lg);
    buff[i-1] = '\0';
    ind_rec = 0;
    g_machine->acia->SetWorkingMode(SEND_INTERRUPT|REC_INTERRUPT);
    return i;
  }
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
  g_machine->acia->PutChar(send_buffer[ind_send]);
  ind_send++;
  if(send_buffer[ind_send] == '\0') {
    send_sema->V();
  }
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
  receive_buffer[ind_rec] = g_machine->acia->GetChar();
  ind_rec++;
  if(receive_buffer[ind_rec] == '\0') {
    g_machine->acia->SetWorkingMode(g_machine->acia->GetWorkingMode() ^ REC_INTERRUPT);
    receive_sema->V();
  }
#else
    printf("**** Warning: receive interrupt handler not implemented yet\n");
    exit(-1);
#endif
  }
