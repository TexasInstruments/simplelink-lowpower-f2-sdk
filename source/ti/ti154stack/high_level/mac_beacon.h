/******************************************************************************

 @file  mac_beacon.h

 @brief Interface to procedures for beacon enabled networks common to both
        device and coordinator.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2025, Texas Instruments Incorporated

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

#ifndef MAC_BEACON_H
#define MAC_BEACON_H

/* ------------------------------------------------------------------------------------------------
 *                                              Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                               Macros
 * ------------------------------------------------------------------------------------------------
 */

/* beacon sync and tracking states */
#define MAC_BEACON_NONE_ST            0   /* no tracking or sync in progress */
#define MAC_BEACON_SYNC_ST            1   /* trying to sync */
#define MAC_BEACON_TRACKING_ST        2   /* tracking successful */

/* special trackBeacon value for internally generated sync request for tx */
#define MAC_TRACK_BEACON_TX_SYNC      0xFF

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct
{
  macTimer_t      periodTimer;      /* timer set to expire at the end of the CAP */
  uint32          capStart;         /* start of CAP in backoffs units */
  uint32          capEnd;           /* end of CAP in backoff units */
  uint8           sched;            /* current superframe schedule */
  uint8           finalCapSlot;     /* final CAP slot of current superframe */
  bool            battLifeExt;      /* battery life extension setting of current superframe */
  uint8           state;            /* beacon track/sync state */
  bool            txSync;           /* TRUE if beacon sync was internally generated */
  bool            rxWindow;         /* TRUE if ready to receive tracked beacon */
  uint8           coordBeaconOrder; /* Beacon order received in coordinator's beacon */
} macBeacon_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macBeaconInit(void);
MAC_INTERNAL_API void macBeaconReset(void);
MAC_INTERNAL_API void macBeaconSetSched(macTx_t *pMsg);
MAC_INTERNAL_API uint8 macBeaconCheckSched(void);
MAC_INTERNAL_API void macBeaconRequeue(macTx_t *pMsg);
MAC_INTERNAL_API uint16 macBeaconCheckTxTime(void);
MAC_INTERNAL_API void macBeaconSetupCap(uint8 sched, uint8 superframeOrder, uint16 beaconLen);
MAC_INTERNAL_API void macBeaconRetransmit(void);

#endif /* MAC_BEACON_H */
