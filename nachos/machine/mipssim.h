/*! \file mipssim.h 
 \brief Internal data structures for simulating the MIPS instruction set.
 DO NOT CHANGE -- part of the machine emulation
  
 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
 of liability and disclaimer of warranty provisions.
*/

#ifndef MIPSSIM_H
#define MIPSSIM_H

#include "kernel/copyright.h"

/*
 * OpCode values.  The names are straight from the MIPS
 * manual except for the following special ones:
 *
 * OP_UNIMP -		means that this instruction is legal, but hasn't
 *			been implemented in the simulator yet.
 * OP_RES -		means that this is a reserved opcode (it isn't
 *			supported by the architecture).
 *
 * Recent changes (I. Puaut, march 2002), addition of some
 *    COP1 floating point instructions (MIPS I only). 
 *    No fixed-point supported,
 *    and no support neither for NaN values. No exception raised
 *    (behavior is unspecified in such cases).
 */

#define OP_ADD		1
#define OP_ADDI		2
#define OP_ADDIU	3
#define OP_ADDU		4
#define OP_AND		5
#define OP_ANDI		6
#define OP_BEQ		7
#define OP_BGEZ		8
#define OP_BGEZAL	9
#define OP_BGTZ		10
#define OP_BLEZ		11
#define OP_BLTZ		12
#define OP_BLTZAL	13
#define OP_BNE		14
#define OP_DIV		16
#define OP_DIVU		17
#define OP_J		18
#define OP_JAL		19
#define OP_JALR		20
#define OP_JR		21
#define OP_LB		22
#define OP_LBU		23
#define OP_LH		24
#define OP_LHU		25
#define OP_LUI		26
#define OP_LW		27
#define OP_LWL		28
#define OP_LWR		29
#define OP_MFHI		31
#define OP_MFLO		32
#define OP_MTHI		34
#define OP_MTLO		35
#define OP_MULT		36
#define OP_MULTU	37
#define OP_NOR		38
#define OP_OR		39
#define OP_ORI		40
#define OP_RFE		41
#define OP_SB		42
#define OP_SH		43
#define OP_SLL		44
#define OP_SLLV		45
#define OP_SLT		46
#define OP_SLTI		47
#define OP_SLTIU	48
#define OP_SLTU		49
#define OP_SRA		50
#define OP_SRAV		51
#define OP_SRL		52
#define OP_SRLV		53
#define OP_SUB		54
#define OP_SUBU		55
#define OP_SW		56
#define OP_SWL		57
#define OP_SWR		58
#define OP_XOR		59
#define OP_XORI		60
#define OP_SYSCALL	61

/* MIPS I floating point instructions (S/D), no fixed point (W) */
#define OP_LWC1         62
#define OP_LDC1         63
#define OP_SWC1         64
#define OP_SDC1         65
#define OP_ABS_S        66
#define OP_ABS_D        67
#define OP_ADD_S        68
#define OP_ADD_D        69
#define OP_DIV_S        70
#define OP_DIV_D        71
#define OP_MUL_S        72
#define OP_MUL_D        73
#define OP_NEG_S        74
#define OP_NEG_D        75
#define OP_SUB_S        76
#define OP_SUB_D        77
#define OP_CVT_S_D      78
#define OP_CVT_S_W      79
#define OP_CVT_W_S      80
#define OP_CVT_W_D      81
#define OP_CVT_D_S      82
#define OP_CVT_D_W      83
#define OP_CEIL_W_S     84
#define OP_CEIL_W_D     85
#define OP_FLOOR_W_S    86
#define OP_FLOOR_W_D    87
#define OP_ROUND_W_S    88
#define OP_ROUND_W_D    89
#define OP_TRUNC_W_S    90
#define OP_TRUNC_W_D    91
#define OP_MOV_S        92
#define OP_MOV_D        93
#define OP_BC1F         94
#define OP_BC1T         95
#define OP_BC1FL        96
#define OP_BC1TL        97
#define OP_SQRT_S       98
#define OP_SQRT_D       99
#define OP_C_F_S        100
#define OP_C_UN_S       101
#define OP_C_EQ_S       102
#define OP_C_UEQ_S      103
#define OP_C_OLT_S      104
#define OP_C_ULT_S      105
#define OP_C_OLE_S      106
#define OP_C_ULE_S      107
#define OP_C_SF_S       108
#define OP_C_NGLE_S     109
#define OP_C_SEQ_S      110
#define OP_C_NGL_S      111
#define OP_C_LT_S       112
#define OP_C_NGE_S      113
#define OP_C_LE_S       114
#define OP_C_NGT_S      115
#define OP_C_F_D        116
#define OP_C_UN_D       117
#define OP_C_EQ_D       118
#define OP_C_UEQ_D      119
#define OP_C_OLT_D      120
#define OP_C_ULT_D      121
#define OP_C_OLE_D      122
#define OP_C_ULE_D      123
#define OP_C_SF_D       124
#define OP_C_NGLE_D     125
#define OP_C_SEQ_D      126
#define OP_C_NGL_D      127
#define OP_C_LT_D       128
#define OP_C_NGE_D      129
#define OP_C_LE_D       130
#define OP_C_NGT_D      131
#define OP_MFC1         132
#define OP_CFC1         133
#define OP_MTC1         134
#define OP_CTC1         135

