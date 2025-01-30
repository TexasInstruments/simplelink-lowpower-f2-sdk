/******************************************************************************

 @file  glucose_server.c

 @brief This file contains the glucose service sample for use with the Basic BLE
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
#include "osal_clock.h"

/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include <ti/bleapp/services/glucose/glucose_server.h>
#include <ti/bleapp/menu_module/menu_module.h>
#include <app_main.h>
#include "ble_stack_api.h"

/*********************************************************************
 * DEFINES
 */
// The size of the notification header is opcode + handle
#define GLS_NOTI_HDR_SIZE   (ATT_OPCODE_SIZE + 2)

// Position of CCC values in the attribute array
#define GLS_MEAS_CONFIG_POS        3
#define GLS_RACP_CONFIG_POS        10

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// GLS Service UUID: 0x180
GATT_BT_UUID( gls_serv_UUID, GLS_SERV_UUID );

// GLS Measurement Characteristic UUID: 0x2A18
GATT_BT_UUID( gls_meas_UUID, GLS_MEAS_UUID );

// GLS Feature Characteristic UUID: 0x2A51
GATT_BT_UUID(gls_feat_UUID, GLS_FEAT_UUID );

// GLS Record Access Control Characteristic UUID: 0x2A52
GATT_BT_UUID(gls_racp_UUID, GLS_RACP_UUID );

static GLS_cb_t *gls_profileCBs = NULL;

/*********************************************************************
 * Service Attributes - variables
 */

// GLS Service declaration
static CONST gattAttrType_t gls_service = { ATT_BT_UUID_SIZE, gls_serv_UUID };

// GLS Measurement
static uint8 gls_meas_props = GATT_PROP_NOTIFY;    // Characteristic properties
static uint8 gls_meas_val = 0;                     // Value variable
static uint8 gls_meas_userDesp[] = "Measurement";  // User description
static gattCharCfg_t *gls_meas_config;             // Client Characteristic Configuration

// GLS Feature
static uint8 gls_feat_props = GATT_PROP_READ;      // Characteristic properties
static uint16 gls_feat_val = 0;                    // Value variable
static uint8 gls_feat_userDesp[] = "Feature";      // User description

// Record Access Control Point
static uint8 gls_racp_props = GATT_PROP_INDICATE | GATT_PROP_WRITE;  // Characteristic properties
static uint8 gls_racp_val = 0;                                       //  Value variable
static uint8 gls_racp_userDesp[] = "Record Access Control Point";    // User description
static gattCharCfg_t *gls_racp_config;                               // Client Characteristic Configuration

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t gls_attrTbl[] =
{
 /*----------------------type---------------------*/ /*------------------permissions-------------------*/ /*---------------pValue---------------*/
   // gls Service
   GATT_BT_ATT( primaryServiceUUID,                    GATT_PERMIT_READ,                                    (uint8 *) &gls_service ),

   // gls Measurement Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &gls_meas_props ),
   // gls Measurement Value
   GATT_BT_ATT( gls_meas_UUID,                         0,                                                   &gls_meas_val ),
   // gls Measurement configuration
   GATT_BT_ATT( clientCharCfgUUID,                     GATT_PERMIT_READ | GATT_PERMIT_WRITE,                (uint8 *) &gls_meas_config ),
   // gls Measurement User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    gls_meas_userDesp ),

   // gls Feature Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &gls_feat_props ),
   // gls Feature Value
   GATT_BT_ATT( gls_feat_UUID,                        GATT_PERMIT_AUTHEN_READ,                              (uint8 *) &gls_feat_val ),
   // gls Feature User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    gls_feat_userDesp ),

   // Record Access Control Point Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &gls_racp_props ),
   // Record Access Control Point Value
   GATT_BT_ATT( gls_racp_UUID,                        GATT_PERMIT_AUTHEN_WRITE,                             &gls_racp_val ),
   // Record Access Control Point configuration
   GATT_BT_ATT( clientCharCfgUUID,                     GATT_PERMIT_READ | GATT_PERMIT_WRITE,                (uint8 *) &gls_racp_config ),
   // Record Access Control Point User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    gls_racp_userDesp ),
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static bStatus_t GLS_readAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                  uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                  uint16_t maxLen, uint8_t method );
static bStatus_t GLS_writeAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                   uint8 *pValue, uint16 len, uint16 offset,
                                   uint8 method );
