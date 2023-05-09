/******************************************************************************

 @file fh_util.c

 @brief TIMAC 2.0 FH utility API

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
#include "fh_api.h"
#include "fh_pib.h"
#include "mac_pib.h"
#include "fh_ie.h"
#include "fh_util.h"
#include "fh_data.h"
#include "fh_dh1cf.h"
#include "fh_nt.h"
#include "fh_sm.h"
#include "fh_mgr.h"
#include "mac_assert.h"
#include "mac_mgmt.h"
#include "mac_hl_patch.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE fh_util_c
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

/* ----------------------------------------------------------------------------
 *                                           Constants
 * ----------------------------------------------------------------------------
 */

/* ----------------------------------------------------------------------------
 *                                           Local Variables
 * ----------------------------------------------------------------------------
 */

 /* ----------------------------------------------------------------------------
 *                                           Global Variables
 * ----------------------------------------------------------------------------
 */

 /* ----------------------------------------------------------------------------
 *                                           Local Prototypes
 * ----------------------------------------------------------------------------
 */
uint32_t FHUTIL_compBfioTxTime(void);
uint8_t FHUTIL_getUcChannel(NODE_ENTRY_s *pEntry, uint32_t txUfsi);


/*!
 * @brief       This function gets the current ufsi from the specific entry
 *              neighbor table and updates the ufsi to the present time
 *
 * @param       pEntry - pointer to neighbor table entry
 */
uint32_t FHUTIL_getTxUfsi(NODE_ENTRY_s *pEntry)
{
    uint32_t timeLimit, txUfsi;
    uint8_t ntDwellTime;
#if 0
    /* ufsi = reftsmp-nUfsi + getCurTstmp */
    MAP_FHUTIL_getCurUfsi((uint32_t *)&txUfsi);
    txUfsi  -= pEntry->ref_timeStamp;
#else
    txUfsi = MAP_FHUTIL_getElapsedTime(pEntry->ref_timeStamp);
#endif

    /* txUfsi  += pEntry->ufsi; */
    ntDwellTime = pEntry->UsieParams_s.dwellInterval;

    txUfsi += MAP_FHUTIL_calcUfsi(pEntry->ufsi, ntDwellTime);

    timeLimit = (FH_USCH_NUM_SLOTS * (ntDwellTime));

/*    if(txUfsi  < 0)
        txUfsi  += timeLimit; */

    if(txUfsi   >= timeLimit)
        txUfsi  -= timeLimit;

    return(txUfsi);
}


/*!
 * This function is called by low level to update backoffDuration
 *
 * Public function defined in fh_util.h
 */
