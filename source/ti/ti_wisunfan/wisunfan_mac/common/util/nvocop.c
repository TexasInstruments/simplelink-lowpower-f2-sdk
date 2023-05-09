/******************************************************************************

 @file  nvocop.c

 @brief This module contains the OSAL simple non-volatile memory functions.

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2023, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

//*****************************************************************************
// Design Overview
//*****************************************************************************
/*
This driver implements a non-volatile (NV) memory system that utilizes 1 page
of on-chip Flash memory. After initialization, this page is ACTIVE
and GPRAM, or Cache RAM, is available for "compaction" when the ACTIVE page does
not have enough empty space for data write operation. Compaction can occur 'just
in time' during a data write operation or 'on demand' by application request. The
compaction process IS NOT designed to survive a power cycle before it completes.
If power is cycled after the page is erased but before all contents have
been written back to the clean page with the active bit of the page
header set all data stored in this module will be lost. If a power cycle occurs
before in compaction before the page is erased, the process is restarted
during initialization.

Each Flash page has a "page header" which indicates its current state (ERASED,
ACTIVE, or XFER), located at the first byte of the Flash page. The remainder of
the Flash page contains NV data items which are packed together following the
page header. Each NV data item has two parts, (1) a data block which is stored
first (lower memory address), (2) immediately followed by item header (higher
memory address). The item header contains information necessary to traverse the
packed data items, as well as, current status of each data item. A
search for the newest instance of an item is sped up by starting the search at
the last entry in the page (higher memory address).
*/

/*********************************************************************
 * INCLUDES
 */

#include <string.h>
#include "hal_adc.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "pwrmon.h"
#include "hal_assert.h"
#include <driverlib/vims.h>

#include "nvocop.h"

/*********************************************************************
 * CONSTANTS
 */

// Length in bytes of a flash word
#define FLASH_WORD_SIZE            HAL_FLASH_WORD_SIZE

// NV page header size in bytes
#define NV_PAGE_HDR_SIZE           FLASH_WORD_SIZE

// NV item header size in bytes
#ifndef ONE_PAGE_NV_FULL_HDR
#define NV_ITEM_HDR_SIZE           (FLASH_WORD_SIZE)
#else
#define NV_ITEM_HDR_SIZE           (2 * FLASH_WORD_SIZE)
#endif

// In case pages 0-1 are ever used, define a null page value.
#define NV_PAGE_NULL               0

// In case item Id 0 is ever used, define a null item value.
#define NV_ITEM_NULL               0

// NV page header offset within a page
#define NV_PAGE_HDR_OFFSET         0

// Flag in a length field of an item header to indicate validity
// of the length field
#define NV_INVALID_LEN_MARK        0x8000

// Flag in an ID field of an item header to indicate validity of
// the identifier field
#define NV_INVALID_ID_MARK         0x8000

#ifdef ONE_PAGE_NV_FULL_HDR
// Flag in an ID field of an item header to indicate active
#define NV_ACTIVE_ID_MARK          0x4000
#define ID(X)                      (X & 0x00FF)
#endif

// Bit difference between active page state indicator value and
// transfer page state indicator value
#define NV_ACTIVE_XFER_DIFF        0x00100000

// active page state indicator value
#define NV_ACTIVE_PAGE_STATE       NV_ACTIVE_XFER_DIFF

// transfer page state indicator value
#define NV_XFER_PAGE_STATE         (NV_ACTIVE_PAGE_STATE ^ NV_ACTIVE_XFER_DIFF)

#if !defined (DeviceFamily_CC13X2) && !defined (DeviceFamily_CC26X2)
#define NV_PAGE                    (uint8)((uint32)NV_FLASH >> 12)
#else
#define NV_PAGE                    (uint8)((uint32)NV_FLASH >> 13)
#endif

// Cache RAM buffer used as a temporary storage buffer during compaction
// This is volatile memory!
#define RAM_BUFFER_ADDRESS         (uint8*)0x11000000
/*********************************************************************
 * MACROS
 */

// Checks whether CC26xx voltage high enough to erase/write NV
#ifdef NV_VOLTAGE_CHECK
#define NV_CHECK_VOLTAGE()  (PWRMON_check(MIN_VDD_FLASH))
#else
#define NV_CHECK_VOLTAGE()  (TRUE)
#endif //NV_VOLTAGE_CHECK

/*********************************************************************
 * TYPEDEFS
 */

