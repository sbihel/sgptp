/*! \file utility.cc 
//  \brief Debugging routines.  
//
//      Allows users to control whether to 
//	print DEBUG statements, based on a command line argument.
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.


#include "utility/utility.h"

// this seems to be dependent on how the compiler is configured.
// if you have problems with va_start, try both of these alternatives

#include <stdarg.h>

static char *enableFlags = NULL; // controls which DEBUG messages are printed


//----------------------------------------------------------------------
// DumpMem
/*!     Prints the raw contents of a memory area, each byte as hex digits
//
// 	\param addr the start address of the area (in the host address space)
//      \param len  the size of the area (in bytes)
*/
//----------------------------------------------------------------------
void
DumpMem(char *addr, int len)
{
#define TOHEX(x) \
  ({ char __x = (x); if(__x < 10) __x+='0'; else __x='a'+(__x-10) ; __x; })

  int i;
  for (i = 0 ; i < len ; i++)
    {
      char s[3];
      if ((i%16) == 0)
	printf("%08lx  ", (unsigned long)&addr[i]);
      else if ((i%8) == 0)
	printf("   ");
      s[0] = TOHEX((addr[i] >> 4) & 0xf);
      s[1] = TOHEX(addr[i] & 0xf);
      s[2] = '\0';
      printf("%s ", s);
      if ((((i+1)%16) == 0) || (i == len-1))
	printf("\n");
    }
}



//----------------------------------------------------------------------
// DebugInit
/*!      Initialize so that only DEBUG messages with a flag in flagList 
//	will be printed.
//
//	If the flag is "+", we enable all DEBUG messages.
//
// 	\param flagList is a string of characters for whose DEBUG messages are 
//		to be enabled.
*/
//----------------------------------------------------------------------
void
DebugInit(char *flagList)
{
    enableFlags = flagList;
}

//----------------------------------------------------------------------
// DebugIsEnabled
/*!     \return TRUE if DEBUG messages with "flag" are to be printed.
*/
//----------------------------------------------------------------------

bool
DebugIsEnabled(char flag)
{
    if (enableFlags != NULL)
       return (strchr(enableFlags, flag) != 0) 
		|| (strchr(enableFlags, '+') != 0);
    else
      return false;
}

//----------------------------------------------------------------------
// DEBUG
/*!      Print a debug message, if flag is enabled.  Like printf,
//	only with an extra argument on the front.
*/
//----------------------------------------------------------------------

void 
DEBUG(char flag, char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  
  if (DebugIsEnabled(flag)) {
    // You will get an unused variable message here -- ignore it.
    vfprintf(stdout, format, ap);
    fflush(stdout);
  }

  va_end(ap);
}
