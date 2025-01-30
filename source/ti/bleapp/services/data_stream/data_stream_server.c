/******************************************************************************

 @file  data_stream_server.c

 @brief This file contains the Data Stream service sample GATT service
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

#include <ti/bleapp/services/data_stream/data_stream_server.h>
#include <ti/bleapp/ble_app_util/inc/bleapputil_api.h>
#include <ti/bleapp/menu_module/menu_module.h>
#include <app_main.h>
#include "ble_stack_api.h"

/*********************************************************************
 * CONSTANTS
 */
// The size of the notification header is opcode + handle
#define DSS_NOTI_HDR_SIZE   (ATT_OPCODE_SIZE + 2)

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Data Stream Server Service UUID: 0xC0C0
GATT_UUID( dss_serv_UUID, DSS_SERV_UUID );

// Data In Characteristic UUID: 0xC0C1
GATT_UUID( dss_dataIn_UUID, DSS_DATAIN_UUID );

// Data Out Characteristic UUID: 0xC0C2
GATT_UUID( dss_dataOut_UUID, DSS_DATAOUT_UUID );

static DSS_cb_t *dss_profileCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Data Stream Server Service declaration
static CONST gattAttrType_t dss_service = { ATT_UUID_SIZE, dss_serv_UUID };

// Characteristic "DataIn" Properties
static uint8 dss_dataIn_props = GATT_PROP_WRITE;

// Characteristic "DataIn" Value variable
static uint8 dss_dataIn_val = 0;

// Characteristic "DataIn" User Description
static uint8 dss_dataIn_userDesp[] = "Write Data";

// Characteristic "DataOut" Properties
static uint8 dss_dataOut_props = GATT_PROP_NOTIFY;

// Characteristic "DataOut" Value variable
static uint8 dss_dataOut_val = 0;

// Characteristic "DataOut" Configuration each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *dss_dataOut_config;

