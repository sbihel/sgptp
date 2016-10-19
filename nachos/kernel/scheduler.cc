/*! \file scheduler.cc 
//  \brief Routines to choose the next thread to run, and to dispatch to that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.


#include "kernel/scheduler.h"
#include "kernel/system.h"
#include "kernel/thread.h"

//----------------------------------------------------------------------
//  Scheduler::Scheduler
/*! 	Constructor. Initialize the list of ready but not 
//      running threads to empty.
*/
//----------------------------------------------------------------------
Scheduler::Scheduler()
{ 
    readyList = new Listint; 
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
/*! 	Destructor. De-allocate the list of ready threads.
*/
//----------------------------------------------------------------------
Scheduler::~Scheduler()
{ 
    delete readyList; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
/*! 	Mark a thread as ready, but not necessarily running yet.
//	Put it in the ready list, for later scheduling onto the CPU.
//
//	\param thread is the thread to be put on the ready list.
*/
//----------------------------------------------------------------------
void
Scheduler::ReadyToRun (Thread *thread)
{
    DEBUG('t', (char *)"Putting thread %s in ready list.\n", thread->GetName());
    readyList->Append((void *)thread);
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
/*! 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
// \return Thread to be scheduled on the CPU
*/
//----------------------------------------------------------------------
Thread *
Scheduler::FindNextToRun ()
{
  Thread * thread=(Thread*)readyList->Remove();
  return thread;
}

//----------------------------------------------------------------------
// Scheduler::SwitchTo
/*! 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable g_current_thread becomes nextThread.
//
//	\param nextThread is the thread to be put into the CPU.
*/
//----------------------------------------------------------------------
void
Scheduler::SwitchTo (Thread *nextThread)
{
Thread *oldThread = g_current_thread;

    g_current_thread->CheckOverflow();	 // check if the old thread
				 // had an undetected stack overflow

    DEBUG('t', (char *)"Switching from thread \"%s\" to thread \"%s\" time %llu\n",
	  g_current_thread->GetName(), nextThread->GetName(),g_stats->getTotalTicks());
    
    // Modify the current thread
    g_current_thread = nextThread;

    // Save the context of old thread
    oldThread->SaveProcessorState();
    oldThread->SaveSimulatorState();

    // Do the context switch if the two threads are different
    if (oldThread!=g_current_thread) {
    	// Restore the state of the operating system from its
    	// kernelContext structure such that it goes on executing when
    	// it was last interrupted
    	nextThread->RestoreProcessorState();
	nextThread->RestoreSimulatorState();
    }

    DEBUG('t', (char *)"Now in thread \"%s\" time %llu\n", g_current_thread->GetName(),g_stats->getTotalTicks());

    // If the old thread gave up the processor because it was finishing,
    // we need to delete its carcass.  Note we cannot delete the thread
    // before now (for example, in Thread::Finish()), because up to this
    // point, we were still running on the old thread's stack!

}

//----------------------------------------------------------------------
// Scheduler::Print
/*! 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
*/
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    printf("Ready list contents: [");
    readyList->Mapcar((VoidFunctionPtr) ThreadPrint);
    printf("]\n");
}
