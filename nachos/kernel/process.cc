/*! \file  process.cc
//  \brief Routines to manage processes
*/

#include "kernel/system.h"
#include "kernel/msgerror.h"
#include "kernel/process.h"

//----------------------------------------------------------------------
// Process::Process
/*! 	Constructor. Create the environment to run a user program
//      (address space, statistics, ...)
//
//	\param executable is the file containing the object code 
//             to load into memory
//      \param err: error code 0 if OK, -1 otherwise
*/
//----------------------------------------------------------------------
Process::Process(char *filename, int *err)
{
  numThreads=0;
  *err = NoError;
  if (filename == NULL)
    {
      DEBUG('t', (char *)"Create empty process\n");

      // Create a statistics object for the program
      stat = g_stats->NewProcStat((char*)"BOOT");

      // Fake process Name
      name = new char[strlen("BOOT")+1];
      strcpy(name, "BOOT");

      // No Executable to open
      exec_file = NULL;
    }
  else
    {
      DEBUG('t', (char *)"Create named process %s\n", filename);

      // Create a statistics object for the program
      stat = g_stats->NewProcStat(filename);

      // Set process name
      name = new char[strlen(filename)+1];
      strcpy(name,filename);

      // Open executable
      exec_file = g_file_system->Open(filename);
      if (exec_file == NULL) {
	// NB : don't delete the stat object, so that statistics can
	// be displayed after the end of the process
	*err = InexistFileError;
	return;
      }
    }

  // Create the new address space associated with this file
  addrspace = new AddrSpace(exec_file, this, err);
  if (*err != NoError)
    {
      delete addrspace;
	// NB : don't delete the stat object, so that statistics can
	// be displayed after the end of the process
      delete [] name;
      return;
    }

}

//----------------------------------------------------------------------
// Process::~Process
//!   Destructor. De-alloate a process and all its components
//      (address space, ...)
//----------------------------------------------------------------------
Process::~Process()
{
  ASSERT(numThreads==0);

  // Delete the address space. Done for all processes, even the one created
  // for startup, for which there is no executable file attached
  delete addrspace;

  // Delete program name
  delete [] name;

  if (exec_file != NULL) {
    if (exec_file)
      delete exec_file;	
    // NB : don't delete the stat object, so that statistics can
    // be displayed after the end of the process
  } 
}
