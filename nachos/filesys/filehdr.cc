/*! \file filehdr.cc 
//  \brief Routines for managing the disk file header 
//
//	(in UNIX, this would be called the i-node).
//      The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
//
//  Copyright (c) 1992-1993 The Regents of the University of California.
//  All rights reserved.  See copyright.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
*/

#include "kernel/system.h"
#include "filesys/filehdr.h"
#include "utility/config.h"
#include "drivers/drvDisk.h"

FileHeader::FileHeader(void)
{
  dataSectors = NULL;
}

FileHeader::~FileHeader(void)
{
  if (dataSectors != NULL) {
    delete [] dataSectors;
  }
}

//----------------------------------------------------------------------
// FileHeader::Allocate
/*! 	Initialize a file header, including allocating space
//      on disk for the file data.
//	Allocate data and header blocks for the file out of the 
//      map of free disk blocks.
//
//	\param freeMap is the bitmap of free disk sectors
//	\param fileSize is the required number of bytes in the file
//	\return false if there are not enough free blocks to accomodate
//	the new file.
*/
//----------------------------------------------------------------------
bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
  int i;
  numBytes = fileSize;

  ASSERT(fileSize <= MAX_FILE_LENGTH);

  // Compute the number of sectors to store the file
  numSectors  = divRoundUp(fileSize, g_cfg->SectorSize);

  // Compute how many header sectors are required
  numHeaderSectors = divRoundUp((numSectors-DatasInFirstSector),DatasInSector);
  
  // Check if there is enough free sectors for both of them
  if (freeMap->NumClear() < numSectors + numHeaderSectors)
    return false;		// not enough space
  DEBUG('f',(char*)"Allocate:\n%d DATA sector(s)\n%d HEADER sector(s)\n",numSectors,numHeaderSectors);
  // allocate all the necessary sectors and put them in their respective table
  for (i = 0; i < numHeaderSectors; i++)
    headerSectors[i] = freeMap->Find();
  // Allocates memory for the table of data sectors
  dataSectors = new int[MAX_DATA_SECTORS];
  for (i = 0; i < numSectors; i++)
    dataSectors[i] = freeMap->Find();

  return true;
}
//----------------------------------------------------------------------
// FileHeader::reAllocate
/*! 	add new data blocks when the file grows up and allocate new header blocks
//      if necessary.
//	Allocate data and header blocks for the file out of the map of free disk blocks.
//
//	\param freeMap is the bit map of free disk sectors
//	\param oldFileSize is the actual number of bytes in the file
//      \param newFileSize is the wanted number of bytes in the file
//	\return false if there are not enough free blocks to accomodate
//	the new file.
*/
//----------------------------------------------------------------------
bool
FileHeader::reAllocate(BitMap *freeMap, int oldFileSize,int newFileSize)
{ 
  int i;

  // How many new data sectors are required
  int newnumSectors  = divRoundUp(newFileSize, g_cfg->SectorSize) - numSectors;
  numBytes=newFileSize;

  // How many new header sectors are required ?
  int newnumHeaderSectors = divRoundUp((numSectors-DatasInFirstSector),
				       DatasInSector) - numHeaderSectors;
  ASSERT(newFileSize <= MAX_FILE_LENGTH);

  // Check if there is enough free space on disk
  if (freeMap->NumClear() < (newnumSectors + newnumHeaderSectors))
    return false;		// not enough space on disk
  DEBUG('f',(char*)"Reallocate :\n%d DATA sector(s)\n%d HEADER sector(s)\n",newnumSectors,newnumHeaderSectors);

  // allocate the new sectors
  for (i = 0; i < newnumHeaderSectors; i++)
    headerSectors[i + numHeaderSectors] = freeMap->Find();
  for (i = 0; i < newnumSectors; i++)
    dataSectors[i + numSectors] = freeMap->Find();

  numSectors += newnumSectors;
  numHeaderSectors+= newnumHeaderSectors;    

  return true;
}
//----------------------------------------------------------------------
// FileHeader::Deallocate
/*! 	De-allocate all the space allocated for data blocks for this file.
//
//	\param freeMap is the bit map of free disk sectors
*/
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    int i;

    // Free the data sectors
    for (i = 0; i < numSectors; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
    }
    // Free the header sectors
    for (i = 0; i < numHeaderSectors; i++) {
	ASSERT(freeMap->Test((int) headerSectors[i]));  // ought to be marked!
	freeMap->Clear((int) headerSectors[i]);
    }
    
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
/*! 	Fetch contents of file header from disk. 
//
//	\param sector is the disk sector containing the file header
*/
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
  int SectorImg[g_cfg->SectorSize / sizeof(int)];
  int i,j;

  // Fills the temporary buffer with zeros
  memset(SectorImg, 0, g_cfg->SectorSize);

  // Read the header from the disk
  // and put it in the temporary buffer
  g_disk_driver->ReadSector(sector, (char *)SectorImg);  

  // Allocates memory for the table of data sectors
  dataSectors = new int[MAX_DATA_SECTORS];

  // Set up the memory image of the file header
  // from the newly read buffer
  isdir=SectorImg[0];
  numBytes=SectorImg[1];
  numSectors=SectorImg[2];
  numHeaderSectors=SectorImg[3];
  ASSERT(numHeaderSectors <= MAX_HEADER_SECTORS);

  // Get the first header sector
  headerSectors[0] = NextHeaderSector(SectorImg);

  // Get the number of the data sectors stored into
  // the header sector in disk
  for(i=4 ; i<DatasInSector ; i++)
    dataSectors[i-4] = SectorImg[i];

  // Get the other numbers of header sectors and data sectors
  for (i=0 ; i<numHeaderSectors ; i++)
    {
      // Fill the temporary buffer with zeroes
      memset(SectorImg, 0, g_cfg->SectorSize);
      g_disk_driver->ReadSector(headerSectors[i],(char *)SectorImg);

      for(j=0 ; j < DatasInSector ; j++)
	dataSectors[DatasInFirstSector + i*DatasInSector + j] = SectorImg[j];

      /* Make sure we don't go out of bouds */
      if (i+1 < numHeaderSectors)
	headerSectors[i+1] = NextHeaderSector(SectorImg);
    }
}



