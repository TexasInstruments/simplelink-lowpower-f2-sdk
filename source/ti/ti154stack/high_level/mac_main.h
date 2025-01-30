/******************************************************************************

 @file  mac_main.h

 @brief Internal interface file for the MAC.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2005-2025, Texas Instruments Incorporated

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

#ifndef MAC_MAIN_H
#define MAC_MAIN_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* State machine states */
enum
{
  MAC_IDLE_ST,          /* Idle state */
  MAC_ASSOCIATING_ST,   /* MAC device is performing the association procedure */
  MAC_POLLING_ST,       /* MAC device is polling coordinator for data */
  MAC_SCANNING_ST,      /* MAC device or coordinator is performing a scan procedure */
  MAC_STARTING_ST       /* MAC coordinator is starting a network */
};

/* State machine events, triggers MAC API and state transistion depending on state machine state.
 * Note that many events require a specific MAC state for their intended action to take place. The
 * comments below only outline their intended action. See hmac_global.c for the full state machine
 * action/transition table. 
 */
enum
{
  /* Triggered by corresponding MAC API call */
  MAC_API_DATA_REQ_EVT,         /* Triggers macApiDataReq, builds and sends a MAC data frame */
  MAC_API_PURGE_REQ_EVT,        /* Triggers macApiPurgeReq, request to purge data request from MAC data queue */
  MAC_API_ASSOCIATE_REQ_EVT,    /* Triggers macApiAssociateReq, builds and sends a MAC association request frame */
  MAC_API_ASSOCIATE_RSP_EVT,    /* Triggers macApiAssociateRsp, builds and sends a MAC association response frame */
  MAC_API_DISASSOCIATE_REQ_EVT, /* Triggers macApiDisassociateReq, builds and sends a MAC disassociation request frame */
  MAC_API_ORPHAN_RSP_EVT,       /* Triggers macApiOrphanRsp, builds and sends a MAC orphan notification response frame */
  MAC_API_POLL_REQ_EVT,         /* Triggers macApiPollReq, builds and sends a data (poll) request frame */
  MAC_API_SCAN_REQ_EVT,         /* Triggers macApiScanReq, initiates MAC ED, active, passive, or orphan
                                   scan process (when MAC is in idle state) */
  MAC_API_START_REQ_EVT,        /* Triggers macApiStartReq, starts the network on a coordinator device */
  MAC_API_SYNC_REQ_EVT,         /* Triggers macApiSyncReq, builds and sends a sync request frame on a beacon device */
  MAC_API_WS_ASYNC_REQ_EVT,     /* Triggers macApiWSAsyncReq, builds and sends an FH asynchronous request */
  MAC_API_PWR_ON_REQ_EVT,       /* Triggers macApiPwrOnReq, currently unused */

  /* Triggered by MAC frame RX */
  MAC_RX_ASSOC_REQ_EVT,         /* Triggers macRxAssociateReq, handles association request RX */
  MAC_RX_ASSOC_RSP_EVT,         /* Triggers macRxAssocRsp or macPollRxAssocRsp (if in poll state),
                                   handles association response RX */
  MAC_RX_DISASSOC_EVT,          /* Triggers macAssocRxDisassoc or macPollRxDisassoc (if in poll state),
                                   handles disassocation request RX */
  MAC_RX_DATA_REQ_EVT,          /* Triggers macRxDataReq, handles data request RX */
  MAC_RX_PAN_CONFLICT_EVT,      /* Triggers macRxPanConflict, handles PAN ID conflict notification by sending sync
                                   loss indication */
  MAC_RX_ORPHAN_EVT,            /* Triggers macRxOrphan, handles orphan notification command frame RX */
  MAC_RX_BEACON_REQ_EVT,        /* Triggers macRxBeaconReq, handles beacon request RX by sending beacon frame */
  MAC_RX_REALIGN_EVT,           /* Triggers macRxCoordRealign or macScanRxCoordRealign (scan state),
                                   handles coord realignment frame RX */
  MAC_RX_GTS_REQ_EVT,           /* No action or state transition change on GTS req RX */
  MAC_RX_BEACON_EVT,            /* Triggers macRxBeacon or macScanRxBeacon (scan state), handles beacon frame RX */
  MAC_RX_DATA_IND_EVT,          /* Triggers macDataRxInd or macPollDataRxInd (poll state), handles data frame RX */

