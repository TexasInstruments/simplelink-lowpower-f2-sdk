
/******************************************************************************
  Filename:       ota_client_app.c
  Revised:        $Date: 2015-04-17 16:14:00 -0700 (Fri, 17 Apr 2015) $
  Revision:       $Revision: 43459 $


  Description:   Over-the-Air Upgrade Cluster ( OTA ) client application


  Copyright 2015 Texas Instruments Incorporated.

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
******************************************************************************/

/******************************************************************************
 * INCLUDES
 */
#include "ti_zstack_config.h"
#if (defined ZCL_SYSCONFIG)
#include "zcl_config.h"
#endif
#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
#include <xdc/std.h>

#include "util_timer.h"
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/BIOS.h>
#include <ti/drivers/Power.h>
#include <string.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include "zcomdef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ota.h"
#include "ota_client.h"
#include "ota_common.h"
#include <crc32.h>

#include "ti_drivers_config.h"
#include "nvintf.h"
#include "zstack.h"
#include "zcl_port.h"
#include "board_key.h"
#include "board_led.h"
#include "rom_jt_154.h"
#include "zstackapi.h"
#include "zcl_port.h"
#include "nwk_util.h"

#include <ti/drivers/utils/Random.h>

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
#include "stub_aps.h"
#endif

#if defined OTA_MMO_SIGN
#include "ota_signature.h"
#endif

#include "mac_util.h"

#include "flash_interface.h"
#include "oad_image_header.h"
#include "oad_image_header_app.h"
#include "ext_flash_layout.h"
#include "oad_switch_agama.h"
#include "sys_ctrl.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif

/*********************************************************************
 * MACROS
 */
/* Must be PROGRESS_WIDTH hashes */
#define HASHES_STR "##############################"
/* Must be PROGRESS_WITDH spaces */
#define SPACES_STR "                              "


/*********************************************************************
 * CONSTANTS
 */

// 30ms period. First Image Query request has + random jitter of 0 to 30 seconds
#define OTA_NEW_IMAGE_QUERY_RATE             30000

#define OTA_INIT_TIMEOUT_VALUE               100

#define OTA_SERVER_DISCOVERY_TIMEOUT         5000

//This is to avoid the device storming the network with many request, consider increasing this if the device is several hops away
#define MINIMUM_IMAGE_BLOCK_REQUEST_TIMEOUT  300

#define PROGRESS_WIDTH                       30

#define OTA_CLIENT_TASK_STACK_SIZE           1280

/*********************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */

uint8_t zclOTA_ClientPdState;
uint32_t zclOTA_DownloadedImageSize;  // Downloaded image size

#if defined OTA_CLIENT_STANDALONE
Task_Struct otaClientTask;
Char otaClientTaskStack[OTA_CLIENT_TASK_STACK_SIZE];
static uint8_t localOtaClientEp;
static uint8_t localOtaClient_zclAttrsArraySize;
static uint8_t localOtaClient_zclCmdsArraySize;
const zclAttrRec_t *localOtaClient_Attrs;
const zclCommandRec_t *localOtaClient_Cmds;
#endif

/******************************************************************************
 * LOCAL VARIABLES
 */

// local OTA attribute pointers
static uint8_t  *otaClient_UpgradeServerID;
static uint32_t *otaClient_FileOffset;
static uint32_t *otaClient_CurrentFileVersion;
static uint32_t *otaClient_DownloadedFileVersion;
static uint8_t  *otaClient_ImageUpgradeStatus;
static uint16_t *otaClient_ManufacturerID;
static uint16_t *otaClient_ImageType;
static uint16_t *otaClient_MinBlockReqDelay;

// Semaphore used to post otaClientEvents to the application thread
static Semaphore_Handle otaClientSemaphore;
static uint16_t otaClientEvents = 0;
static bool reloadMatchDescTimer = false;

static bool reloadQueryServerTimer = false;

/* App service ID used for messaging with stack service task */
uint8_t otaServiceTaskId;

// Clock resources for ZCL_OTA_IMAGE_BLOCK_WAIT_EVT
static ClockP_Struct imageBlockWaitClkStruct;
static ClockP_Handle imageBlockWaitClkHandle;

// Clock resources for ZCL_OTA_UPGRADE_WAIT_EVT
static ClockP_Struct upgradeWaitClkStruct;
static ClockP_Handle upgradeWaitClkHandle;

// Clock resources for ZCL_OTA_QUERY_SERVER_EVT
static ClockP_Struct queryServerClkStruct;
static ClockP_Handle queryServerClkHandle;

// Clock resources for ZCL_OTA_BLOCK_RSP_TO_EVT
static ClockP_Struct blockRspToClkStruct;
static ClockP_Handle blockRspToClkHandle;

// Clock resources for ZCL_OTA_IMAGE_QUERY_TO_EVT
static ClockP_Struct imageQueryToClkStruct;
static ClockP_Handle imageQueryToClkHandle;

// Clock resources for ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT
static ClockP_Struct imageBlockReqDelayClkStruct;
static ClockP_Handle imageBlockReqDelayClkHandle;

// Clock resources for ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT
static ClockP_Struct sendMatchDescriptorClkStruct;
static ClockP_Handle sendMatchDescriptorClkHandle;

// Retry counters
static uint8_t otaClient_BlockRetry;
static uint8_t otaClient_UpgradeEndRetry;

static zclOTA_FileID_t otaClient_CurrentDlFileId;
static cId_t otaClusters = ZCL_CLUSTER_ID_OTA;
static uint16_t otaClient_UpdateDelay;
// OTA Service Discover Information:
static bool otaClient_OtaZDPTransSeq = false;

static afAddrType_t otaClient_otaServerAddr;                         // Server address

// Image block command field control value
static uint8_t otaClient_ImageBlockFC = OTA_BLOCK_FC_REQ_DELAY_PRESENT; // set bitmask field control value(s) for device


#ifndef CUI_DISABLE
CUI_clientHandle_t gOTAClientCuiHandle;
static uint32_t gOTAClientInfoLine;
static uint32_t gOTAClientInfoLine2;
#endif

/******************************************************************************
 * LOCAL FUNCTIONS
 */

static ZStatus_t otaClient_HdlIncoming ( zclIncoming_t *pInMsg );
static ZStatus_t otaClient_ProcessImageNotify ( zclIncoming_t *pInMsg );
static ZStatus_t otaClient_ProcessQueryNextImageRsp ( zclIncoming_t *pInMsg );
static ZStatus_t otaClient_ProcessImageBlockRsp ( zclIncoming_t *pInMsg );
static ZStatus_t otaClient_ProcessUpgradeEndRsp ( zclIncoming_t *pInMsg );
static ZStatus_t otaClient_ProcessQuerySpecificFileRsp ( zclIncoming_t *pInMsg );
static void otaClient_UpgradeComplete ( uint8_t status );
static uint8_t otaClient_CmpFileId ( zclOTA_FileID_t *f1, zclOTA_FileID_t *f2 );
static void otaClient_ImageBlockWaitExpired ( void );
static ZStatus_t otaClient_sendImageBlockReq ( afAddrType_t *dstAddr );
static void otaClient_StartTimer ( ClockP_Handle cHandle, ClockP_Struct *cStruct,  uint32_t seconds);
static void otaClient_StartTimeoutEvent( ClockP_Handle cHandle, ClockP_Struct *cStruct,  uint32_t mSeconds);
static void otaClient_setPollRate(uint32_t newPollRate, bool rxOnIdle);
static uint16_t otaClient_GetRand( void );
static void otaClient_initializeClocks( void );
static void otaClient_ProcessOTAMsgs( zclOTA_CallbackMsg_t* pMsg );
static uint8_t otaClient_ProcessUnhandledFoundationZCLMsgs ( zclIncoming_t *pMsg );
static void otaClient_ProcessImageBlockWaitTimeoutCallback(UArg a0);
static void otaClient_ProcessUpgradeWaitTimeoutCallback(UArg a0);
static void otaClient_ProcessQueryServerClkStructTimeoutCallback(UArg a0);
static void otaClient_ProcessBlockRspToTimeoutCallback(UArg a0);
static void otaClient_ProcessImageQueryToTimeoutCallback(UArg a0);
static void otaClient_ProcessImageBlockReqDelayTimeoutCallback(UArg a0);
static void otaClient_ProcessSendMatchDescriptorTimeoutCallback(UArg a0);

#if defined (OTA_CLIENT_STANDALONE)
static void otaClient_Initialization(void);
static void otaClient_InitStack(void);
static void otaClient_ProcessLoop(void);
static void otaClient_SetupCallbacks(void);
static void otaClient_ProcessZstackMsgs(zstackmsg_genericReq_t *pMsg);
static void otaClient_ProcessAfIncomingMsg(zstack_afIncomingMsgInd_t *pInMsg);
static endPointDesc_t  zclOtaEpDesc = {0};
/* App service task otaClientEvents, set by the stack service task when sending a message */
static uint32_t otaServiceTaskEvents;
#endif // OTA_CLIENT_STANDALONE

#ifdef FACTORY_IMAGE
static uint8_t otaClient_EraseExtFlashPages(uint8_t imgStartPage, uint32_t imgLen, uint32_t pageSize);
static uint32_t otaClient_FindFactoryImgAddr();
#endif

