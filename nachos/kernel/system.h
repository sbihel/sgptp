/*! \file system.h 
    \brief Global variables used in Nachos

   Copyright (c) 1992-1993 The Regents of the University of California.
   All rights reserved.  See copyright.h for copyright notice and limitation 
   of liability and disclaimer of warranty provisions.
*/


#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <map>
using namespace std;

#include "utility/list.h"
#include "utility/objid.h"

/*! Each syscall makes sure that the object that the user passes to it
 * are of the expected type, by checking the typeId field against
 * these identifiers
 */
typedef enum {
  SEMAPHORE_TYPE_ID = 0xdeefeaea,
  LOCK_TYPE_ID = 0xdeefcccc,
  CONDITION_TYPE_ID = 0xdeefcdcd,
  FILE_TYPE_ID = 0xdeadbeef,
  THREAD_TYPE_ID = 0xbadcafe,
  INVALID_TYPE_ID = 0xf0f0f0f
} ObjectTypeId;

// Forward declarations (ie in other files)
class Config;
class Statistics;
class SyscallError;
class Thread;
class Scheduler;
class PageFaultManager;
class PhysicalMemManager;
class SwapManager;
class FileSystem;
class OpenFileTable;
class DriverDisk;
class DriverConsole;
class DriverACIA;
class Machine;

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	//!< Initialization,
						//!< called before anything else
extern void Cleanup();				//!< Cleanup, called when
						//!< Nachos is done.
// Global variables per type
// By convention, all globals are in lower case and start by g_
// ------------------------------------------------------------

// Hardware components
extern Machine* g_machine;	                //!< Machine (includes CPU and peripherals)

// Thread management
extern Thread *g_current_thread;		//!< The thread holding the CPU
extern Thread *g_thread_to_be_destroyed;  	//!< The thread that just finished
extern Listint *g_alive;                        //!< List of existing threads
extern Scheduler *g_scheduler;			//!< Thread scheduler

// Device drivers
extern DriverDisk *g_disk_driver;               //!< Disk driver
extern DriverDisk *g_swap_disk_driver;          //!< Swap disk driver
extern DriverConsole *g_console_driver;         //!< Console driver
extern DriverACIA *g_acia_driver;               //!< Serial line driver

// Other Nachos components
extern FileSystem  *g_file_system;                 //!< File system
extern OpenFileTable *g_open_file_table;           //!< Open File Table
extern SwapManager *g_swap_manager;                //!< Management of swap area
extern PageFaultManager *g_page_fault_manager;     //!< Page fault handler (used in VMM)
extern PhysicalMemManager *g_physical_mem_manager;//!< Physical memory manager
extern SyscallError *g_syscall_error;              //!< Error management
extern ObjId *g_object_ids;                        //!< list of system objects (used in exception.cc to verify existence of semas, conditions, files ...
extern Config *g_cfg;                             //!< Configuration of Nachos
extern Statistics *g_stats;			  //!< performance metrics

// Endianess of data in ELF file and host endianess 
//
//Nachos supports both little and big-mips compilers, variable
//mips_endianess is set when scanning the ELF header to detect the
//endianess.
// Variable host_endianess indicates the host endianess, detected
// automatically when starting the MIPS simulator
extern char mips_endianess;
extern char host_endianess;
#define IS_BIG_ENDIAN 0
#define IS_LITTLE_ENDIAN 1

// Name of files used to emulate the Nachos disk
#define DISK_FILE_NAME (char*)"DISK"
#define DISK_SWAP_NAME (char*)"SWAPDISK"

#endif // SYSTEM_H
