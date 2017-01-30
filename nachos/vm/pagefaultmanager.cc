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
#else // #ifdef ETUDIANTS_TP
  //TODO, access to unmapped virtual address  might be due to concurrent problems

  if(g_machine->mmu->translationTable->getBitIo(virtualPage) == true) {
    printf("TODO, another page fault resolution in progress\n");  //TODO, how to wait?
    exit(-1);
    return ((ExceptionType)0);
  }
  g_machine->mmu->translationTable->setBitIo(virtualPage);  // Deny other page fault

  int pp = g_physical_mem_manager->AddPhysicalToVirtualMapping(g_current_thread->GetProcessOwner()->addrspace,
      virtualPage);

  if(g_machine->mmu->translationTable->getBitSwap(virtualPage) == 1) {
    int num_sector = g_machine->mmu->translationTable->getAddrDisk(virtualPage);
    if(num_sector == -1) {  //TODO, should we ask for a physical mapping after to avoid "deadlock"?
      printf("TODO, wait for the page to be unlocked\n");  //TODO, how to wait?
      exit(-1);
      return ((ExceptionType)0);
    }
    char temp_page[g_cfg->PageSize];
    g_swap_manager->GetPageSwap(num_sector, temp_page);

    memcpy(&(g_machine->mainMemory[pp*g_cfg->PageSize]), temp_page, g_cfg->PageSize);
    g_swap_manager->ReleasePageSwap(num_sector);
    g_machine->mmu->translationTable->clearBitSwap(virtualPage);
  } else if(g_machine->mmu->translationTable->getAddrDisk(virtualPage) == -1) { // anonymous page
    memset(&(g_machine->mainMemory[pp*g_cfg->PageSize]), 0, g_cfg->PageSize);
  } else { // in executable
    int page_position = g_machine->mmu->translationTable->getAddrDisk(virtualPage);
    g_current_thread->GetProcessOwner()->exec_file->ReadAt((char *)&(g_machine->mainMemory[pp*g_cfg->PageSize]),
        g_cfg->PageSize, page_position);
  }

  g_physical_mem_manager->UnlockPage(pp);

  g_machine->mmu->translationTable->clearBitIo(virtualPage);
  g_machine->mmu->translationTable->setBitValid(virtualPage);
#endif /* ETUDIANTS_TP */

  return ((ExceptionType)0);
}
