/*! \file thread.cc 
//  \brief Routines to manage threads.  
//
//   There are four main operations:
//	- Constructor : create an inactive thread
//      - Start : bind the thread to a process, and prepare it to be
//               dispatched on the CPU
//	- Finish : called when a thread finishes, to clean up
//	- Yield : relinquish control over the CPU to another ready thread
//	- Sleep : relinquish control over the CPU, but thread is now blocked.
//		In other words, it will not run again, until explicitly 
//		put back on the ready queue.
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "kernel/thread.h"
#include "kernel/msgerror.h"
#include "kernel/synch.h"
#include "kernel/scheduler.h"

#define UNSIGNED_LONG_AT_ADDR(addr) (*((unsigned long int*)(addr)))

#define STACK_FENCEPOST 0xdeadbeef	// this is put at the end of the
					// simulator stack, for detecting 
					// stack overflows

//----------------------------------------------------------------------
// Thread::Thread
/*! 	Constructor. Initialize an empty thread (just a name)
//
//	\param threadName is an arbitrary string, used for debugging.
*/
//----------------------------------------------------------------------
Thread::Thread(char *threadName)
{
  name = new char[strlen(threadName)+1];
  strcpy(name,threadName);
  typeId = THREAD_TYPE_ID;
 
  // No process owner yet
  process = NULL;
}

//----------------------------------------------------------------------
// Thread::~Thread
/*! 	Destructor. De-allocate a thread.
//
// 	NOTE: the current thread *cannot* delete itself directly,
//	since it is still running on the stack that we need to delete.
//
//      When the last thread of a process has finished, its 
//      process can be deallocated.
*/
//----------------------------------------------------------------------

Thread::~Thread()
{
    DEBUG('t', (char *)"Deleting thread \"%s\"\n", name);
    typeId = INVALID_TYPE_ID;

    //CheckOverflow();

    // Delete the simulator stack In case this==g_current_thread, it
    // means we are currently deleting the last executing thread in
    // the system at system shutdown time. It this situation, we do not
    // free the stack since we are still using it
    if (this !=g_current_thread) 
      DeallocBoundedArray(simulator_context.stackBottom,simulator_context.stackSize);

    // NB: the thread stack itself is not freed, we do not attempt to
    // reuse the address space dedicated to stack The corresponding
    // physical memory will be deallocated when the process will be
    // deleted

    // Protect from other accesses to the process object
    IntStatus oldLevel = g_machine-> interrupt->SetStatus(INTERRUPTS_OFF);    

    // Signals to the process that we terminated
    process->numThreads--;

    // If I'm the last thread of the process, delete it
    if (process->numThreads==0) {
      delete process;
    }

    g_machine->interrupt->SetStatus(oldLevel);  

    delete [] name;
}


//----------------------------------------------------------------------
// Thread::Start
/*!  Attach a thread to a process context (essentially an address
//   space), and prepare it to be dispatched on the CPU
//
// \return NoError on success, an error code on error
*/
//----------------------------------------------------------------------
int Thread::Start(Process *owner,
		  int32_t func, int arg)
{
  ASSERT(process == NULL);
  printf("**** Warning: method Thread::Start is not implemented yet\n");
  exit(-1);
}

//----------------------------------------------------------------------
// Thread::InitThreadContext
/*!	Set the initial values for the thread contact
//
//      \param initialPCREG initial value for the PC register
//      \param initialSP initial value for the SP register
//      \param arg argument to pass to the user thread
*/
//----------------------------------------------------------------------
void
Thread::InitThreadContext(int32_t initialPCREG,int32_t initialSP, int32_t arg)
{
    int i;

    for (i = 0; i < NUM_INT_REGS; i++)
	thread_context.int_registers[i] = 0;

    // Initial program counter -- must be location of "Start"
    thread_context.int_registers[PC_REG] = initialPCREG;

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    thread_context.int_registers[NEXTPC_REG] = initialPCREG+4;

    // Arguments
    thread_context.int_registers[4] = arg;
    
    // Set the stack register 
    thread_context.int_registers[STACK_REG] = initialSP;
}

