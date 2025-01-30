/******************************************************************************

 @file  mt_ext.h

 @brief Extended Monitor/Test command/response definitions

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
#ifndef MTEXT_H
#define MTEXT_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include "mt.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Public Function Prototypes
 *****************************************************************************/
/*!
 * @brief   Initialize the extended MT command/response processor
 *
 */
extern void MtExt_init(void);

/*!
 * @brief   Process incoming extended MT command/acknowledge messages
 *
 * @param   pMpb - pointer to RPC command/response message parameter block
 */
extern uint8_t MtExt_processIncoming(Mt_mpb_t *pMpb);

/*!
 * @brief   Send message using extended MT fragmentation. If an outgoing
 *          fragmentation is active, that sequence will be terminated and
 *          an ACK with error status will be sent.
 *
 * @param   pMpb - pointer to RPC command/response message parameter block
 *
 * @return  error - message dropped if not MTRPC_SUCCESS
 */
extern uint8_t MtExt_sendMessage(Mt_mpb_t *pMpb);

#ifdef __cplusplus
}
#endif

#endif /* MTEXT_H */