#define OP_UNIMP	136
#define OP_RES		137

#define MaxOpcode	137

/*
 * Miscellaneous definitions:
 */

#define IndexToAddr(x) ((x) << 2)

// Mask to obtain the sign bit of integers
#define SIGN_BIT	0x80000000

// Constant for return address register
#define R31		31

// Constants used for instruction decoding
#define SPECIAL 140
#define BCOND	141
#define COP1    142
#define IFMT 1
#define JFMT 2
#define RFMT 3

/*!
 * Information related to the opcode of a MIPS instruction
 */
struct OpInfo {
    int opCode;		/*!< Translated op code. */
    int format;		/*!< Format type (IFMT or JFMT or RFMT) */
};

/*!
 * The table is used to translate bits 31:26 of the instruction
 * into a value suitable for the "opCode" field of a MemWord structure,
 * or into a special value (SPECIAL, BCOND, COP1) for further decoding.
 */
static OpInfo opTable[] = {
    {SPECIAL, RFMT}, {BCOND, IFMT}, {OP_J, JFMT}, {OP_JAL, JFMT},
    {OP_BEQ, IFMT}, {OP_BNE, IFMT}, {OP_BLEZ, IFMT}, {OP_BGTZ, IFMT},
    {OP_ADDI, IFMT}, {OP_ADDIU, IFMT}, {OP_SLTI, IFMT}, {OP_SLTIU, IFMT},
    {OP_ANDI, IFMT}, {OP_ORI, IFMT}, {OP_XORI, IFMT}, {OP_LUI, IFMT},
    {OP_UNIMP, IFMT}, {COP1, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_LB, IFMT}, {OP_LH, IFMT}, {OP_LWL, IFMT}, {OP_LW, IFMT},
    {OP_LBU, IFMT}, {OP_LHU, IFMT}, {OP_LWR, IFMT}, {OP_RES, IFMT},
    {OP_SB, IFMT}, {OP_SH, IFMT}, {OP_SWL, IFMT}, {OP_SW, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_SWR, IFMT}, {OP_RES, IFMT},
    {OP_UNIMP, IFMT}, {OP_LWC1, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT},
    {OP_RES, IFMT}, {OP_LDC1, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_UNIMP, IFMT}, {OP_SWC1, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT},
    {OP_RES, IFMT}, {OP_SDC1, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}
};

/*!
 * The table below is used to convert the "funct" field of SPECIAL
 * instructions into the "opCode" field of a MemWord.
 */

static int specialTable[] = {
    OP_SLL, OP_RES, OP_SRL, OP_SRA, OP_SLLV, OP_RES, OP_SRLV, OP_SRAV,
    OP_JR, OP_JALR, OP_RES, OP_RES, OP_SYSCALL, OP_UNIMP, OP_RES, OP_RES,
    OP_MFHI, OP_MTHI, OP_MFLO, OP_MTLO, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_MULT, OP_MULTU, OP_DIV, OP_DIVU, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_ADD, OP_ADDU, OP_SUB, OP_SUBU, OP_AND, OP_OR, OP_XOR, OP_NOR,
    OP_RES, OP_RES, OP_SLT, OP_SLTU, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES
};


/* !
 * The table below is used to convert the "function" field of COP1
 * instructions for RS=S (single precision floating point instructions) 
 * into the "Opcode" field of a MemWord.
 */

