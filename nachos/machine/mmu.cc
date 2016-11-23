/*! \file mmu.cc 
//  \brief Routines to translate virtual addresses to physical addresses
//
//	Software sets up a table of legal translations.  We look up
//	in the table on every memory reference to find the true physical
//	memory location.
//
// One single types of translation are supported here, a linear page table.
// The virtual page # is used as an index
// into the table, to find the physical page #.
//
// NB: one could also use a TLB in replacement or addition of
// the linear page table (not integrated in the current version
// of the source code).
//
*/
// DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "machine/machine.h"
#include "kernel/system.h"
#include "kernel/addrspace.h"
#include "vm/physMem.h"
#include "vm/pagefaultmanager.h"

//----------------------------------------------------------------------
// MMU::MMU()
/*! Construction. Empty for now  
*/
//----------------------------------------------------------------------
MMU::MMU() {
  translationTable = NULL;
}

//----------------------------------------------------------------------
// MMU::~MMU()
/*! Destructor. Empty for now
*/
//----------------------------------------------------------------------
MMU::~MMU() {
  translationTable = NULL;
}

//----------------------------------------------------------------------
// MMU::ReadMem
/*!     Read "size" (1, 2, 4) bytes of virtual memory at "addr" into 
//	the location pointed to by "value".
//
//
//	\param addr the virtual address to read from
//	\param size the number of bytes to read (1, 2, 4)
//	\param value the place to write the result
//      \return Returns false if the translation step from 
//              virtual to physical memory failed, true otherwise.
*/
//----------------------------------------------------------------------
bool
MMU::ReadMem(int virtAddr, int size, int *value, bool is_instruction)
{
  int data;
  ExceptionType exc;
  int physAddr;
  int physAddrEnd;
  
    DEBUG('h', (char *)"Reading VA 0x%x, size %d\n", virtAddr, size);

    // Update statistics
    g_current_thread->GetProcessOwner()->stat->incrMemoryAccess();

    // Perform address translation
    exc = Translate(virtAddr, &physAddr, size, false);
    Translate(virtAddr, &physAddrEnd, size, false);
    if (exc==NO_EXCEPTION) ASSERT(physAddr==physAddrEnd);

    // Raise an exception if one has been detected during address translation
    if (exc != NO_EXCEPTION) {
	g_machine->RaiseException(exc, virtAddr);
	return false;
    }
    
    // Read data from main memory
    switch (size) {
      case 1:
	data = g_machine->mainMemory[physAddr];
	*value = data;
	break;
	
      case 2:
	data = *(unsigned short *) &g_machine->mainMemory[physAddr];
	*value = ShortToHost(data);
	break;
	
      case 4:
	data = *(unsigned int *) &g_machine->mainMemory[physAddr];
	*value = WordToHost(data);
	break;

    default: ASSERT(false);
    }

    DEBUG('h', (char *)"\tValue read = %8.8x\n", *value);

    return (true);
}

//----------------------------------------------------------------------
// MMU::WriteMem
/*!      Write "size" (1, 2, 4) bytes of the contents of "value" into
//	virtual memory at location "addr".
//
//   	Returns false if the translation step from virtual to physical memory
//   	failed.
//
//	\param addr the virtual address to write to
//	\param size the number of bytes to be written (1, 2, or 4)
//	\param value the data to be written
*/
//----------------------------------------------------------------------
bool
MMU::WriteMem(int addr, int size, int value)
{
    ExceptionType exc;
    int physicalAddress;
    int physAddrEnd;
     
    DEBUG('h', (char *)"Writing VA 0x%x, size %d, value 0x%x\n", addr, size, value);

    // Update statistics
    g_current_thread->GetProcessOwner()->stat->incrMemoryAccess();

    // Perform address translation
    exc = Translate(addr, &physicalAddress, size, true);
    Translate(addr, &physAddrEnd, size, true);
    if (exc==NO_EXCEPTION) ASSERT(physicalAddress==physAddrEnd);

    if (exc != NO_EXCEPTION) {
	g_machine->RaiseException(exc, addr);
	return false;
    }

    // Write into the machine main memory
    switch (size) {
      case 1:
	g_machine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
	break;

      case 2:
	*(unsigned short *) &g_machine->mainMemory[physicalAddress]
		= ShortToMachine((unsigned short) (value & 0xffff));
	break;
      
      case 4:
	*(unsigned int *) &g_machine->mainMemory[physicalAddress]
		= WordToMachine((unsigned int) value);
	break;
      default: ASSERT(false);
    }
    DEBUG('h', (char *)"\tValue written");

    return true;
}