// NV item header structure
#ifndef ONE_PAGE_NV_FULL_HDR
typedef struct
{
  uint16 id;
  uint16 len;
} NvItemHdr_t;
#else
typedef struct
{
  uint16 systemId;
  uint16 itemId;
  uint16 subId;
  uint16 len;
} NvItemHdr_t;
#endif

#ifdef ONE_PAGE_NV_FULL_HDR
typedef enum
{
  NV_OFFSET_DATA,
  NV_OFFSET_HDR
} NvItemOffset_t;
#endif
// Note that osalSnvId_t and osalSnvLen_t cannot be bigger than uint16

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

extern uint8* HalFlashGetAddress( uint8 pg, uint16 offset );

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2)
#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=4096
#pragma location = ".snvSectors"
const uint8 NV_FLASH[FLASH_PAGE_SIZE]  = {0xFF};
#elif defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma location = (SNV_FIRST_PAGE << 12);
const uint8 NV_FLASH[FLASH_PAGE_SIZE] = {0xFF};
#else
#error "Unknown Compiler! Support for SNV not provided!"
#endif
#else
#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=8192
#pragma location = ".snvSectors"
const uint8 NV_FLASH[FLASH_PAGE_SIZE]  = {0xFF};
#elif defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma location = (SNV_FIRST_PAGE << 13);
const uint8 NV_FLASH[FLASH_PAGE_SIZE] = {0xFF};
#else
#error "Unknown Compiler! Support for SNV not provided!"
#endif
#endif
// active page offset
static uint16 pgOff;

// Flag to indicate that a fatal error occurred while writing to or erasing the
// Flash memory. If flag is set, it's unsafe to attempt another write or erase.
// This flag locks writes to Flash until the next system reset.
static uint8 failF = NVINTF_FAILURE;

// Flag to indicate that a non-fatal error occurred while writing to or erasing
// Flash memory. If flag is set, it's safe to attempt another write or erase.
// This flag is reset by any API calls that cause an erase/write to Flash.
static uint8 failW;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void   setActivePage( void );
static void   setXferPage(void);
static void   erasePage( void );
static uint16 compactPage( void );

static void   writeWord( uint16 offset, uint8 *pBuf, osalSnvLen_t cnt );
#ifndef ONE_PAGE_NV_FULL_HDR
static void   writeItem(uint16 offset, osalSnvId_t id, uint16 alignedLen,
                        uint8 *pBuf );
static uint16 findItemInCache(uint16 offset, osalSnvId_t id);
static uint16 findItem( uint16 offset, osalSnvId_t id );
#else
static void   writeItem(uint16 offset, NVINTF_itemID_t nv_id, uint16 alignedLen,
                        uint8 *pBuf );
static uint16 findItemInCache(uint16 offset, NVINTF_itemID_t nv_id);
static uint16 findItem( uint16 offset, NVINTF_itemID_t nv_id );
static uint16 findHeader(uint16 offset, NVINTF_itemID_t nv_id);
static uint16 findItemOffset(uint16 offset, NVINTF_itemID_t nv_id, NvItemOffset_t mode);
#endif
static void   findOffset( void );

static void   enableCache ( uint8 state );
static uint8  disableCache ( void );

//*****************************************************************************
// API Functions - NV driver
//*****************************************************************************

/*********************************************************************
 * @fn      NV_initNV
 *
 * @brief   Initialize the NV flash pages.
 *
 * @param   param - not used.
 *
 * @return  NVINTF_SUCCESS if initialization succeeds, NVINTF_FAILURE otherwise.
 */
uint8 NVOCOP_initNV( void *param )
{
  failW = failF;

#if !defined( NO_OSAL_SNV )
  uint32 pgHdr;
  failF = failW = NVINTF_SUCCESS;

  // Pick active page and clean up erased page if necessary
  HalFlashRead(NV_PAGE, NV_PAGE_HDR_OFFSET, (uint8 *)(&pgHdr), NV_PAGE_HDR_SIZE);

  if ( pgHdr == NV_ACTIVE_PAGE_STATE )
  {
    findOffset();
  }
  else if ( pgHdr == NV_XFER_PAGE_STATE)
  {
    findOffset();

    compactPage();
  }
  else
  {
    // Erase this page if it is not erased.
    // This is to ensure that any page that we're in the middle of
    // compacting gets erased.
    erasePage();

    setActivePage();

    pgOff = NV_PAGE_HDR_SIZE;
  }

  return failW;
#else

  return NVINTF_FAILURE;
#endif // NO_OSAL_SNV
}

