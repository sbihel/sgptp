/*! \file  sysdep.cc
//  \brief Implementation of system-dependent interface
//
//      Nachos uses the 
//	routines defined here, rather than directly calling the UNIX library,
//	to simplify porting between versions of UNIX, and even to
//	other systems, such as MSDOS.
//
//	On UNIX, almost all of these routines are simple wrappers
//	for the underlying UNIX system calls.
//
//	NOTE: all of these routines refer to operations on the underlying
//	host machine (e.g., the DECstation, SPARC, etc.), supporting the 
//	Nachos simulation code.  Nachos implements similar operations,
//	(such as opening a file), but those are implemented in terms
//	of hardware devices, which are simulated by calls to the underlying
//	routines in the host workstation OS.
//
//	This file includes lots of calls to C routines.  C++ requires
//	us to wrap all C definitions with a "extern "C" block".
// 	This prevents the internal forms of the names from being
// 	changed by the C++ compiler.
*/
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "utility/config.h"
#include "machine/machine.h"
#include "machine/interrupt.h"
#include "kernel/system.h"

extern "C" {
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>

// Port Number for sockets

//#define N_PORT 32009

// UNIX routines called by procedures in this file 

// int creat(char *name, unsigned short mode);
// int open(const char *name, int flags, ...);

//int unlink(char *name);
//int read(int filedes, char *buf, int numBytes);
//int write(int filedes, char *buf, int numBytes);
//int lseek(int filedes, int offset, int whence);
int tell(int filedes);
int close(int filedes);
  //int unlink(char *name);

// definition varies slightly from platform to platform, so don't 
// define unless gcc complains
// extern int recvfrom(int s, void *buf, int len, int flags, void *from, int *fromlen);
// extern int sendto(int s, void *msg, int len, int flags, void *to, int tolen);
//int bind (int,struct sockaddr *, int);
//int socket(int, int, int);
//int recvfrom (int, char*, int, int, struct sockaddr *, int *);
//int sendto (int,const char *, int, int,const struct sockaddr *, int);

int getpagesize(void);
void srand(unsigned seed);
int rand(void);
unsigned sleep(unsigned);
void abort();

}


//----------------------------------------------------------------------
// PollFile
/*! 	Check open file or open socket to see if there are any 
//	characters that can be read immediately.  If so, read them
//	in, and return true.
//
//	In the ACIA case, if there are no threads for us to run,
//	and no characters to be read,
//	we need to give the other side a chance to get our host's CPU
//	(otherwise, we'll go really slowly, since UNIX time-slices
//	infrequently, and this would be like busy-waiting).  So we
//	delay for a short fixed time, before allowing ourselves to be
//	re-scheduled (sort of like a Yield, but cast in terms of UNIX).
//
//	\param fd the file descriptor of the file to be polled
*/
//----------------------------------------------------------------------

bool
PollFile(int fd)
{
    int rfd = (1 << fd), wfd = 0, xfd = 0, retVal;
    struct timeval pollTime;

    // decide how long to wait if there are no characters on the file
    pollTime.tv_sec = 0;
    if (g_machine->GetStatus() == IDLE_MODE)
        pollTime.tv_usec = 20000;             // delay to let other nachos run
    else
        pollTime.tv_usec = 0;                 // no delay

    // poll file or socket
    retVal = select(32, (fd_set*)&rfd, (fd_set*)&wfd, (fd_set*)&xfd, &pollTime);

    ASSERT((retVal == 0) || (retVal == 1));
    if (retVal == 0)
	return false;                 	      // no char waiting to be read
    return true;
}

//----------------------------------------------------------------------
// OpenForWrite
/*! 	Open a file for writing.  Create it if it doesn't exist; truncate it 
//	if it does already exist.  Return the file descriptor.
//
//	\param name file name
//      \return file descriptor
*/
//----------------------------------------------------------------------
int
OpenForWrite(char *name)
{
    int fd = open(name, O_RDWR|O_CREAT|O_TRUNC, 0666);

    ASSERT(fd >= 0); 
    return fd;
}

//----------------------------------------------------------------------
// OpenForReadWrite
/*! 	Open a file for reading or writing.
//	Return the file descriptor, or error if it doesn't exist.
//
//	\param name file name
*/
//----------------------------------------------------------------------
int
OpenForReadWrite(char *name, bool crashOnError)
{
    int fd = open(name, O_RDWR, 0);

    ASSERT(!crashOnError || fd >= 0);
    return fd;
}

//----------------------------------------------------------------------
// Read
//! 	Read characters from an open file.  Abort if read fails.
//----------------------------------------------------------------------
void
Read(int fd, char *buffer, int nBytes)
{
    int retVal = read(fd, buffer, nBytes);
    ASSERT(retVal == nBytes);
}

