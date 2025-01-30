/******************************************************************************

 @file  cgm_server.c

 @brief This file contains the CGM service sample for use with the Basic BLE
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

#include <ti/bleapp/services/continuous_glucose_monitoring/cgm_server.h>
#include <ti/bleapp/menu_module/menu_module.h>
#include <app_main.h>
#include "ble_stack_api.h"

/*********************************************************************
 * DEFINES
 */

// The size of the notification header is opcode + handle
#define CGMS_NOTI_HDR_SIZE   (ATT_OPCODE_SIZE + 2)

// Position of CCC values in the attribute array
#define CGMS_MEAS_CONFIG_POS        3
#define CGMS_RACP_CONFIG_POS        19
#define CGMS_CGMCP_CONFIG_POS       23

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// CGM Service UUID: 0x181F
GATT_BT_UUID( cgms_serv_UUID, CGMS_SERV_UUID );

// CGM Measurement Characteristic UUID: 0x2AA7
GATT_BT_UUID( cgms_meas_UUID, CGMS_MEAS_UUID );

// CGM Feature Characteristic UUID: 0x2AA8
GATT_BT_UUID( cgms_feat_UUID, CGMS_FEAT_UUID );

// CGM Status Characteristic UUID: 0x2AA9
GATT_BT_UUID( cgms_stat_UUID, CGMS_STAT_UUID );

// CGM Session Start Time Characteristic UUID: 0x2AAA
GATT_BT_UUID( cgms_sst_UUID, CGMS_SST_UUID );

// CGM Session Run Time Characteristic UUID: 0x2AAB
GATT_BT_UUID( cgms_srt_UUID, CGMS_SRT_UUID );

// Record Access Control Point Characteristic UUID: 0x2A52
GATT_BT_UUID( cgms_racp_UUID, CGMS_RACP_UUID );

// Record Access Control Point Characteristic UUID: 0x2AAC
GATT_BT_UUID( cgms_cgmcp_UUID, CGMS_CGMCP_UUID );

static CGMS_cb_t *cgms_profileCBs = NULL;

/*********************************************************************
 * Service Attributes - variables
 */

// CGM Service declaration
static CONST gattAttrType_t cgms_service = { ATT_BT_UUID_SIZE, cgms_serv_UUID };

// CGM Measurement
static uint8 cgms_meas_props = GATT_PROP_NOTIFY;    // Characteristic properties
static uint8 cgms_meas_val = 0;                     // Value variable
static uint8 cgms_meas_userDesp[] = "Measurement";  // User description
static gattCharCfg_t *cgms_meas_config;             // Client Characteristic Configuration

// CGM Feature
static uint8 cgms_feat_props = GATT_PROP_READ;      // Characteristic properties
static CGMS_feat_t cgms_feat_val = {0};             // Value variable
static uint8 cgms_feat_userDesp[] = "Feature";      // User description

// CGM Status
static uint8 cgms_stat_props = GATT_PROP_READ;      // Characteristic properties
static CGMS_status_t cgms_stat_val = {0};           // Value variable
static uint8 cgms_stat_userDesp[] = "Status";       // User description

//  CGM Session Start Time
static uint8 cgms_sst_props = GATT_PROP_READ | GATT_PROP_WRITE;  // Characteristic properties
static CGMS_sst_t cgms_sst_val = {0};                            // Value variable
static uint8 cgms_sst_userDesp[] = "Status";                     // User description

//  CGM Session Run Time
static uint8 cgms_srt_props = GATT_PROP_READ;       // Characteristic properties
static uint16 cgms_srt_val = 0;                     // Value variable
static uint8 cgms_srt_userDesp[] = "Status";        // User description

// Record Access Control Point
static uint8 cgms_racp_props = GATT_PROP_INDICATE | GATT_PROP_WRITE;  // Characteristic properties
static uint8 cgms_racp_val = 0;                                       //  Value variable
static uint8 cgms_racp_userDesp[] = "Record Access Control Point";    // User description
static gattCharCfg_t *cgms_racp_config;                               // Client Characteristic Configuration

