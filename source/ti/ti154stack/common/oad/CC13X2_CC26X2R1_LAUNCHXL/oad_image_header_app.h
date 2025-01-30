/******************************************************************************

 @file  oad_image_header_app.h

 @brief This file extends the _imgHdr that is established in the
    oad_image_header_app.c file for application specific code use.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2001-2025, Texas Instruments Incorporated
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
 
 
 ******************************************************************************/

#ifndef _OAD_IMAGE_HEADER_APP_H
#define _OAD_IMAGE_HEADER_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <oad_image_header.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#if defined(OAD_ONCHIP)
#define TOAD_MEMORY_CFG  DELTA_IMG_MEMORY_CFG_ONCHIP //!< Delta Image built for on-chip OAD */
#else
#define TOAD_MEMORY_CFG  DELTA_IMG_MEMORY_CFG_OFFCHIP //!< Delta Image built for off-chip OAD */
#endif

/*******************************************************************************
 * Typedefs
 */

/*******************************************************************************
 * Externs
 */
/* Make the header visible to other components */
extern const imgHdr_t _imgHdr;

/* Make MCUBoot constants and parameters from the linker file available to other components */
#ifdef MCUBOOT

extern uint32_t _MCUBOOT_HEAD_SIZE, _TRAILER_SIZE, _OAD_HDR_SIZE, _ENTRY_SIZE, _PRIMARY_SLOT_BASE, _SLOT_SIZE, _OAD_HDR_START, _IMG_END_ADDR;
extern const mcuboot_image_header_t* _mcuBootHdr;

#endif // MCUBOOT
#endif /* _OAD_IMAGE_HEADER_APP_H */
