/******************************************************************************

 @file  mt_sys.h

 @brief Monitor/Test command definitions for MT SYS subsystem

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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
#ifndef MTSYS_H
#define MTSYS_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include <stdint.h>

#include "mt.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Reset Definitions
 *****************************************************************************/
/*! Hardware reset */
#define MTSYS_RESET_HARD   0x00
/*! Host reset request */
#define MTSYS_RESET_HOST   0x01
/*! HAL assert reset request */
#define MTSYS_ASSERT_HAL   0x02
/*! MAC assert reset request */
#define MTSYS_ASSERT_MAC   0x03
/*! RTOS assert reset request */
#define MTSYS_ASSERT_RTOS  0x04

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 * @brief   Process incoming MT SYS command messages
 *
 * @param   pBuf - pointer to command message buffer
 *
 * @return  Command processing error status
 */
extern uint8_t MtSys_commandProcessing(Mt_mpb_t *pMpb);

/*!
 * @brief   Send an MT "reset response" message
 */
extern void MtSys_resetInd(void);

/*!
 * @brief   Process a "reset request"
 *
 * @param   reason - reason for the reset
 */
extern void MtSys_resetReq(uint8_t reason);

/*!
 * @brief   Update enabled MT SYS callbacks
 *
 * @param   cbBits - bit mask of affected callbacks
 *
 * @return  Current MT SYS callback enabled bit mask
 */
extern uint32_t MtSys_setCallbacks(uint32_t cbBits);

#ifdef __cplusplus
}
#endif

#endif /* MTSYS_H */