FHAPI_status FHUTIL_adjBackOffDur(NODE_ENTRY_s *pEntry, uint32_t *pBackoffDur,
                          uint32_t *txUfsi)
{
    uint32_t posInSlot, posInBcSlot, bfioTemp, txDiMicro;
    uint16_t delay1, slotIdx;
    int32_t remTime, remBceTime, remBcfTime, slotEdgeTime;
    uint32_t guardTime;
    FHAPI_status status = FHAPI_STATUS_SUCCESS;

    // RPL messages are sent before PC in Wi-SUN
    if((FHPIB_db.macBcDwellInterval) && (FHPIB_db.macBcDwellInterval) && FH_hnd.bsStarted)
    {
        slotEdgeTime = FH_SLOT_EDGE_PROTECTION_TIME;

        if(MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_SLR_5K)
        {
            slotEdgeTime = FH_LRM_SLOT_EDGE_PROTECTION_TIME;
        }

        MAP_FHUTIL_getCurBfio(&posInBcSlot, &slotIdx);

        /*pos in BC slot */
        bfioTemp = posInBcSlot * 1000 + *pBackoffDur;

        remBceTime = FHPIB_db.macBcDwellInterval*1000 - bfioTemp;
        remBcfTime = FHPIB_db.macBcInterval*1000 - bfioTemp;

        if((remBceTime > slotEdgeTime)
            || (remBcfTime < slotEdgeTime))
        {
            /*return status ERR_OUT_OF_SLOT */
            /*BC slot */
            return FHAPI_STATUS_ERR_OUT_SLOT;
        }
        /*check for end of bc interval, beg. of bc slot */
        if((remBcfTime < slotEdgeTime) && (remBcfTime > 0))
        {

             /*Take an additional backoff */
             delay1 = slotEdgeTime/1000 + FH_SLOT_ERR_EST_TIME_MS;
             /* remTime + guardTime; */
             /* compute the delay duration */
             *pBackoffDur += (delay1) * MAC_UNIT_BACKOFF_TIME;

             DBG_PRINTL1(DBGSYS, "FHTraceLog: Add backOff in BC slot(0x%0x)",\
                         delay1);
             return status;
        }


        /*check for end of BC Slot */
        if((remBceTime < slotEdgeTime) && (remBceTime > 0))
        {
            /*Take an additional backoff */
            delay1 = slotEdgeTime/1000;
            /* remTime + guardTime */
            /* compute the delay duration */
            *pBackoffDur += (delay1) * MAC_UNIT_BACKOFF_TIME;

            DBG_PRINTL1(DBGSYS, "FHTraceLog: Add backOff in BC slot(0x%0x)",\
                        delay1);
        }
    }
    /*If fixed UC channel, no need to look for slot edges */
    *txUfsi = 0;
    if(!pEntry->UsieParams_s.channelFunc)
    {
        return status;
    }

    /*calculate guard time in us */
    guardTime = MAP_FHUTIL_getGuardTime(pEntry);

    guardTime += FH_SLOT_EDGE_PROTECTION_TIME;

    *txUfsi = MAP_FHUTIL_getTxUfsi(pEntry);
    /* slotIdx = ufsi/usie.ucDwellTime */
    /*delta = ufsi - slotIdx*usie.ucDwellTime */
    /* time into the slot */

    posInSlot = (*txUfsi % (pEntry->UsieParams_s.dwellInterval))*1000;

    posInSlot += *pBackoffDur;

    /* time remaining in slot */
    txDiMicro = (pEntry->UsieParams_s.dwellInterval)*1000;
    remTime =  txDiMicro - posInSlot;
    /*for UC pos 250-gt to 250 */
    if((remTime < guardTime) && (remTime >= 0))
    {
        /*Take an additional backoff */
        delay1 = (guardTime << 1)/1000;

        /* compute the delay duration */
        *pBackoffDur += (delay1) * MAC_UNIT_BACKOFF_TIME;

        DBG_PRINTL1(DBGSYS, "FHTraceLog: Add guardTime in backOffUnits(0x%0x)",\
                    delay1);
        return status;

    }
    /*for UC pos 0-gt */
    if((remTime > -guardTime) && (remTime < 0))
    {
        /* Take an additional backoff */
        delay1 = guardTime/1000;

        /* compute the delay duration */
        *pBackoffDur += (delay1) * MAC_UNIT_BACKOFF_TIME;

        DBG_PRINTL1(DBGSYS, "FHTraceLog: Add guardTime in backOffUnits(0x%0x)",\
                    delay1);
    }

    return status;
}

/*!
 * @brief       Calcultaes the tx channel for unicast transmission
 *
 * @param       pEntry - Pointer to neighbor table entry
 * @param       txUfsi - The current ufsi value for the neighbor
 *
 * @return      uint8_t - computed tx channel
 */
uint8_t FHUTIL_getUcChannel(NODE_ENTRY_s *pEntry, uint32_t txUfsi)
{
    uint16_t slotIdx;
    uint8_t channel;

    if(!pEntry->UsieParams_s.channelFunc)
        return pEntry->UsieParams_s.chInfo.fixedChannel;

    /* get the previously compute tx ufsi and add backoff duation to it */
    /*  txUfsi = FH_hnd.txUfsi + backOffDur + FH_PHY_RADIO_FIXED_DELAY;*/

    slotIdx  = (txUfsi)/(pEntry->UsieParams_s.dwellInterval);


    channel = MAP_FHDH1CF_getTxChannelNum(slotIdx,
                                 pEntry);

    return(channel);

}

