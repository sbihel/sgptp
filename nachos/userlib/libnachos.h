/*! \file libnachos.h
    \brief Function structures for programs 
   
  	Libnachos proposes several 'libc-like' functions
	for:
  		Input-Output operations,
  		String operations,
  		Memory operations,
        System calls are defined in kernel/syscalls.h

	Nachos-libc functions are prefixed by 'n' to avoid
	any confusion with standard libc functions.
  
    Copyright (c) 1999-2000 INSA de Rennes.
    All rights reserved.  
    See copyright_insa.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
*/

#include "userlib/syscall.h"

typedef void (*VoidNoArgFunctionPtr)(); 
typedef unsigned int size_t;

// Thread management
// ----------------------------
ThreadId threadCreate(char * debug_name, VoidNoArgFunctionPtr func);

// Input/Output operations :
// ------------------------------------

// Print on the standard output specified parameters.
void n_printf(const char *format, ...);

// Format <buff> (of max length <len>) according to the format <format>
int n_snprintf(char * buff, int len, const char *format, ...);

// Read an integer on the standard input
int n_read_int(void);

// String operations :
// -------------------

// Compare two strings byte by byte.
int n_strcmp(const char *s1, const char *s2);

// Copy a string.
char* n_strcpy(char *dst, const char *src);

// Return the number of bytes in a string.
size_t n_strlen(const char *s);

// appends a copy of a string, to the end of another string.
char* n_strcat(char *dst, const char *src);

// Return a upper-case letter, 
// equivalent to the lower-case letter given. 
int n_toupper(int c);

// Return a lower-case letter, 
// equivalent to the upper-case letter given. 
int n_tolower(int c);

// Convert a string in integer.
int n_atoi(const char *str);

// Concerning memory area operations :
// -----------------------------------

// Compare two memory area, looking at the first n bytes .
int n_memcmp(const void *s1, const void *s2, size_t n);

// Copy n byte from an memory area to another.
void* n_memcpy(void *s1, const void *s2, size_t n);

// Set the first n bytes in a memory area to a specified value.
void* n_memset(void *s, int c, size_t n);
