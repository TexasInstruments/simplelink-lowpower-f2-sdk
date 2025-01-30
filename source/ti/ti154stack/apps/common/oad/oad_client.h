/******************************************************************************

 @file oad_client.h

 @brief OAD Client Header

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
#ifndef OADClient_H
#define OADClient_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef OSAL_PORT2TIRTOS
#include "osal_port.h"
#include <semaphore.h>
#else
#include "icall.h" // for ICall_Semaphore
#endif /*OSAL_PORT2TIRTOS */
#include "native_oad/oad_protocol.h"

#ifndef CUI_DISABLE
#include "cui.h"
#endif


/** @brief RF parameter struct
 *  RF parameters are used with the SOADProtocol_open() and SOADProtocol_Params_init() call.
 */
typedef struct {
    uint16_t *pEvent;             ///< Event handle to post to
#ifdef OSAL_PORT2TIRTOS
    sem_t *eventSem;    ///< Semaphore to post event
#else
    ICall_Semaphore eventSem;    ///< Semaphore to post event
#endif /*OSAL_PORT2TIRTOS */

#ifndef CUI_DISABLE
    CUI_clientHandle_t *pOadCuiHndl; ///< Handle for outputting OAD status messages
#endif
} OADClient_Params_t;

 /** @brief  Function to open the SOADProtocol module
 *
 *  @param  params      An pointer to OADClient_Params_t structure for initialization
 */
extern void OADClient_open(OADClient_Params_t *params);

 /** @brief  Function to process OAD events
 *
 *  @param  pEvent      Event to process
 */
extern void OADClient_processEvent(uint16_t *pEvent);

/** @brief  Function abort OAD
*
*  @param  resume      set to true if a auto resume is required
*/
void OADClient_abort(bool resume);

/** @brief  Function abort OAD
*
*  @param  delay      time in ms to start resume
*/
void OADClient_resume(uint32_t delay);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OADClient_H */
