/**************************************************************************************************
  Filename:       mt_nwk.h
  Revised:        $Date: 2010-02-04 14:28:44 -0800 (Thu, 04 Feb 2010) $
  Revision:       $Revision: 21656 $

  Description:    MonitorTest functions for the NWK layer.


  Copyright 2007-2010 Texas Instruments Incorporated.

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


/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/

#include "hal_types.h"
#include "nl_mede.h"

/***************************************************************************************************
 * MACROS
 ***************************************************************************************************/
#define NWKCB_CHECK(cbi) (_nwkCallbackSub & (cbi))

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
/* NWK Callback subscription IDs */
#define CB_ID_NLDE_DATA_CNF          0x0001
#define CB_ID_NLDE_DATA_IND          0x0002
#define CB_ID_NLME_INIT_COORD_CNF    0x0004
#define CB_ID_NLME_JOIN_CNF          0x0008
#define CB_ID_NLME_JOIN_IND          0x0010
#define CB_ID_NLME_LEAVE_CNF         0x0020
#define CB_ID_NLME_LEAVE_IND         0x0040
#define CB_ID_NLME_POLL_CNF          0x0080
#define CB_ID_NLME_SYNC_IND          0x0200
#define CB_ID_NLME_NWK_DISC_CNF      0x2000
#define CB_ID_NLME_START_ROUTER_CNF	 0x8000

/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
extern uint16_t _nwkCallbackSub;

/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/

#ifdef MT_NWK_FUNC
/*
 *   Process all the NWK commands that are issued by test tool
 */
extern uint8_t MT_NwkCommandProcessing (byte *pBuf);

#endif   /* NWK Command Processing in MT */

#ifdef MT_NWK_CB_FUNC
/*
 * Process the callback subscription for NLDE-DATA.confirm
 */
extern void nwk_MTCallbackSubDataConfirm(byte nsduHandle, ZStatus_t status);

/*
 * Process the callback subscription for NLDE-DATA.indication
 */
extern void nwk_MTCallbackSubDataIndication(uint16_t SrcAddress, int16_t nsduLength,
                                            byte *nsdu, byte LinkQuality);

/*
 * Process the callback subscription for NLME-INIT-COORD.confirm
 */
extern void nwk_MTCallbackSubInitCoordConfirm(ZStatus_t Status);

/*
 * Process the callback subscription for NLME-START-ROUTER.confirm
 */
extern void nwk_MTCallbackSubStartRouterConfirm(ZStatus_t Status);

/*
 * Process the callback subscription for NLME_NWK-DISC.confirm
 */
extern void nwk_MTCallbackSubNetworkDiscoveryConfirm(byte ResultCount,	networkDesc_t *NetworkList);

/*
 * Process the callback subscription for NLME-JOIN.confirm
 */
extern void nwk_MTCallbackSubJoinConfirm(uint16_t PanId, ZStatus_t Status);

/*
 * Process the callback subscription for NLME-INIT-COORD.indication
 */
extern void nwk_MTCallbackSubJoinIndication(uint16_t ShortAddress, byte *ExtendedAddress,
                                            byte CapabilityFlags);

/*
 * Process the callback subscription for NLME-LEAVE.confirm
 */
extern void nwk_MTCallbackSubLeaveConfirm(byte *DeviceAddress, ZStatus_t Status);

/*
 * Process the callback subscription for NLME-LEAVE.indication
 */
extern void nwk_MTCallbackSubLeaveIndication(byte *DeviceAddress);

/*
 *  Process the callback subscription for NLME-SYNC.indication
 */
extern void nwk_MTCallbackSubSyncIndication(void);

/*
 *  Process the callback subscription for NLME-POLL.confirm
 */
extern void nwk_MTCallbackSubPollConfirm(byte status);

#endif   /*NWK Callback Processing in MT*/
/*
 * Process the callback for Ping
 *
 */
extern void nwk_MTCallbackPingConfirm(uint16_t DstAddress, byte pingSeqNo,
                                      uint16_t delay, byte routeCnt, byte *routeAddr);

/*********************************************************************
*********************************************************************/