/*!
 * Calculates the tx channel for unicast transmission at a given offset
 *
 * Public function defined in fh_util.h
 */
uint8_t FHUTIL_getTxChannel(NODE_ENTRY_s *pEntry, uint32_t txUfsi,
                            uint32_t offset)
{
    uint8_t channel;
    uint32_t bfio, timeLimit;
    uint16_t slotIdx;

    MAP_FHUTIL_getCurBfio(&bfio, &slotIdx);
    bfio += ((offset)/1000);

    /*get channel number */
    if(((bfio >= FHPIB_db.macBcInterval) ||
         (bfio < FHPIB_db.macBcDwellInterval)) && (FH_hnd.bsStarted))
    {
        //slotIdx = FH_hnd.bcSlotIdx;
        if(bfio >= FHPIB_db.macBcInterval)
        {
             slotIdx++;
        }

        channel = MAP_FHDH1CF_getBCChannelNum(slotIdx, FH_hnd.bcNumChannels);
        return(channel);
    }
    /* Update tx ufsi and add backoff duation to it */
    txUfsi += (offset/1000);

    timeLimit = (FH_USCH_NUM_SLOTS * (pEntry->UsieParams_s.dwellInterval));
    if(txUfsi   >= timeLimit)
    {
        txUfsi  -= timeLimit;
    }
    channel = MAP_FHUTIL_getUcChannel(pEntry, txUfsi);

    return(channel);
}

/*!
 *This function computes the backoff duration to be used for
 *              a broadcast transmission
 *
 * Public function defined in fh_util.h
 */
FHAPI_status FHUTIL_getBcTxParams(uint32_t *pBackOffDur, uint8_t *pChIdx)
{
    int32_t remTime, slotEdgeTime;
    uint32_t bfio;
    uint16_t slotIdx;

    slotEdgeTime = FH_SLOT_EDGE_PROTECTION_TIME;

    if(MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_SLR_5K)
    {
        slotEdgeTime = FH_LRM_SLOT_EDGE_PROTECTION_TIME;
    }

    /*limit backoff to dwell time */

    remTime = FHPIB_db.macBcDwellInterval*1000 - slotEdgeTime - FH_SLOT_ERR_EST_TIME;
    if (*pBackOffDur > remTime)
    {
         remTime = remTime - FH_SLOT_ERR_EST_TIME;
         *pBackOffDur %= remTime;
    }

    /* get the current bfio in us*/
    /* = FH_hnd.bfio*1000 + FH_macGetElapsedFHTime(&FH_hnd.bcTimer);*/
    MAP_FHUTIL_getCurBfio(&bfio, &slotIdx);

    /* pos at backoff expiry */
    bfio *= 1000;
    bfio += *pBackOffDur;

    /* check if in broadcast slot */
    remTime = FHPIB_db.macBcDwellInterval*1000 - bfio;



    if((remTime < slotEdgeTime))
    {
        DBG_PRINTL1(DBGSYS, "FHAPI_getTxParams - out of bc slot(bfio+bo=0X%x)",\
                    bfio);
        return(FHAPI_STATUS_ERR_OUT_SLOT);
    }

    if(remTime > ((FHPIB_db.macBcDwellInterval*1000  - FH_SLOT_ERR_EST_TIME)))
    {
        *pBackOffDur += FH_SLOT_ERR_EST_TIME;
    }

    *pChIdx = MAP_FHDH1CF_getBCChannelNum(FH_hnd.bcSlotIdx, FH_hnd.bcNumChannels);
    /* bsi = 0 for now, need to get from MAC */

    return(FHAPI_STATUS_SUCCESS);
}

/*!
 * This function gets the current value of the ufsi
 *
 * Public function defined in fh_util.h
 */