/*********************************************************************
 * @fn      NV_compactNV
 *
 * @brief   Compacts NV if fewer byte are free than minAvail bytes.
 *
 * @param   minAvail - number of free bytes in NV for a compaction to not
 *                     immediately occur.  If set to zero compaction will
 *                     always occur.
 *
 * @return  NVINTF_SUCCESS if successful,
 *          NVINTF_FAILURE if failed
 */
uint8 NVOCOP_compactNV( uint16 minAvail )
{
  uint8 ret = failF;

#if !defined( NO_OSAL_SNV )
  uint16 remainder;

  if (ret != NVINTF_SUCCESS)
  {
    return NVINTF_FAILURE;
  }

  // Number of bytes left on active page
  remainder = FLASH_PAGE_SIZE - pgOff;

  // Time to do a compaction?
  if( (remainder < minAvail) || (minAvail == 0) )
  {
    // Change the ACTIVE page to XFER mode
    setXferPage();

    // Reset failW
    failW = NVINTF_SUCCESS;

    // Compact page.
    remainder = compactPage();

    if (remainder == 0)
    {
      ret = (failW == NVINTF_SUCCESS) ? NVINTF_FAILURE : failW;
    }
  }
#endif // !NO_OSAL_SNV

  return ret;
}

//*****************************************************************************
// API Functions - NV Data Items
//*****************************************************************************

/*********************************************************************
 * @fn      NV_readItem
 *
 * @brief   Read data from NV.
 *
 * @param   nv_id  - Valid NV item Id.
 * @param   len    - Length of data to read.
 * @param   *pBuf  - Data is read into this buffer.
 *
 * @return  NVINTF_SUCCESS if successful.
 *          Otherwise, NVINTF_FAILURE for failure.
 */
uint8 NVOCOP_readItem( NVINTF_itemID_t nv_id, uint16 offset, uint16 len, void *pBuf )
{
#if !defined( NO_OSAL_SNV )
#ifndef ONE_PAGE_NV_FULL_HDR
  osalSnvId_t id = nv_id.itemID;

  uint16 itemOffset = findItem(pgOff, id);
#else
  uint16 itemOffset = findItem(pgOff, nv_id);
#endif

  if (itemOffset != 0)
  {
    HalFlashRead(NV_PAGE, itemOffset, pBuf, len);

    return NVINTF_SUCCESS;
  }
#endif // !NO_OSAL_SNV

  return NVINTF_FAILURE;
}

/*********************************************************************
 * @fn      NV_writeItem
 *
 * @brief   Write a data item to NV.
 *
 * @param   nv_id - Valid NV item Id.
 * @param   len   - Length of data to write.
 * @param   *pBuf - Data to write.
 *
 * @return  NVINTF_SUCCESS if successful, failure code otherwise.
 */
uint8 NVOCOP_writeItem( NVINTF_itemID_t nv_id, uint16 len, void *pBuf )
{
#if !defined( NO_OSAL_SNV )
  uint16 alignedLen;
#ifndef ONE_PAGE_NV_FULL_HDR
  osalSnvId_t id = nv_id.itemID;
#endif

  // Reset failW
  failW = NVINTF_SUCCESS;

  {
#ifndef ONE_PAGE_NV_FULL_HDR
    uint16 offset = findItem(pgOff, id);
#else
    uint16 offset = findItem(pgOff, nv_id);
#endif

    if (offset > 0)
    {
      uint8 tmp;
      osalSnvLen_t i;

      for (i = 0; i < len; i++)
      {
        HalFlashRead(NV_PAGE, offset, &tmp, 1);
        if (tmp != ((uint8 *)pBuf)[i])
        {
          break;
        }
        offset++;
      }

      if (i == len)
      {
        // Changed value is the same value as before.
        // Return here instead of re-writing the same value to NV.
        return NVINTF_SUCCESS;
      }
    }
  }

  alignedLen = ((len + FLASH_WORD_SIZE - 1) / FLASH_WORD_SIZE) * FLASH_WORD_SIZE;

  if ( pgOff + alignedLen + NV_ITEM_HDR_SIZE > FLASH_PAGE_SIZE )
  {
    setXferPage();
    compactPage();
#ifdef ONE_PAGE_NV_FULL_HDR
    /* protect memory out of NV bound */
    if ( pgOff + alignedLen + NV_ITEM_HDR_SIZE > FLASH_PAGE_SIZE )
    {
        return NVINTF_FAILURE;
    }
#endif
  }

  // pBuf shall be referenced beyond its valid length to save code size.
#ifndef ONE_PAGE_NV_FULL_HDR
  writeItem(pgOff, id, alignedLen, pBuf);
#else
  writeItem(pgOff, nv_id, alignedLen, pBuf);
#endif

  // Check if failed
  if (failW != NVINTF_SUCCESS)
  {
    return failW;
  }

  pgOff += alignedLen + NV_ITEM_HDR_SIZE;

  return NVINTF_SUCCESS;
#else

  return NVINTF_FAILURE;
#endif // !NO_OSAL_SNV
}

