/*! \file drvDisk.h 
    \brief Data structures to export a synchronous interface to the raw 
  	   disk device.

   Copyright (c) 1992-1993 The Regents of the University of California.
   All rights reserved.  See copyright.h for copyright notice and limitation 
   of liability and disclaimer of warranty provisions.
*/

#include "kernel/copyright.h"

#ifndef SYNCHDISK_H
#define SYNCHDISK_H

#include "machine/disk.h"
#include "kernel/synch.h"

class Semaphore;
class Lock;

/*! \brief Defines a "synchronous" disk abstraction.
//
// As with other I/O devices, the raw physical disk is an asynchronous
// device -- requests to read or write portions of the disk (sectors)
// return immediately, and an interrupt occurs later to signal that the
// operation completed.  (Also, the physical characteristics of the
// disk device assume that only one operation can be requested at a
// time).
//
// This class provides the abstraction that for any individual thread
// making a request, it waits around until the operation finishes before
// returning.
*/
class DriverDisk {
  public:
  DriverDisk(char* sem_name, char* lock_name, Disk* theDisk); 
                                        // Constructor. Initializes the disk
                                        // driver by initializing the raw Disk.
    ~DriverDisk();			// Destructor. De-allocate the driver data
    
    void ReadSector(int sectorNumber, char* data);
    					// Read/write a disk sector, returning
    					// only once the data is actually read 
					// or written.  
    void WriteSector(int sectorNumber, char* data);
    
    void RequestDone();			// Called by the disk device interrupt
					// handler, to signal that the
					// current disk operation is complete.

private:
  Semaphore *semaphore; 		/*!< To synchronize requesting thread 
					     with the interrupt handler
					*/
  Lock *lock;		  		/*!< Mutual exclusion on the disk device
					*/
  Disk *disk;                         /* The disk */
};

void DiskRequestDone();
void DiskSwapRequestDone();

#endif // SYNCHDISK_H
