/*! \file stats.h 
    \brief Data structures for gathering statistics about
	Nachos performance.

 DO NOT CHANGE -- these stats are maintained by the machine emulation
  
 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
   of liability and disclaimer of warranty provisions.
*/

#ifndef STATS_H
#define STATS_H

#include "utility/utility.h"
#include "utility/list.h"
#include "utility/config.h"

/*! \brief Defines Nachos statistics that are kept at run-time

   Contains all information that don't concern only one process
*/

class ProcessStat;

class Statistics {
 private:
  Listint *allStatistics;      //!< enables to keep  statistics of all processes when they are finished.
  Time totalTicks;	   //!< Total time spent running Nachos
  Time idleTicks;           //!< Time spent idle (no thread to run)
                          
 public:
  Statistics();            // initialyses everything to zero
  ~Statistics();           // de-allocate the list
  ProcessStat *NewProcStat(char *name); /* create a new ProcessStat, link it to allStatistics 
			        and return a pointer on it. It is called by the 
			        method which create a new process */        

  void Print();            /* prints collected statistics, including 
                               process statistics
                           */
  void incrTotalTicks (Time val) {totalTicks+=val;}
  void setTotalTicks(Time val) {totalTicks=val;}
  Time getTotalTicks(void) {return totalTicks;}
  void incrIdleTicks (Time val) {idleTicks +=val;}
};



/*! \brief Defines statistics that concern a particular process
//
// Each thread fom a same process will modify the same ProcessState 
//
// The fields in this class are public to make it easier to update.
*/

class ProcessStat {
private:
  char name[MAXSTRLEN];          //!< name of the process
  Time systemTicks;              //!< Time spent executing system code
  Time userTicks;                //!< Time spent executing user code including memory accesses
  int numInstruction;
  int numDiskReads;             //!< number of disk read requests
  int numDiskWrites;            //!< number of disk write requests
  int numConsoleCharsRead;      //!< number of characters read from the keyboard
  int numConsoleCharsWritten;   //!< number of characters written to the display
  
  int numMemoryAccess;          //!< number of Memory accesses
  int numPageFaults;            //!< number of virtual memory page faults
public:
  ProcessStat(char *name);      /* initialises everything to zero and 
                                     initialises the name of the process */
  void incrSystemTicks(Time val);
  void incrUserTicks(Time val);
  Time getUserTime(void) {return userTicks;}
  Time getSystemTime(void) {return systemTicks;}
  void incrMemoryAccess(void);
  void incrPageFault(void) {numPageFaults++;}
  void incrNumCharWritten(void) {numConsoleCharsWritten++;}
  void incrNumCharRead(void) {numConsoleCharsRead++;}
  void incrNumDiskReads(void) {numDiskReads++;}
  void incrNumDiskWrites(void) {numDiskWrites++;}
  void incrNumInstruction(void) {numInstruction++;}
  int getNumInstruction(void) {return numInstruction;}
  void Print(void);
};

// Constants used to reflect the relative time an operation would
// take in a real system, expressed in processor cycles
#define USER_TICK       1   //!< average number of cycles for instruction
#define SYSTEM_TICK     1   //!< average number of cycles for system call
#define MEMORY_TICKS   10   //!< cycles the cpu takes to access a memory location

// Speed of the peripherals (expressed in nanoseconds)
// The speeds of the peripherals are not linked to those of the CPU
#define ROTATION_TIME  1000  //!< time disk takes to rotate one sector
#define SEEK_TIME      1000  //!< time disk takes to seek past one track
#define CONSOLE_TIME   1000  //!< time to read or write one character
#define CHECK_TIME     1000  //!< time between two checks of the reception register 
#define SEND_TIME      1000  //!< time to send a char via the ACIA object
#define TIMER_TIME    10000 //!< interval between time interrupts 

#endif // STATS_H


