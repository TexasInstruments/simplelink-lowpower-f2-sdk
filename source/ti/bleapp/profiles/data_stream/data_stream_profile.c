/******************************************************************************

 @file  data_stream_profile.c

 @brief This file contains the Data Stream profile sample GATT profile
        for use with the BLE sample application.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2010-2023, Texas Instruments Incorporated
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

#ifdef SYSCFG
#include "ti_ble_config.h"
#endif

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
static DataStreamProfile_CBs_t *dataStreamProfile_appCB = NULL;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void DataStreamProfile_cccUpdateCB( uint16 connHandle, uint16 pValue );
static void DataStreamProfile_incomingDataCB( uint16 connHandle, uint8 *pValue, uint16 len );
static void DataStreamProfile_cccUpdateInvokeFromFWContext( char *pData );
static void DataStreamProfile_incomingDataInvokeFromFWContext( char *pData );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Data Stream profile callback function for incoming data
static DataStreamServer_CB_t dataStreamProfile_serverCB =
{
  DataStreamProfile_cccUpdateCB,
  DataStreamProfile_incomingDataCB
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      DataStreamProfile_start
 *
 * @brief   This function adds the Data Stream Server service,
 *          registers the service and profile callback function and allocates
 *          buffer for incoming data.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DataStreamProfile_start( DataStreamProfile_CBs_t *appCallbacks )
{
  uint8 status = SUCCESS;

  // Add data stream service
  status = DataStreamServer_addService();
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Register to service callback function
  status = DataStreamServer_registerProfileCBs( &dataStreamProfile_serverCB );
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Registers the application callback function
  if ( appCallbacks )
  {
    dataStreamProfile_appCB = appCallbacks;
  }
  else
  {
    return ( INVALIDPARAMETER );
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DataStreamProfile_SendData
 *
 * @brief   Send data over the GATT notification
 *
 * @param   pValue - pointer to data to write
 * @param   len - length of data to write
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DataStreamProfile_sendData( uint8 *pValue, uint16 len )
{
  uint8 status = SUCCESS;

  status = DataStreamServer_setParameter( DATASTREAMSERVER_DATAOUT_ID, pValue, len );

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DataStreamProfile_cccUpdateCB
 *
 * @brief   Callback from Data_Stream_Server indicating CCC has been update
 *
 * @param   connHandle - connection message was received on
 * @param   pValue - pointer to data
 *
 * @return  none
 */
static void DataStreamProfile_cccUpdateCB( uint16 connHandle, uint16 pValue )
{
  DataStream_cccUpdate_t *cccUpdate = NULL;

  if ( dataStreamProfile_appCB && dataStreamProfile_appCB->pfnDataStreamCccUpdateCb )
  {
    // This allocation will be free by bleapp_util
    cccUpdate = (DataStream_cccUpdate_t *)ICall_malloc( sizeof( DataStream_cccUpdate_t ) );
    if ( cccUpdate != NULL )
    {
      // If allocation was successful,
      cccUpdate->connHandle = connHandle;
      cccUpdate->value = pValue;

      // Callback function to notify application of change
      BLEAppUtil_invokeFunction( DataStreamProfile_cccUpdateInvokeFromFWContext, (char *)cccUpdate );
    }
  }
}

/*********************************************************************
 * @fn      DataStreamProfile_IncomingDataCB
 *
 * @brief   Callback from Data_Stream_Server indicating incoming data
 *
 * @param   connHandle - connection message was received on
 * @param   pValue - pointer to data to write
 * @param   len - length of data to write
 *
 * @return  none
 */
static void DataStreamProfile_incomingDataCB( uint16 connHandle, uint8 *pValue, uint16 len )
{
  DataStream_dataIn_t *dataIn = NULL;

  if ( dataStreamProfile_appCB && dataStreamProfile_appCB->pfnDataStreamDataInCb )
  {
    // Check if the data length is no longer than the allowed maximum
    if ( len > DATASTREAM_MAX_DATA_IN_LEN )
    {
      len = 0;
    }

   // This allocation will be free by bleapp_util
   dataIn = (DataStream_dataIn_t *)ICall_malloc( sizeof( DataStream_dataIn_t ) + len );
   if ( dataIn != NULL )
   {
     // If allocation was successful,
     // copy out data out of the buffer and send it to the application
     if ( len > 0 )
     {
       memcpy( dataIn->pValue, pValue, len );
     }
     dataIn->connHandle = connHandle;
     dataIn->len = len;

     // Callback function to notify application of change
     BLEAppUtil_invokeFunction( DataStreamProfile_incomingDataInvokeFromFWContext, (char *)dataIn );
    }
  }
}

/*********************************************************************
 * @fn      DataStreamProfile_cccUpdateInvokeFromFWContext
 *
 * @brief   This function will be called from the BLE App Util module
 *          context.
 *          Calling the application callback
 *
 * @param   pData - data
 *
 * @return  None
 */
static void DataStreamProfile_cccUpdateInvokeFromFWContext( char *pData )
{
  dataStreamProfile_appCB->pfnDataStreamCccUpdateCb( (DataStream_cccUpdate_t *)pData );
}

/*********************************************************************
 * @fn      DataStreamProfile_incomingDataInvokeFromFWContext
 *
 * @brief   This function will be called from the BLE App Util module
 *          context.
 *          Calling the application callback
 *
 * @param   pData - data
 *
 * @return  None
 */
static void DataStreamProfile_incomingDataInvokeFromFWContext( char *pData )
{
  dataStreamProfile_appCB->pfnDataStreamDataInCb( (DataStream_dataIn_t *)pData );
}

/*********************************************************************
*********************************************************************/
