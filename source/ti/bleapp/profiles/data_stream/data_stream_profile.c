/******************************************************************************

 @file  data_stream_profile.c

 @brief This file contains the Data Stream profile sample GATT profile
        for use with the BLE sample application.

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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <icall.h>
/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include <ti/bleapp/profiles/data_stream/data_stream_profile.h>
#include <ti/bleapp/services/data_stream/data_stream_server.h>

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static DSP_cb_t *dsp_appCB = NULL;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void DSP_onCccUpdateCB( char *pValue );
static void DSP_incomingDataCB( char *pValue );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Data Stream profile callback function for incoming data
static DSS_cb_t dsp_serverCB =
{
  DSP_onCccUpdateCB,
  DSP_incomingDataCB
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      DSP_start
 *
 * @brief   This function adds the Data Stream Server service,
 *          registers the service and profile callback function and allocates
 *          buffer for incoming data.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DSP_start( DSP_cb_t *appCallbacks )
{
  uint8 status = SUCCESS;

  // Add data stream service
  status = DSS_addService();
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Register to service callback function
  status = DSS_registerProfileCBs( &dsp_serverCB );
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Registers the application callback function
  if ( appCallbacks )
  {
    dsp_appCB = appCallbacks;
  }
  else
  {
    return ( INVALIDPARAMETER );
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DSP_SendData
 *
 * @brief   Send data over the GATT notification
 *
 * @param   pValue - pointer to data to write
 * @param   len - length of data to write
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DSP_sendData( uint8 *pValue, uint16 len )
{
  uint8 status = SUCCESS;

  status = DSS_setParameter( DSS_DATAOUT_ID, pValue, len );

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DSP_onCccUpdateCB
 *
 * @brief   Callback from Data_Stream_Server indicating CCC has been update
 *
 * @param   connHandle - connection message was received on
 * @param   pValue - pointer to data
 *
 * @return  none
 */
static void DSP_onCccUpdateCB( char *pValue  )
{
  DSS_cccUpdate_t *cccUpdate = ( DSS_cccUpdate_t *)pValue;

  if ( dsp_appCB && dsp_appCB->pfnOnCccUpdateCb )
  {
    dsp_appCB->pfnOnCccUpdateCb( cccUpdate->connHandle, cccUpdate->value );
  }
}

/*********************************************************************
 * @fn      DSP_IncomingDataCB
 *
 * @brief   Callback from Data_Stream_Server indicating incoming data
 *
 * @param   pValue - pointer to the incoming data
 *
 * @return  none
 */
static void DSP_incomingDataCB( char *pValue )
{
  DSS_dataIn_t *dataIn = ( DSS_dataIn_t * )pValue;

  if ( dsp_appCB && dsp_appCB->pfnIncomingDataCB )
  {
    dsp_appCB->pfnIncomingDataCB( dataIn->connHandle, dataIn->pValue, dataIn->len );
  }
}

/*********************************************************************
*********************************************************************/
