/**************************************************************************************************
  Filename:       rtg.h
  Revised:        $Date: 2014-11-18 14:45:15 -0800 (Tue, 18 Nov 2014) $
  Revision:       $Revision: 41167 $

  Description:    Interface to mesh routing functions


  Copyright 2004-2014 Texas Instruments Incorporated.

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

#ifndef RTG_H
#define RTG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "nwk_util.h"
#include "nwk_bufs.h"
#include "zglobals.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define RTG_TIMER_INTERVAL    1000

//Route request command option
#define MTO_ROUTE           0x01       // Used in option of NLME_RouteDiscoveryRequest() and rtgTable[]
#define NO_ROUTE_CACHE      0x02       // Used in option of NLME_RouteDiscoveryRequest() and rtgTable[]
#define RTG_RECORD          0x04       // Used in option of rtgTable[]
#define MTO_ROUTE_RC        0x08       // Sender has route cache. Used in option of rtgTable[]
#define MTO_ROUTE_NRC       0x10       // Sender doesn't have route cache. Used in option of rtgTable[]
#define DEST_IEEE_ADDR      0x20       // Used in option of route request command frame
#define MULTICAST_ROUTE     0x40       // Ued in all three places
#define RREQ_OPTIONS_MASK   0x78       // b'0111,1000   0-2, 7 are reserved bits

#define RTG_MTO_DEST_ADDRESS   NWK_BROADCAST_SHORTADDR_DEVZCZR  //0xFFFC

#define RREP_ORI_IEEE       0x10
#define RREP_RES_IEEE       0x20
#define RREP_MULTICAST      0x40
#define RREP_OPTIONS_MASK   (RREP_ORI_IEEE | RREP_RES_IEEE | RREP_MULTICAST)

#define RTG_END_DEVICE_ADDR_TYPE 0
#define RTG_ROUTER_ADDR_TYPE     1

#define RTG_NO_EXPIRY_TIME  0xFF

/*********************************************************************
 * TYPEDEFS
 */

typedef enum
{
  RTG_SUCCESS,
  RTG_FAIL,
  RTG_TBL_FULL,
  RTG_HIGHER_COST,
  RTG_NO_ENTRY,
  RTG_INVALID_PATH,
  RTG_INVALID_PARAM,
  RTG_SRC_TBL_FULL
} RTG_Status_t;

// status values for routing entries
#define RT_INIT       0
#define RT_ACTIVE     1
#define RT_DISC       2
#define RT_LINK_FAIL  3
#define RT_REPAIR     4

// Routing table entry
//   Notice, if you change this structure, you must also change
//   rtgItem_t in zd_profile.h
typedef struct
{
  uint16_t  dstAddress;
  uint16_t  nextHopAddress;
  byte    expiryTime;
  byte    status;
  uint8_t   options;
} rtgEntry_t;

// Route discovery table entry
typedef struct
{
  byte    rreqId;
  uint16_t  srcAddress;
  uint16_t  previousNode;
  byte    forwardCost;
  byte    residualCost;
  byte    expiryTime;
} rtDiscEntry_t;

// Broadcast table entry.
typedef struct
{
  uint16_t srcAddr;
  uint8_t  bdt; // broadcast delivery time
  uint8_t  pat; // passive ack timeout
  uint8_t  mbr; // max broadcast retries
  uint8_t  handle;
  // Count of non-sleeping neighbors and router children.
  uint8_t  ackCnt;
  uint8_t  id;
} bcastEntry_t;

// Source routing table
typedef struct
{
  uint8_t    expiryTime;
  uint8_t    relayCount;
  uint16_t   dstAddress;
  uint16_t*  relayList;
} rtgSrcEntry_t;
/*********************************************************************
 * GLOBAL VARIABLES
 */

extern rtgEntry_t rtgTable[];
extern rtDiscEntry_t rtDiscTable[];

extern rtgSrcEntry_t rtgSrcTable[];

/*********************************************************************
 * FUNCTIONS
 */

extern void RTG_Init( void );

