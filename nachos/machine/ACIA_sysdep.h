/*! \file ACIA_sysdep.h
    \brief Data structure to simulate an Asynchronous Communicating 
    Interface Adapter.
  
    The system dependent ACIA provides emissions and receptions of 
    char via a simulated serial link. Il will be implemented by using 
    sockets through the net. An emission and a reception can be 
    parallelized (full duplex operation). 
    All the accesses to the sockets are already defined in the module
    sysdep.h, so they will just have to be renamed.
  
    DO NOT CHANGE -- part of the machine emulation
  
    Copyright (c) 1999-2000 INSA de Rennes.
    All rights reserved.  
    See copyright_insa.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
*/

#ifndef _ACIA_SIM
#define _ACIA_SIM

// Forward declaration
class ACIA;

/*! \brief This class is used to simulate an Asynchronous Communicating 
    Interface Adapter on top of Unix sockets.
  
    The system dependent ACIA provides emissions and receptions of
    bytes using sockets. An emission and a reception can be done in
    parallel (full duplex).
 */
class ACIA_sysdep{
 public:
  /** Initializes a system dependent part of the ACIA.
   * \param interface: the non-system dependent part of the Acia simulation (ACIA)
   * \param machine: the MIPS machine
  */
  ACIA_sysdep(ACIA *interface, Machine *m);

  /** Deallocates it and close the socket. */
  ~ACIA_sysdep();


  /** Check if there is an incoming char. 
   * Schedule the interrupt to execute itself again in a while.
   * Check if a char had came through the socket. If there is one, 
   * input register's value and state are modified and 
   * in Interrupt mode, execute the reception handler.
   * The data reception register of the ACIA object is overwritten
   * in all the cases.
   */
  void InterruptRec(); 
    
  /**  Send a char through the socket and drain the output register.  In
   * Interrupt mode, execute the emission handler.
   */ 
  void InterruptEm();

  /** Schedules an interrupt to simulate 
   * the output register dumping.
   */ 
  void SendChar();

  /** Simulate the input register draining because it must be clear just after
   * a read operation.
   */ 
  void Drain();

 private:  
  ACIA *interface; //!< ACIA
  int sock; //!< UNIX socket number for incoming/outgoing packets.
  char sockName[32]; //!< File name corresponding to UNIX socket.
};

#endif // _ACIA_SIM