  /* Triggered by MAC frame TX success or failure */
  MAC_TX_ASSOC_REQ_OK_EVT,      /* On association req TX success, triggers start response timer
                                   (macStartResponseTimer) */
  MAC_TX_ASSOC_REQ_FAIL_EVT,    /* On association req TX fail, triggers failure handling (macAssocFailed)  */
  MAC_TX_ASSOC_RSP_EVT,         /* On association rsp TX success, triggers comm status indication to app
                                   (macCommStatusInd) */
  MAC_TX_DISASSOC_EVT,          /* On disassocation notification TX success, triggers disassociation confirm
                                   (macDisassocComplete) */
  MAC_TX_ORPHAN_OK_EVT,         /* On orphan notification TX success, if in scanning state, setup scan timer
                                   (macScanStartTimer) */
  MAC_TX_ORPHAN_FAIL_EVT,       /* On orphan notification TX fail, if in scanning state, move on to next channel
                                   to scan (macScanNextChan) */
  MAC_TX_BEACON_REQ_OK_EVT,     /* On beacon req TX success, if in scanning state, setup scan timer
                                   (macScanStartTimer) */
  MAC_TX_BEACON_REQ_FAIL_EVT,   /* On beacon req TX fail, if in scanning state, move on to next channel to scan
                                   (macScanNextChan) */
  MAC_TX_REALIGN_OK_EVT,        /* On coord realignment TX success, if in starting state, continue with coord
                                   realignment process (macStartContinue). Otherwise, send comm status indication
                                   (macCommStatusInd). */
  MAC_TX_REALIGN_FAIL_EVT,      /* On coord realignment TX fail, if in starting state, end realignment process
                                   (macStartComplete) and transition to idle state. Otherwise, send comm status
                                   indication (macCommStatusInd). */
  MAC_TX_DATA_REQ_OK_EVT,       /* On data poll (request) TX success with no data pending, handle data request
                                   completion (macAssocDataReqComplete or macPollDataReqComplete, depending
                                   on MAC state) */
  MAC_TX_DATA_REQ_FAIL_EVT,     /* On data poll (request) TX fail, handle identically to MAC_TX_DATA_REQ_OK_EVT
                                   (data poll without pending data) */
  MAC_TX_DATA_REQ_PEND_EVT,     /* On data poll (request) TX pending (ACK with pending bit set RX), start the
                                   response frame timer (macStartFrameResponseTimer) */
  MAC_TX_CONFLICT_OK_EVT,       /* On PAN ID conflict notification TX success, sends sync loss indication
                                   (macPanConflictComplete) */

  /* Triggered by timer event or starts timer event */
  MAC_TIM_RESPONSE_WAIT_EVT,    /* On association resp timer timeout (triggered by macStartResponseTimer),
                                   resends association request (macAssocDataReq) */
  MAC_TIM_FRAME_RESPONSE_EVT,   /* On data poll pending data timer timeout (triggered by macStartFrameResponseTimer),
                                   triggers timeout handling (macAssocFrameResponseTimeout or
                                   macPollFrameResponseTimeout depending on MAC state) */
  MAC_TIM_SCAN_EVT,             /* On API scan request (macApiScanReq), starts scanning process (macScanNextChan) */
  MAC_TIM_SYNC_EVT,             /* Unused event */
  MAC_TIM_EXP_INDIRECT_EVT,     /* On indirect frame expiration in the TX data queue, trigger handling function
                                   (macIndirectExpire) */