//----------------------------------------------------------------------
// FileHeader::WriteBack
/*! 	Write the modified contents of the file header back to disk. 
//
//	\param sector is the disk sector to contain the file header
*/
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
  int SectorImg[DatasInSector];
  int i,j;

  // Fills the temporary buffer with zeroes
  memset(SectorImg, 0, g_cfg->SectorSize);

  // Fills the header of the first header sector
  SectorImg[0]=isdir;
  SectorImg[1]=numBytes;
  SectorImg[2]=numSectors;
  SectorImg[3]=numHeaderSectors;

  // Fills the number of the data sectors and the first header
  // sector in the temporary buffer
  for(i=4 ; i<DatasInSector ; i++)
    SectorImg[i] = dataSectors[i-4];
  NextHeaderSector(SectorImg) = headerSectors[0];

  // Write the first header sector into disk
  g_disk_driver->WriteSector(sector, (char *)SectorImg);

  // Write the following header sectors into disk
  for(i=0 ; i<numHeaderSectors ; i++)
    {
      memset(SectorImg, 0, g_cfg->SectorSize);
      for(j=0 ; j<DatasInSector ; j++)
	SectorImg[j] = dataSectors[j + DatasInFirstSector + i*DatasInSector];
      if (i+1 < numHeaderSectors)
	NextHeaderSector(SectorImg) = headerSectors[i+1];
      else
	NextHeaderSector(SectorImg) = 0;
      g_disk_driver->WriteSector(headerSectors[i],(char *)SectorImg);
    }
}


//----------------------------------------------------------------------
// FileHeader::ByteToSector
/*!     This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	\param offset is the location within the file of the byte in question
//      \return which disk sector is storing a particular byte within the file.
*/
//----------------------------------------------------------------------
int
FileHeader::ByteToSector(int offset)
{
    return(dataSectors[offset / g_cfg->SectorSize]);
}

//----------------------------------------------------------------------
// FileHeader::FileLength
/*!  	\return the number of bytes in the file.
 */
//----------------------------------------------------------------------
int 
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::ChangeFileLength
/*!  	\param newsize the new size of the file, in bytes.
 */
//----------------------------------------------------------------------
void
FileHeader::ChangeFileLength(int newsize)
{
  numBytes = newsize;
  ASSERT(newsize <= MAX_FILE_LENGTH);
  
}
//----------------------------------------------------------------------
// FileHeader::MaxFileLength
/*!  	\return the maximum number of bytes we can put in the file without reallocating.
 */
//----------------------------------------------------------------------
int 
FileHeader::MaxFileLength()
{
    return numSectors * g_cfg->SectorSize;
}
//----------------------------------------------------------------------
// FileHeader::Print
/*! 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
*/
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char data[g_cfg->SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	g_disk_driver->ReadSector(dataSectors[i], data);
        for (j = 0; (j < g_cfg->SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
}

//----------------------------------------------------------------------
// FileHeader::Isdir
/*! 	if this header is a directory header then this
//	function return true.
//      \return true if this file is a directory
*/
//----------------------------------------------------------------------
bool 
FileHeader::IsDir()
{
  return (isdir==1);
}

//----------------------------------------------------------------------
// FileHeader::SetFile
/*! 	Mark this file as a file and not a directory.
//	
*/
//----------------------------------------------------------------------
void 
FileHeader::SetFile()
{
  isdir=0;
}

//----------------------------------------------------------------------
// FileHeader::SetDir
/*!    Mark this file as a directory.
//	
*/
//----------------------------------------------------------------------
void
FileHeader::SetDir()   
{
  isdir=1;
}
