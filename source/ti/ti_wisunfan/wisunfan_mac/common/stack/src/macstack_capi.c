/******************************************************************************

 @file  macstack_capi.c

 @brief MAC stack C interface implementation on top of
        dispatcher messaging interface.

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2013-2023, Texas Instruments Incorporated

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

 ******************************************************************************
 
 
 *****************************************************************************/

#include "Macs.h"
#include "MacStackMSG.h"
#include <string.h>
#include <ICall.h>

extern ICall_EntityID macAppEntity;

/*********************************************************************
 * LOCAL FUNCTIONS
 */


/*********************************************************************
 * Compare a received TIMAC Reset Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacResetCS(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacResetReqCmd *pMsg = (TimacMSG_MacResetReqCmd *)msg;

  return ((pMsg->hdr.event == MAC_RESET_REQ)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Set Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacSetCS(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacSetReqCmd *pMsg = (TimacMSG_MacSetReqCmd *)msg;

  return ((pMsg->hdr.event == MAC_SET_REQ)? true : false);
}

/*********************************************************************
 * Compare a received TIMAC FH Enable Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacFHEnableCS(ICall_ServiceEnum src, ICall_EntityID dest,
                                 const void *msg)
{
  TimacMSG_MacCommonCmd *pMsg = (TimacMSG_MacCommonCmd *)msg;

  return ((pMsg->hdr.event == MAC_ENABLE_FH)? true : false);
}

/*********************************************************************
 * Compare a received TIMAC FH Set Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacFHSetCS(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacFHSetReqCmd *pMsg = (TimacMSG_MacFHSetReqCmd *)msg;

  return ((pMsg->hdr.event == MAC_FH_SET_REQ)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Set Security Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacSetSecurityCS(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacSetReqCmd *pMsg = (TimacMSG_MacSetReqCmd *)msg;

  return ((pMsg->hdr.event == MAC_SET_SECURITY_REQ)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Get Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacGetCS(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacGetReqCmd *pMsg = (TimacMSG_MacGetReqCmd *)msg;

  return ((pMsg->hdr.event == MAC_GET_REQ)? true : false);
}

/*********************************************************************
 * Compare a received TIMAC FH Get Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacFHGetCS(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacFHGetReqCmd *pMsg = (TimacMSG_MacFHGetReqCmd *)msg;

  return ((pMsg->hdr.event == MAC_FH_GET_REQ)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Get Security Ptr Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacGetPtrSec(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacGetReqCmd *pMsg = (TimacMSG_MacGetReqCmd *)msg;

  return ((pMsg->hdr.event == MAC_GET_SECURITY_PTR_REQ)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Get Security Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacGetSecurityCS(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacGetReqCmd *pMsg = (TimacMSG_MacGetReqCmd *)msg;

  return ((pMsg->hdr.event == MAC_GET_SECURITY_REQ)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Rand Byte Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacRandByte(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacRandByteCmd *pMsg = (TimacMSG_MacRandByteCmd *)msg;

  return ((pMsg->hdr.event == MAC_RANDOM_BYTE)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Src Match Add Entry Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacSrcMatchAddEntry(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacSrcMatchEntry_t *pMsg = (TimacMSG_MacSrcMatchEntry_t *)msg;

  return ((pMsg->hdr.event == MAC_SRC_MATCH_ADD_ENTRY)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Src Match Delete Entry Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacSrcMatchDeleteEntry(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacSrcMatchEntry_t *pMsg = (TimacMSG_MacSrcMatchEntry_t *)msg;

  return ((pMsg->hdr.event == MAC_SRC_MATCH_DELETE_ENTRY)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC Src Match Check All Pending message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacSrcMatchCheckAllPending(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacSrcMatchCheckAllPending_t *pMsg = (TimacMSG_MacSrcMatchCheckAllPending_t *)msg;

  return ((pMsg->hdr.event == MAC_SRC_MATCH_CHECK_ALL_PENDING)? true : false);
}


/*********************************************************************
 * Compare a received TIMAC MCPS Data Alloc Command Status message for a match.
 *
 * @param src   originator of the message as a service enumeration
 * @param dest  destination entity id of the message
 * @param msg   pointer to the message body
 *
 * @return TRUE when the message matches. FALSE, otherwise.
 */
static bool matchTimacMcpsDataAlloc(ICall_ServiceEnum src, ICall_EntityID dest,
                              const void *msg)
{
  TimacMSG_MacMcpsDataAlloc_t *pMsg = (TimacMSG_MacMcpsDataAlloc_t *)msg;

  return ((pMsg->hdr.event == MAC_MCPS_DATA_ALLOC)? true : false);
}



