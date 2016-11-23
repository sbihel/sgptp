/*! \file  addrspace.cc 
//  \brief Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//         1. Generate an ELF (Executable and Linkable Format) binary
//            using a MIPS cross-compiler (see how to do this in
//            test/Makefile)
//         2. Load the ELF file into the Nachos file system
//            (see documentation of configuration file nachos.cfg)
//         3. Execute it. You can do this
//            - when stating Nachos (see Nachos.cfg)
//            - by calling the appropriate system call in another
//              program (Exec)
//            - by typing the program name in the Nachos shell
//
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "machine/machine.h"
#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "kernel/synch.h"
#include "utility/stats.h"
#include "filesys/filesys.h"
#include "filesys/filehdr.h"
#include "filesys/openfile.h"
#include "vm/physMem.h"
#include "kernel/elf32.h"
#include "kernel/addrspace.h"

#define LONG2HOST(var) var = WordToHost(var)
#define SHORT2HOST(var) var = ShortToHost(var)

// Forward references
static void CheckELFHeader (Elf32_Ehdr *ehdr,int *err);
static void SwapELFSectionHeader (Elf32_Shdr *shdr);

//----------------------------------------------------------------------
/** 	Create an address space to run a user program.
 //	Load the program from a file "exec_file", and set everything
 //	up so that we can start executing user instructions.
 //
 //      Executables are in ELF (Executable and Linkable Format) (see elf32.h)
 //      and can be generated using a standard MIPS cross-compiler
 //      (here gcc).
 //
 //      For now, the code and data are entirely loaded into memory and
 //      the stacks are preallocated (NB: memory here stands for the
 //      memory of the simulated MIPS machine). Code/Data loading will
 //      be changed in the virtual memory assignment.
 //
 //      Don't look at this code right now. You may get lost. You will
 //      have plenty of time to do so in the virtual memory assignment
 //
 //	\param exec_file is the file containing the object code 
 //             to load into memory, or NULL when the address space
 //             should be empty
 //   \param process: process to be executed
 //   \param err: error code 0 if OK, -1 otherwise 
 */
