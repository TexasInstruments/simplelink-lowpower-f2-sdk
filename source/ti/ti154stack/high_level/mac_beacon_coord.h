/******************************************************************************

 @file  mac_beacon_coord.h

 @brief Interface to coordinator only procedures for beacon enabled networks.

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

#ifndef MAC_BEACON_COORD_H
#define MAC_BEACON_COORD_H

/* ------------------------------------------------------------------------------------------------
 *                                              Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Time before beacon transmission to prepare beacon, in symbols */
#define MAC_BEACON_PREPARE_OFFSET     64

/* The timer alignment is initialized using this offset when a network is started, in symbols */
#define MAC_BEACON_START_OFFSET       32

/* Extra time to allow for battery life extension to work, in symbols */
#define MAC_BEACON_BATT_LIFE_OFFSET   32

/* Time before enhanced beacon transmission to prepare beacon, in symbols */
#define MAC_EBEACON_PREPARE_OFFSET    160

#define MAC_EBEACON_NOT_STARTED       0x01  /* The enhanced beacon is prepared */
#define MAC_EBEACON_PREPARED          0x02  /* The enhanced beacon is prepared */
#define MAC_EBEACON_QUEUED            0x04  /* The cmd to tx beacon has been queued */
#define MAC_EBEACON_TX                0x08  /* The enhanced beacon has been tx */

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct
{
  macTimer_t      txTimer;            /* timer for beacon transmission */
  macTimer_t      prepareTimer;       /* timer for beacon preparation */
  macTimer_t      battLifeTimer;      /* timer for battery life extension */
  uint32          prepareTime;        /* beacon preparation time in superframe */
  macTx_t         *pBeacon;           /* pointer to beacon to be transmitted */
#ifdef FEATURE_ENHANCED_BEACON
  macTimer_t      eBeaconTxTimer;     /* timer for enhanced beacon transmission */
  macTimer_t      eBeaconPrepareTimer;/* timer for enhanced beacon transmission */
  uint32          eBeaconPrepareTime; /* ebeacon preparation time */
#endif /* FEATURE_ENHANCED_BEACON */
  uint8           origPhyId;          /* original phy Id */
  uint8           origSched;          /* original superframe schedule */
  uint8           eBeaconStatus;      /* Flags indicating enhanced beacon status */

} macBeaconCoord_t;


/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macApiBeaconStartReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macBeaconStartContinue(macEvent_t *pEvent);
MAC_INTERNAL_API void macBeaconSetupBroadcast(void);
MAC_INTERNAL_API void macOutgoingNonSlottedTx(void);
MAC_INTERNAL_API void macBeaconSchedRequested(void);
MAC_INTERNAL_API void macBeaconClearIndirect(void);
MAC_INTERNAL_API void macTxBeaconCompleteCallback(uint8 status);
MAC_INTERNAL_API void macBeaconPrepareCallback(uint8 param);

#endif /* MAC_BEACON_COORD_H */