#ifdef ONE_PAGE_NV_FULL_HDR
/******************************************************************************
 * @fn      NV_deleteItem
 *
 * @brief   Delete an existing NV item from Flash memory
 *
 * @param   id - NV item type identifier
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
uint8 NVOCOP_deleteItem(NVINTF_itemID_t id)
{
    NvItemHdr_t hdr;
    uint16 cOff;
    uint16 pOff;

    // erase whole page once requested
    if ((id.systemID == 0xFF) && (id.itemID == 0xFFFF) && (id.subID == 0xFFFF))
    {
        erasePage();
        setActivePage();
        pgOff = NV_PAGE_HDR_SIZE;
        return (NVINTF_SUCCESS);
    }

#if defined (NVOCTP_DIAGNOSTICS)
    if(id.systemID == NVINTF_SYSID_NVDRVR)
    {
        // Protect NV driver item(s)
        return (NVINTF_BADSYSID);
    }
#endif

    pOff = 0;
    while((cOff = findHeader(pgOff, id)) && (cOff != pOff))
    {
        // read header
        HalFlashRead(NV_PAGE, cOff, (uint8 *)&hdr, sizeof(NvItemHdr_t));
        //set the item as inactive
        hdr.systemId &= ~ NV_ACTIVE_ID_MARK;
        // write header back
        writeWord(cOff, (uint8 *)&hdr, sizeof(NvItemHdr_t) / FLASH_WORD_SIZE);
        pOff = cOff;
    }

    if(!cOff)
    {
        return (pOff ? NVINTF_SUCCESS : NVINTF_NOTFOUND);
    }
    else
    {
        return (NVINTF_CORRUPT);
    }
}
#endif

//*****************************************************************************
// Local NV Driver Utility Functions
//*****************************************************************************

/*********************************************************************
 * @fn      setActivePage
 *
 * @brief   Set page header active state to be active.
 *
 * @param   none
 *
 * @return  none
 */
static void setActivePage(void)
{
  uint32 pgHdr;

  pgHdr = NV_ACTIVE_PAGE_STATE;

  writeWord( NV_PAGE_HDR_OFFSET, (uint8*) &pgHdr, 1);
}

/*********************************************************************
 * @fn      setXferPage
 *
 * @brief   Set active page header state to be transfer state.
 *
 * @param   none
 *
 * @return  none
 */
static void setXferPage(void)
{
  uint32 pgHdr;

  // erase difference bit between active state and xfer state
  pgHdr = NV_XFER_PAGE_STATE;

  writeWord( NV_PAGE_HDR_OFFSET, (uint8*)&pgHdr, 1 );
}

uint32_t sector_addr;
/*********************************************************************
 * @fn      erasePage
 *
 * @brief   Erases a page in Flash.
 *
 * @param   none
 *
 * @return  none
 */
static void erasePage( void )
{
  if (!NV_CHECK_VOLTAGE())
  {
    // Power monitor indicates low voltage
    failW = NVINTF_FAILURE;
    return;
  }
  else
  {
    halIntState_t cs;
    uint8 state;
    uint32_t err;

    HAL_ENTER_CRITICAL_SECTION(cs);

    // Disable the cache.
    state = disableCache();

    sector_addr =  (uint32)HalFlashGetAddress(NV_PAGE, 0);

    // Erase the page.
    err = FlashSectorErase( sector_addr);

    // Enable the cache.
    enableCache(state);

      // Page erase failed, further usage is unsafe.
    if (err != FAPI_STATUS_SUCCESS)
    {
      failF = failW = NVINTF_FAILURE;
    }

    HAL_EXIT_CRITICAL_SECTION(cs);
  }
}

/*********************************************************************
 * @fn      findOffset
 *
 * @brief   find an offset of an empty space in active page
 *          where to write a new item to.
 *
 * @param   None
 *
 * @return  none
 */