  /* Internal event */
  MAC_INT_SCAN_COMPLETE_EVT,    /* On scan completion, triggers handling function (macScanComplete) */
  MAC_INT_ASSOC_FAILED_EVT,     /* On association failure, triggers handling function (macAssocFailed) */
  MAC_INT_AUTO_POLL_EVT,        /* On beacon RX in a beacon mode device, sends an automatic poll request
                                   (macAutoPoll) */
  MAC_INT_START_COMPLETE_EVT,   /* On macStartContinue completion, trigger the network start completion handler
                                   (macStartComplete) */
  MAC_INT_TX_COMPLETE_EVT,      /* On TX completion or certain other conditions, trigger TX of queued data
                                   frames (macDataSend) */
  MAC_INT_BEACON_RESP_TIM_EVT,  /* On data poll pending timer start (macStartFrameResponseTimer) in a beacon mode
                                   device, start the data response frame timer (macBeaconStartFrameResponseTimer) */
  MAC_INT_BROADCAST_PEND_EVT,   /* On beacon RX with pending broadcast frame on a beacon mode device, start the
                                   broadcast frame timer (macStartBroadcastPendTimer)*/

  /* Other events and additions to base MAC events */
  MAC_NO_ACTION_EVT,                         /* No action taken when event is triggered */
  MAC_RX_ENHANCED_BEACON_REQ_EVT,            /* On enhanced beacon request RX, handle by sending an enhanced or normal
                                                beacon as requested (macRxEnhancedBeaconReq). Currently unsupported. */
  MAC_TX_ENHANCED_BEACON_REQ_OK_EVT,         /* Unused event */
  MAC_TX_ENHANCED_BEACON_REQ_FAIL_EVT,       /* Unused event */
  /* not needed MAC_RX_WS_ASYNC_IND_EVT, */
  MAC_CSMA_TIM_EXP_EVT,                      /* On CSMA delay timer expiration , trigger handler function
                                                (macTxCsmaDelay) */
  MAC_BC_TIM_EXP_EVT,                        /* On FH backoff timer expiration, trigger handler function
                                                (macBcTimerHandler) */
  TX_BACKOFF_TIM_EXP_EVT,                    /* On TX backoff timer expiration (requested by macRequestBackoff),
                                                reschedule frame TX (macTxBackoffHandler) */
  RX_BACKOFF_TIM_EXP_EVT                     /* On RX backoff timer expiration (requested by macRequestBackoff),
                                                retry radio RX on command */
};

/* This must be set to the last API event in the enumeration */
#define MAC_API_EVT_MAX   MAC_API_PWR_ON_REQ_EVT

/* Enumeration of action function table sets; maximum of 16 */
enum
{
  MAC_MGMT_ACTION_1,         // Set of management actions
  MAC_MGMT_ACTION_2,
  MAC_SCAN_ACTION,           // Set of scan actions
  MAC_DEVICE_ACTION_1,       // Set of general actions for all devices
  MAC_DEVICE_ACTION_2,
  MAC_COORD_ACTION_1,        // Set of actions for coordinator devices
  MAC_COORD_ACTION_2,
  MAC_COORD_ACTION_3,
  MAC_BEACON_DEVICE_ACTION,  // Set of actions for only beacon-mode devices
  MAC_ACTION_SET_MAX
};

/* these macros process the action enumeration value */
#define MAC_ACTION_INIT(n)      ((n) << 4)
#define MAC_ACTION_SET(n)       ((n) >> 4)
#define MAC_ACTION_ID(n)        ((n) & 0x0F)

/* Enumeration of action functions; maximum of 16 per set.
 * See hmac_global.c for the full state machine action/transition table.
 * See above EVT enum for an overview of corresponding events to MAC actions.
 */
enum
{
  /* Mgmt set 1: See mac_mgmt.c for function implmentations */
  MAC_NO_ACTION = MAC_ACTION_INIT(MAC_MGMT_ACTION_1),
  MAC_API_BAD_STATE,
  MAC_API_PENDING,
  MAC_API_DATA_REQ,
  MAC_COMM_STATUS_IND,
  MAC_DATA_SEND,
  MAC_DATA_RX_IND,
  MAC_API_WS_ASYNC_REQ,
  MAC_CSMA_TIM_EXPIRY,
  MAC_BC_TIM_EXPIRY,
  TX_BACKOFF_TIM_EXPIRY,
  RX_BACKOFF_TIM_EXPIRY,
  NOP_BACKOFF_TIM_EXPIRY,

