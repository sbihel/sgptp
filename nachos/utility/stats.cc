/*! \file stats.cc
//  \brief Routines for managing statistics about Nachos performance.
*/
// DO NOT CHANGE -- these stats are maintained by the machine emulation.
//
//  Copyright (c) 1992-1993 The Regents of the University of California.
//  All rights reserved.  See copyright.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
//

#include "kernel/copyright.h"
#include "kernel/system.h"
#include "utility/stats.h"

//----------------------------------------------------------------------
// Statistics::Statistics
//!     Initializes performance metrics to zero, at system startup.
//
//
//----------------------------------------------------------------------
Statistics::Statistics()
{
  allStatistics = new Listint;
  idleTicks=totalTicks=0;
}


//----------------------------------------------------------------------
// Statistics::Print
/*!     Prints performance metrics, when we've finished everything
//      at system shutdown. Thanks to AllStatistics, we are here able 
//      to print each process performances.
*/
//----------------------------------------------------------------------
void
Statistics::Print()
{
  ProcessStat *s;
  int tmp;
  Listint *list =new Listint;

  printf("\n");

  while (!(allStatistics->IsEmpty())) {
    s = ((ProcessStat *) allStatistics->SortedRemove(&tmp));
    s->Print();
    printf("\n");
    list->Append((void *)s);
  }
  
  delete allStatistics;
  allStatistics = list;
  printf("\nConcerning Nachos : \n");
  printf("   Idle time : %llu cycles on %dMz processor (%llu sec, %llu nanos)\n",
	 idleTicks,g_cfg->ProcessorFrequency,
	 cycle_to_sec(idleTicks,g_cfg->ProcessorFrequency),
	 cycle_to_nano(idleTicks,g_cfg->ProcessorFrequency));
  printf("   Total time : %llu cycles on %dMz processor (%llu sec, %llu nanos) \n",
	 totalTicks,g_cfg->ProcessorFrequency,
	 cycle_to_sec(totalTicks,g_cfg->ProcessorFrequency),
	 cycle_to_nano(totalTicks,g_cfg->ProcessorFrequency));
}

ProcessStat*
Statistics::NewProcStat(char *name)
{
  ProcessStat *procstat = new ProcessStat(name);
  allStatistics->Append((void *)procstat);
  return procstat;
}

//----------------------------------------------------------------------
// Statistics::~Statistics
//!    De-allocate all ProcessStats and the allStatistics list
//     
//----------------------------------------------------------------------
 
Statistics::~Statistics()
{
  ProcessStat *s;
  int tmp;

  while (!(allStatistics->IsEmpty()))
    {
      s = ((ProcessStat *) allStatistics->SortedRemove(&tmp));
      delete s;
    }
  delete allStatistics;
}

//----------------------------------------------------------------------
// ProcessStat::ProcessStat
/*!     Initializes performance metrics to zero, when a process startups
.        
//      \param name name of the process 
*/
//----------------------------------------------------------------------               
ProcessStat::ProcessStat(char *processName) 
{
  strcpy(name,processName);
  numInstruction=numDiskReads=numDiskWrites=0;
  numConsoleCharsRead=numConsoleCharsWritten=0;
  numMemoryAccess=numPageFaults=0;
  systemTicks = userTicks = 0;
}

//----------------------------------------------------------------------
// ProcessStat::incrSystemTicks
/*!     Increments the time spent in the operating system, at the process and system level
.        
//      \param val increment
*/
//----------------------------------------------------------------------   
void ProcessStat::incrSystemTicks(Time val) { 
  systemTicks+= val;          // Process level
  g_stats->incrTotalTicks(val); // System level
}

//----------------------------------------------------------------------
// ProcessStat::incrSystemTicks
/*!     Increments the time spent in user mode, at the process and system level
.        
//      \param val increment
*/
//----------------------------------------------------------------------   
void ProcessStat::incrUserTicks(Time val) { 
  userTicks += val;           // Process level
  g_stats->incrTotalTicks(val); // System level
}
  
//----------------------------------------------------------------------
// ProcessStat::incrMemoryAccess(void)
/*!     Updates stats concerning a memory access (process and system level)
.        
*/
//----------------------------------------------------------------------   
void ProcessStat::incrMemoryAccess(void) {

  // Process level
  numMemoryAccess++;
  userTicks += MEMORY_TICKS;

  // System level
  g_stats->incrTotalTicks(MEMORY_TICKS);
}

//----------------------------------------------------------------------
// ProcessStat::Print
/*!     Prints per-process statistics
.        
//      \param val increment
*/
//----------------------------------------------------------------------   
void ProcessStat::Print(void)
{
  printf("------------------------------------------------------------\n");
  printf("Statistics for process : %s \n", name);
  printf("   Number of instructions executed : %d\n",numInstruction); 
  printf("   System time : %llu cycles on %dMz processor (%llu sec,%llu nanos)\n",
	 systemTicks,g_cfg->ProcessorFrequency,
	 cycle_to_sec(systemTicks,g_cfg->ProcessorFrequency),
	 cycle_to_nano(systemTicks,g_cfg->ProcessorFrequency));
  printf("   User time   : %llu cycles on %dMz processor (%llu sec,%llu nanos)\n",
	 userTicks,g_cfg->ProcessorFrequency,
	 cycle_to_sec(userTicks,g_cfg->ProcessorFrequency),
	 cycle_to_nano(userTicks,g_cfg->ProcessorFrequency));
  printf("   Disk Input/Output : reads %d , writes %d \n", 
	 numDiskReads,numDiskWrites);
  printf("   Console Input Output : reads %d , writes %d \n",
	 numConsoleCharsRead, numConsoleCharsWritten);
  printf("   Memory Management : %d accesses, %d page faults\n", 
	   numMemoryAccess, numPageFaults);

    printf("------------------------------------------------------------\n");
}
      
