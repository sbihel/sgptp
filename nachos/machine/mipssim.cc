/*! \file mipssim.cc 
// \brief simulate a MIPS R2/3000 processor
//
//   This code has been adapted from Ousterhout's MIPSSIM package.
//   Byte ordering is little-endian, so we can be compatible with
//   DEC RISC systems.
*/
//   DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include <math.h>   /* For emulating floating point MIPS instructions */
#include "machine/machine.h"
#include "machine/mipssim.h"
#include "kernel/system.h"
#include "kernel/thread.h"

// Forward definition
static void Mult(int a, int b, bool signedArith, int* hiPtr, int* loPtr);

//----------------------------------------------------------------------
// Machine::Run
/*! 	Make the MIPS machine start the execution of a user program.
//	Called by the kernel when the program starts up; never returns.
//
//	This routine is re-entrant, in that it can be called multiple
//	times concurrently -- one for each thread executing user code.
*/
//----------------------------------------------------------------------
void
Machine::Run()
{
  Instruction instr;
  
  // Execution time of every executed instruction (for statistics)
  int tps;

  // We are now in user mode
  this->status = USER_MODE;

  // Machine main loop : execute instructions one at a time
  for (;;) {
      tps = OneInstruction(&instr);

      // machine mode is not set accordingly in case of page faults
      // triggered by the instruction... Have to fix that
      this->status =  USER_MODE;

      // Advance simulated time and check if there are any pending 
      // interrupts to be called. 
      interrupt->OneTick(tps);

      // Call the debugger is required
      if (singleStep && (runUntilTime <= g_stats->getTotalTicks()))
	  Debugger();
    }
}

//----------------------------------------------------------------------
// TypeToReg
//! 	Retrieve the register number referred to in an instruction. 
//      \param reg Register type according to the types defined in machine.h
//      \param instr Instruction
//      \return Register number found in the instruction, -1 in case of
//              invalid register type
//----------------------------------------------------------------------
static int 
TypeToReg(RegType reg, Instruction *instr)
{
    switch (reg) {
      case RS:
	return instr->rs;
      case RT:
	return instr->rt;
      case RD:
	return instr->rd;
      case FS:
	return instr->fs;
      case FT:
	return instr->ft;
      case FD:
	return instr->fd;
    case EXTRA:
	return instr->extra;
      default:
	return -1;
    }
}

// ----------------------------------------------------------------------
// Operations to load and store double precision floating points from/to
// general purpose float registers
//----------------------------------------------------------------------
static double get_double(int reg)
{
  int vint[2];
  ASSERT(reg+1 < NUM_FP_REGS);
  if (host_endianess == IS_BIG_ENDIAN) {
    vint[1] = g_machine->float_registers[reg];
    vint[0] = g_machine->float_registers[reg+1];
  }
  else {
    vint[0] = g_machine->float_registers[reg];
    vint[1] = g_machine->float_registers[reg+1];
  }
  return *((double*) vint);
}

static void set_double(int reg,double val)
{
  int *vint;
  ASSERT(reg+1 < NUM_FP_REGS);
  vint = (int *) (&val);
  if (host_endianess == IS_BIG_ENDIAN) {
    g_machine->float_registers[reg] = vint[1];
    g_machine->float_registers[reg+1] = vint[0];
  }
  else {
    g_machine->float_registers[reg] = vint[0];
    g_machine->float_registers[reg+1] = vint[1];
  }
}