//----------------------------------------------------------------------
// StartThreadExecution, ThreadPrint
/*!	Dummy function because C++ does not allow a pointer to a member
//	function.  So in order to do this, we create a dummy C function
//	(which we can pass a pointer to), that then simply calls the 
//	member function.
*/
//----------------------------------------------------------------------
void ThreadPrint(int arg)
{ 
  Thread *t = (Thread *)arg; printf("%s", t->GetName()); 
}

void StartThreadExecution(void) {
  printf("Starting thread\n");
  g_machine->interrupt->SetStatus(INTERRUPTS_ON);
  g_machine->Run();
  // Should not return there ...
  ASSERT(0);
}

//----------------------------------------------------------------------
// Thread::InitSimulatorContext
/*! 	
//
//  Sets-up the simulator context : fills it with the appropriate
//  values such that the low-level context switch executes function
//  StartThreadExecution
// 	\param base_stack_addr is the lowest address of the kernel stack
//	
//----------------------------------------------------------------------
*/
void 
Thread::InitSimulatorContext(int8_t* base_stack_addr,
			  unsigned long int stack_size)
{
  DEBUG('t', (char *)"Init simulator context \"%s\" with stack=%p\n",
	name,  base_stack_addr);

  ASSERT(base_stack_addr != NULL);

  // Fill in buf with the current context
  // and then fill busf such that StartThreadExecution
  // will be called when a setcontext will be made on buf
  // NB: the gcc implementation of makecontext
  //     interprets ss_sp as the stack BASE and not stack BOTTOM
  //     (may not be portable to other architectures/compilers)
  ASSERT(getcontext(&(simulator_context.buf))==0);
  simulator_context.buf.uc_stack.ss_sp = base_stack_addr;
  simulator_context.buf.uc_stack.ss_size = stack_size;
  simulator_context.buf.uc_stack.ss_flags = 0;
  simulator_context.buf.uc_link = NULL;
  makecontext(&simulator_context.buf,StartThreadExecution,0); 

  // Setup kernel stack parameters for low-level context switch
  simulator_context.stackBottom = base_stack_addr;
  simulator_context.stackSize   = stack_size;

  // Mark the bottom of the stack in order to detect stack overflows
  UNSIGNED_LONG_AT_ADDR(simulator_context.stackBottom) = STACK_FENCEPOST;
} 

//----------------------------------------------------------------------
// Thread::Join
/*! 	
//      Sleep the thread until another thread finishes.
//	\param Idthread thread to wait for
//----------------------------------------------------------------------
*/
void 
Thread::Join(Thread *Idthread)
{ 
    while (g_alive->Search(Idthread)) Yield();
}
  
//----------------------------------------------------------------------
// Thread::CheckOverflow
/*! 	Check a thread's stack to see if it has overrun the space
//	that has been allocated for it.  If we had a smarter compiler,
//	we wouldn't need to worry about this, but we don't.
//
// 	NOTE: Nachos will not catch all stack overflow conditions.
//	In other words, your program may still crash because of an overflow.
//
// 	If you get bizarre results (such as seg faults where there is no code)
// 	then you *may* need to increase the stack size.  You can avoid stack
// 	overflows by not putting large data structures on the stack.
// 	Don't do this: void foo() { int bigArray[10000]; ... }
*/
//----------------------------------------------------------------------

void
Thread::CheckOverflow()
{
  ASSERT(UNSIGNED_LONG_AT_ADDR(simulator_context.stackBottom) == STACK_FENCEPOST);
}

//----------------------------------------------------------------------
// Thread::Finish
/*! 	Called by static function threadStart when a thread has finished
//      its job (see userlib/libnachos.c).
//
// 	NOTE: we don't immediately de-allocate the thread data structure 
//	or the execution stack, because we're still running in the thread 
//	and we're still on the stack!  Instead, we set "g_thread_to_be_destroyed", 
//	so that Scheduler::SwitchTo() will call the destructor, once we're
//	running in the context of a different thread.
//
// 	NOTE: we disable interrupts, so that we don't get a time slice 
//	between setting g_thread_to_be_destroyed and going to sleep.
*/
//----------------------------------------------------------------------
void
Thread::Finish ()
{

    DEBUG('t', (char *)"Finishing thread \"%s\"\n", GetName());
 
    
  printf("**** Warning: method Thread::Finish is not fully implemented yet\n");

  // Go to sleep
  Sleep();  // invokes SWITCH

 }

