/******************************************************************************

 @file  health_thermometer_server.c

 @brief This file contains the health thermometer service sample for use with the Basic BLE
         Profiles sample application.

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

#include <health_thermometer_server.h>
#include <ti/bleapp/menu_module/menu_module.h>
#include <app_main.h>
#include "ble_stack_api.h"

/*********************************************************************
 * DEFINES
 */
// The size of the indication header is opcode + handle
#define HTS_IND_HDR_SIZE   (ATT_OPCODE_SIZE + 2)

// Position of CCC values in the attribute array
#define HTS_MEAS_CONFIG_POS        3

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
// HTS Service UUID: 0x1809
GATT_BT_UUID( hts_serv_UUID, HTS_SERV_UUID );

// HTS Measurement Characteristic UUID: 0x2A1C
GATT_BT_UUID( hts_meas_UUID, HTS_MEAS_UUID );

static HTS_cb_t *hts_profileCBs = NULL;

/*********************************************************************
 * Service Attributes - variables
 */
// HTS Service declaration
static CONST gattAttrType_t hts_service = { ATT_BT_UUID_SIZE, hts_serv_UUID };

// HTS Temperature Measurement
static uint8 hts_meas_props = GATT_PROP_INDICATE;               // Characteristic properties
static uint8 hts_meas_val = 0;                                  // Value variable
static uint8 hts_meas_userDesp[] = "Temperature Measurement";   // User description
static gattCharCfg_t *hts_meas_config;                          // Client Characteristic Configuration

/*********************************************************************
 * Profile Attributes - Table
 */
static gattAttribute_t hts_attrTbl[] =
{
 /*----------------------type---------------------*/ /*------------------permissions-------------------*/ /*---------------pValue---------------*/
   // hts Service
   GATT_BT_ATT( primaryServiceUUID,                    GATT_PERMIT_READ,                                    (uint8 *) &hts_service ),

   // HTS Temperature Measurement Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &hts_meas_props ),
   // HTS Temperature Measurement Value
   GATT_BT_ATT( hts_meas_UUID,                         0,                                                   &hts_meas_val ),
   // HTS Temperature Measurement configuration
   GATT_BT_ATT( clientCharCfgUUID,                     GATT_PERMIT_READ | GATT_PERMIT_WRITE,                (uint8 *) &hts_meas_config ),
   // HTS Temperature Measurement User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    hts_meas_userDesp ),
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static bStatus_t HTS_writeAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                   uint8 *pValue, uint16 len, uint16 offset,
                                   uint8 method );
static bStatus_t HTS_cCCWriteReq( uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 len, uint16 offset,
                                     uint16 validCfg, InvokeFromBLEAppUtilContext_t callback );
static bStatus_t HTS_sendInd( uint8 *pValue, uint16 len, gattCharCfg_t *charCfgTbl, uint8 *pAttValue );
static bStatus_t HTS_initCCC( void );
void HT_gattEventHandler(uint32 event, BLEAppUtil_msgHdr_t *pMsgData);

/*********************************************************************
 * Globals
*/
BLEAppUtil_EventHandler_t htGATTHandler =
{
    .handlerType    = BLEAPPUTIL_GATT_TYPE,
    .pEventHandler  = HT_gattEventHandler,
    .eventMask      = BLEAPPUTIL_ATT_HANDLE_VALUE_CFM
};

/*********************************************************************
 * Service CALLBACKS
 */

/*
 * HTS Server Callbacks
 * \note When an operation on a characteristic requires authorization and
 * pfnAuthorizeAttrCB is not defined for that characteristic's service, the
 * Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
 * operation on a characteristic requires authorization the Stack will call
 * pfnAuthorizeAttrCB to check a client's authorization prior to calling
 * pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
 * made within these functions.
 *
 */
