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

void dump_args(int argc, char *argv[]) {
  int i;
  n_printf("%d\n", argc);
  for(i = 0; i < argc; i++) {
    n_printf("%s ", argv[i]);
  }
  n_printf("\n");
}

int main() {
  char *argv[3] = {"Bonjour", "le", "monde"};

  threadCreate2("dump_args", dump_args, 3, argv);

  return 0;
}
