/*! \file openfile.cc 
//  \brief Routines to manage an open Nachos file  
//
//      As in UNIX, a
//	file must be open before we can read or write to it.
//	Once we're all done, we can close it (in Nachos, by deleting
//	the OpenFile data structure).
//
//	Also as in UNIX, for convenience, we keep the file header in
//	memory while the file is open.
*/
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include <strings.h>
#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "filesys/filehdr.h"
#include "filesys/openfile.h"
#include "drivers/drvDisk.h"


//----------------------------------------------------------------------
// OpenFile::OpenFile
/*! 	Open a Nachos file for reading and writing.  Bring the file header
//	into memory while the file is open.
//
//	\param sector the location on disk of the file header for this file
*/
//----------------------------------------------------------------------
OpenFile::OpenFile(int sector)
{
  // Allocate the file header and file name
  hdr = new FileHeader;
  name = new char[g_cfg->MaxFileNameSize];
  ASSERT(hdr != 0);

  // Fetch the file header from disk
  hdr->FetchFrom(sector);

  // Set OpenFile parameters
  fSector=sector;
  seekPosition = 0; 
  typeId = FILE_TYPE_ID;
}

//----------------------------------------------------------------------
// OpenFile::~OpenFile
//! 	Close a Nachos file, de-allocating any in-memory data structures.
//----------------------------------------------------------------------
OpenFile::~OpenFile()
{
  typeId = INVALID_TYPE_ID;
  delete hdr;
  delete [] name;
}

//----------------------------------------------------------------------
// OpenFile::Seek
/*! 	Change the current location within the open file -- the point at
//	which the next Read or Write will start from.
//
//	\param position the location within the file for the next Read/Write
*/
//----------------------------------------------------------------------

void
OpenFile::Seek(int position)
{
    seekPosition = position;
}	

//----------------------------------------------------------------------
// OpenFile::Read
/*! 	Read a portion of a file, starting from seekPosition.
//	Return the number of bytes actually read, and as a
//	side effect, increment the current position within the file.
//
//	Implemented using the more primitive ReadAt.
//
//	\param into the buffer to contain the data to be read from disk 
//	\param numBytes the number of bytes to transfer
*/
//----------------------------------------------------------------------
int
OpenFile::Read(char *into, int numBytes)
{
   int result = ReadAt(into, numBytes, seekPosition);
   seekPosition += result;
   return result;
}

//----------------------------------------------------------------------
// OpenFile::Write
/*! 	Write a portion of a file, starting from seekPosition.
//	Return the number of bytes actually written, and as a
//	side effect, increment the current position within the file.
//
//	Implemented using the more primitive WriteAt.
//
//   \param into the buffer containing the data to be written to disk 
//   \param numBytes  the number of bytes to transfer
*/
//----------------------------------------------------------------------
int
OpenFile::Write(char *into, int numBytes)
{
  int result = WriteAt(into, numBytes, seekPosition);
  seekPosition += result;
  return result;
}

//----------------------------------------------------------------------
// OpenFile::ReadAt
/*! 
// 	Read a portion of a file, starting at "position".
//	Return the number of bytes actually read, but has
//	no side effects.
//
//	There is no guarantee the request starts or ends on an even disk sector
//	boundary; however the disk only knows how to read a whole disk
//	sector at a time.  
//
//	We read in all of the full or partial sectors that are part of the
//	   request, but we only copy the part we are interested in.
//
//	\param into  the buffer to contain the data to be read from disk 
//	\param numBytes the number of bytes to transfer
//	\param position the offset within the file of the first byte to be
//			read
*/
//----------------------------------------------------------------------
int
OpenFile::ReadAt(char *into, int numBytes, int position)
{
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;

    // Check if the location in the file is valid
    if ((numBytes <= 0) || (position >= fileLength))
    	return 0; 
				
    if ((position + numBytes) > fileLength)		
	numBytes = fileLength - position;
    DEBUG('f', (char*)"Reading %d bytes at %d, from file of length %d.\n", 	
			numBytes, position, fileLength);

    // Compute the list of sectors to be read
    firstSector = divRoundDown(position, g_cfg->SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, g_cfg->SectorSize);
    numSectors = 1 + lastSector - firstSector;

    // read in all the full and partial sectors that we need
    char buf [numSectors * g_cfg->SectorSize];
    for (i = firstSector; i <= lastSector; i++)	
        g_disk_driver->ReadSector(hdr->ByteToSector(i * g_cfg->SectorSize), 
					&buf[(i - firstSector) * g_cfg->SectorSize]);

    // copy the part we want
    bcopy(&buf[position - (firstSector * g_cfg->SectorSize)], into, numBytes);
    return numBytes;
}

