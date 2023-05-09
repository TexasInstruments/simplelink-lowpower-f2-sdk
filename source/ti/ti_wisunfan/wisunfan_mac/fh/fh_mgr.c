/******************************************************************************

 @file fh_mgr.c

 @brief TIMAC 2.0 FH manager API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2023, Texas Instruments Incorporated

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

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include "hal_types.h"

#include "mac_low_level.h"
#include "mac_radio.h"
#include "mac_rx_onoff.h"
#include "mac_main.h"
#include "fh_api.h"
#include "fh_ie.h"
#include "fh_data.h"
#include "fh_pib.h"
#include "fh_dh1cf.h"
#include "fh_sm.h"
#include "fh_mgr.h"
#include "fh_util.h"
#include "mac_pib.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE fh_mgr_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#ifdef FEATURE_FREQ_HOP_MODE

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

void FHMGR_macStartOneShotTimer(FH_macFHTimer_t *pTimer)
{
    halIntState_t s;

    HAL_ENTER_CRITICAL_SECTION(s);

    MAP_ICall_setTimer((uint_fast32_t)pTimer->duration, (MAP_ICall_TimerCback)pTimer->pFunc,
                   NULL, (MAP_ICall_TimerID*)&pTimer->timerId);
    HAL_EXIT_CRITICAL_SECTION(s);
}


void FHMGR_macStartFHTimer(FH_macFHTimer_t *pTimer, bool dedicated)
{
    uint32_t  nowTick, compTick;
    halIntState_t s;

    HAL_ENTER_CRITICAL_SECTION(s);

    nowTick = MAP_ICall_getTicks();
    /*compensate for hwi/swi latency */
    if(pTimer->compTime)
    {
        compTick = nowTick - pTimer->clockTickCnt;
        compTick -= pTimer->prevTimerDuration;
        pTimer->duration -= compTick;
    }

    pTimer->clockTickCnt = nowTick;

    MAP_ICall_setTimer((uint_fast32_t)pTimer->duration, (MAP_ICall_TimerCback)pTimer->pFunc,
                   NULL, (MAP_ICall_TimerID*)&pTimer->timerId);
    pTimer->prevTimerDuration = pTimer->duration;
    /*compensate for interrupt delay */
    pTimer->compTime = 1;
    HAL_EXIT_CRITICAL_SECTION(s);
}

uint32_t FHMGR_macGetElapsedFHTime(FH_macFHTimer_t *pTimer)
{
    uint32_t temp32;

    /*wrap around? */
    temp32 = (uint32_t) MAP_ICall_getTicks();

    return ( temp32 - pTimer->clockTickCnt);
}


/**************************************************************************************************
 * @fn          FHMGR_ucTimerIIsrCb
 *
 * @brief       This function is called on UC timer expiry.
 *
 * input parameters
 *
 * @param       - None
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 *************************************************************************************************
 */
void FHMGR_ucTimerIsrCb(uint8_t parameter)
{
    if(!pMacPib->rxOnWhenIdle)
    {
        /*For sleepy device, do not start timer, in case it got started during JOIN process */
        return;
    }

    FH_hnd.ucTimer.duration = (FHPIB_db.macUcDwellInterval * TICKPERIOD_MS_US);
    /*Re-start Unicast Timer */
    MAP_FHMGR_macStartFHTimer(&FH_hnd.ucTimer, TRUE);

    /*Increment he slot number */
    /*Rolls over evry 2^16 slots */
    FH_hnd.ucSlotIdx += 1;

    /*ufsi is cumulative for all slots */
    FH_hnd.ufsi += FHPIB_db.macUcDwellInterval;

    /* roll over for slot # and ufsi */
    if(!FH_hnd.ucSlotIdx)
        FH_hnd.ufsi	 = 0;

    MAP_FHSM_event(&FH_FSM, FH_EVT_UC_TIMER, &FH_hnd);

    DBG_PRINTL2(DBGSYS, "FHTraceLog: ucTimerIsr slotNo(0x%X) usfi(0x%0x)", FH_hnd.ucSlotIdx,FH_hnd.ufsi);
}

/**************************************************************************************************
 * @fn          FHMGR_bcTimerIIsrCb
 *
 * @brief       This function is called on BC timer interrupt.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 *************************************************************************************************
 */
extern MAC_INTERNAL_API void macSymbolBCTimerPowerUp(void);

void FHMGR_BCTimerEventUpd(void)
{
    uint16_t bc_event;

    bc_event = FH_EVT_BC_TIMER;
    MAP_FHSM_event(&FH_FSM, bc_event, &FH_hnd);
#ifdef MACRADIODBG
    macDbgRadioState[macDbgCntr++] = (macRadioYielded << 16) | 10;
    macDbgRadioState[macDbgCntr++] = FH_hnd.bfio;
    macDbgRadioState[macDbgCntr++] = FH_hnd.bcSlotIdx;
    macDbgRadioState[macDbgCntr++] = macPhyChannel;
    macDbgCntr &= 0x1FF;
#endif

    DBG_PRINT3(DBGSYS, "FHTraceLog: bcTimerIsr slotNo(0x%X) bfio(0x%0x) bcDwellActive(%d)", FH_hnd.bcSlotIdx,FH_hnd.bfio,FH_hnd.bcDwellActive);
}

