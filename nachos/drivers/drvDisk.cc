/*! \file drvDisk.cc 
//  \brief Routines to synchronously access the disk
//
//      The physical disk 
//	is an asynchronous device (disk requests return immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the disk providing a synchronous interface (requests wait until
//	the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the physical disk can only
//	handle one operation at a time, use a lock to enforce mutual
//	exclusion.
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "drivers/drvDisk.h"

//----------------------------------------------------------------------
// DiskRequestDone
/*! 	Disk interrupt handler.  Need this to be a C routine, because 
//	C++ can't handle pointers to member functions.
*/
//----------------------------------------------------------------------

void DiskRequestDone()
{
    g_disk_driver->RequestDone();
}

//----------------------------------------------------------------------
// DiskSwapRequestDone
/*! 	Disk Swap interrupt handler.  Need this to be a C routine, because 
//	C++ can't handle pointers to member functions.
*/
//----------------------------------------------------------------------

void DiskSwapRequestDone()
{
    g_swap_disk_driver->RequestDone();
}

//----------------------------------------------------------------------
// DriverDisk::DriverDisk
/*! 	Constructor.
//      Initialize the disk driver, in turn
//	initializing the physical disk.
*/
//----------------------------------------------------------------------

DriverDisk::DriverDisk(char* sem_name, char *lock_name, Disk* theDisk)
{
    semaphore = new Semaphore((char*)sem_name, 0);
    lock = new Lock((char*)lock_name);
    disk = theDisk;
}

//----------------------------------------------------------------------
// DriverDisk::~DriverDisk
/*! 	Destructor.
//      De-allocate data structures needed for the disk driver.
*/
//----------------------------------------------------------------------

DriverDisk::~DriverDisk()
{
    delete lock;
    delete semaphore;
}

//----------------------------------------------------------------------
// DriverDisk::ReadSector
/*! 	Read the contents of a disk sector into a buffer. Return only
//	after the data has been read.
//
//	\param sectorNumber the disk sector to read
//	\param data the buffer to hold the contents of the disk sector
*/
//----------------------------------------------------------------------

void
DriverDisk::ReadSector(int sectorNumber, char* data)
{
    DEBUG('d', (char*)"[sdisk] rd req\n");
    lock->Acquire();			// only one disk I/O at a time
    disk->ReadRequest(sectorNumber, data);
    DEBUG('d', (char*)"[sdisk] rd req: wait irq\n");
    semaphore->P();			// wait for interrupt
    DEBUG('d', (char*)"[sdisk] rd req: wait irq OK\n");
    lock->Release();
}

//----------------------------------------------------------------------
// DriverDisk::WriteSector
/*! 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	\param sectorNumber  the disk sector to be written
//	\param data  the new contents of the disk sector
*/
//----------------------------------------------------------------------

void
DriverDisk::WriteSector(int sectorNumber, char* data)
{
    DEBUG('d', (char*)"[sdisk] wr req\n");
    lock->Acquire();			// only one disk I/O at a time
    disk->WriteRequest(sectorNumber, data);
    DEBUG('d', (char*)"[sdisk] wr req: wait irq...\n");
    semaphore->P();			// wait for interrupt
    DEBUG('d', (char*)"[sdisk] wr req: wait irq OK\n");
    lock->Release();
}

//----------------------------------------------------------------------
// DriverDisk::RequestDone
/*! 	Disk interrupt handler. Wake up any thread waiting for the disk
//	request to finish.
*/
//----------------------------------------------------------------------

void
DriverDisk::RequestDone()
{ 
  DEBUG('d', (char*)"[sdisk] req done\n");
  semaphore->V();
}