/******************************************************************************
 * @fn      otaClient_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void otaClient_initializeClocks(void)
{
  imageBlockWaitClkHandle = UtilTimer_construct(
    &imageBlockWaitClkStruct,
    otaClient_ProcessImageBlockWaitTimeoutCallback,
    OTA_INIT_TIMEOUT_VALUE,
    0,
    false,
    0);


  upgradeWaitClkHandle = UtilTimer_construct(
    &upgradeWaitClkStruct,
    otaClient_ProcessUpgradeWaitTimeoutCallback,
    OTA_INIT_TIMEOUT_VALUE,
    0,
    false,
    0);

  queryServerClkHandle = UtilTimer_construct(
    &queryServerClkStruct,
    otaClient_ProcessQueryServerClkStructTimeoutCallback,
    OTA_INIT_TIMEOUT_VALUE,
    0,
    false,
    0);


  blockRspToClkHandle = UtilTimer_construct(
    &blockRspToClkStruct,
    otaClient_ProcessBlockRspToTimeoutCallback,
    OTA_INIT_TIMEOUT_VALUE,
    0,
    false,
    0);

  imageQueryToClkHandle = UtilTimer_construct(
    &imageQueryToClkStruct,
    otaClient_ProcessImageQueryToTimeoutCallback,
    OTA_INIT_TIMEOUT_VALUE,
    0,
    false,
    0);


  imageBlockReqDelayClkHandle = UtilTimer_construct(
    &imageBlockReqDelayClkStruct,
    otaClient_ProcessImageBlockReqDelayTimeoutCallback,
    OTA_INIT_TIMEOUT_VALUE,
    0,
    false,
    0);

 sendMatchDescriptorClkHandle = UtilTimer_construct(
    &sendMatchDescriptorClkStruct,
    otaClient_ProcessSendMatchDescriptorTimeoutCallback,
    OTA_SERVER_DISCOVERY_TIMEOUT,
    0,
    false,
    0);
}



/******************************************************************************
 * @fn      otaClient_Init
 *
 * @brief   Call to initialize the OTA Client Task
 *
 * @param   task_id
 *
 * @return  none
 */
