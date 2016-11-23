//---------------------------------------------------------------
/*! \file swapManager.h
   \brief Data structures for the swap mamager
  
   This file provides functions in order to access and 
   manage the swapping mechanism in Nachos.
  
    Copyright (c) 1999-2000 INSA de Rennes.
    All rights reserved.  
    See copyright_insa.h for copyright notice and limitation 
    of liability and disclaimer of warranty provisions.
  
*/
//---------------------------------------------------------------

#ifndef __SWAPMGR_H
#define __SWAPMGR_H

// Forward declarations
class BackingStore;
class DriverDisk;
class BitMap;
class OpenFile;

//-----------------------------------------------------------------
/*! \brief Implements the swap manager
  
   This class implements data structures for providing a swapping
   mechanism in Nachos.

   The class provides operations to:
     - save a page from a buffer to the swapping area, 
     - restore a page from the swapping area to a buffer,
     - release an unused page in the swapping area,
*/
//-----------------------------------------------------------------

class SwapManager {
public:

  /**
   * Initializes the swapping area
   *
   * Initialize the page_flags bitmap to specify that the sectors
   * of the swapping area are free
   */
  SwapManager();

  /**
   * De-allocate the swapping area
   *
   * De-allocate the page_flags bitmap
   */
  ~SwapManager(); 
  
  /** Fill a buffer with the swap information in a specific sector in the swap area
   *
   * \param num_sector: sector number in the swap area
   * \param Swap_Page: buffer where to put the data read from the swap area
   */
  void GetPageSwap(int num_sector,char* Swap_Page);   

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
  int PutPageSwap(int num_sector, char* SwapPage);

  /** This method frees an unused page in the swap area by modifying the
   * page allocation bitmap. This method is called when exiting a
   * process to de-allocate its swap area
   *
   *  \param num_sector: the sector number to free
   */ 
  void ReleasePageSwap(int num_sector); 

  /** This method gives access to the swapdisk's driver */
  DriverDisk * GetSwapDisk ();   

private:

  /** Disk containing the swap area */
  DriverDisk *swap_disk;

  /** Bitmap used to know if sectors in the swap area are free or busy */
  BitMap *page_flags; 

  /** Returns the number of a free page in the swap area
   *
   * This method scans the allocation bitmap page_flags to decide which
   * page is used
   *
   * \return Number of the found free page in the swap area, or -1 of
   * there is no page available
   */
  int GetFreePage();
};

#endif // __SWAPMGR_H



