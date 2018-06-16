/*
 * Copyright (C) 2018 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef CRYPTO_SAMFLASHUTIL_H
#define CRYPTO_SAMFLASHUTIL_H

// Utility definitions for accessing flash memory in the
// Arduino Due and other SAM-based Arduino variants.

#if defined (__arm__) && defined (__SAM3X8E__)

#include <Arduino.h>

// Find the highest-numbered flash memory region on this device.
#if defined(IFLASH1_ADDR)
#define SAM_FLASH_ADDR         IFLASH1_ADDR
#define SAM_FLASH_SIZE         IFLASH1_SIZE
#define SAM_FLASH_PAGE_SIZE    IFLASH1_PAGE_SIZE
#define SAM_FLASH_REGION_SIZE  IFLASH1_LOCK_REGION_SIZE
#define SAM_EFC                EFC1
#elif defined(IFLASH0_ADDR)
#define SAM_FLASH_ADDR         IFLASH0_ADDR
#define SAM_FLASH_SIZE         IFLASH0_SIZE
#define SAM_FLASH_PAGE_SIZE    IFLASH0_PAGE_SIZE
#define SAM_FLASH_REGION_SIZE  IFLASH0_LOCK_REGION_SIZE
#define SAM_EFC                EFC0
#else
#define SAM_FLASH_ADDR         IFLASH_ADDR
#define SAM_FLASH_SIZE         IFLASH_SIZE
#define SAM_FLASH_PAGE_SIZE    IFLASH_PAGE_SIZE
#define SAM_FLASH_REGION_SIZE  IFLASH_LOCK_REGION_SIZE
#define SAM_EFC                EFC
#endif

// Storage for the RNG seed in the last page of flash memory.
#define RNG_SEED_ADDR (SAM_FLASH_ADDR + SAM_FLASH_SIZE - SAM_FLASH_PAGE_SIZE)
#define RNG_SEED_PAGE ((SAM_FLASH_SIZE / SAM_FLASH_PAGE_SIZE) - 1)

// Storage for the KeyRing implementation.
#define KEY_RING_STORAGE_SIZE       4096
#define KEY_RING_STORAGE_ADDR \
    (SAM_FLASH_ADDR + SAM_FLASH_SIZE - KEY_RING_STORAGE_SIZE - \
     SAM_FLASH_PAGE_SIZE)
#define KEY_RING_STORAGE_NUM_PAGES \
    (KEY_RING_STORAGE_SIZE / SAM_FLASH_PAGE_SIZE)
#define KEY_RING_STORAGE_FIRST_PAGE \
    ((SAM_FLASH_SIZE / SAM_FLASH_PAGE_SIZE) - KEY_RING_STORAGE_NUM_PAGES - 1)

// Initialize the device for write access to flash memory.
inline void crypto_sam_flash_init(void)
{
    // Initialize the flash memory chip for access mode 128 and 6 wait states.
    efc_init(SAM_EFC, EFC_ACCESS_MODE_128, 6);
}

// Erase and write a specific page.  It is assumed that the data
// to write has already been loaded into the latch registers.
inline void crypto_sam_erase_and_write(unsigned page)
{
    // Erase the page and write its new contents.
    efc_perform_command(SAM_EFC, EFC_FCMD_EWP, page);
}

// Unlock the memory region containing a page.
inline void crypto_sam_unlock_page(unsigned page)
{
    unsigned pagesPerRegion = SAM_FLASH_REGION_SIZE / SAM_FLASH_PAGE_SIZE;
    unsigned region = page - (page % pagesPerRegion);
    efc_perform_command(SAM_EFC, EFC_FCMD_CLB, region);
}

// Lock the memory region containing a page.
inline void crypto_sam_lock_page(unsigned page)
{
    unsigned pagesPerRegion = SAM_FLASH_REGION_SIZE / SAM_FLASH_PAGE_SIZE;
    unsigned region = page - (page % pagesPerRegion);
    efc_perform_command(SAM_EFC, EFC_FCMD_SLB, region);
}

#endif

#endif
