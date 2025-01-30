/******************************************************************************

 @file  mac_coord.h

 @brief Internal interface file for the MAC coordinator module.

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

#ifndef MAC_COORD_H
#define MAC_COORD_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_high_level.h"
#include "mac_timer.h"
#include "api_mac.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Typedefs
 * ------------------------------------------------------------------------------------------------
 */
/* Coordinator info type */
typedef struct
{
  macTimer_t             indirectTimer;      /* indirect frame timer */
  ApiMac_mlmeStartReq_t* pStartReq;         /* buffer allocated to store copy of start parameters */
  bool                   beaconing;          /* TRUE if beaconing network has been started */
  ApiMac_sec_t           realignSec;         /* Security parameters for the coordinator realignment frame */
  ApiMac_sec_t           beaconSec;          /* Security parameters for the beacon frame */
#ifdef FEATURE_ENHANCED_BEACON
  uint8*                 pIEIDs;             /* buffer containing IE IDs */
  uint8                  numIEs;             /* number of IE IDs */
#endif /* FEATURE_ENHANCED_BEACON */
} macCoord_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/* Action functions */
MAC_INTERNAL_API void macIndirectExpire(macEvent_t *pEvent);
MAC_INTERNAL_API void macRxAssociateReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macApiAssociateRsp(macEvent_t *pEvent);
MAC_INTERNAL_API void macRxOrphan(macEvent_t *pEvent);
MAC_INTERNAL_API void macApiOrphanRsp(macEvent_t *pEvent);
MAC_INTERNAL_API void macApiPurgeReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macApiStartReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macStartContinue(macEvent_t *pEvent);
MAC_INTERNAL_API void macStartComplete(macEvent_t *pEvent);
#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
MAC_INTERNAL_API void macRxBeaconReq(macEvent_t *pEvent);
#endif /* (FEATURE_BEACON_MODE) || (FEATURE_NON_BEACON_MODE) */
MAC_INTERNAL_API void macRxDataReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macRxPanConflict(macEvent_t *pEvent);
#ifdef FEATURE_ENHANCED_BEACON
MAC_INTERNAL_API void macRxEnhancedBeaconReq(macEvent_t *pEvent);
#endif /* FEATURE_ENHANCED_BEACON */

/* utility functions */
MAC_INTERNAL_API uint8 macStartBegin(macEvent_t *pEvent);
MAC_INTERNAL_API uint8 macBuildAssociateRsp(macEvent_t *pEvent);

MAC_INTERNAL_API void macStartSetParams(ApiMac_mlmeStartReq_t *pStartReq);
MAC_INTERNAL_API uint8 macBuildRealign(macTx_t *pMsg, sAddr_t *pDstAddr, uint16 coordShortAddr,
                                      uint16 newPanId, uint8 logicalChannel);
MAC_INTERNAL_API void macCoordReset(void);
MAC_INTERNAL_API bool macIndirectSend(sAddr_t *pAddr, uint16 panId);
#ifdef FEATURE_ENHANCED_BEACON
MAC_INTERNAL_API macTx_t *macBuildEnhancedBeacon(uint8 beaconOrder, uint8 superframeOrder,
                                                 bool battLifeExt, uint8 coexistIe);
#endif /* FEATURE_ENHANCED_BEACON */


#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
uint8 macPendAddrLen(uint8 *maxPending);
uint8 *macBuildPendAddr(uint8 *p, uint8 maxPending, bool *pBroadcast);
#endif /* (FEATURE_BEACON_MODE) || (FEATURE_NON_BEACON_MODE) */

void macIndirectMark(macTx_t *pMsg);
void macIndirectTxFrame(macTx_t *pMsg);
void macIndirectRequeueFrame(macTx_t *pMsg);

#ifdef FEATURE_ENHANCED_BEACON
bool macProcessEBeaconFilterIe(macEvent_t* pEvent, uint8* pContent, uint32* pAttribIds);
#endif /* FEATURE_ENHANCED_BEACON */

/**************************************************************************************************
*/

#endif /* MAC_COORD_H */