CONST gattServiceCBs_t hts_servCB =
{
  NULL,             // Read callback function pointer
  HTS_writeAttrCB,  // Write callback function pointer
  NULL              // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      HT_gattEventHandler
 *
 * @brief   The purpose of this function is to handle GATT related
 *          events that rise from the GATT and were registered in
 *          @ref BLEAppUtil_registerEventHandler
 *
 * @param   event - message event.
 * @param   pMsgData - pointer to message data.
 *
 * @return  none
 */
void HT_gattEventHandler(uint32 event, BLEAppUtil_msgHdr_t *pMsgData)
{
    gattMsgEvent_t *gattMsg = (gattMsgEvent_t *)pMsgData;
    switch(event)
    {
        // An indication confirmation packet was received
        case BLEAPPUTIL_ATT_HANDLE_VALUE_CFM:
        {
            MenuModule_printf( APP_MENU_PROFILE_STATUS_LINE4, 0,
                               "Health Thermometer server: indication confirmation packet was received on connectionHandle: "
                               MENU_MODULE_COLOR_YELLOW "%d " MENU_MODULE_COLOR_RESET, gattMsg->connHandle );
        }

        default:
        {
            break;
        }
    }
}

/*********************************************************************
 * @fn      HTS_addService
 *
 * @brief   This function initializes the Health Thermometer Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t HTS_addService( void )
{
  uint8 status = SUCCESS;

  // Allocate Client Characteristic Configuration table
  status = HTS_initCCC();
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Register GATT attribute list and CBs with GATT Server
  status = GATTServApp_RegisterService( hts_attrTbl,
                                        GATT_NUM_ATTRS( hts_attrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &hts_servCB );

  // Register Gatt handler of HT
  status = BLEAppUtil_registerEventHandler(&htGATTHandler);
  if( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }
  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      HTS_initCCC
 *
 * @brief   Allocate and initialize Client Characteristic Configuration table
 *
 * @return  SUCCESS, or bleMemAllocError
 */
static bStatus_t HTS_initCCC( void )
{
  // Allocate client characteristic configuration table for HTS Temperature Measurement
  hts_meas_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS );
  if ( hts_meas_config == NULL )
  {
    // Allocation fails
    // Return error status
    return ( bleMemAllocError );
  }

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, hts_meas_config );

  return ( SUCCESS );
}

/*********************************************************************
 * @fn      HTS_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t HTS_registerProfileCBs( HTS_cb_t *profileCallback )
{
  bStatus_t status = SUCCESS;

  if ( profileCallback != NULL )
  {
    hts_profileCBs = profileCallback;
  }
  else
  {
    status = INVALIDPARAMETER;
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      HTS_setParameter
 *
 * @brief   Set a HT Service parameter.
 *
 * @param   param - Characteristic UUID
 * @param   pValue - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 * @param   len - length of data to write
 *`
 * @return  SUCCESS or error status
 */