//----------------------------------------------------------------------
AddrSpace::AddrSpace(OpenFile * exec_file, Process *p, int *err)
{
  Elf32_Ehdr elfHdr;      // Header du fichier exécutable

  *err  = 0;
  translationTable = NULL;
  freePageId = 0;
  process = p;

  /* Empty user address space requested ? */
  if (exec_file == NULL)
    {
      // Allocate translation table now
      translationTable = new TranslationTable();
      return;
    }

  // Read the header
  exec_file->ReadAt((char *) &elfHdr, sizeof(elfHdr), 0);

  // Check the file format
  CheckELFHeader(&elfHdr,err);
  printf("Check done \n");
  ASSERT(*err==NoError);

  printf("\n****  Loading file %s :\n", exec_file->GetName());

  /* Retrived the contents of section table*/
  Elf32_Shdr section_table[elfHdr.e_shnum*sizeof(elfHdr)];
  exec_file->ReadAt((char *) section_table, elfHdr.e_shnum*sizeof(elfHdr),
		    elfHdr.e_shoff);
  /* Swap the section header */
  int i;
  for (i = 0 ; i < elfHdr.e_shnum ; i++)
    SwapELFSectionHeader(& section_table[i]);

  /* Retrieve the section containing section names */
  Elf32_Shdr * shname_section = & section_table[elfHdr.e_shstrndx];
  char *shnames = new char[shname_section->sh_size];
  exec_file->ReadAt(shnames, shname_section->sh_size,
		    shname_section->sh_offset);

  // Create an empty translation table
  translationTable = new TranslationTable();

  // Compute the highest virtual address to init the translation table
  int mem_topaddr = 0;
  for (i = 0 ; i < elfHdr.e_shnum ; i++)
    {
      // Ignore empty sections
      if (section_table[i].sh_size <= 0)
	continue;
      int section_topaddr = section_table[i].sh_addr
                    + section_table[i].sh_size;
      if ((section_table[i].sh_flags & SHF_ALLOC)
	  && (section_topaddr > mem_topaddr))
	mem_topaddr = section_topaddr;
    }

  // Allocate space in virtual memory
  int base_addr = this->Alloc(divRoundUp(mem_topaddr, g_cfg->PageSize));
  // Make sure this region really starts at virtual address 0
  ASSERT(base_addr == 0);

  DEBUG('a', (char*)"Allocated virtual area [0x0,0x%x[ for program\n",
	mem_topaddr);

  // Loading of all sections
  for (i = 0 ; i < elfHdr.e_shnum ; i++)
    {
      // Retrieve the section name
      const char *section_name = shnames + section_table[i].sh_name;

      DEBUG('a', (char*)"Section %d : size=0x%x name=\"%s\"\n",
	     i, section_table[i].sh_size, section_name);

      // Ignore empty sections
      if (section_table[i].sh_size <= 0)
	continue;

      // Look if this section has to be loaded (SHF_ALLOC flag)
      if (! (section_table[i].sh_flags & SHF_ALLOC))
	continue;

      printf("\t- Section %s : file offset 0x%x, size 0x%x, addr 0x%x, %s%s\n",
	     section_name,
	     (unsigned)section_table[i].sh_offset,
	     (unsigned)section_table[i].sh_size,
	     (unsigned)section_table[i].sh_addr,
	     (section_table[i].sh_flags & SHF_WRITE)?"R/W":"R",
	     (section_table[i].sh_flags & SHF_EXECINSTR)?"/X":"");

      // Make sure section is aligned on page boundary
      ASSERT((section_table[i].sh_addr % g_cfg->PageSize)==0);


      // Initializes the page table entries and loads the section
      // in memory (demand paging will be implemented later on)
      for (unsigned int pgdisk = 0,
	     virt_page = section_table[i].sh_addr / g_cfg->PageSize ;
	   pgdisk < divRoundUp(section_table[i].sh_size, g_cfg->PageSize) ;
	   pgdisk++, virt_page ++)
	{

	  /* Without demand paging */
	  
	  // Set up default values for the page table entry
	  translationTable->clearBitSwap(virt_page);
	  translationTable->setBitReadAllowed(virt_page);
	  if (section_table[i].sh_flags & SHF_WRITE)
	    translationTable->setBitWriteAllowed(virt_page);
	  else translationTable->clearBitWriteAllowed(virt_page);
	  translationTable->clearBitIo(virt_page);

	  // Get a page in physical memory, halt of there is not sufficient space
	  int pp = g_physical_mem_manager->FindFreePage();
	  if (pp == -1) { 
	    printf("Not enough free space to load program %s\n",
		   exec_file->GetName());
	    g_machine->interrupt->Halt(-1);
	  }
	  g_physical_mem_manager->tpr[pp].virtualPage=virt_page;
	  g_physical_mem_manager->tpr[pp].owner = this;
	  g_physical_mem_manager->tpr[pp].locked=true;
	  translationTable->setPhysicalPage(virt_page,pp);
	  
	  // The SHT_NOBITS flag indicates if the section has an image
	  // in the executable file (text or data section) or not 
	  // (bss section)
	  if (section_table[i].sh_type != SHT_NOBITS) {
	    // The section has an image in the executable file
	    // Read it from the disk
	    exec_file->ReadAt((char *)&(g_machine->mainMemory[translationTable->getPhysicalPage(virt_page)*g_cfg->PageSize]),
			      g_cfg->PageSize, section_table[i].sh_offset + pgdisk*g_cfg->PageSize);

	  }
	  else {
	    // The section does not have an image in the executable
	    // Fill it with zeroes
	    memset(&(g_machine->mainMemory[translationTable->getPhysicalPage(virt_page)*g_cfg->PageSize]),
		   0, g_cfg->PageSize);
	  }
	  
	  // The page has been loded in physical memory but
	  // later-on will be saved in the swap disk. We have to indicate this
	  // in the translation table
	  translationTable->setAddrDisk(virt_page,-1);

	  // The entry is valid
	  translationTable->setBitValid(virt_page);
	  
	  /* End of code without demand paging */
	}
    }
  delete [] shnames;

  // Get program start address
  CodeStartAddress = (int32_t)elfHdr.e_entry;
  printf("\t- Program start address : 0x%lx\n\n",
	 (unsigned long)CodeStartAddress);

  // Init the number of memory mapped files to zero
  nb_mapped_files = 0;
}

//----------------------------------------------------------------------
/**   Deallocates an address space and in particular frees
 *   all memory it uses (RAM and swap area).
 */
