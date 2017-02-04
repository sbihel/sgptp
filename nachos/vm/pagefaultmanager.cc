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
    Process *proc = g_current_thread->GetProcessOwner();
    AddrSpace *addr_space = proc->addrspace;
    TranslationTable *trans_table = addr_space->translationTable;

    int disk_addr = trans_table->getAddrDisk(virtualPage);
    int page_size = g_cfg->PageSize;
    char tmp_page[page_size];
    int addr_phy;

    if (trans_table->getBitIo(virtualPage)) {
      while (trans_table->getBitIo(virtualPage)) {
        g_current_thread->Yield();
      }
      return NO_EXCEPTION;
    }

    trans_table->setBitIo(virtualPage);

    if (!trans_table->getBitSwap(virtualPage)) {
      if (disk_addr == -1) {
        memset(tmp_page, 0x0, page_size);
      } else {
        if (proc->exec_file->ReadAt(tmp_page, page_size, disk_addr) != page_size) {
          return PAGEFAULT_EXCEPTION;
        } else {
          // ok
        }
      }
    } else {
      g_swap_manager->GetPageSwap(disk_addr, tmp_page);
    }

    addr_phy = g_physical_mem_manager->AddPhysicalToVirtualMapping(addr_space, virtualPage);

    memcpy(&(g_machine->mainMemory[addr_phy * page_size]), tmp_page, page_size);

    trans_table->setPhysicalPage(virtualPage, addr_phy);
    trans_table->setBitValid(virtualPage);
    g_physical_mem_manager->UnlockPage(addr_phy);
    trans_table->clearBitIo(virtualPage);

    return NO_EXCEPTION;
#endif
}