extern rtgEntry_t *RTG_GetRtgEntry( uint16_t DstAddress, uint8_t options);

extern RTG_Status_t RTG_RemoveRtgEntry( uint16_t DstAddress, uint8_t options );

extern uint16_t RTG_GetNextHop( uint16_t DstAddress, uint16_t avoidAddr,
                             uint16_t avoidAddr2, uint16_t avoidAddr3, uint8_t options );

extern byte RTG_ProcessRreq(
           NLDE_FrameFormat_t *ff, uint16_t macSrcAddress, uint16_t *nextHopAddr );

extern void RTG_ProcessRrep( NLDE_FrameFormat_t *ff, uint16_t macSrcAddress );

extern void RTG_ProcessRrec( NLDE_FrameFormat_t *ff );

extern uint8_t RTG_ProcessRErr( NLDE_FrameFormat_t *ff );
extern void RTG_TimerEvent( void );

extern uint16_t RTG_AllocNewAddress( byte deviceType );

extern void RTG_DeAllocTreeAddress( uint16_t shortAddr );

extern void RTG_DeAllocStochasticAddress( uint16_t shortAddr );

extern void RTG_BcastTimerHandler( void );

extern byte RTG_BcastChk( NLDE_FrameFormat_t *ff, uint16_t macSrcAddr );

extern byte RTG_BcastAdd(NLDE_FrameFormat_t*ff, uint16_t macSrcAddr, byte handle);

extern void RTG_BcastDel( byte handle );

extern void RTG_DataReq( OsalPort_EventHdr *inMsg );

extern byte RTG_PoolAdd( NLDE_FrameFormat_t *ff );

extern uint16_t RTG_GetTreeRoute( uint16_t dstAddress );

extern uint16_t RTG_SrcGetNextHop( uint8_t rtgIndex, uint16_t* rtgList);

extern uint8_t RTG_ValidateSrcRtg(uint8_t relayCnt, uint8_t relayIdx, uint16_t* relayList );

extern uint8_t RTG_RtgRecordInitiation( uint16_t DstAddress, uint16_t SrcAddress, uint8_t options);

extern RTG_Status_t RTG_GetRtgSrcEntry( uint16_t dstAddr, uint8_t* pRelayCnt, uint16_t** ppRelayList);
extern RTG_Status_t RTG_CheckRtStatus( uint16_t DstAddress, byte RtStatus, uint8_t options );

extern uint8_t RTG_ProcessRtDiscBits( uint8_t rtDiscFlag, uint16_t dstAddress, uint8_t* pSrcRtgSet, uint8_t options );

extern uint8_t RTG_RouteMaintanence( uint16_t DstAddress, uint16_t SrcAddress, uint8_t options );

extern void RTG_FillCSkipTable( byte *children, byte *routers,
                                byte depth, uint16_t *pTbl );


extern void RTG_SendBrokenRoute( uint16_t nwkSrcAddr, uint16_t nwkDstAddr,
                   uint8_t srcRouteSet, uint16_t macSrcAddr, uint16_t macDstAddr );

extern uint16_t RTG_CalcTreeAddress(  byte deviceType );

extern uint16_t RTG_GetStochastic( byte deviceType );

extern uint16_t RTG_GetNextTreeHop( uint16_t dstAddress );

extern uint16_t RTG_ChildGetNextHop( uint16_t DstAddr );

extern uint8_t RTG_GetAncestors( uint16_t dstAddr, uint16_t ancestorAddr, uint16_t *pRtgDst );

extern void RTG_nextHopIsBad( uint16_t nextHop );

extern ZStatus_t RTG_SendRErr( uint16_t SrcAddress, uint16_t DstAddress, byte ErrorCode );

extern RTG_Status_t RTG_AddSrcRtgEntry_Guaranteed( uint16_t srcAddr, uint8_t relayCnt, uint16_t* pRelayList );

extern void RTG_initRtgTable( void );

extern void RTG_MTORouteReq(void);
extern void RTG_SendNextEDMsg( uint16_t shortAddr, uint8_t sendBroadcast, uint8_t sendNoData );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* RTG_H */
