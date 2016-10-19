/*! \file directory.cc 
//  \brief Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
//  Copyright (c) 1992-1993 The Regents of the University of California.
//  All rights reserved.  See copyright.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
*/

#include "kernel/system.h"
#include "kernel/msgerror.h"
#include "utility/utility.h"
#include "utility/config.h"
#include "filesys/filehdr.h"
#include "filesys/directory.h"


//----------------------------------------------------------------------
// Directory::Directory
/*! 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	\param size is the number of entries in the directory
*/
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
      {
	//printf("entree %i inutilisee\n",i);
	table[i].inUse = false;
      }
}

//----------------------------------------------------------------------
// Directory::~Directory
//! 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
/*! 	Read the contents of the directory from disk.
//
//	\param file is the file containing the directory contents
*/
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
/*! 	Write any modifications to the directory back to disk
//
//	\param file is the file to contain the new directory contents
*/
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
/*! 	Look up file name in directory.
//      
//      \return   its location in the table of directory entries,
//                -1 if the name isn't in the directory.
//
//	\param name the file name to look up
*/
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FILENAMEMAXLEN))
	    return i;
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
/*! 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//      \return the disk sector number where the file's header is stored 
//              or -1 if the name isn't in the directory. 
// 
//	\param name the file name to look up
*/
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
	return table[i].sector;
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
/*! 	Add a file into the directory. 
//
//	\param name the name of the file being added
//	\param newSector the disk sector containing the added file's header
//      \return NoError, AlreadyInDirectory or NoSpaceInDirectory.
*/
//----------------------------------------------------------------------

int
Directory::Add(char *name, int newSector)
{ 
    if (FindIndex(name) != -1)
	return AlreadyInDirectory;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = true;
            strncpy(table[i].name, name, FILENAMEMAXLEN); 
            table[i].sector = newSector;
        return NoError;
	}

    // no space.  Fix when we have extensible files.
    return NoSpaceInDirectory;
}

//----------------------------------------------------------------------
// Directory::Remove
/*! 	Remove a file name from the directory. 
//
//	\param name the file name to be removed
//      \return NoError, or InexistDirectoryError
*/
//----------------------------------------------------------------------
int
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	return InexistDirectoryError; // name not in directory
    table[i].inUse = false;
    return NoError;	
}

//----------------------------------------------------------------------
// Directory::List
/*! 	List all the file names in the directory.(recursive function)
 *
 *   \param name the name of the Dir to print
 *   \param depth the depth in the recursion (to print a nice
 *          hierarchy with spaces)
*/
//----------------------------------------------------------------------
void
Directory::List(char *name,int depth)
{
  Directory dir(g_cfg->NumDirEntries);

  for (int i = 0; i < tableSize; i++)
    if (table[i].inUse)
      {

	/* Print a nice Tree branch dependent on the depth in the
	 * recursion, like in:
	 * " +-- toto"
	 */
	for(int j=0;j<depth;j++)
	  {
	    if(j<depth-3)
	      printf(" ");
	    if (j==depth-3)
	      printf("+");
	    if (j>depth-3)
	      printf("-");
	  }
	printf("%s", table[i].name);

	OpenFile file(table[i].sector);
	if (file.IsDir())
	  {
	    printf("(D)\n");
	    char dirname[g_cfg->MaxFileNameSize];
	    strcpy(dirname,name);
	    strcat(dirname,table[i].name);
	    dir.FetchFrom(& file);
	    dir.List(dirname, depth+4);
	  }
	else printf("\n");
      }
}

//----------------------------------------------------------------------
//Directory::Print
/*! 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
*/
//----------------------------------------------------------------------

void
Directory::Print()
{ 
  FileHeader hdr;

  printf("Directory contents:\n");
  for (int i = 0; i < tableSize; i++)
    if (table[i].inUse) {
      printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
      hdr.FetchFrom(table[i].sector);
      hdr.Print();
    }
  printf("\n");
}

//----------------------------------------------------------------------
//Directory::empty
/*! 	Tests if a directory is empty.
//	\return true if the directory is empty.
*/
//----------------------------------------------------------------------
bool
Directory::empty()
{
  bool empti = true;
  for (int i = 0; i < tableSize; i++)
    if (table[i].inUse) 
      return false;
  return empti;
}