  /* Mgmt set 2: See mac_mgmt.c for function implmentations */
  MAC_API_PWR_ON_REQ = MAC_ACTION_INIT(MAC_MGMT_ACTION_2),
  MAC_API_DISASSOCIATE_REQ,
  MAC_DISASSOC_COMPLETE,
  MAC_RX_BEACON,
  MAC_RX_DISASSOC,

  /* Scan: See MAC_Init() for state->function mapping and mac_scan.c for function implementations */
  MAC_API_SCAN_REQ = MAC_ACTION_INIT(MAC_SCAN_ACTION),
  MAC_SCAN_COMPLETE,
  MAC_SCAN_FAILED_IN_PROGRESS,
  MAC_SCAN_NEXT_CHAN,
  MAC_SCAN_RX_BEACON,
  MAC_SCAN_START_TIMER,

  /* Device set 1: See mac_device.c for function implementations */
  MAC_API_ASSOCIATE_REQ = MAC_ACTION_INIT(MAC_DEVICE_ACTION_1),
  MAC_API_POLL_REQ,
  MAC_ASSOC_DATA_REQ,
  MAC_ASSOC_DATA_REQ_COMPLETE,
  MAC_ASSOC_DATA_RX_IND,
  MAC_ASSOC_FAILED,
  MAC_ASSOC_FRAME_RESPONSE_TIMEOUT,
  MAC_POLL_DATA_REQ_COMPLETE,
  MAC_POLL_DATA_RX_IND,
  MAC_POLL_FRAME_RESPONSE_TIMEOUT,
  MAC_POLL_RX_ASSOC_RSP,
  MAC_RX_ASSOC_RSP,
  MAC_START_FRAME_RESPONSE_TIMER,
  MAC_START_RESPONSE_TIMER,

  /* Device set 2: See mac_device.c for function implementations */
  MAC_ASSOC_RX_DISASSOC = MAC_ACTION_INIT(MAC_DEVICE_ACTION_2),
  MAC_POLL_RX_DISASSOC,
  MAC_SCAN_RX_COORD_REALIGN,
  MAC_RX_COORD_REALIGN,
  MAC_PAN_CONFLICT_COMPLETE,

  /* Coordinator set 1: See mac_coord.c/mac_beacon_coord.c for function implementations */
  MAC_API_START_REQ = MAC_ACTION_INIT(MAC_COORD_ACTION_1),
  MAC_START_CONTINUE,

  /* Coordinator set 2: See mac_coord.c for function implementations */
  MAC_API_ASSOCIATE_RSP = MAC_ACTION_INIT(MAC_COORD_ACTION_2),
  MAC_RX_ASSOC_REQ,
  MAC_RX_BEACON_REQ,
  MAC_RX_DATA_REQ,
  MAC_START_COMPLETE,
  MAC_INDIRECT_EXPIRE,
  MAC_RX_ENHANCED_BEACON_REQ,

  /* Coordinator set 3: See mac_coord.c for function implementations */
  MAC_API_ORPHAN_RSP = MAC_ACTION_INIT(MAC_COORD_ACTION_3),
  MAC_API_PURGE_REQ,
  MAC_RX_ORPHAN,
  MAC_RX_PAN_CONFLICT,

  /* Beacon device: See mac_beacon_device.c for function implementations  */
  MAC_API_SYNC_REQ = MAC_ACTION_INIT(MAC_BEACON_DEVICE_ACTION),
  MAC_AUTO_POLL,
  MAC_BEACON_START_FRAME_RSP_TIMER,
  MAC_START_BROADCAST_PEND_TIMER
};

/* Special hdr.event mask value used to mark buffer as reserved (do not deallocate) */
#define MAC_BUF_RESERVED                0x80

/* OSAL task events (as uint8) */
#define MAC_RESPONSE_WAIT_TASK_EVT      0x01
#define MAC_FRAME_RESPONSE_TASK_EVT     0x02
#define MAC_SCAN_TASK_EVT               0x04
#define MAC_EXP_INDIRECT_TASK_EVT       0x08
#define MAC_TX_COMPLETE_TASK_EVT        0x10
#define MAC_RX_QUEUE_TASK_EVT           0x20
#define MAC_START_COMPLETE_TASK_EVT     0x40
#define MAC_BROADCAST_PEND_TASK_EVT     0x80
#define MAC_CSMA_TIM_TASK_EVT           0x100
#define MAC_BC_TIM_TASK_EVT             0x200
#define MAC_TX_DATA_REQ_TASK_EVT        0x400