/* Initializes the MAC Stack parameters
 * over messaging interface using dispatcher.
 */
void MAC_StackInitParamsMsg( void )
{
  /* Allocate message buffer space */
  TimacMSG_MacStackInitParamsCmd *msg = (TimacMSG_MacStackInitParamsCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_STACK_INIT_PARAMS;
    msg->hdr.status = 0;
    msg->retransmit = 0;
    msg->pendingMsg = 0;
    msg->pMacCbackQueryRetransmit = MAC_CbackQueryRetransmit;
    msg->pMacCbackCheckPending    = MAC_CbackCheckPending;

    /* Send the message to ICALL_SERVICE_CLASS_TIMAC. */
    ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                         ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);
  }
}


/* Implementation of MAC_InitDevice() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_InitDevice( void )
{
  /* Allocate message buffer space */
  TimacMSG_MacCommonCmd *msg = (TimacMSG_MacCommonCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_INIT_DEVICE;
    msg->hdr.status = 0;

    /* Send the message to ICALL_SERVICE_CLASS_TIMAC. */
    ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                         ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);
  }
}


/* Implementation of MAC_InitCoord() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_InitCoord( void )
{
  /* Allocate message buffer space */
  TimacMSG_MacCommonCmd *msg = (TimacMSG_MacCommonCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_INIT_COORD;
    msg->hdr.status = 0;

    /* Send the message to ICALL_SERVICE_CLASS_TIMAC. */
    ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                         ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);
  }
}


/* Implementation of MAC_InitBeaconDevice() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_InitBeaconDevice( void )
{
  /* Allocate message buffer space */
  TimacMSG_MacCommonCmd *msg = (TimacMSG_MacCommonCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_INIT_BEACON_DEVICE;
    msg->hdr.status = 0;

    /* Send the message to ICALL_SERVICE_CLASS_TIMAC. */
    ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                         ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);
  }
}


/* Implementation of MAC_InitBeaconCoord() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_InitBeaconCoord( void )
{
  /* Allocate message buffer space */
  TimacMSG_MacCommonCmd *msg = (TimacMSG_MacCommonCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_INIT_BEACON_COORD;
    msg->hdr.status = 0;

    /* Send the message to ICALL_SERVICE_CLASS_TIMAC. */
    ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                         ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);
  }
}


/* Implementation of MAC_InitFH() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_InitFH( void )
{
  /* Allocate message buffer space */
  TimacMSG_MacCommonCmd *msg = (TimacMSG_MacCommonCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_INIT_FH;
    msg->hdr.status = 0;

    /* Send the message to ICALL_SERVICE_CLASS_TIMAC. */
    ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                         ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);
  }
}

/* Implementation of MAC_StartFH() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_StartFH( void )
{
  /* Allocate message buffer space */
  TimacMSG_MacCommonCmd *msg = (TimacMSG_MacCommonCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_START_FH;
    msg->hdr.status = 0;

    /* Send the message to ICALL_SERVICE_CLASS_TIMAC. */
    ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                         ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);
  }
}

/* Implementation of MAC_EnableFH() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_EnableFH( void )
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacCommonCmd *msg = (TimacMSG_MacCommonCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_ENABLE_FH;
    msg->hdr.status = 0;

    /* Send the message to ICALL_SERVICE_CLASS_TIMAC. */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacCommonCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacFHEnableCS,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }
  return status;
}

