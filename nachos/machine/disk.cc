/*! \file disk.cc 
//  \brief Routines to simulate a physical disk device; 
//
//      Reading and writing
//	to the disk is simulated as reading and writing to a UNIX file.
//	See disk.h for details about the behavior of disks (and
//	therefore about the behavior of this simulation).
//
//	Disk operations are asynchronous, so we have to invoke an interrupt
//	handler when the simulated operation completes.
*/
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "machine/machine.h"
#include "machine/interrupt.h"
#include "machine/disk.h"
#include "utility/stats.h"
#include "utility/config.h"
#include "kernel/system.h"
#include "kernel/thread.h"

//! dummy procedure because we can't take a pointer of a member function
static void DiskDone(int64_t arg) {((Disk *)arg)->HandleInterrupt(); }

//----------------------------------------------------------------------
// Disk::Disk()
/*! 	Constructor. Initialize a simulated disk.  
//      Open the UNIX file (creating it
//	if it doesn't exist), and check the magic number to make sure it's 
// 	OK to treat it as Nachos disk storage.
//
//	\param name text name of the file simulating the Nachos disk
//	\param callWhenDone interrupt handler to be called when disk read/write
//	   request completes
//	\param callArg argument to pass the interrupt handler
*/
//----------------------------------------------------------------------

Disk::Disk(char* name, VoidNoArgFunctionPtr callWhenDone)
{
    int magicNum;
    int tmp = 0;

    DEBUG('h', (char *)"Initializing the disk, 0x%x\n", callWhenDone);
    handler = callWhenDone;
    lastSector = 0;
    bufferInit = 0;

    // Open the UNIX file used to simulate the disk
    fileno = OpenForReadWrite(name, false);
    if (fileno >= 0) {		 	// file exists, check magic number 
	Read(fileno, (char *) &magicNum, g_cfg->MagicSize);
	ASSERT(magicNum == g_cfg->MagicNumber);
    } else {				// file doesn't exist, create it
        fileno = OpenForWrite(name);
	magicNum = g_cfg->MagicNumber;  
	WriteFile(fileno, (char *) &magicNum, g_cfg->MagicSize); // write magic number

	// need to write at end of file, so that reads will not return EOF
        Lseek(fileno, g_cfg->DiskSize - sizeof(int), 0);	
	WriteFile(fileno, (char *)&tmp, sizeof(int));  
    }
    DEBUG('h', (char *)"[ctor] Clear active\n");
    active = false;
}

//----------------------------------------------------------------------
// Disk::~Disk()
/*! 	Destructor. Clean up disk simulation, by closing the UNIX file 
//      representing the disk.
*/
//----------------------------------------------------------------------

Disk::~Disk()
{
    Close(fileno);
}

//----------------------------------------------------------------------
// Disk::PrintSector()
//! 	Dump the data in a disk read/write request, for debugging only.
//   \param writing indicate if it is a write request
//   \param sector sector numer
//   \param sector contents
//----------------------------------------------------------------------

static void
PrintSector (bool writing, int sector, char *data)
{
    int *p = (int *) data;

    if (writing)
        printf("Writing sector: %d\n", sector); 
    else
        printf("Reading sector: %d\n", sector); 
    for (unsigned int i = 0; i < (g_cfg->SectorSize/sizeof(int)); i++)
	printf("%x ", p[i]);
    printf("\n"); 
}

//----------------------------------------------------------------------
// Disk::ReadRequest
/*!	Simulate a request to read a single disk sector
//	   Do the read immediately to the UNIX file
//	   Set up an interrupt handler to be called later,
//	      that will notify the caller when the simulator says
//	      the operation has completed.
//
//	Note that a disk only allows an entire sector to be read,
//	not part of a sector.
//
//	\param sectorNumber the disk sector to read
//	\param data the buffer to hold the incoming bytes
*/
//----------------------------------------------------------------------
void
Disk::ReadRequest(int sectorNumber, char* data)
{
    int ticks = ComputeLatency(sectorNumber, false);

    // Only one request at a time
    ASSERT(!active);		

    // Sanity check of the sector number
    ASSERT((sectorNumber >= 0) && (sectorNumber < NUM_SECTORS))

    DEBUG('h', (char *)"Reading from sector %d\n", sectorNumber);

    // Read in the UNIX file
    Lseek(fileno, g_cfg->SectorSize * sectorNumber + g_cfg->MagicSize, 0);
    Read(fileno, data, g_cfg->SectorSize);
    if (DebugIsEnabled('h'))
	PrintSector(false, sectorNumber, data);
    
    DEBUG('h', (char *)"[rdrq] Set active\n");
    active = true;
    UpdateLast(sectorNumber);
    
    // Update the statistics
    g_current_thread->GetProcessOwner()->stat->incrNumDiskReads();

    // Schedule the end of IO interrupt
    g_machine->interrupt->Schedule(DiskDone, (int64_t) this, ticks, DISK_INT);
}

//----------------------------------------------------------------------
// Disk::WriteRequest
/*!	Simulate a request to write a single disk sector
//	   Do the write immediately to the UNIX file
//	   Set up an interrupt handler to be called later,
//	      that will notify the caller when the simulator says
//	      the operation has completed.
//
//	Note that a disk only allows an entire sector to be written,
//	not part of a sector.
//
//	\param sectorNumber the disk sector to write
//	\param data the bytes to be written
*/
//----------------------------------------------------------------------

