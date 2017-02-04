/*! \file pagefaultmanager.cc
Routines for the page fault managerPage Fault Manager
*/
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.  
//  See copyright_insa.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
//

#include "kernel/thread.h"
#include "vm/swapManager.h"
#include "vm/physMem.h"
#include "vm/pagefaultmanager.h"

PageFaultManager::PageFaultManager() {
}

// PageFaultManager::~PageFaultManager()
/*! Nothing for now
*/
PageFaultManager::~PageFaultManager() {
}

// ExceptionType PageFault(int virtualPage)
/*! 	
//	This method is called by the Memory Management Unit when there is a 
//      page fault. This method loads the page from :
//      - read-only sections (text,rodata) $\Rightarrow$ executive
//        file
//      - read/write sections (data,...) $\Rightarrow$ executive
//        file (1st time only), or swap file
//      - anonymous mappings (stack/bss) $\Rightarrow$ new
//        page from the MemoryManager (1st time only), or swap file
//
//	\param virtualPage the virtual page subject to the page fault
//	  (supposed to be between 0 and the
//        size of the address space, and supposed to correspond to a
//        page mapped to something [code/data/bss/...])
//	\return the exception (generally the NO_EXCEPTION constant)
*/  
ExceptionType PageFaultManager::PageFault(int virtualPage) 
{
#ifndef ETUDIANTS_TP
  printf("**** Warning: page fault manager is not implemented yet\n");
  exit(-1);
  return ((ExceptionType)0);
#endif
#ifdef ETUDIANTS_TP
  TranslationTable* tt = g_machine->mmu->translationTable;
  char* buffer = new char[g_cfg->PageSize];

  if (tt->getBitSwap(virtualPage) == 0) {
    if(tt->getAddrDisk(virtualPage) == -1) { // anonymous page
      memset(buffer,0,g_cfg->PageSize);
    } else { // read from file
      g_current_thread->GetProcessOwner()->exec_file->ReadAt(buffer, g_cfg->PageSize, tt->getAddrDisk(virtualPage));
    }
  } else { // page on disk
    while(tt->getBitIo(virtualPage)) {
      IntStatus oldStatus = g_machine->interrupt->GetStatus();
      g_machine->interrupt->SetStatus(INTERRUPTS_OFF);
      g_current_thread->Sleep();
      g_machine->interrupt-> SetStatus(oldStatus);
    }
    tt->setBitIo(virtualPage);
    g_swap_manager->GetPageSwap(tt->getAddrDisk(virtualPage), buffer);
    tt->clearBitIo(virtualPage);
  }

  tt->clearBitM(virtualPage);
  tt->setBitU(virtualPage);

  long physPage = g_physical_mem_manager->AddPhysicalToVirtualMapping(g_current_thread->GetProcessOwner()->addrspace,virtualPage);

  for(int i = 0; i < g_cfg->PageSize; i++) {
    g_machine->mainMemory[physPage*g_cfg->PageSize + i] = buffer[i];
  }

  tt->setPhysicalPage(virtualPage,physPage);
  tt->setBitValid(virtualPage);

  g_physical_mem_manager->UnlockPage(physPage);

  delete buffer;

  return NO_EXCEPTION;
#endif
}