//----------------------------------------------------------------------
// int Machine::OneInstruction
/*!	Execute one instruction from a user-level program
//
// 	If there is any kind of exception or interrupt, we invoke the 
//	exception handler, and when it returns, we return to Run(), which
//	will re-invoke us in a loop.  This allows us to
//	re-start the instruction execution from the beginning, in
//	case any of our state has changed.  On a syscall,
// 	the OS software must increment the PC so execution begins
// 	at the instruction immediately after the syscall. 
//
//	This routine is re-entrant, in that it can be called multiple
//	times concurrently -- one for each thread executing user code.
//	We get re-entrancy by never caching any data -- we always re-start the
//	simulation from scratch each time we are called (or after trapping
//	back to the Nachos kernel on an exception or interrupt), and we always
//	store all data back to the machine registers and memory before
//	leaving.  This allows the Nachos kernel to control our behavior
//	by controlling the contents of memory, the translation table,
//	and the register set.
//
//  \param instr Instruction to be executed
//  \return Execution time of the instruction in cycles
*/
//----------------------------------------------------------------------
int
Machine::OneInstruction(Instruction *instr)
{
  int32_t raw;                  // binary representation of the instruction
  int nextLoadReg = 0; 	
  int nextLoadValue = 0; 	// record delayed load operation, to apply
				// in the future
  int execution_time;           // execution time of the instruction

  // For floating point operations
  float *fsptr,*fdptr;        // To store result of FP operations
  float f1,f2;                // For FP operations
  double d1,d2;               // For FP operations

  // Temporary variable
  int tmp;

  // Fetch instruction from memory
  if (!mmu->ReadMem(int_registers[PC_REG], 4, &raw, true))
    return 0;			// exception occurred

  // Constant execution time for user instructions (see stats.h)
  execution_time = USER_TICK;

  // Update statistics
  g_current_thread->GetProcessOwner()->stat->incrNumInstruction();
    
  // Decode instruction
  instr->value = raw;
  instr->Decode();

  // Print its textual representation if debug flag 'm' is set
  if (DebugIsEnabled('m')) {

    struct OpString *stri = &opStrings[instr->opCode];

    ASSERT(instr->opCode <= MaxOpcode);
    printf("Thread %s At PC = 0x%x: ",g_current_thread->GetName(),int_registers[PC_REG]);
    if (instr->opCode==OP_BEQ ||
	instr->opCode==OP_BGEZAL||
	instr->opCode==OP_BGEZ||
	instr->opCode==OP_BGTZ||
	instr->opCode==OP_BLEZ||
	instr->opCode==OP_BLTZAL||
	instr->opCode==OP_BLTZ||
	instr->opCode==OP_BNE||
	instr->opCode==OP_JAL||
	instr->opCode==OP_J||
	instr->opCode==OP_JALR||
	instr->opCode==OP_JR||
	instr->opCode==OP_BC1F||
	instr->opCode==OP_BC1T
	) {
      // In case of a branch, extra is not directly the offset
      printf(stri->string, 
	     IndexToAddr(instr->extra), 
	     TypeToReg(stri->args[1], instr), 
	     TypeToReg(stri->args[2], instr));
    }
    else {
      // Normal instruction
      printf(stri->string, 
	     TypeToReg(stri->args[0], instr), 
	     TypeToReg(stri->args[1], instr), 
	     TypeToReg(stri->args[2], instr));
    }
    printf(" Time total %llu\n",g_stats->getTotalTicks());
  }

  // Compute next Program Counter (PC), but don't install in 
  // case there's an error or branch.
  int pcAfter = int_registers[NEXTPC_REG] + 4;
  int sum, diff, value;
  unsigned int rs, rt, imm;

  // Execute the instruction
  // Look at the opCode field to perform the right action
  switch (instr->opCode) {
	
      case OP_ADD:
	sum = int_registers[(int)instr->rs] + int_registers[(int)instr->rt];
	if (!((int_registers[(int)instr->rs] ^ int_registers[(int)instr->rt])
	      & SIGN_BIT) &&
	    ((int_registers[(int)instr->rs] ^ sum) & SIGN_BIT)) {
	    RaiseException(OVERFLOW_EXCEPTION, 0);
	    return 0;
	}
	int_registers[(int)instr->rd] = sum;
	break;
	
      case OP_ADDI:
	sum = int_registers[(int)instr->rs] + instr->extra;
	if (!((int_registers[(int)instr->rs] ^ instr->extra) & SIGN_BIT) &&
	    ((instr->extra ^ sum) & SIGN_BIT)) {
	    RaiseException(OVERFLOW_EXCEPTION, 0);
	    return 0;
	}
	int_registers[(int)instr->rt] = sum;
	break;
	
      case OP_ADDIU:
	int_registers[(int)instr->rt] = int_registers[(int)instr->rs] + instr->extra;
	break;
	
      case OP_ADDU:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rs]
	  + int_registers[(int)instr->rt];
	break;
	
      case OP_AND:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rs]
	  & int_registers[(int)instr->rt];
	break;
	
      case OP_ANDI:
	int_registers[(int)instr->rt] = int_registers[(int)instr->rs]
	  & (instr->extra & 0xffff);
	break;
	
      case OP_BEQ:
	if (int_registers[(int)instr->rs] == int_registers[(int)instr->rt])
	    pcAfter = int_registers[NEXTPC_REG] + IndexToAddr(instr->extra);
	break;
	
      case OP_BGEZAL:
	int_registers[R31] = int_registers[NEXTPC_REG] + 4;
      case OP_BGEZ:
	if (!(int_registers[(int)instr->rs] & SIGN_BIT))
	    pcAfter = int_registers[NEXTPC_REG] + IndexToAddr(instr->extra);
	break;
	
      case OP_BGTZ:
	if (int_registers[(int)instr->rs] > 0)
	    pcAfter = int_registers[NEXTPC_REG] + IndexToAddr(instr->extra);
	break;
	
      case OP_BLEZ:
	if (int_registers[(int)instr->rs] <= 0)
	    pcAfter = int_registers[NEXTPC_REG] + IndexToAddr(instr->extra);
	break;
	
      case OP_BLTZAL:
	int_registers[R31] = int_registers[NEXTPC_REG] + 4;
      case OP_BLTZ:
	if (int_registers[(int)instr->rs] & SIGN_BIT)
	    pcAfter = int_registers[NEXTPC_REG] + IndexToAddr(instr->extra);
	break;
	
      case OP_BNE:
	if (int_registers[(int)instr->rs] != int_registers[(int)instr->rt])
	    pcAfter = int_registers[NEXTPC_REG] + IndexToAddr(instr->extra);
	break;
	
      case OP_DIV:
	if (int_registers[(int)instr->rt] == 0) {
	    int_registers[LO_REG] = 0;
	    int_registers[HI_REG] = 0;
	} else {
	    int_registers[LO_REG] =  int_registers[(int)instr->rs]
	      / int_registers[(int)instr->rt];
	    int_registers[HI_REG] = int_registers[(int)instr->rs]
	      % int_registers[(int)instr->rt];
	}
	break;
	
      case OP_DIVU:	  
	  rs = (unsigned int) int_registers[(int)instr->rs];
	  rt = (unsigned int) int_registers[(int)instr->rt];
	  if (rt == 0) {
	      int_registers[LO_REG] = 0;
	      int_registers[HI_REG] = 0;
	  } else {
	      tmp = rs / rt;
	      int_registers[LO_REG] = (int) tmp;
	      tmp = rs % rt;
	      int_registers[HI_REG] = (int) tmp;
	  }
	  break;
	
      case OP_JAL:
	int_registers[R31] = int_registers[NEXTPC_REG] + 4;
      case OP_J:
	pcAfter = (pcAfter & 0xf0000000) | IndexToAddr(instr->extra);
	break;
	
      case OP_JALR:
	int_registers[(int)instr->rd] = int_registers[NEXTPC_REG] + 4;
      case OP_JR:
	pcAfter = int_registers[(int)instr->rs];
	break;
	
      case OP_LB:
      case OP_LBU:
	tmp = int_registers[(int)instr->rs] + instr->extra;
	if (!mmu->ReadMem(tmp, 1, &value,false))
	    return 0;
	if ((value & 0x80) && (instr->opCode == OP_LB))
	    value |= 0xffffff00;
	else
	    value &= 0xff;
	nextLoadReg = instr->rt;
	nextLoadValue = value;
	break;
	
      case OP_LH:
      case OP_LHU:	  
	tmp = int_registers[(int)instr->rs] + instr->extra;
	if (tmp & 0x1) {
	  RaiseException(ADDRESSERROR_EXCEPTION, tmp);
	  return 0;
	}
	if (!mmu->ReadMem(tmp, 2, &value,false))
	  return 0;

	if ((value & 0x8000) && (instr->opCode == OP_LH))
	    value |= 0xffff0000;
	else
	    value &= 0xffff;
	nextLoadReg = instr->rt;
	nextLoadValue = value;
	break;
      	
      case OP_LUI:
	int_registers[(int)instr->rt] = instr->extra << 16;
	break;
	
      case OP_LW:
	tmp = int_registers[(int)instr->rs] + instr->extra;
	if (tmp & 0x3) {
	    RaiseException(ADDRESSERROR_EXCEPTION, tmp);
	    return 0;
	}
	if (!mmu->ReadMem(tmp, 4, &value,false))
	  return 0;
	nextLoadReg = instr->rt;
	nextLoadValue = value;
	break;
    	
      case OP_LWL:	  
	tmp = int_registers[(int)instr->rs] + instr->extra;

	// ReadMem assumes all 4 byte requests are aligned on an even 
	// word boundary.  Also, the little endian/big endian swap code would
        // fail (I think) if the other cases are ever exercised.
	//ASSERT((tmp & 0x3) == 0);  

	if (!mmu->ReadMem(tmp, 4, &value,false))
	  return 0;
	if (int_registers[LOAD_REG] == instr->rt)
	    nextLoadValue = int_registers[LOADVALUE_REG];
	else
	    nextLoadValue = int_registers[(int)instr->rt];
	switch (tmp & 0x3) {
	  case 0:
	    nextLoadValue = value;
	    break;
	  case 1:
	    nextLoadValue = (nextLoadValue & 0xff) | (value << 8);
	    break;
	  case 2:
	    nextLoadValue = (nextLoadValue & 0xffff) | (value << 16);
	    break;
	  case 3:
	    nextLoadValue = (nextLoadValue & 0xffffff) | (value << 24);
	    break;
	}
	nextLoadReg = instr->rt;
	break;
      	
      case OP_LWR:
	tmp = int_registers[(int)instr->rs] + instr->extra;

	// ReadMem assumes all 4 byte requests are aligned on an even 
	// word boundary.  Also, the little endian/big endian swap code would
        // fail (I think) if the other cases are ever exercised.
	//ASSERT((tmp & 0x3) == 0);  

	if (!mmu->ReadMem(tmp, 4, &value,false))
	  return 0;
	if (int_registers[LOAD_REG] == instr->rt)
	    nextLoadValue = int_registers[LOADVALUE_REG];
	else
	    nextLoadValue = int_registers[(int)instr->rt];
	switch (tmp & 0x3) {
	  case 0:
	    nextLoadValue = (nextLoadValue & 0xffffff00) |
		((value >> 24) & 0xff);
	    break;
	  case 1:
	    nextLoadValue = (nextLoadValue & 0xffff0000) |
		((value >> 16) & 0xffff);
	    break;
	  case 2:
	    nextLoadValue = (nextLoadValue & 0xff000000)
		| ((value >> 8) & 0xffffff);
	    break;
	  case 3:
	    nextLoadValue = value;
	    break;
	}
	nextLoadReg = instr->rt;
	break;
    	
      case OP_MFHI:
	int_registers[(int)instr->rd] = int_registers[HI_REG];
	break;
	
      case OP_MFLO:
	int_registers[(int)instr->rd] = int_registers[LO_REG];
	break;
	
      case OP_MTHI:
	int_registers[HI_REG] = int_registers[(int)instr->rs];
	break;
	
      case OP_MTLO:
	int_registers[LO_REG] = int_registers[(int)instr->rs];
	break;
	
      case OP_MULT:
	Mult(int_registers[(int)instr->rs], int_registers[(int)instr->rt], true,
	     &int_registers[HI_REG], &int_registers[LO_REG]);
	break;
	
      case OP_MULTU:
	Mult(int_registers[(int)instr->rs], int_registers[(int)instr->rt], false,
	     &int_registers[HI_REG], &int_registers[LO_REG]);
	break;
	
      case OP_NOR:
	int_registers[(int)instr->rd] = ~(int_registers[(int)instr->rs]
				     | int_registers[(int)instr->rt]);
	break;
	
      case OP_OR:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rs]
	  | int_registers[(int)instr->rs];
	break;
	
      case OP_ORI:
	int_registers[(int)instr->rt] = int_registers[(int)instr->rs]
	  | (instr->extra & 0xffff);
	break;
	
      case OP_SB:
	if (!mmu->WriteMem((unsigned) 
		(int_registers[(int)instr->rs] + instr->extra), 1,
			       int_registers[(int)instr->rt]))
	    return 0;
	break;
	
      case OP_SH:
	if (!mmu->WriteMem((unsigned) 
		(int_registers[(int)instr->rs] + instr->extra), 2,
			       int_registers[(int)instr->rt]))
	    return 0;
	break;
	
      case OP_SLL:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rt] << instr->extra;
	break;
	
      case OP_SLLV:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rt] <<
	    (int_registers[(int)instr->rs] & 0x1f);
	break;
	
      case OP_SLT:
	if (int_registers[(int)instr->rs] < int_registers[(int)instr->rt])
	    int_registers[(int)instr->rd] = 1;
	else
	    int_registers[(int)instr->rd] = 0;
	break;
	
      case OP_SLTI:
	if (int_registers[(int)instr->rs] < instr->extra)
	    int_registers[(int)instr->rt] = 1;
	else
	    int_registers[(int)instr->rt] = 0;
	break;
	
      case OP_SLTIU:	  
	rs = int_registers[(int)instr->rs];
	imm = instr->extra;
	if (rs < imm)
	    int_registers[(int)instr->rt] = 1;
	else
	    int_registers[(int)instr->rt] = 0;
	break;
      	
      case OP_SLTU:	  
	rs = int_registers[(int)instr->rs];
	rt = int_registers[(int)instr->rt];
	if (rs < rt)
	    int_registers[(int)instr->rd] = 1;
	else
	    int_registers[(int)instr->rd] = 0;
	break;
      	
      case OP_SRA:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rt] >> instr->extra;
	break;
	
      case OP_SRAV:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rt] >>
	    (int_registers[(int)instr->rs] & 0x1f);
	break;
	
      case OP_SRL:
	tmp = int_registers[(int)instr->rt];
	tmp >>= instr->extra;
	int_registers[(int)instr->rd] = tmp;
	break;
	
      case OP_SRLV:
	tmp = int_registers[(int)instr->rt];
	tmp >>= (int_registers[(int)instr->rs] & 0x1f);
	int_registers[(int)instr->rd] = tmp;
	break;
	
      case OP_SUB:	  
	diff = int_registers[(int)instr->rs] - int_registers[(int)instr->rt];
	if (((int_registers[(int)instr->rs] ^ int_registers[(int)instr->rt])
	     & SIGN_BIT) &&
	    ((int_registers[(int)instr->rs] ^ diff) & SIGN_BIT)) {
	    RaiseException(OVERFLOW_EXCEPTION, 0);
	    return 0;
	}
	int_registers[(int)instr->rd] = diff;
	break;
      	
      case OP_SUBU:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rs]
	  - int_registers[(int)instr->rt];
	break;
	
      case OP_SW:
	if (!mmu->WriteMem((unsigned) 
		(int_registers[(int)instr->rs] + instr->extra), 4,
			       int_registers[(int)instr->rt]))
	    return 0;
	break;
	
      case OP_SWL:	  
	tmp = int_registers[(int)instr->rs] + instr->extra;

	// The little endian/big endian swap code would
        // fail (I think) if the other cases are ever exercised.
	//ASSERT((tmp & 0x3) == 0);  

	if (!mmu->ReadMem(tmp & ~0x3, 4, &value,false))
	  return 0;
	switch (tmp & 0x3) {
	  case 0:
	    value = int_registers[(int)instr->rt];
	    break;
	  case 1:
	    value = (value & 0xff000000) | ((int_registers[(int)instr->rt] >> 8) &
					    0xffffff);
	    break;
	  case 2:
	    value = (value & 0xffff0000) | ((int_registers[(int)instr->rt] >> 16) &
					    0xffff);
	    break;
	  case 3:
	    value = (value & 0xffffff00) | ((int_registers[(int)instr->rt] >> 24) &
					    0xff);
	    break;
	}
	if (!mmu->WriteMem((tmp & ~0x3), 4, value))
	    return 0;
	break;
    	
      case OP_SWR:	  
	tmp = int_registers[(int)instr->rs] + instr->extra;

	// The little endian/big endian swap code would
        // fail (I think) if the other cases are ever exercised.
	//ASSERT((tmp & 0x3) == 0);  

	if (!mmu->ReadMem(tmp & ~0x3, 4, &value,false))
	  return 0;
	switch (tmp & 0x3) {
	  case 0:
	    value = (value & 0xffffff) | (int_registers[(int)instr->rt] << 24);
	    break;
	  case 1:
	    value = (value & 0xffff) | (int_registers[(int)instr->rt] << 16);
	    break;
	  case 2:
	    value = (value & 0xff) | (int_registers[(int)instr->rt] << 8);
	    break;
	  case 3:
	    value = int_registers[(int)instr->rt];
	    break;
	}
	if (!mmu->WriteMem((tmp & ~0x3), 4, value))
	    return 0;
	break;
    	
      case OP_SYSCALL:
	RaiseException(SYSCALL_EXCEPTION, 0);
	return 0; 
	
      case OP_XOR:
	int_registers[(int)instr->rd] = int_registers[(int)instr->rs]
	  ^ int_registers[(int)instr->rt];
	break;
	
      case OP_XORI:
	int_registers[(int)instr->rt] = int_registers[(int)instr->rs]
	  ^ (instr->extra & 0xffff);
	break;
	
    /* Floating point instructions (at least some of them...) */
    /* No delay loads are implemented for FP operations */
    /* No exceptions are signalled on overflow. Fixed point operations */
    /* are not supported yet */
    /* NB : potentially non portable code (the floating point representation
       of the target architecture must be the same than the MIPS one */

    /* Load/store FP operations */
    case OP_LWC1:
      tmp = int_registers[(int)instr->rs] + instr->extra;
      if (tmp & 0x3) {
	  RaiseException(ADDRESSERROR_EXCEPTION, tmp);
	  return 0;
      }
      if (!mmu->ReadMem(tmp, 4, &value,false))
	  return 0;
      float_registers[(int)instr->ft] = value;
      break; 

    case OP_LDC1:
      tmp = int_registers[(int)instr->rs] + instr->extra;
      if (tmp & 0x7) {
	  RaiseException(ADDRESSERROR_EXCEPTION, tmp);
	  return 0;
      }
      if (!mmu->ReadMem(tmp, 4, &value,false))
	  return 0;
      float_registers[(int)instr->ft] = value;
      if (!mmu->ReadMem(tmp+4, 4, &value,false))
	  return 0;
      float_registers[(int)instr->ft+1] = value;
      break; 

    case OP_SWC1:
      if (!mmu->WriteMem((unsigned) 
		(int_registers[(int)instr->rs] + instr->extra), 4, 
		float_registers[(int)instr->ft]))
      return 0;
      break;

    case OP_SDC1:
      if (!mmu->WriteMem((unsigned) 
		(int_registers[(int)instr->rs] + instr->extra), 4, 
		float_registers[(int)instr->ft]))
      return 0;
      if (!mmu->WriteMem((unsigned) 
		(int_registers[(int)instr->rs] + instr->extra+4), 4, 
		float_registers[(int)instr->ft+1]))
      return 0;
      break;

     case OP_MOV_S:
      float_registers[(int)instr->fd] = float_registers[(int)instr->fs];
      break;

    case OP_MOV_D:
      float_registers[(int)instr->fd] = float_registers[(int)instr->fs];
      float_registers[(int)instr->fd+1] = float_registers[(int)instr->fs+1];  
      break;

    case OP_MFC1:
      int_registers[(int)instr->rt] = float_registers[(int)instr->fs];
      break;

    case OP_CFC1:
      int_registers[(int)instr->rt] = float_registers[(int)instr->fs];
      break;

    case OP_MTC1:
      float_registers[(int)instr->fs] = int_registers[(int)instr->rt];
      break;

    case OP_CTC1:
      float_registers[(int)instr->fs] = int_registers[(int)instr->rt];
      break;

    /* Arithmetic operations on floats and doubles 
       (abs,add,sqrt,sub,mul,div,neg) */
    case OP_ABS_S: 
      fdptr = (float *) &(float_registers[(int)instr->fd]);
      fsptr = (float *) &(float_registers[(int)instr->fs]);
      (*fdptr) = (float) fabs((double)(*fsptr));
      break; 

    case OP_ABS_D:
      d1 = get_double(instr->fs);
      set_double(instr->fd,fabs(d1));
      break; 

    case OP_ADD_S:
      f1 = *((float*) &float_registers[(int)instr->fs]);
      f2 = *((float*) &float_registers[(int)instr->ft]);
      fdptr = (float*) &float_registers[(int)instr->fd];
      (*fdptr) = f1+f2;
      break;

    case OP_ADD_D: {
      d1 = get_double(instr->fs);
      d2 = get_double(instr->ft);     
      set_double(instr->fd,d1+d2);
    }
      break; 

    case OP_DIV_S:
      f1 = *((float*) &float_registers[(int)instr->fs]);
      f2 = *((float*) &float_registers[(int)instr->ft]);
      fdptr = (float*) &float_registers[(int)instr->fd];
      (*fdptr) = f1 / f2;
      break; 

    case OP_DIV_D:
      d1 = get_double(instr->fs);
      d2 = get_double(instr->ft);
      set_double(instr->fd, d1 / d2);
      break;

    case OP_MUL_S:
      f1 = *((float*) &float_registers[(int)instr->fs]);
      f2 = *((float*) &float_registers[(int)instr->ft]);
      fdptr = (float*) &float_registers[(int)instr->fd];
      (*fdptr) = f1*f2;
      break; 

    case OP_MUL_D:
      d1 = get_double(instr->fs);
      d2 = get_double(instr->ft);
      set_double(instr->fd, d1*d2);
      break; 

    case OP_NEG_S:
      fdptr = (float *) &(float_registers[(int)instr->fd]);
      fsptr = (float *) &(float_registers[(int)instr->fs]);
      (*fdptr) = -1.0*(*fsptr);
      break;

    case OP_NEG_D:
      d1 = get_double(instr->fs);
      set_double(instr->fd,-1.0*d1);
      break; 

    case OP_SUB_S:
      f1 = *((float*) &float_registers[(int)instr->fs]);
      f2 = *((float*) &float_registers[(int)instr->ft]);
      fdptr = (float*) &float_registers[(int)instr->fd];
      (*fdptr) = f1 - f2;
      break; 

    case OP_SUB_D:
      d1 = get_double(instr->fs);
      d2 = get_double(instr->ft);
      set_double(instr->fd, d1-d2);
      break; 

    case OP_SQRT_S:
      fdptr = (float *) &(float_registers[(int)instr->fd]);
      fsptr = (float *) &(float_registers[(int)instr->fs]);
      if (*fsptr <0) {
	RaiseException(OVERFLOW_EXCEPTION,0); return 0;
      }
      /* Compute the square root*/
      /* (in double precision, op on floats does not exist) */
      (*fdptr) = (float) sqrt((double)(*fsptr));
      break; 

    case OP_SQRT_D:
      d1 = get_double(instr->fs);
      if (d1<0) {
	RaiseException(OVERFLOW_EXCEPTION,0); return 0;
      }
      set_double(instr->fd,sqrt(d1));
      break;

   /* Conversion operations between float and double */
    case OP_CVT_S_D:
      d1 = get_double(instr->fs);
      f1 = (float) d1;
      fdptr = (float*) &float_registers[(int)instr->fd];
      (*fdptr) = f1;
      break;

   case OP_CVT_D_S:
      f1 = *((float*) &float_registers[(int)instr->fs]);
      d1 = (double) f1;
      set_double(instr->fd,d1);
      break;  

    case OP_CVT_S_W:
      *((float*) &float_registers[(int)instr->fd]) = 
	(float) float_registers[(int)instr->fs];
      break;
    case OP_CVT_W_S:
      float_registers[(int)instr->fd] = (int) 
	(*((float*) &float_registers[(int)instr->fs]));
      break;
    case OP_CVT_D_W:
      set_double(instr->fd,(double)float_registers[(int)instr->fs]);
      break;
    case OP_CVT_W_D:
      float_registers[(int)instr->fd] = (int) get_double(instr->fs);
      break;
   
    
    /* Floating point comparison instructions */
    /* NB : the implementation of these instructions is greatly simplified
            compared to the original MIPS processor. No attention is paid
	    to exceptions except address errors, no support
	    for NaN numbers and for the IEEE FP standard concerning
	    the "unordered" relation */
	    
    case OP_C_SF_S: /* Condition False. Always set CC to False */
    case OP_C_F_S:
    case OP_C_F_D:
    case OP_C_SF_D:
      cc = false;
      break;
     
    case OP_C_EQ_S:  /* Test equality regarless of "unordered" floats  */
    case OP_C_UEQ_S: /* and exception requests*/
    case OP_C_SEQ_S:
    case OP_C_NGL_S:
      f1 = *((float*) &float_registers[(int)instr->fs]);
      f2 = *((float*) &float_registers[(int)instr->ft]);
      if (f1==f2) cc=true; else cc=false;
      break;

    case OP_C_OLT_S: /* Less than, regarless of "unordered" floats  */
    case OP_C_ULT_S: /* and exception requests */
    case OP_C_LT_S:
    case OP_C_NGE_S:
      f1 = *((float*) &float_registers[(int)instr->fs]);
      f2 = *((float*) &float_registers[(int)instr->ft]);
      if (f1<f2) cc=true; else cc=false;
      break;

    case OP_C_OLE_S: /* Less or equal, regarless of "unordered" floats  */
    case OP_C_ULE_S: /* and exception requests */
    case OP_C_LE_S:
    case OP_C_NGT_S:
      f1 = *((float*) &float_registers[(int)instr->fs]);
      f2 = *((float*) &float_registers[(int)instr->ft]);
      if (f1<=f2) cc=true; else cc=false;
      break;
      
    case OP_C_EQ_D:  /* Test equality regarless of "unordered" doubles  */
    case OP_C_UEQ_D: /* and exception requests */
    case OP_C_SEQ_D:
    case OP_C_NGL_D:
      d1 = get_double(instr->fs);
      d2 = get_double(instr->ft);
      if (d1==d2) cc=true; else cc=false;
      break;

    case OP_C_OLT_D: /* Less than, regarless of "unordered" doubles  */
    case OP_C_ULT_D: /* and exception requests */
    case OP_C_LT_D:
    case OP_C_NGE_D:
      d1 = get_double(instr->fs);
      d2 = get_double(instr->ft);
      if (d1<d2) cc=true; else cc=false;
      break;

    case OP_C_OLE_D: /* Less or equal, regarless of "unordered" doubles  */
    case OP_C_ULE_D: /* and exception requests */
    case OP_C_LE_D:
    case OP_C_NGT_D:
      d1 = get_double(instr->fs);
      d2 = get_double(instr->ft);
      if (d1<=d2) cc=true; else cc=false;
      break;

    /* Floating point branch instructions (MIPS I only) */
    case OP_BC1F:
      if (cc == false) {
	pcAfter = int_registers[NEXTPC_REG] + IndexToAddr(instr->extra);
      }
      break;
    case OP_BC1T:
      if (cc == true) {
	pcAfter = int_registers[NEXTPC_REG] + IndexToAddr(instr->extra);
      }
      break;

    /* Reserved or non implemented yet instructions */

    case OP_C_UN_S: // "Unordered" condition of IEEE FP standard
    case OP_C_UN_D:
    case OP_C_NGLE_S:
    case OP_C_NGLE_D:

    case OP_BC1FL:  // All "likely" FP branches not implemented yet
    case OP_BC1TL:

    case OP_CEIL_W_S: // All Fixed point word instructions non impl yet
    case OP_CEIL_W_D:
    case OP_FLOOR_W_S:
    case OP_FLOOR_W_D:
    case OP_ROUND_W_S:
    case OP_ROUND_W_D:
    case OP_TRUNC_W_S:
    case OP_TRUNC_W_D:

    // Non implemented instruction
    // Signal it and stop Nachos
    case OP_UNIMP: {
       struct OpString *str = &opStrings[instr->opCode];
       printf("***** Fatal: not implemented yet MIPS instruction 0x%x\n",
	      instr->value); 
       ASSERT(instr->opCode <= MaxOpcode);
       printf("At PC = 0x%x: ", int_registers[PC_REG]);
       printf(str->string, TypeToReg(str->args[0], instr), 
	      TypeToReg(str->args[1], instr), 
	      TypeToReg(str->args[2], instr));
       printf("\n");
       }
    // Instruction opcode reserved for future MIPS extensions
    // Signal it and stop Nachos
    case OP_RES:
	RaiseException(ILLEGALINSTR_EXCEPTION, int_registers[PC_REG]);
	return 0;
	
    default:
	ASSERT(false);
    }

    // Now we have successfully executed the instruction.
    
    // Do any delayed load operation
    DelayedLoad(nextLoadReg, nextLoadValue);
    
    // For debugging, in case we are jumping into lala-land
    int_registers[PREVPC_REG] = int_registers[PC_REG];

    // Advance program counters.
    int_registers[PC_REG] = int_registers[NEXTPC_REG];
    int_registers[NEXTPC_REG] = pcAfter;

    return execution_time;
}

