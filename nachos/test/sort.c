/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.  
//  See copyright_insa.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
*/

// Nachos system calls
#include "userlib/syscall.h"

// Table to be sorted
#define NUM 30
int A[NUM];	

int
main()
{
    int i, j, key;

    Write("Start sort\n",11,ConsoleOutput);
    
    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < NUM; i++)		
        A[i] = NUM - i;


    for (i = 0; i < NUM; i++) {
      n_printf("%d ",A[i]);
    }
    n_printf("\n");
    
    /* Write("\n\n",2,ConsoleOutput);*/

    /* then sort! */
    for (j=1;j<NUM;j++) {
      key = A[j];
      i = j - 1;
      while (i>= 0 && A[i] > key) {
	A[i+1] = A[i];
	i--;
      }
      A[i+1] = key;
    }

    for (i = 0; i < NUM; i++) {
      n_printf("%d ",A[i]);
    }
    n_printf("\n");
   

    Write("End sort\n",9,ConsoleOutput);
    Exit(A[0]);		/* and then we're done -- should be 0! */

    return 0;
}
