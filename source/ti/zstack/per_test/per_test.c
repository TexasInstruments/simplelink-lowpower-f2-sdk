/**************************************************************************************************
  Filename:       per_test.c
  Revised:        $Date: 2018-05-2 11:51:49 -0700 (Wed, 2 May 2018) $
  Revision:       $Revision: - $

  Description:    This file contains the implementation of the Packet Error Rate
	                test interface.


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

#ifdef PER_TEST
/*********************************************************************
 * INCLUDES
 */

#include "per_test.h"
#include "string.h"
#include "zstackmsg.h"
#include "rom_jt_154.h"
#include "util_timer.h"
#include "zstackapi.h"
#include <ti/sysbios/knl/Semaphore.h>


/*********************************************************************
 * MACROS
 */


#define PER_TEST_SEND1    1
#define PER_TEST_SEND2    2
#define PER_TEST_FWD1     3
#define PER_TEST_FWD2     4

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * CONSTANTS
 */

#define PER_TEST_CLUSTER         ZCL_CLUSTER_ID_GENERAL_ON_OFF
#define PER_TEST_ENDPOINT        8
#define PER_TEST_FC_BUFFER_SIZE  15
#define PER_TEST_INVALID_FC      0xFFFF

/*********************************************************************
 * GLOBAL VARIABLES
 */
#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
uint32_t  FramesQueued = 0;          //Frames queued
uint32_t  FramesFailedToSend = 0;  //Frames attempted to be send but failed   (invalid configuration of the frame send or no parent connected?)
uint32_t  FramesSuccess = 0;     //APS ACK Received
uint32_t  FramesNoAck = 0;        //Frames that did not receive APS ACK
#endif

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8_t perTest_fcIndex;
// initialize every entry to invalid FC value
static uint16_t perTest_fcBuffer[PER_TEST_FC_BUFFER_SIZE];

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING) || defined(PER_TEST_ENABLE_FWD_NOTIFICATION)
static uint8_t stAppID = 0xFF;
#endif

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
static Semaphore_Handle sem;
static uint16_t events = 0;

static ClockP_Handle PERTest_sendCmdClkHandle;
static ClockP_Struct PERTest_sendCmdClkStruct;

static zclOptionRec_t PERTestOptions[] =
{
  {
    PER_TEST_CLUSTER,
    ( AF_ACK_REQUEST ),
  },
};

static afAddrType_t perTest_DstAddr;
#endif

#ifdef  PER_TEST_ENABLE_FWD_NOTIFICATION
static uint32_t  CounterFramesQueued = 0;
static uint32_t  CounterFramesSuccess = 0;

static uint32_t  CounterFramesNoMacAck = 0;
static uint32_t  CounterFramesNoRoute = 0;
static uint32_t  CounterFramesTransExpired = 0;
#endif



#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
/* Only one screen */
//Frame send accepted by the nwk layer to be send
char FrameQueuedStr[] =    "QUEUED: ";
//Frame failed to Queue
char FrameFailedStr[] =    "FAILED: ";
//Frame for which we receive APS ACK
char FrameSuccessStr[] =   "SUCCESS: ";
//Frame for which we did not receive APS ACK
char FrameNoAckStr[] =    "NoAck: ";
#endif



#if defined(PER_TEST_ENABLE_FWD_NOTIFICATION)
/* First Screen */
//Frame received at nwk layer and queued to be send
char FrameQueuedStr[] =    "QUEUED: ";
//Frame MAC confirm success
char FrameSuccessStr[] =   "SUCCESS: ";
/* Second Screen */
//Nwk layer says no route to destination
char FrameNoRouteStr[] =   "NoRoute: ";
//No MAC ACK
char FrameNoAckStr[] =     "NoMAck: ";
//Frame expired at MAC layer
char FrameExpiredStr[] =   "Expired: ";
#endif

static CUI_clientHandle_t gPERCuiHandle;
static uint32_t gPerTestInfoLine;

/*********************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 */

/*********************************************************************
 * LOCAL FUNCTION DEFINITIONS
 */
static void PerTest_initializeClocks(void);

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)

static void PERTest_processPERTestSendCmdTimeoutCallback(UArg a0);
static void PERTest_sendCmd(void);
static void PERTest_dataConfirmAnalysis(zstack_afDataConfirmInd_t *req);
#endif

