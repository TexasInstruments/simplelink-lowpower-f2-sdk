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

/* State machine events */
enum
{
    MAC_API_DATA_REQ_EVT, /* MAP_macApiDataReq */
    MAC_API_PURGE_REQ_EVT,
    MAC_API_START_REQ_EVT,
    MAC_API_WS_ASYNC_REQ_EVT,
    MAC_RX_DATA_IND_EVT,
    MAC_INT_START_COMPLETE_EVT,
    MAC_INT_TX_COMPLETE_EVT,
    MAC_NO_ACTION_EVT,
    MAC_CSMA_TIM_EXP_EVT,
    TX_BACKOFF_TIM_EXP_EVT,
    RX_BACKOFF_TIM_EXP_EVT,
    MAC_NO_OF_EVENTS
};
/* This must be set to the last API event in the enumeration */
#define MAC_API_EVT_MAX   MAC_API_PWR_ON_REQ_EVT

/* Enumeration of action function table sets; maximum of 16 */
enum
{
  MAC_MGMT_ACTION_1,
  MAC_MGMT_ACTION_2,
  MAC_SCAN_ACTION,
  MAC_DEVICE_ACTION_1,
  MAC_DEVICE_ACTION_2,
  MAC_COORD_ACTION_1,
  MAC_COORD_ACTION_2,
  MAC_COORD_ACTION_3,
  MAC_BEACON_DEVICE_ACTION,
  MAC_ACTION_SET_MAX
};

/* these macros process the action enumeration value */
#define MAC_ACTION_INIT(n)      ((n) << 4)
#define MAC_ACTION_SET(n)       ((n) >> 4)
#define MAC_ACTION_ID(n)        ((n) & 0x0F)

/* Enumeration of action functions; maximum of 16 per set */
enum
{
  /* Mgmt set 1 */
  MAC_NO_ACTION = MAC_ACTION_INIT(MAC_MGMT_ACTION_1),
  MAC_API_DATA_REQ,
  MAC_DATA_SEND,
  MAC_DATA_RX_IND,
  MAC_API_WS_ASYNC_REQ,
  MAC_CSMA_TIM_EXPIRY,
  TX_BACKOFF_TIM_EXPIRY,
  RX_BACKOFF_TIM_EXPIRY,

  /* Coordinator set 1 */
  MAC_API_START_REQ = MAC_ACTION_INIT(MAC_COORD_ACTION_1),

  /* Coordinator set 2 */
  MAC_START_COMPLETE = MAC_ACTION_INIT(MAC_COORD_ACTION_2),

  /* Coordinator set 3 */
  MAC_API_PURGE_REQ = MAC_ACTION_INIT(MAC_COORD_ACTION_3),
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
#define MAC_MSG_LOG_EVT                 0x2000

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
