/*! \file  utility.h 
    \brief Miscellaneous useful definitions, including debugging routines.
  
  	The debugging routines allow the user to turn on selected
  	debugging messages, controllable from the command line arguments
  	passed to Nachos (-d).  You are encouraged to add your own
  	debugging flags.  The pre-defined debugging flags are:
  
  	'+' -- turn on all debug messages
     	't' -- thread system
     	's' -- semaphores, locks, and conditions
     	'i' -- interrupt emulation
     	'm' -- machine emulation
     	'd' -- disk emulation
     	'f' -- file system
     	'a' -- address spaces
        'x' -- virtual memory

 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
 of liability and disclaimer of warranty provisions.
*/

#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>
#include <inttypes.h>
#include "kernel/copyright.h"

// Miscellaneous useful routines

// Time values (expressed in processor cycles)
typedef uint64_t Time;

// Macro to convert nanosecs into number of processor cycles
#define nano_to_cycles(nano,frequency) ((nano*frequency)/1000)
#define cycle_to_sec(cycle,frequency) ((cycle/frequency) / 1000000)
#define cycle_to_nano(cycle,frequency) ( (1000*cycle/frequency) % 1000000000)

// Divide and either round up or down 
#define divRoundDown(n,s)  ((n) / (s))
#define divRoundUp(n,s)    (((n) / (s)) + ((((n) % (s)) > 0) ? 1 : 0))

// This declares the type "VoidFunctionPtr" to be a "pointer to a
// function taking a void* argument and returning nothing".  With
// such a function pointer (say it is "func"), we can call it like this:
// This is used by Thread::Fork and for interrupt handlers, as well
// as a couple of other places.

typedef void (*VoidFunctionPtr)(int64_t arg); 
typedef void (*VoidNoArgFunctionPtr)(); 

// Include interface that isolates us from the host machine system library.
// Requires definition of bool, and VoidFunctionPtr
#include "machine/sysdep.h"				

// Interface to debugging routines.

extern void DebugInit(char* flags);	// enable printing debug messages

extern bool DebugIsEnabled(char flag); 	// Is this debug flag enabled?

extern void DEBUG (char flag, char* format, ...);  	// Print debug message 
							// if flag is enabled

extern void DumpMem(char *addr, int len); // Prints a mem area in hex format

//----------------------------------------------------------------------
// ASSERT
/*!     If condition is false,  print a message and dump core.
//	Useful for documenting assumptions in the code.
//
//	NOTE: needs to be a #define, to be able to print the location 
//	where the error occurred.
*/
//----------------------------------------------------------------------
#define ASSERT(condition)                                                     \
    if (!(condition)) {                                                       \
        fprintf(stderr, "Assertion failed: line %d, file \"%s\"\n",           \
                __LINE__, __FILE__);                                          \
	fflush(stderr);							      \
        Abort();                                                              \
    }

#endif // UTILITY_H