FHAPI_status FHUTIL_getCurUfsi(uint32_t *ufsi)
{
    uint8_t  intState;
    uint32_t temp32;

    HAL_ENTER_CRITICAL_SECTION(intState);

    temp32 = MAP_FHMGR_macGetElapsedFHTime(&FH_hnd.ucTimer);
    *ufsi = (temp32 *(CLOCK_TICK_PERIOD)/1000);
    *ufsi += FH_hnd.ufsi;

    HAL_EXIT_CRITICAL_SECTION(intState);

    return(FHAPI_STATUS_SUCCESS);
}

/*!
 *This function gets the current value of broadcast fractional interval offset
 *
 * Public function defined in fh_util.h
 */
FHAPI_status FHUTIL_getCurBfio(uint32_t *bfio, uint16_t *slotIdx)
{
    uint8_t  intState;
    uint32_t temp32;
    uint32_t macBcInterval;

    if(!FH_hnd.bsStarted)
    {
        *bfio = 0;
        *slotIdx = 0;
        return(FHAPI_STATUS_SUCCESS);
    }
#if 0
    if(!pMacPib->rxOnWhenIdle)
    {
    	/*Sleepy Device curr BC time calculation */
    	macBcInterval = FHPIB_db.macBcInterval;
    	/*get time elapsed from last update*/
        temp32 = MAP_FHUTIL_getElapsedTime(FH_hnd.btieTs);

        /*calc. and update bfio and slotIdx at current time */
        *slotIdx = FH_hnd.bcSlotIdx + (temp32/macBcInterval);

         *bfio = FH_hnd.bfio + temp32 % macBcInterval;

         if(*bfio >= macBcInterval)
 	    {
     	    *bfio -= macBcInterval;
     	    *slotIdx += 1;
 	    }
    }
    else
#endif
    {
    	/*Non-Sleepy Device curr BC time calculation */

        HAL_ENTER_CRITICAL_SECTION(intState);

        macBcInterval = FHPIB_db.macBcInterval;

        temp32 = MAP_FHMGR_macGetElapsedFHTime(&FH_hnd.bcTimer);
        *bfio = (temp32 * (CLOCK_TICK_PERIOD) / 1000);
        *bfio += FH_hnd.bfio;

        *slotIdx = FH_hnd.bcSlotIdx;
        if(*bfio >= macBcInterval)
	    {
    	    *bfio -= macBcInterval;
    	    *slotIdx += 1;
	    }
        HAL_EXIT_CRITICAL_SECTION(intState);
    }
    return(FHAPI_STATUS_SUCCESS);
}

/*!
 * Computes the elapsed time since last known RTC time stamp
 *
 * Public function defined in fh_util.h
 */
uint32_t FHUTIL_getElapsedTime(uint32_t prevTime)
{
    uint32_t temp32, temp32_1;

    temp32 = MAP_ICall_getTicks();

    temp32 -= prevTime;

    temp32_1 = (temp32/(1000/ CLOCK_TICK_PERIOD));

    return(temp32_1);

}

/*!
 * @brief       Computes the bfio value based on sfd detection time
 * *
 * @return      uint32_t compued bfio value
 */
uint32_t FHUTIL_compBfioTxTime(void)
{
    /*compensate for packet length time */

    return(MAP_FHUTIL_getElapsedTime(FH_hnd.rxSfdTs));

}

#if !defined(TIMAC_ROM_PATCH)
/*!
 * update btie based on bfio and slotIdx
 *
 * Public function defined in fh_util.h
 */

