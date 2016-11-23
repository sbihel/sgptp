/*! \file fsmisc.cc 
//  \brief Miscellaneous routines for the Nachos file system
//
//	We implement:
//	   Copy -- copy a file from UNIX to Nachos
//	   Print -- cat the contents of a Nachos file 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
*/

#include "kernel/system.h"
#include "kernel/thread.h"
#include "kernel/msgerror.h"
#include "utility/utility.h"
#include "utility/stats.h"
#include "filesys/filesys.h"

#define TransferSize 	10	// make it small (10), just to be difficult

//----------------------------------------------------------------------
// Copy
// 	Copy the contents of the UNIX file "from" to the Nachos file "to"
//----------------------------------------------------------------------

void
Copy(char *from, char *to)
{
    FILE *fp;
    OpenFile* openFile=NULL;
    int amountRead, fileLength;
    // Open UNIX file
    if ((fp = fopen(from, "r")) == NULL) {	 
	printf("Copy: couldn't open Unix file %s\n", from);
	exit(-1);
	return;
    }

    // Figure out length of UNIX file
    fseek(fp, 0, 2);		
    fileLength = ftell(fp);
    fseek(fp, 0, 0);

    // Create a Nachos file of the same length
    printf("Copying Unix file %s to Nachos file %s\n",
	   from, to);    
    if (g_file_system->Create(to, fileLength) != NoError) { // Create Nachos file
	printf("Copy: couldn't create Nachos file %s\n", to);
	fclose(fp);
	exit(-1);
	return;
    }
    openFile = g_file_system->Open(to);
    ASSERT(openFile != NULL);
    
    // Copy the data in TransferSize chunks
    char buffer[TransferSize];
    while ((amountRead = fread(buffer, sizeof(char), TransferSize, fp)) > 0)
	openFile->Write(buffer, amountRead);	

    // Close the UNIX and the Nachos files
    delete openFile;
    fclose(fp);
}

//----------------------------------------------------------------------
// Print
// 	Print the contents of the Nachos file "name".
//----------------------------------------------------------------------

void
Print(char *name)
{
    OpenFile *openFile;    
    int i, amountRead;

    if ((openFile = g_file_system->Open(name)) == NULL) {
	printf("Print: unable to open Nachos file %s\n", name);
	return;
    }
    
    char buffer[TransferSize];
    while ((amountRead = openFile->Read(buffer, TransferSize)) > 0)
	for (i = 0; i < amountRead; i++)
	    printf("%1x ", buffer[i]);

    delete openFile;		// close the Nachos file
    return;
}
