/*! \file  machine.h 
    \brief Data structures to simulate the MIPS machine
  
 User programs are loaded into "mainMemory"; to Nachos,
 this looks just like an array of bytes.  Of course, the Nachos
 kernel is in memory too -- but as in most machines these days,
 the kernel is loaded into a separate memory region from user
 programs, and accesses to kernel memory are not translated or paged.
  
 In Nachos, user programs are executed one instruction at a time, 
 by the MIPS simulator.  Each memory reference is translated, checked
 for errors, etc.

 DO NOT CHANGE -- part of the machine emulation
  
 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
 of liability and disclaimer of warranty provisions.
*/

#ifndef MACHINE_H
#define MACHINE_H

// Integer sizes (such that MIPS integers are 32-bit values)
#include <stdint.h>

#include "machine/disk.h"
#include "kernel/copyright.h"
#include "utility/stats.h"

// Possible exceptions recognized by the machine
enum ExceptionType { NO_EXCEPTION,           //!< Everything ok!
		     SYSCALL_EXCEPTION,      //!< A program executed a system call.
		     PAGEFAULT_EXCEPTION,    //!< Page fault exception
		     READONLY_EXCEPTION,     /*!< Write attempted to
					         page marked "read-only" */
		     BUSERROR_EXCEPTION,     /*!< Translation resulted
					      in an invalid physical
					      address (mis-aligned or
					      out-of-bounds) */
		     ADDRESSERROR_EXCEPTION, /*!< Reference that was
					     not mapped in the address
					     space */
		     OVERFLOW_EXCEPTION,     //!< Integer overflow in add or sub.
		     ILLEGALINSTR_EXCEPTION, //!< Unimplemented or reserved instr.
		     
		     NUM_EXCEPTION_TYPES
};

#include "machine/translationtable.h"
#include "machine/mmu.h"
#include "machine/ACIA.h"
#include "machine/interrupt.h"
class Console;

/*! Nachos can be running kernel code (SYSTEM_MODE), user code (USER_MODE),
 or there can be no runnable thread, because the ready list 
 is empty (IDLE_MODE).
*/
enum MachineStatus {IDLE_MODE, SYSTEM_MODE, USER_MODE};

// User program CPU state.  The full set of MIPS registers, plus a few
// more because we need to be able to start/stop a user program between
// any two instructions (thus we need to keep track of things like load
// delay slots, etc.)

#define STACK_REG	29	//!< User's stack pointer
#define RETADDR_REG	31	//!< Holds return address for procedure calls
#define NUM_GP_REGS	32	//!< 32 general purpose registers on MIPS
#define HI_REG		32	//!< Double register to hold multiply result
#define LO_REG		33
#define PC_REG		34	//!< Current program counter
#define NEXTPC_REG	35	//!< Next program counter (for branch delay) 
#define PREVPC_REG	36	//!< Previous program counter (for debugging)
#define LOAD_REG	37	//!< The register target of a delayed load.
#define LOADVALUE_REG 	38	//!< The value to be loaded by a delayed load.
#define BADVADDR_REG	39	//!< The failing virtual address on an exception

#define NUM_INT_REGS 	40      //!< Number of integer registers
#define NUM_FP_REGS     32      //!< Number of floating point registers

/*! \brief  Defines an instruction
//
//  Represented in both
// 	undecoded binary form
//      decoded to identify
//	    - operation to do
//	    - registers to act on
//	    - any immediate operand value
*/
class Instruction {
  public:
    void Decode();	//!< Decode the binary representation of the instruction

    uint32_t value; //!< Binary representation of the instruction

    int opCode;     /*!< Type of instruction.  This is NOT the same as the
		       opcode field from the instruction: see defs in mips.h
		     */
    int8_t rs, rt, rd; //!< Three registers from instruction.
    int8_t fs, ft, fd; //!< The same thing, but for FP operations
    int32_t extra;       /*!< Immediate or target or shamt field or offset.
		       Immediates are sign-extended.
		     */
};

