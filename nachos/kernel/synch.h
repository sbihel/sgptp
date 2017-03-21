/*! \file synch.h 
    \brief Data structures for synchronizing threads.
  
  	Three kinds of synchronization are defined here: semaphores,
  	locks, and condition variables. Part or all of them are to be
  	implemented as part of the first assignment.
  
  	Note that all the synchronization objects take a "name" as
  	part of the initialization.  This is solely for debugging purposes.
  
   Copyright (c) 1992-1993 The Regents of the University of California.
   All rights reserved.  See copyright.h for copyright notice and limitation 
   synch.h -- synchronization primitives.  
*/

#ifndef SYNCH_H
#define SYNCH_H

#include "kernel/copyright.h"
#include "kernel/system.h"
#include "kernel/thread.h"
#include "utility/list.h"

/*! \brief Defines the "semaphore" synchronization tool
//
// The semaphore has only two operations P() and V():
//
//	P() -- decrement, then wait if the value becomes < 0
//
//	V() -- increment, waking up a thread waiting in P() if necessary
// 
// Note that the interface does *not* allow a thread to read the value of 
// the semaphore directly -- even if you did read the value, the
// only thing you would know is what the value used to be.  You don't
// know what the value is now, because by the time you get the value
// into a register, a context switch might have occurred,
// and some other thread might have called P or V, so the true value might
// now be different.
*/
class Semaphore {
public:
  //! Create and set initial value
  Semaphore(char* debugName, int initialValue);
  
  //! Delete semaphore
  ~Semaphore();  
  
  //! debugging assist
  char* getName() { return name;}
    
  void P();	 // these are the only operations on a semaphore
  void V();	 // they are both *atomic*
    
private:
  char *name;      //!< useful for debugging
  int value;       //!< semaphore value
  Listint *queue;  //!< threads waiting in P() for the value to be > 0

public:
  //! signature to make sure the semaphore is in the correct state
  ObjectTypeId typeId;
};

/*! \brief Defines the "lock" synchronization tool
//
// A lock can be BUSY or FREE.
// There are only two operations allowed on a lock: 
//
//	Acquire -- wait until the lock is FREE, then set it to BUSY
//
//	Release -- wake up a thread waiting in Acquire if necessary,
//	           or else set the lock to FREE
//
// In addition, by convention, only the thread that acquired the lock
// may release it.  As with semaphores, you can't read the lock value
// (because the value might change immediately after you read it).  
*/
class Lock {
public:
  //! Lock creation
  Lock(char* debugName);

  //! Delete a lock
  ~Lock();

  //! For debugging 
  char* getName() { return name; }
  
  //! Acquire a lock (atomic operation)
  void Acquire();

  //! Release a lock (atomic operation)
  void Release(); 
  
  //! true if the current thread holds this lock.  Useful for checking
  //! in Release, and in Condition variable operations below.
  bool isHeldByCurrentThread();	 
  
private:
  char* name;            //!< for debugging
  Listint * sleepqueue;  //!< threads waiting to acquire the lock
  bool free;             //!< to know if the lock is free
  Thread * owner;        //!< Thread who has acquired the lock

public:
  //! signature to make sure the lock is in the correct state
  ObjectTypeId typeId;
};


/*! \class Condition 
\brief Defines the "condition variable" synchronization tool
//
// A condition
// variable does not have a value, but threads may be queued, waiting
// on the variable.  These are only operations on a condition variable: 
//
//	Wait() -- relinquish the CPU until signaled, 
//
//	Signal() -- wake up a thread, if there are any waiting on 
//		the condition
//
//	Broadcast() -- wake up all threads waiting on the condition
//
*/
class Condition {
public:
  //! Create a condition and initialize it to "no one waiting"
  Condition(char* debugName);

  //! Deallocate the condition
  ~Condition();

  //! For debugging
  char* getName() { return (name); }
  
  void Wait(); 	     // Wait until the condition is signalled
  void Signal();     // Wake up one of the thread waiting on the condition 
  void Broadcast();  // Wake up all threads waiting on the condition

private:
  char* name;           //!< For debbuging
  Listint * waitqueue;  //!< Threads asked to wait

public:
  //! Signature to make sure the condition is in the correct state
  ObjectTypeId typeId;
};

#endif // SYNCH_H
