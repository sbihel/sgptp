/*! \file pagefaultmanager.h
   \brief Data structures for the page fault manager
  
    Copyright (c) 1999-2000 INSA de Rennes.
    All rights reserved.  
    See copyright_insa.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
*/


#ifndef PFM_H
#define PFM_H

#include "machine/machine.h"

/*! \brief Defines the page fault manager
   This object manages the page fault of the simulated MIPS processor 
   for the Nachos kernel.
*/
class PageFaultManager {

public:
   PageFaultManager();

  ~PageFaultManager();
 
  ExceptionType PageFault(int virtualPage); //!< Page faut handler
};

#endif // PFM_H
