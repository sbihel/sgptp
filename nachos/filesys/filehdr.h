/*! \file filehdr.h 
    \brief Data structures for managing a disk file header.  
  
  	A file header describes where on disk to find the data in a file,
  	along with other information about the file (for instance, its
  	length, owner, etc.)
  
  
    Copyright (c) 1992-1993 The Regents of the University of California.
    All rights reserved.  See copyright.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
*/

#include "kernel/copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "machine/disk.h"
#include "utility/bitmap.h"

/*! \brief Defines the Nachos "file header" 
//
// (in UNIX terms,the "i-node"), describing where on disk to find all
// of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.
*/

class Config;
extern Config *g_cfg;

// -------------------------------------------------
// Representation of file/directory headers on disk
// -------------------------------------------------
//
// 1. First header sector
//
//   .----------------------.
//   |   isDir              | if it is a directory, 1, 0 otherwise
//   |   numBytes           | total size of the data (header excluded)
//   |   numSectors         | total number of sectors
//   |   numHeaderSectors   | number of header sectors
//   .----------------------.
//   |   List of the        | The list of the sector addresses containing data
//   |   Data sectors       | (at most DatasInFirstSector sectors)
//   |                      |
//   . ---------------------.
//   |  Next header sector  | The sector containing the remaining of the
//   |                      | list of data sectors (a "normal" header sector,
//   .----------------------. see below)
//
// 2. The other "normal" header sectors
//
//   .----------------------.
//   |  List of the         | The list of the sector addresses containing data
//   |  Data sectors (ctd.) | (at most DatasInSector sectors)
//   |                      | 
//   .----------------------.
//   |  Next header sector  | The sector containing the remaining of the
//   |                      | list of data sectors (a "normal" header sector,
//   .----------------------. see below)
//
// Be careful when modifying the format of the file header on disk
// Methods FetchFrom and WriteBack assume THIS representation

// Number of sector numbers that can be stored in the first sector
// representing a file header, which contains a header of 4 ints
// and a trailer of 1 int (5 integers in total)
#define DatasInFirstSector \
  ((int)((g_cfg->SectorSize - 5*sizeof(int)) / sizeof(int)))

// Number of sector numbers that can be put in a "normal" header sector
#define DatasInSector \
  ((int)((g_cfg->SectorSize - 1*sizeof(int)) / sizeof(int)))

// Get the value of the next header sector, given the hdrSector array
// of int representing the contents of the current header sector
#define NextHeaderSector(hdrSector) \
  (((int*)(hdrSector))[(g_cfg->SectorSize / sizeof(int)) - 1])

//! Maximum number of header sectors in a file
#define MAX_HEADER_SECTORS 32  

//! Maximum number of data sectors in a file
// (computed according to the disk representation of the file)  
#define MAX_DATA_SECTORS \
  ((int)((MAX_HEADER_SECTORS-1)*DatasInSector + DatasInFirstSector))

#define MAX_FILE_LENGTH \
  ((int)((MAX_DATA_SECTORS) * g_cfg->SectorSize))

/*! \brief Defines a file header in the Nachos file system
 */
class FileHeader {
  public:

  FileHeader(void);   // Initialize the header (made empty)
  ~FileHeader(void);  // Deallocate the file header

  bool Allocate(BitMap *bitMap, int fileSize); //!< Initialize a file header, 
					       //!< including allocating space 
					       //!< on disk for the file data

  bool reAllocate(BitMap*,int,int);            //!< add new data blocks needed
                                               //!< and new header blocks if necessary
					       
  void Deallocate(BitMap *bitMap);  	       //!< De-allocate this file's 
					       //<! data blocks
						
  void FetchFrom(int sectorNumber); 	//!< Initialize file header from disk
  void WriteBack(int sectorNumber); 	//!< Write modifications to file header
					//!< back to disk
					

  int ByteToSector(int offset);	        //!< Convert a byte offset into the file
					//!< to the disk sector containing
					//!< the byte
					
  int FileLength();		      //!< Return the length of the file 
				      //!< in bytes
  void ChangeFileLength(int);         //!< sets the length of the file
                                      //!< in bytes
  int MaxFileLength();             //!< Return the maximum length of the file
                                   //!< without reallocating data blocks
  void Print();			   //!< Print the contents of the file.
  bool IsDir();                    //!< return true if the file header is marked 
                                   //!< as a directory.
  void SetFile();                  //!< Mark this header as a file header
  void SetDir();                   //!< Mark this header as a directory header
  private:
  int isdir;
  int numBytes;			        //!< Number of bytes in the file
  int numSectors;			//!< Number of data sectors in the file
  int *dataSectors;	                /*!< Disk sector numbers for each data 
					 block in the file
					*/
  int numHeaderSectors;               //!< number of sectors used for the header
  int headerSectors[MAX_HEADER_SECTORS];  /*!< Disk sectors numbers for each header
					  block of the file
					 */
};

#endif // FILEHDR_H