/* Implementation of MAC_MlmeResetReq() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_MlmeResetReq( bool setDefaultPib )
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacResetReqCmd *msg = (TimacMSG_MacResetReqCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_RESET_REQ;
    msg->hdr.status = 0;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacResetReqCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacResetCS,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->status;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return (status);
}


/* Implementation of MAC_MlmeScanReq() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmeScanReqMsg(Timac_MacMlmeScanReq_t *pData)
{
   /* Allocate message buffer space */
   TimacMSG_ScanReqCmd *msg = (TimacMSG_ScanReqCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event = MAC_MLME_SCAN_REQ;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmeScanReq, pData, sizeof(Timac_MacMlmeScanReq_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of MAC_MlmeStartReq() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmeStartReqMsg(Timac_MacMlmeStartReq_t *pData)
{
  /* Allocate message buffer space */
   TimacMSG_StartReqCmd *msg = (TimacMSG_StartReqCmd *)
      ICall_allocMsg(sizeof(*msg) + pData->mpmparams.numIEs);

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event = MAC_START_REQ;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmeStartReq, pData, sizeof(Timac_MacMlmeStartReq_t));
     msg->macMlmeStartReq.mpmparams.pIEIDs = NULL;
     if ( pData->mpmparams.numIEs > 0 )
     {
       if (NULL != pData->mpmparams.pIEIDs)
       {
         msg->macMlmeStartReq.mpmparams.pIEIDs = (uint8*)(msg + 1);
         memcpy(msg->macMlmeStartReq.mpmparams.pIEIDs,
                pData->mpmparams.pIEIDs, pData->mpmparams.numIEs);
       }
     }
     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of MAC_MlmeAssociateReq() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmeAssociateReqMsg(Timac_MacMlmeAssociateReq_t *pData)
{
  /* Allocate message buffer space */
   TimacMSG_AssociateReqCmd *msg = (TimacMSG_AssociateReqCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event =  MAC_MLME_ASSOCIATE_REQ;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmeAssociateReq, pData, sizeof(Timac_MacMlmeAssociateReq_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of MAC_MlmeDisassociateReq() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmeDisassociateReqMsg(Timac_MacMlmeDisassociateReq_t *pData)
{
  /* Allocate message buffer space */
   TimacMSG_DisassociateReqCmd *msg = (TimacMSG_DisassociateReqCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event =  MAC_MLME_DISASSOCIATE_REQ;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmeDisassociateReq, pData, sizeof(Timac_MacMlmeDisassociateReq_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of MAC_MlmeOrphanRsp() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmeOrphanRspMsg(Timac_MacMlmeOrphanRsp_t *pData)
{
  /* Allocate message buffer space */
   TimacMSG_OrphanRspCmd *msg = (TimacMSG_OrphanRspCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event =  MAC_MLME_ORPHAN_RSP;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmeOrphanRsp, pData, sizeof(Timac_MacMlmeOrphanRsp_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of MAC_MlmeAssociateRsp() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmeAssociateRspMsg(Timac_MacMlmeAssociateRsp_t *pData)
{
  /* Allocate message buffer space */
   TimacMSG_AssociateRspCmd *msg = (TimacMSG_AssociateRspCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event =  MAC_MLME_ASSOCIATE_RSP;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmeAssociateRsp, pData, sizeof(Timac_MacMlmeAssociateRsp_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of MAC_MlmePollReq() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmePollReqMsg(Timac_MacMlmePollReq_t *pData)
{
  /* Allocate message buffer space */
   TimacMSG_PollReqCmd *msg = (TimacMSG_PollReqCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event =  MAC_MLME_POLL_REQ;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmePollReq, pData, sizeof(Timac_MacMlmePollReq_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of MAC_McpsPurgeReqMsg() function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_McpsPurgeReqMsg(Timac_MacMcpsPurgeReq_t *pData)
{
  /* Allocate message buffer space */
   TimacMSG_PurgeReqCmd *msg = (TimacMSG_PurgeReqCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event =  MAC_MCPS_PURGE_REQ;
     msg->hdr.status = 0;
     memcpy(&msg->macMcpsPurgeReq, pData, sizeof(Timac_MacMcpsPurgeReq_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}

/* Implementation of MAC_MlmeWSAsyncReqMsg function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmeWSAsyncReqMsg(Timac_MacMlmeWSAsyncReq_t *pData)
{
  /* Allocate message buffer space */
   TimacMSG_WSAsyncReqCmd *msg = (TimacMSG_WSAsyncReqCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event = MAC_MLME_WS_ASYNC_REQ;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmeWSAsyncReq, pData, sizeof(Timac_MacMlmeWSAsyncReq_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of sAddrExtCpy function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void *sAddrExtCpy(uint_least8_t * pDest, const uint_least8_t * pSrc)
{
  return memcpy(pDest, pSrc, SADDR_EXT_LEN);
}


/* Implementation of MAC_MlmeSetReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_MlmeSetReq(uint_least8_t pibAttribute, void *pValue)
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacSetReqCmd *msg = (TimacMSG_MacSetReqCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_SET_REQ;
    msg->hdr.status = 0;
    msg->paramID = pibAttribute;
    msg->paramValue = pValue;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacSetReqCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacSetCS,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return (status);
}


/* Implementation of MAC_MlmeSetSecurityReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_MlmeSetSecurityReq(uint_least8_t pibAttribute, void *pValue)
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacSetReqCmd *msg = (TimacMSG_MacSetReqCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_SET_SECURITY_REQ;
    msg->hdr.status = 0;
    msg->paramID = pibAttribute;
    msg->paramValue = pValue;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacSetReqCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacSetSecurityCS,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return (status);
}


/* Implementation of  MAC_MlmeGetPointerSecurityReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_MlmeGetPointerSecurityReq(uint8 pibAttribute, void **pValue)
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacSecurityGetPtrReqCmd *msg = (TimacMSG_MacSecurityGetPtrReqCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_GET_SECURITY_PTR_REQ;
    msg->hdr.status = 0;
    msg->paramID = pibAttribute;
    msg->pValue = pValue;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacSecurityGetPtrReqCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacGetPtrSec,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }
  return (status);
}


/* Implementation of MAC_MlmeGetReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_MlmeGetReq(uint8 pibAttribute, void *pValue)
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacGetReqCmd *msg = (TimacMSG_MacGetReqCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_GET_REQ;
    msg->hdr.status = 0;
    msg->paramID = pibAttribute;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacSetReqCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacGetCS,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
        memcpy(pValue, msg->pValue, msg->len);
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_free(msg->pValue);
    ICall_freeMsg(msg);
  }
  return (status);
}


/* Implementation of MAC_MlmeGetSecurityReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_MlmeGetSecurityReq(uint8 pibAttribute, void *pValue)
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacGetReqCmd *msg = (TimacMSG_MacGetReqCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_GET_SECURITY_REQ;
    msg->hdr.status = 0;
    msg->paramID = pibAttribute;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacSetReqCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacGetSecurityCS,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
        memcpy(pValue, msg->pValue, msg->len);
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_free(msg->pValue);
    ICall_freeMsg(msg);
  }
  return (status);
}

/* Implementation of MAC_MlmeFHSetReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_MlmeFHSetReq(uint16 pibAttribute, void *pValue)
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacFHSetReqCmd *msg = (TimacMSG_MacFHSetReqCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_FH_SET_REQ;
    msg->hdr.status = 0;
    msg->paramID = pibAttribute;
    msg->paramValue = pValue;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacFHSetReqCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacFHSetCS,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return (status);
}

/* Implementation of MAC_MlmeFHGetReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_MlmeFHGetReq(uint16 pibAttribute, void *pValue)
{
  uint8       status = MAC_NO_RESOURCES;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacFHGetReqCmd *msg = (TimacMSG_MacFHGetReqCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_FH_GET_REQ;
    msg->hdr.status = 0;
    msg->paramID = pibAttribute;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacFHGetReqCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacFHGetCS,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
        memcpy(pValue, msg->pValue, msg->len);
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_free(msg->pValue);
    ICall_freeMsg(msg);
  }
  return (status);
}

/* Implementation of MAC_RandomByte function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_RandomByte(void)
{
  uint8  randByte;
  ICall_Errno errno;

  /* Allocate message buffer space */
  TimacMSG_MacRandByteCmd *msg = (TimacMSG_MacRandByteCmd *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_RANDOM_BYTE;
    msg->hdr.status = 0;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacRandByteCmd *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacRandByte,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        randByte = pCmdStatus->randByte;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return (randByte);
}



/* Implementation of MAC_MlmeSyncReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MlmeSyncReqMsg(Timac_MacMlmeSyncReq_t *pData)
{
   /* Allocate message buffer space */
   TimacMSG_SyncReqCmd *msg = (TimacMSG_SyncReqCmd *)
      ICall_allocMsg(sizeof(*msg));

   if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event = MAC_SYNC_REQ;
     msg->hdr.status = 0;
     memcpy(&msg->macMlmeSyncReq, pData, sizeof(Timac_MacMlmeSyncReq_t));

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}

/* Implementation of MAC_SrcMatchEnable function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_SrcMatchEnable(void)
{
  TimacMSG_HDR *msg = (TimacMSG_HDR *)
      ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
   {
     /* Fill in the message content */
     msg->event = MAC_SRC_MATCH_ENABLE;
     msg->status = 0;

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}

/* Implementation of MAC_SrcMatchAddEntry function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_SrcMatchAddEntry ( sAddr_t *addr, uint16 panID )
{
  ICall_Errno errno;
  uint8 status;
  TimacMSG_MacSrcMatchEntry_t *msg = (TimacMSG_MacSrcMatchEntry_t *)
    ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_SRC_MATCH_ADD_ENTRY;
    msg->hdr.status = 0;
    msg->addr = (Timac_sAddr_t *)addr;
    msg->panID = panID;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacSrcMatchEntry_t *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacSrcMatchAddEntry,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return status;
}


/* Implementation of MAC_SrcMatchDeleteEntry function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_SrcMatchDeleteEntry ( sAddr_t *addr, uint16 panID )
{
  ICall_Errno errno;
  uint8 status;
  TimacMSG_MacSrcMatchEntry_t *msg = (TimacMSG_MacSrcMatchEntry_t *)
    ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_SRC_MATCH_DELETE_ENTRY;
    msg->hdr.status = 0;
    msg->addr = (Timac_sAddr_t *)addr;
    msg->panID = panID;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacSrcMatchEntry_t *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacSrcMatchDeleteEntry,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        status = pCmdStatus->hdr.status;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return status;
}


/* Implementation of MAC_SrcMatchAckAllPending function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_SrcMatchAckAllPending ( uint8 option  )
{
  TimacMSG_MacSrcMatchAckAllPending_t *msg = (TimacMSG_MacSrcMatchAckAllPending_t *)
    ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
   {
     /* Fill in the message content */
     msg->hdr.event = MAC_SRC_MATCH_ACK_ALL_PENDING;
     msg->hdr.status = 0;
     msg->option = option;

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, msg);
   }
}


/* Implementation of MAC_SrcMatchCheckAllPending function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
uint8 MAC_SrcMatchCheckAllPending ( void )
{
  ICall_Errno errno;
  uint_least8_t pend;
  TimacMSG_MacSrcMatchCheckAllPending_t *msg = (TimacMSG_MacSrcMatchCheckAllPending_t *)
    ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_SRC_MATCH_CHECK_ALL_PENDING;
    msg->hdr.status = 0;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacSrcMatchCheckAllPending_t *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacSrcMatchCheckAllPending,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        pend = pCmdStatus->pend;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return pend;
}

/* Implementation of MAC_McpsDataAlloc function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
Timac_MacMcpsDataReq_t *MAC_McpsDataAllocMsg(uint_least16_t len,
                                             uint_least8_t securityLevel,
                                             uint_least8_t keyIdMode,
                                             uint_least32_t includeFhIEs,
                                             uint_least16_t payloadIeLen )
{
  ICall_Errno errno;
  Timac_MacMcpsDataReq_t *pDataReq = NULL;
  TimacMSG_MacMcpsDataAlloc_t *msg = (TimacMSG_MacMcpsDataAlloc_t *)
    ICall_allocMsg(sizeof(*msg));

  if (msg != NULL)
  {
    /* Fill in the message content */
    msg->hdr.event = MAC_MCPS_DATA_ALLOC;
    msg->hdr.status = 0;
    msg->len = len;
    msg->securityLevel = securityLevel;
    msg->keyIdMode = keyIdMode;
    msg->includeFhIEs = includeFhIEs;
    msg->payloadIeLen = payloadIeLen;

    /* Send the message */
    errno = ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                                 ICALL_MSG_FORMAT_KEEP, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      TimacMSG_MacMcpsDataAlloc_t *pCmdStatus = NULL;

      errno = ICall_waitMatch(ICALL_TIMEOUT_FOREVER, matchTimacMcpsDataAlloc,
                              NULL, NULL, (void **)&pCmdStatus);

      if (errno  == ICALL_ERRNO_SUCCESS)
      {
        pDataReq = pCmdStatus->pDataReq;
      }
    }

    /* pCmdStatus is the same as msg */
    ICall_freeMsg(msg);
  }

  return pDataReq;
}


/* Implementation of MAC_McpsDataReq function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_McpsDataReqMsg(Timac_MacMcpsDataReq_t *pData)
{
   if (pData != NULL)
   {
     /* Fill in the message content */
     pData->hdr.event = MAC_MCPS_DATA_REQ;
     pData->hdr.status = 0;

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, pData);
   }
}


/* Implementation of mac_msg_deallocate function
 * over messaging interface using dispatcher.
 * See function API header file to find the description
 * of the function interface.
 */
void MAC_MacMsgDeallocate(TimacMSG_HDR *pData)
{
  if (pData != NULL)
   {
     /* Fill in the message content */
     pData->event = MAC_MSG_DEALLOCATE;
     pData->status = 0;

     /* Send the message */
     ICall_sendServiceMsg(macAppEntity, ICALL_SERVICE_CLASS_TIMAC,
                          ICALL_MSG_FORMAT_KEEP, pData);
   }
}
