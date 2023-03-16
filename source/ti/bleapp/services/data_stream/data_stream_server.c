/******************************************************************************

 @file  data_stream_server.c

 @brief This file contains the Data Stream service sample GATT service
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

#include <ti/bleapp/services/data_stream/data_stream_server.h>
#include "ble_stack_api.h"

/*********************************************************************
 * CONSTANTS
 */
// The size of the notification header is opcode + handle
#define DATASTREAM_NOTI_HDR_SIZE   (ATT_OPCODE_SIZE + 2)

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Data Stream Server Service UUID: 0xC0C0
GATT_UUID( dataStreamServer_serv_UUID, DATASTREAMSERVER_SERV_UUID );

// Data In Characteristic UUID: 0xC0C1
GATT_UUID( dataStreamServer_dataIn_UUID, DATASTREAMSERVER_DATAIN_UUID );

// Data Out Characteristic UUID: 0xC0C2
GATT_UUID( dataStreamServer_dataOut_UUID, DATASTREAMSERVER_DATAOUT_UUID );

static DataStreamServer_CB_t *dataStreamServer_profileCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Data Stream Server Service declaration
static CONST gattAttrType_t dataStreamServer_service = { ATT_UUID_SIZE, dataStreamServer_serv_UUID };

// Characteristic "DataIn" Properties
static uint8 dataStreamServer_dataIn_props = GATT_PROP_WRITE;

// Characteristic "DataIn" Value variable
static uint8 dataStreamServer_dataIn_val = 0;

// Characteristic "DataIn" User Description
static uint8 dataStreamServer_dataIn_userDesp[] = "Write Data";

// Characteristic "DataOut" Properties
static uint8 dataStreamServer_dataOut_props = GATT_PROP_NOTIFY;

// Characteristic "DataOut" Value variable
static uint8 dataStreamServer_dataOut_val = 0;

// Characteristic "DataOut" Configuration each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *dataStreamServer_dataOut_config;

