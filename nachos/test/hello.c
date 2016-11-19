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

typedef void (*VoidFunctionPtr)();

typedef struct {
  VoidFunctionPtr func;
  int argc;
  char **argv;
} FunArgs;

static void threadStart(int fa) {
  VoidFunctionPtr func = ((FunArgs*)fa)->func;
  int argc = ((FunArgs*)fa)->argc;
  char **argv = ((FunArgs*)fa)->argv;

  VoidFunctionPtr func2;
  func2 = (VoidFunctionPtr)func;
  func2(argc, argv);

  Exit(0);
}

ThreadId threadCreate2(char *debug_name, VoidFunctionPtr func, int argc, char *argv[]) {
  FunArgs fa;
  fa.func = func;
  fa.argc = argc;
  fa.argv = argv;

  return newThread(debug_name, (int)threadStart, (int)(&fa));
}

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