//----------------------------------------------------------------------
// MMU::Translate(int virtAddr, int* physAddr, int size, bool writing)
/*! 	Translate a virtual address into a physical address, using 
//      a linear page table.
//         - Look for a translation of the virtual page in the page table
//             - if found, check access rights and physical address
//                correctness, returns the physical page
//         - else Look for a translation of the virtual page in the
//           translation pages
//             - make sure the entry corresponds to a correct entry,
//               ie it maps something (phys mem or disk) <=> at least one
//               of the readAllowed or writeAllowed bits is set ?
//             - check access rights
//	       - If bit valid=true : physical page already known
//	       - Else if bit valid=false : raise a page fault exception
//             - returns the physical page
//
//      If everything is ok, set the use/dirty bits in
//	the translation table entry, and store the translated physical 
//	address in "physAddr".  If there was an error, returns the type
//	of the exception.
//
//	\param virtAddr the virtual address to translate
//	\param physAddr pointer to the place to store the physical address
*/
ExceptionType 
MMU::Translate(int virtAddr, int* physAddr, int size, bool writing)
{
  DEBUG('h', (char *)"\tTranslate 0x%x, %s: ",
	virtAddr, writing ? "write" : "read");
  
  // check for alignment errors
  /*
  if (((size == 4) && (virtAddr & 0x3))
      || ((size == 2) && (virtAddr & 0x1))){
    DEBUG('h', (char *)"alignment problem at %d, size %d!\n", virtAddr, size);
    return BusErrorException;
  }
  */

  // Compute virtual page number and offset in the page
  int vpn = virtAddr / g_cfg->PageSize;
  int offset = virtAddr % g_cfg->PageSize;

  /*
   * Complete the addres translation
   */

  // check the virtual page number
  if (vpn >= translationTable->getMaxNumPages()) {
    DEBUG('h', (char *)"virtual page # %d too large for page table size %d!\n",
	  vpn, translationTable->getMaxNumPages());
    return ADDRESSERROR_EXCEPTION;
  }

  // is the page correctly mapped ?
  if (!translationTable->getBitReadAllowed(vpn) && !translationTable->getBitWriteAllowed(vpn)) {
    DEBUG('h', (char *)"virtual page # %d not mapped !\n", vpn);
    return ADDRESSERROR_EXCEPTION;
  }

  // Check access rights
  if (writing && !translationTable->getBitWriteAllowed(vpn)) {
    DEBUG('h', (char *)"write access on read-only virtual page # %d !\n",
	  vpn);
    return READONLY_EXCEPTION;
  }

  // If the page is not yet in main memory, run the page fault manager
  if (!translationTable->getBitValid(vpn)) {
    // Update statistics
    g_current_thread->GetProcessOwner()->stat->incrPageFault();
    DEBUG('h', (char *)"Raising page fault exception for page number %i\n",
	  vpn);

    // call the page fault manager
    g_machine->RaiseException(PAGEFAULT_EXCEPTION, virtAddr);

    if (!translationTable->getBitValid(vpn)) {
      printf("Error: page fault failed (bit valid should be set to 1)\n");
      exit(-1);
    }
  }

  // Make sure physical address is correct
  if ((translationTable->getPhysicalPage(vpn) < 0)
      || (translationTable->getPhysicalPage(vpn) >= g_cfg->NumPhysPages))
    {
      DEBUG('h', (char *)"MMU: Translated physical page out of bounds (0x%x)\n",
	    translationTable->getPhysicalPage(vpn));
      return BUSERROR_EXCEPTION;
    }
  
  // Set the U/M bits
  if (writing) {
    translationTable->setBitM(vpn);
  } 
  translationTable->setBitU(vpn);
  g_current_thread->GetProcessOwner()->stat->incrMemoryAccess();

  *physAddr = translationTable->getPhysicalPage(vpn) * g_cfg->PageSize + offset;
  DEBUG('h', (char *)"phys addr = 0x%x\n", *physAddr);
  return NO_EXCEPTION;
}
