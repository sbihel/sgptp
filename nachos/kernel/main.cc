/*! \file  main.cc 
//  \brief Bootstrap code to initialize the operating system kernel.
//
// Usage: nachos -d <debugflags>
//		-s -x <nachos file>
//              -z -f <configfile> 
//
//    -d causes certain debugging messages to be printed (cf. utility.h)
//    -s causes user programs to be executed in single-step mode
//    -z prints the copyright message
//    -f <configfile> gives the name of a configuration file for Nachos
//    -x runs a user program
//
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "kernel/copyright.h"
#undef MAIN

#include "kernel/system.h"
#include "kernel/msgerror.h"
#include "kernel/thread.h"
#include "utility/utility.h"
#include "utility/config.h"
#include "utility/objid.h"

// External functions used by this file
extern void Copy(char *unixFile, char *nachosFile);
extern void Print(char *file);
extern void StartProcess(char *file);

//----------------------------------------------------------------------
// main
/*! 	Bootstrap the operating system kernel.  
//	
//	Check command line arguments
//	Initialize data structures
//	(optionally) Call test procedure
//
//	\param argc is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	\param argv is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
*/
//----------------------------------------------------------------------

int
main(int argc, char **argv)
{
  int argCount; // Number of arguments for a particular command
  int err;      // Error code

  // Init Nachos data structures
  Initialize(argc, argv);
  char * startfilename = g_cfg->ProgramToRun;

  // Process command line arguments
  for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
    argCount = 1;

    if (!strcmp(*argv, "-z")) {              // print copyright
      printf ("%s",copyright);
      exit(0);
    }
    
    if (!strcmp(*argv, "-h")) {              // give commande line arguments
      printf ("Usage: %s [opts]\n",argv[0]);
      printf ("   -d <debugflags> : turn on debug flags specified in <debugflags>\n");
      printf ("   -s              : single step mode\n");
      printf ("   -x <binary>     : execute MIPS binary file <binary>\n");
      printf ("   -z              : print copyright information\n");
      printf ("   -f <cfgfile>    : use <cfgfile> instead of default configuration file nachos.cfg\n");
      printf ("   -h              : list command line arguments\n");
      exit(0);
    }
    if (!strcmp(*argv, "-x")) {      	    // run a user program
      ASSERT(argc > 1);
      argCount = 2;
      startfilename = argv[1];
    }
  }
  
  if (g_cfg->Remove) {	// remove Nachos file
    g_file_system->Remove(g_cfg->FileToRemove);
  }
  if (g_cfg->MakeDir) { // Make Nachos directory
    g_file_system->Mkdir(g_cfg->DirToMake);
  }
  if (g_cfg->RemoveDir) { // Remove Nachos file
    g_file_system->Rmdir(g_cfg->DirToRemove);
  }
  if (g_cfg->NbCopy!=0) {// copy from UNIX to Nachos
    
    for(int i=0;i<g_cfg->NbCopy;i++) {
      if ((strlen(g_cfg->ToCopyUnix[i])!=0)
	  && (strlen(g_cfg->ToCopyNachos[i])!=0))
	Copy(g_cfg->ToCopyUnix[i],g_cfg->ToCopyNachos[i]);
    }
  }
  if (g_cfg->Print) {	// print a Nachos file
    Print(g_cfg->FileToPrint);
  }
  if (g_cfg->ListDir) {	// list Nachos directory
    g_file_system->List();
  }
  if (g_cfg->PrintFileSyst) {	// print entire filesystem
    g_file_system->Print();
  }
  
  if (! strcmp(startfilename,"")) {
    printf("Warning: No program to start\n");
  }
  else {
    Process * p = new Process(startfilename, & err);
    if (err != NoError) {
      fprintf(stderr, g_syscall_error->GetFormat(err), startfilename);
      exit(-1);
    }
    g_machine->mmu->translationTable = p->addrspace->translationTable;
    Thread * t = new Thread(startfilename);
    g_object_ids->AddObject(t);
    err = t->Start(p, p->addrspace->getCodeStartAddress(), -1);
  }
    
  g_current_thread->Finish();	// NOTE: if the procedure "main" 
				// returns, then the program "nachos"
				// will exit (as any other normal program
				// would).  But there may be other
				// threads on the ready list.  We switch
				// to those threads by saying that the
				// "main" thread is finished, preventing
				// it from returning.
  return(0);			// Not reached...
}











