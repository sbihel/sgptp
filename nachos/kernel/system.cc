/*! \file system.cc 
//  \brief Nachos initialization and cleanup routines.
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "kernel/system.h"
#include "kernel/thread.h"
#include "kernel/scheduler.h"
#include "kernel/msgerror.h"
#include "drivers/drvConsole.h"
#include "drivers/drvDisk.h"
#include "drivers/drvACIA.h"
#include "utility/config.h"
#include "utility/utility.h"
#include "utility/stats.h"
#include "vm/swapManager.h"
#include "vm/pagefaultmanager.h"
#include "vm/physMem.h"
#include "filesys/oftable.h"
#include "filesys/filesys.h"
#include "utility/objid.h"

/*!  This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.
*/


// Hardware components
Machine* g_machine;	                //!< Machine (includes CPU and peripherals)

// Thread management
Thread *g_current_thread;		//!< The thread holding the CPU
Thread *g_thread_to_be_destroyed;  	//!< The thread that just finished
Listint *g_alive;                        //!< List of existing threads
Scheduler *g_scheduler;			//!< Thread scheduler

// Device drivers
DriverDisk *g_disk_driver;               //!< Disk driver
DriverDisk *g_swap_disk_driver;          //!< Swap disk driver
DriverConsole *g_console_driver;         //!< Console driver
DriverACIA *g_acia_driver;               //!< Serial line driver

// Other Nachos components
FileSystem  *g_file_system;                 //!< File system
OpenFileTable *g_open_file_table;           //!< Open File Table
SwapManager *g_swap_manager;                //!< Management of swap area
PageFaultManager *g_page_fault_manager;     //!< Page fault handler (used in VMM)
PhysicalMemManager *g_physical_mem_manager; //!< Physical memory manager
SyscallError *g_syscall_error;              //!< Error management
ObjId *g_object_ids;                        //!< list of system objects (used in exception.cc to verify existence of semas, conditions, files ...
Config *g_cfg;                             //!< Configuration of Nachos
Statistics *g_stats;			  //!< performance metrics

// Endianess of data in ELF file and endianess of host
char mips_endianess;
char host_endianess;

//----------------------------------------------------------------------
// CleanupOK
/*!     Function called when the user presses Ctrl-C
*/
//----------------------------------------------------------------------
void
CleanupOK()
{
  Cleanup();
  Exit(0);
}

//----------------------------------------------------------------------
// TimerInterruptHandler
/*! 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	\param dummy is because every interrupt handler takes one argument,
//		whether it needs it or not.
*/
//----------------------------------------------------------------------
/*
static void
TimerInterruptHandler(int dummy)
{
    if (machine->getStatus() != IdleMode)
	machine->interrupt->YieldOnReturn();
}
*/

//----------------------------------------------------------------------
// Initialize
/*! 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	\param argc is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	\param argv is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
*/
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
  int errStatus;
  int argCount;
  char* debugArgs = (char*)"";
  char filename[MAXSTRLEN];
  bool debugUserProg = false;	//!< single step user program

  strcpy(filename,CONFIGFILENAME);

  // Scan the arguments
  for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
    argCount = 1;
    if (!strcmp(*argv, (char*)"-d")) {
      if (argc == 1)
	debugArgs = (char*)"+";	// turn on all debug flags
      else {
	debugArgs = *(argv + 1);
	argCount = 2;
      }
    }
    
    if (!strcmp(*argv, (char*)"-s"))
      debugUserProg = true;
    if (!strcmp(*argv, (char*)"-f")) {
      strcpy(filename,*(argv + 1));
    }
  }

  // Scan configuration file to set up Nachos parameters
  g_cfg = new Config(filename); 

  // Set up debug level
  DebugInit(debugArgs);			// initialize DEBUG messages

  // Create the statistics object (used from the very start)
  g_stats = new Statistics();

  // Create the Nachos hardware
  g_machine = new Machine(debugUserProg);

  // Create the device drivers
  g_disk_driver = new DriverDisk("sem disk","lock disk",g_machine->disk);
  if (g_cfg->ACIA) g_acia_driver = new DriverACIA();
  g_console_driver = new DriverConsole();

  // Create the different objects making the Nachos kernel
  g_scheduler = new Scheduler();		// Initialize the ready queue
  g_page_fault_manager = new PageFaultManager();
  g_swap_manager = new SwapManager();
  g_swap_disk_driver = g_swap_manager->GetSwapDisk();
  g_physical_mem_manager = new PhysicalMemManager();  
  g_syscall_error = new SyscallError();

  // Init the Nachos internal data structures
  g_alive = new Listint();                // List of threads (initially empty)
  g_object_ids = new ObjId();              // List of objects (initially empty)
  g_thread_to_be_destroyed = NULL;
  g_open_file_table = new OpenFileTable;

  // Cleanup if user presses Ctrl-C
  CallOnUserAbort(CleanupOK);

  // We didn't explicitly allocate the current thread we are running in.
  // But if it ever tries to give up the CPU, we better have a Thread
  // object to save its state. 
  // It's just a temporary thread
  
  // Create the process (address space + statistics) context for this temporary thread
  Process *rootProcess = new Process(NULL,&errStatus);
  if (errStatus != NoError) Exit(-1);
  
  // Create the root thread 
  g_current_thread = new Thread((char*)"main-temp");
  errStatus = g_current_thread->Start(rootProcess, 0x0, -1);
  if (errStatus != NoError) exit(-1);
  
  // Remove g_current_thread from ready list (inserted by default)
  // because it is currently executing
  ASSERT(g_current_thread == g_scheduler->FindNextToRun());
  
  // Enable interrupts
  g_machine->interrupt->SetStatus(INTERRUPTS_ON);

  // Init the Nachos file system
  // NB: uses the disk, so blocks the calling thread.
  // Thus; FileSystem initiaization has to be done after the first
  // (temporary) thread is created
  g_file_system = new FileSystem(g_cfg->FormatDisk);

}

//----------------------------------------------------------------------
// Cleanup
/*! 	Nachos is halting.  De-allocate global data structures.
*/
//----------------------------------------------------------------------
void
Cleanup()
{
  // Delete currently executing thread if any This has to be done
  // because the last running thread, even if finished, is not deleted
  // yet (deletion is done in the following context switch), for the
  // last executing thread after cleanup, there is no following
  // context switch, we have to free resources here.
  if (g_current_thread!=NULL) {
    delete g_current_thread;
  }

  // Clean all global objects
  printf("\nCleaning up...\n");    
  if (g_cfg->PrintStat) {
    g_stats->Print();
  }
  delete g_disk_driver;
  delete g_console_driver;
  if (g_cfg->ACIA) delete g_acia_driver;
  delete g_syscall_error;
  delete g_file_system;
  delete g_open_file_table;
  delete g_swap_manager;
  delete g_scheduler;
  delete g_stats;
  delete g_physical_mem_manager;
  delete g_page_fault_manager;
  delete g_cfg;
  delete g_alive;
  delete g_object_ids;
  delete g_machine;
}
