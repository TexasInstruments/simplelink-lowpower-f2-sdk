/******************************************************************************

 @file fh_mgr.h

 @brief TIMAC 2.0 FH manager API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated

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

/******************************************************************************
 Includes
 *****************************************************************************/

#ifndef FH_MGR_H
#define FH_MGR_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "fh_pib.h"
#include "fh_data.h"
#include "mac_low_level.h"

/*! Clock tick period  */
#define CLOCK_TICK_PERIOD     (10)
/*! tick number for one ms  */
#define TICKPERIOD_MS_US      (1000/(CLOCK_TICK_PERIOD))

#define FH_UC_DWELLTIME_BUF		(2)
/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/*!
 * @brief       start the FH one shot timer
 *
 * @param pTimer - pointer to FH timer object. It can be either broadcast or unicast
 *

 */
void FHMGR_macStartOneShotTimer(FH_macFHTimer_t *pTimer);

/*!
 * @brief       start the FH timer
 *
 * @param pTimer - pointer to FH timer object. It can be either broadcast or unicast
 * @param dedicated - dedicated RAT timer (yes or no)
 *

 */
void FHMGR_macStartFHTimer(FH_macFHTimer_t *pTimer, bool dedicated);

/*!
 * @brief       stop the FH timer
 *
 * @param pTimer - pointer to FH timer object. It can be either broadcast or unicast
 *

 */
void FHMGR_macCancelFHTimer(FH_macFHTimer_t *pTimer);

/*!
 * @brief       get the elapsed time since the timer is started.
 *
 * @param pTimer - pointer to FH timer object. It can be either broadcast or unicast
 *

 */
uint32_t FHMGR_macGetElapsedFHTime(FH_macFHTimer_t *pTimer);

/*!
 * @brief       broadcast timer callback function.
 *
 * @param parameter - parameter passed by the ISR
 *
 */
void FHMGR_bcTimerIsrCb(uint8_t parameter);

/*!
 * @brief       unicast timer callback function.
 *
 * @param parameter - parameter passed by the ISR
 *
 */
void FHMGR_ucTimerIsrCb(uint8_t parameter);

/*!
 * @brief       update the Radio BC channel
 *
 * @param pData - pointer to FH handler
 *
 */
void FHMGR_updateRadioBCChannel(void *pData);

/*!
 * @brief       update the Radio UC channel
 *
 * @param pData - pointer to FH handler
 *
 */
void FHMGR_updateRadioUCChannel(void *pData);

/*!
 * @brief       update the UCchannel data channel pending flag
 *
 * @param pData - pointer to FH handler
 *
 */
void FHMGR_pendUcChUpdate(void *pData);

/*!
 * @brief       update the BC channel data pending flag
 *
 * @param pData - pointer to FH handler
 *
 */
void FHMGR_pendBcChUpdate(void *pData);

/*!
 * @brief       during hopping state, update the channel and packet
 *
 * @param pData - pointer to FH handler
 *
 */
void FHMGR_updateHopping(void *pData);


void FHMGR_BCTimerEventUpd(void);

#endif