//----------------------------------------------------------------------
// Machine::DelayedLoad
/*! 	Simulate effects of a delayed load.
//
// 	NOTE -- RaiseException/CheckInterrupts must also call DelayedLoad,
//	since any delayed load must get applied before we trap to the kernel.
*/
//----------------------------------------------------------------------
void
Machine::DelayedLoad(int nextReg, int nextValue)
{
    int_registers[int_registers[LOAD_REG]] = int_registers[LOADVALUE_REG];
    int_registers[LOAD_REG] = nextReg;
    int_registers[LOADVALUE_REG] = nextValue;
    int_registers[0] = 0; 	// and always make sure R0 stays zero.
}

//----------------------------------------------------------------------
// Instruction::Decode
//! 	Decode a MIPS instruction 
//----------------------------------------------------------------------
void
Instruction::Decode()
{
    OpInfo *opPtr;
    
    // Fetch the rs,rt, ... fields from their location in the
    // instruction binary representation (see the MIPS manual for
    // more details)
    rs = (value >> 21) & 0x1f;
    rt = (value >> 16) & 0x1f;
    rd = (value >> 11) & 0x1f;
    fs = (value >> 11) & 0x1f;
    ft = (value >> 16) & 0x1f;
    fd = (value >> 6) & 0x1f;
    opPtr = &opTable[(value >> 26) & 0x3f];

    // Fetch the opcode
    opCode = opPtr->opCode;

    // Actually decode the instruction
    if (opPtr->format == IFMT) {
	extra = value & 0xffff;
	if (extra & 0x8000) {
    	   extra |= 0xffff0000;
	}
    } else if (opPtr->format == RFMT) {
	extra = (value >> 6) & 0x1f;
    } else {
	extra = value & 0x3ffffff;
    }
    if (opCode == SPECIAL) {
	opCode = specialTable[value & 0x3f];
    } else if (opCode == BCOND) {
	int i = value & 0x1f0000;

	if (i == 0) {
    	    opCode = OP_BLTZ;
	} else if (i == 0x10000) {
    	    opCode = OP_BGEZ;
	} else if (i == 0x100000) {
    	    opCode = OP_BLTZAL;
	} else if (i == 0x110000) {
    	    opCode = OP_BGEZAL;
	} else {
    	    opCode = OP_UNIMP;
	}
    } else if (opCode == COP1) { /* Floating point instructions */
      if (rs == 0x10 ) { opCode = cop1STable[value & 0x3f];
      } else if (rs == 0x11 ) { opCode = cop1DTable[value & 0x3f];
      } else if (rs == 0x08 && rt == 0x00) { opCode = OP_BC1F; 
      } else if (rs == 0x08 && rt == 0x01) { opCode = OP_BC1T; 
      } else if (rs == 0x00 ) { opCode = OP_MFC1;
      } else if (rs == 0x02 ) { opCode = OP_CFC1;
      } else if (rs == 0x04 ) { opCode = OP_MTC1;
      } else if (rs == 0x06 ) { opCode = OP_CTC1;
      } else if (rs == 0x14 ) {
	int i = value & 0x3f;
	if (i==0x20) opCode = OP_CVT_S_W;
	else if (i==0x21) opCode = OP_CVT_D_W;
	else opCode = OP_UNIMP;
      } else opCode = OP_UNIMP;
    }
}