#ifdef PER_TEST_ENABLE_FWD_NOTIFICATION
static void PERTest_routingFramesAnalysis(zstack_nwkFrameFwdNotification_t *req);
#endif

static void PERTest_UpdateStatusLine(void);

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

void PERTest_init(Semaphore_Handle appSem, uint8_t stEnt, CUI_clientHandle_t gCuiHandle)
{
#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
  sem = appSem;
#endif

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING) || defined(PER_TEST_ENABLE_FWD_NOTIFICATION)
  stAppID = stEnt;
#endif
  PerTest_initializeClocks();

  uint8_t i;
  for ( i = 0; i < PER_TEST_FC_BUFFER_SIZE; i++)
  {
    perTest_fcBuffer[i] = PER_TEST_INVALID_FC;
  }

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
    zstack_sysConfigWriteReq_t zstack_sysConfigWriteReq;

    memset(&zstack_sysConfigWriteReq,0,sizeof(zstack_sysConfigWriteReq_t));

    //Set poll rate
    zstack_sysConfigWriteReq.has_pollRate = TRUE;
    zstack_sysConfigWriteReq.pollRate = POLL_RATE;
    zstack_sysConfigWriteReq.pollRateType = POLL_RATE_TYPE_DEFAULT;

    //Set the APS ACK wait time
    zstack_sysConfigWriteReq.has_apsAckWaitDuration = TRUE;
    zstack_sysConfigWriteReq.apsAckWaitDuration = PER_TEST_APS_ACKS_WAIT_TIME/2;   //(for some reason this is multiplied by 2, so divide it by 2)

    //Set APS retries retries
    zstack_sysConfigWriteReq.has_apsFrameRetries = TRUE;
    zstack_sysConfigWriteReq.apsFrameRetries = PER_TEST_APS_ACKS;

    Zstackapi_sysConfigWriteReq(stAppID, &zstack_sysConfigWriteReq);

    //Enable APS ACKs
    zcl_registerClusterOptionList(PER_TEST_ENDPOINT,1,PERTestOptions);
#endif


#ifdef PER_TEST_ENABLE_FWD_NOTIFICATION
  zstack_setNwkFrameFwdNotificationReq_t zstack_setNwkFrameFwdNotificationReq;
  zstack_setNwkFrameFwdNotificationReq.Enabled = TRUE;

  Zstackapi_SetNwkFrameFwdNotificationReq(stAppID,&zstack_setNwkFrameFwdNotificationReq);
#endif

#ifndef CUI_DISABLE
  gPERCuiHandle = gCuiHandle;
  CUI_statusLineResourceRequest(gCuiHandle, "PER Test", false, &gPerTestInfoLine);
#endif // CUI_DISABLE
}

void PERTest_process(void)
{
#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
  if ( events & SAMPLEAPP_PER_TEST_SEND_CMD_EVT )
  {
      PERTest_sendCmd();
      events &= ~SAMPLEAPP_PER_TEST_SEND_CMD_EVT;
  }
#endif
}