//----------------------------------------------------------------------
// ReadPartial
/*!	Read characters from an open file, returning as many as are
//	available.
*/
//----------------------------------------------------------------------
int
ReadPartial(int fd, char *buffer, int nBytes)
{
    return read(fd, buffer, nBytes);
}


//----------------------------------------------------------------------
// WriteFile
//! 	Write characters to an open file.  Abort if write fails.
//----------------------------------------------------------------------
void
WriteFile(int fd, char *buffer, int nBytes)
{
    int retVal = write(fd, buffer, nBytes);
    ASSERT(retVal == nBytes);
}

//----------------------------------------------------------------------
// Lseek
//! 	Change the location within an open file.  Abort on error.
//----------------------------------------------------------------------
void 
Lseek(int fd, int offset, int whence)
{
    int retVal = lseek(fd, offset, whence);
    ASSERT(retVal >= 0);
}

//----------------------------------------------------------------------
// Tell
//! 	Report the current location within an open file.
//----------------------------------------------------------------------
int 
Tell(int fd)
{
  return lseek(fd,0,SEEK_CUR);
}

//----------------------------------------------------------------------
// Close
//! 	Close a file.  Abort on error.
//----------------------------------------------------------------------
void 
Close(int fd)
{
    int retVal = close(fd);
    ASSERT(retVal >= 0); 
}

//----------------------------------------------------------------------
// Unlink
//! 	Delete a file.
//----------------------------------------------------------------------
bool 
Unlink(char *name)
{
    return unlink(name);
}

//----------------------------------------------------------------------
// OpenSocket
/*! 	Open an interprocess communication (IPC) connection.  For now, 
//	just open a datagram port where other Nachos (simulating 
//	workstations on a network) can send messages to this Nachos.
*/
//----------------------------------------------------------------------
int
OpenSocket()
{
  int s = socket(AF_INET,SOCK_DGRAM,0);
  ASSERT(s >= 0);
  return s;
}

//----------------------------------------------------------------------
// CloseSocket
//! 	Close the IPC connection. 
//----------------------------------------------------------------------
void
CloseSocket(int sockID)
{
    (void) close(sockID);
}

//----------------------------------------------------------------------
// InitSocketName
//! 	Initialize a UNIX socket address -- magical!
//----------------------------------------------------------------------
static void 
InitSocketName(struct sockaddr_in *uname, char *name)
{
  struct hostent host,*haddr;
  int i;
  // we search the machine IP address
  haddr = gethostbyname(name);
  if (haddr == (struct hostent *)NULL) {
    printf("Can't find host %s\n",name);
    exit(-1);
  }
  else host = *haddr;
  // port definition
  uname->sin_family = AF_INET;
  //uname->sin_port=N_PORT;
  uname->sin_port=g_cfg->NumPortDist;
  bcopy(*host.h_addr_list,&(uname->sin_addr),4);
  for(i=0;i<8;i++)
	uname->sin_zero[i]=0;
}

//----------------------------------------------------------------------
// AssignNameToSocket
/*!	Give a UNIX file name to the IPC port, so other instances of Nachos
//	can locate the port. 
*/
//----------------------------------------------------------------------
void
AssignNameToSocket(char *socketName, int sockID)
{
  struct sockaddr_in uname;
  struct in_addr sad;
  int i;

  // port definition
  uname.sin_family = AF_INET;
  //uname.sin_port=N_PORT;
  uname.sin_port=g_cfg->NumPortLoc;
  sad.s_addr=0;
  uname.sin_addr=sad;
  for(i=0;i<8;i++)
	uname.sin_zero[i]=0;
  bind(sockID,(struct sockaddr*)(&uname),sizeof(struct sockaddr_in));
  // non blocking read
  fcntl(sockID,F_SETFL,O_NDELAY|fcntl(sockID,F_GETFL,0));
  DEBUG('h', (char *)"Created %s socket\n", socketName);
}

//----------------------------------------------------------------------
// ReadFromSocket
//! 	Read a fixed size packet off the IPC port.  Abort on error.
//----------------------------------------------------------------------
int
ReadFromSocket(int sockID, char *buffer, int packetSize)
{
    int retVal;
//    extern int errno;
    struct sockaddr_in uName;
    socklen_t size = sizeof(uName);
   
    retVal = recvfrom(sockID, buffer, packetSize, 0,
				   (struct sockaddr *) &uName, &size);
    return retVal;
}

//----------------------------------------------------------------------
// SendToSocket
/*! 	Transmit a fixed size packet to another Nachos' IPC port.
//	Abort on error.
*/
//----------------------------------------------------------------------
void
SendToSocket(int sockID, char *buffer, int packetSize, char *toName)
{
    struct sockaddr_in uName;

    InitSocketName(&uName, toName);
    sendto(sockID, buffer, packetSize, 0,(sockaddr *) &uName, sizeof(uName));
}