uint8_t FHUTIl_updateBTIE(uint32_t bfio, uint16_t slotIdx)
{
    uint32_t macBcInterval, temp32;
    uint64_t temp64, temp64_r;
    uint8_t  intState, status = 0;
    uint32_t comp_time;

     /*(bfsi/(2^32))*(32768*250)) */
    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &macBcInterval);

    temp64 = ((uint64_t)bfio * macBcInterval);
    temp64_r = (uint64_t)(0x80000000);
    temp64 += temp64_r;
    temp32 = (uint32_t)(temp64 >> 32);

      /* compensate for TX time */
    comp_time = MAP_FHUTIL_compBfioTxTime();
    temp32 += comp_time;
    //temp32 += FHUTIL_compBfioTxTime();

    /* boundary check */
    if(temp32 >= macBcInterval)
    {
      temp32 = temp32 - macBcInterval;
      slotIdx += 1;
    }
    DBG_PRINTL2(DBGSYS, "FHTraceLog: Parsed BT IE values: bfio=(0x%X) slotNo=(0x%X)",\
                temp32,slotIdx);

    HAL_ENTER_CRITICAL_SECTION(intState);

    if((temp32 != FH_hnd.bfio) || (slotIdx != FH_hnd.bcSlotIdx))
    {
        FH_hnd.bfio = temp32;
        FH_hnd.bcSlotIdx = slotIdx;
        FH_hnd.pendBcChHop = 0;
        /*Store btieTs for sleepy devices */
        FH_hnd.btieTs = MAP_ICall_getTicks();
        status = 1;
    }
    HAL_EXIT_CRITICAL_SECTION(intState);
    return status;
}
#endif

/*!
 * Calculates UFSI based on ufsi and dwell time
 *
 * Public function defined in fh_util.h
 */
uint32_t FHUTIL_calcUfsi(uint32_t ufsi, uint8_t ucDwellTime)
{
    uint32_t compUfsi;

    compUfsi = ufsi * (ucDwellTime);

    /* round before dividing */
    compUfsi = (compUfsi + (1 << 7)) >> 8;

    return (compUfsi);
}

/*!
 * Asserts the exection based on error conditions
 *
 * Public function defined in fh_util.h
 */

void FHUTIL_assert(void *pData)
{
    /* avoid compiler warning */
    (void)pData;
    MAC_ASSERT(0);
}

/*!
 * Performs no action. useful for state transisions with no special
 *              handling requirements
 *
 * Public function defined in fh_util.h
 */
void FHUTIL_noAction(void *pData)
{

}

#ifdef FH_PRINT_DEBUG
uint32 rx_dbgtsp[128];
uint32 rx_dbgtspcntr = 0;
extern uint8 macPhyChannel;
uint32 mac_tx_dbg_cnt = 0;
void FH_printDebugInfo(void)
{
    uint32_t bfio;
    uint16_t slotIdx;

    //MAP_FHUTIL_getCurUfsi(&ufsi);
    rx_dbgtsp[rx_dbgtspcntr++] = FH_hnd.bfio;
    MAP_FHUTIL_getCurBfio(&bfio, &slotIdx);

    //rx_dbgtsp[rx_dbgtspcntr++] = ufsi;
    rx_dbgtsp[rx_dbgtspcntr++] = macPhyChannel;
//  rx_dbgtsp[rx_dbgtspcntr++] = ((slotIdx << 16) | bfio);
    rx_dbgtsp[rx_dbgtspcntr++] = (bfio);
    rx_dbgtsp[rx_dbgtspcntr++] = (slotIdx);
    rx_dbgtspcntr &= 0x7f;
}

uint32 tx_dbgtsp[128];
uint32 tx_dbgtspcntr = 0;

/*!
 * @brief       Prints TX debug information.
 *
 * Public function defined in fh_util.h
 */
void FHUTIL_printTxDebugInfo(void)
{
    uint32_t ufsi, bfio;
    uint16_t slotIdx;

    ufsi = 0; //MAP_FHUTIL_getTxUfsi(FHNT_hnd.pNonSleepNtTable);

    MAP_FHUTIL_getCurBfio(&bfio, &slotIdx);

    tx_dbgtsp[tx_dbgtspcntr++] = ufsi;
    tx_dbgtsp[tx_dbgtspcntr++] = macPhyChannel;
//    tx_dbgtsp[tx_dbgtspcntr++] = ((slotIdx << 16) | bfio);
    tx_dbgtsp[tx_dbgtspcntr++] = (bfio);
    tx_dbgtsp[tx_dbgtspcntr++] = (slotIdx);
    tx_dbgtspcntr &= 0x7f;

    mac_tx_dbg_cnt++;
}

