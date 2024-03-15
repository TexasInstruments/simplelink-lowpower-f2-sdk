/**************************************************************************************************
  Filename:       zd_nwk_mgr.h
  Revised:        $Date: 2007-07-18 10:02:49 -0700 (Wed, 18 Jul 2007) $
  Revision:       $Revision: 14945 $

  Description:    This file contains the interface to the ZigBee Network Manager.


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

#ifndef ZDNWKMGR_H
#define ZDNWKMGR_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "nwk_globals.h"
#include "nwk_util.h"
#include "zd_app.h"

/******************************************************************************
 * CONSTANTS
 */

// Network Manager Role
#define ZDNWKMGR_DISABLE                  0x00
#define ZDNWKMGR_ENABLE                   0x01

// Energy level threshold
#define ZDNWKMGR_ACCEPTABLE_ENERGY_LEVEL  0x1E

// Minimum transmissions attempted for Channel Interference detection
#if !defined ( ZDNWKMGR_MIN_TRANSMISSIONS )
  #define ZDNWKMGR_MIN_TRANSMISSIONS      20
#endif

// Minimum transmit failure rate for Channel Interference detection
#define ZDNWKMGR_CI_TX_FAILURE            25

// Minimum transmit failure rate for Channel Change
#define ZDNWKMGR_CC_TX_FAILURE            50

// Min and Max Scan Counts for Update Request
#define ZDNWKMGR_MIN_SCAN_COUNT           0
#define ZDNWKMGR_MAX_SCAN_COUNT           5

// Update Request and Notify timers
#define ZDNWKMGR_UPDATE_NOTIFY_TIMER      60  // 1(h) * 60(m)
#define ZDNWKMGR_UPDATE_REQUEST_TIMER     60  // 1(h) * 60(m)

// Network Manager Events
#define ZDNWKMGR_CHANNEL_CHANGE_EVT       0x0001
#define ZDNWKMGR_UPDATE_NOTIFY_EVT        0x0002
#define ZDNWKMGR_UPDATE_REQUEST_EVT       0x0004
#define ZDNWKMGR_SCAN_REQUEST_EVT         0x0008

#define ZDNWKMGR_BCAST_DELIVERY_TIME      ( _NIB.BroadcastDeliveryTime * 100 )

/*********************************************************************
 * TYPEDEFS
 */

// Used for Management Network Update Request message
typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t transSeq;
  uint16_t srcAddr;
  uint32_t channelMask;
  uint8_t scanDuration;
  uint8_t scanCount;
  int16_t nwkManagerAddr;
  uint8_t wasBroadcast;
} ZDNwkMgr_MgmtNwkUpdateRequest_t;

// Used for Management Network Update Notify command
typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t srcAddr;
  uint8_t status;
  uint32_t scannedChannels;
  uint16_t totalTransmissions;
  uint16_t txFailures;
  uint8_t listCount;
  uint8_t *energyValues;
} ZDNwkMgr_MgmtNwkUpdateNotify_t;

// Used for Channel Interference message
typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t totalTransmissions;
  uint16_t txFailures;
} ZDNwkMgr_ChanInterference_t;

// Used for ED Scan Confirm message
typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t status;
  uint32_t scannedChannels;
  uint8_t energyDetectList[ED_SCAN_MAXCHANNELS];
} ZDNwkMgr_EDScanConfirm_t;

// Used for Network Report command
typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t srcAddr;
  uint8_t  reportType;
  uint8_t  EPID[Z_EXTADDR_LEN];
  uint8_t  reportInfoCnt;
  uint16_t panIDs[];
} ZDNwkMgr_NetworkReport_t;

// Used for Network Update command
typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t  updateType;
  uint8_t  updateInfoCnt;
  uint16_t newPanID;
} ZDNwkMgr_NetworkUpdate_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern byte ZDNwkMgr_TaskID;

/******************************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * Task Level Control
 */
/*
 * ZDNwkMgr Task Initialization Function
 */
extern void ZDNwkMgr_Init( byte task_id );

/*
 * ZDNwkMgr Task Event Processing Function
 */
extern uint32_t ZDNwkMgr_event_loop( byte task_id, uint32_t events );

/*********************************************************************
 * Application Level Functions
 */
// Frequency Agility functions
extern void (*pZDNwkMgr_EDScanConfirmCB)( NLME_EDScanConfirm_t *EDScanConfirm );
extern void (*pZDNwkMgr_ProcessDataConfirm)( afDataConfirm_t *afDataConfirm );
extern void (*pZDNwkMgr_ReportChannelInterference)( NLME_ChanInterference_t *chanInterference );

// PAN ID Conflict functions
extern void (*pZDNwkMgr_NetworkReportCB)( ZDNwkMgr_NetworkReport_t *pReport );
extern void (*pZDNwkMgr_NetworkUpdateCB)( ZDNwkMgr_NetworkUpdate_t *pUpdate );

#if defined ( NWK_MANAGER )
/*
 * ZDNwkMgr set the local device as the Network Manager
 */
extern void NwkMgr_SetNwkManager( void );
#endif

/******************************************************************************
******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDNWKMGR_H */
