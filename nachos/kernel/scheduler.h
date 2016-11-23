//-----------------------------------------------------------------------
/*! \file scheduler.h
    \brief Defines the Nachos scheduler
  
   The class defines the scheduler/dispatcher abstraction -- 
   the data structures and operations needed to keep track of which 
   thread is running, and which threads are ready but not running.

   Copyright (c) 1992-1993 The Regents of the University of California.
   All rights reserved.  See copyright.h for copyright notice and limitation 
   of liability and disclaimer of warranty provisions.
  
*/
//-----------------------------------------------------------------------

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "kernel/copyright.h"
#include "utility/list.h"

class Thread;

class Scheduler {
public:
  
  //! Constructor. Initializes list of ready threads.  
  Scheduler();
   
  //! Destructor. De-allocates the ready list. 
  ~Scheduler();
    			
  //! Inserts a thread in the ready list
  void ReadyToRun(Thread* thread);
    
  //! Dequeue first thread of the ready list, if any, and return thread. 
  Thread* FindNextToRun();
    		
  //! Causes a context switch to nextThread
  void SwitchTo(Thread* nextThread);
    
  //! Print contents of ready list.  
  void Print();

protected:  
  //! Queue of threads that are ready to run,but not running.
  Listint *readyList;
};

#endif // SCHEDULER_H
