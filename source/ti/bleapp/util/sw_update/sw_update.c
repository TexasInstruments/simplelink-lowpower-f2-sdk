/******************************************************************************

@file  sw_update.c

 @brief ///////////

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2022-2025, Texas Instruments Incorporated
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

#include "hal_types.h"
#include "comdef.h"
#include "icall.h"
#include "ti/bleapp/util/sw_update/sw_update.h"
#include "bootutil/image.h"
#include "flash_interface.h"

/*********************************************************************
 * MACROS
 */

#define SW_UPDATE_FLASH_PAGE(x,Ext)  Ext==0? FLASH_PAGE(x) :  EXT_FLASH_PAGE(x)

/*********************************************************************
 * LOCAL VARIABLES
 */

/*!
 * This struct contains all the global information that the module needs
 */
typedef struct
{
    struct image_header primaryHdr;           //!< Store the existing Image Header inside flash
    struct image_header candidateImageHeader; //!< Store the candidate Image Header
    uint16              pageSize;             //!< Flash page size
    uint16              pageMask;             //!< Flash page mask
} swUpdateModuleGlobalData_t;

static swUpdateModuleGlobalData_t* pSwUpdateModuleGlobalData;

#define HDR_OFFSET 0X0
#define FLASH_OPEN 1

//temporal solution until offchip OAD dosen't need padding.
#define INT_FLASH_SIZE (INTFLASH_PAGE_SIZE*MAX_ONCHIP_FLASH_PAGES)
#define MCUBOOT_SIZE 0x6000
#define NVS_SIZE 0x4000
#define OFFCHIP_SLOT_SIZE (INT_FLASH_SIZE - MCUBOOT_SIZE - NVS_SIZE)
//end of temporal

/*********************************************************************
 * LOCAL FUNCTIONS - Prototypes
 */


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SwUpdate_Open
 *
 * @brief   Open software update module include init
 *          global variable
 *
 * @param   source - From which source for example OAD
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
Status_t SwUpdate_Open(swUpdateSource_e source)
{
    Status_t status = SUCCESS;

    if(NULL == pSwUpdateModuleGlobalData)
    {
        pSwUpdateModuleGlobalData = (swUpdateModuleGlobalData_t *)ICall_malloc(sizeof(swUpdateModuleGlobalData_t));
        if(NULL == pSwUpdateModuleGlobalData)
        {
           return(FAILURE);
        }
    }

    // This variable controls whether the OAD module uses internal or external flash memory
    pSwUpdateModuleGlobalData->pageSize = hasExternalFlash()?EFL_PAGE_SIZE:INTFLASH_PAGE_SIZE;
    pSwUpdateModuleGlobalData->pageMask = hasExternalFlash()?(~EXTFLASH_PAGE_MASK):(~INTFLASH_PAGE_MASK);

    status = flash_open();
    if(status != FLASH_OPEN)
    {
       return(NV_OPER_FAILED);
    }

    status = readFlash(HDR_OFFSET, (uint8_t *)&(pSwUpdateModuleGlobalData->primaryHdr),sizeof(pSwUpdateModuleGlobalData->primaryHdr));

    return (status);
}

/*********************************************************************
 * @fn      SwUpdate_Close
 *
 * @brief   Close software update module and free memory if allocated
 *
 */
void SwUpdate_Close(void)
{
    flash_close();
}

/*********************************************************************
 * @fn      SwUpdate_CheckImageHeader
 *
 * @brief   This function check if header is valid and return image size
 *
 * @param   image header
 *
 * @return  Image_len or INVALID_HEADER
 */
int SwUpdate_CheckImageHeader(uint8 *imageHeader)
{
    struct image_header *testHeader = (struct image_header *)(imageHeader);
    uint32 candidateImageLength;

    if (IMAGE_MAGIC != testHeader->ih_magic)
    {
        return(INVALID_HEADER);
    }
    pSwUpdateModuleGlobalData->candidateImageHeader = *testHeader;

    if(hasExternalFlash())//temporal solution until offchip OAD dosen't need padding.
    {

        candidateImageLength = OFFCHIP_SLOT_SIZE;
    }
    else
    {
        candidateImageLength = pSwUpdateModuleGlobalData->candidateImageHeader.ih_img_size;
    }

    return (candidateImageLength);
}

