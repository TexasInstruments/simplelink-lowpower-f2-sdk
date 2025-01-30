/******************************************************************************

 @file  mac_timer.h

 @brief Timer interface for high level MAC.

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

#ifndef MAC_TIMER_H
#define MAC_TIMER_H

/* ------------------------------------------------------------------------------------------------
 *                                               Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                               Macros
 * ------------------------------------------------------------------------------------------------
 */

#define MAC_BEACON_INTERVAL(x)        (((uint32) MAC_A_BASE_SUPERFRAME_DURATION) << (x))
#define MAC_SF_INTERVAL(x)            MAC_BEACON_INTERVAL(x)

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct macTimer_s
{
  struct macTimer_s     *pNext;                     /* next timer in queue */
  int32                 symbol;                     /* timer expiration count */
  void                  (*pFunc)(uint8 parameter);  /* timer callback function */
  uint8                 parameter;                  /* callback function parameter */
} macTimer_t;

typedef struct macTimerHeader_s
{
  struct macTimer_s     * pNext;
} macTimerHeader_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Globals
 * ------------------------------------------------------------------------------------------------
 */
extern macTimer_t *macTimerActive;

/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macTimerInit(void);
MAC_INTERNAL_API void macTimer(macTimer_t *pTimer, uint32 backoffs);
MAC_INTERNAL_API void macTimerCancel(macTimer_t *pTimer);
MAC_INTERNAL_API void macTimerStart(uint32 initTime, uint8 beaconOrder);
MAC_INTERNAL_API uint32 macTimerGetTime(void);
MAC_INTERNAL_API void macTxBeaconFrame(void);
MAC_INTERNAL_API void macTimerAligned(macTimer_t *pTimer, uint32 backoffs);
MAC_INTERNAL_API void macTimerRealign(macRx_t *pRxBeacon, uint8 beaconOrder);
MAC_INTERNAL_API void macTimerSyncRollover(uint8 beaconOrder);
MAC_INTERNAL_API void macTimerSetRollover(uint8 beaconOrder);
MAC_INTERNAL_API bool macTimerCheckUnAlignedQ(macTimer_t *pTimer);
MAC_INTERNAL_API void macTimerUpdActive(macTimer_t *pTimer);
#endif /* MAC_TIMER_H */