// Characteristic "DataOut" User Description
static uint8 dataStreamServer_dataOut_userDesp[] = "Server Data";

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t dataStreamServer_attrTbl[] =
{
 /*--------------------type-------------------*/ /*------------permissions-------------*/ /*------------------pValue--------------------*/
   // Data Stream Service
   GATT_BT_ATT( primaryServiceUUID,                 GATT_PERMIT_READ,                        (uint8 *) &dataStreamServer_service ),

   // DataIn Characteristic Properties
   GATT_BT_ATT( characterUUID,                      GATT_PERMIT_READ,                        &dataStreamServer_dataIn_props ),
   // DataIn Characteristic Value
   GATT_ATT( dataStreamServer_dataIn_UUID,          GATT_PERMIT_WRITE,                        &dataStreamServer_dataIn_val ),
   // DataIn Characteristic User Description
   GATT_BT_ATT( charUserDescUUID,                   GATT_PERMIT_READ,                        dataStreamServer_dataIn_userDesp ),

   // DataOut Characteristic Properties
   GATT_BT_ATT( characterUUID,                      GATT_PERMIT_READ,                        &dataStreamServer_dataOut_props ),
   // DataOut Characteristic Value
   GATT_ATT( dataStreamServer_dataOut_UUID,         0,                                        &dataStreamServer_dataOut_val ),
   // DataOut Characteristic configuration
   GATT_BT_ATT( clientCharCfgUUID,                  GATT_PERMIT_READ | GATT_PERMIT_WRITE,    (uint8 *) &dataStreamServer_dataOut_config ),
   // DataOut Characteristic User Description
   GATT_BT_ATT( charUserDescUUID,                   GATT_PERMIT_READ,                        dataStreamServer_dataOut_userDesp ),
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
bStatus_t DataStreamServer_writeAttrCB( uint16 connHandle,
                                        gattAttribute_t *pAttr,
                                        uint8 *pValue, uint16 len,
                                        uint16 offset, uint8 method );

static bStatus_t DataStreamServer_sendNotification( uint8 *pValue, uint16 len );

/*********************************************************************
 * PROFILE CALLBACKS
 */

// Data Stream Server Callbacks
// Note: When an operation on a characteristic requires authorization and
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
// operation on a characteristic requires authorization the Stack will call
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
// made within these functions.
CONST gattServiceCBs_t dataStreamServer_servCBs =
{
  NULL,                           // Read callback function pointer
  DataStreamServer_writeAttrCB,  // Write callback function pointer
  NULL                            // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      DataStreamServer_addService
 *
 * @brief   This function initializes the Data Stream Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DataStreamServer_addService( void )
{
  uint8 status = SUCCESS;

  // Allocate Client Characteristic Configuration table
  dataStreamServer_dataOut_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) *
                                                                   MAX_NUM_BLE_CONNS );
  if ( dataStreamServer_dataOut_config == NULL )
  {
    return ( bleMemAllocError );
  }

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, dataStreamServer_dataOut_config );

  // Register GATT attribute list and CBs with GATT Server
  status = GATTServApp_RegisterService( dataStreamServer_attrTbl,
                                        GATT_NUM_ATTRS( dataStreamServer_attrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &dataStreamServer_servCBs );

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DataStreamServer_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t DataStreamServer_registerProfileCBs( DataStreamServer_CB_t *profileCallback )
{
  if ( profileCallback )
  {
    dataStreamServer_profileCBs = profileCallback;

    return ( SUCCESS );
  }

  return ( INVALIDPARAMETER );
}

/*********************************************************************
 * @fn      DataStreamServer_setParameter
 *
 * @brief   Set a Data Stream Service parameter.
 *
 * @param   param - Characteristic UUID
 * @param   pValue - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 * @param   len - length of data to write
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DataStreamServer_setParameter(uint8 param, void *pValue, uint16 len)
{
  bStatus_t status = SUCCESS;

  switch ( param )
  {
    case DATASTREAMSERVER_DATAOUT_ID:
      status = DataStreamServer_sendNotification( (uint8 *)pValue, len );
      break;

    default:
      status = INVALIDPARAMETER;
      break;
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DataStreamServer_writeAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DataStreamServer_writeAttrCB( uint16 connHandle,
                                        gattAttribute_t *pAttr,
                                        uint8 *pValue, uint16 len,
                                        uint16 offset, uint8 method )
{
  bStatus_t status = SUCCESS;

  // See if request is regarding a Client Characteristic Configuration
  if ( ! memcmp( pAttr->type.uuid, clientCharCfgUUID, pAttr->type.len ) )
  {
    // Allow only notifications
    status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                             offset, GATT_CLIENT_CFG_NOTIFY );

    // Notify profile
    if ( status == SUCCESS )
    {
      uint16 value = BUILD_UINT16( pValue[0], pValue[1] );

      if ( dataStreamServer_profileCBs && dataStreamServer_profileCBs->pfnCccUpdateCb )
      {
        // Call profile function from stack task context.
        dataStreamServer_profileCBs->pfnCccUpdateCb( connHandle, value );
      }
    }
  }

  // See if request is regarding the DataIn Characteristic Value
  else if ( ! memcmp( pAttr->type.uuid, dataStreamServer_dataIn_UUID, pAttr->type.len ) )
  {
    // Only notify profile if there is any data in the payload
    if ( len > 0 )
    {
      if ( dataStreamServer_profileCBs && dataStreamServer_profileCBs->pfnIncomingDataCB )
      {
        // Call profile function from stack task context.
        dataStreamServer_profileCBs->pfnIncomingDataCB( connHandle, pValue, len );
      }
    }
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has WRITE permissions.
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DataStreamServer_sendNotification
 *
 * @brief   Transmits data over BLE notifications.
 *
 * @param   pValue - pointer to data to be written
 * @param   len - length of data to be written
 *
 * @return  SUCCESS, or stack call status
 */
static bStatus_t DataStreamServer_sendNotification(uint8 *pValue, uint16 len)
{
  bStatus_t status = SUCCESS;
  gattAttribute_t *pAttr = NULL;
  attHandleValueNoti_t noti = {0};
  linkDBInfo_t connInfo = {0};
  uint16 offset = 0;
  uint8 i = 0;

  // Find the characteristic value attribute
  pAttr = GATTServApp_FindAttr(dataStreamServer_attrTbl,
                               GATT_NUM_ATTRS(dataStreamServer_attrTbl),
                               &dataStreamServer_dataOut_val);
  if ( pAttr != NULL )
  {
    for ( i = 0; i < MAX_NUM_BLE_CONNS; i++ )
    {
      gattCharCfg_t *pItem = &( dataStreamServer_dataOut_config[i] );

      if ( ( pItem->connHandle != LINKDB_CONNHANDLE_INVALID ) &&
           ( pItem->value == GATT_CLIENT_CFG_NOTIFY) )
      {
        // Find out what the maximum MTU size is for each connection
        status = linkDB_GetInfo(pItem->connHandle, &connInfo);
        offset = 0;

        while ( status == SUCCESS &&  len != offset )
        {
          // Determine allocation size
          uint16_t allocLen = (len - offset);
          if ( allocLen > ( connInfo.MTU - DATASTREAM_NOTI_HDR_SIZE ) )
          {
            // If len > MTU split data to chunks of MTU size
            allocLen = connInfo.MTU - DATASTREAM_NOTI_HDR_SIZE;
          }

          noti.len = allocLen;
          noti.pValue = (uint8 *)GATT_bm_alloc( pItem->connHandle, ATT_HANDLE_VALUE_NOTI, allocLen, 0 );
          if ( noti.pValue != NULL )
          {
            // If allocation was successful, copy out data and send it
            memcpy(noti.pValue, pValue + offset, noti.len);
            noti.handle = pAttr->handle;

            // Send the data over BLE notifications
            status = GATT_Notification( pItem->connHandle, &noti, FALSE );

            // If unable to send the data, free allocated buffers and return
            if ( status != SUCCESS )
            {
              GATT_bm_free( (gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI );
            }
            else
            {
              // Increment data offset
              offset += allocLen;
            }
          }
          else
          {
            status = bleNoResources;
          }
        } // End of while
      }
    } // End of for
  } // End of if

  // Return status value
  return ( status );
}

/*********************************************************************
*********************************************************************/
