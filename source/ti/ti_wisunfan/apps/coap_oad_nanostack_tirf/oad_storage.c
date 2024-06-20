/******************************************************************************

 @file oad_storage.c

 @brief OAD Storage

Group: CMCU, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2022, Texas Instruments Incorporated
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

/*********************************************************************
 * INCLUDES
 */

#include <string.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/flash.h)

#include "flash_interface.h"

#include "oad_storage.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
static bool useExternalFlash = false;

void OADStorage_init(void)
{
    // Open the flash interface
    flash_open();

    // This variable controls whether the OAD module uses internal or external
    // flash memory
    useExternalFlash = hasExternalFlash();
}

OADStorage_Status_t OADStorage_imgBlockWrite(uint32_t blkNum, uint16_t blockLen, uint8_t *pBlockData, uint16_t dataLen)
{
    uint8_t flashStat;
    uint8_t page;
    uint32_t offset;

    // Calculate address to write as (start of OAD range) + (offset into range)
    uint32_t blkStartAddr = blockLen*blkNum;

    if (useExternalFlash)
    {
        page = EXT_FLASH_PAGE(blkStartAddr);
        offset = (blkStartAddr & (~EXTFLASH_PAGE_MASK));
    }
    else
    {
        page = FLASH_PAGE(blkStartAddr);
        offset = (blkStartAddr & (~INTFLASH_PAGE_MASK));
    }

    // Write a OAD_BLOCK to Flash.
    flashStat = writeFlashPg(page, offset, pBlockData, dataLen);

    // Cancel OAD due to flash program error
    if(FLASH_SUCCESS != flashStat)
    {
        return (OADStorage_FlashError);
    }

    // Return and request the next block
    return (OADStorage_Status_Success);
}

OADStorage_Status_t OADStorage_eraseImg(uint32_t imageLen)
{
    // Calculate number of flash pages to erase
    OADStorage_Status_t status = OADStorage_Status_Success;
    uint8_t flashStat, numFlashPages;
    uint32_t page;

    if (useExternalFlash)
    {
        numFlashPages = imageLen / EFL_PAGE_SIZE;
        if(0 != (imageLen % EFL_PAGE_SIZE))
        {
            numFlashPages += 1;
        }
    }
    else
    {
        numFlashPages = imageLen / INTFLASH_PAGE_SIZE;
        if(0 != (imageLen % INTFLASH_PAGE_SIZE))
        {
            numFlashPages += 1;
        }
    }
    // Erase flash pages from image location
    for(page = 0; page < numFlashPages; ++page)
    {
        flashStat = eraseFlashPg(page);
        if(flashStat == FLASH_FAILURE)
        {
            // Return on failure
            status = OADStorage_FlashError;
            break;
        }
    }
    return status;
}

void OADStorage_close(void)
{
    // close the flash interface
    flash_close();
}

/*********************************************************************
*********************************************************************/