//----------------------------------------------------------------------
// Thread::Yield
/*! 	Relinquish the CPU if any other thread is ready to run.
//	If so, put the thread on the end of the ready list, so that
//	it will eventually be re-scheduled.
//
//	NOTE: returns immediately if no other thread on the ready queue.
//	Otherwise returns when the thread eventually works its way
//	to the front of the ready list and gets re-scheduled.
//
//	NOTE: we disable interrupts, so that looking at the thread
//	on the front of the ready list, and switching to it, can be done
//	atomically.  On return, we re-set the interrupt level to its
//	original state, in case we are called with interrupts disabled. 
*/
//----------------------------------------------------------------------
void
Thread::Yield ()
{
    Thread *nextThread;
    IntStatus oldLevel = g_machine->interrupt->SetStatus(INTERRUPTS_OFF);
    
    ASSERT(this == g_current_thread);
    
    DEBUG('t', (char *)"Yielding thread \"%s\"\n", GetName());
    
    nextThread = g_scheduler->FindNextToRun();
    if (nextThread != NULL) {
	g_scheduler->ReadyToRun(this);
	g_scheduler->SwitchTo(nextThread);
    }
    (void) g_machine->interrupt->SetStatus(oldLevel);
}

//----------------------------------------------------------------------
// Thread::Sleep
/*! 	Relinquish the CPU, because the current thread is blocked
//	waiting on a synchronization variable (Semaphore, Lock, or Condition).
//	Eventually, some thread will wake this thread up, and put it
//	back on the ready queue, so that it can be re-scheduled.
//
//	NOTE: if there are no threads on the ready queue, that means
//	we have no thread to run.  "Interrupt::Idle" is called
//	to signify that we should idle the CPU until the next I/O interrupt
//	occurs (the only thing that could cause a thread to become
//	ready to run).
//
//	NOTE: we assume interrupts are already disabled, because it
//	is called from the synchronization routines which must
//	disable interrupts for atomicity.   We need interrupts off 
//	so that there can't be a time slice between pulling the first thread
//	off the ready list, and switching to it.
*/
//----------------------------------------------------------------------
void
Thread::Sleep ()
{
    Thread *nextThread;
    
    ASSERT(this == g_current_thread);
    ASSERT(g_machine->interrupt->GetStatus() == INTERRUPTS_OFF);
    
    DEBUG('t', (char *)"Sleeping thread \"%s\"\n", GetName());

    // In case, there is nobody else to execute, we wait for an
    // interrupt In case there is no interrupt to come in the future,
    // Nachos exists
    //
    // Note that during this phase, variable g_current_thread is still
    // set to the thread which is put to sleep, which is weird and
    // would need to be fixed
    while ((nextThread = g_scheduler->FindNextToRun()) == NULL) {
	DEBUG('t', (char *)"Nobody to run => idle\n");
	g_machine->interrupt->Idle();	// no one to run, wait for an interrupt
    }

    // Once we have another thread to execute, perform the context switch
    g_scheduler->SwitchTo(nextThread);
}

//----------------------------------------------------------------------
// Thread::SaveProcessorState
/*!	Save the CPU state of a user program on a context switch
*/
//----------------------------------------------------------------------
void
Thread::SaveProcessorState()
{
  printf("**** Warning: method Thread::SaveProcessorState is not implemented yet\n");
  exit(-1);
}

//----------------------------------------------------------------------
// Thread::RestoreProcessorState
/*!	Restore the CPU state of a user program on a context switch.
*/
//----------------------------------------------------------------------

void
Thread::RestoreProcessorState()
{
  printf("**** Warning: method Thread::RestoreProcessorState is not implemented yet\n");
  exit(-1);
}

//----------------------------------------------------------------------
// Thread::SaveSimulatorState
/*!	Save the simulator state.
*/
//----------------------------------------------------------------------
void
Thread::SaveSimulatorState()
{
  getcontext(&(simulator_context.buf));
}

//----------------------------------------------------------------------
// Thread::RestoreSimulatorState
/*!	Restore simulator state.
*/
//----------------------------------------------------------------------
void
Thread::RestoreSimulatorState()
{    	
  setcontext(&(simulator_context.buf)); 
}