//----------------------------------------------------------------------
// CallOnUserAbort
/*! 	Arrange that "func" will be called when the user aborts (e.g., by
//	hitting ctl-C.
*/
//----------------------------------------------------------------------
void 
CallOnUserAbort(VoidNoArgFunctionPtr func)
{
  (void)signal(SIGINT, (void (*)(int))func);
}

//----------------------------------------------------------------------
// Sleep
/*! 	Put the UNIX process running Nachos to sleep for x seconds,
//	to give the user time to start up another invocation of Nachos
//	in a different UNIX shell.
*/
//----------------------------------------------------------------------

void 
Delay(int seconds)
{
    (void) sleep((unsigned) seconds);
}

//----------------------------------------------------------------------
// Abort
//! 	Quit and drop core.
//----------------------------------------------------------------------
void 
Abort()
{
    abort();
}

//----------------------------------------------------------------------
// Exit
//! 	Quit without dropping core.
//----------------------------------------------------------------------
void 
Exit(int exitCode)
{
    exit(exitCode);
}

//----------------------------------------------------------------------
// RandomInit
/*! 	Initialize the pseudo-random number generator.  We use the
//	now obsolete "srand" and "rand" because they are more portable!
*/
//----------------------------------------------------------------------
void 
RandomInit(unsigned seed)
{
    srand(seed);
}

//----------------------------------------------------------------------
// Random
//! 	Return a pseudo-random number.
//----------------------------------------------------------------------
int 
Random()
{
    return rand();
}

//----------------------------------------------------------------------
// AllocBoundedArray
/*! 	Return an array, with the two pages just before 
//	and after the array unmapped, to catch illegal references off
//	the end of the array.  Particularly useful for catching overflow
//	beyond fixed-size thread execution stacks.
//
//	Note: Just return the useful part!
//      Makes sure that protected pages are on page size boundaries
//      Assumes that the requested size is a multiple of page size
//      (to be fixed)
//
//	\param size amount of useful space needed (in bytes)
*/
//----------------------------------------------------------------------
int8_t* 
AllocBoundedArray(size_t size)
{
  int8_t *ptr = new int8_t[size];
  return ptr;

  /*
  IP: I commented that code because if was not safe wrt memory allocation
  (the strange address calculations turn out to be wrong, the freed area
  in DallocBoundedArray was not exactly the same as the area allocated in
  AllocBoundedArray) - Anybody willing to correct that is strongly encouraged to
  do so.

  printf("AllocBoundedArray %d bytes \n",size);

  size_t pgSize = getpagesize();
  ASSERT(size%pgSize == 0);
  // Allocates more space than need due to alignment constraints of mprotect
  // + 1 extra integer to find the base address when deallocating the data
  int8_t *ptr = new int8_t[1 + pgSize * 3 + size];


  // Find the newt page boundary after the extra integer
  int8_t *ptr_aligned_sup = ALIGN_SUP(ptr+1,pgSize);
  // Store the offset from the returned allocated size just before the protected page
  size_t* ptr_size = (size_t*) (ptr_aligned_sup-sizeof(size_t));
  *ptr_size = ptr_aligned_sup - ptr;

  printf("Alloc size %lu ptr 0x%lx ptrsize %d\n",1 + pgSize * 3 + size, ptr,*ptr_size);

  // Protects the page before and after the zone
  mprotect(ptr_aligned_sup, pgSize, PROT_NONE);
  mprotect(ptr_aligned_sup + pgSize + size, pgSize, PROT_NONE);

  printf("Returned 0x%lx ptralignedsup 0x%lx  pgsize 0x%lx\n",ptr_aligned_sup + pgSize,ptr_aligned_sup,pgSize);
  return (ptr_aligned_sup + pgSize);
  */

}

//----------------------------------------------------------------------
// DeallocBoundedArray
/*! 	Deallocate an array of integers, unprotecting its two boundary pages.
//
//	\param ptr the array to be deallocated
//	\param size amount of useful space in the array (in bytes)
*/
//----------------------------------------------------------------------
void 
DeallocBoundedArray(int8_t *ptr, size_t size)
{
  delete [] ptr;
  /*
  printf("DeallocBoundedArray %d bytes \n",size);
 
  size_t pgSize = getpagesize();
  size_t* ptr_size = (size_t*) ((ptr-pgSize-sizeof(size_t)));

   // Unprotect the two pages
   mprotect(ptr - pgSize, pgSize, PROT_READ | PROT_WRITE | PROT_EXEC);
   mprotect(ptr + size, pgSize, PROT_READ | PROT_WRITE | PROT_EXEC);
   
   // De-allocate the whole area

   printf("Free size %d ptr 0x%lx ptrsize %d\n",size,ptr - pgSize - *ptr_size,*ptr_size);

   delete [] (ptr - pgSize - *ptr_size);*/
}
