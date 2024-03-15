/**************************************************************************************************
  Filename:       bdb_touchlink_target.h
  Revised:        $Date: 2013-05-10 15:34:14 -0700 (Fri, 10 May 2013) $
  Revision:       $Revision: 34239 $

  Description:    This file contains the ZCL Touch Link Target definitions.


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

#ifndef TOUCHLINK_TARGET_H
#define TOUCHLINK_TARGET_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl_general.h"
#include "bdb_tl_commissioning.h"

/*********************************************************************
 * CONSTANTS
 */

// Task Events
#define TOUCHLINK_NWK_DISC_CNF_EVT                               0x0001
#define TOUCHLINK_NWK_JOIN_IND_EVT                               0x0002
#define TOUCHLINK_RESET_TO_FN_EVT                                0x0004
#define TOUCHLINK_START_NWK_EVT                                  0x0008
#define TOUCHLINK_JOIN_ATTEMPT_EVT                               0x0010
#define TOUCHLINK_NWK_START_EVT                                  0x0020
#define TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT                     0x0040
#define TOUCHLINK_TARGET_ENABLE_TIMEOUT                          0x0080
#define TOUCHLINK_NWK_RESTORE_NWK_PARAMETERS_EVT                 0x0100
#define TOUCHLINK_NWK_FORMATION_SUCCESS_EVT                      0x0200
#define TOUCHLINK_NWK_ANNOUNCE_EVT                               0x0400

#define TARGET_PERPETUAL_OPERATION_ENABLED          FALSE
#define TOUCHLINK_TARGET_PERPETUAL                  86400001 //currently we decided that 24 hours is the maximum time. more than this - will be treated as perpetual

/*********************************************************************
 * TYPEDEFS
 */
typedef void (*tlGCB_TargetEnable_t)( uint8_t enable );

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*-------------------------------------------------------------------
 * TASK API - These functions must only be called by OSAL.
 */

/*
 * Initialization for the TouchLink Target task
 */
extern void touchLinkTarget_Init( uint8_t task_id );

/*
 * Event Process for the TouchLink Target task
 */
extern uint32_t touchLinkTarget_event_loop( uint8_t task_id, uint32_t events );

/*
 * @brief   This callback is called to process a Leave Confirmation message.
 */
extern void* targetZdoLeaveCnfCB( void *pParam );

/*
 * @brief   Send out a Scan Response command.
 */
extern ZStatus_t touchLink_TargetSendScanRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint32_t transID, uint8_t seqNum );


#if (ZSTACK_ROUTER_BUILD)
/*
 * @brief   Send out a Network Start Response command.
 */
extern void targetSendNwkStartRsp( afAddrType_t *dstAddr, uint32_t transID, uint8_t status,
                                   bdbTLNwkParams_t *pNwkParams, uint8_t nwkUpdateId, uint8_t seqNum );

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
/*
 * @brief   Send network start event to touchlink event loop
 */
extern void targetProcessNwkStartEvt(afAddrType_t *pAddr, bdbTLNwkStartReq_t *pNwkStartReq, uint8_t seqNum, bool allowStealing);
#endif
#endif  //  ZSTACK_ROUTER_BUILD

/*
 * @brief   Process ZDO device state change
 */
void targetProcessStateChange( devStates_t devState );

/*
 * @brief   Send network join event to touchlink event loop
 */
extern void targetProcessNwkJoinEvt(afAddrType_t *pAddr, bdbTLNwkJoinReq_t *pNwkJoinReq, uint8_t seqNum, bool allowStealing);

/*
 * @brief   Send factory new reset to event loop.
 */
extern void targetProcessResetToFNEvt( void );

/*
 * @brief   Send network Update event to touchlink event loop
 */
extern void targetProcessNwkUpdateEvt(bdbTLNwkUpdateReq_t *pNwkUpdateReq);

/*
 * @brief   General function to allow stealing when performing TL as target
 */
extern void bdb_TouchlinkSetAllowStealing( bool allow );

/*
 * @brief   General function to get the allow stealing value
 */
extern bool bdb_TouchlinkGetAllowStealing( void );

/*
 * @brief   Register an Application's Enable/Disable callback function.
 *          Refer to touchLinkTarget_EnableCommissioning to enable/disable TL as target
 */
extern void bdb_RegisterTouchlinkTargetEnableCB( tlGCB_TargetEnable_t pfnTargetEnableChange );

/*
 * @brief   This callback is called to process a Leave Confirmation message.
 *
 *          Note: this callback function returns a pointer if it has handled
 *                the confirmation message and no further action should be
 *                taken with it. It returns NULL if it has not handled the
 *                confirmation message and normal processing should take place.
 */
extern void *initiatorZdoLeaveCnfCB( void *pParam );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* TOUCHLINK_TARGET_H */