// CGM Specific Ops Control Point
static uint8 cgms_cgmcp_props = GATT_PROP_INDICATE | GATT_PROP_WRITE;  // Characteristic properties
static uint8 cgms_cgmcp_val = 0;                                       //  Value variable
static uint8 cgms_cgmcp_userDesp[] = "Specific Ops Control Point";     // User description
static gattCharCfg_t *cgms_cgmcp_config;                               // Client Characteristic Configuration

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t cgms_attrTbl[] =
{
 /*----------------------type---------------------*/ /*------------------permissions-------------------*/ /*---------------pValue---------------*/
   // CGM Service
   GATT_BT_ATT( primaryServiceUUID,                    GATT_PERMIT_READ,                                    (uint8 *) &cgms_service ),

   // CGM Measurement Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &cgms_meas_props ),
   // CGM Measurement Value
   GATT_BT_ATT( cgms_meas_UUID,                         0,                                                  &cgms_meas_val ),
   // CGM Measurement configuration
   GATT_BT_ATT( clientCharCfgUUID,                     GATT_PERMIT_READ | GATT_PERMIT_AUTHEN_WRITE,         (uint8 *) &cgms_meas_config ),
   // CGM Measurement User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    cgms_meas_userDesp ),

   // CGM Feature Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &cgms_feat_props ),
   // CGM Feature Value
   GATT_BT_ATT( cgms_feat_UUID,                        GATT_PERMIT_AUTHEN_READ,                             (uint8 *) &cgms_feat_val ),
   // CGM Feature User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    cgms_feat_userDesp ),

   // CGM Status Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &cgms_stat_props ),
   // CGM Status Value
   GATT_BT_ATT( cgms_stat_UUID,                        GATT_PERMIT_AUTHEN_READ,                             (uint8 *) &cgms_stat_val ),
   // CGM Status User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    cgms_stat_userDesp ),

   // CGM Session Start Time Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &cgms_sst_props ),
   // CGM Session Start Time Value
   GATT_BT_ATT( cgms_sst_UUID,                         GATT_PERMIT_AUTHEN_READ | GATT_PERMIT_AUTHEN_WRITE,  (uint8 *) &cgms_sst_val ),
   // CGM Session Start Time User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    cgms_sst_userDesp ),

   // CGM Session Run Time Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &cgms_srt_props ),
   // CGM Session Run Time Value
   GATT_BT_ATT( cgms_srt_UUID,                         GATT_PERMIT_AUTHEN_READ,                             (uint8 *) &cgms_srt_val ),
   // CGM Session Run Time User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    cgms_srt_userDesp ),

   // Record Access Control Point Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &cgms_racp_props ),
   // Record Access Control Point Value
   GATT_BT_ATT( cgms_racp_UUID,                        GATT_PERMIT_AUTHEN_WRITE,                            &cgms_racp_val ),
   // Record Access Control Point configuration
   GATT_BT_ATT( clientCharCfgUUID,                     GATT_PERMIT_READ | GATT_PERMIT_AUTHEN_WRITE,         (uint8 *) &cgms_racp_config ),
   // Record Access Control Point User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    cgms_racp_userDesp ),

   // CGM Specific Ops Control Point Properties
   GATT_BT_ATT( characterUUID,                         GATT_PERMIT_READ,                                    &cgms_cgmcp_props ),
   // CGM Specific Ops Control Point Value
   GATT_BT_ATT( cgms_cgmcp_UUID,                       GATT_PERMIT_AUTHEN_WRITE,                            &cgms_cgmcp_val ),
   // CGM Specific Ops Control Point configuration
   GATT_BT_ATT( clientCharCfgUUID,                     GATT_PERMIT_READ | GATT_PERMIT_AUTHEN_WRITE,         (uint8 *) &cgms_cgmcp_config ),
   // CGM Specific Ops Control Point User Description
   GATT_BT_ATT( charUserDescUUID,                      GATT_PERMIT_READ,                                    cgms_cgmcp_userDesp ),
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t CGMS_readAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                  uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                  uint16_t maxLen, uint8_t method );
static bStatus_t CGMS_writeAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                   uint8 *pValue, uint16 len, uint16 offset,
                                   uint8 method );
