/**************************************************************************************************
  Filename:       gp_common.c
  Revised:        $Date: 2016-02-25 11:51:49 -0700 (Thu, 25 Feb 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the implementation of the cGP stub.


  Copyright 2006-2015 Texas Instruments Incorporated.

  All rights reserved not granted herein.
  Limited License.

  Texas Instruments Incorporated grants a world-wide, royalty-free,
  non-exclusive license under copyrights and patents it now or hereafter
  owns or controls to make, have made, use, import, offer to sell and sell
  ("Utilize") this software subject to the terms herein. With respect to the
  foregoing patent license, such license is granted solely to the extent that
  any such patent is necessary to Utilize the software alone. The patent
  license shall not apply to any combinations which include this software,
  other than combinations with devices manufactured by or for TI ("TI
  Devices"). No hardware patent is licensed hereunder.

  Redistributions must preserve existing copyright notices and reproduce
  this license (including the above copyright notice and the disclaimer and
  (if applicable) source code license limitations below) in the documentation
  and/or other materials provided with the distribution.

  Redistribution and use in binary form, without modification, are permitted
  provided that the following conditions are met:

    * No reverse engineering, decompilation, or disassembly of this software
      is permitted with respect to any software provided in binary form.
    * Any redistribution and use are licensed by TI for use only with TI Devices.
    * Nothing shall obligate TI to provide you with source code for the software
      licensed and provided to you in object code.

  If software source code is provided to you, modification and redistribution
  of the source code are permitted provided that the following conditions are
  met:

    * Any redistribution and use of the source code, including any resulting
      derivative works, are licensed by TI for use only with TI Devices.
    * Any redistribution and use of any object code compiled from the source
      code and any resulting derivative works, are licensed by TI for use
      only with TI Devices.

  Neither the name of Texas Instruments Incorporated nor the names of its
  suppliers may be used to endorse or promote products derived from this
  software without specific prior written permission.

  DISCLAIMER.

  THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "string.h"
#include "zglobals.h"
#include "dgp_stub.h"
#include "gp_common.h"
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "zcomdef.h"
#include "aps.h"
#include "zcl_port.h"
#include "zcl_general.h"
#include "zcl_green_power.h"
#include "zcl.h"
#include "af.h"
#include "gp_bit_fields.h"

#include "gp_interface.h"
#include "mac_api.h"
#include "zd_sec_mgr.h"
#include "bdb.h"

#include "zstackmsg.h"
#include "zstackapi.h"
#include <ti/sysbios/knl/Semaphore.h>
#include "util_timer.h"


 /*********************************************************************
 * MACROS
 */

 /*********************************************************************
 * CONSTANTS
 */
uint8_t const ppgCommissioningWindow =  180;  //180 seconds by defaut

#ifdef GP_SHARED_KEY
  CONFIG_ITEM uint8_t zgpSharedKey[SEC_KEY_LEN] = GP_SHARED_KEY;
#else
  CONFIG_ITEM uint8_t zgpSharedKey[SEC_KEY_LEN] = {0xFF};
#endif

/*********************************************************************
 * TYPEDEFS
 */

 /*********************************************************************
 * GLOBAL VARIABLES
 */



//List to filter duplicated packets
gp_DataInd_t        *gp_DataIndList = NULL;

uint8_t gpApplicationAllowChannelChange = FALSE;  //Flag to indicate if application allows or not change channel during GP commissioning

const uint8_t gGP_TX_QUEUE_MAX_ENTRY = GP_TX_QUEUE_MAX_ENTRY;
gp_DataReqPending_t gpTxQueueList[GP_TX_QUEUE_MAX_ENTRY];


#endif // (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
//List of dataInd waiting for SecRsp
gp_DataInd_t *dgp_DataIndList = NULL;
uint8_t  dGP_StubHandle = 0;
uint8_t  GPEP_Handle = 0;

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
gpCommissioningNotificationMsg_t commissioningNotificationMsg;
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
ClockP_Struct gpAppDataSendClk;
ClockP_Handle gpAppDataSendClkHandle;
ClockP_Struct gpAppExpireDuplicateClk;
ClockP_Handle gpAppExpireDuplicateClkHandle;
ClockP_Struct gpAppTempMasterTimeoutClk;
ClockP_Handle gpAppTempMasterTimeoutClkHandle;
uint8_t gpAppEntity;
uint32_t *gpAppEvents;
static Semaphore_Handle gpAppSem;
static uint16_t gpDataSendEvent;
static uint16_t gpExpireDuplicateEvent;
static uint16_t gpTempMasterTimeoutEvent;
#endif

/*********************************************************************
 * ZCL General Profile Callback table
 */