//----------------------------------------------------------------------
AddrSpace::~AddrSpace()
{
  int i;

  if (translationTable != NULL) {
    
    // For every virtual page
    for (i = 0 ; i <  freePageId ; i++) {
      
      // If it is in physical memory, free the physical page
      if (translationTable->getBitValid(i))
	g_physical_mem_manager->RemovePhysicalToVirtualMapping(translationTable->getPhysicalPage(i));
      // If it is in the swap disk, free the corresponding disk sector
      if (translationTable->getBitSwap(i)) {
	int addrDisk = translationTable->getAddrDisk(i);
	if (addrDisk >= 0) {
	  g_swap_manager->ReleasePageSwap(translationTable->getAddrDisk(i));
	}  
      }
    }
    delete translationTable;
  }
}

//----------------------------------------------------------------------
/**	Allocates a new stack of size g_cfg->UserStackSize
 *
 *      Allocation is done by calling Alloc, a very simple
 *      allocation procedure of virtual memory areas.
 *
 *      \return stack pointer (at the end of the allocated stack)
 */
//----------------------------------------------------------------------
int AddrSpace::StackAllocate(void)
{
  // Optional : leave an anmapped blank space below the stack to
  // detect stack overflows
#define STACK_BLANK_LEN 4 // in pages
  int blankaddr = this->Alloc(STACK_BLANK_LEN);
  DEBUG('a', (char*)"Allocated unmapped virtual area [0x%x,0x%x[ for stack overflow detection\n",
	blankaddr*g_cfg->PageSize, (blankaddr+STACK_BLANK_LEN)*g_cfg->PageSize);

  // The new stack parameters
  int stackBasePage, numPages;
  numPages = divRoundUp(g_cfg->UserStackSize, g_cfg->PageSize);

  // Allocate virtual space for the new stack
  stackBasePage = this->Alloc(numPages);
  ASSERT (stackBasePage >= 0);
  DEBUG('a', (char*)"Allocated virtual area [0x%x,0x%x[ for stack\n",
	stackBasePage*g_cfg->PageSize,
	(stackBasePage+numPages)*g_cfg->PageSize);

  for (int i = stackBasePage ; i < (stackBasePage + numPages) ; i++) {
    /* Without demand paging */

    // Allocate a new physical page for the stack, halt if not page availabke
    int pp = g_physical_mem_manager->FindFreePage();
    if (pp == -1) { 
      printf("Not enough free space to load stack\n");
      g_machine->interrupt->Halt(-1);
    }
    g_physical_mem_manager->tpr[pp].virtualPage=i;
    g_physical_mem_manager->tpr[pp].owner = this;
    g_physical_mem_manager->tpr[pp].locked=true;
    translationTable->setPhysicalPage(i,pp);

    // Fill the page with zeroes
    memset(&(g_machine->mainMemory[translationTable->getPhysicalPage(i)*g_cfg->PageSize]),
	   0x0,g_cfg->PageSize);
    translationTable->setAddrDisk(i,-1);
    translationTable->setBitValid(i);
    translationTable->clearBitSwap(i);
    translationTable->setBitReadAllowed(i);
    translationTable->setBitWriteAllowed(i);
    translationTable->clearBitIo(i);
    /* End of code without demand paging */
    }

  int stackpointer = (stackBasePage+numPages)*g_cfg->PageSize - 4*sizeof(int);
  return stackpointer;
}

//----------------------------------------------------------------------
/**  Allocate numPages virtual pages in the current address space
//
//    \param numPages the number of contiguous virtual pages to allocate
//    \return the virtual page number of the beginning of the allocated
//      area, or -1 when not enough virtual space is available
*/
//----------------------------------------------------------------------
int AddrSpace::Alloc(int numPages) 
{
  int result = freePageId;

  DEBUG('a', (char*)"Virtual space alloc request for %d pages\n", numPages);

  // Check if the translation table is big enough for the allocation
  // to succeed
  if (freePageId + numPages >= translationTable->getMaxNumPages())
    return -1;

  // Do the allocation.
  // The allocation procedure is VERY SIMPLE. It just remembers
  // the number of the lastly allocated virtual page and increments it
  // when new pages are allocated. No de-allocation mechanisms is
  // implemented.
  freePageId += numPages;
  return result;
}

//----------------------------------------------------------------------
/** Map an open file in memory
 *
 * \param f: pointer to open file descriptor
 * \param size: size to be mapped in bytes (rounded up to next page boundary)
 * \return the virtual address at which the file is mapped
 */
// ----------------------------------------------------------------------
int AddrSpace::Mmap(OpenFile *f, int size)
{
  printf("**** Warning: method AddrSpace::Mmap is not implemented yet\n");
  exit(-1);
}