#define MAC_TX_BACKOFF_TIM_TASK_EVT     0x800
#define MAC_RX_BACKOFF_TIM_TASK_EVT     0x1000

/* State table format */
#define MAC_ACTION_POS                  0       /* action position */
#define MAC_NEXT_STATE_POS              1       /* next state position */
#define MAC_NUM_COLS                    2       /* number of columns in state tables */

/* Configurable features */
enum
{
  MAC_FEAT_DEVICE,
  MAC_FEAT_COORD,
  MAC_FEAT_SECURITY,
  MAC_FEAT_BEACON_COORD,
  MAC_FEAT_BEACON_DEVICE,
  MAC_FEAT_GTS_CLIENT,
  MAC_FEAT_GTS_SERVER,
  MAC_FEAT_MAX
};

/* Configurable feature masks */
#define MAC_FEAT_DEVICE_MASK            ((uint16) 1 << MAC_FEAT_DEVICE)
#define MAC_FEAT_COORD_MASK             ((uint16) 1 << MAC_FEAT_COORD)
#define MAC_FEAT_SECURITY_MASK          ((uint16) 1 << MAC_FEAT_SECURITY)
#define MAC_FEAT_BEACON_COORD_MASK      ((uint16) 1 << MAC_FEAT_BEACON_COORD)
#define MAC_FEAT_BEACON_DEVICE_MASK     ((uint16) 1 << MAC_FEAT_BEACON_DEVICE)
#define MAC_FEAT_GTS_CLIENT_MASK        ((uint16) 1 << MAC_FEAT_GTS_CLIENT)
#define MAC_FEAT_GTS_SERVER_MASK        ((uint16) 1 << MAC_FEAT_GTS_SERVER)

#define MAC_FEAT_GTS_MASK               (MAC_FEAT_GTS_CLIENT_MASK | MAC_FEAT_GTS_SERVER_MASK)

/* RX enable/disable masks */
#define MAC_RX_POLL                     0x01    /* polling/associating */
#define MAC_RX_WHEN_IDLE                0x02    /* PIB rxOnWhenIdle */
#define MAC_RX_SCAN                     0x04    /* scanning */
#define MAC_RX_ACK_WAITING              0x08    /* waiting for an acknowledgment */
#define MAC_RX_BROADCAST_PEND           0x10    /* pending broadcast */
#define MAC_RX_BEACON_DEVICE            0x20    /* beacon tracking */
#define MAC_RX_BEACON_NETWORK           0x40    /* beacon network */
#define MAC_RX_BEACON_SYNC              0x80    /* beacon sync */

#define MAC_RX_FH_BC_SLOT               0x20  /* enable RX during BC slot for FH */

/* TX mode masks */
#define MAC_TX_MODE_NON_CSMA            0x01    /* frame tx without CSMA */
#define MAC_TX_MODE_SLOTTED             0x02    /* frame tx in slotted mode */
#define MAC_TX_MODE_INTERNAL            0x08    /* frame is for internal purposes */
#define MAC_TX_MODE_SCAN_RELATED        0x10    /* frame is scan related */
#define MAC_TX_MODE_BROADCAST           0x20    /* broadcast frame in beacon network */
#define MAC_TX_MODE_BEACON              0x40    /* beacon tx in beacon network */
#define MAC_TX_MODE_REQUESTED           0x80    /* indirect frame was requested in beacon network */

/* TX schedule masks */
#define MAC_TX_SCHED_OUTGOING_CAP       0x01    /* outgoing CAP (default) */
#define MAC_TX_SCHED_INCOMING_CAP       0x02    /* incoming CAP */
#define MAC_TX_SCHED_INDIRECT_NO_ACK    0x04    /* frame is indirect and not acked */
#define MAC_TX_SCHED_READY              0x10    /* frame scheduled as ready to tx */
#define MAC_TX_SCHED_SCAN_RELATED       0x20    /* frame is scan related */
#define MAC_TX_SCHED_MARKED             0x40    /* frame is marked for pending address list */
#define MAC_TX_SCHED_INDIRECT           0x80    /* frame is indirect */
#define MAC_TX_SCHED_INACTIVE           0xFF    /* inactive portion of the superframe */