zclGp_AppCallbacks_t zclGreenPower_CmdCallbacks =
{
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
  zclGp_GpPairingConfigurationCommandCB,
  zclGp_GpNotificationCommandCB,
  zclGp_GpCommissioningNotificationCommandCB,
#endif
  zclGp_GpPairingCommandCB,
  zclGp_GpProxyTableReqCB,
  zclGp_GpProxyCommissioningModeCB,
  zclGp_GpResponseCommandCB
};

 /*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
// SINK
static uint8_t  zclGp_gpsMaxSinkTableEntries = GPS_MAX_SINK_TABLE_ENTRIES;
static uint8_t  zclGp_gpsCommunicationMode = GPS_COMMUNICATION_MODE;
static uint8_t  zclGp_gpsCommissioningExitMode = GPS_COMMISSIONING_EXIT_MODE;
static uint16_t zclGp_gpsCommissioningWindow = GPS_COMMISSIONING_WINDOW;
static uint8_t  zclGp_gpsSecurityLevel = GPS_SECURITY_LEVEL;
static uint8_t  zclGp_gpsFunctionality[3] = GPS_FUNCTIONALITY;
static uint8_t  zclGp_gpsActiveFunctionality[3] = GPS_ACTIVE_FUNCTIONALITY;
#endif

// PROXY
static uint8_t  pZclGp_gppProxyTable[GPP_MAX_PROXY_TABLE_ENTRIES][PROXY_TBL_LEN];
static uint8_t  zclGp_gppMaxProxyTableEntries = GPP_MAX_PROXY_TABLE_ENTRIES;
static uint8_t  zclGp_gppNotificationRetryNumber = GPP_NOTIFICATION_RETRY_NUMBER;
static uint8_t  zclGp_gppNotificationRetryTimer = GPP_NOTIFICATION_RETRY_TIMER;
static uint8_t  zclGp_gppMaxSearchCounter = GPP_MAX_SEARCH_COUNTER;
static uint8_t *pZclGp_gppBlockedGPDID = NULL;
static uint8_t  zclGp_gppFunctionality[3] = GPP_FUNCTIONALITY;
static uint8_t  zclGp_gppActiveFunctionality[3] = GPP_ACTIVE_FUNCTIONALITY;
static uint8_t  zclGp_gpSharedSecurityKeyType = GP_SHARED_SEC_KEY_TYPE;
static uint8_t  zclGp_gpSharedSecurityKey[SEC_KEY_LEN] = GP_SHARED_KEY;
static uint8_t  zclGp_gpLinkKey[SEC_KEY_LEN] = GP_LINK_LEY;
static uint16_t zclGp_clusterRevision = GP_CLUSTER_REVISION;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
static CONST zclAttrRec_t zclGp_Attrs[] =
{
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPS_MAX_SINK_TABLE_ENTRIES,
     ZCL_DATATYPE_UINT8,
     ACCESS_CONTROL_READ,
     (void *)&zclGp_gpsMaxSinkTableEntries
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_SINK_TABLE,
     ZCL_DATATYPE_LONG_OCTET_STR,
     ACCESS_CONTROL_READ,
     NULL // Use application's callback to Read this attribute
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPS_COMMUNICATION_MODE,
     ZCL_DATATYPE_BITMAP8,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
     (void *)&zclGp_gpsCommunicationMode
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPS_COMMISSIONING_EXIT_MODE,
     ZCL_DATATYPE_BITMAP8,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
     (void *)&zclGp_gpsCommissioningExitMode
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPS_COMMISSIONING_WINDOW,
     ZCL_DATATYPE_UINT16,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
     (void *)&zclGp_gpsCommissioningWindow
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPS_SECURITY_LEVEL,
     ZCL_DATATYPE_BITMAP8,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
     (void *)&zclGp_gpsSecurityLevel
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPS_FUNCTIONALITY,
     ZCL_DATATYPE_BITMAP24,
     ACCESS_CONTROL_READ | ACCESS_CLIENT,
     (void *)&zclGp_gpsFunctionality
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPS_ACTIVE_FUNCTIONALITY,
     ZCL_DATATYPE_BITMAP24,
     ACCESS_CONTROL_READ | ACCESS_CLIENT,
     (void *)&zclGp_gpsActiveFunctionality
  }
 },
#endif
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY)
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPP_MAX_PROXY_TABLE_ENTRIES,
     ZCL_DATATYPE_UINT8,
     ACCESS_CONTROL_READ | ACCESS_CLIENT,
     (void *)&zclGp_gppMaxProxyTableEntries
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_PROXY_TABLE,
     ZCL_DATATYPE_LONG_OCTET_STR,
     ACCESS_CONTROL_READ | ACCESS_CLIENT,
     (void *)&pZclGp_gppProxyTable // Use application's callback to Read this attribute
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPP_NOTIFICATION_RETRY_NUMBER,
     ZCL_DATATYPE_UINT8,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
     (void *)&zclGp_gppNotificationRetryNumber
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPP_NOTIFICATION_RETRY_TIMER,
     ZCL_DATATYPE_UINT8,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
     (void *)&zclGp_gppNotificationRetryTimer
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPP_MAX_SEARCH_COUNTER,
     ZCL_DATATYPE_UINT8,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
     (void *)&zclGp_gppMaxSearchCounter
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPP_BLOCKED_GPD_ID,
     ZCL_DATATYPE_LONG_OCTET_STR,
     ACCESS_CONTROL_READ | ACCESS_CLIENT,
     (void *)&pZclGp_gppBlockedGPDID
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPP_FUNCTIONALITY,
     ZCL_DATATYPE_BITMAP24,
     ACCESS_CONTROL_READ | ACCESS_CLIENT,
     (void *)&zclGp_gppFunctionality
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_GPP_ACTIVE_FUNCTIONALITY,
     ZCL_DATATYPE_BITMAP24,
     ACCESS_CONTROL_READ | ACCESS_CLIENT,
     (void *)&zclGp_gppActiveFunctionality
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_SHARED_SEC_KEY_TYPE,
     ZCL_DATATYPE_BITMAP8,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_GLOBAL,
     (void *)&zclGp_gpSharedSecurityKeyType
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_SHARED_SEC_KEY,
     ZCL_DATATYPE_128_BIT_SEC_KEY,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_GLOBAL,
     (void *)&zclGp_gpSharedSecurityKey
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_GP_LINK_KEY,
     ZCL_DATATYPE_128_BIT_SEC_KEY,
     ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_GLOBAL,
     (void *)&zclGp_gpLinkKey
  }
 },
 {
  ZCL_CLUSTER_ID_GREEN_POWER,
  {  // Attribute record
     ATTRID_CLUSTER_REVISION,
     ZCL_DATATYPE_UINT16,
     ACCESS_CONTROL_READ | ACCESS_GLOBAL,
     (void *)&zclGp_clusterRevision
  }
 }
#endif
};

static uint8_t CONST zclGp_NumAttributes = ( sizeof(zclGp_Attrs) / sizeof(zclGp_Attrs[0]) );

static endPointDesc_t  zclGreenPowerEpDesc = {0};

// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define GREEN_POWER_EP_MAX_INCLUSTERS        1

#define GREEN_POWER_EP_MAX_OUTCLUSTERS       1

static const cId_t greenPower_EP_OutClusterList[GREEN_POWER_EP_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_GREEN_POWER
};

static SimpleDescriptionFormat_t greenPower_EP_SimpleDesc =
{
  GREEN_POWER_INTERNAL_ENDPOINT,         //  int Endpoint;
  ZCL_GP_PROFILE_ID,                     //  uint16_t AppProfId;
  ZCL_GP_DEVICEID_PROXY_BASIC,           //  uint16_t AppDeviceId;
  0,                                     //  int   AppDevVer:4;
  0,                                     //  int   AppFlags:4;
  0,                                     //  byte  AppNumInClusters;
  NULL,                                  //  byte *pAppInClusterList;
  GREEN_POWER_EP_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)greenPower_EP_OutClusterList  //  byte *pAppInClusterList;
};

 /*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclGp_ReadWriteAttrCB( uint16_t clusterId, uint16_t attrId, uint8_t oper, uint8_t *pValue, uint16_t *pLen );
static uint8_t GP_RecoveryKey(uint8_t GPDFKeyType, uint8_t KeyType, uint8_t status, void* Key);
static void gp_u32CastPointer( uint8_t *data, uint8_t *p );
static void gp_u8CastPointer( uint8_t *data, uint8_t *p );
static void gp_DataIndFree( gp_DataInd_t* dataInd, gp_DataInd_t **DataIndList);
static void zclGreenPower_initializeClocks(void);
static void zclSampleAppsGp_ProcessDataSendTimeoutCallback(UArg a0);
static void zclSampleAppsGp_ProcessExpireDuplicateTimeoutCallback(UArg a0);
#endif // (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

static uint8_t* GPEP_findHandle(uint8_t handle);


#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
/*********************************************************************
 * @fn          app_Green_Power_Init
 *
 * @brief       Initialize the green power module.
 *              This function must be called by the application during its initialization.
 *
 * @param       zclSampleAppEvents - the events process by the sample application
 *
 * @return      none
 */
void app_Green_Power_Init(uint8_t  zclSampleApp_Entity, uint32_t *zclSampleAppEvents,
                          Semaphore_Handle zclSampleAppSem, uint16_t gpDataSendEventValue,
                          uint16_t gpExpireDuplicateEventValue, uint16_t gpTempMasterEventValue)
{
  gpAppEntity = zclSampleApp_Entity;
  gpAppEvents = zclSampleAppEvents;
  gpAppSem = zclSampleAppSem;
  gpDataSendEvent = gpDataSendEventValue;
  gpExpireDuplicateEvent = gpExpireDuplicateEventValue;
  gpTempMasterTimeoutEvent = gpTempMasterEventValue;

  zclGreenPower_initializeClocks( );
}

