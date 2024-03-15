/**************************************************************************************************
  Filename:       bdb_touchlink.h
  Revised:        $Date: 2013-07-15 15:29:01 -0700 (Mon, 15 Jul 2013) $
  Revision:       $Revision: 34720 $

  Description:    This file contains the BDB TouchLink Initiator definitions.


  Copyright 2011-2013 Texas Instruments Incorporated.

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

#ifndef TOUCHLINK_INITIATOR_H
#define TOUCHLINK_INITIATOR_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "zcl_general.h"
#include "bdb_tl_commissioning.h"

/*********************************************************************
 * CONSTANTS
 */
// time interval (in msec) between target selection and configuration,
// to allow target visual identification by the user.
#define TOUCHLINK_INITIATOR_IDENTIFY_INTERVAL          500

// If defined, the initiator will determine new TouchLink network parameters.
// Otherwise, it will be determind by the target starting the network.
//#define TOUCHLINK_INITIATOR_SET_NEW_NWK_PARAMS

#define TOUCHLINK_NWK_DISC_CNF_EVT                               0x0001
#define TOUCHLINK_NWK_JOIN_IND_EVT                               0x0002
#define TOUCHLINK_START_NWK_EVT                                  0x0004
#define TOUCHLINK_JOIN_NWK_ATTEMPT_EVT                           0x0008
#define TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT                     0x0010
#define TOUCHLINK_TL_SCAN_BASE_EVT                               0x0020
#define TOUCHLINK_CFG_TARGET_EVT                                 0x0040
#define TOUCHLINK_W4_NWK_START_RSP_EVT                           0x0080
#define TOUCHLINK_W4_NWK_JOIN_RSP_EVT                            0x0100
#define TOUCHLINK_DISABLE_RX_EVT                                 0x0200
#define TOUCHLINK_W4_REJOIN_EVT                                  0x0400
#define TOUCHLINK_NOTIFY_APP_EVT                                 0x0800
#define TOUCHLINK_NWK_RTR_START_EVT                              0x1000
#define TOUCHLINK_NWK_FORMATION_SUCCESS_EVT                      0x2000
#define TOUCHLINK_NWK_ANNOUNCE_EVT                               0x4000


/*********************************************************************
 * TYPEDEFS
 */
// This callback is called to notify the application when a target device is
// successfully touch-linked.
typedef ZStatus_t (*touchLink_NotifyAppTLCB_t)( epInfoRec_t *pData );

// This callback is called to decide whether to select a device, which responded to scan, as a target.
// Note newScanRsp value should be copied if used beyond the call scope.
typedef uint8_t (*touchLink_SelectDiscDevCB_t)( const bdbTLScanRsp_t *newScanRsp, int8_t newRssi );

/*********************************************************************
 * MACROS
 */

// TOUCHLINK Commissioning Utility commands initiation enablement
#define TOUCHLINK_UTILITY_SEND_EPINFO_ENABLED
#define TOUCHLINK_UTILITY_SEND_GETEPLIST_ENABLED
#define TOUCHLINK_UTILITY_SEND_GETGRPIDS_ENABLED

/*********************************************************************
 * VARIABLES
 */
extern bdbTLDeviceInfo_t touchLinkSampleRemote_DeviceInfo;
extern uint16_t selectedTargetNwkAddr;
extern ZLongAddr_t selectedTargetIEEEAddr;

/*********************************************************************
 * FUNCTIONS
 */

/*-------------------------------------------------------------------
 * TASK API - These functions must only be called by OSAL.
 */

/*
 * Initialization for the TouchLink Initiator task
 */
void touchLinkInitiator_Init( uint8_t task_id );

/*
 *  Event Process for the TouchLink Initiator task
 */
uint32_t touchLinkInitiator_event_loop( uint8_t task_id, uint32_t events );

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/

/*
 * Register application task to receive unprocessed messages
 */
ZStatus_t touchLinkInitiator_RegisterForMsg( uint8_t taskId );

/*
 * Start Touch-Link device discovery
 */
ZStatus_t touchLinkInitiator_StartDevDisc( void );

/*
 * Abort Touch-Link
 */
ZStatus_t touchLinkInitiator_AbortTL( void );

/*
 * Change Channel for Frequency Agility
 */
ZStatus_t touchLinkInitiator_ChannelChange( uint8_t targetChannel );

/*
 * Send EP Info
 */
ZStatus_t touchLinkInitiator_SendEPInfo( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t seqNum);

/*
 * Send Reset to FN to the selected target of the current TL transaction
 */
ZStatus_t touchLinkInitiator_ResetToFNSelectedTarget( void );

#if (ZSTACK_ROUTER_BUILD)
/*
 * Set the router permit join flag
 */
ZStatus_t touchLinkInitiator_PermitJoin( uint8_t duration );
#endif

/*
 * @brief   Select a unique PAN ID and Extended PAN ID when compared to
 *          the PAN IDs and Extended PAN IDs of the networks detected
 *          on the TOUCHLINK channels. The selected Extended PAN ID must be
 *          a random number (and not equal to our IEEE address).
 */
void initiatorSelectNwkParams( void );

/*
 * @brief   Send out a Scan Response command.
 */
ZStatus_t touchLink_InitiatorSendScanRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint32_t transID, uint8_t seqNum );

/*
 * @brief   Get scan base time
 */
uint16_t initiatorProcessGetScanBaseTime(bool stop);

/*
 * @brief   Send network join event to touchlink event loop
 */
void initiatorProcessNwkJoinEvt(afAddrType_t *pAddr, bdbTLNwkJoinReq_t *pNwkJoinReq, uint8_t seqNum);

/*
 * @brief   Process ZDO device state change
 */
void initiatorProcessStateChange(devStates_t state);

/*
 * @brief   Set our network state to its original state.
 */
void initiatorSetNwkToInitState();

/*
 * @brief   clear selected target variable.
 */
void initiatorClearSelectedTarget(void);

/*
 * @brief   Send network update event to touchlink event loop
 */
void initiatorProcessNwkUpdateEvt(bdbTLNwkUpdateReq_t *pNwkUpdateReq );

/*
 * @brief   Send scan response event to touchlink event loop
 */
void initiatorProcessScanRsp(afAddrType_t *pSrcAddr, bdbTLScanRsp_t *pRsp);

/*
 * @brief   Send device information response to touchlink event loop
 */
void initiatorProcessDevInfoRsp(afAddrType_t *pDstAddr, bdbTLDeviceInfoRsp_t *pRsp);

/*
 * @brief   Send network start response to touchlink event loop
 */
void initiatorProcessNwkStartRsp(bdbTLNwkStartRsp_t *pRsp);

/*
 * @brief   Send network join response to touchlink event loop
 */
void initiatorProcessNwkJointRsp(afAddrType_t *pSrcAddr, bdbTLNwkJoinRsp_t *pRsp);

/*
 * @brief   Set our network state to its original state.
 */
void initiatorSetNwkToInitState( void );

/*
 * @brief   clear selected target variable.
 */
void initiatorClearSelectedTarget( void );
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* TOUCHLINK_INITIATOR_H */
