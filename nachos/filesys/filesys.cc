/*! \file filesys.cc 
//  \brief Routines to manage the overall operation of the file system.
//
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   - A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   - A number of data blocks
//	   - An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   - A bitmap of free disk sectors (cf. bitmap.h)
//	   - A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   - there is no synchronization for concurrent accesses
//	   - there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   - there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
//       Points 1 and 2 are removed in the file system assignment
//
//  Copyright (c) 1992-1993 The Regents of the University of California.
//  All rights reserved.  See copyright.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
*/


#include "kernel/system.h"
#include "kernel/msgerror.h"
#include "machine/disk.h"
#include "utility/config.h"
#include "utility/bitmap.h"
#include "filesys/directory.h"
#include "filesys/filehdr.h"
#include "filesys/filesys.h"
#include "filesys/oftable.h"

/*! Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
*/
#define FreeMapSector 		0
#define DirectorySector 	1

//----------------------------------------------------------------------
// decompname
/*! this function returns the name of the first directory 
//  in the path given in argument, as well as the remainder of the path
//  if we call decompname("/bin/new/halt",head,tail), we get
//     "bin" in head and "/new/halt" in tail
//
//  \param orig_path : the pathname we want to decompose
//  \param head : the first element in the path (for instance bin)
//  \param tail : the remainder of the string (for instance  /new/halt)
//  \note tail MAY point to the address of orig_path, in which case
//    orig_path is altered
//  \return true if everything goes ok, otherwise, return false.
//  \note head and tail MUST be large enough to contain the
//        resulting strings
*/
//----------------------------------------------------------------------
bool decompname (const char * orig_path, char * head, char * tail)
{
  // Remove the leading / if any
  while (*orig_path == '/') orig_path++;
  const char * path = orig_path;

  // Init "head" with the first element in the path
  while (*path != '\0' && *path != '/') *head++ = *path++;
  *head = '\0';
  
  // Init "tail" with the remainder
  if(*path == '\0') {
    *head = '\0';
    strcpy(tail, orig_path);
    return false;
  } else {
    strcpy(tail, path);
    return true;
  }
}

//----------------------------------------------------------------------
// FindDir
/*!
//  this function takes a complete pathname and returns
//  the disc sector of the fileheader of the directory.
//
//  IMPORTANT WARNING : name is modified : after the execution name
//  contains only the name of the file (e.g. if we called
//  FindDir("/bin/halt") then name is "halt" after the execution, the
//  function returns the sector number of the directory "/bin").
//
//  Actually, this function does not support complete pathnames
//  This is one of the objectives of the file system assignment
//
//   \param name is the complete name (relatively to the root
//     directory). The contents of this string will be modified!
//   \return the sector number of the directory, or -1 in case of an
//     error
*/
//----------------------------------------------------------------------
int FindDir(char *name)
{
  DEBUG('f', (char*)"FindDir [%s]\n", name);

  // Fetch the root directory from disk
  Directory directory(g_cfg->NumDirEntries);
  directory.FetchFrom(g_file_system->GetDirFile());

  // Start the search in the root directory
  int sector = DirectorySector;
  char dirname[g_cfg->MaxFileNameSize];
  while(decompname(name, dirname, name)) {

    // Get the sector of the file/directory corresponding to 'name'
    sector = directory.Find(dirname);
    if (sector < 0)
      return -1; // This file/directory does not exist ...

    // Check that it is a directory
    OpenFile file(sector);
    if(file.GetFileHeader()->IsDir())
      directory.FetchFrom(&file);
    else
      return -1;
  }

  DEBUG('f', (char*)"FindDir done => [%s] @%d\n", name, sector);
  return sector;
}