/*********************************************************************
 * @fn          zcl_gpSendNotification
 *
 * @brief       Notify the application to send a Green Power Notification command
 *
 * @param       zclSampleAppEvents - the events process by the sample application
 *
 * @return      none
 */
void zcl_gpSendNotification(void)
{
  gpNotificationMsg_t *gpNotification = NULL;
  gpCmdPayloadMsg_t *pCmdPayloadMsgCurr = NULL;
  uint16_t nwkAddr;
  uint8_t nwkSeqNum;
  uint8_t apsSecNum;

  gpNotification = gp_GetHeadNotificationMsg( );
  if(gpNotification == NULL)
  {
    return;
  }

  pCmdPayloadMsgCurr = ( gpCmdPayloadMsg_t* )gpNotification->pMsg;

  // To save the NIB nwk sequense number and use the GP alias nwk sequence number
  // for the GP notification
  nwkSeqNum = _NIB.SequenceNum;
  _NIB.SequenceNum = gpNotification->secNum;

  // save aps sequence number
  apsSecNum = APS_Counter;
  APS_Counter = gpNotification->secNum;

  // To save the NIB nwk address and use the GP alias nwk address for the GP notification
  nwkAddr = _NIB.nwkDevAddress;
  _NIB.nwkDevAddress = gpNotification->nwkAlias;

  zcl_SendCommand(GREEN_POWER_INTERNAL_ENDPOINT, &gpNotification->addr, ZCL_CLUSTER_ID_GREEN_POWER,
                   COMMAND_GP_NOTIFICATION, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                   TRUE, 0,  APS_Counter, pCmdPayloadMsgCurr->lenght, pCmdPayloadMsgCurr->pMsg);

  // restore nwk radius
  zcl_radius = AF_DEFAULT_RADIUS;

  // restore aps sequence number
  APS_Counter = apsSecNum;

  // Restore the NIB nwk sequence number
  _NIB.SequenceNum = nwkSeqNum;

  // Restore the NIB nwk address
  _NIB.nwkDevAddress = nwkAddr;

  gp_NotificationMsgClean(gp_GetPHeadNotification( ));
  if(gp_GetHeadNotificationMsg( ) != NULL)
  {
    UtilTimer_setTimeout(gpAppDataSendClkHandle, GP_QUEUE_DATA_SEND_INTERVAL);
    UtilTimer_start(&gpAppDataSendClk);
  }
}

/*********************************************************************
 * @fn          zcl_gpSendCommissioningNotification
 *
 * @brief       Notify the application to send a Green Power Commissioning Notification command
 *
 * @param       zclSampleAppEvents - the events process by the sample application
 *
 * @return      none
 */
void zcl_gpSendCommissioningNotification(void)
{
  uint16_t nwkAddr;
  uint8_t nwkSeqNum;
  uint8_t savedApsCounter;

  // SAVE NWK PARAMETERS
  // To save the NIB nwk sequense number and use the GP alias nwk sequence number
  // for the GP notification
  nwkSeqNum = _NIB.SequenceNum;
  // To save APS counter
  savedApsCounter = APS_Counter;
  // To save the NIB nwk address and use the GP alias nwk address for the GP notification
  nwkAddr = _NIB.nwkDevAddress;

  // SET NWK PARAMETERS FOR SENDING THE MESSAGE
  // Set alias sequence number
  _NIB.SequenceNum = commissioningNotificationMsg.seqNum;
  // Set Aps counter equal to alias sequence number
  APS_Counter = _NIB.SequenceNum;
  // Set alias nwk addresss
  _NIB.nwkDevAddress = commissioningNotificationMsg.aliasNwk;

  zcl_SendCommand(GREEN_POWER_INTERNAL_ENDPOINT, &commissioningNotificationMsg.addr,
           ZCL_CLUSTER_ID_GREEN_POWER, COMMAND_GP_COMMISSIONING_NOTIFICATION, TRUE,
           ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, 0, APS_Counter, commissioningNotificationMsg.len,
           commissioningNotificationMsg.pbuf);

  // RESTORE NEK PARAMETERS
  // Restore the NIB nwk sequence number
  _NIB.SequenceNum = nwkSeqNum;
  // Restore APS counter
  APS_Counter = savedApsCounter;
  // Restore the NIB nwk address
  _NIB.nwkDevAddress = nwkAddr;
}
#endif

/*********************************************************************
* @fn          gp_endpointInit
*
* @brief       This function registers the Green Power endpoint and initializes
*              the proxy and sink table.
*
* @param       none
*
* @return      none
*/
void gp_endpointInit( uint8_t entity )
{
  //Register Endpoint
  zclGreenPowerEpDesc.endPoint = GREEN_POWER_INTERNAL_ENDPOINT;
  zclGreenPowerEpDesc.simpleDesc = &greenPower_EP_SimpleDesc;
  zclport_registerEndpoint(entity, &zclGreenPowerEpDesc);

  //Register the ZCL Green Power Cluster Library callback functions
  zclGp_RegisterCmdCallbacks( GREEN_POWER_INTERNAL_ENDPOINT, &zclGreenPower_CmdCallbacks );

  // Register the application's attribute list
  zcl_registerAttrList( GREEN_POWER_INTERNAL_ENDPOINT, zclGp_NumAttributes, zclGp_Attrs );
  zcl_registerReadWriteCB( GREEN_POWER_INTERNAL_ENDPOINT, zclGp_ReadWriteAttrCB, NULL );

  gp_ProxyTblInit( FALSE );
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
  gp_SinkTblInit( FALSE );
#endif
}

/*******************************************************************************
 * @fn      gp_DataIndFindDuplicate
 *
 * @brief   Find if frame is duplicates
 *
 * @param   handle - Handler of gp data indication
 *          secLvl - Security level
 *
 * @return  none
 */
gp_DataInd_t* gp_DataIndFindDuplicate(uint8_t handle, uint8_t secLvl)
{
  gp_DataInd_t* temp = NULL;
  gp_DataInd_t* tempList = NULL;
  temp = gp_DataIndGet(handle);

  if(temp != NULL)
  {
    tempList = gp_DataIndList;

    while(tempList != NULL)
    {
      //search for MAC seq num
      if( (secLvl == 0) &&
          (temp->SeqNumber == tempList->SeqNumber) &&
          (temp->appID == tempList->appID) &&
          (temp->SrcId == tempList->SrcId) &&
          (temp->SecReqHandling.dGPStubHandle != tempList->SecReqHandling.dGPStubHandle) )
      {
          break;
      }
      //Other secLevels uses SecFrameCounter
      else if( (secLvl != 0) &&
              (temp->GPDSecFrameCounter == tempList->GPDSecFrameCounter) &&
              (temp->SecReqHandling.dGPStubHandle != tempList->SecReqHandling.dGPStubHandle) )
      {
        break;
      }
      tempList = tempList->SecReqHandling.next;
    }
  }
  return tempList;
}

 /*********************************************************************
 * @fn          gp_PopulateField
 *
 * @brief       General function fill the proxy table vector item
 *
 * @param
 *
 * @return
 */
