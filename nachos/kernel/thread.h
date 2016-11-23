/*! \file thread.h
    \brief Data structures for managing threads
      
    A thread represents sequential execution of code within a program.
    So the state of a thread includes the program counter,
    the processor registers, and the execution stack.
  
    Note that because we allocate a fixed size stack for each
    thread, it is possible to overflow the stack -- for instance,
    by recursing to too deep a level.  
  
 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
 of liability and disclaimer of warranty provisions.
*/

class Thread;

#ifndef THREAD_H
#define THREAD_H

#include "machine/machine.h"
#include "kernel/copyright.h"
#include "kernel/system.h"
#include "kernel/process.h"
#include "utility/utility.h"
#include "utility/stats.h"
#include <ucontext.h> 

// Size of the simulator's execution stack
#define SIMULATORSTACKSIZE	(32 * 1024) // in Bytes

// External function, dummy routine whose sole job is to call Thread::Print.
extern void ThreadPrint(int arg);	 

class Semaphore;
class Process;

/*! \brief Defines the context of the Nachos simulator
*/
typedef struct {
ucontext_t buf;
int8_t *stackBottom;
int stackSize;
} simulatorContextT;


/*! \brief Defines the thread context (MIPS virtual machine)
*/
typedef struct {
  //! Integer CPU register state (value of
  //  MIPS registers)
  int32_t int_registers[NUM_INT_REGS];

  //! Floating point general purpose registers
  int32_t float_registers[NUM_FP_REGS];

  //! Condition code register.
  int8_t cc;
} threadContextT;


/*! \brief Data structures for managing threads  
 *  
 */
class Thread {
public:
  //! Build an empty thread
  Thread(char *debugName);
  
  //! Deallocate a Thread.
  ~Thread();			

  //! Start a thread, attaching it to a process (return NoError on success)
  int Start(Process *owner, int32_t func, int arg);

  //! Wait for another thread to finish its execution
  void Join(Thread *Idthread);

  //! Relinquish the CPU if any other thread is runnable.
  void Yield();  			
    
  //! Put the thread to sleep and relinquish the processor 
  void Sleep();  			
    
  //! Finish the execution of the thread, and prepare its deallocation
  void Finish();  				
    
  //! Check if a thread has overflowed its stack.
  void CheckOverflow();    
   
  //! Sets-up the thread simulator context : fills it with the appropriate
  //  values such that the low-level context switch executes function
  //  StartThreadExecution.
  void InitSimulatorContext(int8_t* stack_addr,
			 unsigned long int stack_size);

  //! Initialize CPU registers,
  //  before jumping to user code
  void InitThreadContext(int32_t initialPCREG,int32_t initialSP, int32_t arg);

  //! Save the processor registers.
  void SaveProcessorState();	
    
  //! Restore the processor registers.
  void RestoreProcessorState();

  //! Save the state of the Nachos simulator.
  void SaveSimulatorState();	
    
  //! Restore Nachos simulator state.
  void RestoreSimulatorState();

  char* GetName() { return (name); }
  Process* GetProcessOwner() { return process; }

protected:
  //! Thread name (for debugging)   
  char* name;

  //! Main resource container the thread is running in.
  Process *process;

  //! MIPS simulator context
  simulatorContextT simulator_context;

  //! Thread context
  threadContextT thread_context;

public:
  //! signature to make sure the thread is in the correct state
  ObjectTypeId typeId;

  int stackPointer;
};

#endif // THREAD_H
