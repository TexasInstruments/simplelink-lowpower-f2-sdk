/******************************************************************************

 @file  mt_util.h

 @brief Monitor/Test command definitions for MT UTIL subsystem

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
#ifndef MTUTIL_H
#define MTUTIL_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include <stdint.h>

#include "mt.h"
#include "mac_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Public Function Prototypes
 *****************************************************************************/
/*!
 * @brief   Process incoming MT UTIL command messages
 *
 * @param   pBuf - pointer to command message buffer
 *
 * @return  Command processing error status
 */
extern uint8_t MtUtil_commandProcessing(Mt_mpb_t *pMpb);

/*!
 * @brief   Initialize the MT UTIL command processor
 */
extern void MtUtil_init(void);

#ifdef __cplusplus
}
#endif

#endif /* MTUTIL_H */