static void findOffset(void)
{
  uint16 offset;
  for (offset = FLASH_PAGE_SIZE - FLASH_WORD_SIZE;
       offset >= NV_PAGE_HDR_SIZE;
       offset -= FLASH_WORD_SIZE)
  {
    uint32 tmp;

    HalFlashRead(NV_PAGE, offset, (uint8 *)&tmp, FLASH_WORD_SIZE);
    if (tmp != 0xFFFFFFFF)
    {
      break;
    }
  }
  pgOff = offset + FLASH_WORD_SIZE;
}

/*********************************************************************
 * @fn      findItemOffset
 *
 * @brief   find a valid item offset from a designated page and offset
 *
 * @param   offset - offset in the NV page from where to start search up.
 *                   Usually this parameter is set to the empty space offset.
 * @param   id     - NV item ID to search for
 *
 * @param   mode   - NV item offset mode
 *
 * @return  offset of the item, 0 when not found
 */
#ifdef ONE_PAGE_NV_FULL_HDR
static uint16 findItemOffset(uint16 offset, NVINTF_itemID_t nv_id, NvItemOffset_t mode)
{
  offset -= FLASH_WORD_SIZE;

  while (offset >= (NV_PAGE_HDR_SIZE + NV_ITEM_HDR_SIZE - FLASH_WORD_SIZE))
  {
    NvItemHdr_t hdr;

    HalFlashRead(NV_PAGE, offset + FLASH_WORD_SIZE - NV_ITEM_HDR_SIZE,
                 (uint8 *) &hdr, NV_ITEM_HDR_SIZE);

    if ((hdr.systemId & NV_ACTIVE_ID_MARK)
     && (ID(hdr.systemId) == nv_id.systemID)
     && (hdr.itemId == nv_id.itemID)
     && (hdr.subId == nv_id.subID))
    {
      // item found
      // length field could be corrupt. Mask invalid length mark.
      if (mode == NV_OFFSET_DATA)
      {
          uint16 len = hdr.len & ~NV_INVALID_LEN_MARK;
          return offset + FLASH_WORD_SIZE - NV_ITEM_HDR_SIZE - len;
      }
      else
      {
          return offset + FLASH_WORD_SIZE - NV_ITEM_HDR_SIZE;
      }
    }
    else if (hdr.len & NV_INVALID_LEN_MARK)
    {
      offset -= FLASH_WORD_SIZE;
    }
    else
    {
      // valid length field
      if (hdr.len + NV_ITEM_HDR_SIZE <= offset)
      {
        // valid length
        offset -= hdr.len + NV_ITEM_HDR_SIZE;
      }
      else
      {
        // active page is corrupt
        // This could happen if NV initialization failed upon failure to erase
        // page and active page is set to uncleanly erased page.
        HAL_ASSERT_FORCED();
        return 0;
      }
    }
  }
  return 0;
}

/*********************************************************************
 * @fn      findHeader
 *
 * @brief   find a valid item header from a designated page and offset
 *
 * @param   offset - offset in the NV page from where to start search up.
 *                   Usually this parameter is set to the empty space offset.
 * @param   id     - NV item ID to search for
 *
 * @return  offset of the item header, 0 when not found
 */
static uint16 findHeader(uint16 offset, NVINTF_itemID_t nv_id)
{
    return (findItemOffset(offset, nv_id, NV_OFFSET_HDR));
}

/*********************************************************************
 * @fn      findItem
 *
 * @brief   find a valid item from a designated page and offset
 *
 * @param   offset - offset in the NV page from where to start search up.
 *                   Usually this parameter is set to the empty space offset.
 * @param   id     - NV item ID to search for
 *
 * @return  offset of the item, 0 when not found
 */
static uint16 findItem(uint16 offset, NVINTF_itemID_t nv_id)
{
    return (findItemOffset(offset, nv_id, NV_OFFSET_DATA));
}
#else
/*********************************************************************
 * @fn      findItem
 *
 * @brief   find a valid item from a designated page and offset
 *
 * @param   offset - offset in the NV page from where to start search up.
 *                   Usually this parameter is set to the empty space offset.
 * @param   id     - NV item ID to search for
 *
 * @return  offset of the item, 0 when not found
 */
