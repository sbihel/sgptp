/*
 ldscript for running user programs under Nachos

 Sections should be aligned on page boundaries. Here an alignement of
 at least 0x2000 is selected, thus supporting pages up to 8KB
 large. See addrspace.cc for details.
*/

ENTRY(__start)
SECTIONS
{

  /* Skip an area of about 8k, so that NULL pointer dereferences can
     be detected */
  . += 0x2000;

  .sys ALIGN(0x4000) : {
    *(.init)
    *(.sys)
  }

  /* Code is aligned on a 16k boundary
     Due to the size of the .sys section, the code start address will
     presumably be at address 0x4000 */
  .text ALIGN(0x400000) : {
    _ftext = .;
    eprol  =  .;
    *(.text)
    *(.fini)
  }
  etext  =  .;
  _etext  =  .;

  /* Initialized data is aligned on a 16k boundary */
  .data ALIGN(0x4000) : {
    _fdata = .;
    *(.data)
    *(.sdata)
  }
  .rodata ALIGN(0x4000) :
  {
    *(.rdata)
    *(.rodata)
  }
  edata  =  .;
  _edata  =  .;

  /* Non-initialized data is aligned on a 16k boundary */
  /* Bss = Block Started by Symbol */
  .bss  ALIGN(0x4000) : {
    *(.bss)
    *(.sbss)
    *(.scommon)
    *(COMMON)
  }

  /* Scratchpad memory (not used in Nachos assignments) */
  .scratchpad  ALIGN(0x4000) : {
  }

  end = .;
  _end = .;

  /* Not used in the Nachos assignments. Don't pay attention to this */
  .cache_locking (NOLOAD) : {
    LONG(ABSOLUTE(eprol)); /* Bottom address of the MIPS user code
                            * (excluding start.s !) */
    LONG(ABSOLUTE(etext)); /* Top address of MIPS user code */
  }

}