//----------------------------------------------------------------------
// FileSystem::FileSystem
/*! 	Initialize the file system.  If format = true, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = false, we just have to open the files
//	representing the bitmap and the directory.
//
//	\param format should we initialize the disk?
*/
//----------------------------------------------------------------------
FileSystem::FileSystem(bool format)
{ 
    DEBUG('f', (char*)"Initializing the file system.\n");
    if (format) {
        BitMap freeMap(NUM_SECTORS);
        Directory directory(g_cfg->NumDirEntries);
	FileHeader mapHdr,dirHdr;

        DEBUG('f', (char*)"Formatting the file system.\n");

	// First, allocate space for FileHeaders for the directory and bitmap
	// (make sure no one else grabs these!)
	freeMap.Mark(FreeMapSector);	    
	freeMap.Mark(DirectorySector);

	// Second, allocate space for the data blocks containing the contents
	// of the directory and bitmap files.  There better be enough space!

	ASSERT(mapHdr.Allocate(&freeMap, FreeMapFileSize));
	ASSERT(dirHdr.Allocate(&freeMap, g_cfg->DirectoryFileSize));

	// Mark the Root directory as a directory
	dirHdr.SetDir();

	// Flush the bitmap and directory FileHeaders back to disk
	// We need to do this before we can "Open" the file, since open
	// reads the file header off of disk (and currently the disk has 
	// garbage on it!).

        DEBUG('f', (char*)"Writing headers back to disk.\n");
	mapHdr.WriteBack(FreeMapSector);    
	dirHdr.WriteBack(DirectorySector);
	
	// OK to open the bitmap and directory files now
	// The file system operations assume these two files are left open
	// while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);

	// Once we have the files "open", we can write the initial version
	// of each file back to disk. The directory at this point is completely
	// empty; but the bitmap has been changed to reflect the fact that
	// sectors on the disk have been allocated for the file headers and
	// to hold the file data for the directory and bitmap.

        DEBUG('f', (char*)"Writing bitmap and directory back to disk.\n");
	freeMap.WriteBack(freeMapFile);	 // flush changes to disk
	directory.WriteBack(directoryFile);

	if (DebugIsEnabled('f')) {
	    freeMap.Print();
	    directory.Print();
	}
    } else {
      // if we are not formatting the disk, just open the files representing
      // the bitmap and directory; these are left open while Nachos is running
      freeMapFile = new OpenFile(FreeMapSector);
      directoryFile = new OpenFile(DirectorySector);
    }
}

//----------------------------------------------------------------------
// FileSystem::FileSystem
/*! 	Destructor of the file system, called when Nachos is stopped
*/
//----------------------------------------------------------------------

FileSystem::~FileSystem()
{ 
  delete freeMapFile;
  delete directoryFile;
}

//----------------------------------------------------------------------
// FileSystem::Create
/*! 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	\param name is the name of file to be created (NOT MODIFIED)
//	\param initialSize is the size of file to be created
//	\return NoError if everything goes ok, otherwise, return an error
//              code as define in msgerror.h
*/
//----------------------------------------------------------------------
int
FileSystem::Create(char *name, int initialSize)
{
    int sector,dirsector;
    char dirname[g_cfg->MaxFileNameSize];

    g_open_file_table->createLock->Acquire();
    strcpy(dirname,name);
    DEBUG('f', (char*)"Creating file %s, size %d\n", name, initialSize);
    
    dirsector=FindDir(dirname);
    if (dirsector == -1) {
      g_open_file_table->createLock->Release();
      return InexistFileError;
    }

    OpenFile dirfile(dirsector);
    Directory directory(g_cfg->NumDirEntries);
    directory.FetchFrom(&dirfile);

    if (directory.Find((char *)dirname) != -1) {
      g_open_file_table->createLock->Release();
      return AlreadyInDirectory;	// file is already in directory
    }

    // Get the freemap from the disk
    BitMap freeMap(NUM_SECTORS);
    freeMap.FetchFrom(freeMapFile);

    // Find a sector to hold the file header
    sector = freeMap.Find();	
    if (sector == -1) {
      g_open_file_table->createLock->Release();
      return OutOfDisk;		// no free block for file header 
    }

    // Add the file in the directory
    int add_result = directory.Add(dirname, sector);
    if (add_result != NoError) {
      g_open_file_table->createLock->Release();
      return add_result;	// Could not add new entry in Dir
    }

    // Indicate that this is a file, not a directory
    FileHeader hdr;
    hdr.SetFile();  

    // Allocate space for the data sectors
    if (!hdr.Allocate(&freeMap, initialSize)) {
      g_open_file_table->createLock->Release();
      return OutOfDisk;	// no space on disk for data
    }

    // everthing worked, flush all changes back to disk
    hdr.WriteBack(sector); 		// File header
    directory.WriteBack(&dirfile);      // Directory
    freeMap.WriteBack(freeMapFile);     // Freemap

    DEBUG('f', (char*)"END Creating file %s, size %d\n", name, initialSize);
    g_open_file_table->createLock->Release();
    return NoError;
}

