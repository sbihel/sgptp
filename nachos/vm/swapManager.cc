//-----------------------------------------------------------------
/*! \file  swapManager.cc
//  \brief Routines of the swap manager
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.  
//  See copyright_insa.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
//
*/
//-----------------------------------------------------------------
 
#include <unistd.h>

#include "drivers/drvDisk.h"
#include "utility/bitmap.h"
#include "kernel/thread.h"
#include "vm/swapManager.h"

//-----------------------------------------------------------------
/**
 * Initializes the swapping area
 *
 * Initialize the page_flags bitmap to specify that the sectors
 * of the swapping area are free
 */
//-----------------------------------------------------------------
SwapManager::SwapManager() {

  swap_disk = new DriverDisk("sem swap disk","lock swap disk",
			     g_machine->diskSwap);
  page_flags = new BitMap(NUM_SECTORS);

}

//-----------------------------------------------------------------
/**
 * De-allocate the swapping area
 *
 * De-allocate the page_flags bitmap
 */
//-----------------------------------------------------------------
SwapManager::~SwapManager() {

  delete page_flags;
  delete swap_disk;

}

//-----------------------------------------------------------------
/** Returns the number of a free page in the swap area
 *
 * This method scans the allocation bitmap page_flags to decide which
 * page is used
 *
 * \return Number of the found free page in the swap area, or -1 of
 * there is no page available
 */
//-----------------------------------------------------------------
int SwapManager::GetFreePage() {
  
  // Scan the page allocation bitmap
  for (int i=0;i<NUM_SECTORS;i++) {
    if (! page_flags->Test(i)) {
      // the page #i is free
      page_flags->Mark(i);
      return i;
    }
  }

  // There is no available page, return -1
  return -1;
}

//-----------------------------------------------------------------
/** This method frees an unused page in the swap area by modifying the
 * page allocation bitmap. This method is called when exiting a
 * process to de-allocate its swap area
 *
 *  \param num_sector: the sector number to free
*/
//-----------------------------------------------------------------
void SwapManager::ReleasePageSwap(int num_sector) {

  DEBUG('v',(char *)"Swap page %i released for thread \"%s\"\n",num_sector,
	g_current_thread->GetName());
  // clear the #num_sector bit of page_flags
  page_flags->Clear(num_sector);

}

//-----------------------------------------------------------------
/** Fill a buffer with the swap information in a specific sector in the swap area
 *
 * \param num_sector: sector number in the swap area
 * \param Swap_Page: buffer where to put the data read from the swap area
 */
//-----------------------------------------------------------------
void SwapManager::GetPageSwap(int num_sector ,char* SwapPage ) {
  
  DEBUG('v',(char *)"Reading swap page %i for \"%s\"\n",num_sector,
	g_current_thread->GetName());
  swap_disk->ReadSector(num_sector,SwapPage);
}

//-----------------------------------------------------------------
/** This method puts a page into the swapping area. If the sector
 *  number given in parameters is set to -1, the swap manager
 *  chooses a free sector and return its number.
 *  
 *  \param num_sector is the sector number used in the swapping area,
 *  \param SwapPage is the buffer to transfer in the swapping area.
 *  \return The sector number used in the swapping area. This number
 *          is used to update the field disk_page in the translation 
 *          table entry.
*/
//-----------------------------------------------------------------
int SwapManager::PutPageSwap(int num_sector,char *SwapPage) {

  if (num_sector >= 0) {
    DEBUG('v',(char *)"Writing swap page %i for \"%s\"\n",num_sector,
	    g_current_thread->GetName());
    swap_disk->WriteSector(num_sector,SwapPage);
    return num_sector;
  }
  else {
    int newpage = GetFreePage();
    if (newpage == -1) {
      return -1;
    }
    else {
      DEBUG('v',(char *)"Writing swap page %i for \"%s\"\n",newpage,
	    g_current_thread->GetName());
      swap_disk->WriteSector(newpage,SwapPage);
      return newpage;
    }
  }		 
}

//-----------------------------------------------------------------
/** This method gives to the DriverDisk for the swap area */
//-----------------------------------------------------------------
DriverDisk * SwapManager::GetSwapDisk ()
{
  return swap_disk;
}   