static int cop1STable [] = {
    OP_ADD_S, OP_SUB_S, OP_MUL_S, OP_DIV_S, 
    OP_SQRT_S, OP_ABS_S, OP_MOV_S, OP_NEG_S,
    OP_RES, OP_RES, OP_RES, OP_RES, 
    OP_ROUND_W_S, OP_TRUNC_W_S, OP_CEIL_W_S, OP_FLOOR_W_S,
    OP_RES, OP_UNIMP, OP_UNIMP, OP_UNIMP, 
    OP_RES, OP_RES, OP_RES, OP_RES, 
    OP_RES, OP_RES, OP_RES, OP_RES, 
    OP_RES, OP_RES, OP_RES, OP_RES,
    OP_RES, OP_CVT_D_S, OP_RES, OP_RES, 
    OP_CVT_W_S, OP_RES, OP_RES, OP_RES,
    OP_RES, OP_RES, OP_RES, OP_RES, 
    OP_RES, OP_RES, OP_RES, OP_RES,
    OP_C_F_S, OP_C_UN_S, OP_C_EQ_S, OP_C_UEQ_S,
    OP_C_OLT_S,  OP_C_ULT_S, OP_C_OLE_S,  OP_C_ULE_S,
    OP_C_SF_S, OP_C_NGLE_S, OP_C_SEQ_S, OP_C_NGL_S,
    OP_C_LT_S, OP_C_NGE_S, OP_C_LE_S, OP_C_NGT_S
};

/* !
 * The table below is used to convert the "function" field of COP1
 * instructions for RS=D (single precision floating point instructions)
 * into the "Opcode" field of a MemWord.
 */

static int cop1DTable [] = {
    OP_ADD_D, OP_SUB_D, OP_MUL_D, OP_DIV_D, 
    OP_SQRT_D, OP_ABS_D, OP_MOV_D, OP_NEG_D,
    OP_RES, OP_RES, OP_RES, OP_RES, 
    OP_ROUND_W_D, OP_TRUNC_W_D, OP_CEIL_W_D, OP_FLOOR_W_D,
    OP_RES, OP_UNIMP, OP_UNIMP, OP_UNIMP, 
    OP_RES, OP_RES, OP_RES, OP_RES, 
    OP_RES, OP_RES, OP_RES, OP_RES, 
    OP_RES, OP_RES, OP_RES, OP_RES,
    OP_CVT_S_D, OP_RES, OP_RES, OP_RES, 
    OP_CVT_W_D, OP_RES, OP_RES, OP_RES,
    OP_RES, OP_RES, OP_RES, OP_RES, 
    OP_RES, OP_RES, OP_RES, OP_RES,
    OP_C_F_D, OP_C_UN_D, OP_C_EQ_D, OP_C_UEQ_D,
    OP_C_OLT_D, OP_C_ULT_D, OP_C_OLE_D, OP_C_ULE_D,
    OP_C_SF_D, OP_C_NGLE_D, OP_C_SEQ_D, OP_C_NGL_D,
    OP_C_LT_D, OP_C_NGE_D, OP_C_LE_D, OP_C_NGT_D
};

//! Stuff to help print out each instruction, for debugging
enum RegType { NONE, RS, RT, RD, FS, FT, FD, EXTRA }; 

//! Textual representation of Mips instructions and their operands
struct OpString {
    char *string;	//!< Printed version of instruction
    RegType args[3];
};