static uint16 findItem(uint16 offset, osalSnvId_t id)
{
  offset -= FLASH_WORD_SIZE;

  while (offset >= (NV_PAGE_HDR_SIZE + NV_ITEM_HDR_SIZE - FLASH_WORD_SIZE))
  {
    NvItemHdr_t hdr;

    HalFlashRead(NV_PAGE, offset + FLASH_WORD_SIZE - NV_ITEM_HDR_SIZE,
                 (uint8 *) &hdr, NV_ITEM_HDR_SIZE);

    if (hdr.id == id)
    {
      // item found
      // length field could be corrupt. Mask invalid length mark.
      uint16 len = hdr.len & ~NV_INVALID_LEN_MARK;
      return offset + FLASH_WORD_SIZE - NV_ITEM_HDR_SIZE - len;
    }
    else if (hdr.len & NV_INVALID_LEN_MARK)
    {
      offset -= FLASH_WORD_SIZE;
    }
    else
    {
      // valid length field
      if (hdr.len + NV_ITEM_HDR_SIZE <= offset)
      {
        // valid length
        offset -= hdr.len + NV_ITEM_HDR_SIZE;
      }
      else
      {
        // active page is corrupt
        // This could happen if NV initialization failed upon failure to erase
        // page and active page is set to uncleanly erased page.
        HAL_ASSERT_FORCED();
        return 0;
      }
    }
  }
  return 0;
}
#endif

/*********************************************************************
 * @fn      findItemInCache
 *
 * @brief   find a valid item from cache
 *
 * @param   ramBuffer - Points buffer to the start of cache ram 0x11000000.
 * @param   offset    - offset in cache.
 * @param   id        - NV item ID to search for.
 *
 * @return  offset of the item, 0 when not found
 */
#ifndef ONE_PAGE_NV_FULL_HDR
static uint16 findItemInCache(uint16 offset, osalSnvId_t id)
#else
static uint16 findItemInCache(uint16 offset, NVINTF_itemID_t nv_id)
#endif
{
  offset -= FLASH_WORD_SIZE;

  while (offset >= (NV_PAGE_HDR_SIZE + NV_ITEM_HDR_SIZE - FLASH_WORD_SIZE)
      && offset < FLASH_PAGE_SIZE)
  {
    NvItemHdr_t hdr;

    // Read header
    memcpy( &hdr, RAM_BUFFER_ADDRESS + offset + FLASH_WORD_SIZE - NV_ITEM_HDR_SIZE,
            NV_ITEM_HDR_SIZE);

    //If you find id return true else update offset and search again
#ifndef ONE_PAGE_NV_FULL_HDR
    if ( hdr.id == id )
#else
    if ((hdr.systemId & NV_ACTIVE_ID_MARK)
     && (ID(hdr.systemId) == nv_id.systemID)
     && (hdr.itemId == nv_id.itemID)
     && (hdr.subId == nv_id.subID))
#endif
    {
      return 1;
    }
    else
    {
      offset -= hdr.len + NV_ITEM_HDR_SIZE;
    }
  }

  return 0;
}

/*********************************************************************
 * @fn      writeItem
 *
 * @brief   Write a data item to NV. Function can write an entire item to NV
 *
 * @param   offset     - offset within the NV page where to write the new item
 * @param   id         - NV item ID
 * @param   alignedLen - Length of data to write, alinged in flash word
 *                       boundary
 * @param   pBuf - Data to write.
 *
 * @return  none
 */
#ifndef ONE_PAGE_NV_FULL_HDR
static void writeItem(uint16 offset, osalSnvId_t id, uint16 alignedLen, uint8 *pBuf )
#else
static void writeItem(uint16 offset, NVINTF_itemID_t nv_id, uint16 alignedLen, uint8 *pBuf )
#endif
{
  NvItemHdr_t hdr;

#ifndef ONE_PAGE_NV_FULL_HDR
  hdr.id = 0xFFFF;
#else
  hdr.systemId = 0xFFFF;
  hdr.itemId = 0xFFFF;
  hdr.subId = 0xFFFF;
#endif
  hdr.len = alignedLen | NV_INVALID_LEN_MARK;

  // Write the len portion of the header first
  writeWord(offset + alignedLen, (uint8 *) &hdr, NV_ITEM_HDR_SIZE / FLASH_WORD_SIZE);

  // remove invalid len mark
  hdr.len &= ~NV_INVALID_LEN_MARK;
  writeWord(offset + alignedLen, (uint8 *) &hdr, NV_ITEM_HDR_SIZE / FLASH_WORD_SIZE);

  // Copy over the data
  writeWord(offset, pBuf, alignedLen / FLASH_WORD_SIZE);

  // value is valid. Write header except for the most significant bit.
#ifndef ONE_PAGE_NV_FULL_HDR
  hdr.id = id | NV_INVALID_ID_MARK;
#else
  hdr.systemId = nv_id.systemID | NV_INVALID_ID_MARK | NV_ACTIVE_ID_MARK;
  hdr.itemId = nv_id.itemID;
  hdr.subId = nv_id.subID;
#endif
  writeWord(offset + alignedLen, (uint8 *) &hdr, NV_ITEM_HDR_SIZE / FLASH_WORD_SIZE);

  // write the most significant bit
#ifndef ONE_PAGE_NV_FULL_HDR
  hdr.id &= ~NV_INVALID_ID_MARK;
#else
  hdr.systemId &= ~NV_INVALID_ID_MARK;
#endif
  writeWord(offset + alignedLen, (uint8 *) &hdr, NV_ITEM_HDR_SIZE / FLASH_WORD_SIZE);
}