void otaClient_Init ( Semaphore_Handle appSem, uint8_t stEnt, uint32_t cuiHandle)
{
  // this is not required if using OTA_CLIENT_STANDALON, as you will just be reassigning otaClientSemaphore and otaServiceTaskId
  // back to the same values. But, it is required if using OTA_CLIENT_INTEGRATED since these parameters come
  // from the main application thread
  otaClientSemaphore = appSem;
  otaServiceTaskId = stEnt;

#ifndef CUI_DISABLE
  gOTAClientCuiHandle = cuiHandle;
#endif
  // Register as a ZCL Plugin
  zcl_registerPlugin ( ZCL_CLUSTER_ID_OTA,
                       ZCL_CLUSTER_ID_OTA,
                       otaClient_HdlIncoming );

  otaClient_initializeClocks();

  otaClient_InitializeSettings ();
  otaClient_DiscoverServer();

  flash_init();

#ifndef CUI_DISABLE
  CUI_statusLineResourceRequest(gOTAClientCuiHandle, "   OTA Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gOTAClientInfoLine);
  CUI_statusLineResourceRequest(gOTAClientCuiHandle, "   OTA Info"CUI_DEBUG_MSG_START"2"CUI_DEBUG_MSG_END, false, &gOTAClientInfoLine2);
  otaClient_UpdateStatusLine();
#endif
#if defined(FACTORY_IMAGE) && defined(EXTERNAL_IMAGE_CHECK)
  // Check if external flash memory is available
  if(hasExternalFlash() == true)
  {
      // Save factory image if there is not one
      if(!otaClient_hasFactoryImage())
      {
          otaClient_saveFactoryImage();
      }
  }
#endif
}


#ifndef CUI_DISABLE
/*********************************************************************
 * @fn          otaClient_UpdateStatusLine
 *
 * @brief       Generate part of the OTA Info string
 *
 * @param       none
 *
 * @return      none
 */
extern void otaClient_UpdateStatusLine(void)
{
    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    strcat(lineFormat, "["CUI_COLOR_YELLOW"Current File Version"CUI_COLOR_RESET"] 0x%08x");

    CUI_statusLinePrintf(gOTAClientCuiHandle, gOTAClientInfoLine, lineFormat, *otaClient_CurrentFileVersion);

    char lineFormat2[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    strcat(lineFormat2, "["CUI_COLOR_YELLOW"Status"CUI_COLOR_RESET"] ");

    if (*otaClient_ImageUpgradeStatus == OTA_STATUS_NORMAL)
    {
      strcat(lineFormat2, ""CUI_COLOR_RED"Stopped"CUI_COLOR_RESET"");
      CUI_statusLinePrintf(gOTAClientCuiHandle, gOTAClientInfoLine2, lineFormat2);
    }
    else if (*otaClient_ImageUpgradeStatus == OTA_STATUS_IN_PROGRESS)
    {
      uint32_t numHashes = (*otaClient_FileOffset / (zclOTA_DownloadedImageSize / PROGRESS_WIDTH));

      strcat(lineFormat2, "In Progress [%s%.*s%.*s%s] (%d%%)");
      CUI_statusLinePrintf(gOTAClientCuiHandle, gOTAClientInfoLine2, lineFormat2,
                          CUI_COLOR_GREEN,
                          numHashes,                       HASHES_STR,
                          (PROGRESS_WIDTH - numHashes),    SPACES_STR,
                          CUI_COLOR_RESET,
                          (uint32_t)((100 * *otaClient_FileOffset) / zclOTA_DownloadedImageSize));
    }
    else
    {
      strcat(lineFormat2, ""CUI_COLOR_GREEN"Completed"CUI_COLOR_RESET"");
      CUI_statusLinePrintf(gOTAClientCuiHandle, gOTAClientInfoLine2, lineFormat2);
    }
}
#endif // CUI_DISABLE

/******************************************************************************
 * @fn      otaClient_StartTimer
 *
 * @brief   Start a ZCL OTA timer.
 *
 * @param   eventId - OSAL event set on timer expiration
 * @param   seconds - timeout in seconds
 *
 * @return  None
 */
static void otaClient_StartTimer ( ClockP_Handle cHandle, ClockP_Struct *cStruct,  uint32_t seconds )
{
  // Record the number of whole minutes to wait
  otaClient_UpdateDelay = ( seconds / 60 );

  if (UtilTimer_isActive(cStruct) == true)
  {
      UtilTimer_stop(cStruct);
  }

  // Set a timer for the remaining seconds to wait.otaClient_HdlIncoming
  UtilTimer_setTimeout(cHandle, ( seconds % 60 ) * 1000);
  UtilTimer_start(cStruct);
}

/******************************************************************************
 * @fn      otaClient_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
 ZStatus_t  otaClient_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8_t endpointStatus = FALSE;

  endpointStatus = otaClient_SetEndpoint( zcl_getRawAFMsg()->endPoint );

  if( endpointStatus == TRUE )
  {
    if ( pInMsg->msg->endPoint == currentOtaEndpoint )
    {
      if ( zcl_ClusterCmd ( pInMsg->hdr.fc.type ) )
      {
        // Is this a manufacturer specific command?
        if ( pInMsg->hdr.fc.manuSpecific == 0 )
        {
          // Is command for server?
          if ( zcl_ServerCmd ( pInMsg->hdr.fc.direction ) )
          {
            stat = ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
          }
          else // Else command is for client
          {
            switch ( pInMsg->hdr.commandID )
            {
              case COMMAND_OTA_UPGRADE_IMAGE_NOTIFY:
              {
                stat = otaClient_ProcessImageNotify ( pInMsg );
              }
              break;
              case COMMAND_OTA_UPGRADE_QUERY_NEXT_IMAGE_RESPONSE:
              {
                stat = otaClient_ProcessQueryNextImageRsp ( pInMsg );
              }
              break;
              case COMMAND_OTA_UPGRADE_IMAGE_BLOCK_RESPONSE:
              {
                stat = otaClient_ProcessImageBlockRsp ( pInMsg );
              }
              break;
              case COMMAND_OTA_UPGRADE_UPGRADE_END_RESPONSE:
              {
                stat = otaClient_ProcessUpgradeEndRsp ( pInMsg );
              }
              break;
              case COMMAND_OTA_UPGRADE_QUERY_DEVICE_SPECIFIC_FILE_RESPONSE:
              {
                stat = otaClient_ProcessQuerySpecificFileRsp ( pInMsg );
              }
              break;
              default:
              {
                stat = ZFailure;
              }
              break;
            }
          }
        }
        else
        {
          // We don't support any manufacturer specific command.
          stat = ZCL_STATUS_UNSUP_MANU_CLUSTER_COMMAND;
        }
      }
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands -- should never get here
    stat = ZFailure;
  }

 return ( stat );
}

/******************************************************************************
 * @fn          otaClient_event_loop
 *
 * @brief       Event Loop Processor for OTA Client task.
 *
 * @param       task_id - TaskId
 *              otaClientEvents - otaClientEvents
 *
 * @return      Unprocessed event bits
 */
uint16_t otaClient_event_loop ( void )
{

  if ( otaClientEvents & ZCL_OTA_IMAGE_BLOCK_WAIT_EVT )
  {
    // If the time has expired, perform the required action
    if ( otaClient_UpdateDelay == 0 )
    {
      otaClient_ImageBlockWaitExpired();
    }
    else
    {
      // Decrement the number of minutes to wait and reset the timer
      otaClient_UpdateDelay--;
      otaClient_StartTimeoutEvent( imageBlockWaitClkHandle, &imageBlockWaitClkStruct,  60000);
    }

    otaClientEvents &= ( ~ZCL_OTA_IMAGE_BLOCK_WAIT_EVT );
  }

  if ( otaClientEvents & ZCL_OTA_UPGRADE_WAIT_EVT )
  {
    // If the time has expired, perform the required action
    if ( otaClient_UpdateDelay == 0 )
    {
      if ( *otaClient_ImageUpgradeStatus == OTA_STATUS_COUNTDOWN )
      {
        otaClient_UpgradeComplete ( ZSuccess );
      }
      else if ( *otaClient_ImageUpgradeStatus == OTA_STATUS_UPGRADE_WAIT )
      {
        if ( ++otaClient_UpgradeEndRetry > OTA_MAX_END_REQ_RETRIES )
        {
          // If we have not heard from the server for N retries, perform the upgrade
          otaClient_UpgradeComplete ( ZSuccess );
        }
        else
        {
          // Send another update end request
          zclOTA_UpgradeEndReqParams_t  req;

          req.status = ZSuccess;
          OsalPort_memcpy(&req.fileId, &otaClient_CurrentDlFileId, sizeof ( zclOTA_FileID_t ));
          zclOTA_SendUpgradeEndReq (currentOtaEndpoint, &otaClient_otaServerAddr, &req );

          // Restart the timer for another hour
          otaClient_StartTimer (upgradeWaitClkHandle, &upgradeWaitClkStruct, 3600 );
        }
      }
    }
    else
    {
      // Decrement the number of minutes to wait and reset the timer
      otaClient_UpdateDelay--;
      otaClient_StartTimeoutEvent( upgradeWaitClkHandle, &upgradeWaitClkStruct,  60000);
    }

    otaClientEvents &= ( ~ZCL_OTA_UPGRADE_WAIT_EVT );
  }

  if ( otaClientEvents & ZCL_OTA_IMAGE_QUERY_TO_EVT )
  {
    if ( *otaClient_ImageUpgradeStatus == OTA_STATUS_NORMAL )// rewrite this and move the application part to here
    {
      // Notify the application task of the timeout waiting for the download to start
      zclOTA_CallbackMsg_t *pMsg;

      pMsg = ( zclOTA_CallbackMsg_t* ) OsalPort_malloc (sizeof ( zclOTA_CallbackMsg_t ));

      if ( pMsg )
      {
        pMsg->hdr.event = ZCL_OTA_CALLBACK_IND;
        pMsg->hdr.status = ZFailure;
        pMsg->ota_event = ZCL_OTA_START_CALLBACK;

        otaClient_ProcessOTAMsgs ( pMsg );
      }
    }

    otaClientEvents &= ( ~ZCL_OTA_IMAGE_QUERY_TO_EVT );
  }

  if ( otaClientEvents & ZCL_OTA_BLOCK_RSP_TO_EVT )
  {
    // We timed out waiting for a Block Response
    if ( ++otaClient_BlockRetry > OTA_MAX_BLOCK_RETRIES )
    {
      // Send a failed update end request
      zclOTA_UpgradeEndReqParams_t  req;

      req.status = ZOtaAbort;
      OsalPort_memcpy ( &req.fileId, &otaClient_CurrentDlFileId, sizeof ( zclOTA_FileID_t ) );

      zclOTA_SendUpgradeEndReq (currentOtaEndpoint, &otaClient_otaServerAddr, &req );

      otaClient_UpgradeComplete ( ZOtaAbort );
    }
    else
    {
      // Send another block request
      otaClient_sendImageBlockReq(&otaClient_otaServerAddr);
    }

    otaClientEvents &= ( ~ZCL_OTA_BLOCK_RSP_TO_EVT );
  }

  if ( otaClientEvents & ZCL_OTA_QUERY_SERVER_EVT )
  {
    if (*otaClient_ImageUpgradeStatus == OTA_STATUS_NORMAL)
    {

      zclOTA_QueryNextImageReqParams_t queryParams;

      queryParams.fieldControl = 0;
      queryParams.fileId.manufacturer = OTA_MANUFACTURER_ID;
      queryParams.fileId.type = OTA_TYPE_ID;
      queryParams.fileId.version = *otaClient_CurrentFileVersion;

      zclOTA_SendQueryNextImageReq (currentOtaEndpoint, &otaClient_otaServerAddr, &queryParams );

      // Set a timer to wait for the response
      otaClient_StartTimeoutEvent( imageQueryToClkHandle, &imageQueryToClkStruct, 10000);

    }
    otaClientEvents &= ( ~ZCL_OTA_QUERY_SERVER_EVT );
  }

  if ( otaClientEvents & ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT )
  {

    otaClient_sendImageBlockReq ( &otaClient_otaServerAddr );

    otaClientEvents &= (~ ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT );
  }

  if ( otaClientEvents & ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT )
  {
    // Look for OTA Server
    otaClient_OtaZDPTransSeq = true; // to keep track if the message was initiated by OTA or APP

    zstack_zdoMatchDescReq_t req;

    req.dstAddr = 0xFFFF;
    req.nwkAddrOfInterest = 0xFFFF;
    req.profileID = ZCL_HA_PROFILE_ID;
    req.n_inputClusters = 1;
    req.pInputClusters = &otaClusters;
    req.n_outputClusters = 0;
    req.pOutputClusters = NULL;

    Zstackapi_ZdoMatchDescReq(otaServiceTaskId, &req);

    otaClientEvents &= ( ~ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT );
  }

  // Discard unknown otaClientEvents
  return 0;
}


/******************************************************************************
 * @fn      otaClient_ProcessImageNotify
 *
 * @brief   Process received Image Notify command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t otaClient_ProcessImageNotify ( zclIncoming_t *pInMsg )
{
  zclOTA_ImageNotifyParams_t  param;
  zclOTA_QueryNextImageReqParams_t req;
  uint8_t *pData;

  // verify message length
  if ( ( pInMsg->pDataLen > PAYLOAD_MAX_LEN_IMAGE_NOTIFY ) ||
       ( pInMsg->pDataLen < PAYLOAD_MIN_LEN_IMAGE_NOTIFY ) )
  {
    // no further processing if invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // verify  in 'normal' state
  if ( ( zclOTA_Permit == FALSE ) ||
       ( *otaClient_ImageUpgradeStatus != OTA_STATUS_NORMAL ) )
  {
    return ZFailure;
  }

  // parse message
  pData = pInMsg->pData;
  param.payloadType = *pData++;
  param.queryJitter = *pData++;
  param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.version = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );

  // if message is broadcast
  if ( pInMsg->msg->wasBroadcast )
  {
    // verify manufacturer
    if ( ( param.payloadType >= NOTIFY_PAYLOAD_JITTER_MFG ) &&
         ( param.fileId.manufacturer != *otaClient_ManufacturerID ) )
    {
      return ZSuccess;
    }

    // verify image type
    if ( ( param.payloadType >= NOTIFY_PAYLOAD_JITTER_MFG_TYPE ) &&
         ( param.fileId.type != *otaClient_ImageType ) )
    {
      return ZSuccess;
    }

    // verify version; if version matches ignore
    if ( ( param.payloadType >= NOTIFY_PAYLOAD_JITTER_MFG_TYPE_VERS ) &&
         ( param.fileId.version == *otaClient_CurrentFileVersion ) )
    {
      return ZSuccess;
    }

    // get random value and compare to query jitter
    if ( ( ( uint8_t ) otaClient_GetRand() % 100 ) > param.queryJitter )
    {
      // if greater than query jitter ignore;
      return ZSuccess;
    }
  }

  // if unicast message, or broadcast and still made it here, send query next image
  req.fieldControl = 0;
  req.fileId.manufacturer = *otaClient_ManufacturerID;
  req.fileId.type = *otaClient_ImageType;
  req.fileId.version = *otaClient_CurrentFileVersion;
  zclOTA_SendQueryNextImageReq (currentOtaEndpoint, & ( pInMsg->msg->srcAddr ), &req );

  return ZSuccess;
}

/******************************************************************************
 * @fn      otaClient_ProcessQueryNextImageRsp
 *
 * @brief   Process received Query Next Image Response.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t otaClient_ProcessQueryNextImageRsp ( zclIncoming_t *pInMsg )
{
  zclOTA_QueryImageRspParams_t  param;
  uint8_t *pData;

  // verify message length
  if ( ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_RSP ) &&
       ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_QUERY_NEXT_IMAGE_RSP ) )
  {
    // no further processing if invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // ignore message if in 'download in progress' state
  if ( *otaClient_ImageUpgradeStatus == OTA_STATUS_IN_PROGRESS )
  {
    return ZSuccess;
  }

  // get status
  pData = pInMsg->pData;
  param.status = *pData++;

  // if status is success
  if ( param.status == ZCL_STATUS_SUCCESS )
  {
    // parse message
    param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.version = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );
    pData += 4;
    param.imageSize = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );

    // verify manufacturer id and image type
    if ( ( param.fileId.type == *otaClient_ImageType ) &&
         ( param.fileId.manufacturer == *otaClient_ManufacturerID ) )
    {
      // store file version and image size
      *otaClient_DownloadedFileVersion = param.fileId.version;
      zclOTA_DownloadedImageSize = param.imageSize;

      // initialize other variables
      *otaClient_FileOffset = 0;
      zclOTA_ClientPdState = ZCL_OTA_PD_MAGIC_0_STATE;

      // set state to 'in progress'
      *otaClient_ImageUpgradeStatus = OTA_STATUS_IN_PROGRESS;

      // store server address
      otaClient_otaServerAddr = pInMsg->msg->srcAddr;

      // Store the file ID
      OsalPort_memcpy ( &otaClient_CurrentDlFileId, &param.fileId, sizeof ( zclOTA_FileID_t ) );

      // send image block request
      otaClient_StartTimeoutEvent( imageBlockReqDelayClkHandle, &imageBlockReqDelayClkStruct, *otaClient_MinBlockReqDelay);

      // Request the IEEE address of the server to put into the
      // ATTRID_OTA_UPGRADE_UPGRADE_SERVER_ID attribute

      zstack_zdoIeeeAddrReq_t pReq;
      pReq.nwkAddr = pInMsg->msg->srcAddr.addr.shortAddr;
      pReq.type = zstack_NwkAddrReqType_SINGLE_DEVICE;
      pReq.startIndex = 0;

      Zstackapi_ZdoIeeeAddrReq( otaServiceTaskId, &pReq);

      if(UtilTimer_isActive(&imageQueryToClkStruct) == true)
      {
          UtilTimer_stop(&imageQueryToClkStruct);
      }
    }
    else
    {
        // Return ZCL_STATUS_MALFORMED_COMMAND if ImageType or ManufacturerID are invalid
        return ZCL_STATUS_MALFORMED_COMMAND;
    }
  }

  // Notify the application task of the failure
  zclOTA_CallbackMsg_t *pMsg;

  pMsg = ( zclOTA_CallbackMsg_t* )OsalPort_malloc( sizeof ( zclOTA_CallbackMsg_t ) );
  if ( pMsg )
  {
    pMsg->hdr.event = ZCL_OTA_CALLBACK_IND;
    pMsg->hdr.status = param.status;
    pMsg->ota_event = ZCL_OTA_START_CALLBACK;

    otaClient_ProcessOTAMsgs ( pMsg );
  }

  return ZCL_STATUS_CMD_HAS_RSP;
}

/******************************************************************************
 * @fn      otaClient_ProcessImageBlockRsp
 *
 * @brief   Process received Image Block Response.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t otaClient_ProcessImageBlockRsp ( zclIncoming_t *pInMsg )
{
  zclOTA_ImageBlockRspParams_t  param;
  zclOTA_UpgradeEndReqParams_t  req;
  uint8_t *pData;
  uint8_t status = ZSuccess;

  // verify in 'in progress' state
  if ( *otaClient_ImageUpgradeStatus != OTA_STATUS_IN_PROGRESS )
  {
    return ZSuccess;
  }

  // get status
  pData = pInMsg->pData;
  param.status = *pData++;

  // if status is success
  if ( param.status == ZCL_STATUS_SUCCESS )
  {
    // verify message length
    if ( pInMsg->pDataLen < PAYLOAD_MAX_LEN_IMAGE_BLOCK_RSP )
    {
      // no further processing if invalid
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    // parse message
    param.rsp.success.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.rsp.success.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.rsp.success.fileId.version = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );
    pData += 4;
    param.rsp.success.fileOffset = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );
    pData += 4;
    param.rsp.success.dataSize = *pData++;
    param.rsp.success.pData = pData;

    // verify manufacturer, image type, file version, file offset
    if ( ( param.rsp.success.fileId.type != *otaClient_ImageType ) ||
         ( param.rsp.success.fileId.manufacturer != *otaClient_ManufacturerID ) ||
         ( param.rsp.success.fileId.version != *otaClient_DownloadedFileVersion ) )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }
    else
    {
      // Drop duplicate packets (retries)
      if ( param.rsp.success.fileOffset != *otaClient_FileOffset )
      {
        return ZSuccess;
      }

      UtilTimer_stop(&blockRspToClkStruct);
      otaClientEvents &= (~ZCL_OTA_BLOCK_RSP_TO_EVT);
      // Stop the timer and clear the retry count
      otaClient_BlockRetry = 0;

      status = zclOTA_ProcessImageData ( param.rsp.success.pData, param.rsp.success.dataSize );


      if ( status == ZSuccess )
      {
        if ( *otaClient_ImageUpgradeStatus == OTA_STATUS_COMPLETE )
        {
          // send upgrade end req with success status
          OsalPort_memcpy ( &req.fileId, &param.rsp.success.fileId, sizeof ( zclOTA_FileID_t ) );

          req.status = ZSuccess;
          zclOTA_SendUpgradeEndReq (currentOtaEndpoint, & ( pInMsg->msg->srcAddr ), &req );
        }
        else
        {
          // send image block request using rate limiting
          otaClient_StartTimeoutEvent( imageBlockReqDelayClkHandle, &imageBlockReqDelayClkStruct, *otaClient_MinBlockReqDelay);
        }
      }
    }
  }
  // else if status is 'wait for data'
  else if ( param.status == ZCL_STATUS_WAIT_FOR_DATA )
  {
    // verify message length
    if ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_IMAGE_BLOCK_WAIT )
    {
      // no further processing if invalid
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    // parse message
    param.rsp.wait.currentTime = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );
    pData += 4;
    param.rsp.wait.requestTime = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );
    pData += 4;
    param.rsp.wait.blockReqDelay = BUILD_UINT16 ( pData[0], pData[1] );

    // check to see if device supports blockReqDelay rate limiting
    if ( ( otaClient_ImageBlockFC & OTA_BLOCK_FC_REQ_DELAY_PRESENT ) != 0 )
    {
      if ( ( param.rsp.wait.requestTime - param.rsp.wait.currentTime ) > 0 )
      {
        // Stop the timer and clear the retry count
        otaClient_BlockRetry = 0;
        if(UtilTimer_isActive(&blockRspToClkStruct) == true)
        {
          UtilTimer_stop(&blockRspToClkStruct);
        }

        // set timer for next image block req
        otaClient_StartTimer (imageBlockWaitClkHandle,&imageBlockWaitClkStruct,
                            ( param.rsp.wait.requestTime - param.rsp.wait.currentTime ) );
      }
      else
      {
        // if wait timer delta is 0, then update device with blockReqDelay value and use rate limiting
        *otaClient_MinBlockReqDelay = param.rsp.wait.blockReqDelay;

        otaClient_StartTimeoutEvent( imageBlockReqDelayClkHandle, &imageBlockReqDelayClkStruct, *otaClient_MinBlockReqDelay + MINIMUM_IMAGE_BLOCK_REQUEST_TIMEOUT);
      }
    }
    else
    {
      // Stop the timer and clear the retry count
      otaClient_BlockRetry = 0;
      if(UtilTimer_isActive(&blockRspToClkStruct) == true)
      {
        UtilTimer_stop(&blockRspToClkStruct);
      }


      // set timer for next image block req
      otaClient_StartTimer (imageBlockWaitClkHandle,&imageBlockWaitClkStruct,
                            ( param.rsp.wait.requestTime - param.rsp.wait.currentTime ) );
    }
  }
  else if ( param.status == ZCL_STATUS_ABORT )
  {
    // download aborted; set state to 'normal' state
    *otaClient_ImageUpgradeStatus = OTA_STATUS_NORMAL;
    //reset endpoint
    otaClient_SetEndpoint(0);
    // Stop the timer and clear the retry count
    otaClient_BlockRetry = 0;
    if(UtilTimer_isActive(&blockRspToClkStruct) == true)
    {
       UtilTimer_stop(&blockRspToClkStruct);
    }

    otaClient_UpgradeComplete ( ZOtaAbort );

    return ZSuccess;
  }
  else
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  if ( status != ZSuccess )
  {
    // download failed; set state to 'normal'
    *otaClient_ImageUpgradeStatus = OTA_STATUS_NORMAL;

    otaClient_SetEndpoint(0);

    // send upgrade end req with failure status
    OsalPort_memcpy ( &req.fileId, &param.rsp.success.fileId, sizeof ( zclOTA_FileID_t ) );

    req.status = status;
    zclOTA_SendUpgradeEndReq (currentOtaEndpoint, & ( pInMsg->msg->srcAddr ), &req );
  }

  return ZSuccess;
}



void otaClient_ProcessIEEEAddrRsp( ZDO_NwkIEEEAddrResp_t *pMsg )
{
    // If this is from the OTA server, record the server's IEEE address

    if ( pMsg->nwkAddr == otaClient_otaServerAddr.addr.shortAddr )
    {
      OsalPort_memcpy ( &otaClient_UpgradeServerID, pMsg->extAddr, Z_EXTADDR_LEN );
    }

}

bool otaClient_ProcessMatchDescRsp ( ZDO_MatchDescRsp_t *pMsg )
{
  if ( otaClient_OtaZDPTransSeq == true ) // cant check this modify to use something else
  {
    if ( pMsg->status == ZSuccess && pMsg->cnt )
    {
      otaClient_otaServerAddr.addrMode = ( afAddrMode_t ) Addr16Bit;
      otaClient_otaServerAddr.addr.shortAddr = pMsg->nwkAddr;
      // Take the first endpoint, Can be changed to search through endpoints
      otaClient_otaServerAddr.endPoint = pMsg->epList[0];
      if(UtilTimer_isActive(&sendMatchDescriptorClkStruct) == true)
      {
        UtilTimer_stop(&sendMatchDescriptorClkStruct);
        reloadMatchDescTimer = false;
      }
      otaClient_OtaZDPTransSeq = false;
    }
    return true;
  }
  return false;
}

/******************************************************************************
 * @fn      otaClient_ProcessUpgradeEndRsp
 *
 * @brief   Process received Upgrade End Response.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t otaClient_ProcessUpgradeEndRsp ( zclIncoming_t *pInMsg )
{
  zclOTA_UpgradeEndRspParams_t  param;
  zclOTA_FileID_t currentFileId = {*otaClient_ManufacturerID, *otaClient_ImageType, *otaClient_DownloadedFileVersion};
  uint8_t *pData;


  // Clear the Upgrade End Request transaction sequence number.  At this stage
  // it's no longer needed.
  zclOTA_OtaUpgradeEndReqTransSeq = 0;

  // verify message length
  if ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_UPGRADE_END_RSP )
  {
    // no further processing if invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // parse message
  pData = pInMsg->pData;
  param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.version = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );
  pData += 4;
  param.currentTime = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );
  pData += 4;
  param.upgradeTime = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );

  if ( ( param.fileId.type != *otaClient_ImageType ) ||
         ( param.fileId.manufacturer != *otaClient_ManufacturerID ) )
  {
    // Return ZCL_STATUS_MALFORMED_COMMAND if ImageType or ManufacturerID are invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // verify in 'download complete'  or 'waiting for upgrade' state
  if ( ( *otaClient_ImageUpgradeStatus == OTA_STATUS_COMPLETE ) ||
       ( ( *otaClient_ImageUpgradeStatus == OTA_STATUS_UPGRADE_WAIT ) && ( param.upgradeTime!=OTA_UPGRADE_TIME_WAIT ) ) )
  {
    // verify manufacturer, image type
    if ( otaClient_CmpFileId ( &param.fileId, &currentFileId ) == FALSE )
    {
      return ZSuccess;
    }

    // check upgrade time
    if ( param.upgradeTime != OTA_UPGRADE_TIME_WAIT )
    {
      uint32_t notifyDelay = 0;

      if ( param.upgradeTime > param.currentTime )
      {
        // time to wait before notification
        notifyDelay = param.upgradeTime - param.currentTime;
      }

      // set state to 'countdown'
      *otaClient_ImageUpgradeStatus = OTA_STATUS_COUNTDOWN;
      // set timer for upgrade complete notification
      otaClient_StartTimer (upgradeWaitClkHandle, &upgradeWaitClkStruct, notifyDelay );
    }
    else
    {
      // Wait for another upgrade end response
      *otaClient_ImageUpgradeStatus = OTA_STATUS_UPGRADE_WAIT;
      // Set a timer for 60 minutes to send another Upgrade End Rsp
      otaClient_StartTimer (upgradeWaitClkHandle, &upgradeWaitClkStruct, 3600 );
      otaClient_UpgradeEndRetry = 0;
    }
  }

  return ZSuccess;
}

/******************************************************************************
 * @fn      otaClient_ProcessQuerySpecificFileRsp
 *
 * @brief   Process received Query Specific File Response.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t otaClient_ProcessQuerySpecificFileRsp ( zclIncoming_t *pInMsg )
{
  zclOTA_QueryImageRspParams_t  param;
  uint8_t *pData;

  // verify message length
  if ( ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_QUERY_SPECIFIC_FILE_RSP ) &&
       ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_QUERY_SPECIFIC_FILE_RSP ) )
  {
    // no further processing if invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // ignore message if in 'download in progress' state
  if ( *otaClient_ImageUpgradeStatus == OTA_STATUS_IN_PROGRESS )
  {
    return ZSuccess;
  }

  // get status
  pData = pInMsg->pData;
  param.status = *pData++;

  // if status is success
  if ( param.status == ZCL_STATUS_SUCCESS )
  {
    // parse message
    param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.version = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );
    pData += 4;
    param.imageSize = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );//OsalPort_buildUint32 ( pData, 4 );

    // verify manufacturer id and image type
    if ( ( param.fileId.type == *otaClient_ImageType ) &&
         ( param.fileId.manufacturer == *otaClient_ManufacturerID ) )
    {
      // store file version and image size
      *otaClient_DownloadedFileVersion = param.fileId.version;
      zclOTA_DownloadedImageSize = param.imageSize;

      // initialize other variables
      *otaClient_FileOffset = 0;

      // set state to 'in progress'
      *otaClient_ImageUpgradeStatus = OTA_STATUS_IN_PROGRESS;

      // send image block request
      otaClient_sendImageBlockReq ( & ( pInMsg->msg->srcAddr ) );
    }
    else
    {
      // Return ZCL_STATUS_MALFORMED_COMMAND if ImageType or ManufacturerID are invalid
        return ZCL_STATUS_MALFORMED_COMMAND;
    }
  }

  return ZSuccess;
}

/******************************************************************************
 * @fn      otaClient_UpgradeComplete
 *
 * @brief   Notify the application task that an upgrade has completed.
 *
 * @param   status - The status of the upgrade
 *
 * @return  none
 */
static void otaClient_UpgradeComplete ( uint8_t status )
{
  // Go back to the normal state
  *otaClient_ImageUpgradeStatus = OTA_STATUS_NORMAL;

  if ( ( zclOTA_DownloadedImageSize == OTA_HEADER_LEN_MIN_ECDSA ) ||
       ( zclOTA_DownloadedImageSize == OTA_HEADER_LEN_MIN ) )
  {
    status = ZFailure;
  }

  // Notify the application task the upgrade stopped
  zclOTA_CallbackMsg_t *pMsg;

  pMsg = ( zclOTA_CallbackMsg_t* )OsalPort_malloc( sizeof ( zclOTA_CallbackMsg_t ) );

  if ( pMsg )
  {
    pMsg->hdr.event = ZCL_OTA_CALLBACK_IND;
    pMsg->hdr.status = status;
    pMsg->ota_event = ZCL_OTA_DL_COMPLETE_CALLBACK;

    otaClient_ProcessOTAMsgs ( pMsg );
  }

}

/******************************************************************************
 * @fn      otaClient_CmpFileId
 *
 * @brief   Called to compare two file IDs
 *
 * @param   f1, f2 - Pointers to the two file IDs to compare
 *
 * @return  TRUE if the file IDs are the same, else FALSE
 */
static uint8_t otaClient_CmpFileId ( zclOTA_FileID_t *f1, zclOTA_FileID_t *f2 )
{
  if ( ( f1->manufacturer == 0xFFFF ) ||
       ( f2->manufacturer == 0xFFFF ) ||
       ( f1->manufacturer == f2->manufacturer ) )
  {
    if ( ( f1->type == 0xFFFF ) ||
         ( f2->type == 0xFFFF ) ||
         ( f1->type == f2->type ) )
    {
      if ( ( f1->version == 0xFFFFFFFF ) ||
           ( f2->version == 0xFFFFFFFF ) ||
           ( f1->version == f2->version ) )
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/******************************************************************************
 * @fn      zclOTA_ImageBlockWaitExpired
 *
 * @brief   Perform action on image block wait timer expiration.
 *
 * @param   none
 *
 * @return  none
 */
static void otaClient_ImageBlockWaitExpired ( void )
{
  // verify in 'in progress' state
  if ( *otaClient_ImageUpgradeStatus == OTA_STATUS_IN_PROGRESS )
  {
    // request next block
    otaClient_sendImageBlockReq ( &otaClient_otaServerAddr );
  }
}

/******************************************************************************
 * @fn      otaClient_sendImageBlockReq
 *
 * @brief   Send an Image Block Request.
 *
 * @param   dstAddr - where you want the message to go
 *
 * @return  ZStatus_t
 */
static ZStatus_t otaClient_sendImageBlockReq ( afAddrType_t *dstAddr )
{
  zclOTA_ImageBlockReqParams_t req;

  req.fieldControl = otaClient_ImageBlockFC; // Image block command field control value
  req.fileId.manufacturer = *otaClient_ManufacturerID;
  req.fileId.type = *otaClient_ImageType;
  req.fileId.version = *otaClient_DownloadedFileVersion;
  req.fileOffset = *otaClient_FileOffset;

  if ( zclOTA_DownloadedImageSize - *otaClient_FileOffset < OTA_MAX_MTU )
  {
    req.maxDataSize = zclOTA_DownloadedImageSize - *otaClient_FileOffset;
  }
  else
  {
    req.maxDataSize = OTA_MAX_MTU;
  }

  req.blockReqDelay = *otaClient_MinBlockReqDelay;

  // Start a timer waiting for a response
  otaClient_StartTimeoutEvent( blockRspToClkHandle, &blockRspToClkStruct,  OTA_MAX_BLOCK_RSP_WAIT_TIME);

  return zclOTA_SendImageBlockReq (currentOtaEndpoint, dstAddr, &req );
}
/******************************************************************************
 * @fn      otaClient_RequestNextUpdate
 *
 * @brief   Called by an application after discovery of the OTA server
 *          to initiate the query next image of the OTA server.
 *
 * @param   srvAddr - Short address of the server
 * @param   srvEndPoint - Endpoint on the server
 *
 * @return  none
 */
void otaClient_RequestNextUpdate ( uint16_t srvAddr, uint8_t srvEndPoint )
{
  // Record the server address
  otaClient_otaServerAddr.addrMode = afAddr16Bit;
  otaClient_otaServerAddr.endPoint = srvEndPoint;
  otaClient_otaServerAddr.addr.shortAddr = srvAddr;

  // Set an event to query the server
  otaClientEvents |= ZCL_OTA_QUERY_SERVER_EVT;
}

/******************************************************************************
 * @fn      otaClient_DiscoverServer
 *
 * @brief   Call to discover the OTA Client
 *
 * @param   task_id
 *
 * @return  none
 */

void otaClient_DiscoverServer( void )//uint8_t task_id )
{
  // Per section 6.1 of ZigBee Over-the-Air Upgrading Cluster spec, we should
  // periodically query the server. It does not specify the rate.  For example's
  // sake, here we query the server periodically between 30-60s.
  uint32_t queryImgJitter = ( ( uint32_t ) otaClient_GetRand() % OTA_NEW_IMAGE_QUERY_RATE ) + ( uint32_t ) OTA_NEW_IMAGE_QUERY_RATE;
  reloadQueryServerTimer = true;
  otaClient_StartTimeoutEvent( queryServerClkHandle, &queryServerClkStruct, queryImgJitter);
  //OsalPortTimers_startReloadTimer (  ZCL_OTA_QUERY_SERVER_EVT, queryImgJitter );

  // Wake up in 5 seconds and do some service discovery for an OTA Server
  queryImgJitter = ( ( uint32_t ) OTA_SERVER_DISCOVERY_TIMEOUT );
  //OsalPortTimers_startReloadTimer ( ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT, queryImgJitter );
  reloadMatchDescTimer = true;
  otaClient_StartTimeoutEvent( sendMatchDescriptorClkHandle, &sendMatchDescriptorClkStruct, queryImgJitter);

  // Initiliaze OTA Update End Request Transaction Seq Number
  zclOTA_OtaUpgradeEndReqTransSeq = 0;

}

/******************************************************************************
 * @fn      otaClient_InitializeSettings
 *
 * @brief   Call to initialize the OTA Client
 *
 * @param   task_id
 *
 * @return  none
 */

void otaClient_InitializeSettings( void )
{
  memset ( otaClient_UpgradeServerID, 0xFF, sizeof ( otaClient_UpgradeServerID ) );

  *otaClient_ImageType = OTA_TYPE_ID;

  *otaClient_ManufacturerID = OTA_MANUFACTURER_ID;
}

/******************************************************************************
 * @fn      otaClient_SetEndpoint
 *
 * @brief   Set OTA endpoint.
 *
 * @param   endpoint - endpoint ID from which OTA functions can be accessed
 *
 * @return  TRUE if endpoint set, else FALSE
 */

bool otaClient_SetEndpoint( uint8_t endpoint )
{
  if( currentOtaEndpoint == OTA_UNUSED_ENDPOINT )
  {
    currentOtaEndpoint = endpoint;
    // Register for all OTA End Point, unhandled, ZCL foundation commands
    zclport_registerZclHandleExternal(currentOtaEndpoint, otaClient_ProcessUnhandledFoundationZCLMsgs);
    return TRUE;
  }
  else if( endpoint == currentOtaEndpoint)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/******************************************************************************
 * @fn      otaClient_ProcessImageBlockWaitTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void otaClient_ProcessImageBlockWaitTimeoutCallback(UArg a0)
{
  (void) a0;  // Parameter is not used

  otaClientEvents |= ZCL_OTA_IMAGE_BLOCK_WAIT_EVT;

  // Wake up the application thread when it waits for clock event
  Semaphore_post(otaClientSemaphore);
}
/******************************************************************************
 * @fn      otaClient_ProcessUpgradeWaitTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void otaClient_ProcessUpgradeWaitTimeoutCallback(UArg a0)
{
  (void) a0;  // Parameter is not used

  otaClientEvents |= ZCL_OTA_UPGRADE_WAIT_EVT;

  // Wake up the application thread when it waits for clock event
  Semaphore_post(otaClientSemaphore);
}
/******************************************************************************
 * @fn      otaClient_ProcessQueryServerClkStructTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void otaClient_ProcessQueryServerClkStructTimeoutCallback(UArg a0)
{
  (void) a0;  // Parameter is not used

  otaClientEvents |= ZCL_OTA_QUERY_SERVER_EVT;
  if ( reloadQueryServerTimer )
  {
      uint32_t queryImgJitter = OTA_NEW_IMAGE_QUERY_RATE;
      UtilTimer_setTimeout(queryServerClkHandle, queryImgJitter);

      UtilTimer_start(&queryServerClkStruct);
  }

  // Wake up the application thread when it waits for clock event
  Semaphore_post(otaClientSemaphore);
}
/******************************************************************************
 * @fn      otaClient_ProcessBlockRspToTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void otaClient_ProcessBlockRspToTimeoutCallback(UArg a0)
{
  (void) a0;  // Parameter is not used

  otaClientEvents |= ZCL_OTA_BLOCK_RSP_TO_EVT;

  // Wake up the application thread when it waits for clock event
  Semaphore_post(otaClientSemaphore);
}
/******************************************************************************
 * @fn      otaClient_ProcessImageQueryToTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void otaClient_ProcessImageQueryToTimeoutCallback(UArg a0)
{
  (void) a0;  // Parameter is not used

  otaClientEvents |= ZCL_OTA_IMAGE_QUERY_TO_EVT;

  // Wake up the application thread when it waits for clock event
  Semaphore_post(otaClientSemaphore);
}
/******************************************************************************
 * @fn      otaClient_ProcessImageBlockReqDelayTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void otaClient_ProcessImageBlockReqDelayTimeoutCallback(UArg a0)
{
  (void) a0;  // Parameter is not used

  otaClientEvents |= ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT;

  // Wake up the application thread when it waits for clock event
  Semaphore_post(otaClientSemaphore);
}
/******************************************************************************
 * @fn      otaClient_ProcessSendMatchDescriptorTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void otaClient_ProcessSendMatchDescriptorTimeoutCallback(UArg a0)
{
  (void) a0;  // Parameter is not used

  otaClientEvents |= ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT;

  if ( reloadMatchDescTimer )
  {
      UtilTimer_setTimeout(sendMatchDescriptorClkHandle, OTA_SERVER_DISCOVERY_TIMEOUT);
      UtilTimer_start(&sendMatchDescriptorClkStruct);
  }

  // Wake up the application thread when it waits for clock event
  Semaphore_post(otaClientSemaphore);
}


static void otaClient_StartTimeoutEvent ( ClockP_Handle cHandle, ClockP_Struct *cStruct,  uint32_t mSeconds)
{
  if(UtilTimer_isActive(cStruct) == true)
  {
    UtilTimer_stop(cStruct);
  }

  UtilTimer_setTimeout(cHandle, mSeconds);

  UtilTimer_start(cStruct);

}

/*********************************************************************
 * @fn      otaClient_ProcessOTAMsgs
 *
 * @brief   Called to process callbacks from the ZCL OTA.
 *
 * @param   none
 *
 * @return  none
 */
static void otaClient_ProcessOTAMsgs( zclOTA_CallbackMsg_t* pMsg )
{
  bool RxOnIdle;

  switch(pMsg->ota_event)
  {
  case ZCL_OTA_START_CALLBACK:
    if (pMsg->hdr.status == ZSuccess)
    {
      // Speed up the poll rate
      RxOnIdle = true;
      otaClient_setPollRate(2000, RxOnIdle);
    }
    break;

  case ZCL_OTA_DL_COMPLETE_CALLBACK:
    if (pMsg->hdr.status == ZSuccess)
    {

      otaClient_loadExtImage(ST_FULL_IMAGE);
    }
    else
    {
#if (ZG_BUILD_ENDDEVICE_TYPE)
      // slow the poll rate back down.
      RxOnIdle = false;
      otaClient_setPollRate(POLL_RATE_MAX, RxOnIdle);
#endif
    }
    break;

  default:
    break;
  }

  OsalPort_free(pMsg);
}

/******************************************************************************
 * @fn      zclOTA_ProcessZCLMsgs
 *
 * @brief   Process unhandled foundation ZCL messages for the OTA End Point.
 *
 * @param   pMsg - a Pointer to the ZCL message
 *
 * @return  none
 */
static uint8_t otaClient_ProcessUnhandledFoundationZCLMsgs ( zclIncoming_t *pMsg )
{
  zclIncomingMsg_t *pCmd;

  pCmd = (zclIncomingMsg_t *)OsalPort_msgAllocate( sizeof ( zclIncomingMsg_t ) );
  if ( pCmd != NULL )
  {
    // fill in the message
    pCmd->hdr.event = ZCL_INCOMING_MSG;
    pCmd->zclHdr    = pMsg->hdr;
    pCmd->clusterId = pMsg->msg->clusterId;
    pCmd->srcAddr   = pMsg->msg->srcAddr;
    pCmd->endPoint  = pMsg->msg->endPoint;
    pCmd->attrCmd   = pMsg->attrCmd;


    switch ( pCmd->zclHdr.commandID )
    {
      case ZCL_CMD_DEFAULT_RSP:
        zclOTA_ProcessInDefaultRspCmd( pCmd );
        break;
      default :
        break;
    }
    OsalPort_msgDeallocate((uint8_t *)pCmd);
  }

  if ( pMsg->attrCmd )
  {
    //OsalPort_free( pMsg->attrCmd );
    OsalPort_free(pMsg->attrCmd);
    pMsg->attrCmd = NULL;
  }

  return 0;
}
/*static void OTA_SystemReset( void )
{
  zstack_sysResetReq_t restReq;
  restReq.newNwkState = false;
  (void)Zstackapi_sysResetReq(otaServiceTaskId, &restReq);
}*/

/******************************************************************************
 * @fn      otaClient_setPollRate
 *
 * @brief   Set the ZStack Thread Poll Rate
 *
 * @param   newPollRate - new poll rate in milliseconds
 *
 * @return  none
 */
static void otaClient_setPollRate(uint32_t newPollRate, bool rxOnIdle )
{
  zstack_sysConfigWriteReq_t  writeReq = { 0 };

  // Set RX on when idle
  writeReq.has_macRxOnIdle = true;
  writeReq.macRxOnIdle = rxOnIdle;
  // Set the new poll rate
  writeReq.has_pollRate = true;
  writeReq.pollRate = newPollRate;
  writeReq.pollRateType = POLL_RATE_TYPE_APP_1;

  (void) Zstackapi_sysConfigWriteReq(otaServiceTaskId, &writeReq);
}

/*********************************************************************
 * @fn        otaClient_GetRand
 *
 * @brief    Random number generator
 *
 * @param   none
 *
 * @return  uint16_t - new random number
 *
 *********************************************************************/
static uint16_t otaClient_GetRand( void )
{
  uint16_t randNum;

  randNum = (uint8_t)(Random_getNumber()&0xFF);
  randNum += (((uint8_t)(Random_getNumber()&0xFF)) << 8);
  return ( randNum );
}

/******************************************************************************
 * @fn          SampleApp_applyFactoryImage
 *
 * @brief       Load the factory image for the SampleApp from external flash
 *              and reboot.
 *
 * @param       none
 *
 * @return      none
 */
void otaClient_loadExtImage(uint8_t imageSelect)
{

  /* Zigbee OAD assumptions:
   * Factory New Metadata and binary image exist in external flash.
   * Zigbee OAD will always take the next slot available after FN header and binary
   */

  if(imageSelect == ST_FULL_IMAGE)
  {
#ifndef OTA_ONCHIP
      if(flash_open() == TRUE)
      {
          // Copy the metadata to the meta page
          ExtImageInfo_t storedImgHdr;
          readFlashPg(EFL_FACT_IMG_META_PG + 1, 0, (uint8_t *)&storedImgHdr,
                      sizeof(ExtImageInfo_t));

          // Populate ext imge info struct
          ExtImageInfo_t extFlMetaHdr;

          // Copy the Metadata header
          OsalPort_memcpy((uint8_t *)&extFlMetaHdr, (uint8_t *)&storedImgHdr,
                  sizeof(ExtImageInfo_t));

          extFlMetaHdr.fixedHdr.imgCpStat = NEED_COPY;

          // Store the metadata
          writeFlashPg(EFL_FACT_IMG_META_PG + 1, 0, (uint8_t *)&extFlMetaHdr, sizeof(ExtImageInfo_t));

          flash_close();

          /* press the virtual reset button */
          SysCtrlSystemReset();
      }
#else
      /* press the virtual reset button */
      SysCtrlSystemReset();
#endif
  }
  else if(imageSelect == ST_FULL_FACTORY_IMAGE)
  {
      OAD_markSwitch();
  }

}

#ifdef FACTORY_IMAGE
/******************************************************************************
 * @fn          otaClient_hasFactoryImage
 *
 * @brief   This function check if the valid factory image exists on external
 *          flash
 *
 * @param   None
 *
 * @return  TRUE If factory image exists on external flash, else FALSE
 *
 */
bool otaClient_hasFactoryImage(void)
{
#if defined(EXTERNAL_IMAGE_CHECK)
  bool rtn = FALSE;
    /* initialize external flash driver */
    if(flash_open() != 0)
    {
        // First check if there is a need to create the factory image
        imgHdr_t metadataHdr;

        // Read First metadata page for getting factory image information
        if(readFlash(EFL_ADDR_META_FACT_IMG, (uint8_t *)&metadataHdr, EFL_METADATA_LEN) == FLASH_SUCCESS)
        {
          /* check Metadata version */
          if( (metadataHdr.fixedHdr.imgType == OAD_IMG_TYPE_FACTORY) &&
              (metadataHdr.fixedHdr.crcStat != CRC_INVALID) )  /* Not an invalid CRC */
          {
              rtn = TRUE; /* Factory image exists return from here */
          }
        }
        //close flash
        flash_close();
    }
    return rtn;
#else
  return (true);
#endif
}


/*******************************************************************************
 * @fn      otaClient_saveFactoryImage
 *
 * @brief   This function creates factory image backup of current running image
 *
 * @return  rtn  OTA_Storage_Status_Success/OTA_Storage_FlashError
 */
uint8_t otaClient_saveFactoryImage(void)
{
  uint8_t rtn = OTA_Storage_Status_Success;
  uint32_t dstAddr = otaClient_FindFactoryImgAddr();
  uint32_t dstAddrStart = dstAddr;
  uint32_t imgStart = _imgHdr.imgPayload.startAddr;
  uint32_t imgLen = _imgHdr.fixedHdr.imgEndAddr - (uint32_t)&_imgHdr;

  /* initialize external flash driver */
  if(flash_open() == TRUE)
  {
      // First erase factory image metadata page
      if(eraseFlashPg(EXT_FLASH_PAGE(EFL_ADDR_META)) != FLASH_SUCCESS)
      {
          /* close driver */
          flash_close();
          return OTA_Storage_FlashError;
      }

      /* Erase - external portion to be written*/
      if(otaClient_EraseExtFlashPages(EXT_FLASH_PAGE(dstAddr),
        (_imgHdr.fixedHdr.imgEndAddr - _imgHdr.imgPayload.startAddr + 1),
        EFL_PAGE_SIZE) == OTA_Storage_Status_Success)
      {
          /* COPY - image from internal to external */
        if(writeFlash(dstAddr, (uint8_t *)(imgStart), imgLen) == FLASH_SUCCESS)
          {
              imgHdr_t imgHdr = { .fixedHdr.imgID = OAD_EXTFL_ID_VAL }; /* Write OAD flash metadata identification */

              /* Copy Image header from internal flash image, skip ID values */
              memcpy( ((uint8_t *)&imgHdr + CRC_OFFSET), ((uint8_t *)imgStart + 8) , OAD_IMG_HDR_LEN);

              /*
              * Calculate the CRC32 value and update that in image header as CRC32
              * wouldn't be available for running image.
              */
              imgHdr.fixedHdr.crc32 = CRC32_calc(imgHdr.imgPayload.startAddr, INTFLASH_PAGE_SIZE, 0,  imgLen, false);

              /* Update CRC status */
              imgHdr.fixedHdr.crcStat = CRC_VALID;

              /* Update image length */
              imgHdr.fixedHdr.len = imgHdr.fixedHdr.imgEndAddr - (uint32_t)&_imgHdr;

              uint32_t *ptr = (uint32_t *)&imgHdr;

              /* update external flash storage address */
              ptr[OAD_IMG_HDR_LEN/4] = dstAddrStart;

              /* Allow application or some other place in BIM to mark factory image as
                pending copy (OAD_IMG_COPY_PEND). Should not be done here, as
                what is in flash at this time will already be the factory
                image. */
              imgHdr.fixedHdr.imgCpStat = DEFAULT_STATE;
              imgHdr.fixedHdr.imgType = OAD_IMG_TYPE_FACTORY;

              /* WRITE METADATA */
              if(writeFlash(EFL_ADDR_META, (uint8_t *)&imgHdr, OAD_IMG_HDR_LEN + 8) != FLASH_SUCCESS)
              {
                  rtn = OTA_Storage_FlashError;
              }
          } // end of if(writeFlash(...))
          else
          {
              rtn = OTA_Storage_FlashError;
          }
      }
      else //  if(extFlashErase(dstAddr, imgLen))
      {
          rtn = OTA_Storage_FlashError;
      } //  end of if(extFlashErase(dstAddr, imgLen))

      /* close driver */
      flash_close();

    } // end of flash_Open

  return(rtn);
}


/*********************************************************************
 * @fn      otaClient_FindFactoryImgAddr
 *
 * @brief   Find a place for factory image in external flash
 *          This will grow the image down from the top of flash
 *
 * @return  destAddr   Destination of Factory image in ext fl
 */
static uint32_t otaClient_FindFactoryImgAddr()
{
    // Create factory image if there isn't one
    uint32_t imgLen = _imgHdr.fixedHdr.imgEndAddr - (uint32_t)&_imgHdr;
    uint8_t numFlashPages = EXT_FLASH_PAGE(imgLen);
    if(EXTFLASH_PAGE_MASK & imgLen)
    {
        numFlashPages += 1;
    }
    // Note currently we have problem in erasing last flash page,
    // workaround to leave last page
    return (EFL_FLASH_SIZE - EXT_FLASH_ADDRESS(numFlashPages + 1, 0));
}


/*********************************************************************
 * @fn      otaClient_EraseExtFlashPages
 *
 * @brief   This function erases external flash pages
 *
 * @param   imgStartPage  Image start page on external flash
 * @param   imgLen        Image length
 * @param   pageSize      Page size of external flash.
 *
 * @return  status        OTA_Storage_Status_Success/OTA_Storage_FlashError
 *
 */
static uint8_t otaClient_EraseExtFlashPages(uint8_t imgStartPage, uint32_t imgLen, uint32_t pageSize)
{
    if(pageSize == 0 )
    {
        return OTA_Storage_FlashError;
    }
    uint8_t status = OTA_Storage_Status_Success;
    uint8_t page;
    uint8_t numFlashPages = imgLen/pageSize;
    if(0 != (imgLen % pageSize))
    {
        numFlashPages += 1;
    }

    // Erase the correct amount of pages
    for(page=imgStartPage; page<(imgStartPage + numFlashPages); ++page)
    {
        uint8_t flashStat = eraseFlashPg(page);
        if(flashStat == FLASH_FAILURE)
        {
            // If we fail to pre-erase, then halt the OTA process
            status = OTA_Storage_FlashError;
            break;
        }
    }
    return status;
}
#endif

#if defined OTA_CLIENT_STANDALONE
/******************************************************************************
 *
 * @fn      otaClientTaskFxn
 *
 * @brief   Function to kick off OTA task
 *
 * @param
 *
 * @return  none
 */
Void otaClient_TaskFxn(UArg a0, UArg a1)
{
#if defined(USE_CACHE_RAM)
    /* Retain the Cache RAM */
    Power_setConstraint(PowerCC26XX_SB_VIMS_CACHE_RETAIN);
#endif
    /* Kick off application */
    otaClient_Initialization();
}

/******************************************************************************
 *
 * @fn      otaClientCreateTask
 *
 * @brief   Call to pass generated endpoint information and initialize task
 *
 * @param   endpoint - Endpoint to register
 *          epDesc  - Endpoint Descriptor
 *          attrArraySize - Size of attribute array
 *          attrs - Array of endpoint attributes
 *          cmdsArraySize - Size of command array
 *          cmds  - Array of commands for endpoint
 *
 * @return  none
 */
void otaClient_CreateTask(uint8_t endpoint, endPointDesc_t epDesc, uint8_t attrArraySize,
                     const zclAttrRec_t attrs[], uint8_t cmdsArraySize, const zclCommandRec_t cmds[])
{
    localOtaClientEp = endpoint;
    zclOtaEpDesc = epDesc;
    localOtaClient_zclAttrsArraySize = attrArraySize;
    localOtaClient_Attrs = attrs;
    localOtaClient_zclCmdsArraySize = cmdsArraySize;
    localOtaClient_Cmds = cmds;

    otaClient_setAttributes( attrs, attrArraySize );
    zclOTA_setAttributes( attrs, attrArraySize );

    Task_Params taskParams;

    Task_Params_init(&taskParams);
    taskParams.stack = otaClientTaskStack;
    taskParams.stackSize = OTA_CLIENT_TASK_STACK_SIZE;
    taskParams.priority = 1;
    Task_construct(&otaClientTask, otaClient_TaskFxn, &taskParams, NULL);
}

/******************************************************************************
 *
 * @fn      otaClient_Initialization
 *
 * @brief   Call to register the task with OSAL and kick off OTA app
 *
 * @param
 *
 * @return  none
 */
static void otaClient_Initialization(void)
{
    /*
    create semaphores for messages / otaClientEvents
     */
    Semaphore_Struct semStruct;
    Semaphore_Params otaSemParam;
    Semaphore_Params_init(&otaSemParam);
    otaSemParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&semStruct, 0, &otaSemParam);
    otaClientSemaphore = Semaphore_handle(&semStruct);

    otaServiceTaskId = OsalPort_registerTask(Task_self(), otaClientSemaphore, &otaServiceTaskEvents);

    //Initialize Stack
    otaClient_InitStack();

    otaClient_ProcessLoop();
}

/******************************************************************************
 *
 * @fn      otaClient_InitStack
 *
 * @brief   Call to register the task with zstack and start GUI and ota task
 *
 * @param
 *
 * @return  none
 */
static void otaClient_InitStack(void)
{
    //Register Endpoint
    zclOtaEpDesc.task_id = &otaServiceTaskId; //TODO

    zclport_registerEndpoint(otaServiceTaskId, &zclOtaEpDesc);
    otaClient_SetEndpoint(localOtaClientEp);

    // Register the application's attribute list
    zcl_registerAttrList( localOtaClientEp, localOtaClient_zclAttrsArraySize, localOtaClient_Attrs );
    zcl_registerCmdList( localOtaClientEp, localOtaClient_zclCmdsArraySize, localOtaClient_Cmds );

    otaClient_SetEndpoint(localOtaClientEp);
    otaClient_SetupCallbacks();

    CUI_clientParams_t clientParams;
    CUI_clientParamsInit(&clientParams);
    clientParams.maxStatusLines = 2;

    gOTAClientCuiHandle = CUI_clientOpen(&clientParams);

    otaClient_Init (otaClientSemaphore, otaServiceTaskId, gOTAClientCuiHandle);
}

/******************************************************************************
 *
 * @fn      otaClient_SetupCallbacks
 *
 * @brief   Registers necessary callbacks to zstack
 *
 * @param
 *
 * @return  none
 */
static void otaClient_SetupCallbacks(void)
{
    zstack_devZDOCBReq_t zdoCBReqOta = {0};

    zdoCBReqOta.has_devStateChange = true;
    zdoCBReqOta.devStateChange = true;

    zdoCBReqOta.has_matchDescRsp = true;
    zdoCBReqOta.matchDescRsp = true;

    zdoCBReqOta.has_ieeeAddrRsp = true;
    zdoCBReqOta.ieeeAddrRsp = true;

    (void)Zstackapi_DevZDOCBReq(otaServiceTaskId, &zdoCBReqOta);
}

/******************************************************************************
 *
 * @fn      otaClient_ProcessLoop
 *
 * @brief   Processing loop for ota
 *
 * @param
 *
 * @return  none
 */
static void otaClient_ProcessLoop(void)
{
    for(;;)
    {
        zstackmsg_genericReq_t *pMsg = NULL;
        bool msgProcessed = FALSE;
        if(Semaphore_pend(otaClientSemaphore, BIOS_WAIT_FOREVER))
        {
            if((pMsg = (zstackmsg_genericReq_t*) OsalPort_msgReceive(otaServiceTaskId)) != NULL)
            {
                otaClient_ProcessZstackMsgs(pMsg);

                // Free any separately allocated memory
                msgProcessed = Zstackapi_freeIndMsg(pMsg);
            }

            if((msgProcessed == FALSE) && (pMsg != NULL))
            {
                OsalPort_msgDeallocate((uint8_t*)pMsg);
            }

            otaClient_event_loop();
        }
    }
}

/******************************************************************************
 *
 * @fn      otaClient_ProcessZstackMsgs
 *
 * @brief   Processes recieved zstack messages
 *
 * @param  pMsg - Received message
 *
 * @return  none
 */
static void otaClient_ProcessZstackMsgs(zstackmsg_genericReq_t *pMsg)
{
    switch(pMsg->hdr.event)
    {
        case zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP:
        {
            zstackmsg_zdoMatchDescRspInd_t  *pInd =
                (zstackmsg_zdoMatchDescRspInd_t *) pMsg;

            ZDO_MatchDescRsp_t *pRsp;

            /*
            * Parse the Match Descriptor Response and give it
            * to the ZCL EZMode module to process
            */
            pRsp = (ZDO_MatchDescRsp_t *) OsalPort_msgAllocate(
                    sizeof(ZDO_MatchDescRsp_t) + pInd->rsp.n_matchList);
            if(pRsp)
            {
                pRsp->status = pInd->rsp.status;
                pRsp->nwkAddr = pInd->rsp.nwkAddrOfInterest;
                pRsp->cnt = pInd->rsp.n_matchList;
                OsalPort_memcpy(pRsp->epList, pInd->rsp.pMatchList, pInd->rsp.n_matchList);

                otaClient_ProcessMatchDescRsp ( pRsp );
                OsalPort_msgDeallocate((uint8_t *)pRsp);
            }
            break;
        }

        case zstackmsg_CmdIDs_AF_INCOMING_MSG_IND:
        {
            // Process incoming data messages
            zstackmsg_afIncomingMsgInd_t *pInd;
            pInd = (zstackmsg_afIncomingMsgInd_t *)pMsg;
            otaClient_ProcessAfIncomingMsg( &(pInd->req) );
        }
        break;

        default:
            break;
    }
}

/******************************************************************************
 *
 * @fn      otaClient_ProcessAfIncomingMsg
 *
 * @brief   Processes recieved data messages
 *
 * @param  pInMsg - Received message
 *
 * @return  none
 */
static void otaClient_ProcessAfIncomingMsg(zstack_afIncomingMsgInd_t *pInMsg)
{
    afIncomingMSGPacket_t afMsg;

    /*
    * All incoming messages are passed to the ZCL message processor,
    * first convert to a structure that ZCL can process.
    */
    afMsg.groupId = pInMsg->groupID;
    afMsg.clusterId = pInMsg->clusterId;
    afMsg.srcAddr.endPoint = pInMsg->srcAddr.endpoint;
    afMsg.srcAddr.panId = pInMsg->srcAddr.panID;
    afMsg.srcAddr.addrMode = (afAddrMode_t)pInMsg->srcAddr.addrMode;
    if( (afMsg.srcAddr.addrMode == afAddr16Bit)
        || (afMsg.srcAddr.addrMode == afAddrGroup)
        || (afMsg.srcAddr.addrMode == afAddrBroadcast) )
    {
        afMsg.srcAddr.addr.shortAddr = pInMsg->srcAddr.addr.shortAddr;
    }
    else if(afMsg.srcAddr.addrMode == afAddr64Bit)
    {
        OsalPort_memcpy(afMsg.srcAddr.addr.extAddr, &(pInMsg->srcAddr.addr.extAddr), 8);
    }
    afMsg.macDestAddr = pInMsg->macDestAddr;
    afMsg.endPoint = pInMsg->endpoint;
    afMsg.wasBroadcast = pInMsg->wasBroadcast;
    afMsg.LinkQuality = pInMsg->linkQuality;
    afMsg.correlation = pInMsg->correlation;
    afMsg.rssi = pInMsg->rssi;
    afMsg.SecurityUse = pInMsg->securityUse;
    afMsg.timestamp = pInMsg->timestamp;
    afMsg.nwkSeqNum = pInMsg->nwkSeqNum;
    afMsg.macSrcAddr = pInMsg->macSrcAddr;
    afMsg.radius = pInMsg->radius;
    afMsg.cmd.DataLength = pInMsg->n_payload;
    afMsg.cmd.Data = pInMsg->pPayload;

    zcl_ProcessMessageMSG(&afMsg);
}
#endif

/*********************************************************************
 * @fn      OTAClient_setAttributes
 *
 * @brief   Sets pointers to attributes used by OTA Client module
 *
 * @param   *attrs - set of attributes from the application
 * @param   numAttrs - number of attributes in the list
 *
 * @return  void
 */
void otaClient_setAttributes( const zclAttrRec_t *attrs, uint8_t numAttrs )
{
  uint8_t i;

  for ( i = 0; i < numAttrs; i++ )
  {
    if ( attrs[i].clusterID == ZCL_CLUSTER_ID_OTA )
    {
      switch(attrs[i].attr.attrId)
      {
        case ATTRID_OTA_UPGRADE_UPGRADE_SERVER_ID:
        {
          otaClient_UpgradeServerID = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_FILE_OFFSET:
        {
          otaClient_FileOffset = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_CURRENT_FILE_VERSION:
        {
          otaClient_CurrentFileVersion = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_DOWNLOADED_FILE_VERSION:
        {
          otaClient_DownloadedFileVersion = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_IMAGE_UPGRADE_STATUS:
        {
          otaClient_ImageUpgradeStatus = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_MANUFACTURER_ID:
        {
          otaClient_ManufacturerID = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_IMAGE_TYPE_ID:
        {
          otaClient_ImageType = attrs[i].attr.dataPtr;
        }
        break;
        case ATTRID_OTA_UPGRADE_MINIMUM_BLOCK_PERIOD:
        {
          otaClient_MinBlockReqDelay = attrs[i].attr.dataPtr;
        }
        break;
        default:
        break;
      }
    }
  }
}
#endif