//----------------------------------------------------------------------
// Mult
/*! 	Simulate R2000 multiplication.
// 	The words at *hiPtr and *loPtr are overwritten with the
// 	double-length result of the multiplication.
*/
//----------------------------------------------------------------------
static void
Mult(int a, int b, bool signedArith, int* hiPtr, int* loPtr)
{
    if ((a == 0) || (b == 0)) {
	*hiPtr = *loPtr = 0;
	return;
    }

    // Compute the sign of the result, then make everything positive
    // so unsigned computation can be done in the main loop.
    bool negative = false;
    if (signedArith) {
	if (a < 0) {
	    negative = !negative;
	    a = -a;
	}
	if (b < 0) {
	    negative = !negative;
	    b = -b;
	}
    }

    // Compute the result in unsigned arithmetic (check a's bits one at
    // a time, and add in a shifted value of b).
    unsigned int bLo = b;
    unsigned int bHi = 0;
    unsigned int lo = 0;
    unsigned int hi = 0;
    for (int i = 0; i < 32; i++) {
	if (a & 1) {
	    lo += bLo;
	    if (lo < bLo)  // Carry out of the low bits?
		hi += 1;
	    hi += bHi;
	    if ((a & 0xfffffffe) == 0)
		break;
	}
	bHi <<= 1;
	if (bLo & 0x80000000)
	    bHi |= 1;
	
	bLo <<= 1;
	a >>= 1;
    }

    // If the result is supposed to be negative, compute the two's
    // complement of the double-word result.
    if (negative) {
	hi = ~hi;
	lo = ~lo;
	lo++;
	if (lo == 0)
	    hi++;
    }
    
    *hiPtr = (int) hi;
    *loPtr = (int) lo;
}