#if !defined(TIMAC_ROM_PATCH)
void FHMGR_bcTimerIsrCb(uint8_t parameter)
{
    uint8_t macBcDwellTime;
    uint32_t macBcInterval, temp32;
    uint16_t bc_event;

#ifdef MACRADIODBG
  macDbgRadioState[macDbgCntr++] = 9;
  macDbgRadioState[macDbgCntr++] = FH_hnd.bcDwellActive;
  macDbgRadioState[macDbgCntr++] = 0;
  macDbgRadioState[macDbgCntr++] = 0;
  macDbgCntr &= 0x1FF;
#endif
    MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);

    if(!macBcDwellTime)
    {
    	/*For sleepy device, do not start timer, in case it got started during JOIN process */
    	return;
    }

    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &macBcInterval);
    if(!FH_hnd.bcDwellActive)
    {
        FH_hnd.bcTimer.duration = (macBcDwellTime * TICKPERIOD_MS_US);
        MAP_FHMGR_macStartFHTimer(&FH_hnd.bcTimer, TRUE);

        FH_hnd.bcSlotIdx += 1;

        FH_hnd.bfio = 0;

        FH_hnd.bcDwellActive = 1;

        MAP_macRxSoftEnable(MAC_RX_FH_BC_SLOT);
        bc_event = FH_EVT_BC_TIMER;
    }
    else
    {
        MAP_macRxSoftDisable(MAC_RX_FH_BC_SLOT);
        FH_hnd.bcDwellActive = 0;

        FH_hnd.bfio = macBcDwellTime;

        FH_hnd.bcTimer.duration = (macBcInterval - macBcDwellTime) * TICKPERIOD_MS_US;

        MAP_FHMGR_macStartFHTimer(&FH_hnd.bcTimer, TRUE);

        if(!pMacPib->rxOnWhenIdle)
        {
            if(FH_hnd.pktPending)
            {
                /* set up next data transmission */
                MAP_macSetEvent(MAC_TX_COMPLETE_TASK_EVT);
            }
            else
            {
                /* Another Unicast TX might have started to power up radio*/
                if(!macRadioYielded)
                {
                    MAP_macRxOffRequest();
                }
            }
            return;
        }

       /*Handle corner cases where current UC slot ends and next UC slot starts
        * within 2 ms (to account for radio synthesizer and RX ON delay)
        * Work around for Back to back Flush/stop commands causing a problem,
        * in that only the first stop command works.
        */
        MAP_FHUTIL_getCurUfsi(&temp32);

        temp32 = (temp32 % FHPIB_db.macUcDwellInterval);

        if(temp32 < ( FHPIB_db.macUcDwellInterval - FH_UC_DWELLTIME_BUF))
        {
            bc_event = FH_EVT_UC_TIMER;
        }
        else
        {
            DBG_PRINT3(DBGSYS, "FHTraceLog: skipping UC, bcTimerIsr slotNo(0x%X) bfio(0x%0x) bcDwellActive(%d)", FH_hnd.bcSlotIdx,FH_hnd.bfio,FH_hnd.bcDwellActive);
            return;
        }
    }

    if(pMacPib->rxOnWhenIdle)
    {
        MAP_FHSM_event(&FH_FSM, bc_event, &FH_hnd);
    }
    else
    {
        MAP_macSymbolBCTimerPowerUp();
    }
}

/**************************************************************************************************
 * @fn          FHMGR_updateRadioUCChannel
 *
 * @brief       This function changes the unicast channel.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 *************************************************************************************************
 */
void FHMGR_updateRadioUCChannel(void *pData)
{
    uint8_t channel, chFunc;
    FH_HND_s *pFH_hnd = (FH_HND_s *)pData;

    MAP_FHPIB_get(FHPIB_UC_CHANNEL_FUNCTION, &chFunc);

    if(chFunc == FHIE_CF_SINGLE_CHANNEL)
    {
        uint16_t ucFixedChannel;
        MAP_FHPIB_get(FHPIB_UC_FIXED_CHANNEL, &ucFixedChannel);
        channel = ucFixedChannel & 0xFF;
    }
    else if(chFunc == FHIE_CF_DH1CF)
    {
        channel = MAP_FHDH1CF_getChannelNum(pFH_hnd);
    }
    else
    {
        return;
    }

    if(!pFH_hnd->bcDwellActive)
    {
        MAP_macRadioSetChannel(channel);
        pFH_hnd->lastChannel = channel;
        pFH_hnd->pendUcChHop = 0;
        DBG_PRINTL2(DBGSYS, "FHTraceLog: UnicastStart: RTCTime(0x%X) slotNo(0x%0x)", (uint32_t) MAP_ICall_getTicks(),FH_hnd.ucSlotIdx);
        /* 2. check UC pend packet*/
        if(pFH_hnd->pktPending)
        {
            /* set up next data transmission */
            MAP_macSetEvent(MAC_TX_COMPLETE_TASK_EVT);
        }
    }
    else
    {
        pFH_hnd->pendUcChHop = 1;
        DBG_PRINTL1(DBGSYS, "FHTraceLog: UnicastPend: RTCTime(0x%X) at bcDwellActive", (uint32_t) MAP_ICall_getTicks());
    }
}
#endif

