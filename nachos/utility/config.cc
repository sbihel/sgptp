/*! \file config.cc
    \brief Routines for setting up the Nachos hardware and software confuguration
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.  
//  See copyright_insa.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
*/
#ifndef CONFIG_CPP
#define CONFIG_CPP

#include <stdio.h>
#include "machine/machine.h"
#include "utility/config.h"
#include "utility/utility.h"
#include "filesys/directory.h"
#include "kernel/system.h"

#define LINE_LENGTH     256
#define COMMAND_LENGTH  80

#define power_of_two(size) (((size) & ((size)-1)) == 0)

void fail(int numligne,char *name,char *ligne)
{
  ligne[strlen(ligne)-1] = '\0';
  printf("Config Error : File %s line %d ---> \"%s\"\n",name,numligne,ligne);
  exit(-1);
}

/**
 * Fill-in the configuration object from configuration information stored in a file
 *
 * \param configname: file name of the configuration file
 */
Config::Config(char *configname) {

  // Default values for configuration parameters
  SectorSize=128;
  PageSize=128;
  NumPhysPages=20;
  MaxVirtPages=1024;
  UserStackSize=8*1024;
  ProcessorFrequency = 100;
  MaxFileNameSize=256;
  NbCopy=0;
  NumDirEntries=10;
  NumPortLoc=32009;
  NumPortDist=32009;
  PrintStat=false;
  FormatDisk=false;
  ListDir=false;
  PrintFileSyst=false;
  Print=false;
  Remove=false;
  MakeDir=false;
  RemoveDir=false;
  ACIA=ACIA_NONE;
  strcpy(ProgramToRun,"");

  int nblignes=0;

  DEBUG('u',(char *)"Reading the configuration file\n");
  char ligne[LINE_LENGTH];
  char commande[COMMAND_LENGTH];
  
  cfg = fopen(configname,"r");

  if (cfg == (FILE *) NULL) {
    printf("Error: can't open file %s\n",configname);
    exit(-1);
  }
  
  while (!feof(cfg)) {
    
    fgets(ligne,LINE_LENGTH,cfg);
    nblignes++;
    if ((ligne[0] != '#')&&(strlen(ligne)!=0)) {

            // Accepted NULL lines
      if (strcmp(ligne,"") == 0) continue;
      if (strcmp(ligne,"\n") == 0) continue;

      if (sscanf(ligne," %s ",commande) != 1) fail(nblignes,configname,ligne);
      if (strlen(commande)!= 0) {
	
	if (strcmp(commande,"ProcessorFrequency") == 0) {
	  if(sscanf(ligne," %s = %i ",commande,&ProcessorFrequency)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}

	if (strcmp(commande,"NumPhysPages") == 0) {
	  if(sscanf(ligne," %s = %i ",commande,&NumPhysPages)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}
	
	if (strcmp(commande,"MaxVirtPages") == 0) {
	  if(sscanf(ligne," %s = %i ",commande,&MaxVirtPages)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}
	if (strcmp(commande,"SectorSize") == 0) {
	  if(sscanf(ligne," %s = %i ",commande,&SectorSize)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}
	if (strcmp(commande,"PageSize") == 0) {
	  if(sscanf(ligne," %s = %i ",commande,&PageSize)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}
	if (strcmp(commande,"UserStackSize") == 0) {
	  if(sscanf(ligne," %s = %i ",commande,&UserStackSize)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}
	if (strcmp(commande,"MaxFileNameSize") == 0) {
	  if(sscanf(ligne," %s = %i ",commande,&MaxFileNameSize)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}

	if (strcmp(commande,"TargetMachineName") == 0) {
	  if(sscanf(ligne," %s = %s ",commande,TargetMachineName)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}
	
	if (strcmp(commande,"ProgramToRun") == 0) {
	  if(sscanf(ligne," %s = %s ",commande,ProgramToRun)!=2)
	    fail(nblignes,configname,ligne);
	  continue;
	}
	
	if (strcmp(commande,"PrintStat") == 0){
	  int v;
	  if(sscanf(ligne," %s = %i ",commande,&v)==2)
	    {
	      PrintStat=(bool)v;
	      if (v==0)
		PrintStat = false;
	      else 
		PrintStat = true;
	    }
	  else fail(nblignes,configname,ligne);
	  continue;
	}

	if (strcmp(commande,"FormatDisk") == 0){
	  int v;
	  if(sscanf(ligne," %s = %i ",commande,&v)==2)
	    {
	      if (v==0)
		FormatDisk = false;
	      else 
		FormatDisk = true;	 
	    }
	  else fail(nblignes,configname,ligne);
	  continue;
	}

      if (strcmp(commande,"ListDir") == 0){
	int v;
	  if(sscanf(ligne," %s = %i ",commande,&v)==2)
	    {
	      if (v==0)
		ListDir = false;
	      else 
		ListDir = true;
	    }
	  else fail(nblignes,configname,ligne);
	  continue;
      }
      
      if (strcmp(commande,"PrintFileSyst") == 0){
	int v;
	if(sscanf(ligne," %s = %i ",commande,&v)==2)
	  {
	    if (v==0)
	      PrintFileSyst = false;
	    else 
	      PrintFileSyst = true;
	  }
	  else fail(nblignes,configname,ligne);
	continue;
      }

      if (strcmp(commande,"FileToCopy") == 0){
	if(sscanf(ligne," %s = %s %s",commande,ToCopyUnix[NbCopy],ToCopyNachos[NbCopy])==3)
	  NbCopy++;
	else fail(nblignes,configname,ligne);
	continue;
      }
      
      if (strcmp(commande,"FileToPrint") == 0){
	if(sscanf(ligne," %s = %s ",commande,FileToPrint)==2)
	  Print=true;
       	else fail(nblignes,configname,ligne);
	continue;
      }
	
      if (strcmp(commande,"FileToRemove") == 0){	
	if(sscanf(ligne," %s = %s ",commande,FileToRemove)==2)
	  Remove=true;
	else fail(nblignes,configname,ligne);	
	continue;
      }

      if (strcmp(commande,"DirToMake") == 0){
	  if(sscanf(ligne," %s = %s ",commande,DirToMake)==2)
	    MakeDir=true;
	  else fail(nblignes,configname,ligne);
	  continue;
      }
      
      if (strcmp(commande,"DirToRemove") == 0){

	if(sscanf(ligne," %s = %s ",commande,DirToRemove)==2)
	  RemoveDir=true;
	else fail(nblignes,configname,ligne);
	continue;
      }

      if (strcmp(commande,"NumDirEntries") == 0){
	if(sscanf(ligne," %s = %i ",commande,&NumDirEntries)!=2)
	  fail(nblignes,configname,ligne);
	continue;
      }
	
      if (strcmp(commande,"UseACIA") == 0){
	char acia_mode[LINE_LENGTH];
	if (sscanf(ligne," %s = %s ",commande,acia_mode)==2) {
	  if (strcmp(acia_mode,"None")==0)
	    ACIA = ACIA_NONE;
	  else if (strcmp(acia_mode,"BusyWaiting")==0)
	    ACIA = ACIA_BUSY_WAITING;
	  else if (strcmp(acia_mode,"Interrupt")==0)
	    ACIA = ACIA_INTERRUPT;
	  else fail(nblignes,configname,ligne);
	}	
	else fail(nblignes,configname,ligne);
	continue;
      }
      
      if (strcmp(commande,"NumPortLoc") == 0){
	if(sscanf(ligne," %s = %i ",commande,&NumPortLoc)!=2)
	  fail(nblignes,configname,ligne);
	continue;
      }
      
      if (strcmp(commande,"NumPortDist") == 0){
	if(sscanf(ligne," %s = %i ",commande,&NumPortDist)!=2)
	  fail(nblignes,configname,ligne);
	continue;
      }

      // Autres variables -> non reconnues
      fail(nblignes,configname,commande);
	
      }
      strcpy(commande,"\0");
      
    }
  }

  fclose(cfg);

  if (PageSize != SectorSize) {
    printf("Warning, PageSize<>SectorSize, setting both to %d\n",SectorSize);
    PageSize   = SectorSize;
  }

  // Check that sector size and page sizes are powers of two
  if (!power_of_two(SectorSize)) {
    printf("Configuration error : SectorSize should be a power of two, exiting\n");
    exit(-1);
  }

  NumDirect = ((SectorSize - 4 * sizeof(int)) / sizeof(int));
  //MaxFileSize = (NumDirect * SectorSize);
  MagicNumber = 0x456789ab;
  MagicSize = sizeof(int);
  DiskSize = (MagicSize + (NUM_SECTORS * SectorSize));
  DirectoryFileSize=(sizeof(DirectoryEntry) * NumDirEntries);
  DEBUG('u',(char *)"End of reading of configuration file\n");
}

#endif // CONFIG_CPP
