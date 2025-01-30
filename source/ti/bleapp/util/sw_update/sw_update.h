/******************************************************************************

@file  sw_update.h

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

#ifndef SW_UPDATE_H
#define SW_UPDATE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include "bootutil/image.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
/*!
 * SW update configuration types
 */
typedef enum
{
    SW_UPDATE_SOURCE_OAD,          //!<this mode is for OAD
}swUpdateSource_e;

/*!
 * This error will return from SwUpdate_CheckImageHeader if there is problem with header
 */
#define INVALID_HEADER (-1)
/*********************************************************************
 * API FUNCTIONS
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
extern Status_t SwUpdate_Open(swUpdateSource_e source);

/*********************************************************************
 * @fn      SwUpdate_Close
 *
 * @brief   Close software update module and free memory if allocated
 *
 */
extern void SwUpdate_Close(void);

/*********************************************************************
 * @fn      SwUpdate_CheckImageHeader
 *
 * @brief   This function check if header is valid and return image size
 *
 * @param   image header
 *
 * @return  Image_len or INVALID_HEADER
 */
extern int SwUpdate_CheckImageHeader(uint8 *imageHeader);

/*********************************************************************
 * @fn      SwUpdate_RevokeImage
 *
 * @brief   This function receives an image slot as input
 *          and performs an invalidation for the header of that
 *          slot by changing the image version to 0.
 *
 * @param   imageSlot - For which slot to perform the operation
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
extern Status_t SwUpdate_RevokeImage(uint8 imageSlot);

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
extern uint32* SwUpdate_GetSWVersion(uint32 hdrAdrr);

/*********************************************************************
 * @fn      SwUpdate_EraseFlash
 *
 * @brief   This function erase flash by image slot
 *
 * @param   imageSlot
 *
 * @return  SUCCESS or INVALIDPARAMETER
 *
 */
extern Status_t SwUpdate_EraseFlash(uint8 imageSlot);

/*********************************************************************
 * @fn      SwUpdate_WriteBlock
 *
 * @brief   Write image to flash block by block
 *
 * @param   blkStartAddr - Address to write new block
 *          len - Block length
 *          pValue - Block data
 *
 * @return  SUCCESS or INVALIDPARAMETER
 *
 */
extern Status_t SwUpdate_WriteBlock(uint32 blkStartAddr, uint16 len, uint8 *pValue);
/*********************************************************************
 * CONSTANTS
 */

/*!
 * Find empty slot
 */
#define EXT_AUTO_SLOT             0

/*!
 * EXTERNAL SLOT
 */
#define EXT_SLOT_1                1
#define EXT_SLOT_2                2
#define EXT_SLOT_3                3
#define EXT_SLOT_4                4

/*!
 * INTERNAL SLOT
 */
#define INT_PRIMARY_SLOT          5
#define INT_SECONDARY_SLOT        6

#define MCUBOOT_SW_VER_LEN        6
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SW_UPDATE_H */