//----------------------------------------------------------------------
/*! Search if the address is in a memory-mapped file
 *
 * \param addr: virtual address to be searched for
 * \return address of file descriptor if found, NULL otherwise
 */
//----------------------------------------------------------------------
OpenFile *AddrSpace::findMappedFile(int32_t addr) {
  printf("**** Warning: method AddrSpace::findMappedFile is not implemented yet\n");
  exit(-1);

}

//----------------------------------------------------------------------
// SwapELFHeader
/*! 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//
*/
//----------------------------------------------------------------------
static void 
SwapELFHeader (Elf32_Ehdr *ehdr)
{
  SHORT2HOST(ehdr->e_type);
  SHORT2HOST(ehdr->e_machine);

  LONG2HOST(ehdr->e_version);
  LONG2HOST(ehdr->e_entry);
  LONG2HOST(ehdr->e_phoff);
  LONG2HOST(ehdr->e_shoff);
  LONG2HOST(ehdr->e_flags);

  SHORT2HOST(ehdr->e_ehsize);
  SHORT2HOST(ehdr->e_phentsize);
  SHORT2HOST(ehdr->e_phnum);
  SHORT2HOST(ehdr->e_shentsize);
  SHORT2HOST(ehdr->e_shnum);
  SHORT2HOST(ehdr->e_shstrndx);
}

//----------------------------------------------------------------------
// SwapELFSectionHeader
/*! 	Do little endian to big endian conversion on the bytes in the 
//	section header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//
*/
//----------------------------------------------------------------------
static void SwapELFSectionHeader (Elf32_Shdr *shdr)
{
  LONG2HOST(shdr->sh_name);
  LONG2HOST(shdr->sh_type);
  LONG2HOST(shdr->sh_flags);
  LONG2HOST(shdr->sh_addr);
  LONG2HOST(shdr->sh_offset);
  LONG2HOST(shdr->sh_size);
  LONG2HOST(shdr->sh_link);
  LONG2HOST(shdr->sh_info);
  LONG2HOST(shdr->sh_addralign);
  LONG2HOST(shdr->sh_entsize);
}

//----------------------------------------------------------------------
// CheckELFHeader
/*! 	Check the ELF header found in the executable file is correct
//
// \param shdr pointer to ELF header
// \param err error code (NoError if everything is correct)
*/
//----------------------------------------------------------------------
static void 
CheckELFHeader (Elf32_Ehdr *elfHdr, int *err)
{
  /* Make sure it is an ELF file by looking at its header (see elf32.h) */
  if (elfHdr->e_ident[EI_MAG0] != 0x7f ||
      elfHdr->e_ident[EI_MAG1] != 'E' ||
      elfHdr->e_ident[EI_MAG2] != 'L' ||
      elfHdr->e_ident[EI_MAG3] != 'F') {
      *err = ExecFileFormatError;
      return;
    }

  /* Make sure the ELF file type is correct */
  if (elfHdr->e_ident[EI_CLASS] != ELFCLASS32 ||
      elfHdr->e_ident[EI_VERSION] != EV_CURRENT) {
     printf("2\n");
     *err = ExecFileFormatError;
      return;
    }

  /* Check the endianess of the generated code */
  if (elfHdr->e_ident[EI_DATA] == ELFDATA2MSB) {   
    mips_endianess = IS_BIG_ENDIAN;
  }
  else {
    mips_endianess = IS_LITTLE_ENDIAN;
  }

  /* Transpose the header so that it can be interpreted by the kernel */
  SwapELFHeader(elfHdr);
      
  /* Make sure ELF binary code a MIPS executable */
  if (elfHdr->e_machine != EM_MIPS ||
      elfHdr->e_type != ET_EXEC) {
    *err = ExecFileFormatError;
    return;
  }

  /* Make sure ELF file internal structures are consistent with what
     we expect */
  if (elfHdr->e_ehsize != sizeof(Elf32_Ehdr) ||
      elfHdr->e_shentsize != sizeof(Elf32_Shdr))
    {
      *err = ExecFileFormatError;
      return;
    }

  /* Make sure ELF section table is available */
  if (elfHdr->e_shoff < sizeof(Elf32_Ehdr))
    {
      *err = ExecFileFormatError;
      return;
    }
  
  /* Make sure there is a string section name section */
  if (elfHdr->e_shstrndx >= elfHdr->e_shnum)
    {
      *err = ExecFileFormatError;
      return;
    }

  *err = NoError;

}