void PERTest_processZStackMsg(zstackmsg_genericReq_t *pMsg)
{

  static uint8_t isPERinitialized = 0;

  switch(pMsg->hdr.event)
  {

#ifdef PER_TEST_ENABLE_FWD_NOTIFICATION
    case zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION_IND:
    {
        zstackmsg_nwkFrameFwdNotification_t *pInd;
        pInd = (zstackmsg_nwkFrameFwdNotification_t*)pMsg;

        PERTest_routingFramesAnalysis(&(pInd->req));
        PERTest_UpdateStatusLine();
    }
    break;
#endif

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
    case zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND:
    {
        // This message is received as a confirmation of a data packet sent.
        // The status is of ZStatus_t type [defined in nl_mede.h]
        // The message fields are defined in af.h
        zstackmsg_afDataConfirmInd_t *pInd;
        pInd = (zstackmsg_afDataConfirmInd_t*)pMsg;

        PERTest_dataConfirmAnalysis(&(pInd->req));
        PERTest_UpdateStatusLine();
    }
    break;

#ifdef PER_TEST_SEND_USE_FINDING_BINDING
    case zstackmsg_CmdIDs_BDB_BIND_NOTIFICATION_CB:
    {
      //If we have created a bind with the light, then start configuration for PER test on zed
      UtilTimer_setTimeout( PERTest_sendCmdClkHandle, SAMPLEAPP_PER_TEST_SEND_CMD_DELAY );
      UtilTimer_start(&PERTest_sendCmdClkStruct);
      isPERinitialized = TRUE;
    }
    break;
#endif

    case zstackmsg_CmdIDs_BDB_NOTIFICATION:
    {
      if(!isPERinitialized)
      {
        zstackmsg_bdbNotificationInd_t *pInd;
        pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;

        switch(pInd->Req.bdbCommissioningMode)
        {
#ifndef PER_TEST_SEND_USE_FINDING_BINDING
          case BDB_COMMISSIONING_NWK_STEERING:
            if(pInd->Req.bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
            {
                //If we are on a network, then start configuration for PER test on zed
                UtilTimer_setTimeout( PERTest_sendCmdClkHandle, SAMPLEAPP_PER_TEST_SEND_CMD_DELAY );
                UtilTimer_start(&PERTest_sendCmdClkStruct);
                isPERinitialized = TRUE;
            }
          break;
#endif
          case BDB_COMMISSIONING_INITIALIZATION:
            if(pInd->Req.bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
            {
              //If we are on a network, then start configuration for PER test on zed
              UtilTimer_setTimeout( PERTest_sendCmdClkHandle, SAMPLEAPP_PER_TEST_SEND_CMD_DELAY );
              UtilTimer_start(&PERTest_sendCmdClkStruct);
              isPERinitialized = TRUE;
            }
          break;
          case BDB_COMMISSIONING_PARENT_LOST:
            if(pInd->Req.bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
            {
              //If we are on a network, then start configuration for PER test on zed
              UtilTimer_setTimeout( PERTest_sendCmdClkHandle, SAMPLEAPP_PER_TEST_SEND_CMD_DELAY );
              UtilTimer_start(&PERTest_sendCmdClkStruct);
              isPERinitialized = TRUE;
            }
          break;
        }
      }
    break;
    }
#endif  //defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
  }
}


/*********************************************************************
 * LOCAL FUNCTIONS
 */

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
static void PERTest_dataConfirmAnalysis(zstack_afDataConfirmInd_t *req)
{
  uint8_t i = 0;
  // find the matching ZCL Transaction Sequence Number
  for( i = (perTest_fcIndex - 1); i != perTest_fcIndex; i = ((i - 1) % PER_TEST_FC_BUFFER_SIZE) )
  {
    if(perTest_fcBuffer[i] == req->transID)
    {
      perTest_fcBuffer[i] = PER_TEST_INVALID_FC;
      switch(req->status)
      {
          case ZSuccess:
            FramesSuccess++;
          break;
          case ZApsNoAck:
            FramesNoAck++;
          break;
          default:
          break;
      }
      break;
    }
  }
}
#endif

#ifdef PER_TEST_ENABLE_FWD_NOTIFICATION
static void PERTest_routingFramesAnalysis(zstack_nwkFrameFwdNotification_t *req)
{
  // do not track broadcasts in statistics
  if( req->nwkFrameFwdNotification.dstAddr == 0xFFFF ||
      req->nwkFrameFwdNotification.dstAddr == 0xFFFD ||
      req->nwkFrameFwdNotification.dstAddr == 0xFFFC )
  {
    return;
  }

  switch(req->nwkFrameFwdNotification.frameState)
  {
      case NWK_FRAME_FWD_MSG_QUEUED:
          //Message has been received and it has been queued
          CounterFramesQueued++;
      break;

      case NWK_FRAME_FWD_MSG_SENT:
          switch (req->nwkFrameFwdNotification.status)
          {
              case ZMacSuccess:
                  //Frame being forwarded correctly
                  CounterFramesSuccess++;
              break;

              case ZMacNoACK:
                  CounterFramesNoMacAck++;
              break;

              case ZNwkNoRoute:
                  CounterFramesNoRoute++;
              break;

              case ZMacTransactionExpired:
                  CounterFramesTransExpired++;
              break;

              default:
                  //does not seems to be any other valid value of interest, if needed it can be added...
              break;
          }
      break;


      default:
          //there is no other possible value
      break;
  }
}
#endif



/*******************************************************************************
 * @fn      PerTest_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void PerTest_initializeClocks(void)
{

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
    // Initialize the timers needed for this application
    PERTest_sendCmdClkHandle = UtilTimer_construct(
    &PERTest_sendCmdClkStruct,
    PERTest_processPERTestSendCmdTimeoutCallback,
    SAMPLEAPP_PER_TEST_SEND_CMD_DELAY,
    0, false, 0);
#endif

}

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
/*******************************************************************************
 * @fn      PERTest_processPERTestSendCmdTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void PERTest_processPERTestSendCmdTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    events |= SAMPLEAPP_PER_TEST_SEND_CMD_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(sem);
}



/*******************************************************************************
 * @fn      PERTest_sendCmd
 *
 * @brief   Send the command to the device intended, parent or using F&B. See PER_TEST_SEND_TO_PARENT and PER_TEST_SEND_USE_FINDING_BINDING in per_test.h
 *
 * @param   none
 *
 * @return  none
 */
static void PERTest_sendCmd(void)
{

#if defined(PER_TEST_SEND_TO_PARENT)
    perTest_DstAddr.addrMode = afAddr16Bit;
    perTest_DstAddr.addr.shortAddr = _NIB.nwkCoordAddress;
    perTest_DstAddr.endPoint = PER_TEST_ENDPOINT;
#endif
#if defined(PER_TEST_SEND_USE_FINDING_BINDING)
    // Set destination address to indirect
    perTest_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
    perTest_DstAddr.endPoint = 0;
    perTest_DstAddr.addr.shortAddr = 0;
#endif
    zstack_getZCLFrameCounterRsp_t Rsp;

    Zstackapi_getZCLFrameCounterReq(stAppID, &Rsp);

    // save the ZCL Transaction Sequence Number for comparison to
    // AF Data Confirm Indication from stack
    perTest_fcBuffer[perTest_fcIndex] = Rsp.zclFrameCounter;
    perTest_fcIndex = ((perTest_fcIndex + 1) % PER_TEST_FC_BUFFER_SIZE);

#ifdef ZCL_ON_OFF
    if(zclGeneral_SendOnOff_CmdToggle( PER_TEST_ENDPOINT, &perTest_DstAddr, TRUE, Rsp.zclFrameCounter ) == ZSuccess)
    {
        FramesQueued++;
    }
    else
    {
        FramesFailedToSend++;
    }
#endif
    //Keep sending the commands
    UtilTimer_setTimeout( PERTest_sendCmdClkHandle, SAMPLEAPP_PER_TEST_SEND_CMD_DELAY );
    UtilTimer_start(&PERTest_sendCmdClkStruct);
}
#endif




/*******************************************************************************
 * @fn      PERTest_UpdateStatusLine
 *
 * @brief   Update the PER status lines in the UI
 *
 * @param
 *
 * @return  none
 */
static void PERTest_UpdateStatusLine(void)
{

    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

#if defined(PER_TEST_SEND_TO_PARENT) || defined(PER_TEST_SEND_USE_FINDING_BINDING)
    strcat(lineFormat, FrameQueuedStr);
    strcat(lineFormat, "%d || ");
    strcat(lineFormat, FrameFailedStr);
    strcat(lineFormat, "%d || ");
    strcat(lineFormat, FrameSuccessStr);
    strcat(lineFormat, "%d || ");
    strcat(lineFormat, FrameNoAckStr);
    strcat(lineFormat, "%d || ");

    CUI_statusLinePrintf(gPERCuiHandle, gPerTestInfoLine, lineFormat, FramesQueued, FramesFailedToSend, FramesSuccess, FramesNoAck);
#endif
#if defined(PER_TEST_ENABLE_FWD_NOTIFICATION)
    strcat(lineFormat, FrameQueuedStr);
    strcat(lineFormat, "%d || ");
    strcat(lineFormat, FrameSuccessStr);
    strcat(lineFormat, "%d || ");
    strcat(lineFormat, FrameNoRouteStr);
    strcat(lineFormat, "%d || ");
    strcat(lineFormat, FrameNoAckStr);
    strcat(lineFormat, "%d || ");
    strcat(lineFormat, FrameExpiredStr);
    strcat(lineFormat, "%d || ");

    CUI_statusLinePrintf(gPERCuiHandle, gPerTestInfoLine, lineFormat, CounterFramesQueued, CounterFramesSuccess, CounterFramesNoMacAck, CounterFramesNoRoute, CounterFramesTransExpired);
#endif
}

#endif /* PER_TEST */