//----------------------------------------------------------------------
// OpenFile::WriteAt
/*!
// 	Write a portion of a file, starting at "position".
//	Return the number of bytes actually written, but has
//	no side effects (except that Write modifies the file, of course).
//
//	There is no guarantee the request starts or ends on an even disk sector
//	boundary; however the disk only knows how to write a whole disk
//	sector at a time.
//
//	   We must first read in any sectors that will be partially written,
//	   so that we don't overwrite the unmodified portion.  We then copy
//	   in the data that will be modified, and write back all the full
//	   or partial sectors that are part of the request.
//
//	\param from the buffer containing the data to be written to disk 
//	\param numBytes the number of bytes to transfer
//	\param position  the offset within the file of the first byte to be
//			 written
*/
//----------------------------------------------------------------------
int
OpenFile::WriteAt(char *from, int numBytes, int position)
{
    int fileLength = hdr->FileLength();
    int maxFileLength = hdr->MaxFileLength();
    int i, firstSector, lastSector, numSectors;
    bool firstAligned, lastAligned;

    // Check the location in the file is valid
    if ((numBytes <= 0) || (position > fileLength))
      return 0;				// check request

    // Allocate new sectors if the file is not big enough
    if ((position + numBytes) > maxFileLength)
      {                                 // there isn't enough place
	// Fetch the freemap from disk
	BitMap freeMap(NUM_SECTORS);
	freeMap.FetchFrom(g_file_system->GetFreeMapFile());
	// Reallocate room for the new sectors in the file header
	if (!hdr->reAllocate(&freeMap, fileLength, position+numBytes))
	  numBytes = fileLength - position;
	else 
	  {
	    // Write back the header and freemap to disk
	    hdr->WriteBack(fSector);
	    freeMap.WriteBack(g_file_system->GetFreeMapFile());
	  }
      }
    else
      if ((position + numBytes) > fileLength)
	hdr->ChangeFileLength(position + numBytes);
	
    DEBUG('f', (char*)"Writing %d bytes at %d, to file of length %d.\n", 	
	  numBytes, position, fileLength);
    
    // Compute the list of sectors to be written
    firstSector = divRoundDown(position, g_cfg->SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, g_cfg->SectorSize);
    numSectors = 1 + lastSector - firstSector;
    
    char buf[numSectors * g_cfg->SectorSize];
    
    firstAligned = (position == (firstSector * g_cfg->SectorSize));
    lastAligned = ((position + numBytes) == ((lastSector + 1) * g_cfg->SectorSize));
    
    // read in first and last sector, if they are to be partially modified
    if (!firstAligned)
      ReadAt(buf, g_cfg->SectorSize, firstSector * g_cfg->SectorSize);	
    if (!lastAligned && ((firstSector != lastSector) || firstAligned))
      ReadAt(&buf[(lastSector - firstSector) * g_cfg->SectorSize], 
	     g_cfg->SectorSize, lastSector * g_cfg->SectorSize);	
    
    // copy in the bytes we want to change 
    bcopy(from, &buf[position - (firstSector * g_cfg->SectorSize)], numBytes);
    
    // write modified sectors back
    for (i = firstSector; i <= lastSector; i++)	
      g_disk_driver->WriteSector(hdr->ByteToSector(i * g_cfg->SectorSize), 
			     &buf[(i - firstSector) * g_cfg->SectorSize]);
    return numBytes;
}

//----------------------------------------------------------------------
// OpenFile::Length
//! 	Return the number of bytes in the file.
//----------------------------------------------------------------------
int
OpenFile::Length() 
{ 
  return hdr->FileLength(); 
}
//----------------------------------------------------------------------
// OpenFile::GetFileHeader
//! 	Return the file's FileHeader.
//----------------------------------------------------------------------
FileHeader *
OpenFile::GetFileHeader()
{
  return hdr;
}
//----------------------------------------------------------------------
// OpenFile::IsDir
//! 	Return true if the file is a directory.
//----------------------------------------------------------------------
bool
OpenFile::IsDir()
{
  return hdr->IsDir();
}
//----------------------------------------------------------------------
// OpenFile::GetName
//! 	Return the name of the file.
//----------------------------------------------------------------------
char*
OpenFile::GetName()
{
  return name;
}
//----------------------------------------------------------------------
// OpenFile::SetName
//! 	Set the name of the file.
//
//      \param the name of the file
//----------------------------------------------------------------------
void
OpenFile::SetName(char *newname)
{
  strcpy(name,newname);
}