// Characteristic "DataOut" User Description
static uint8 dss_dataOut_userDesp[] = "Server Data";

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t dss_attrTbl[] =
{
 /*--------------------type-------------------*/ /*------------permissions-------------*/ /*------------------pValue--------------------*/
   // Data Stream Service
   GATT_BT_ATT( primaryServiceUUID,                 GATT_PERMIT_READ,                        (uint8 *) &dss_service ),

   // DataIn Characteristic Properties
   GATT_BT_ATT( characterUUID,                      GATT_PERMIT_READ,                        &dss_dataIn_props ),
   // DataIn Characteristic Value
   GATT_ATT( dss_dataIn_UUID,                       GATT_PERMIT_WRITE,                       &dss_dataIn_val ),
   // DataIn Characteristic User Description
   GATT_BT_ATT( charUserDescUUID,                   GATT_PERMIT_READ,                        dss_dataIn_userDesp ),

   // DataOut Characteristic Properties
   GATT_BT_ATT( characterUUID,                      GATT_PERMIT_READ,                        &dss_dataOut_props ),
   // DataOut Characteristic Value
   GATT_ATT( dss_dataOut_UUID,                      0,                                       &dss_dataOut_val ),
   // DataOut Characteristic configuration
   GATT_BT_ATT( clientCharCfgUUID,                  GATT_PERMIT_READ | GATT_PERMIT_WRITE,    (uint8 *) &dss_dataOut_config ),
   // DataOut Characteristic User Description
   GATT_BT_ATT( charUserDescUUID,                   GATT_PERMIT_READ,                        dss_dataOut_userDesp ),
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t DSS_writeAttrCB( uint16 connHandle,
                                  gattAttribute_t *pAttr,
                                  uint8 *pValue, uint16 len,
                                  uint16 offset, uint8 method );

static bStatus_t DSS_sendNotification( uint8 *pValue, uint16 len );

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
CONST gattServiceCBs_t dss_servCBs =
{
  NULL,                           // Read callback function pointer
  DSS_writeAttrCB,                // Write callback function pointer
  NULL                            // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      DSS_addService
 *
 * @brief   This function initializes the Data Stream Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t DSS_addService( void )
{
  bStatus_t status = SUCCESS;

  // Allocate Client Characteristic Configuration table
  dss_dataOut_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS );
  if ( dss_dataOut_config == NULL )
  {
    return ( bleMemAllocError );
  }

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, dss_dataOut_config );

  // Register GATT attribute list and CBs with GATT Server
  status = GATTServApp_RegisterService( dss_attrTbl,
                                        GATT_NUM_ATTRS( dss_attrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &dss_servCBs );

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DSS_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback functions.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t DSS_registerProfileCBs( DSS_cb_t *profileCallback )
{
  if ( profileCallback )
  {
    dss_profileCBs = profileCallback;

    return ( SUCCESS );
  }

  return ( INVALIDPARAMETER );
}

/*********************************************************************
 * @fn      DSS_setParameter
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
bStatus_t DSS_setParameter(uint8 param, void *pValue, uint16 len)
{
  bStatus_t status = SUCCESS;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return ( INVALIDPARAMETER );
  }

  switch ( param )
  {
    case DSS_DATAOUT_ID:
      status = DSS_sendNotification( (uint8 *)pValue, len );
      break;

    default:
      status = INVALIDPARAMETER;
      break;
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DSS_writeAttrCB
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
static bStatus_t DSS_writeAttrCB( uint16 connHandle,
                                        gattAttribute_t *pAttr,
                                        uint8 *pValue, uint16 len,
                                        uint16 offset, uint8 method )
{
  bStatus_t status = SUCCESS;

  // Verify input parameters
  if ( pAttr == NULL || pValue == NULL )
  {
    return ( INVALIDPARAMETER );
  }

  /******************************************************/
  /****** Client Characteristic Configuration ***********/
  /******************************************************/

  if ( ! memcmp( pAttr->type.uuid, clientCharCfgUUID, pAttr->type.len ) )
  {
     DSS_cccUpdate_t *cccUpdate;

    // Allow only notifications
    status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                             offset, GATT_CLIENT_CFG_NOTIFY );

    // Notify profile
    if ( status == SUCCESS )
    {
      // This allocation will be free by bleapp_util
      cccUpdate = (DSS_cccUpdate_t *)ICall_malloc( sizeof( DSS_cccUpdate_t ) );
      if ( cccUpdate == NULL )
      {
        // Return error status
        return ( bleMemAllocError );
      }

      // Copy the data and send it to the profile
      cccUpdate->connHandle = connHandle;
      cccUpdate->value = BUILD_UINT16( pValue[0], pValue[1] );

      // Callback function to notify profile of change on the client characteristic configuration
      BLEAppUtil_invokeFunction( dss_profileCBs->pfnOnCccUpdateCB, (char *)cccUpdate );
    }
  }

  /******************************************************/
  /*********** Data In Characteristic  ******************/
  /******************************************************/
  else if ( ! memcmp( pAttr->type.uuid, dss_dataIn_UUID, pAttr->type.len ) )
  {
    // Only notify profile if there is any data in the payload
    if ( len > 0  && dss_profileCBs && dss_profileCBs->pfnIncomingDataCB)
    {
      DSS_dataIn_t *dataIn;

      if ( len > DSS_MAX_DATA_IN_LEN )
      {
        len = 0;
      }

      // This allocation will be free by bleapp_util
      dataIn = (DSS_dataIn_t *)ICall_malloc( sizeof( DSS_dataIn_t ) + len);
      if ( dataIn == NULL )
      {
        // Return error status
        return ( bleMemAllocError );
      }

      // If allocation was successful,
      // Copy the data and send it to the profile
      if ( len > 0 )
      {
        memcpy( dataIn->pValue, pValue, len );
      }
      dataIn->connHandle = connHandle;
      dataIn->len = len;

      // Callback function to notify profile of change on the client characteristic configuration
      status = BLEAppUtil_invokeFunction( dss_profileCBs->pfnIncomingDataCB, (char *)dataIn );
    }
  }

  // If we get here, that means you've forgotten to add an if clause for a
  // characteristic value attribute in the attribute table that has WRITE permissions.
  else
  {
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      DSS_sendNotification
 *
 * @brief   Transmits data over BLE notifications.
 *
 * @param   pValue - pointer to data to be written
 * @param   len - length of data to be written
 *
 * @return  SUCCESS, or stack call status
 */
static bStatus_t DSS_sendNotification(uint8 *pValue, uint16 len)
{
  bStatus_t status = SUCCESS;
  gattAttribute_t *pAttr = NULL;
  attHandleValueNoti_t noti = {0};
  linkDBInfo_t connInfo = {0};
  uint16 offset = 0;
  uint8 i = 0;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return ( INVALIDPARAMETER );
  }

  // Find the characteristic value attribute
  pAttr = GATTServApp_FindAttr(dss_attrTbl, GATT_NUM_ATTRS(dss_attrTbl), &dss_dataOut_val);
  if ( pAttr != NULL )
  {

    // Check the ccc value for each BLE connection
    for ( i = 0; i < MAX_NUM_BLE_CONNS; i++ )
    {
      gattCharCfg_t *pItem = &( dss_dataOut_config[i] );

      // If the connection has register for notifications
      if ( ( pItem->connHandle != LINKDB_CONNHANDLE_INVALID ) &&
           ( pItem->value == GATT_CLIENT_CFG_NOTIFY) )
      {
        // Find out what the maximum MTU size is for each connection
        status = linkDB_GetInfo(pItem->connHandle, &connInfo);
        offset = 0;

        while ( status != bleTimeout && status != bleNotConnected &&  len > offset )
        {
          // Determine allocation size
          uint16_t allocLen = (len - offset);
          if ( allocLen > ( connInfo.MTU - DSS_NOTI_HDR_SIZE ) )
          {
            // If len > MTU split data to chunks of MTU size
            allocLen = connInfo.MTU - DSS_NOTI_HDR_SIZE;
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

              // Failed to send notification, print error message
              MenuModule_printf(APP_MENU_PROFILE_STATUS_LINE4, 0,
                                "Failed to send notification - Error: " MENU_MODULE_COLOR_RED "%d " MENU_MODULE_COLOR_RESET,
                                 status);
            }
            else
            {
              // Increment data offset
              offset += allocLen;

              // Notification sent
              MenuModule_printf(APP_MENU_PROFILE_STATUS_LINE4, 0,
                                "Notification sent");
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