static bStatus_t GLS_cCCWriteReq( uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 len, uint16 offset,
                                     uint16 validCfg, InvokeFromBLEAppUtilContext_t callback );
static bStatus_t GLS_sendNotiInd( uint8 *pValue, uint16 len, gattCharCfg_t *charCfgTbl, uint8 *pAttValue );
static bStatus_t GLS_initCCC( void );

/*********************************************************************
 * Service CALLBACKS
 */

/*
 * gls Server Callbacks
 * \note When an operation on a characteristic requires authorization and
 * pfnAuthorizeAttrCB is not defined for that characteristic's service, the
 * Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
 * operation on a characteristic requires authorization the Stack will call
 * pfnAuthorizeAttrCB to check a client's authorization prior to calling
 * pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
 * made within these functions.
 *
 */
CONST gattServiceCBs_t gls_servCB =
{
  GLS_readAttrCB,   // Read callback function pointer
  GLS_writeAttrCB,  // Write callback function pointer
  NULL              // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      GLS_addService
 *
 * @brief   This function initializes the Glucose Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GLS_addService( void )
{
  uint8 status = SUCCESS;

  // Allocate Client Characteristic Configuration table
  status = GLS_initCCC();
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Register GATT attribute list and CBs with GATT Server
  status = GATTServApp_RegisterService( gls_attrTbl,
                                        GATT_NUM_ATTRS( gls_attrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &gls_servCB );

  // Return status value
  return ( status );
}


/*********************************************************************
 * @fn      GLS_initCCC
 *
 * @brief   Allocate and initialize Client Characteristic Configuration table
 *
 * @return  SUCCESS, or bleMemAllocError
 */
static bStatus_t GLS_initCCC( void )
{
  // Allocate client characteristic configuration table for GLS Measurement
  gls_meas_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS );
  if ( gls_meas_config == NULL )
  {
    // Allocation fails
    // Return error status
    return ( bleMemAllocError );
  }

  // Allocate client characteristic configuration table for Record Access Control Point
  gls_racp_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS );
  if ( gls_racp_config == NULL )
  {
    // Allocation fails
    // Free allocated variables
    ICall_free ( gls_meas_config );

    // Return error status
    return ( bleMemAllocError );
  }

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, gls_meas_config );
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, gls_racp_config );

  return ( SUCCESS );
}

/*********************************************************************
 * @fn      GLS_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t GLS_registerProfileCBs( GLS_cb_t *profileCallback )
{
  bStatus_t status = SUCCESS;

  if ( profileCallback )
  {
    gls_profileCBs = profileCallback;
  }
  else
  {
    status = INVALIDPARAMETER;
  }

  // Return status value
  return ( status );
}


/*********************************************************************
 * @fn      GLS_setParameter
 *
 * @brief   Set a GLSC Service parameter.
 *
 * @param   param - Characteristic UUID
 * @param   pValue - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 * @param   len - length of data to write
 *
 * @return  SUCCESS or error status
 */

