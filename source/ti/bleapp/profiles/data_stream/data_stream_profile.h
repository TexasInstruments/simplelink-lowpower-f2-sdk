/******************************************************************************

 @file  data_stream_profile.h

 @brief This file contains the Data Stream profile definitions and prototypes.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2010-2025, Texas Instruments Incorporated
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

#ifndef DATASTREAMPROFILE_H
#define DATASTREAMPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include <ti/bleapp/ble_app_util/inc/bleapputil_api.h>

/*********************************************************************
 * Profile Callback
 */
// Callback to indicate client characteristic configuration has been updated
typedef void (*DSP_onCccUpdate_t)( uint16 connHandle, uint16 pValue);

// Callback when receiving data
typedef void (*DSP_incomingData_t)( uint16 connHandle, char *pValue, uint16 len );

typedef struct
{
  DSP_onCccUpdate_t   pfnOnCccUpdateCb;  // Called when client characteristic configuration has been updated
  DSP_incomingData_t  pfnIncomingDataCB;     // Called when receiving data
} DSP_cb_t;

/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************
 * @fn      dataStreamProfile_start
 *
 * @brief   This function adds the Data Stream Server service,
 *          registers the application callback function and initializes
 *          buffer for incoming data.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DSP_start( DSP_cb_t *appCallbacks);

/*
 * @fn      dataStreamProfile_sendData
 *
 * @brief   Send data
 *
 * @param   pValue - pointer to data to write
 * @param   len - length of data to write
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DSP_sendData( uint8 *pValue, uint16 len );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* DATASTREAMPROFILE_H */