/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* API parameter checking macro */
#if defined(MAC_NO_PARAM_CHECK) && MAC_NO_PARAM_CHECK == TRUE
#define MAC_PARAM_STATUS(expr, status)
#else
#define MAC_PARAM_STATUS(expr, status)  if ((expr) == FALSE) status = MAC_INVALID_PARAMETER
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* State table type */
typedef const uint8 (CODE * macStateTbl_t)[MAC_NUM_COLS];

/* Event structure for API events with buffer for command frame */
typedef struct
{
  macEventHdr_t               hdr;
  sData_t                     mpdu;
  macTxIntData_t              internal;
  union
  {
    ApiMac_mlmeAssociateReq_t     associateReq;
    ApiMac_mlmeAssociateRsp_t     associateRsp;
    ApiMac_mlmeDisassociateReq_t  disassociateReq;
    ApiMac_mlmeOrphanRsp_t        orphanRsp;
    ApiMac_mlmeWSAsyncReq_t       asyncReq;
  } mac;
} macApiDataEvent_t;

/* MLME purge request type */
typedef struct
{
  uint8                       msduHandle;
} macMcpsPurgeReq_t;


/* Event structure for API events */
typedef struct
{
  macEventHdr_t               hdr;
  union
  {
    ApiMac_mlmePollReq_t      pollReq;
    macMlmeScanReq_t          scanReq;
    ApiMac_mlmeStartReq_t     startReq;
    ApiMac_mlmeSyncReq_t      syncReq;
    ApiMac_mlmeWSAsyncReq_t   wsAsyncReq;
    macMcpsPurgeReq_t         purgeReq;
  } mac;
} macApiEvent_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mcpsDataReq_t        dataReq;
} macMcpsDataReqApiMac_t;

/* Union of event structures */
typedef union
{
  macEventHdr_t               hdr;
  macApiEvent_t               api;
  macApiDataEvent_t           apiData;
  macMcpsDataReqApiMac_t      apiDataReq;
  macMcpsDataReq_t            dataReq;
  macMcpsDataInd_t            dataInd;
  macTx_t                     tx;
  macRx_t                     rx;
  macMlmeBeaconNotifyInd_t    beaconNotify;
} macEvent_t;

/* Action function type */
typedef void (*macAction_t)(macEvent_t *pEvent);

/* Action function set type */
typedef const macAction_t * macActionSet_t;

/* Type for mac_main data */
typedef struct
{
  macEvent_t    *pPending;      /* pointer to pending buffer */
  uint8         *pBuf;
  uint16        featureMask;    /* mask of initialized features */
  uint8         state;          /* state machine state */
} macMain_t;

/* Type for module reset functions */
typedef void (*macReset_t)(void);

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macMainReset(void);
MAC_INTERNAL_API void macSetEvent(uint8 event);
MAC_INTERNAL_API void macSetEvent16(uint16 event);
MAC_INTERNAL_API void macCbackForEvent(macEvent_t *pEvent, uint8 status);
MAC_INTERNAL_API bool macStateIdle(void);
MAC_INTERNAL_API bool macStateIdleOrPolling(void);
MAC_INTERNAL_API bool macStateScanning(void);
MAC_INTERNAL_API void macExecute(macEvent_t *pEvent);
MAC_INTERNAL_API uint8 macSendMsg(uint8 event, void *pData);
MAC_INTERNAL_API uint8 macSendDataMsg(uint8 event, void *pData, ApiMac_sec_t *sec);
void macMainSetTaskId(uint8 taskId);
MAC_INTERNAL_API void macMainReserve(uint8 *ptr);
MAC_INTERNAL_API void mac_msg_deallocate(uint8 **msg_ptr);

/**************************************************************************************************
*/

#endif /* MAC_MAIN_H */