void gp_PopulateField( uint8_t* pField, uint8_t** pData, uint8_t size )
{
  switch ( size )
  {
    case ( sizeof( uint8_t ) ):
      gp_u8CastPointer( pField, *pData );
      break;
    case ( sizeof( uint16_t ) ):
      gp_u16CastPointer( pField, *pData );
      break;
    case ( sizeof( uint32_t ) ):
      gp_u32CastPointer( pField, *pData );
      break;
    case ( Z_EXTADDR_LEN ):
      zcl_memcpy( pField, *pData, Z_EXTADDR_LEN );
      break;
    case ( SEC_KEY_LEN ):
      zcl_memcpy( pField, *pData, SEC_KEY_LEN );
      break;
  }
  *pData += size;
}

/*********************************************************************
 * @fn          gpLookForGpd
 *
 * @brief       To see if the GPD id is in this NV entry
 *
 * @param       currEntryId - NV ID of the proxy table
 *              pNew - New table entry array to be added
 *
 * @return      TRUE if the GPD has an entry in current NV vector
 */
uint8_t gpLookForGpd( uint16_t currEntryId, uint8_t* pNew )
{
  uint8_t status;

  uint8_t newEntry[GP_TBL_OPT_AND_GPD_ID_LEN];
  uint8_t currEntry[GP_TBL_OPT_AND_GPD_ID_LEN];

  zcl_memcpy(newEntry, pNew, GP_TBL_OPT_AND_GPD_ID_LEN);

  status = zclport_readNV(ZCL_PORT_PROXY_TABLE_NV_ID, currEntryId,
                          0,
                          GP_TBL_OPT_AND_GPD_ID_LEN,
                          currEntry);

  if((status == SUCCESS) && (GP_TBL_COMP_APPLICATION_ID( newEntry[GP_TBL_OPT], currEntry[GP_TBL_OPT])))
  {
    if((GP_GET_APPLICATION_ID(newEntry[GP_TBL_OPT]) == GP_OPT_APP_ID_GPD))
    {
      status = zcl_memcmp(&newEntry[GP_TBL_SRC_ID], &currEntry[GP_TBL_SRC_ID], sizeof(uint32_t));
    }
    else if((GP_GET_APPLICATION_ID(newEntry[GP_TBL_OPT]) == GP_OPT_APP_ID_IEEE))
    {
      status = zcl_memcmp(&newEntry[GP_TBL_GPD_ID], &currEntry[GP_TBL_GPD_ID], Z_EXTADDR_LEN);
    }
    return status;
  }
  return FALSE;
}

/*********************************************************************
 * @fn          GP_SecReq
 *
 * @brief       Primitive from dGP stub to GP EndPoint asking how to process a GPDF.
 *
 * @param       gp_SecReq
 *
 * @return      none
 */
void GP_SecReq(gp_SecReq_t *gp_SecReq)
{
  gp_SecRsp_t *gp_SecRsp = NULL;
  uint8_t keyType;
  uint8_t key[SEC_KEY_LEN];


  gp_SecRsp = (gp_SecRsp_t*)zcl_mem_alloc( sizeof(gp_SecRsp_t) );
  if(gp_SecRsp != NULL)
  {
    gp_SecRsp->hdr.event = GP_SEC_RSP;
    gp_SecRsp->hdr.status = ZSuccess;
    gp_SecRsp->dGPStubHandle = gp_SecReq->dGPStubHandle;
    gp_SecRsp->EndPoint = gp_SecReq->EndPoint;

    zcl_memcpy(&gp_SecRsp->gp_SecData, &gp_SecReq->gp_SecData, sizeof(gp_SecData_t));
    zcl_memcpy(&gp_SecRsp->gpdID, &gp_SecReq->gpdID, sizeof(gpdID_t));

    //Find duplicates A.3.6.1.2 Duplicate filtering
    if(gp_DataIndFindDuplicate(gp_SecReq->dGPStubHandle, gp_SecReq->gp_SecData.GPDFSecLvl))
    {
        gp_SecRsp->Status = GP_SEC_RSP_DROP_FRAME;

#if defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS)
        zstack_gpSecRsp_t secRsp;

        secRsp.Status = gp_SecRsp->Status;
        secRsp.dGPStubHandle = gp_SecRsp->dGPStubHandle;
        secRsp.gpdID = gp_SecRsp->gpdID;
        secRsp.EndPoint = gp_SecRsp->EndPoint;
        secRsp.gp_SecData = gp_SecRsp->gp_SecData;
        zcl_memcpy(secRsp.GPDKey, gp_SecRsp->GPDKey, SEC_KEY_LEN);

       Zstackapi_gpSecRsp(gpAppEntity, &secRsp);
       zcl_mem_free(gp_SecRsp);
#else
       OsalPort_msgSend(gp_TaskID, (uint8_t*)gp_SecRsp);
#endif
        return;
    }

    //Section A.3.7.3.1.2
#if defined (ENABLE_GREENPOWER_COMBO_BASIC) && (ZG_BUILD_RTR_TYPE)
    gp_SecRsp->Status = gp_SecurityOperationSink(gp_SecReq ,&keyType, key);
    if( gp_SecRsp->Status == GP_SEC_RSP_NO_ENTRY )
    {
      //Section A.3.7.3.1.3
      gp_SecRsp->Status = gp_SecurityOperationProxy(gp_SecReq ,&keyType, key);
    }
#else
    //Section A.3.7.3.1.3
    gp_SecRsp->Status = gp_SecurityOperationProxy(gp_SecReq ,&keyType, key);
#endif
    switch(gp_SecRsp->Status)
    {
      case GP_SEC_RSP_ERROR:  //This should not happen
      case GP_SEC_RSP_DROP_FRAME:
        gp_SecRsp->Status = GP_SEC_RSP_DROP_FRAME;
      break;

      case GP_SEC_RSP_MATCH:
        if(GP_RecoveryKey(gp_SecReq->gp_SecData.GPDFKeyType, keyType, gp_SecRsp->Status, key) == GP_SEC_RSP_DROP_FRAME)
        {
          gp_SecRsp->Status = GP_SEC_RSP_DROP_FRAME;
        }
        else
        {
          zcl_memcpy(gp_SecRsp->GPDKey, key, SEC_KEY_LEN);
        }
      break;
    }
#if defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS)
    zstack_gpSecRsp_t secRsp;

    secRsp.Status = gp_SecRsp->Status;
    secRsp.dGPStubHandle = gp_SecRsp->dGPStubHandle;
    secRsp.gpdID = gp_SecRsp->gpdID;
    secRsp.EndPoint = gp_SecRsp->EndPoint;
    secRsp.gp_SecData = gp_SecRsp->gp_SecData;
    zcl_memcpy(secRsp.GPDKey, gp_SecRsp->GPDKey, SEC_KEY_LEN);

    Zstackapi_gpSecRsp(gpAppEntity, &secRsp);
    zcl_mem_free(gp_SecRsp);
#else
    OsalPort_msgSend(gp_TaskID, (uint8_t*)gp_SecRsp);
#endif
  }
}

/*********************************************************************
 * @fn         proxyTableCpy
 *
 * @brief      Copy proxy table memory.
 *
 * @param      dst - pointer to destination memory
 * @param      src - pointer to source memroy
 *
 * @return     none
 */