static bStatus_t CGMS_cCCWriteReq( uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 len, uint16 offset,
                                     uint16 validCfg, InvokeFromBLEAppUtilContext_t callback );
static bStatus_t CGMS_sendNotiInd( uint8 *pValue, uint16 len, gattCharCfg_t *charCfgTbl, uint8 *pAttValue );
static bStatus_t CGMS_initCCC( void );
static bStatus_t CGM_sstIsValid( uint8 *pValue );
static void CGM_sstUpdate( uint8 *pValue );



/*********************************************************************
 * Service CALLBACKS
 */

/*
 * CGM Server Callbacks
 * \note When an operation on a characteristic requires authorization and
 * pfnAuthorizeAttrCB is not defined for that characteristic's service, the
 * Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
 * operation on a characteristic requires authorization the Stack will call
 * pfnAuthorizeAttrCB to check a client's authorization prior to calling
 * pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
 * made within these functions.
 *
 */
CONST gattServiceCBs_t cgms_servCB =
{
  CGMS_readAttrCB,   // Read callback function pointer
  CGMS_writeAttrCB,  // Write callback function pointer
  NULL               // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      CGMS_addService
 *
 * @brief   This function initializes the Data Stream Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMS_addService( void )
{
  uint8 status = SUCCESS;

  // Allocate Client Characteristic Configuration table
  status = CGMS_initCCC();
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Register GATT attribute list and CBs with GATT Server
  status = GATTServApp_RegisterService( cgms_attrTbl,
                                        GATT_NUM_ATTRS( cgms_attrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &cgms_servCB );

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      CGMS_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t CGMS_registerProfileCBs( CGMS_cb_t *profileCallback )
{
  bStatus_t status = SUCCESS;

  if ( profileCallback )
  {
    cgms_profileCBs = profileCallback;
  }
  else
  {
    status = INVALIDPARAMETER;
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      CGMS_setParameter
 *
 * @brief   Set a CGM Service parameter.
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
bStatus_t CGMS_setParameter(uint8 param, void *pValue, uint16 len)
{
  bStatus_t status = bleInvalidRange;

  // Verify input parameters
  if ( pValue == NULL)
  {
    return ( INVALIDPARAMETER );
  }

  switch ( param )
  {
    // CGM Measurement
    case CGMS_MEAS_ID:
      if ( len >=  CGMS_MEAS_MIN_RECORD_LEN && len <= CGMS_MEAS_MAX_RECORD_LEN)
      {
        // Send data over BLE notification
        status = CGMS_sendNotiInd(pValue, len, cgms_meas_config, &cgms_meas_val);
      }
      break;

    // CGM Feature
    case CGMS_FEAT_ID:
      if ( len ==  CGMS_FEAT_LEN)
      {
        status = SUCCESS;
        VOID memcpy( &cgms_feat_val, pValue, CGMS_FEAT_LEN );
      }
      break;

    // CGM Status
    case CGMS_STAT_ID:
      if ( len ==  CGMS_STAT_LEN)
      {
        status = SUCCESS;
        VOID memcpy( &cgms_stat_val, pValue, CGMS_STAT_LEN );
      }
      break;

    // Set CGM SST
    case CGMS_SST_ID:
      if ( len == CGMS_SST_LEN )
      {
        status = SUCCESS;
        VOID memcpy( &cgms_sst_val, pValue, CGMS_SST_LEN );
      }
      break;

    // CGM SRT
    case CGMS_SRT_ID:
      if ( len == CGMS_SRT_LEN)
      {
        status = SUCCESS;
        VOID memcpy( &cgms_srt_val, pValue, CGMS_SRT_LEN );
      }
      break;

    // Record Access Control Point
    case CGMS_RACP_ID:
      if ( len == CGMS_RACP_RSP_LEN )
      {
        // Send data over BLE indication
        status = CGMS_sendNotiInd( pValue, len, cgms_racp_config, &cgms_racp_val);
      }
      break;

    // CGM Specific Ops Control Point
    case CGMS_CGMCP_ID:
      if ( len == CGMS_CGMCP_RSP_GENERAL_LEN || len == CGMS_CGMCP_RSP_COMM_INT_LEN )
      {
        // Send data over BLE indication
        status = CGMS_sendNotiInd( pValue, len, cgms_cgmcp_config, &cgms_cgmcp_val);
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
 * @fn      CGMS_getParameter
 *
 * @brief   Get a CGM parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  SUCCESS or error status
 */
bStatus_t CGMS_getParameter( uint8 param, void *pValue )
{
  bStatus_t status = SUCCESS;
  switch ( param )
  {
    // CGM Feature
    case CGMS_FEAT_ID:
      VOID memcpy( pValue, &cgms_feat_val, CGMS_FEAT_LEN);
      break;

    // CGM Status
    case CGMS_STAT_ID:
      VOID memcpy( pValue, &cgms_stat_val, CGMS_STAT_LEN);
      break;

    // CGM SST
    case CGMS_SST_ID:
      VOID memcpy( pValue, &cgms_sst_val, CGMS_SST_LEN);
      break;

    // CGM SRT
    case CGMS_SRT_ID:
      VOID memcpy( pValue, &cgms_srt_val,CGMS_SRT_LEN);
      break;

    default:
      status = INVALIDPARAMETER;
      break;
  }

  // Return status value
  return ( status );
}
/*********************************************************************
 * @fn          CGMS_readAttrCB
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
static bStatus_t CGMS_readAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
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
      // CGM Feature
      case CGMS_FEAT_UUID:
        *pLen = 6;
         VOID memcpy( pValue, &cgms_feat_val.cgmFeat, 3);
         VOID memcpy( pValue+3, &cgms_feat_val.typeLoc, 1);
         VOID memcpy( pValue+4, &cgms_feat_val.e2eCrc, 2);

         break;

       // CGM Status
       case CGMS_STAT_UUID:
         *pLen = CGMS_STAT_LEN;
         VOID memcpy( pValue, pAttr->pValue, CGMS_STAT_LEN );
         break;

       // CGM Session Start Time
       case CGMS_SST_UUID:
         *pLen = CGMS_SST_LEN;
         VOID memcpy( pValue, &cgms_sst_val, CGMS_SST_LEN );
         break;

       // CGM Session Run Time
       case CGMS_SRT_UUID:
         *pLen = CGMS_SRT_LEN;
         VOID memcpy( pValue, pAttr->pValue, CGMS_SRT_LEN );
         break;

       // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
       // gattserverapp handles those reads
      default:
        status = INVALIDPARAMETER;
        break;
    }
  }

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
 * @fn      CGMS_writeAttrCB
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
static bStatus_t CGMS_writeAttrCB( uint16 connHandle,
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
    // CGM Measurements
    if (( pAttr->handle == cgms_attrTbl[CGMS_MEAS_CONFIG_POS].handle ) &&
        ( cgms_profileCBs && cgms_profileCBs->pfnMeasOnCccUpdateCB ))
    {
      status = CGMS_cCCWriteReq( connHandle, pAttr, pValue, len, offset,
                                 GATT_CLIENT_CFG_NOTIFY, cgms_profileCBs->pfnMeasOnCccUpdateCB );
    }

    // Record Access Control Point
    else if (( pAttr->handle == cgms_attrTbl[CGMS_RACP_CONFIG_POS].handle ) &&
             ( cgms_profileCBs && cgms_profileCBs->pfnRACPOnCccUpdateCB ))
    {
      status = CGMS_cCCWriteReq( connHandle, pAttr, pValue, len, offset,
                                 GATT_CLIENT_CFG_INDICATE, cgms_profileCBs->pfnRACPOnCccUpdateCB );
    }

    // CGM Specific Ops Control Point
    else if (( pAttr->handle == cgms_attrTbl[CGMS_CGMCP_CONFIG_POS].handle ) &&
             ( cgms_profileCBs && cgms_profileCBs->pfnCGMCPOnCccUpdateCB ))
    {
      status = CGMS_cCCWriteReq( connHandle, pAttr, pValue, len, offset,
                                 GATT_CLIENT_CFG_INDICATE, cgms_profileCBs->pfnCGMCPOnCccUpdateCB );
    }
  }

  /******************************************************/
  /****** Session Start Time Characteristic *************/
  /******************************************************/

  else if ( ! memcmp( pAttr->type.uuid, cgms_sst_UUID, pAttr->type.len ))
  {
    // Verify input parameters
    if ( len == CGMS_SST_LEN && CGM_sstIsValid( pValue ))
    {
      // Update and save SST
      CGM_sstUpdate( pValue );

      if ( cgms_profileCBs && cgms_profileCBs->pfnSSTUpdateCB )
      {
        // This allocation will be free by bleapp_util
        char *sst = (char *)ICall_malloc( CGMS_SST_LEN );
        if( sst != NULL )
        {
          // Copy the data
          VOID memcpy ( sst, pValue, CGMS_SST_LEN );

          // Call profile function from stack task context.
          BLEAppUtil_invokeFunction( cgms_profileCBs->pfnSSTUpdateCB, sst );
        }
      }
    }
    else
    {
      return ( CGMS_SST_NOT_VALID );
    }
  }

  /******************************************************/
  /******** Record Access Control Point *****************/
  /******************************************************/
  else if ( ! memcmp( pAttr->type.uuid, cgms_racp_UUID, pAttr->type.len ) )
  {
    // In case the CCCD is improperly configured
    if (cgms_racp_config[connHandle].value != GATT_CLIENT_CFG_INDICATE)
    {
      status = CGMS_RACP_CCCD_IMPROPERLY_CONFG;
    }

    // In case a RACP procedure is already in progress
    else if (( len >= CGMS_RACP_REQ_MIN_LEN ) &&
        ( pValue[0] == RACP_OPCODE_REPORT_STORED_RECORDS ) &&
        ( cgmp_racpProc & RACP_PROC_INPROGRESS ))
    {
      status = CGMS_RACP_PROC_IN_PROG;
    }

    // Only notify profile if the data is in appropriate size
    else if (( len >= CGMS_RACP_REQ_MIN_LEN &&  len <= CGMS_RACP_REQ_MAX_LEN ) &&
             ( cgms_profileCBs && cgms_profileCBs->pfnRACPReqCB ))
    {
      CGMS_racpReq_t *racpReq = NULL;

      // This allocation will be free by bleapp_util
      racpReq = (CGMS_racpReq_t *)ICall_malloc( sizeof( CGMS_racpReq_t ));
      if ( racpReq != NULL )
      {
        memset( racpReq, 0, sizeof( CGMS_racpReq_t ));
        // Copy the data and send it to the profile
        VOID memcpy( racpReq, pValue, len );
        // if there is RACP in progress and the peer ask to abort, flag the App to stop sending notifications.
        if((racpReq->opCode == RACP_OPCODE_ABORT_OPERATION) && (cgmp_racpProc & RACP_PROC_INPROGRESS))
        {
          cgmp_racpProc |= RACP_PROC_ABORT;
        }
        // Callback function to notify profile of change on RACP characteristic
        BLEAppUtil_invokeFunction( cgms_profileCBs->pfnRACPReqCB, (char *)racpReq );
      }
    }
  }

  /******************************************************/
  /******** CGM Specific Ops Control Point **************/
  /******************************************************/

  else if ( ! memcmp( pAttr->type.uuid, cgms_cgmcp_UUID, pAttr->type.len ))
  {
    if ( cgms_cgmcp_config[connHandle].value != GATT_CLIENT_CFG_INDICATE )
    {
       status = CGMS_RACP_CCCD_IMPROPERLY_CONFG;
    }
    // Only notify profile if the data is in appropriate size
    else if (( len >= CGMS_CGMCP_REQ_MIN_LEN || len <= CGMS_CGMCP_REQ_MIN_LEN ) &&
              cgms_profileCBs &&
              cgms_profileCBs->pfnCGMCPReqCB )
    {
      CGMS_cgmcpReq_t *cgmcpReq;

      // This allocation will be free by bleapp_util
      cgmcpReq = (CGMS_cgmcpReq_t *)ICall_malloc( CGMS_CGMCP_REQ_MAX_LEN );
      if ( cgmcpReq != NULL )
      {
        memset(cgmcpReq, 0, sizeof( CGMS_cgmcpReq_t ));
        // Copy the data and send it to the profile
        VOID memcpy( cgmcpReq, pValue, len );

        // Callback function to notify the profile of change on the characteristic
        BLEAppUtil_invokeFunction( cgms_profileCBs->pfnCGMCPReqCB, (char *)cgmcpReq );
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
 * @fn      CGMS_sendNotiInd
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
static bStatus_t CGMS_sendNotiInd(uint8 *pValue, uint16 len, gattCharCfg_t *charCfgTbl, uint8 *pAttValue)
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
  pAttr = GATTServApp_FindAttr(cgms_attrTbl, GATT_NUM_ATTRS(cgms_attrTbl), pAttValue);
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
        {
          // Determine allocation size
          uint16_t allocLen = (len - offset);
          if ( allocLen > ( connInfo.MTU - CGMS_NOTI_HDR_SIZE ) )
          {
            // If len > MTU split data to chunks of MTU size
            allocLen = connInfo.MTU - CGMS_NOTI_HDR_SIZE;
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
 * @fn      CGMS_allocCCC
 *
 * @brief   Allocate and initialize Client Characteristic Configuration table
 *
 * @return  SUCCESS, or bleMemAllocError
 */
static bStatus_t CGMS_initCCC( void )
{
  // Allocate client characteristic configuration table for CGM Measurement
  cgms_meas_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS );
  if ( cgms_meas_config == NULL )
  {
    // Allocation fails
    // Return error status
    return ( bleMemAllocError );
  }

  // Allocate client characteristic configuration table for Record Access Control Point
  cgms_racp_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS );
  if ( cgms_racp_config == NULL )
  {
    // Allocation fails
    // Free allocated variables
    ICall_free ( cgms_meas_config );

    // Return error status
    return ( bleMemAllocError );
  }

  // Allocate client characteristic configuration table for CGM Specific Ops Control Point
  cgms_cgmcp_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS );
  if ( cgms_cgmcp_config == NULL )
  {
    // Allocation fails
    // Free allocated variables
    ICall_free ( cgms_meas_config );
    ICall_free ( cgms_racp_config );

    // Return error status
    return ( bleMemAllocError );
  }

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, cgms_meas_config );
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, cgms_racp_config );
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, cgms_cgmcp_config );

  return ( SUCCESS );
}

/*********************************************************************
 * @fn      CGMS_cCCWriteReq
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
static bStatus_t CGMS_cCCWriteReq (uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 len, uint16 offset,
                                     uint16 validCfg, InvokeFromBLEAppUtilContext_t callback)
{
  bStatus_t status = SUCCESS;
  CGMS_cccUpdate_t *cccUpdate = NULL;

  // Process CCC Write request
  status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len, offset, validCfg );

  if ( status == SUCCESS )
  {
    // This allocation will be free by bleapp_util
    cccUpdate = (CGMS_cccUpdate_t *)ICall_malloc( sizeof( CGMS_cccUpdate_t ) );
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
 * @fn      CGM_sstIsValid
 *
 * @brief   Verify that the CGM Session Start Time characteristic value are valid.
 *
 * @param   pValue - pointer to SST value
 *
 * @return  TRUE -  IF CGM Session Start Time characteristic value are valid
 *          FALSE - Otherwise
 */
static bStatus_t CGM_sstIsValid( uint8 *pValue )
{
  CGMS_sst_t *sst = (CGMS_sst_t *)pValue;

  if ( // Check validity of DST Offset parameter
       (( sst->dstOffset != CGMS_SST_DST_OFFSET_STANDARD_TIME )        &&
        ( sst->dstOffset != CGMS_SST_DST_OFFSET_HALF_DAYLIGHT_TIME )   &&
        ( sst->dstOffset != CGMS_SST_DST_OFFSET_DAYLIGHT_TIME )        &&
        ( sst->dstOffset != CGMS_SST_DST_OFFSET_DOUBLE_DAYLIGHT_TIME ) &&
        ( sst->dstOffset != CGMS_SST_DST_OFFSET_UNKNOWN )) ||

       // Check validity of Time Zone parameter
       (( sst->timeZone != CGMS_SST_TIMEZONE_UNKNOWN ) &&
        (( sst->timeZone < CGMS_SST_TIMEZONE_MIN ) || ( sst->timeZone > CGMS_SST_TIMEZONE_MAX ))) ||

       // Check validity of Year parameter
       (( sst->year != CGMS_SST_YEAR_UNKNOWN ) &&
        (( sst->year < CGMS_SST_YEAR_MIN ) || ( sst->year > CGMS_SST_YEAR_MAX ))) ||

       // Check validity of Month parameter
       (( sst->month != CGMS_SST_MONTH_UNKNOWN ) &&
        (( sst->month < CGMS_SST_MONTH_MIN ) || ( sst->month > CGMS_SST_MONTH_MAX ))) ||

       // Check validity of Day parameter
       (( sst->day != CGMS_SST_DAY_UNKNOWN ) &&
        (( sst->day < CGMS_SST_DAY_MIN ) || ( sst->day > CGMS_SST_DAY_MAX ))) ||

       // Check validity of Hour parameter
       ( sst->hours > CGMS_SST_HOUR_MAX ) ||

       // Check validity of Minutes parameter
       ( sst->minutes > CGMS_SST_MINUTE_MAX ) ||

       // Check validity of Seconds parameter
       ( sst->seconds > CGMS_SST_SECOND_MAX ))
  {
    // One or more parameters is not valid
    return FALSE;
  }

  // All the parameters are valid
  return TRUE;
}

/*********************************************************************
 * @fn      CGM_sstUpdate
 *
 * @brief   Update the CGM Session Start Time characteristic value
 *
 * @param   pValue - pointer to SST value
 *
 * @return  none
 */
static void CGM_sstUpdate( uint8 *pValue )
{
  CGMS_sst_t *sst = (CGMS_sst_t *)pValue;
  UTCTimeStruct startTime = {0};
  UTCTime secTime = 0;

  /*******************************************************/
  /* Convert SST to startTime in UTCTimeStruct structure */
  /*******************************************************/
  startTime.seconds = sst->seconds;
  startTime.minutes = sst->minutes;
  startTime.hour = sst->hours;
  startTime.day = sst->day;
  startTime.month = sst->month - 1;
  startTime.year = sst->year;

  /*******************************************************/
  /* Calculate SST                                       */
  /*******************************************************/

  // Convert start time structure to seconds
  secTime = osal_ConvertUTCSecs( &startTime );
  // Subtract offset
  secTime = secTime - ( cgm_curTimeOffset * 60 );
  // Convert seconds to time structure
  osal_ConvertUTCTime( &startTime, secTime );

  /*******************************************************/
  /* Convert startTime to SST                            */
  /*******************************************************/
  sst->seconds = startTime.seconds;
  sst->minutes = startTime.minutes;
  sst->hours = startTime.hour;
  sst->day = startTime.day;
  sst->month = startTime.month + 1;
  sst->year = startTime.year;

  // Save the data
  VOID memcpy (&cgms_sst_val, sst, CGMS_SST_LEN);
}

/*********************************************************************
*********************************************************************/