bStatus_t GLS_setParameter(uint8 param, void *pValue, uint16 len)
{
  bStatus_t status = bleInvalidRange;

  // Verify input parameters
  if ( pValue == NULL)
  {
    return ( INVALIDPARAMETER );
  }

  switch ( param )
  {
    // GL Measurement
    case GLS_MEAS_ID:
      if ( len >=  GLS_MEAS_MIN_RECORD_LEN && len <= GLS_MEAS_MAX_RECORD_LEN)
      {
        // Send data over BLE notification
        status = GLS_sendNotiInd(pValue, len, gls_meas_config, &gls_meas_val);
      }
      break;

    // GL Feature
    case GLS_FEAT_ID:
      if ( len ==  GLS_FEAT_LEN)
      {
        status = SUCCESS;
        VOID memcpy( &gls_feat_val, pValue, GLS_FEAT_LEN );
      }
      break;

    // Record Access Control Point
    case GLS_RACP_ID:
      if ( len == GLS_RACP_RSP_LEN )
      {
        // Send data over BLE indication
        status = GLS_sendNotiInd( pValue, len, gls_racp_config, &gls_racp_val);
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
 * @fn      GLS_getParameter
 *
 * @brief   Get a GL parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  SUCCESS or error status
 */
bStatus_t GLS_getParameter( uint8 param, void *pValue )
{
  bStatus_t status;
  switch ( param )
  {
    // GL Feature
    case GLS_FEAT_ID:
      VOID memcpy( pValue, &gls_feat_val, GLS_FEAT_LEN);
      status = SUCCESS;
      break;

    default:
      status = INVALIDPARAMETER;
      break;
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn          GLS_readAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t GLS_readAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                           uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                           uint16_t maxLen, uint8_t method )
{
  bStatus_t status = SUCCESS;

  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }

  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

    switch ( uuid )
    {
      // GL Feature
      case GLS_FEAT_UUID:
        *pLen = 2;
         VOID memcpy( pValue, &gls_feat_val, 2);
         break;

       // gattserverapp handles those reads
      default:
        status = INVALIDPARAMETER;
        break;
    }
  }
  // Wrong Size
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  // Return status value
  return ( status );
}


/*********************************************************************
 * @fn      GLS_writeAttrCB
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
static bStatus_t GLS_writeAttrCB( uint16 connHandle,
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
    // GL Measurements
    if (( pAttr->handle == gls_attrTbl[GLS_MEAS_CONFIG_POS].handle ) &&
        ( gls_profileCBs && gls_profileCBs->pfnMeasOnCccUpdateCB ))
    {

      status = GLS_cCCWriteReq( connHandle, pAttr, pValue, len, offset,
                                GATT_CLIENT_CFG_NOTIFY, gls_profileCBs->pfnMeasOnCccUpdateCB );
    }

    // Record Access Control Point
    else if (( pAttr->handle == gls_attrTbl[GLS_RACP_CONFIG_POS].handle ) &&
             ( gls_profileCBs && gls_profileCBs->pfnRACPOnCccUpdateCB ))
    {

      status = GLS_cCCWriteReq( connHandle, pAttr, pValue, len, offset,
                                GATT_CLIENT_CFG_INDICATE, gls_profileCBs->pfnRACPOnCccUpdateCB );
    }
  }

  /******************************************************/
  /******** Record Access Control Point *****************/
  /******************************************************/
  else if ( ! memcmp( pAttr->type.uuid, gls_racp_UUID, pAttr->type.len ) )
  {
    // In case the CCCD is improperly configured
    if (gls_racp_config[connHandle].value != GATT_CLIENT_CFG_INDICATE)
    {
      status = GLS_RACP_CCCD_IMPROPERLY_CONFG;
    }

    // In case a RACP procedure is already in progress
    else if (( len >= GLS_RACP_REQ_MIN_LEN ) &&
        ( pValue[0] == RACP_OPCODE_REPORT_STORED_RECORDS ) &&
        ( glp_racpProc & RACP_PROC_INPROGRESS ))
    {
      status = GLS_RACP_PROC_IN_PROG;
    }

    // Only notify profile if the data is in appropriate size
    else if (( len >= GLS_RACP_REQ_MIN_LEN &&  len <= GLS_RACP_REQ_MAX_LEN ) &&
             ( gls_profileCBs && gls_profileCBs->pfnRACPReqCB ))
    {
      GLS_racpReq_t *racpReq = NULL;

      // This allocation will be free by bleapp_util
      racpReq = (GLS_racpReq_t *)ICall_malloc( sizeof( GLS_racpReq_t ));
      if ( racpReq != NULL )
      {
        memset( racpReq, 0, sizeof( GLS_racpReq_t ));
        // Copy the data and send it to the profile
        VOID memcpy( racpReq, pValue, len );
        // if there is RACP in progress and the peer ask to abort, flag the App to stop sending notifications.
        if((racpReq->opCode == RACP_OPCODE_ABORT_OPERATION) && (glp_racpProc & RACP_PROC_INPROGRESS))
        {
            glp_racpProc |= RACP_PROC_ABORT;
        }
        // Callback function to notify profile of change on RACP characteristic
        BLEAppUtil_invokeFunction( gls_profileCBs->pfnRACPReqCB, (char *)racpReq );
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
 * @fn      GLS_sendNotiInd
 *
 * @brief   Transmits data over BLE Notification/Indication.
 *
 * @param   pValue - pointer to data to be written
 * @param   len - length of data to be written
 * @param   charCfgTbl - characteristic configuration table.
 * @param   pAttValue - pointer to attribute value.
 *
 *
 * @return  SUCCESS, or stack call status
 */
static bStatus_t GLS_sendNotiInd(uint8 *pValue, uint16 len, gattCharCfg_t *charCfgTbl, uint8 *pAttValue)
{
  bStatus_t status = SUCCESS;
  gattAttribute_t *pAttr = NULL;
  attHandleValueNoti_t noti = {0};
  linkDBInfo_t connInfo = {0};
  uint16 offset = 0;
  uint8 i = 0;

  // Verify input parameters
  if (( charCfgTbl == NULL ) || ( pValue == NULL ) || ( pAttValue == NULL ))
  {
    return ( INVALIDPARAMETER );
  }

  // Find the characteristic value attribute
  pAttr = GATTServApp_FindAttr(gls_attrTbl, GATT_NUM_ATTRS(gls_attrTbl), pAttValue);
  if ( pAttr != NULL )
  {
    for ( i = 0; i < MAX_NUM_BLE_CONNS; i++ )
    {
      gattCharCfg_t *pItem = &( charCfgTbl[i] );
      if ( ( pItem->connHandle != LINKDB_CONNHANDLE_INVALID ) &&
           ( pItem->value != GATT_CFG_NO_OPERATION) )
      {
        // Find out what the maximum MTU size is for each connection
        status = linkDB_GetInfo(pItem->connHandle, &connInfo);
        offset = 0;

        while ( status != bleTimeout && status != bleNotConnected &&  len > offset )
          // Determine allocation size
        {
          uint16_t allocLen = (len - offset);
          if ( allocLen > ( connInfo.MTU - GLS_NOTI_HDR_SIZE ) )
          {
            // If len > MTU split data to chunks of MTU size
            allocLen = connInfo.MTU - GLS_NOTI_HDR_SIZE;
          }

          noti.len = allocLen;
          noti.pValue = (uint8 *)GATT_bm_alloc( pItem->connHandle, ATT_HANDLE_VALUE_NOTI, allocLen, 0 );
          if ( noti.pValue != NULL )
          {
            // If allocation was successful, copy out data and send it
            VOID memcpy(noti.pValue, pValue + offset, noti.len);
            noti.handle = pAttr->handle;
            if( pItem->value == GATT_CLIENT_CFG_NOTIFY )
            {
              // Send the data over BLE notifications
              status = GATT_Notification( pItem->connHandle, &noti, TRUE );
            }
            else if ( pItem->value == GATT_CLIENT_CFG_INDICATE )
            {
              // Send the data over BLE indication
              status = GATT_Indication( pItem->connHandle, (attHandleValueInd_t *)&noti, TRUE, INVALID_TASK_ID);
            }
            else
            {
              // If Client Characteristic Configuration is not notifications/indication
              status = INVALIDPARAMETER;
            }

            // If unable to send the data, free allocated buffers and return
            if ( status != SUCCESS )
            {
              GATT_bm_free( (gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI );

              // Failed to send notification/indication, print error message
              MenuModule_printf(APP_MENU_PROFILE_STATUS_LINE4, 0,
                                "Failed to send notification/indication - Error: " MENU_MODULE_COLOR_RED "%d " MENU_MODULE_COLOR_RESET,
                                 status);
            }
            else
            {
              // Increment data offset
              offset += allocLen;

              // Notification/indication sent
              MenuModule_printf(APP_MENU_PROFILE_STATUS_LINE4, 0,
                                "Notification/indication sent");
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
 * @fn      GLS_cCCWriteReq
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
static bStatus_t GLS_cCCWriteReq (uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 len, uint16 offset,
                                     uint16 validCfg, InvokeFromBLEAppUtilContext_t callback)
{
  bStatus_t status = SUCCESS;
  GLS_cccUpdate_t *cccUpdate = NULL;

  // Process CCC Write request
  status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len, offset, validCfg );

  if ( status == SUCCESS )
  {
    // This allocation will be free by bleapp_util
    cccUpdate = (GLS_cccUpdate_t *)ICall_malloc( sizeof( GLS_cccUpdate_t ) );
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