void proxyTableCpy(void *dst, void *src)
{
  zcl_memcpy( dst, src, PROXY_TBL_LEN );
}

/*********************************************************************
 * @fn         sinkTableCpy
 *
 * @brief      Copy sink table memory.
 *
 * @param      dst - pointer to destination memory
 * @param      src - pointer to source memroy
 *
 * @return     none
 */
void sinkTableCpy(void *dst, void *src)
{
  zcl_memcpy( dst, src, SINK_TBL_ENTRY_LEN );
}

/*********************************************************************
* @fn          gp_expireDuplicateFiltering
*
* @brief       Process the expiration of the packets in the duplicate filtering
*              list. Assumption is the first in the queue is the first into expire.
*
* @param       none
*
* @return      none
*/
void gp_expireDuplicateFiltering(void)
{
    gp_DataInd_t* temp;
    uint32_t timeout = 0;

    temp = gp_DataIndList;

    while(temp != NULL)
    {
        timeout = gp_DataIndList->SecReqHandling.timeout;
        if(timeout >= temp->SecReqHandling.timeout)
        {
            gp_DataInd_t *expired = temp;

            temp = temp->SecReqHandling.next;
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
            gp_DataIndFree(expired, &gp_DataIndList);
#else
            gp_DataIndReleaseFromList(expired, &gp_DataIndList);
#endif
        }
        else
        {
            temp->SecReqHandling.timeout -= timeout;
            temp = temp->SecReqHandling.next;
        }
    }

    if((timeout > 0) && (gp_DataIndList != NULL))
    {
#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
        UtilTimer_setTimeout(gpAppExpireDuplicateClkHandle, timeout );
        UtilTimer_start(&gpAppExpireDuplicateClk);
#else
        OsalPortTimers_startTimer(gp_TaskID, GP_DUPLICATE_FILTERING_TIMEOUT_EVENT, timeout);
#endif
    }
}

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
 /*******************************************************************************
  *
  * @fn          gp_processDataIndMsg
  *
  * @brief       Process green power data indication message from gpStub
  *
  * @param       pInMsg - pointer to incoming message
  *
  * @return      none
  *
  */
 void gp_processDataIndMsg(zstack_gpDataInd_t *pInMsg)
 {
   gp_DataInd_t *pMsg = NULL;

   pMsg = zcl_mem_alloc(sizeof(gp_DataInd_t) + pInMsg->GPDasduLength);

   if(pMsg != NULL)
   {
     /*
      * All incoming messages are passed to proxy
      */
     zcl_memcpy(&(pMsg->SecReqHandling), &(pInMsg->SecReqHandling), sizeof(gp_DataIndSecReq_t));
     pMsg->timestamp = pInMsg->timestamp;
     pMsg->status = pInMsg->status;
     pMsg->Rssi = pInMsg->Rssi;
     pMsg->LinkQuality = pInMsg->LinkQuality;
     pMsg->SeqNumber = pInMsg->SeqNumber;
     zcl_memcpy(&(pMsg->srcAddr), &(pInMsg->srcAddr), sizeof(sAddr_t));
     pMsg->srcPanID = pInMsg->srcPanID;
     pMsg->appID = pInMsg->appID;
     pMsg->GPDFSecLvl = pInMsg->GPDFSecLvl;
     pMsg->GPDFKeyType = pInMsg->GPDFKeyType;
     pMsg->AutoCommissioning = pInMsg->AutoCommissioning;
     pMsg->RxAfterTx = pInMsg->RxAfterTx;
     pMsg->SrcId = pInMsg->SrcId;
     pMsg->EndPoint = pInMsg->EndPoint;
     pMsg->GPDSecFrameCounter = pInMsg->GPDSecFrameCounter;
     pMsg->GPDCmmdID = pInMsg->GPDCmmdID;
     pMsg->MIC = pInMsg->MIC;
     pMsg->GPDasduLength = pInMsg->GPDasduLength;
     zcl_memcpy(pMsg->GPDasdu, pInMsg->GPDasdu, pMsg->GPDasduLength);

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
     gp_dataIndSink(pMsg);
     gp_dataIndProxy(pMsg);
#else
     gp_dataIndProxy(pMsg);
#endif

     zcl_mem_free(pMsg);
   }
 }

 /*******************************************************************************
  *
  * @fn          gp_processSecRecMsg
  *
  * @brief       Process green power security request message from gpStub
  *
  * @param       pInMsg - pointer to incoming message
  *
  * @return      none
  *
  */
 void gp_processSecRecMsg(zstack_gpSecReq_t *pInMsg)
 {
     gp_SecReq_t Msg;

     /*
      * All incoming messages are passed to proxy
      */
     zcl_memcpy(&(Msg.gpdID), &(pInMsg->gpdID), sizeof(gpdID_t));
     Msg.EndPoint = pInMsg->EndPoint;
     zcl_memcpy(&(Msg.gp_SecData), &(pInMsg->gp_SecData), sizeof(gp_SecData_t));
     Msg.dGPStubHandle = pInMsg->dGPStubHandle;

     GP_SecReq(&Msg);
 }

 /*******************************************************************************
  *
  * @fn          gp_processCheckAnnceMsg
  *
  * @brief       Process device announce with proxy table for address conflict resolution
  *
  * @param       pInMsg - pointer to incoming message
  *
  * @return      none
  *
  */
 void gp_processCheckAnnceMsg(zstack_gpCheckAnnounce_t *pInMsg)
 {
     gp_CheckAnnouncedDevice(pInMsg->extAddr, pInMsg->nwkAddr);
 }

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
 /*******************************************************************************
  *
  * @fn          gp_processGpCommissioningSuccesss
  *
  * @brief       Process Green Power Success command
  *
  * @param       pInMsg - pointer to incoming message
  *
  * @return      none
  *
  */
 void gp_processGpCommissioningSuccesss(zstack_gpCommissioningSuccess_t *pInMsg)
 {
     ZStatus_t status;
     uint8_t pEntry[PROXY_TBL_LEN];

     status = gp_getSinkTableByIndex(pInMsg->index, pEntry);
     // Entry was found
     if( status == ZSuccess  )
     {
         uint32_t secFrameCnt;

         secFrameCnt = BUILD_UINT32(pEntry[SINK_TBL_SEC_FRAME], pEntry[SINK_TBL_SEC_FRAME+1],
                                    pEntry[SINK_TBL_SEC_FRAME+2], pEntry[SINK_TBL_SEC_FRAME+3]);

         zclGp_SendGpPairing(pEntry, GP_ACTION_EXTEND, secFrameCnt,zcl_InSeqNum);
     }
 }
