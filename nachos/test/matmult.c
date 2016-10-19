/* matmult.c 
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 *
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.  
//  See copyright_insa.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
 */

#include "userlib/syscall.h"

#define Dim 	10	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

/* The matrices to be filled-in and multiplied */
int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int
main()
{
    int i, j, k;

    Write("Start matmult\n",14,ConsoleOutput);
    
    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];

    /*
      // Print the result
    for (i=0;i<Dim;i++)
      for (j=0;j<Dim;j++) {
	WriteInt(C[i][j]);
	Write("  ",2,ConsoleOutput);
      }
    */

    //Write("End matmult\n",12,ConsoleOutput);
    Exit(C[Dim-1][Dim-1]);		/* and then we're done */

    return 0;
}
