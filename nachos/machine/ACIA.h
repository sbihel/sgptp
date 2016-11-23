/*! \file ACIA.h
    \brief Data Structures to emulate an ACIA device

  An ACIA (Asynchronous Communication Interface Adapter) is an asynchronous
  device (requests return immediately, and an interrupt can be raised
  later on). It offers two different modes of operation:

      1 - A Busy Waiting mode. In this mode, the ACIA does not raise any
      interrupt when a char transfer is complete. It's up to the driver
      to read the ACIA state register to detect transfer completion to be
      able to send the folloging characters.

      2- An Interrupt mode. In this mode, the ACIA raises an interrupt
      when a character has been successfully emitted/received. There are
      different interrupt levels for emission and reception.

    DO NOT CHANGE -- part of the machine emulation
  
    Copyright (c) 1999-2000 INSA de Rennes.
    All rights reserved.  
    See copyright_insa.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.  */
		
#ifndef _ACIA
#define _ACIA

// Forward declarations
class ACIA_sysdep;

//! Constants to define empty and full registers
enum RegStatus {EMPTY,FULL};

/* The ACIA working mode can be busy waiting or interrupt mode
    (there are two different interrupt levels for emission and reception) */
#define BUSY_WAITING  0    //!< Indicate that no ACIA interrupts are allowed.
#define REC_INTERRUPT 1    //!< Indicate that reception interrupts are allowed.
#define SEND_INTERRUPT  2  //!< Indicates that send interrupts are allowed.


/*! \brief Defines an ACIA (Asynchronous Communication Interface
  Adapter) device. An ACIA is an asynchronous
  device (requests return immediately, and an interrupt can be raised
  later on). It offers two different modes of operation:

      1 - A Busy Waiting mode. In this mode, the ACIA does not raise any
      interrupt when a char transfer is complete. It's up to the driver
      to read the ACIA state register to detect transfer completion to be
      able to send the folloging characters.

      2- An Interrupt mode. In this mode, the ACIA raises an interrupt
      when a character has been successfully emitted/received. There are
      different interrupt levels for emission and reception.
 */
class ACIA{
 public:
  /** Initialize the ACIA device.
  */
  ACIA(Machine *m);
  
  /** Deallocates the data structures used by the ACIA device.
   */
  ~ACIA();
    
  /**  Change the working mode for the ACIA. The parameter is a
    bitwise OR of flags defining its working mode (BUSY_WAITING,
    REC_INTERRUPT, EM_INTERRUPT).  It must be used to enable/disable
    reception and emission interrupts in the Interrupt mode.
   
    \param mod: the new mode (a bitwise OR between flags REC_INTERRUPT, EM_INTERRUPT and BUSY_WAITING)
  */
  void SetWorkingMode(int mod);
  
  /** Get the current working mode for the ACIA (BUSY_WAITING,
    REC_INTERRUPT, EM_INTERRUPT).
  */
  int GetWorkingMode(void);

  /** Get the state of the output register (used in the BUSY_WAITING
      mode). 
      \return status of output register (EMPTY/FULL)
  */
  RegStatus GetOutputStateReg();
  
  /** Get the state of the input register (used in the BUSY_WAITING
      mode). 
      \return status of input register (EMPTY/FULL)
  */
  RegStatus GetInputStateReg();
    
  /** Get the input data register value. This method does not include
   * any synchronization. When calling this method one must be sure
   * that there is a character available 
   \return Byte received
  */ 
  char GetChar();
  
  /** Write a character into the ACIA output register. This method does
   * not include any synchronization. When calling this method one must
   * be sure that the ACIA is ready to send a character.  
   \param c: The character to be sent.
  */ 
 void PutChar(char);

private:
  //! Output data register (filled-in by method PutChar)
  char outputRegister;
  //! Input data register (read by method GetChar)
  char inputRegister;
  
  //! Emission state register.
  RegStatus inputStateRegister;
  //! Reception state register.
  RegStatus outputStateRegister; 

  //! Working mode : Interrupt or Busy Waiting mode.
  int mode;  

  /*! The class ACIA_sysdep is in charge of the system dependent parts
    of the ACIA, using UDP sockets.  It will be able to write on
    the registers of an object from the class ACIA.  */
  friend class ACIA_sysdep;
  ACIA_sysdep *sysdep;
};

#endif // _ACIA
