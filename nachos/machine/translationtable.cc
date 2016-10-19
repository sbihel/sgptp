/*! \file translationtable..cc
// \brief Data structures for address translation
//
// DO NOT CHANGE -- part of the machine emulation
//
//  Copyright (c) 1999-2000 INSA de Rennes.
//  All rights reserved.  
//  See copyright_insa.h for copyright notice and limitation 
//  of liability and disclaimer of warranty provisions.
//
*/

#include "machine/translationtable.h"
#include "kernel/system.h"
#include "kernel/thread.h"
#include "utility/config.h"

//----------------------------------------------------------------------
// TranslationTable::TranslationTable
/*!  Constructor.A llocate the page table entries
*/
//----------------------------------------------------------------------
TranslationTable::TranslationTable() {

  // Init private fields
  maxNumPages = g_cfg->MaxVirtPages;
  
  DEBUG('h',(char *)"Allocationg translation table for %d pages (%ld kB)\n",
	maxNumPages, ((long long)maxNumPages*g_cfg->PageSize) >> 10);
  pageTable = new PageTableEntry[maxNumPages];

}

//----------------------------------------------------------------------
// TranslationTable::~TranslationTable()
/*!  Destructor. De-allocate the memory management structures
*/
//----------------------------------------------------------------------
TranslationTable::~TranslationTable() {
 delete [] pageTable;
 DEBUG('h',(char *)"Translation table destroyed");
 
}

//----------------------------------------------------------------------
// TranslationTable::getMaxNumPages()
/*! Get the number of pages that can be translated using the
//  translation table.
*/
//----------------------------------------------------------------------
int TranslationTable::getMaxNumPages() {
  return maxNumPages;
}

//----------------------------------------------------------------------
// TranslationTable::setPhysicalPage
/*!  Set the physical page of a virtual page
//   \param virtualPage : the virtual page
//   \param physicalPage : the physical page
*/
//----------------------------------------------------------------------
void TranslationTable::setPhysicalPage(int virtualPage, int physicalPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].physicalPage = physicalPage;
}

//----------------------------------------------------------------------
//  TranslationTable::getPhysicalPage
/*!  Get the physical page of a virtual page
//   \param virtualPage : the virtual page
//   \return the physical page
*/
//----------------------------------------------------------------------
int TranslationTable::getPhysicalPage(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].physicalPage;
}

//----------------------------------------------------------------------
//  TranslationTable::setAddrDisk
/*!  Set the disk address of a virtual page
//   \param virtualPage : the virtual page
//   \param addrDisk : the address on disk (page number in swap, bytes in
//                     executable file)
*/
//----------------------------------------------------------------------
void TranslationTable::setAddrDisk(int virtualPage, int addrDisk) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].addrDisk = addrDisk;
}

//----------------------------------------------------------------------
//  TranslationTable::getAddrDisk
/*!  Get the disk address of a virtual page
//   \param virtualPage : the virtual page
//   \return the disk address (page number in swap, or bytes in exec file)
*/
//----------------------------------------------------------------------
int TranslationTable::getAddrDisk(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].addrDisk;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitValid
/*!  Set the bit valid of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::setBitValid(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].valid = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitValid
/*!  Clear the bit valid of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::clearBitValid(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].valid = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitValid
/*!  Get the bit valid of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit valid
*/
//----------------------------------------------------------------------
bool TranslationTable::getBitValid(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return  pageTable[virtualPage].valid;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitIo
/*!  Set the bit io of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::setBitIo(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].io = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitIo
/*!  Clear the bit io of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::clearBitIo(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].io = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitIo
/*!  Get the bit io of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit io
*/
//----------------------------------------------------------------------
bool TranslationTable::getBitIo(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].io;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitSwap
/*!  Set the bit swap of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::setBitSwap(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].swap = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitSwap
/*!  Clear the bit swap of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::clearBitSwap(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].swap = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitSwap
/*!  Get the bit swap of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit swap
*/
//----------------------------------------------------------------------
bool TranslationTable::getBitSwap(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].swap;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitReadAllowed
/*!  Set the bit readAllowed of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::setBitReadAllowed(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].readAllowed = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitReadAllowed
/*!  Clear the bit readAllowed of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::clearBitReadAllowed(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].readAllowed = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitReadAllowed
/*!  Get the bit readAllowed of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit readAllowed
*/
//----------------------------------------------------------------------
bool TranslationTable::getBitReadAllowed(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].readAllowed;
}


//----------------------------------------------------------------------
//  TranslationTable::setBitWriteAllowed
/*!  Set the bit writeAllowed of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::setBitWriteAllowed(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].writeAllowed = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitWriteAllowed
/*!  Clear the bit writeAllowed of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void TranslationTable::clearBitWriteAllowed(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].writeAllowed = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitWriteAllowed
/*!  Get the bit writeAllowed of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit writeAllowed
*/
//----------------------------------------------------------------------
bool TranslationTable::getBitWriteAllowed(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].writeAllowed;
}

void TranslationTable::setBitU(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].U = true;
}

void TranslationTable::clearBitU(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].U = false;
}
bool TranslationTable::getBitU(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].U;
}

void TranslationTable::setBitM(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].M = true;
}

void TranslationTable::clearBitM(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].M = false;
}
bool TranslationTable::getBitM(int virtualPage) {
  ASSERT ((virtualPage >= 0) && (virtualPage < maxNumPages));
  return   pageTable[virtualPage].M;
}

//----------------------------------------------------------------------
//   PageTableEntry::PageTableEntry
/*!  Constructor. Defaut initialization of a page table entry
*/
//----------------------------------------------------------------------
PageTableEntry::PageTableEntry()
{
  valid=false;
  swap=false;
  addrDisk = -1;
  readAllowed=false;
  writeAllowed=false;
  U = false;
  M = false;
}