/*********************************************************************
 * @fn      SwUpdate_RevokeImage
 *
 * @brief   This function receives an image slot as input
 *          and performs an invalidation for the header of that
 *          slot by changing the image version to 0.
 *
 * @param   imageSlot - For which slot to perform the operation
 *
 * @return  FLASH_SUCCESS or INVALIDPARAMETER
 */
Status_t SwUpdate_RevokeImage(uint8 imageSlot)
{
    Status_t status = FLASH_SUCCESS;
    struct image_header emptyHeader = {0};

    status = flash_open();

    if(INT_PRIMARY_SLOT == imageSlot)
    {
        writeFlash(HDR_OFFSET, (uint8_t *)&(emptyHeader),sizeof(pSwUpdateModuleGlobalData->primaryHdr));

    }
    flash_close();

    return (status);
}

/*********************************************************************
 * @fn      SwUpdate_GetSWVersion
 *
 * @brief   This function extract sw version of MCU header from
 *          given address
 *
 * @param   hdrAdrr - Header address
 *
 * @return  Image version struct
 */
uint32* SwUpdate_GetSWVersion(uint32 hdrAdrr)
{
    struct image_header * img_hdr = (struct image_header*)hdrAdrr;

    return ((uint32 *)&(img_hdr->ih_ver));
}
/*********************************************************************
 * @fn      SwUpdate_EraseImageSlot
 *
 * @brief   This function erase flash by image slot
 *
 * @param   imageSlot
 *
 * @return  SUCCESS or INVALIDPARAMETER
 *
 */
Status_t SwUpdate_EraseImageSlot(uint8 imageSlot)
{
    uint8 status = FLASH_SUCCESS;

    uint32 imgLen = pSwUpdateModuleGlobalData->candidateImageHeader.ih_img_size;
    uint16 imgStartPage = 0;
    uint16 numFlashPages = imgLen/(pSwUpdateModuleGlobalData->pageSize);

    if(0 != (imgLen % (pSwUpdateModuleGlobalData->pageSize)))
    {
        numFlashPages += 1;
    }

    // Erase the correct amount of pages
    for(uint8_t page=imgStartPage; page<(imgStartPage + numFlashPages); ++page)
    {
        status = eraseFlashPg(page);
        if(FLASH_SUCCESS != status)
        {
            // If we fail to pre-erase, then halt the OAD process
            status = FLASH_FAILURE;
            break;
        }
    }

    return (status);
}

/*********************************************************************
 * @fn      SwUpdate_WriteBlock
 *
 * @brief   Write image to flash block by block
 *
 * @param   blkStartAddr - Address to write new block
 *          len - Block length, we assume that len can't be longer
 *                than 1 page
 *          pValue - Block data
 *
 * @return  SUCCESS or INVALIDPARAMETER
 *
 */
Status_t SwUpdate_WriteBlock(uint32 blkStartAddr, uint16 len, uint8 *pValue)
{
    uint8 status = FLASH_SUCCESS;
    uint8 page = 0xFF;
    uint32 offset = 0;
    uint8 candidatePageToDelete = 0xFF;

    page = SW_UPDATE_FLASH_PAGE(blkStartAddr,hasExternalFlash());
    offset = blkStartAddr & (pSwUpdateModuleGlobalData->pageMask);
    if(offset == 0)
    {
        candidatePageToDelete = page;
    }
    if(offset+len>(pSwUpdateModuleGlobalData->pageSize))
    {
        candidatePageToDelete = page+1;
    }
    if(0xFF != candidatePageToDelete)
    {
        status = eraseFlashPg(candidatePageToDelete);
        if(FLASH_SUCCESS != status)
        {
            return (FLASH_FAILURE);
        }
    }
    // Write a block to Flash.
    status = writeFlashPg(page,offset,pValue,len);
    if(FLASH_SUCCESS != status)
    {
        return (FLASH_FAILURE);
    }

    return (status);
}