#endif // ENABLE_GREENPOWER_COMBO_BASIC
#endif // USE_ICALL

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
 /*********************************************************************
  * @fn          gp_sinkAddProxyEntry
  *
  * @brief       General function fill the proxy table vector
  *
  * @param       pSinkEntry - Pointer to sink table entry
  *
  * @return
  */
 void gp_sinkAddProxyEntry( uint8_t* pSinkEntry )
 {
   uint8_t  newProxyEntry[PROXY_TBL_LEN];
   uint8_t  searchCounter = 0x0A;
   gpSinkTableOptions_t sinkOptions = {0};
   gpProxyTableOptions_t proxyOptions = {0};

   zcl_memset(newProxyEntry, 0x00, PROXY_TBL_LEN);

   zcl_memcpy( &sinkOptions, &pSinkEntry[SINK_TBL_OPT], sizeof( uint16_t ) );
   proxyOptions.appId = sinkOptions.appId;
   proxyOptions.entryActive = TRUE;
   proxyOptions.entryValid = TRUE;
   proxyOptions.sequenceNumberCap = sinkOptions.sequenceNumberCap;
   if(sinkOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST)
   {
     proxyOptions.lightUnicast = TRUE;
     proxyOptions.derivedGroupGPS = FALSE;
     proxyOptions.commissionedGroupGPS = FALSE;
   }
   else if(sinkOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID)
   {
     proxyOptions.lightUnicast = FALSE;
     proxyOptions.derivedGroupGPS = TRUE;
     proxyOptions.commissionedGroupGPS = FALSE;
   }
   else if(sinkOptions.communicationMode == GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID)
   {
     proxyOptions.lightUnicast = FALSE;
     proxyOptions.derivedGroupGPS = FALSE;
     proxyOptions.commissionedGroupGPS = TRUE;
   }
   proxyOptions.firstToForward = TRUE;
   proxyOptions.inRange = TRUE;
   proxyOptions.gpdFixed = sinkOptions.gpdFixed;
   proxyOptions.hasAllUnicastRoutes = FALSE;
   proxyOptions.assignedAlias = sinkOptions.assignedAlias;
   proxyOptions.securityUse = sinkOptions.securityUse;
   proxyOptions.optionsExtension = FALSE;

   zcl_memcpy( &newProxyEntry[GP_TBL_OPT], &proxyOptions, sizeof(uint16_t) );
   zcl_memcpy( &newProxyEntry[GP_TBL_GPD_ID], &pSinkEntry[GP_TBL_GPD_ID], Z_EXTADDR_LEN );
   zcl_memcpy( &newProxyEntry[PROXY_TBL_GPD_EP], &pSinkEntry[SINK_TBL_GPD_EP], sizeof(uint8_t) );
   zcl_memcpy( &newProxyEntry[PROXY_TBL_ALIAS], &pSinkEntry[SINK_TBL_ALIAS], sizeof(uint16_t) );
   zcl_memcpy( &newProxyEntry[PROXY_TBL_SEC_OPT], &pSinkEntry[SINK_TBL_SEC_OPT], sizeof(uint8_t) );
   zcl_memcpy( &newProxyEntry[PROXY_TBL_SEC_FRAME], &pSinkEntry[SINK_TBL_SEC_FRAME], sizeof(uint32_t) );
   zcl_memcpy( &newProxyEntry[PROXY_TBL_GPD_KEY], &pSinkEntry[SINK_TBL_GPD_KEY], SEC_KEY_LEN );
   if((proxyOptions.derivedGroupGPS) || (proxyOptions.commissionedGroupGPS))
   {
     SET_BIT( &newProxyEntry[PROXY_TBL_GRP_TBL_ENTRIES], PROXY_TBL_1ST_GRP_BIT );
     zcl_memcpy( &newProxyEntry[PROXY_TBL_1ST_GRP_ADDR], &pSinkEntry[SINK_TBL_1ST_GRP_ADDR], sizeof ( uint32_t ) );
   }
   zcl_memcpy( &newProxyEntry[PROXY_TBL_RADIUS], &pSinkEntry[SINK_TBL_RADIUS], sizeof(uint8_t) );
   zcl_memset( &newProxyEntry[PROXY_TBL_SEARCH_COUNTER], searchCounter, sizeof(uint8_t) );

   // Update the proxy table, setting options to 0x08 to force sink add
   gp_UpdateProxyTbl( newProxyEntry, 0x08, FALSE );
 }

 /*********************************************************************
  * @fn          gp_processCommissioningKey
  *
  * @brief       Process a Green Power Commissioning command security key
  *              either for key encryption or decryption
  *
  * @param       pCommissioningCmd - Pointer to received commissioning command
  *              pKeyRsp - pointer to key processing response
  *
  *
  * @return
  */
ZStatus_t gp_processCommissioningKey(gpdID_t *pGPDId, gpdCommissioningCmd_t *pCommissioningCmd, zstack_gpEncryptDecryptCommissioningKeyRsp_t *pKeyRsp)
 {
     zstack_gpEncryptDecryptCommissioningKey_t msg;
#ifdef ZCL_READ
     uint8_t gpLinkKey[SEC_KEY_LEN];
     uint16_t keyLen;
#endif

     zcl_memset(&msg, 0, sizeof(zstack_gpEncryptDecryptCommissioningKey_t));
     msg.encrypt = !pCommissioningCmd->extOptions.gpdKeyPresent;
     zcl_memcpy( &msg.gpdId, pGPDId, sizeof(gpdID_t) );
     msg.keyType = pCommissioningCmd->extOptions.keyType;
     zcl_memcpy( msg.key, pCommissioningCmd->gpdKey, SEC_KEY_LEN );
     msg.secFrameCounter = pCommissioningCmd->gpdOutCounter;
     msg.keyMic = pCommissioningCmd->gpdKeyMic;
#ifdef ZCL_READ
     // Get the green power Link Key
     zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER,
                      ATTRID_GP_LINK_KEY, gpLinkKey, &keyLen);
     zcl_memcpy( msg.secKey, gpLinkKey, SEC_KEY_LEN );
#endif

     // Encrypt/Decrypt Network Key
     Zstackapi_gpEncryptDecryptCommissioningKey( gpAppEntity, &msg, pKeyRsp );
     if( pKeyRsp->status != SUCCESS )
     {
         //No memory or CCM star error
         return pKeyRsp->status;
     }
     return pKeyRsp->status;
 }
#endif // ENABLE_GREENPOWER_COMBO_BASIC
#endif //(DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

 /*********************************************************************
  * @fn          gp_DataIndAppendToList
  *
  * @brief       Append a DataInd to a list of DataInd (waiting for GP Sec Rsp List,
  *              or list to filter duplicate packets)
  *
  * @param       gp_DataInd to be appended
  *
  * @return      None
  */
void gp_DataIndAppendToList(gp_DataInd_t *gp_DataInd, gp_DataInd_t **DataIndList)
{
  if(*DataIndList == NULL)
  {
    *DataIndList = gp_DataInd;
  }
  else
  {
    gp_DataInd_t *dgp_DataIndTemp;

    dgp_DataIndTemp = *DataIndList;
    while(dgp_DataIndTemp->SecReqHandling.next != NULL)
    {
      dgp_DataIndTemp = dgp_DataIndTemp->SecReqHandling.next;
    }
    dgp_DataIndTemp->SecReqHandling.next = gp_DataInd;
  }
}

/*********************************************************************
 * @fn          gp_DataIndGet
 *
 * @brief       Search for a DataInd entry with matching handle
 *
 * @param       handle to search for
 *
 * @return      gp_DataInd entry, or NULL if not found
 */