void
Disk::WriteRequest(int sectorNumber, char* data)
{
    int ticks = ComputeLatency(sectorNumber, true);

    // Only one request at a time
    ASSERT(!active);

    // Sanity check of the sector number
    ASSERT((sectorNumber >= 0) && (sectorNumber < NUM_SECTORS));
    
    DEBUG('h', (char *)"Writing to sector %d\n", sectorNumber);

    // Write in the UNIX file
    Lseek(fileno, g_cfg->SectorSize * sectorNumber + g_cfg->MagicSize, 0);
    WriteFile(fileno, data, g_cfg->SectorSize);
    if (DebugIsEnabled('h'))
	PrintSector(true, sectorNumber, data);
    
    DEBUG('h', (char *)"[wrrq] Set active\n");
    active = true;
    UpdateLast(sectorNumber);

    // Update statistics
    g_current_thread->GetProcessOwner()->stat->incrNumDiskWrites();

    // Schedule the end of IO interrupt
    g_machine->interrupt->Schedule(DiskDone, (int64_t) this, ticks, DISK_INT);
}

//----------------------------------------------------------------------
// Disk::HandleInterrupt()
/*! 	Called when it is time to invoke the disk interrupt handler,
//	to tell the Nachos kernel that the disk request is done.
*/
//----------------------------------------------------------------------

void
Disk::HandleInterrupt ()
{ 
    DEBUG('h', (char *)"[isr] Clear active\n");
    active = false;

    // Call the disk interrupt handler
    (*handler)();
}

//----------------------------------------------------------------------
// Disk::TimeToSeek()
/*!	Returns how long it will take to position the disk head over the correct
//	track on the disk.  Since when we finish seeking, we are likely
//	to be in the middle of a sector that is rotating past the head,
//	we also return how long until the head is at the next sector boundary.
//	
//   	Disk seeks at one track per SEEK_TIME nanos (cf. stats.h)
//   	and rotates at one sector per ROTATION_TIME nanos
*/
//----------------------------------------------------------------------

int
Disk::TimeToSeek(int newSector, int *rotation) 
{

    int newTrack = newSector / SECTORS_PER_TRACK;
    int oldTrack = lastSector / SECTORS_PER_TRACK;
    int seek = abs(newTrack - oldTrack) * 
      nano_to_cycles(SEEK_TIME,g_cfg->ProcessorFrequency);
				// how long will seek take?
    int over = (g_stats->getTotalTicks() + seek) % 
      (nano_to_cycles(ROTATION_TIME,g_cfg->ProcessorFrequency)); 
				// will we be in the middle of a sector when
				// we finish the seek?

    *rotation = 0;
    if (over > 0)	 	// if so, need to round up to next full sector
   	*rotation = nano_to_cycles(ROTATION_TIME,g_cfg->ProcessorFrequency) - over;
    return seek;
}

//----------------------------------------------------------------------
// Disk::ModuloDiff()
/*! 	Return number of sectors of rotational delay between target sector
//	"to" and current sector position "from"
*/
//----------------------------------------------------------------------

int 
Disk::ModuloDiff(int to, Time from)
{
    int toOffset = to % SECTORS_PER_TRACK;
    int fromOffset = from % SECTORS_PER_TRACK;

    return ((toOffset - fromOffset) + SECTORS_PER_TRACK) % SECTORS_PER_TRACK;
}

//----------------------------------------------------------------------
// Disk::ComputeLatency()
/*! 	Return how long will it take to read/write a disk sector, from
//	the current position of the disk head.
//
//   	Latency = seek time + rotational latency + transfer time
//   	Disk seeks at one track per SEEK_TIME nanos (cf. stats.h)
//   	and rotates at one sector per ROTATION_TIME nanos
//
//   	To find the rotational latency, we first must figure out where the 
//   	disk head will be after the seek (if any).  We then figure out
//   	how long it will take to rotate completely past newSector after 
//	that point.
//
//   	The disk also has a "track buffer"; the disk continuously reads
//   	the contents of the current disk track into the buffer.  This allows 
//   	read requests to the current track to be satisfied more quickly.
//   	The contents of the track buffer are discarded after every seek to 
//   	a new track.
*/
//----------------------------------------------------------------------

int
Disk::ComputeLatency(int newSector, bool writing)
{
    int rotation;
    int seek = TimeToSeek(newSector, &rotation);
    Time timeAfter = g_stats->getTotalTicks() + seek + rotation;
    Time rot_time = nano_to_cycles(ROTATION_TIME,g_cfg->ProcessorFrequency);

#ifndef NOTRACKBUF	// turn this on if you don't want the track buffer stuff
    // check if track buffer applies
    if ((writing == false) && (seek == 0) 
		&& ( (Time)((timeAfter - bufferInit) / rot_time) 
	     		> (Time)ModuloDiff(newSector, bufferInit / rot_time))) {
        DEBUG('h', (char *)"Request latency = %d\n",rot_time);
	return rot_time; // time to transfer sector from the track buffer
    }
#endif // NOTRACKBUF

    rotation += ModuloDiff(newSector, timeAfter / rot_time) * rot_time;

    DEBUG('h', (char *)"Request latency = %d\n", seek + rotation + rot_time);
    return(seek + rotation + rot_time);
}

//----------------------------------------------------------------------
// Disk::UpdateLast
/*!   	Keep track of the most recently requested sector.  So we can know
//	what is in the track buffer.
// \param newSector accessed sector
*/
//----------------------------------------------------------------------
void
Disk::UpdateLast(int newSector)
{
    int rotate;
    int seek = TimeToSeek(newSector, &rotate);
    
    if (seek != 0)
	bufferInit = g_stats->getTotalTicks() + seek + rotate;
    lastSector = newSector;
}