/*!
 * Prints CSMA debug information.
 *
 * Public function defined in fh_util.h
 */
#if 0
void FHUTIL_printCsmaDebugInfo(uint32_t backOffDur, uint8_t macTxCsmaBackoffDelay)
{
    uint32_t ufsi;

    ufsi = MAP_HUTIL_getTxUfsi(FHNT_hnd.pNonSleepNtTable);

    tx_dbgtsp[tx_dbgtspcntr++] = ufsi;
    tx_dbgtsp[tx_dbgtspcntr++] = backOffDur;
    tx_dbgtsp[tx_dbgtspcntr++] = macTxCsmaBackoffDelay;
    tx_dbgtsp[tx_dbgtspcntr++] = 0;
    tx_dbgtspcntr &= 0x7f;

    mac_tx_dbg_cnt++;
}
#endif
#endif

/*!
 * Compute the physical channel cooresponding to 0 index based on
 *              PHY configurations
 *
 * Public function defined in fh_util.h
 */
uint32_t FHUTIL_getCh0(void)
{
    uint32_t ch0;
    uint8 status;
    uint8_t phyId;
    macMRFSKPHYDesc_t mrFskPhy;
    MAP_MAC_MlmeGetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &phyId);
    status = MAP_MAC_GetPHYParamReq( MAC_PHY_DESCRIPTOR, phyId, &mrFskPhy );
    if(status == MAC_SUCCESS)
        ch0 = mrFskPhy.firstChCentrFreq;
    else
        ch0 = FHIE_DEFAULT_CH0;
    return(ch0);
}

/*!
 * Compute the channel spacing to be used based on configuration
 *
 * Public function defined in fh_util.h
 */
#if !defined(TIMAC_ROM_PATCH)
uint8_t FHUTIL_getChannelSpacing(void)
{
    uint8_t channelSpacing;
    uint8 status;
    uint8_t phyId;
    macMRFSKPHYDesc_t mrFskPhy;
    MAP_MAC_MlmeGetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &phyId);
    status = MAP_MAC_GetPHYParamReq( MAC_PHY_DESCRIPTOR, phyId, &mrFskPhy );
    if(status == MAC_SUCCESS)
    {
        channelSpacing = (mrFskPhy.channelSpacing/200 - 1)%3;
    }
    else
        channelSpacing = FHIE_DEFAULT_CHANNEL_SPACING;
    return(channelSpacing);
}
#endif

/*!
 * Compute the maximum number of channels based on configuration
 *
 * Public function defined in fh_util.h
 */
uint8_t FHUTIL_getMaxChannels(void)
{
    uint8_t maxChannels;
    uint8 status;
    uint8_t phyId;
    macMRFSKPHYDesc_t mrFskPhy;
    MAP_MAC_MlmeGetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &phyId);
    status = MAP_MAC_GetPHYParamReq( MAC_PHY_DESCRIPTOR, phyId, &mrFskPhy );
    if(status == MAC_SUCCESS)
        maxChannels = mrFskPhy.noOfChannels;
    else
        maxChannels = FHIE_DEFAULT_NO_OF_CHANNELS;
    return(maxChannels);
}

/*!
 * Computes the number of ones in a exclude channel bit accounting
 *              for max possible channels
 *
 * Public function defined in fh_util.h
 */