gp_DataInd_t* gp_DataIndGet(uint8_t handle)
{
  gp_DataInd_t* temp;

  temp = dgp_DataIndList;

  if(temp != NULL)
  {
    while((temp->SecReqHandling.dGPStubHandle != handle) && (temp != NULL))
    {
      temp = temp->SecReqHandling.next;
    }
  }
  return temp;
}

 /*********************************************************************
 * @fn          gp_GetHandle
 *
 * @brief       Returns a new handle for the type of msg.
 *
 * @param       handleType - type of handle to request
 *
 * @return      handle - returns the value of the handle, if
 *              0 the handle was not valid
 */
uint8_t gp_GetHandle(uint8_t handleType)
{
  uint8_t  tempHandle = 0;
  uint8_t* pHandleData = NULL;
  getHandleFunction_t  pGetHandleFunction = NULL;

  switch(handleType)
  {
    case DGP_HANDLE_TYPE:
      pGetHandleFunction = dGP_findHandle;
      pHandleData = &dGP_StubHandle;
    break;
    case GPEP_HANDLE_TYPE:
      pGetHandleFunction = GPEP_findHandle;
      pHandleData = &GPEP_Handle;
    break;
  }

  if((pGetHandleFunction == NULL) || (pHandleData == NULL))
  {
    return tempHandle;
  }

  do
  {
    tempHandle = (*pHandleData)++;
    //check if the handle is not in use by other msg
    if(pGetHandleFunction(tempHandle) != NULL)
    {
      tempHandle = 0;
    }
  }
  while (tempHandle == 0);

  return tempHandle;
}

/*********************************************************************
* @fn          gp_channelConfiguration
*
* @brief       General function fill uint16_t from pointer.
*
* @param       pBitField - pointer to the bit field.
*              bit       - position of the bit to set in the given bitfield.
*
* @return
*/
void gp_channelConfiguration(uint8_t channel)
{

}

/*********************************************************************
* @fn          gp_u16CastPointer
*
* @brief       General function fill uint16_t from pointer.
*
* @param       pBitField - pointer to the bit field.
*              bit       - position of the bit to set in the given bitfield.
*
* @return
*/
void gp_u16CastPointer( uint8_t *data, uint8_t *p )
{
   *(uint16_t*)data = 0;
   *(uint16_t*)data |= ((*(uint16_t*)p++)      & 0x00FF);
   *(uint16_t*)data |= ((*(uint16_t*)p++ << 8) & 0xFF00);
}

/*********************************************************************
* @fn          gp_u16CastPointerReverse
*
* @brief       General function fill uint16_t reversed from pointer.
*
* @param       pBitField - pointer to the bit field.
*              bit       - position of the bit to set in the given bitfield.
*
* @return
*/
void gp_u16CastPointerReverse( uint8_t *data, uint8_t *p )
{
  *(uint16_t*)data = 0;
  *(uint16_t*)data |= ((*(uint16_t*)p++ << 8) & 0xFF00);
  *(uint16_t*)data |= ((*(uint16_t*)p++)      & 0x00FF);
}

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*********************************************************************
* @fn          gp_aliasDerivation
*
* @brief       General function fill uint16_t from pointer.
*
* @param       gpdId - Green Power Device id structure
*
*
* @return   addr - Alias short address
*/
uint16_t gp_aliasDerivation(gpdID_t *pGpdId)
{
  uint16_t lsbAddr = 0;
  uint16_t hsbAddr = 0;
  uint16_t alias = 0;
  uint16_t addr = 0;

  gp_u16CastPointer((uint8_t*)&alias, (uint8_t*)&pGpdId->id);
  if ((alias == 0x0000) || (alias > 0xFFF7))
  {
    if(pGpdId->appID == GP_OPT_APP_ID_GPD)
    {
      hsbAddr |= ((pGpdId->id.srcID >> 16) & 0x00FF);
      hsbAddr |= ((pGpdId->id.srcID >> 16) & 0xFF00);

      lsbAddr |= ((pGpdId->id.srcID) & 0x00FF);
      lsbAddr |= ((pGpdId->id.srcID) & 0xFF00);
    }
    else if(pGpdId->appID == GP_OPT_APP_ID_IEEE)
    {
      hsbAddr |= (pGpdId->id.gpdExtAddr[1]);
      hsbAddr |= (pGpdId->id.gpdExtAddr[0] << 16);
      lsbAddr |= (pGpdId->id.gpdExtAddr[7]);
      lsbAddr |= (pGpdId->id.gpdExtAddr[6] << 16);
    }

    addr = lsbAddr ^ hsbAddr;
    if((addr == 0x0000) || (addr > 0xFFF7))
    {
      if(lsbAddr == 0x0000)
      {
        addr = 0x0007;
      }
      else
      {
        addr = lsbAddr - 8;
      }
    }
    return addr;
  }
  return alias;
}

/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

 /*********************************************************************
  * @fn      zclGp_ReadWriteAttrCB
  *
  * @brief   Handle green power attributes.
  *
  * @param   clusterId - cluster that attribute belongs to
  * @param   attrId - attribute to be read or written
  * @param   oper - ZCL_OPER_LEN, ZCL_OPER_READ, or ZCL_OPER_WRITE
  * @param   pValue - pointer to attribute value, OTA endian
  * @param   pLen - length of attribute value read, native endian
  *
  * @return  status
  */
static ZStatus_t zclGp_ReadWriteAttrCB( uint16_t clusterId, uint16_t attrId, uint8_t oper,
                                          uint8_t *pValue, uint16_t *pLen )
 {
   ZStatus_t status = ZSuccess;
   uint16_t proxyTableIndex;
   uint8_t i;

   switch ( oper )
   {
     case ZCL_OPER_LEN:
     case ZCL_OPER_READ:
       if ( attrId == ATTRID_GP_SINK_TABLE )
       {
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)

         for ( i = 0; i < GPS_MAX_SINK_TABLE_ENTRIES ; i++ )
         {
           proxyTableIndex = i;

           status = pt_ZclReadGetSinkEntry ( proxyTableIndex, pValue, (uint8_t*)pLen );
           if ( status != ZSuccess )
           {
             status = ZFailure;
             break;
           }
         }
#else
         status = ZFailure;
#endif
       }
       if ( attrId == ATTRID_GP_PROXY_TABLE )
       {
         for ( i = 0; i < GPP_MAX_PROXY_TABLE_ENTRIES ; i++ )
         {
           proxyTableIndex = i;

           status = pt_ZclReadGetProxyEntry ( proxyTableIndex, pValue, (uint8_t*)pLen );
           if ( status != ZSuccess )
           {
             if(status == NV_INVALID_DATA)
             {
               // End of table
               status = ZSuccess;
             }
             else
             {
               status = ZFailure;
             }
             break;
           }
         }
       }
       break;

     case ZCL_OPER_WRITE:
       status = ZFailure;
       break;
   }

   return ( status );
 }

/*********************************************************************
 * @fn          GP_RecoveryKey
 *
 * @brief       Procedure to retrieve the key to be used to decrypt the GPDF
 *
 * @param       GPDFKeyType   KeyType from the GPDF
 * @param       KeyType       Key type in the Proxy table entry for this GPD
 * @param       Status        Previous status to search key (MATCH, TX_THEN_DROP)
 * @param       Key[out]      Key to be used
 *
 * @return      Status
 */
