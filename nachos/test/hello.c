/* hello.c
 *	Simple hello world program
 *
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.  
//  See copyright_insa.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
 */

#include "userlib/syscall.h"
#include "userlib/libnachos.h"

int
main()
{
  n_printf("** ** ** Bonjour le monde ** ** **\n");

  return 0;
}
