/*! \file mmu.h
   \brief Data structures for the MMU (Memory Management Unit)

    DO NOT CHANGE -- part of the machine emulation
  
    Copyright (c) 1999-2000 INSA de Rennes.
    All rights reserved.  
    See copyright_insa.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
*/


#ifndef MMU_H
#define MMU_H

/*! \brief Defines a MMU - Memory Management Unit
*/
// This object manages the memory of the simulated MIPS processor for
// the Nachos kernel.
class MMU {
public:
  MMU();
  
  ~MMU();
  
  bool ReadMem(int addr, int size, int* value, bool is_instruction);
                                //!< Read or write 1, 2, or 4 bytes of virtual 
				//!< memory (at addr).  Return FALSE if a 

  bool WriteMem(int addr, int size, int value);
    				//!< Write or write 1, 2, or 4 bytes of virtual 
				//!< memory (at addr).  Return FALSE if a 
				//!< correct translation couldn't be found.
  
  ExceptionType Translate(int virtAddr, int* physAddr,
			  int size, bool writing);
    				//!< Translate an address, and check for 
				//!< alignment. Set the use and dirty bits in 
				//!< the translation entry appropriately,
    				//!< and return an exception code if the 
				//!< translation couldn't be completed.
  
  // NOTE: the hardware translation of virtual addresses in the user program
  // to physical addresses (relative to the beginning of "mainMemory")
  // is controlled by a traditional linear page table
  TranslationTable *translationTable; //!< Pointer to the translation table
};

#endif // MMU_H