uint8_t FHUTIL_getBitCounts(uint8_t *bitmap, uint8_t maxChannels)
{
    uint8_t i;
    uint8_t count;
    uint8_t data;
    uint8_t noOfBytes;
    uint8_t noOfRBits;
    uint8_t rBitsMask;

    noOfBytes = ((maxChannels+7)>>3);
    noOfRBits = (maxChannels % 8);
    rBitsMask = (1 << noOfRBits) - 1;


    if(!bitmap || !maxChannels)
    {
        return(0);
    }

    count = 0;
    for(i = 0; i < noOfBytes; i++)
    {
        data = bitmap[i];
        if((i == noOfBytes - 1) && noOfRBits )
            data &= rBitsMask;
        /* Brian Kernighan's counting bits set */
        for(; data; count++)
        {
            data &= data - 1;
        }
    }
    return(count);
}

/*!
 * Updates the total number of unicast channels based on
 *              exclude channel bitmap
 *
 *Public function defined in fh_util.h
 */
void FHUTIL_updateUcNumChannels(uint8_t *bitmap)
{
    uint8_t maxChannels;
    uint8_t excludedChannels;
    maxChannels = MAP_FHUTIL_getMaxChannels();
    excludedChannels = MAP_FHUTIL_getBitCounts(bitmap, maxChannels);
    FH_hnd.ucNumChannels = maxChannels - excludedChannels;
}

/*!
 * Updates the total number of broadcast channels based on
 *              exclude channel bitmap
 *
 * Public function defined in fh_util.h
 */
void FHUTIL_updateBcNumChannels(uint8_t *bitmap)
{
    uint8_t maxChannels;
    uint8_t excludedChannels;
    maxChannels = MAP_FHUTIL_getMaxChannels();
    excludedChannels = MAP_FHUTIL_getBitCounts(bitmap, maxChannels);
    FH_hnd.bcNumChannels = maxChannels - excludedChannels;
}

/*!
 * Computes the dynamic guardtime to be applied ffor a neighbor
 *
 * Public function defined in fh_util.h
 */
uint32_t FHUTIL_getGuardTime(NODE_ENTRY_s *pEntry)
{
    uint32_t elapsedTime;
    uint32_t guardTime;
    uint32_t maxGuardTime;
    uint8_t myClockDrift;
    uint8_t theirClockDrift;
    uint8_t myDwellTime;

    if(!pEntry)
    {
        return 0;
    }

    MAP_FHPIB_get(FHPIB_UC_DWELL_INTERVAL, &myDwellTime);
    /* timeLimit = (FH_USCH_NUM_SLOTS * (myDwellTime));
       MAP_FHUTIL_getCurUfsi(&ufsi);
       elapsedTime  = (timeLimit + ufsi - (pEntry->ref_timeStamp)) % timeLimit;

       elapsedTime = MAP_FHUTIL_getElapsedTime(pEntry->ref_timeStamp);*/
    /*elapsed time in Ticks */
    elapsedTime = MAP_ICall_getTicks() - (pEntry->ref_timeStamp);

    MAP_FHPIB_get(FHPIB_CLOCK_DRIFT, &myClockDrift);
    theirClockDrift = pEntry->UsieParams_s.clockDrift;
    /* ClockDrift = 255 means the info is not provided */
    if(myClockDrift == 255)
    {
        myClockDrift = 10;
    }
    if(theirClockDrift == 255)
    {
        theirClockDrift = 10;
    }
    /* guardTime = (elapsedTime * (myClockDrift + theirClockDrift)) / 1000000;*/
    /*guardTime in us */
    guardTime = ((elapsedTime/100000) * (myClockDrift + theirClockDrift));
    maxGuardTime = pEntry->UsieParams_s.dwellInterval >> 2; /// 4;
    /*convert to us */
    maxGuardTime *= 1000;
    guardTime = (guardTime < maxGuardTime ? guardTime : maxGuardTime);

    return(guardTime);
}

/*!
 * Computes the elapsed time between two given rtc time stamps
 *
 * Public function defined in fh_util.h
 */
uint32_t FHUTIL_elapsedTime(uint32_t curTime, uint32_t oldTime)
{
    uint32_t elapsedTime;
    /* both time should be unsigned interger */
    elapsedTime = curTime - oldTime;

    return(elapsedTime);
}
#endif