//! Textual form of instructions
static struct OpString opStrings[] = {
	{(char*)"Shouldn't happen", {NONE, NONE, NONE}},
	{(char*)"ADD r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"ADDI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char*)"ADDIU r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char*)"ADDU r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"AND r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"ANDI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char*)"BEQ r%d,r%d,%d", {RS, RT, EXTRA}},
	{(char*)"BGEZ r%d,%d", {RS, EXTRA, NONE}},
	{(char*)"BGEZAL r%d,%d", {RS, EXTRA, NONE}},
	{(char*)"BGTZ r%d,%d", {RS, EXTRA, NONE}},
	{(char*)"BLEZ r%d,%d", {RS, EXTRA, NONE}},
	{(char*)"BLTZ r%d,%d", {RS, EXTRA, NONE}},
	{(char*)"BLTZAL r%d,%d", {RS, EXTRA, NONE}},
	{(char*)"BNE r%d,r%d,%d", {RS, RT, EXTRA}},
	{(char*)"Shouldn't happen", {NONE, NONE, NONE}},
	{(char*)"DIV r%d,r%d", {RS, RT, NONE}},
	{(char*)"DIVU r%d,r%d", {RS, RT, NONE}},
	{(char*)"J 0x%x", {EXTRA, NONE, NONE}},
	{(char*)"JAL 0x%x", {EXTRA, NONE, NONE}},
	{(char*)"JALR r%d,r%d", {RD, RS, NONE}},
	{(char*)"JR r%d,r%d", {RD, RS, NONE}},
	{(char*)"LB r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"LBU r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"LH r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"LHU r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"LUI r%d,%d", {RT, EXTRA, NONE}},
	{(char*)"LW r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"LWL r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"LWR r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"Shouldn't happen", {NONE, NONE, NONE}},
	{(char*)"MFHI r%d", {RD, NONE, NONE}},
	{(char*)"MFLO r%d", {RD, NONE, NONE}},
	{(char*)"Shouldn't happen", {NONE, NONE, NONE}},
	{(char*)"MTHI r%d", {RS, NONE, NONE}},
	{(char*)"MTLO r%d", {RS, NONE, NONE}},
	{(char*)"MULT r%d,r%d", {RS, RT, NONE}},
	{(char*)"MULTU r%d,r%d", {RS, RT, NONE}},
	{(char*)"NOR r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"OR r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"ORI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char*)"RFE", {NONE, NONE, NONE}},
	{(char*)"SB r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"SH r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"SLL r%d,r%d,%d", {RD, RT, EXTRA}},
	{(char*)"SLLV r%d,r%d,r%d", {RD, RT, RS}},
	{(char*)"SLT r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"SLTI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char*)"SLTIU r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char*)"SLTU r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"SRA r%d,r%d,%d", {RD, RT, EXTRA}},
	{(char*)"SRAV r%d,r%d,r%d", {RD, RT, RS}},
	{(char*)"SRL r%d,r%d,%d", {RD, RT, EXTRA}},
	{(char*)"SRLV r%d,r%d,r%d", {RD, RT, RS}},
	{(char*)"SUB r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"SUBU r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"SW r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"SWL r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"SWR r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char*)"XOR r%d,r%d,r%d", {RD, RS, RT}},
	{(char*)"XORI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char*)"SYSCALL", {NONE, NONE, NONE}},

	/* Some of the floating point instructions (MIPS I, no "W" instr) */
	{(char*)"LWC1 f%d,%d(r%d)", {FT, EXTRA, RS}},
	{(char*)"LDC1 f%d,%d(r%d)", {FT, EXTRA, RS}},
	{(char*)"SWC1 f%d,%d(r%d)", {FT, EXTRA, RS}},
	{(char*)"SDC1 f%d,%d(r%d)", {FT, EXTRA, RS}},
	{(char*)"ABS.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"ABS.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"ADD.S f%d,f%d,f%d", {FD, FS, FT}},
	{(char*)"ADD.D f%d,f%d,f%d", {FD, FS, FT}},
	{(char*)"DIV.S f%d,f%d,f%d", {FD, FS, FT}},
	{(char*)"DIV.D f%d,f%d,f%d", {FD, FS, FT}},
	{(char*)"MUL.S f%d,f%d,f%d", {FD, FS, FT}},
	{(char*)"MUL.D f%d,f%d,f%d", {FD, FS, FT}},
	{(char*)"NEG.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"NEG.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"SUB.S f%d,f%d,f%d", {FD, FS, FT}},
	{(char*)"SUB.D f%d,f%d,f%d", {FD, FS, FT}},
	{(char*)"CVT.S.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"CVT.S.W f%d,f%d", {FD, FS, NONE}},
	{(char*)"CVT.W.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"CVT.W.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"CVT.D.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"CVT.D.W f%d,f%d", {FD, FS, NONE}},
	{(char*)"CEIL.W.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"CEIL.W.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"FLOOR.W.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"FLOOR.W.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"ROUND.W.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"ROUND.W.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"TRUNC.W.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"TRUNC.W.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"MOV.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"MOV.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"BC1F %d", {EXTRA, NONE, NONE}},
	{(char*)"BC1T %d", {EXTRA, NONE, NONE}},
	{(char*)"BC1FL %d", {EXTRA, NONE, NONE}},
	{(char*)"BC1TL %d", {EXTRA, NONE, NONE}},
	{(char*)"SQRT.S f%d,f%d", {FD, FS, NONE}},
	{(char*)"SQRT.D f%d,f%d", {FD, FS, NONE}},
	{(char*)"C.F.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.UN.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.EQ.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.UEQ.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.OLT.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.ULT.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.OLE.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.ULE.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.SF.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.NGLE.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.SEQ.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.NGL.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.LT.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.NGE.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.LE.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.NGT.S f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.F.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.UN.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.EQ.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.UEQ.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.OLT.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.ULT.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.OLE.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.ULE.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.SF.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.NGLE.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.SEQ.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.NGL.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.LT.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.NGE.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.LE.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"C.NGT.D f%d,f%d", {FS, FT, NONE}},
	{(char*)"OP_MFC1 r%d,f%d", {RT, FS, NONE}},
        {(char*)"OP_CFC1 r%d,f%d", {RT, FS, NONE}},
	{(char*)"OP_MTC1 r%d,f%d", {RT, FS, NONE}},
        {(char*)"OP_CTC1 r%d,f%d", {RT, FS, NONE}},
	{(char*)"Unimplemented", {NONE, NONE, NONE}},
	{(char*)"Reserved", {NONE, NONE, NONE}}
      };

#endif // MIPSSIM_H