bStatus_t HTS_setParameter(uint8 param, void *pValue, uint16 len)
{
  bStatus_t status = bleInvalidRange;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return ( INVALIDPARAMETER );
  }

  switch ( param )
  {
    // HT Measurement
    case HTS_MEAS_ID:
      if ( len >=  HTS_MEAS_MIN_LEN &&  len <= HTS_MEAS_MAX_LEN )
      {
        // Send data over BLE Indication
        status = HTS_sendInd(pValue, len, hts_meas_config, &hts_meas_val);
      }
      break;

    default:
      status = INVALIDPARAMETER;
      break;
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      HTS_writeAttrCB
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
static bStatus_t HTS_writeAttrCB( uint16 connHandle,
                                   gattAttribute_t *pAttr,
                                   uint8 *pValue, uint16 len,
                                   uint16 offset, uint8 method )
{
  bStatus_t status = SUCCESS;

  /******************************************************/
  /****** Client Characteristic Configuration ***********/
  /******************************************************/

  if ( ! memcmp( pAttr->type.uuid, clientCharCfgUUID, pAttr->type.len ) )
  {
    // HT Measurements
    if (( pAttr->handle == hts_attrTbl[HTS_MEAS_CONFIG_POS].handle ) &&
        ( hts_profileCBs && hts_profileCBs->pfnMeasOnCccUpdateCB ))
    {

      status = HTS_cCCWriteReq( connHandle, pAttr, pValue, len, offset,
                                GATT_CLIENT_CFG_INDICATE, hts_profileCBs->pfnMeasOnCccUpdateCB );
    }
  }
  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      HTS_sendInd
 *
 * @brief   Transmits data over BLE Indication.
 *
 * @param   pValue - pointer to data to be written
 * @param   len - length of data to be written
 * @param   charCfgTbl - characteristic configuration table.
 * @param   pAttValue - pointer to attribute value.
 *
 *
 * @return  SUCCESS, or stack call status
 */
static bStatus_t HTS_sendInd(uint8 *pValue, uint16 len, gattCharCfg_t *charCfgTbl, uint8 *pAttValue)
{
  bStatus_t status = SUCCESS;
  gattAttribute_t *pAttr = NULL;
  attHandleValueInd_t ind = {0};
  linkDBInfo_t connInfo = {0};
  uint16 offset = 0;
  uint8 i = 0;

  // Verify input parameters
  if (( charCfgTbl == NULL ) || ( pValue == NULL ) || ( pAttValue == NULL ))
  {
    return ( INVALIDPARAMETER );
  }

  // Find the characteristic value attribute
  pAttr = GATTServApp_FindAttr(hts_attrTbl, GATT_NUM_ATTRS(hts_attrTbl), pAttValue);
  if ( pAttr != NULL )
  {
    for ( i = 0; i < MAX_NUM_BLE_CONNS; i++ )
    {
      gattCharCfg_t *pItem = &( charCfgTbl[i] );
      if ( ( pItem->connHandle != LINKDB_CONNHANDLE_INVALID ) &&
           ( pItem->value == GATT_CLIENT_CFG_INDICATE) )
      {
        // Find out what the maximum MTU size is for each connection
        status = linkDB_GetInfo(pItem->connHandle, &connInfo);
        offset = 0;

        while ( status != bleTimeout && status != bleNotConnected &&  len > offset )
          // Determine allocation size
        {
          uint16_t allocLen = (len - offset);
          if ( allocLen > ( connInfo.MTU - HTS_IND_HDR_SIZE ) )
          {
            // If len > MTU split data to chunks of MTU size
            allocLen = connInfo.MTU - HTS_IND_HDR_SIZE;
          }

          ind.len = allocLen;
          ind.pValue = (uint8 *)GATT_bm_alloc( pItem->connHandle, ATT_HANDLE_VALUE_IND, allocLen, 0 );
          if ( ind.pValue != NULL )
          {
            // If allocation was successful, copy out data and send it
            VOID memcpy(ind.pValue, pValue + offset, ind.len);
            ind.handle = pAttr->handle;
            // Send the data over BLE indication
            status = GATT_Indication( pItem->connHandle, (attHandleValueInd_t *)&ind, FALSE, BLEAppUtil_getSelfEntity());
            // If unable to send the data, free allocated buffers and return
            if ( status != SUCCESS )
            {
              GATT_bm_free( (gattMsg_t *)&ind, ATT_HANDLE_VALUE_IND );

              // Failed to send indication, print error message
              MenuModule_printf(APP_MENU_PROFILE_STATUS_LINE3, 0,
                                "Health Thermometer server: Failed to send indication - Error: " MENU_MODULE_COLOR_RED "%d " MENU_MODULE_COLOR_RESET,
                                 status);
            }
            else
            {
              // Increment data offset
              offset += allocLen;

              // Indication sent
              MenuModule_printf(APP_MENU_PROFILE_STATUS_LINE3, 0,
                                "Health Thermometer server: Indication sent");
            }
          }
          else //  End of "if ( ind.pValue != NULL )"
          {
            status = bleNoResources;
          }
        } // End of "while ( status != bleTimeout && status != bleNotConnected &&  len > offset )"
      }
    } // End of "for ( i = 0; i < MAX_NUM_BLE_CONNS; i++ )"
  } // End of "if ( pAttr != NULL )"

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      HTS_cCCWriteReq
 *
 * @brief   Process CCC Write request and update the profile
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   validCfg - valid configuration
 * @param   callback - profile callback function
 *
 * @return  SUCCESS or stack call status
 */
static bStatus_t HTS_cCCWriteReq (uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 len, uint16 offset,
                                     uint16 validCfg, InvokeFromBLEAppUtilContext_t callback)
{
  bStatus_t status = SUCCESS;
  HTS_cccUpdate_t *cccUpdate = NULL;

  // Process CCC Write request
  status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len, offset, validCfg );

  if ( status == SUCCESS )
  {
    // This allocation will be free by bleapp_util
    cccUpdate = (HTS_cccUpdate_t *)ICall_malloc( sizeof( HTS_cccUpdate_t ) );
    if ( cccUpdate != NULL )
    {
      // Copy the data and send it to the profile
      cccUpdate->connHandle = connHandle;
      cccUpdate->value = BUILD_UINT16( pValue[0], pValue[1] );

      // Callback function to notify profile of change on the client characteristic configuration
      BLEAppUtil_invokeFunction( callback, (char *)cccUpdate );
    }
  }

  // Return status value
  return ( status );
}

/*********************************************************************
*********************************************************************/