/**************************************************************************************************
 * @fn          FHMGR_updateRadioBCChannel
 *
 * @brief       This function changes the broadcast channel.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 *************************************************************************************************
 */
#ifdef BCPKTDBG
struct {
    uint32_t yield;
    uint32_t bfio;
    uint32_t bfio1;
    uint32_t slotidx;
    uint32_t slotidx1;
    uint32_t channel;
    uint32_t channel1;
}macDbgBcSlot[32];
uint32 macDbgCntr = 31;
#endif
void FHMGR_updateRadioBCChannel(void *pData)
{
    uint8_t channel, chFunc;
    FH_HND_s *pFH_hnd = (FH_HND_s *)pData;

    MAP_FHPIB_get(FHPIB_BC_CHANNEL_FUNCTION, &chFunc);

    /* 1. update channel */
    if(chFunc == FHIE_CF_SINGLE_CHANNEL)
    {
        uint16_t bcFixedChannel;
        MAP_FHPIB_get(FHPIB_BC_FIXED_CHANNEL, &bcFixedChannel);
        channel = bcFixedChannel & 0xFF;
    }
    else if(chFunc == FHIE_CF_DH1CF)
    {
        channel = MAP_FHDH1CF_getBCChannelNum(pFH_hnd->bcSlotIdx, pFH_hnd->bcNumChannels);
    }
    else
    {
        return;
    }

    MAP_macRadioSetChannelRx(channel);
    pFH_hnd->lastChannel = channel;
    pFH_hnd->pendBcChHop = 0;
    DBG_PRINTL2(DBGSYS, "FHTraceLog: broadcastStart: bfio=%d slotNo(%d)", FH_hnd.bfio,FH_hnd.bcSlotIdx);

#ifdef BCPKTDBG
    uint32_t bfio;
    uint16_t slotIdx;
    MAP_FHUTIL_getCurBfio(&bfio, &slotIdx);
    macDbgCntr++;
    macDbgCntr &= 0x1F;
    macDbgBcSlot[macDbgCntr].yield = (macRadioYielded << 16) | 10;
    macDbgBcSlot[macDbgCntr].bfio = pFH_hnd->bfio;
    macDbgBcSlot[macDbgCntr].bfio1 = bfio;
    macDbgBcSlot[macDbgCntr].slotidx = pFH_hnd->bcSlotIdx;
    macDbgBcSlot[macDbgCntr].slotidx1 = slotIdx;
    macDbgBcSlot[macDbgCntr].channel = macPhyChannel;
    macDbgBcSlot[macDbgCntr].channel1 = pFH_hnd->lastChannel;
#endif

    /* 2. check pend packet*/
    if((pFH_hnd->bcPktPending))
    {
        /* set up next data transmission */
        MAP_macSetEvent(MAC_TX_COMPLETE_TASK_EVT);
    }
}

void FHMGR_pendUcChUpdate(void *pData)
{
    FH_HND_s *pFH_hnd = (FH_HND_s *)pData;

    pFH_hnd->pendUcChHop = 1;

    DBG_PRINT0(DBGSYS, "UC pend update");
}

void FHMGR_macCancelFHTimer(FH_macFHTimer_t *pTimer)
{
    MAP_ICall_stopTimer((MAP_ICall_TimerID*)pTimer->timerId);
    /*no delay compensation first time */
    pTimer->compTime = 0;
}

void FHMGR_pendBcChUpdate(void *pData)
{
    FH_HND_s *pFH_hnd = (FH_HND_s *)pData;

    pFH_hnd->pendBcChHop = 1;

    DBG_PRINT0(DBGSYS, "BC pend update");
}

void FHMGR_updateHopping(void *pData)
{
    FH_HND_s *pFH_hnd = (FH_HND_s *)pData;

    DBG_PRINTL2(DBGSYS, "FHTraceLog: updateHopping: bcUpd(0x%X) ucUpd(0x%X)", pFH_hnd->pendBcChHop, pFH_hnd->pendUcChHop);

    /* for sleepy nodes, the channel has to be set to its
     * intended receive channel as set by PIB */
    if(!pMacPib->rxOnWhenIdle)
    {
        return;
    }

    if(pFH_hnd->pendBcChHop)
    {
        MAP_FHMGR_updateRadioBCChannel(pData);
        pFH_hnd->pendBcChHop = 0;
    }
    else if((pFH_hnd->pendUcChHop) && (!pFH_hnd->bcDwellActive))
    {
        MAP_FHMGR_updateRadioUCChannel(pData);
        pFH_hnd->pendUcChHop = 0;
    }
    else
    {
        MAP_macRadioSetChannel(pFH_hnd->lastChannel);
        DBG_PRINTL2(DBGSYS, "FHTraceLog: RxResume: RTCTime(0x%X) Channel(0x%X)", (uint32_t) MAP_ICall_getTicks(), pFH_hnd->lastChannel);
    }
}

#endif
