/**************************************************************************************************
  Filename:       APS.h
  Revised:        $Date: 2015-06-02 15:55:43 -0700 (Tue, 02 Jun 2015) $
  Revision:       $Revision: 43961 $

  Description:    Primitives of the Application Support Sub Layer Task functions.


  Copyright 2004-2015 Texas Instruments Incorporated.

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

#ifndef APS_H
#define APS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "aps_mede.h"
#include "binding_table.h"
#include "reflecttrack.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint16_t             nwkSrcAddr;
  uint8_t              nwkSecure;
  aps_FrameFormat_t* aff;
} APS_CmdInd_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  zAddrType_t SrcAddress;
  uint16_t SrcPanId;
  uint16_t macDestAddr;
  NLDE_Signal_t sig;
  byte SecurityUse;
  uint32_t timestamp;
  uint8_t nwkSeqNum;
  uint8_t radius;
  aps_FrameFormat_t aff;
} apsInMsg_t;

typedef void (*pfnBindingTimeoutCB)( void );

typedef struct
{
  uint16_t srcAddr;
  uint8_t  broadcastId;
  uint16_t expiryTime;
} apsEndDeviceBroadcast_t;

/*********************************************************************
 * CONSTANTS
 */

// APS Command IDs - sent messages
#define APS_INCOMING_MSG 0x01
#define APS_CMD_PKT      0x02

// APS Message Fields
#define APS_MSG_ID     0x00
#define APS_MSG_ID_LEN 0x01

// APS Command Messages
#define APS_CMD_PKT_HDR ((uint8_t)                    \
                         (sizeof(APSME_CmdPkt_t) +  \
                          APS_MSG_ID_LEN          ))

// APS Events
#define APS_EDBIND_TIMEOUT_TIMER_ID 0x0001
#define APS_DATA_REQUEST_HOLD       0x0002
#define APS_ENDDEVICE_BROADCAST_EVT 0x0004
#define APS_DUP_REJ_TIMEOUT_EVT     0x0008

// APS Relector Types
#define APS_REFLECTOR_PUBLIC  0
#define APS_REFLECTOR_PRIVATE 1

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t APS_Counter;
extern byte APS_TaskID;
extern uint16_t AIB_MaxBindingTime;
extern uint8_t  AIB_apsUseExtendedPANID[Z_EXTADDR_LEN];
extern uint8_t  AIB_apsUseInsecureJoin;
extern uint8_t *AIB_apsTrustCenterAddress;
extern uint8_t APS_AllowDataRequests;
extern uint32_t APS_lastDupTime;


#if ( ZG_BUILD_ENDDEVICE_TYPE ) && ( RFD_RX_ALWAYS_ON_CAPABLE == TRUE )
  extern apsEndDeviceBroadcast_t apsEndDeviceBroadcastTable[];
#endif

/*********************************************************************
 * APS System Functions
 */

/*
 * Initialization function for the APS layer.
 */
extern void APS_Init( byte task_id );

/*
 * Event Loop Processor for APS.
 */
extern uint32_t APS_event_loop( byte task_id, uint32_t events );

/*
 * Setup the End Device Bind Timeout
 */
extern void APS_SetEndDeviceBindTimeout( uint16_t timeout, pfnBindingTimeoutCB pfnCB );

/*
 * APS Command Indication
 */
extern void APS_CmdInd( APS_CmdInd_t* ind );

/*
 * APS Reflector Initiatialization
 *   APS_REFLECTOR_PUBLIC
 *   APS_REFLECTOR_PRIVATE
 */
extern void APS_ReflectorInit( void );

#if ( ZG_BUILD_ENDDEVICE_TYPE ) && ( RFD_RX_ALWAYS_ON_CAPABLE == TRUE )
  /*
   * APS_InitEndDeviceBroadcastTable - Initialize the End Device Broadcast table
   */
  extern void APS_InitEndDeviceBroadcastTable( void );

  /*
   * APS_EndDeviceBroadcastCheck
   */
  extern uint8_t APS_EndDeviceBroadcastCheck( NLDE_FrameFormat_t *ff );
#endif // ( ZG_BUILD_ENDDEVICE_TYPE ) && ( RFD_RX_ALWAYS_ON_CAPABLE == TRUE )


/*********************************************************************
 * REFLECTOR FUNCTION POINTERS
 */

extern ZStatus_t (*pAPS_UnBind)( byte SrcEndpInt,
                            uint16_t ClusterId, zAddrType_t *DstAddr, byte DstEndpInt );

/*
 * Fill in pItem w/ info from the Nth valid binding table entry.
 */
extern ZStatus_t (*pAPS_GetBind)( uint16_t Nth, apsBindingItem_t *pItem );

extern uint8_t (*pAPS_DataConfirmReflect)( nwkDB_t *rec, uint8_t status );

extern void (*pAPS_DataIndReflect)( zAddrType_t *SrcAddress, aps_FrameFormat_t *aff,
                 NLDE_Signal_t *sig, uint8_t AckRequest, uint8_t discoverRoute,
                 uint8_t SecurityUse, uint32_t timestamp,
                 uint16_t txOptions, uint8_t apsRetries );

/****************************************************************************
****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* APS_H */