/*********************************************************************
 * @fn      compactPage
 *
 * @brief   Compacts the page specified.
 *
 * @param   none
 *
 * @return  remaining unused bytes on the flash page.
 */
static uint16 compactPage()
{
  uint8 state;
  uint16 srcOff, dstOff;
#ifndef ONE_PAGE_NV_FULL_HDR
  osalSnvId_t lastId = (osalSnvId_t) 0xFFFF;
#else
  NVINTF_itemID_t lastId;

  lastId.systemID = 0xFF;
  lastId.itemID = 0xFFFF;
  lastId.subID = 0xFFFF;
#endif

  // Start writing to base of RAM buffer.
  dstOff = 0;

  // Disable cache to use as a temporary storage buffer during compaction.
  state = disableCache();

  // Read from the latest value
  srcOff = pgOff - FLASH_WORD_SIZE;

  while (srcOff >= (NV_PAGE_HDR_SIZE + NV_ITEM_HDR_SIZE - FLASH_WORD_SIZE))
  {
    NvItemHdr_t hdr;

    HalFlashRead(NV_PAGE, srcOff + FLASH_WORD_SIZE - NV_ITEM_HDR_SIZE,
                 (uint8 *) &hdr, NV_ITEM_HDR_SIZE);

#ifndef ONE_PAGE_NV_FULL_HDR
    if (hdr.id == 0xFFFF || hdr.id == NV_ITEM_NULL)
#else
    if ((hdr.systemId == 0xFFFF && hdr.itemId == 0xFFFF && hdr.subId == 0xFFFF)
     || (hdr.systemId == NV_ITEM_NULL && hdr.itemId == NV_ITEM_NULL
     && hdr.subId == NV_ITEM_NULL))
#endif
    {
      // Invalid entry. Skip this one.
      if (hdr.len & NV_INVALID_LEN_MARK)
      {
        srcOff -= FLASH_WORD_SIZE;
      }
      else
      {
        if (hdr.len + NV_ITEM_HDR_SIZE <= srcOff)
        {
          srcOff -= hdr.len + NV_ITEM_HDR_SIZE;
        }
        else
        {
          // invalid length. Source page must be a corrupt page.
          // This is possible only if the NV initialization failed upon erasing
          // what is selected as active page.
          // This is supposed to be a very rare case, as power should be
          // shutdown exactly during erase and then the page header is
          // still retained as either the Xfer or the Active state.

          // For production code, it might be useful to attempt to erase the page
          // so that at next power cycle at least the device is runnable
          // (with all entries removed).
          // However, it might be still better not to attempt erasing the page
          // just to see if this very rare case actually happened.
          //erasePage(srcPg);

          HAL_ASSERT_FORCED();

          // Enable cache use
          enableCache(state);

          return 0;
        }
      }

      continue;
    }

    // Consider only valid item
#ifndef ONE_PAGE_NV_FULL_HDR
    if (!(hdr.id & NV_INVALID_ID_MARK) && hdr.id != lastId)
#else
    if (!(hdr.systemId & NV_INVALID_ID_MARK) && (hdr.systemId & NV_ACTIVE_ID_MARK)
      && (ID(hdr.systemId) != ID(lastId.systemID) || hdr.itemId != lastId.itemID
      || hdr.subId != lastId.subID))
#endif
    {
      // lastId is used to speed up compacting in case the same item ID
      // items were neighboring each other contiguously.
#ifndef ONE_PAGE_NV_FULL_HDR
      lastId = (osalSnvId_t) hdr.id;
#else
      lastId.systemID = ID(hdr.systemId);
      lastId.itemID = hdr.itemId;
      lastId.subID = hdr.subId;
#endif

      // Check if the latest value of the item was already written
      if (findItemInCache(dstOff, lastId) == 0)
      {
        // This item was not copied over yet, This must be the latest value.
        // Transfer item from flash active page to cache memory after while loop completes a
        // compacted version of the flash memory will exist in cache.
        HalFlashRead(NV_PAGE, srcOff + FLASH_WORD_SIZE - NV_ITEM_HDR_SIZE - hdr.len,
                     RAM_BUFFER_ADDRESS + dstOff,
                     hdr.len + NV_ITEM_HDR_SIZE);

        dstOff += hdr.len + NV_ITEM_HDR_SIZE;
      }
    }
    srcOff -= hdr.len + NV_ITEM_HDR_SIZE;
  }

  // Erase the currently active page
  erasePage();

  // Write items back from cache to srcPg
  if (dstOff > 0)
  {
    if (NV_CHECK_VOLTAGE())
    {
      HalFlashWrite((uint32)NV_FLASH + NV_PAGE_HDR_SIZE, RAM_BUFFER_ADDRESS,
                    dstOff);

      // Set srcPg as the active page
      setActivePage();
    }
    else
    {
      failW = NVINTF_FAILURE;
    }
  }

  // Enable cache use
  enableCache(state);

  if (!failW)
  {
    pgOff = dstOff + NV_PAGE_HDR_SIZE; // update active page offset

    return ( FLASH_PAGE_SIZE - dstOff );
  }
  else
  {
    pgOff = 0;

    return 0;
  }
}

