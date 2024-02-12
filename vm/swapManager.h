//---------------------------------------------------------------
/*! \file swapManager.h
   \brief Data structures for the swap mamager

   This file provides functions in order to access and
   manage the swapping mechanism in Nachos.

 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes 1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details
 * (see see <http://www.gnu.org/licenses/>).
 * -----------------------------------------------------

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

  /** Fill a buffer with the swap information in a
   * specific sector in the swap area
   *
   * \param disk_addr: disk address in the swap area
   * \param Swap_Page: buffer where to put the data read from the swap area
   */
  void GetPageSwap(uint32_t disk_addr, char *Swap_Page);

  /** This method puts a page into the swapping area. If the sector
   *  number given in parameters is set to -1, the swap manager
   *  chooses a free sector and return its number.
   *
   *  \param disk_addr: disk address in the swap area
   *  \param SwapPage is the buffer to transfer in the swapping area.
   *  \return risk address in the swap area (if page table has to be updated)
   */
  int PutPageSwap(uint32_t disk_addr, char *SwapPage);

  /** This method frees an unused page in the swap area by modifying the
   * page allocation bitmap. This method is called when exiting a
   * process to de-allocate its swap area
   *
   *  \param num_sector: the sector number to free
   */
  void ReleasePageSwap(uint32_t num_sector);

  /** This method gives access to the swapdisk's driver */
  DriverDisk *GetSwapDisk();

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

#endif   // __SWAPMGR_H