//----------------------------------------------------------------------
// FileSystem::Open
/*! 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	\param name the text name of the file to be opened (NOT MODIFIED)
*/
//----------------------------------------------------------------------
OpenFile *
FileSystem::Open(char *name)
{ 
  OpenFile *openFile = NULL;
  int sector,dirsector;
  char dirname[g_cfg->MaxFileNameSize];

  // Find the directory containing the file
  strcpy(dirname,name);
  dirsector=FindDir(dirname);
  if (dirsector == -1) return NULL;

  // Read the directory from disk
  OpenFile dirfile(dirsector);
  DEBUG('f', (char*)"Opening file %s\n", name);
  Directory directory(g_cfg->NumDirEntries);
  directory.FetchFrom(&dirfile);

  // Find the file in the directory
  sector = directory.Find(dirname); 
  if (sector >= 0)
    { 		
    openFile = new OpenFile(sector);	// name was found in directory 
    openFile->SetName(name);
    if (openFile->IsDir())
      {
	delete openFile;
	return NULL;
      }
    }

  return openFile;	     		// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
/*! 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	\param name the text name of the file to be removed (NOT MODIFIED)
//	\return NoError if the file was deleted, else an error code
//              as defined in msgerror.h
*/
//----------------------------------------------------------------------
int
FileSystem::Remove(char *name)
{ 
  int sector = -1;
  int dirsector = -1;
  char dirname[g_cfg->MaxFileNameSize];

  strcpy(dirname,name);

  // Get the sector number of the parent directory
  dirsector = FindDir(dirname);
  
  // Check if the path is correct
  if (dirsector == -1) {
    return InexistDirectoryError;
  }

  // Fetch the directory from the disk
  OpenFile dirfile(dirsector);
  Directory directory(g_cfg->NumDirEntries);
  directory.FetchFrom(&dirfile);
  
  // Look for the file in the directory
  DEBUG('f', (char*)"looking for %s in the drectory\n",name);
  sector = directory.Find(dirname);

  // Look if we find the file in the directory
  if (sector == -1) return InexistFileError; // file not found 

  // Fetch the file header from disk
  FileHeader fileHdr;
  fileHdr.FetchFrom(sector);

  // Do nothing if it's a directory
  if (fileHdr.IsDir()) return NotAFile;

  // Get the freemap file from the disk
  BitMap freeMap(NUM_SECTORS);
  freeMap.FetchFrom(freeMapFile);

  // Indicate that sectors are deallocated in the freemap
  fileHdr.Deallocate(&freeMap);      	// remove data blocks
  freeMap.Clear(sector);	      	// remove header block

  // Remove the file from the directory
  directory.Remove(dirname);
  
  // Flush everything to disk
  freeMap.WriteBack(freeMapFile);    	// freemap
  directory.WriteBack(&dirfile);        // directory

  return NoError;
} 

//----------------------------------------------------------------------
// FileSystem::List
//! 	List all the files in the file system directory.
//----------------------------------------------------------------------
void
FileSystem::List()
{
  Directory directory(g_cfg->NumDirEntries);
  directory.FetchFrom(directoryFile);

  printf("\nNachOS File System content :\n----------------------------\n");
  directory.List((char*)"/",0);

  BitMap bitmap(NUM_SECTORS);
  bitmap.FetchFrom(freeMapFile);
  printf("Free Space : %d bytes ( %d %% )\n",bitmap.NumClear()*g_cfg->SectorSize,(int)((float)(bitmap.NumClear()*g_cfg->SectorSize)*100/(float)(NUM_SECTORS*g_cfg->SectorSize)));

}

//----------------------------------------------------------------------
// FileSystem::Print
/*! 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
*/
//----------------------------------------------------------------------
void
FileSystem::Print()
{
    
    printf("Bit map file header:\n");
    FileHeader bitHdr;
    bitHdr.FetchFrom(FreeMapSector);
    bitHdr.Print();

    printf("Directory file header:\n");
    FileHeader dirHdr;
    dirHdr.FetchFrom(DirectorySector);
    dirHdr.Print();

    BitMap freeMap(NUM_SECTORS);
    freeMap.FetchFrom(freeMapFile);
    freeMap.Print();

    Directory directory(g_cfg->NumDirEntries);
    directory.FetchFrom(directoryFile);
    directory.Print();
} 

//----------------------------------------------------------------------
// FileSystem::GetFreeMapFile()
/*!    return the free map file (used by the open file table).
//	
*/
//----------------------------------------------------------------------
OpenFile *FileSystem::GetFreeMapFile() {
  return freeMapFile;
}

//----------------------------------------------------------------------
// FileSystem::GetDirFile()
/*!    return the base directory file (used by the open file table).
//	
*/
//----------------------------------------------------------------------
OpenFile *FileSystem::GetDirFile(){
  return directoryFile;
}

//----------------------------------------------------------------------
// FileSystem::mkdir
/*! 	Create a directory in the Nachos file system (similar to UNIX create).
//	A directory is a file containing a table of directory entries.
//
//	The steps to create a dirctory are:
//	  Make sure the name is valid and does not exist
//        Create a file containing the directory entries  
// mkdir fails if:
//   		dir already exists
//	 	no free space for file header
//	 	no free entry for dir in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	\param name is the name of directory to be created (NOT MODIFIED)
//	\return NoError if everything goes ok, an error code otherwise
//              (see msgerror.h)
*/
//----------------------------------------------------------------------
int
FileSystem::Mkdir(char *dirname)
{ 
  char name[g_cfg->MaxFileNameSize];
  strcpy(name, dirname);
  DEBUG('f', (char*)"Mkdir %s\n", name);

  // Lokk for the sector number of the parent directory
  int parentsect = FindDir(name); // => modifie name
  if (parentsect < 0)
    return InexistDirectoryError;

  // Fetch it from disk
  OpenFile parentdirfile(parentsect);
  Directory parentdir(g_cfg->NumDirEntries);
  parentdir.FetchFrom(&parentdirfile);

  // Check that the directory does not exit yet
  if (parentdir.Find(name) >= 0)
    return AlreadyInDirectory; // Le sous-rep existe deja !

  // Get the freemap
  BitMap freeMap(NUM_SECTORS);
  freeMap.FetchFrom(freeMapFile);

  // Get a free sector for the file header
  int hdr_sect = freeMap.Find();
  if (hdr_sect < 0)
    return OutOfDisk; // plus de place sur le disque

  // Allocate free sectors for the directory contents
  FileHeader hdr;
  if (!hdr.Allocate(&freeMap, g_cfg->DirectoryFileSize)) 
    return OutOfDisk; // no space on disk for data

  // Add the directory in the parent directory
  int add_result = parentdir.Add(name, hdr_sect);
  if (add_result != NoError)
    return add_result;  

  /*
   * Flush everything to disk
   */

  // File header
  hdr.SetDir();
  hdr.WriteBack(hdr_sect);

  // New directory (initially empty)
  OpenFile newdirfile(hdr_sect);
  Directory newdir(g_cfg->NumDirEntries);
  newdir.WriteBack(&newdirfile);

  // Parent directory
  parentdir.WriteBack(&parentdirfile);
  freeMap.WriteBack(freeMapFile);

  return NoError;  
}

//----------------------------------------------------------------------
// FileSystem::Rmdir
/*! 	Delete a directory from the file system.  This requires:
//	 check the name is valid
//       check the directory  is empty   
//          Remove it from its directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	\param namedir the text name of the directory to be removed
//	       (NOT MODIFIED)
//	\return NoError if the directory was deleted, else an error
//              code (see msgerror.h)
*/
//----------------------------------------------------------------------
int
FileSystem::Rmdir(char *dirname)
{
  char name[g_cfg->MaxFileNameSize];
  strcpy(name,dirname);

  DEBUG('f', (char*)"Rmdir %s", name);

  // Get the sector number of the parent directory
  int parentsect = FindDir(name); // => modifie name
  if (parentsect < 0)
    return InexistDirectoryError;

  // Fetch it from disk
  OpenFile parentdirfile(parentsect);
  Directory parentdir(g_cfg->NumDirEntries);
  parentdir.FetchFrom(&parentdirfile);

  // Check that the directory to be removed exist
  int thedirsect = parentdir.Find(name);
  if (thedirsect < 0)
    return InexistDirectoryError; // directory not found 

  // Get its header
  FileHeader thedirheader;
  thedirheader.FetchFrom(thedirsect);

  // Check that is is a directory
  if (! thedirheader.IsDir())
    return NotADirectory;

  // Fetch its contents from the disk
  OpenFile thedirfile(thedirsect);
  Directory thedir(g_cfg->NumDirEntries);
  thedir.FetchFrom(& thedirfile); 

  // Check that is is empty
  if (! thedir.empty())
    return DirectoryNotEmpty; //directory is not empty

  // Get the freemap from disk
  BitMap freeMap(NUM_SECTORS);
  freeMap.FetchFrom(freeMapFile);

  // Deallocate the data sectors of the directory
  thedirheader.Deallocate(&freeMap);

  // Deallocate the sector containing the directory header
  freeMap.Clear(thedirsect);

  // We remove the directory from its parent directory
  parentdir.Remove(name);
 
  // Flush everything to disk
  freeMap.WriteBack(freeMapFile);         // freemap
  parentdir.WriteBack(&parentdirfile);    // parent directory

  return NoError;
}