/*********************************************************************
 * @fn      verifyWordM
 *
 * @brief   verify the written word.
 *
 * @param   offset - A valid offset into the page.
 * @param   pBuf   - Pointer to source buffer.
 * @param   cnt    - Number of 4-byte blocks to verify.
 *
 * @return  none
 */
static void verifyWordM(uint16 offset, uint8 *pBuf, osalSnvLen_t cnt )
{
  uint8 tmp[FLASH_WORD_SIZE];

  while (cnt--)
  {
    // Reading byte per byte will reduce code size but will slow down
    // and not sure it will meet the timing requirements.
    HalFlashRead(NV_PAGE, offset, tmp, FLASH_WORD_SIZE);
    if (memcmp(tmp, pBuf, FLASH_WORD_SIZE))
    {
      failF = failW = NVINTF_FAILURE;
      return;
    }
    offset += FLASH_WORD_SIZE;
    pBuf += FLASH_WORD_SIZE;
  }
}

/*********************************************************************
 * @fn      writeWord
 *
 * @brief   Writes a Flash-WORD to NV.
 *
 * @param   offset - A valid offset into the page.
 * @param   pBuf   - Pointer to source buffer.
 * @param   cnt    - Number of words to write.
 *
 * @return  none
 */
static void writeWord( uint16 offset, uint8 *pBuf, osalSnvLen_t cnt )
{
  uint32 addr = (uint32) HalFlashGetAddress(NV_PAGE, offset);

  if (NV_CHECK_VOLTAGE())
  {
    // Enter Critical Section
    halIntState_t cs;
    HAL_ENTER_CRITICAL_SECTION(cs);

    // Disable Cache
    uint8 state;
    state = disableCache();

    // Write data.
    HalFlashWrite(addr, pBuf, cnt * (FLASH_WORD_SIZE) );

    // Enable cache.
    enableCache(state);

    verifyWordM(offset, pBuf, cnt);

    HAL_EXIT_CRITICAL_SECTION(cs);
  }
  else
  {
    failW = NVINTF_FAILURE;
  }
}

/*********************************************************************
 * @fn      enableCache
 *
 * @brief   enable cache.
 *
 * @param   state - the VIMS state returned from disableCache.
 *
 * @return  none.
 */
static void enableCache ( uint8 state )
{
  if ( state != VIMS_MODE_DISABLED )
  {
    // Enable the Cache.
    VIMSModeSet( VIMS_BASE, VIMS_MODE_ENABLED );
  }
}

/*********************************************************************
 * @fn      disableCache
 *
 * @brief   invalidate and disable cache.
 *
 * @param   none
 *
 * @return  VIMS state
 */
static uint8 disableCache ( void )
{
  uint8 state = VIMSModeGet( VIMS_BASE );

  // Check VIMS state
  if ( state != VIMS_MODE_DISABLED )
  {
    // Invalidate cache
    VIMSModeSet( VIMS_BASE, VIMS_MODE_DISABLED );

    // Wait for disabling to be complete
    while ( VIMSModeGet( VIMS_BASE ) != VIMS_MODE_DISABLED );

  }

  return state;
}

/*********************************************************************
*********************************************************************/