static uint8_t GP_RecoveryKey(uint8_t GPDFKeyType, uint8_t KeyType, uint8_t status, void* Key)
{
  //TODO: A.3.7.3.4 Incoming frames: key recovery
  if(GPDFKeyType)
  {
    uint8_t TempKey[SEC_KEY_LEN] = {0xFF};

    if(KeyType == GP_SECURITY_KEY_TYPE_OUT_OF_BOX_GPD_KEY)
    {
      //Is the key of the Proxy table entry empty?
      if(zcl_memcmp(Key, (void*)TempKey, SEC_KEY_LEN))
      {
        return GP_SEC_RSP_DROP_FRAME;
      }
      else
      {
        //There is a key, then use it
        return status;
      }
    }
    else if(KeyType == GP_SECURITY_KEY_TYPE_DERIVED_IND_GPD_KEY)
    {
      //Is the key of the Proxy table entry empty?
      if(zcl_memcmp(Key, (void*)TempKey, SEC_KEY_LEN))
      {
        return GP_SEC_RSP_DROP_FRAME;
      }
      //There is a key, then use it. Derived keys are provided by the Sink
      //device at paring time according to A.3.7.1.2.2
      return status;
    }
    else
    {
      return GP_SEC_RSP_DROP_FRAME;
    }
  }
  else
  {
    uint8_t  gpSharedSecKeyType = 0;
    uint8_t TempKey[SEC_KEY_LEN] = {0xFF};

#ifdef ZCL_READ
    uint16_t AttLen;

    //Get the SharedKeyType Attribute
    zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT, ZCL_CLUSTER_ID_GREEN_POWER, ATTRID_GP_SHARED_SEC_KEY_TYPE, &gpSharedSecKeyType, &AttLen);
#endif

    if( (KeyType == gpSharedSecKeyType) && (!zcl_memcmp((void*)TempKey, (void*)zgpSharedKey, SEC_KEY_LEN)) )
    {
      //Use key shared key
      zcl_memcpy(Key, (void*)zgpSharedKey, SEC_KEY_LEN);
      return status;
    }
    if(KeyType == GP_SECURITY_KEY_TYPE_ZIGBEE_NWK_KEY)
    {
      ZDSecMgrReadKeyFromNv(ZCD_NV_EX_LEGACY, ZCD_NV_PRECFGKEY, Key);
    }
    //There is a key, then use it. Derived keys are provided by the Sink
    //device at paring time according to A.3.7.1.2.2
    else
    {
      return status;
    }
  }
  return status;
}

/*********************************************************************
 * @fn          gp_u32CastPointer
 *
 * @brief       General function fill uint32_t from pointer.
 *
 * @param       pBitField - pointer to the bit field.
 *              bit       - position of the bit to set in the given bitfield.
 *
 * @return
 */
static void gp_u32CastPointer( uint8_t *data, uint8_t *p )
{
    *(uint32_t*)data |= ((*(uint32_t*)p++)       & 0x000000FF);
    *(uint32_t*)data |= ((*(uint32_t*)p++ <<  8) & 0x0000FF00);
    *(uint32_t*)data |= ((*(uint32_t*)p++ << 16) & 0x00FF0000);
    *(uint32_t*)data |= ((*(uint32_t*)p++ << 24) & 0xFF000000);
}

 /*********************************************************************
 * @fn          gp_u8CastPointer
 *
 * @brief       General function fill uint16_t from pointer.
 *
 * @param       pBitField - pointer to the bit field.
 *              bit       - position of the bit to set in the given bitfield.
 *
 * @return
 */
static void gp_u8CastPointer( uint8_t *data, uint8_t *p )
{
    *(uint8_t*)data |= *(uint8_t*)p;
}

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
/*********************************************************************
 * @fn          gp_DataIndFree
 *
 * @brief       Releases an element in the list
 *
 * @param       dataInd Pointer to the element in the list to be released
 *
 * @param       DataIndList List from which the data ind will be released
 *
 * @return      none
 */
static void gp_DataIndFree( gp_DataInd_t* dataInd, gp_DataInd_t **DataIndList )
{
  //Check the first entry
  if(*DataIndList != NULL)
  {
    if(*DataIndList == dataInd)
    {
      gp_DataInd_t  *next = (*DataIndList)->SecReqHandling.next;
      zcl_mem_free((uint8_t*)*DataIndList);
      *DataIndList = next;
    }
    else
    {
      gp_DataInd_t  *previous = *DataIndList;
      gp_DataInd_t  *current  = (*DataIndList)->SecReqHandling.next;

      while((current != NULL) && (previous != NULL))
      {
        if(current == dataInd)
        {
          gp_DataInd_t  *temp = current;
          previous->SecReqHandling.next = current->SecReqHandling.next;
          zcl_mem_free((uint8_t*)temp);
          return;
        }
        previous = current;
        current = current->SecReqHandling.next;
      }
    }
  }
}

/*******************************************************************************
 * @fn      zclSampleAppsGp_ProcessDataSendTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleAppsGp_ProcessDataSendTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    //Assign the Green Power data send event
    *gpAppEvents |= gpDataSendEvent;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(gpAppSem);
}

/*******************************************************************************
 * @fn      zclSampleAppsGp_ProcessExpireDuplicateTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleAppsGp_ProcessExpireDuplicateTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    //Assign the Green Power data send event
    *gpAppEvents |= gpExpireDuplicateEvent;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(gpAppSem);
}

/*******************************************************************************
 * @fn      zclSampleAppsGp_ProcessTempMasterTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleAppsGp_ProcessTempMasterTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    //Assign the Green Power data send event
    *gpAppEvents |= gpTempMasterTimeoutEvent;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(gpAppSem);
}

/*******************************************************************************
 * @fn      zclGreenPower_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclGreenPower_initializeClocks(void)
{
    // Construct the timer used to send the data indication queue
    gpAppDataSendClkHandle = UtilTimer_construct(
    &gpAppDataSendClk,
    zclSampleAppsGp_ProcessDataSendTimeoutCallback,
    GP_QUEUE_DATA_SEND_INTERVAL,
    0,
    false,
    0);

    gpAppExpireDuplicateClkHandle = UtilTimer_construct(
    &gpAppExpireDuplicateClk,
    zclSampleAppsGp_ProcessExpireDuplicateTimeoutCallback,
    gpDuplicateTimeout,
    0,
    false,
    0);

    gpAppTempMasterTimeoutClkHandle = UtilTimer_construct(
    &gpAppTempMasterTimeoutClk,
    zclSampleAppsGp_ProcessTempMasterTimeoutCallback,
    gpBirectionalCommissioningChangeChannelTimeout,
    0,
    false,
    0);
}
#endif // USE_ICALL

#endif

/*******************************************************************************
 * @fn      GPEP_findHandle
 *
 * @brief   Finds Ep with given handler
 *
 * @param   none
 *
 * @return  none
 */
static uint8_t* GPEP_findHandle(uint8_t handle)
{
  //No list of GPEP messages to check with. See GP-DataCnf
  return NULL;
}

/*********************************************************************
*********************************************************************/
