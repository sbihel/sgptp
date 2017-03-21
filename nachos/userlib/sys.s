/* Start.s 
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 *
 *  Copyright (c) 1992-1993 The Regents of the University of California.
 *  All rights reserved.  See copyright.h for copyright notice and limitation 
 *  of liability and disclaimer of warranty provisions.
 */

#define IN_ASM
#include "userlib/syscall.h"

	 // Equivalent to ".text", but with a different name, in order
	 // to be correctly handled by the ldscript
        .section .sys,"ax",@progbits
	
        .align  2

/* -------------------------------------------------------------
 * __start
 *	Initialize running a C program, by calling "main". 
 *
 * 	NOTE: This has to be first, so that it gets loaded at location 0.
 *	The Nachos kernel always starts a program by jumping to location 0.
 * -------------------------------------------------------------
 */

	.globl __start
	.ent	__start
__start:

/* Call the program entry point */
	jal	main
	move	$4,$0		
	jal	Exit	 /* if we return from main, exit(0) */
	.end __start

/* -------------------------------------------------------------
 * System call stubs:
 *	Assembly language assist to make system calls to the Nachos kernel.
 *	There is one stub per system call, that places the code for the
 *	system call into register r2, and leaves the arguments to the
 *	system call alone (in other words, arg1 is in r4, arg2 is 
 *	in r5, arg3 is in r6, arg4 is in r7)
 *
 * 	The return value is in r2. This follows the standard C calling
 * 	convention on the MIPS.
 * -------------------------------------------------------------
 */

	.globl Halt
	.ent	Halt
Halt:
	addiu $2,$0,SC_HALT
	syscall
	j	$31
	.end Halt

	.globl Exit
	.ent	Exit
Exit:
	addiu $2,$0,SC_EXIT
	syscall
	j	$31
	.end Exit

	.globl Exec
	.ent	Exec
Exec:
	addiu $2,$0,SC_EXEC
	syscall
	j	$31
	.end Exec

	.globl Join
	.ent	Join
Join:
	addiu $2,$0,SC_JOIN
	syscall
	j	$31
	.end Join

	.globl Create
	.ent	Create
Create:
	addiu $2,$0,SC_CREATE
	syscall
	j	$31
	.end Create

	.globl Open
	.ent	Open
Open:
	addiu $2,$0,SC_OPEN
	syscall
	j	$31
	.end Open

	.globl Read
	.ent	Read
Read:
	addiu $2,$0,SC_READ
	syscall
	j	$31
	.end Read

	.globl Write
	.ent	Write
Write:
	addiu $2,$0,SC_WRITE
	syscall
	j	$31
	.end Write

	.globl Seek
	.ent	Seek
Seek:
	addiu $2,$0,SC_SEEK
	syscall
	j	$31
	.end Seek

	.globl Close
	.ent	Close
Close:
	addiu $2,$0,SC_CLOSE
	syscall
	j	$31
	.end Close

	.globl FSList
	.ent	FSList
FSList:
	addiu $2,$0,SC_FSLIST
	syscall
	j	$31
	.end FSList

	.globl newThread
	.ent	newThread
newThread:
	addiu $2,$0,SC_NEW_THREAD
	syscall
	j	$31
	.end newThread
	
	.globl Remove
	.ent	Remove
Remove:
	addiu $2,$0,SC_REMOVE
	syscall
	j	$31
	.end Remove

	.globl Yield
	.ent	Yield
Yield:
	addiu $2,$0,SC_YIELD
	syscall
	j	$31
	.end Yield
	
	.globl PError
	.ent	PError
PError:	
	addiu $2,$0,SC_PERROR
	syscall
	j	$31
	.end PError


	.globl P
	.ent	P
P:
	addiu $2,$0,SC_P
	syscall
	j	$31
	.end P
	
	.globl V
	.ent	V
V:	
	addiu $2,$0,SC_V
	syscall
	j	$31
	.end V
	
	.globl SemCreate
	.ent	SemCreate
SemCreate:	
	addiu $2,$0,SC_SEM_CREATE
	syscall
	j	$31
	.end SemCreate

	.globl SemDestroy
	.ent	SemDestroy
SemDestroy:
	addiu $2,$0,SC_SEM_DESTROY
	syscall
	j	$31
	.end SemDestroy

	.globl SysTime
	.ent	SysTime
SysTime:
	addiu $2,$0,SC_SYS_TIME
	syscall
	j	$31
	.end SysTime

	.globl LockCreate
	.ent	LockCreate
LockCreate:
	addiu $2,$0,SC_LOCK_CREATE
	syscall
	j	$31
	.end LockCreate

	.globl LockDestroy
	.ent	LockDestroy
LockDestroy:	
	addiu $2,$0,SC_LOCK_DESTROY
	syscall
	j	$31
	.end LockDestroy
		
	.globl LockAcquire
	.ent	LockAcquire
LockAcquire:
	addiu $2,$0,SC_LOCK_ACQUIRE
	syscall
	j	$31
	.end LockAcquire

	.globl LockRelease
	.ent	LockRelease
LockRelease:
	addiu $2,$0,SC_LOCK_RELEASE
	syscall
	j	$31
	.end LockRelease

	.globl CondCreate
	.ent	CondCreate
CondCreate:
	addiu $2,$0,SC_COND_CREATE
	syscall
	j	$31
	.end CondCreate

	.globl CondDestroy
	.ent	CondDestroy
CondDestroy:	
	addiu $2,$0,SC_COND_DESTROY
	syscall
	j	$31
	.end CondDestroy


	.globl CondWait
	.ent	CondWait
CondWait:	
	addiu $2,$0,SC_COND_WAIT
	syscall
	j	$31
	.end CondWait

	.globl CondSignal
	.ent	CondSignal
CondSignal:	
	addiu $2,$0,SC_COND_SIGNAL
	syscall
	j	$31
	.end CondSignal

	.globl CondBroadcast
	.ent	CondBroadcast
CondBroadcast:	
	addiu $2,$0,SC_COND_BROADCAST
	syscall
	j	$31
	.end CondBroadcast

	.globl TtySend
	.ent	TtySend
TtySend:	
	addiu $2,$0,SC_TTY_SEND
	syscall
	j	$31
	.end TtySend

	.globl TtyReceive
	.ent	TtyReceive
TtyReceive:	
	addiu $2,$0,SC_TTY_RECEIVE
	syscall
	j	$31
	.end TtyReceive
	
	.globl Mkdir
	.ent	Mkdir
Mkdir:	addiu $2,$0,SC_MKDIR
	syscall
	j	$31
	.end Mkdir
	
	.globl Rmdir
	.ent	Rmdir
Rmdir:	addiu $2,$0,SC_RMDIR
	syscall
	j	$31
	.end Rmdir
	
	.globl Mmap
	.ent	Mmap
Mmap:	addiu $2,$0,SC_MMAP
	syscall
	j	$31
	.end Mmap
