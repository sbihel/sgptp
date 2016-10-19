/*
//  shell.c: a very simple shell running on Nachos
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.  
//  See copyright_insa.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
*/

// Nachos system calls
#include "userlib/libnachos.h"

int
main()
{
    ThreadId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[2], buffer[60];
    int i,bg;

    prompt[0] = '-';
    prompt[1] = '>';

    // Welcome message
    n_printf("Welcome to NachOS\n");

    while( 1 )
    {

      // Write the prompt
      Write(prompt, 2, output);
 
      // Wait for a command
      Read(buffer, 60, input); 
      
	i=0;
	bg=0;
	while(buffer[i++] != '\n' ) {};
	buffer[--i]=' ';
	
	
	
	while(buffer[--i] == ' ') {};
	
	
	// Background execution
	if (buffer[i]=='&') {
	    bg=1;
	    buffer[i]=' ';
	    while(buffer[--i] == ' ') {};
	    buffer[++i]='\0';
	    
	} else {buffer[++i] = '\0';}
	
		
	// Execute the command
	// In the case it is a background command, don't wait for its completion
	if( i > 0 ) {
	  newProc = Exec(buffer);
	  if (newProc == -1) {
	    n_printf("\nUnable to run %s\n", buffer);
	  }
	  else if (!bg) Join(newProc);

	}
    }
}

