/*! \file config.h
    \brief Data structures for setting up the Nachos hardware and
	software configuration
  
    Copyright (c) 1999-2000 INSA de Rennes.
    All rights reserved.  
    See copyright_insa.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
*/
#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include "machine/translationtable.h"
#include "filesys/filesys.h"

#define MAXSTRLEN      100
#define CONFIGFILENAME "nachos.cfg"

/* Running modes of the ACIA */
#define ACIA_NONE 0
#define ACIA_BUSY_WAITING 1
#define ACIA_INTERRUPT 2

/*! \brief Defines Nachos hardware and software configuration 
*
* Used to avoid recompiling Nachos when a change in the configuration
* is needed (turning on/of debug flags, changing memory size, etc.).
* There is a default value for every configuration parameter when not
* specified in the configuration file (see file config.cc)
*/
class Config {

 public:

  // Hardware configuration
  int PageSize;            //!< Page size in bytes
  int NumPhysPages;        //!< Number of pages in the memory of the simulated MIPS machine
  int SectorSize;          //!< Disk sector size in bytes (should be equal to the page size)
  int ProcessorFrequency;  //!< Frequency of the processor (MHz) used to obtain execution time statistics
  int DiskSize;            //!< Total size of the disk (number of sectors)
  int ACIA;                //!< Use ACIA if USE_ACIA, don't use it if ACIA_NONE

  // File system configuration
  int NumDirect;           //!< Number of data sectors storable in the first header sector
  int MaxFileSize;         //!< Maximum length of a file
  int MaxFileNameSize;     //!< Maximum length of a file name (absolute, path included)
  int NumDirEntries;       //!< Maximum number of files in a directory
  int DirectoryFileSize;   //!< Length of a directory file
  int NumPortLoc;	   //!< Local ACIA's port number
  int NumPortDist;	   //!< Distant ACIA's port number
  char TargetMachineName[MAXSTRLEN];     //!< The name of the target machine for the ACIA

  // Kernel (process and address space) configuration
  int MaxVirtPages;        //!< Maximum number of virtual pages in each address space (used to allocate the page table)
  bool TimeSharing;        //!< Use the time sharing mode if true (1) - not implemented in the base code
  int MagicNumber;         //!< 0x456789ab
  int MagicSize;           //!< Size of an integer 
  int UserStackSize;       //!< Stack size of user threads in bytes

  // Configuration of actions to be done when Nachos is started and exited
  int NbCopy;              //!< Number of files to copy
  bool ListDir;            //!< List all the files and directories if true
  bool PrintFileSyst;      //!< Print all the files in the file system if true
  bool PrintStat;          //!< Print the statistics if true
  bool FormatDisk;         //!< Format the disk if true
  bool Print;              //!< Print  FileToPrint if true
  bool Remove;             //!< Remove FileToRemove if true
  bool MakeDir;            //!< Make DirToMake if true
  bool RemoveDir;          //!< remove DirToRemove if true
  char ToCopyUnix[100][MAXSTRLEN];       //!< The table of files to copy from the UNIX filesystem
  char ToCopyNachos [100][MAXSTRLEN];    //!< The table of files to copy to the nachos filesystem
  char ProgramToRun[MAXSTRLEN];          //!< The name of the program to execute
  char FileToPrint[MAXSTRLEN];           //!< The name of the file to print
  char FileToRemove[MAXSTRLEN];          //!< The name of the file to remove
  char DirToMake[MAXSTRLEN];             //!< The name of the directory to make
  char DirToRemove[MAXSTRLEN];           //!< The name of the directory to remove

  /**
   * Fill-in the configuration object from configuration information stored in a file
   *
   * \param configname: file name of the configuration file
   */
  Config(char *configname);

  /**
   * Destructor
   */
  ~Config(){;}
  
 private:
  
  /** File descriptor of the configuration file */
  FILE *cfg;

  /** File name of the configuration file */
  char *name;
};

#endif // CONFIG_H