/*! \brief Defines the simulated execution hardware
// 
// User programs shouldn't be able to tell that they are running on our 
// simulator or on the real hardware, except 
//	- we only partially support floating point instructions (only
//	  "ordered operations", no FP "likely branches", no fixed point
//	  words
//	- the system call interface to Nachos is not the same as UNIX 
//	  (10 system calls in Nachos vs. 200 in UNIX!)
// If we were to implement more of the UNIX system calls, we ought to be
// able to run Nachos on top of Nachos!
//
// The procedures in this class are defined in machine.cc, mipssim.cc, and
// translate.cc.
*/

class Machine {
  public:
  Machine(bool debug);	//!<  Constructor. Initialize the MIPS machine
			//!<  for running user programs
  ~Machine();		//!<  Destructor. De-allocate the data structures

// Routines callable by the Nachos kernel
  void Run();	 		//!< Run a user program

  int32_t ReadIntRegister(int num);	//!< Read the contents of an Integer CPU register

  void WriteIntRegister(int num, int32_t value);
				//!< Store a value into an Integer CPU register

  int32_t ReadFPRegister(int num); //!< Read the contents of a floating point register

  void WriteFPRegister(int num, int32_t value);
				//!< store a value into a floating point register

  int8_t ReadCC (void);           //!< Read floating point code condition register
  void WriteCC (int8_t cc);       //!< Write floating point code condition register
  
  MachineStatus GetStatus() { return status; } //!< idle, kernel, user
  void SetStatus(MachineStatus st) { status = st; }

// Routines internal to the machine simulation -- DO NOT call these 

    int OneInstruction(Instruction *instr); 	
    				//!< Run one instruction of a user program.
                                //!< Return the execution time of the instr (cycle)
    void DelayedLoad(int nextReg, int nextVal);  	
				//!< Do a pending delayed load (modifying a reg)

    void RaiseException(ExceptionType which, int badVAddr);
				//!< Trap to the Nachos kernel, because of a
				//!< system call or other exception.  

    void Debugger();		//!< Invoke the user program debugger
    void DumpState();		//!< Print the user CPU and memory state 


  // Data structures -- all of these are accessible to Nachos kernel code.
  // "public" for convenience.
  //
  // Note that *all* communication between the user program and the kernel 
  // are in terms of these data structures.
  
  int32_t int_registers[NUM_INT_REGS]; //!< CPU Integer registers, for executing user programs
  
  int32_t float_registers[NUM_FP_REGS]; //!< Floating point general purpose registers

  int8_t cc;                     /*!< Condition code. Note that
				 since only MIPS I FP instrs are implemented */

  int8_t *mainMemory;		/*!< Physical memory to store user program,
				  code and data, while executing
				*/

  MMU *mmu;                     /*!< Machine memory management unit */
  ACIA *acia;                   /*!< ACIA Hardware */
  Interrupt *interrupt;         /*!< Interrupt management */
  Disk *disk;		  	/*!< Raw disk device (hardware) */
  Disk *diskSwap;		/*!< Swap raw disk device (hardware) */
  Console *console;             /*!< Console */

private:
  MachineStatus status;	//!< idle, kernel mode, user mode

  bool singleStep;		/*!< Drop back into the debugger after each
				  simulated instruction
				*/
  Time runUntilTime;		/*!< Drop back into the debugger when simulated
				  time reaches this value
				*/
};

//! Entry point into Nachos to handle user system calls and exceptions.
// Defined in exception.cc
extern void ExceptionHandler(ExceptionType which, int vaddr);


// Routines for converting Words and Short Words to and from the
// simulated machine's format of little endian.  If the host machine
// is little endian (DEC and Intel), these end up being NOPs.
//
// What is stored in each format:
//	- host byte ordering:
//	   - kernel data structures
//	   - user registers
//	- simulated machine byte ordering:
//	   - contents of main memory
uint32_t WordToHost(uint32_t word);
uint16_t ShortToHost(uint16_t shortword);
uint32_t WordToMachine(uint32_t word);
uint16_t ShortToMachine(uint16_t shortword);

#endif // MACHINE_H
