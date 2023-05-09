/******************************************************************************

 @file ti_154stack_oad_config.h

 @brief Configuration parameters for OAD applications

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2023, Texas Instruments Incorporated
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
#ifndef TI_154STACK_OAD_CONFIG_H
#define TI_154STACK_OAD_CONFIG_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include "ti_154stack_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Constants
 *****************************************************************************/

/* Number of bytes sent in an OAD Block
 * Can be a minimum of 16 and a maximum of 496B
 * Increasing past 128B on Linux VM may cause instability
 */
#if (CONFIG_PHY_ID == APIMAC_250KBPS_IEEE_PHY_0)
#define OAD_BLOCK_SIZE    64
#else
#define OAD_BLOCK_SIZE    128
#endif

#define OAD_BLOCK_REQ_RATE          200
#define OAD_BLOCK_REQ_POLL_DELAY    50

#ifdef __cplusplus
}
#endif

#endif /* TI_154STACK_OAD_CONFIG_H */
