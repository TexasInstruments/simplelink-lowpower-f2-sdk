/******************************************************************************

 @file fh_ie.c

 @brief TIMAC 2.0 FH IE API

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

#include <stdint.h>
#include "mac_pib.h"
#include "fh_api.h"
#include "fh_pib.h"
#include "fh_util.h"
#include "fh_data.h"
#include "fh_ie.h"
#ifdef FHIEPATCH
#include "fh_ie_patch.h"
#endif
#include "fh_nt.h"
#include "mac_mcu.h"
#include "mac_hl_patch.h"
/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE fh_ie_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#ifdef FEATURE_FREQ_HOP_MODE

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Local variables
 *****************************************************************************/
#ifdef FHIE_DBG
FHIE_dbg_t  FHIE_dbg;
#endif
/******************************************************************************
 Glocal variables
 *****************************************************************************/

/* defined in mac_mgmt.c */
//extern bool macPanCoordinator;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
uint32 FHIE_getCcaSfdTime(uint8 fhFrameType)
{
    uint32_t ccaSfdTime;
    macMRFSKPHYDesc_t *pPhyDesc;
#ifdef FEATURE_WISUN_SUPPORT
    /* Get current PHY Descriptor*/
    pPhyDesc = MAP_macMRFSKGetPhyDesc(pMacPib->curPhyID);
#endif

    ccaSfdTime = FH_PHY_SFD_TIME_MS; /* Value corresponds to 50 kbps data rate */

    if (MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_SLR_5K)
    {
        ccaSfdTime = FH_LRM_PHY_SFD_TIME_MS;
    }
#ifdef FEATURE_WISUN_SUPPORT
    else if(pPhyDesc->symbolRate == MAC_MRFSK_SYMBOL_RATE_100_K)
    {
        ccaSfdTime = ccaSfdTime / 2;
    }
    else if(pPhyDesc->symbolRate == MAC_MRFSK_SYMBOL_RATE_150_K)
    {
        ccaSfdTime = ccaSfdTime / 3;
    }
    else if(pPhyDesc->symbolRate == MAC_MRFSK_SYMBOL_RATE_200_K)
    {
        ccaSfdTime = ccaSfdTime / 4;
    }
    else if(pPhyDesc->symbolRate == MAC_MRFSK_SYMBOL_RATE_300_K)
    {
        ccaSfdTime = ccaSfdTime / 6;
    }
#endif
    else if ((pMacPib->curPhyID >= MAC_200KBPS_915MHZ_PHY_132)
         && (pMacPib->curPhyID <= MAC_200KBPS_868MHZ_PHY_133))
    {
        ccaSfdTime = FH_PHY_SFD_TIME_200k_MS;
    }

    if(fhFrameType != MAC_FH_FRAME_ACK)
    {
#ifndef FEATURE_WISUN_SUPPORT
        /* Get current PHY Descriptor*/
        pPhyDesc = MAP_macMRFSKGetPhyDesc(pMacPib->curPhyID);
#endif
        if (pPhyDesc->ccaType == MAC_CCA_TYPE_LBT)
        {
            ccaSfdTime += FH_hnd.ccaTime;
        }
        else
        {
            /* for FCC band, the default value approx.250 us for all rates*/
            ccaSfdTime += FH_DEFAULT_CCA_TIME;
        }
    }
    ccaSfdTime += FH_PHY_RADIO_DELAY;

    return ((ccaSfdTime + FH_HALF_MS) /(2 * FH_HALF_MS));
}

#if !defined(TIMAC_ROM_PATCH)
void FHIE_genUTIE(uint8_t *pBuf, uint8_t fhFrameType)
{
    uint8_t  ucDwellTime;
    uint32_t temp32;

    MAP_FHPIB_get(FHPIB_UC_DWELL_INTERVAL, &ucDwellTime);

    MAP_FHUTIL_getCurUfsi(&temp32 );

    /* compensate for SFD time and CCA
     * round up to ms
     */
    temp32 += MAP_FHIE_getCcaSfdTime(fhFrameType);

    /*multiply by 256, divide by 250 */
    temp32 <<= 8;
    //24 bit value
    //utie = ufsi/dwell_time
    temp32 = (temp32 + (ucDwellTime >> 1))/ucDwellTime;
    *pBuf++ = fhFrameType;
    *pBuf++ = ((temp32 >> 0));
    *pBuf++ = ((temp32 >> 8));
    *pBuf++ = ((temp32 >>16));
}

void FHIE_genBTIE(uint8_t *pBuf, uint8_t frameType)
{
    uint32_t temp32, bcInterval;
    uint16_t slotIdx;
    uint64_t temp64;

    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &bcInterval);

    MAP_FHUTIL_getCurBfio(&temp32, &slotIdx);

    /* compensate for SFD time and CCA
     * round up to ms
     */
    temp32 += MAP_FHIE_getCcaSfdTime(frameType);

    if(temp32 >= bcInterval)
    {
        temp32  = temp32 - bcInterval;
        slotIdx += 1;
    }

    DBG_PRINTL2(DBGSYS, "FHTraceLog: Generated BT IE values: bfio=(0x%X) slotNo=(0x%X)", temp32,slotIdx);

    /*32-bit value */
    /* BTIE = bfio/broadcast interval */
    //long division, lot of cycles.
    temp64 = temp32 << 16;
    temp64 = temp64 << 16;
    //temp32 = (temp64 + (bcInterval >> 1))/bcInterval;
    temp32 = MAP_macMcuLongDiv(temp64, bcInterval);
    *pBuf++ = ((slotIdx >> 0));
    *pBuf++ = ((slotIdx >> 8));

    *pBuf++ = ((temp32 >> 0));
    *pBuf++ = ((temp32 >> 8));
    *pBuf++ = ((temp32 >> 16));
    *pBuf++ = ((temp32 >> 24));
}


uint8_t FHIE_getChannelPlan(FHIE_channelPlan_t *pChPlan)
{
    uint8_t i;

    pChPlan->ch0 = MAP_FHUTIL_getCh0();
    pChPlan->channelSpacing = MAP_FHUTIL_getChannelSpacing();
    pChPlan->noOfChannels = MAP_FHUTIL_getMaxChannels();
    pChPlan->regulatoryDomain = 0;
    pChPlan->operatingClass = 0;

    for(i=0; i < NO_OF_CHANNEL_PLAN; i++)
    {
        if((pChPlan->ch0 == FHIE_channelPlan[i].ch0)
        && (pChPlan->channelSpacing == FHIE_channelPlan[i].channelSpacing)
        && (pChPlan->noOfChannels == FHIE_channelPlan[i].noOfChannels))
        {
            pChPlan->regulatoryDomain = FHIE_channelPlan[i].regulatoryDomain;
            pChPlan->operatingClass = FHIE_channelPlan[i].operatingClass;
            return(FHIE_CP_REGULATORY_DOMAIN);
        }
    }
    return(FHIE_CP_VENDOR_SPECIFIED);
}

#endif

uint8_t FHIE_isChannelPlanValid(uint8_t channelPlan,
                                       FHIE_channelPlan_t *pChPlan)
{
    FHIE_channelPlan_t chPlan;
    uint32_t different = 0;
    uint8_t valid;

    MAP_FHIE_getChannelPlan(&chPlan);
    if(channelPlan == FHIE_CP_REGULATORY_DOMAIN)
    {
        different = ((pChPlan->regulatoryDomain) ^ (chPlan.regulatoryDomain));
        different += ((pChPlan->operatingClass) ^ (chPlan.operatingClass));
    }
    else if(channelPlan == FHIE_CP_VENDOR_SPECIFIED)
    {
        different = ((pChPlan->ch0) ^ (chPlan.ch0));
        different += ((pChPlan->channelSpacing) ^ (chPlan.channelSpacing));
        different += ((pChPlan->noOfChannels) ^ (chPlan.noOfChannels));
    }
    else
    {
        return(FH_INVALID);
    }

    valid = (different ? FH_INVALID : FH_VALID);

    return(valid);
}

/* excluded channel ranges to channel mask conversion */
uint16_t FHIE_genExcludedChannelMask(uint8_t *pData,
                                            uint8_t *pExcludedChannelMask,
                                            uint8_t id)
{
    uint8_t i;
    uint8_t j;
    uint8_t idx;
    uint8_t offset;
    uint16_t range;
    uint16_t startChannelNumber;
    uint16_t endChannelNumber;

    MAP_osal_memset(pExcludedChannelMask, 0, FHPIB_MAX_BIT_MAP_SIZE*sizeof(uint8_t));

    range = *pData++;

    for(i=0; i < range; i++)
    {
        startChannelNumber = MAKE_UINT16(pData[0], pData[1]);
        endChannelNumber = MAKE_UINT16(pData[2], pData[3]);
        pData += 4;

        for(j=startChannelNumber; j <= endChannelNumber; j++)
        {
            idx = j / 8;
            offset = j % 8;
            pExcludedChannelMask[idx] |= (1 << offset);
        }
    }

    if(range)
    {
        return(4*range + 1);
    }

    return(0);
}


/* excluded channel mask to channel ranges conversion */
uint16_t FHIE_genExcludedChannelRanges(uint8_t *pData, uint8_t id)
{
    uint8_t i;
    uint8_t j;
    uint8_t nwait_bit;
    uint16_t range;
    uint8_t excludedChannelMask[FHPIB_MAX_BIT_MAP_SIZE];
    uint16_t startChannelNumber;
    uint16_t endChannelNumber;
    uint8_t bitMapSize;
    uint16_t noOfChannels;
    uint8_t *pBuf = pData;

    if(id == FHIE_PIE_SUB_IE_SUB_ID_US_IE)
    {
        MAP_FHPIB_get(FHPIB_UC_EXCLUDED_CHANNELS, excludedChannelMask);
    }
    else if(id == FHIE_PIE_SUB_IE_SUB_ID_BS_IE)
    {
        MAP_FHPIB_get(FHPIB_BC_EXCLUDED_CHANNELS, excludedChannelMask);
    }
    else
    {
        return(0);
    }

    nwait_bit = 0;
    range = 0;

    if(pData)
    {
        /* place holder for number of ranges */
        pBuf++;
    }

    noOfChannels = MAP_FHUTIL_getMaxChannels();
    bitMapSize = ((noOfChannels+7)>>3);
    for(i=0; i < bitMapSize; i++)
    {
        for(j=0; j < 8; j++)
        {
            if(((excludedChannelMask[i] >> j) & 1) ^ nwait_bit )
            {
                if(!nwait_bit)
                {
                    /* hit start of 1 */
                    range++;
                    if(pData)
                    {
                        startChannelNumber = (i*8 + j) + FH_CHANNEL_OFFSET;
                        *pBuf++ = GET_BYTE(startChannelNumber, 0);
                        *pBuf++ = GET_BYTE(startChannelNumber, 1);
                    }
                }
                else
                {
                    /* hit start of 0 */
                    if(pData)
                    {
                        endChannelNumber = (i*8 + j - 1) + FH_CHANNEL_OFFSET;
                        *pBuf++ = GET_BYTE(endChannelNumber, 0);
                        *pBuf++ = GET_BYTE(endChannelNumber, 1);
                    }
                }
                nwait_bit ^= 1;
            }
        }
    }

    if(nwait_bit)
    {
        /* 1 reached the end without hitting zero */
        if(pData)
        {
            endChannelNumber = (i*8 - 1) + FH_CHANNEL_OFFSET;
            *pBuf++ = GET_BYTE(endChannelNumber, 0);
            *pBuf++ = GET_BYTE(endChannelNumber, 1);
        }
    }

    if(range)
    {
        if(pData)
        {
            *pData = range;
        }
        return(4*range + 1);
    }

    return(0);
}

uint8_t FHIE_getExcludedChannelControl(uint8_t id)
{
    uint16_t rangeLength;
    uint16_t noOfChannels;

    if((id != FHIE_PIE_SUB_IE_SUB_ID_US_IE)
    && (id != FHIE_PIE_SUB_IE_SUB_ID_BS_IE))
    {
        return(FHIE_ECC_NO_EC);
    }

    rangeLength = MAP_FHIE_genExcludedChannelRanges(NULL, id);

    noOfChannels = MAP_FHUTIL_getMaxChannels();

    if(!rangeLength)
    {
        return(FHIE_ECC_NO_EC);
    }
    else if(rangeLength < ((noOfChannels+7)>>3))
    {
        return(FHIE_ECC_RANGE);
    }
    else
    {
        return(FHIE_ECC_BIT_MASK);
    }
}

uint16_t FHIE_genCommonInformation(uint8_t *pData, uint8_t cp,
                                          uint8_t cf, uint8_t ecc,
                                          FHIE_channelPlan_t *pChPlan,
                                          uint8_t ucFlag)
{
    uint16_t fixedChannel;
    uint8_t excludedChannelMask[FHPIB_MAX_BIT_MAP_SIZE];
    uint16_t rangeLength;
    uint8_t *pDataStart = pData;

    if(cp == FHIE_CP_REGULATORY_DOMAIN)
    {
        *pData++ = pChPlan->regulatoryDomain;
        *pData++ = pChPlan->operatingClass;
    }
    else if(cp == FHIE_CP_VENDOR_SPECIFIED)
    {
        *pData++ = GET_BYTE(pChPlan->ch0, 0);
        *pData++ = GET_BYTE(pChPlan->ch0, 1);
        *pData++ = GET_BYTE(pChPlan->ch0, 2);
        *pData++ = pChPlan->channelSpacing;
        *pData++ = GET_BYTE(pChPlan->noOfChannels, 0);
        *pData++ = GET_BYTE(pChPlan->noOfChannels, 1);
    }
    else
    {
        // do nothing
    }

    if(cf == FHIE_CF_SINGLE_CHANNEL)
    {
        if(ucFlag == FH_UC)
        {
            MAP_FHPIB_get(FHPIB_UC_FIXED_CHANNEL, &fixedChannel);
        }
        else
        {
            MAP_FHPIB_get(FHPIB_BC_FIXED_CHANNEL, &fixedChannel);
        }

        *pData++ = GET_BYTE(fixedChannel, 0);
        *pData++ = GET_BYTE(fixedChannel, 1);
    }
    else
    {
        /* do nothing */
    }

    if(ecc == FHIE_ECC_RANGE)
    {
        // need to implement
        if(ucFlag == FH_UC)
        {
            rangeLength = MAP_FHIE_genExcludedChannelRanges(pData,
                    FHIE_PIE_SUB_IE_SUB_ID_US_IE);
        }
        else
        {
            rangeLength = MAP_FHIE_genExcludedChannelRanges(pData,
                    FHIE_PIE_SUB_IE_SUB_ID_BS_IE);
        }

        pData += rangeLength;
    }
    else if(ecc == FHIE_ECC_BIT_MASK)
    {
        if(ucFlag == FH_UC)
        {
            MAP_FHPIB_get(FHPIB_UC_EXCLUDED_CHANNELS, excludedChannelMask);
        }
        else
        {
            MAP_FHPIB_get(FHPIB_BC_EXCLUDED_CHANNELS, excludedChannelMask);
        }

        /* consider invalid channels */
        if(pChPlan->noOfChannels)
        {
            uint8_t idx;
            uint8_t offset;

            idx = (pChPlan->noOfChannels - 1) / 8;
            offset = (pChPlan->noOfChannels - 1) % 8;
            excludedChannelMask[idx] |= ~((1 << (offset + 1)) - 1);
        }

        MAP_osal_memcpy(pData, excludedChannelMask, (pChPlan->noOfChannels+7)>>3);
        pData += ((pChPlan->noOfChannels+7)>>3);
    }
    else
    {
        /* do nothing */
    }

    return((uint16_t)(pData - pDataStart));
}

#if !defined(TIMAC_ROM_PATCH)
uint16_t FHIE_genPieContent(uint8_t *pData, uint8_t id)
{
    uint16_t ieContentLength;
    uint16_t ieCommonLength;
    uint32_t broadcastInterval;
    uint16_t broadcastSchedId;
    uint8_t dwellInterval;
    uint8_t clockDrift;
    uint8_t timingAccuracy;
    uint8_t channelPlan;
    uint8_t channelFunction;
    uint8_t excludedChannelControl;
    FHIE_channelPlan_t chPlan;
    FHIE_panIE_t panIe;
    FHIE_netNameIE_t netNameIe;
    FHIE_panVerIE_t panVerIe;
    FHIE_gtkHashIE_t gtkHashIe;

    ieContentLength = 0;
#ifdef FHIEPATCH
    ieContentLength = MAP_FHIEPATCH_genPieContent(pData, id);
    if(ieContentLength)
    {
        return(ieContentLength);
    }
#endif
    switch(id)
    {
    case FHIE_PIE_SUB_IE_SUB_ID_US_IE:
        MAP_FHPIB_get(FHPIB_CLOCK_DRIFT, &clockDrift);
        MAP_FHPIB_get(FHPIB_TIMING_ACCURACY, &timingAccuracy);
        MAP_FHPIB_get(FHPIB_UC_DWELL_INTERVAL, &dwellInterval);
        channelPlan = MAP_FHIE_getChannelPlan(&chPlan);
        MAP_FHPIB_get(FHPIB_UC_CHANNEL_FUNCTION, &channelFunction);
        if(channelFunction)
        {
            excludedChannelControl = MAP_FHIE_getExcludedChannelControl(id);
        }
        else
        {
            /* ECC field MUST be set to 0 when CF field is set to zero*/
            excludedChannelControl = FHIE_ECC_NO_EC;
        }
        *pData++ = dwellInterval;
        *pData++ = clockDrift;
        *pData++ = timingAccuracy;
        *pData++ = MAKE_CP(channelPlan, channelFunction,
                           excludedChannelControl);
        ieContentLength += FH_US_IE_FIXED_PART_LEN;
        ieCommonLength = MAP_FHIE_genCommonInformation(pData, channelPlan,
                                                   channelFunction,
                                                   excludedChannelControl,
                                                   &chPlan, FH_UC);
        ieContentLength += ieCommonLength;
        pData += ieCommonLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_BS_IE:
        MAP_FHPIB_get(FHPIB_BC_INTERVAL, &broadcastInterval);
        MAP_FHPIB_get(FHPIB_BROCAST_SCHED_ID, &broadcastSchedId);
        MAP_FHPIB_get(FHPIB_CLOCK_DRIFT, &clockDrift);
        MAP_FHPIB_get(FHPIB_TIMING_ACCURACY, &timingAccuracy);
        MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &dwellInterval);
        channelPlan = MAP_FHIE_getChannelPlan(&chPlan);
        MAP_FHPIB_get(FHPIB_BC_CHANNEL_FUNCTION, &channelFunction);
        if(channelFunction)
        {
            excludedChannelControl = MAP_FHIE_getExcludedChannelControl(id);
        }
        else
        {
            /* ECC field MUST be set to 0 when CF field is set to zero*/
            excludedChannelControl = FHIE_ECC_NO_EC;
        }
        MAP_osal_memcpy(pData, &broadcastInterval, sizeof(broadcastInterval));
        pData += sizeof(broadcastInterval);
        MAP_osal_memcpy(pData, &broadcastSchedId, sizeof(broadcastSchedId));
        pData += sizeof(broadcastSchedId);
        *pData++ = dwellInterval;
        *pData++ = clockDrift;
        *pData++ = timingAccuracy;
        *pData++ = MAKE_CP(channelPlan, channelFunction,
                           excludedChannelControl);
        ieContentLength += FH_BS_IE_FIXED_PART_LEN;
        ieCommonLength = MAP_FHIE_genCommonInformation(pData, channelPlan,
                                                   channelFunction,
                                                   excludedChannelControl,
                                                   &chPlan, FH_BC);
        ieContentLength += ieCommonLength;
        pData += ieCommonLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_PAN_IE:
        MAP_FHPIB_get(FHPIB_PAN_SIZE, &panIe.panSize);
        MAP_FHPIB_get(FHPIB_ROUTING_COST, &panIe.routingCost);
        MAP_FHPIB_get(FHPIB_USE_PARENT_BS_IE, &panIe.useParentBSIE);
        MAP_FHPIB_get(FHPIB_ROUTING_METHOD, &panIe.routingMethod);
        MAP_FHPIB_get(FHPIB_EAPOL_READY, &panIe.eapolReady);
        MAP_FHPIB_get(FHPIB_FAN_TPS_VERSION, &panIe.fanTpsVersion);
        *pData++ = GET_BYTE(panIe.panSize, 0);
        *pData++ = GET_BYTE(panIe.panSize, 1);
        *pData++ = panIe.routingCost;
        *pData++ = MAKE_PANCTL(panIe.useParentBSIE, panIe.routingMethod,
                               panIe.eapolReady, panIe.fanTpsVersion);
        ieContentLength += FH_PAN_IE_LEN;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE:
        MAP_osal_memset(netNameIe.netName, 0, FH_NETNAME_IE_LEN_MAX);
        MAP_FHPIB_get(FHPIB_NET_NAME, &netNameIe.netName);
        MAP_osal_memcpy(pData, &netNameIe.netName, MAP_osal_strlen((char *)netNameIe.netName));
        ieContentLength += MAP_osal_strlen((char *)netNameIe.netName);
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE:
        MAP_FHPIB_get(FHPIB_PAN_VERSION, &panVerIe.panVersion);
        *pData++ = GET_BYTE(panVerIe.panVersion, 0);
        *pData++ = GET_BYTE(panVerIe.panVersion, 1);
        ieContentLength += FH_PANVER_IE_LEN;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE:
        MAP_FHPIB_get(FHPIB_GTK_0_HASH, &gtkHashIe.gtk0Hash);
        MAP_FHPIB_get(FHPIB_GTK_1_HASH, &gtkHashIe.gtk1Hash);
        MAP_FHPIB_get(FHPIB_GTK_2_HASH, &gtkHashIe.gtk2Hash);
        MAP_FHPIB_get(FHPIB_GTK_3_HASH, &gtkHashIe.gtk3Hash);
        MAP_osal_memcpy(pData, gtkHashIe.gtk0Hash, FH_GTK0HASH_LEN);
        pData += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(pData, gtkHashIe.gtk1Hash, FH_GTK0HASH_LEN);
        pData += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(pData, gtkHashIe.gtk2Hash, FH_GTK0HASH_LEN);
        pData += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(pData, gtkHashIe.gtk3Hash, FH_GTK0HASH_LEN);
        pData += FH_GTK0HASH_LEN;
        ieContentLength += FH_GTKHASH_IE_LEN;
        break;

    default:
        return(0);
    }

    return(ieContentLength);
}

uint16_t FHIE_genPie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo)
{
    uint8_t *pBuf;
    uint16_t ieDescriptor;
    uint16_t subIeDescriptor;
    uint16_t ieLength;
    uint16_t subIeLength;
    uint32_t ieBitmap;
    uint32_t bitmapMask;
    uint8_t ieId;
    uint8_t typeLong;

#ifdef FHIE_DBG
    FHIE_dbg.pie_gen_try++;
#endif

    pBuf = pData;
    ieLength = 0;
    typeLong = 1;

    pBuf += IE_DESCRIPTOR_LEN;
    ieLength += IE_DESCRIPTOR_LEN;

    bitmapMask = FH_WISUN_PIE_BITMAP_START;
    do
    {
        ieBitmap = bitmap & bitmapMask;
        bitmap &= ~bitmapMask;
        bitmapMask <<= 1;

        subIeLength = 0;
        ieId = 0;
        switch(ieBitmap)
        {
        case FH_WISUN_PIE_US_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_US_IE;
            typeLong = 1;
            break;

        case FH_WISUN_PIE_BS_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_BS_IE;
            typeLong = 1;
            break;

        case FH_WISUN_PIE_PAN_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_PAN_IE;
            typeLong = 0;
            break;

        case FH_WISUN_PIE_NETNAME_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE;
            typeLong = 0;
            break;

        case FH_WISUN_PIE_PANVER_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE;
            typeLong = 0;
            break;

        case FH_WISUN_PIE_GTKHASH_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE;
            typeLong = 0;
            break;

        default:
        if(ieBitmap)
        {
#ifdef FHIEPATCH
            ieId = MAP_FHIEPATCH_getPieId(ieBitmap, &typeLong);
#endif
#ifdef FHIE_DBG
            FHIE_dbg.pie_gen_not_supported++;
#endif
        }
            break;
        }
        if(ieId)
        {
            subIeLength = MAP_FHIE_genPieContent(pBuf+SUB_IE_DESCRIPTOR_LEN, ieId);
            if(typeLong)
            {
                subIeDescriptor = MAKE_PIE_SUB_IE_LONG(ieId, subIeLength);
            }
            else
            {
                subIeDescriptor = MAKE_PIE_SUB_IE_SHORT(ieId, subIeLength);
            }
            *pBuf++ = GET_BYTE(subIeDescriptor, 0);
            *pBuf++ = GET_BYTE(subIeDescriptor, 1);
            pBuf += subIeLength;
            ieLength += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
        }
    } while(bitmap);

    if(ieLength > IE_DESCRIPTOR_LEN)
    {
        ieDescriptor = MAKE_PIE(PIE_GROUP_ID_WISUN,
                                ieLength - IE_DESCRIPTOR_LEN);
        *pData++ = GET_BYTE(ieDescriptor, 0);
        *pData++ = GET_BYTE(ieDescriptor, 1);
    }
    else
    {
        ieLength = 0;
#ifdef FHIE_DBG
        FHIE_dbg.pie_gen_fail++;
#endif
    }

    return(ieLength);
}

uint16_t FHIE_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo)
{
    uint8_t *pBuf;
    uint16_t ieDescriptor;
    uint16_t ieLength;
    uint16_t subIeLength;
    uint32_t ieBitmap;
    uint32_t bitmapMask;
    uint8_t  macBcDwellTime, frameType = 0;

    pBuf = pData;
    ieLength = 0;

    bitmapMask = FH_WISUN_HIE_BITMAP_START;
    do
    {
#ifdef FHIE_DBG
        FHIE_dbg.hie_gen_try++;
#endif
        ieBitmap = bitmap & bitmapMask;
        bitmap &= ~bitmapMask;
        bitmapMask <<= 1;

        subIeLength = 0;
#ifdef FHIEPATCH
        subIeLength = MAP_FHIEPATCH_genHie(pBuf, ieBitmap, pMacTx, pIeInfo);
        if(subIeLength)
        {
            ieLength += subIeLength; /* Descriptor + content */
            pBuf += subIeLength;
            continue;
        }
#endif
        switch(ieBitmap)
        {
        case FH_WISUN_HIE_FC_IE_BITMAP:
            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_FC_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_FC_IE;
            if(pIeInfo)
            {
                *pBuf++ = pIeInfo->fcIe.txFlowControl;
                *pBuf++ = pIeInfo->fcIe.rxFlowControl;
            }
            else    /*if data not given, set to 0 */
            {
                *pBuf++ = 0;
                *pBuf++ = 0;
            }
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
            break;

        case FH_WISUN_HIE_UT_IE_BITMAP:
            /* pass UTIE content pointer */
            if(pMacTx)
            {
                pMacTx->internal.ptrUTIE = pBuf;
                frameType = pMacTx->internal.fhFrameType;
            }
            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_UT_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_UT_IE;
            /* pass UTIE content pointer */

            MAP_FHIE_genUTIE(pBuf, frameType);

            pBuf += HIE_LEN_UT_IE;
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
            break;

        case FH_WISUN_HIE_RSL_IE_BITMAP:
            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_RSL_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_RSL_IE;
            if(pIeInfo)
            {
                *pBuf++ = pIeInfo->rslIe.rsl;
            }
            else    /*if data not given, set to 0 */
            {
                *pBuf++ = 0;
            }
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
            break;

        case FH_WISUN_HIE_BT_IE_BITMAP:
            MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);
            if(macBcDwellTime)
            {
            /* pass BTIE content pointer */
            if(pMacTx)
            {
                pMacTx->internal.ptrBTIE = pBuf;
                frameType = pMacTx->internal.fhFrameType;
            }
            subIeLength = HIE_LEN_SUB_ID + HIE_LEN_BT_IE;
            ieDescriptor = MAKE_HIE(HIE_ELEM_ID_WISUN, subIeLength);
            *pBuf++ = GET_BYTE(ieDescriptor, 0);
            *pBuf++ = GET_BYTE(ieDescriptor, 1);
            *pBuf++ = FHIE_HIE_SUB_ID_BT_IE;

            MAP_FHIE_genBTIE(pBuf, frameType);

            pBuf += HIE_LEN_BT_IE;
            ieLength += (IE_DESCRIPTOR_LEN + subIeLength);
            }
            break;

        default:
        if(ieBitmap)
        {
#ifdef FHIE_DBG
            FHIE_dbg.hie_gen_fail++;
#endif
        }
            break;
        }
    } while(bitmap);

    return(ieLength);
}
#endif

uint16_t FHIE_getCommonInformationLen(uint8_t cp, uint8_t cf,
                                             uint8_t ecc,
                                             FHIE_channelPlan_t *pChPlan,
                                             uint8_t ucFlag)
{
    uint16_t rangeLength;
    uint16_t commonLength;

    commonLength = 0;

    if(cp == FHIE_CP_REGULATORY_DOMAIN)
    {
        commonLength += FH_CP_RD_LEN;
    }
    else if(cp == FHIE_CP_VENDOR_SPECIFIED)
    {
        commonLength += FH_CP_VS_LEN;
    }
    else
    {
        /* do nothing */
    }

    if(cf == FHIE_CF_SINGLE_CHANNEL)
    {
        commonLength += FH_UC_FIXED_CHANNEL_LEN;
    }
    else
    {
        /* do nothing */
    }

    if(ecc == FHIE_ECC_RANGE)
    {
        if(ucFlag == FH_UC)
        {
            rangeLength = MAP_FHIE_genExcludedChannelRanges(NULL,
                    FHIE_PIE_SUB_IE_SUB_ID_US_IE);
        }
        else
        {
            rangeLength = MAP_FHIE_genExcludedChannelRanges(NULL,
                    FHIE_PIE_SUB_IE_SUB_ID_BS_IE);
        }

        commonLength += rangeLength;
    }
    else if(ecc == FHIE_ECC_BIT_MASK)
    {
        commonLength += ((pChPlan->noOfChannels+7)>>3);
    }
    else
    {
        /* do nothing */
    }

    return(commonLength);
}

#if !defined(TIMAC_ROM_PATCH)
uint16_t FHIE_getPieContentLen(uint8_t id)
{
    uint16_t ieContentLength;
    uint16_t ieCommonLength;
    uint8_t channelPlan;
    uint8_t channelFunction;
    uint8_t excludedChannelControl;
    FHIE_channelPlan_t chPlan;
    FHIE_netNameIE_t netName;

    ieContentLength = 0;
#ifdef FHIEPATCH
    ieContentLength = MAP_FHIEPATCH_getPieContentLen(id);
    if(ieContentLength)
    {
        return(ieContentLength);
    }
#endif
    switch(id)
    {
    case FHIE_PIE_SUB_IE_SUB_ID_US_IE:
        channelPlan = MAP_FHIE_getChannelPlan(&chPlan);
        MAP_FHPIB_get(FHPIB_UC_CHANNEL_FUNCTION, &channelFunction);
        if(channelFunction)
        {
            excludedChannelControl = MAP_FHIE_getExcludedChannelControl(id);
        }
        else
        {
            /* ECC field MUST be set to 0 when CF field is set to zero*/
            excludedChannelControl = FHIE_ECC_NO_EC;
        }
        ieContentLength += FH_US_IE_FIXED_PART_LEN;
        ieCommonLength = MAP_FHIE_getCommonInformationLen(channelPlan,
                                                      channelFunction,
                                                      excludedChannelControl,
                                                      &chPlan, FH_UC);
        ieContentLength += ieCommonLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_BS_IE:
        channelPlan = MAP_FHIE_getChannelPlan(&chPlan);
        MAP_FHPIB_get(FHPIB_BC_CHANNEL_FUNCTION, &channelFunction);
        if(channelFunction)
        {
            excludedChannelControl = MAP_FHIE_getExcludedChannelControl(id);
        }
        else
        {
            /* ECC field MUST be set to 0 when CF field is set to zero*/
            excludedChannelControl = FHIE_ECC_NO_EC;
        }
        ieContentLength += FH_BS_IE_FIXED_PART_LEN;
        ieCommonLength = MAP_FHIE_getCommonInformationLen(channelPlan,
                                                      channelFunction,
                                                      excludedChannelControl,
                                                      &chPlan, FH_BC);
        ieContentLength += ieCommonLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_PAN_IE:
        ieContentLength += FH_PAN_IE_LEN;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE:
        MAP_FHPIB_get(FHPIB_NET_NAME, netName.netName);
        ieContentLength += MAP_osal_strlen((char *)netName.netName);
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE:
        ieContentLength += FH_PANVER_IE_LEN;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE:
        ieContentLength += FH_GTKHASH_IE_LEN;
        break;

    default:
        return(0);
    }

    return(ieContentLength);
}
#endif

uint16_t FHIE_getPieLen(uint32_t bitmap)
{
    uint16_t ieLength;
    uint16_t subIeLength;
    uint32_t ieBitmap;
    uint32_t bitmapMask;
    uint8_t ieId;

    ieLength = 0;

    ieLength += IE_DESCRIPTOR_LEN;

    bitmapMask = FH_WISUN_PIE_BITMAP_START;
    do
    {
        ieBitmap = bitmap & bitmapMask;
        bitmap &= ~bitmapMask;
        bitmapMask <<= 1;

        subIeLength = 0;
        ieId = 0;
        switch(ieBitmap)
        {
        case FH_WISUN_PIE_US_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_US_IE;
            break;

        case FH_WISUN_PIE_BS_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_BS_IE;
            break;

        case FH_WISUN_PIE_PAN_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_PAN_IE;
            break;

        case FH_WISUN_PIE_NETNAME_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE;
            break;

        case FH_WISUN_PIE_PANVER_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE;
            break;

        case FH_WISUN_PIE_GTKHASH_IE_BITMAP:
            ieId = FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE;
            break;

        default:
        if(ieBitmap)
        {
#ifdef FHIEPATCH
            ieId = MAP_FHIEPATCH_getPieId(ieBitmap, NULL);
#endif
#ifdef FHIE_DBG
            FHIE_dbg.pie_getlen_fail++;
#endif
        }
            break;
        }
        if(ieId)
        {
            subIeLength = MAP_FHIE_getPieContentLen(ieId);
            ieLength += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
        }
#ifdef FHIE_DBG
        FHIE_dbg.pie_getlen_try++;
#endif
    } while(bitmap);

    if(ieLength > IE_DESCRIPTOR_LEN)
    {
        /* do nothing */
    }
    else
    {
        ieLength = 0;
    }

    return(ieLength);
}

#if !defined(TIMAC_ROM_PATCH)
uint16_t FHIE_getHieLen(uint32_t bitmap)
{
    uint16_t ieLength;
    uint16_t subIeLength;
    uint32_t ieBitmap;
    uint32_t bitmapMask;
    uint8_t  macBcDwellTime;

    ieLength = 0;

    bitmapMask = FH_WISUN_HIE_BITMAP_START;
    do
    {
#ifdef FHIE_DBG
        FHIE_dbg.hie_getlen_try++;
#endif
        ieBitmap = bitmap & bitmapMask;
        bitmap &= ~bitmapMask;
        bitmapMask <<= 1;

        subIeLength = 0;
#ifdef FHIEPATCH
        subIeLength = MAP_FHIEPATCH_getHieLen(ieBitmap);
        if(subIeLength)
        {
            ieLength += subIeLength;
            continue;
        }
#endif
        switch(ieBitmap)
        {
        case FH_WISUN_HIE_FC_IE_BITMAP:
            subIeLength = HIE_LEN_FC_IE;
            ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            break;

        case FH_WISUN_HIE_UT_IE_BITMAP:
            subIeLength = HIE_LEN_UT_IE;
            ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            break;

        case FH_WISUN_HIE_RSL_IE_BITMAP:
            subIeLength = HIE_LEN_RSL_IE;
            ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            break;

        case FH_WISUN_HIE_BT_IE_BITMAP:
            MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);
            if(macBcDwellTime)
            {
              subIeLength = HIE_LEN_BT_IE;
              ieLength += (IE_DESCRIPTOR_LEN + HIE_LEN_SUB_ID + subIeLength);
            }
            break;

        default:
        if(ieBitmap)
        {
#ifdef FHIE_DBG
            FHIE_dbg.hie_getlen_fail++;
#endif
        }
            break;
        }
    } while(bitmap);

    return(ieLength);
}
#endif

uint16_t FHIE_parseCommonInformation(uint8_t *pData, uint8_t chinfo,
                                            usieParams_t *pUsIeParams,
                                            uint8_t ucFlag)
{
    uint8_t i;
    uint16_t rangeLength;
    uint8_t noOfRange;
    uint8_t cp;
    uint8_t cf;
    uint8_t ecc;
    FHIE_channelPlan_t chPlan;
    uint8_t *pDatStart = pData;

    cp = (chinfo)&7;
    cf = ((chinfo)>>3)&7;
    ecc = ((chinfo)>>6)&3;

    MAP_osal_memset(&chPlan, 0, sizeof(FHIE_channelPlan_t));
    if(cp == FHIE_CP_REGULATORY_DOMAIN)
    {
        chPlan.regulatoryDomain = *pData++;
        chPlan.operatingClass = *pData++;
        chPlan.ch0 = 0;
        chPlan.channelSpacing = 0;
        chPlan.noOfChannels = 0;
        for(i=0; i < NO_OF_CHANNEL_PLAN; i++)
        {
            if((chPlan.regulatoryDomain
                == FHIE_channelPlan[i].regulatoryDomain)
            && (chPlan.operatingClass == FHIE_channelPlan[i].operatingClass))
            {
                chPlan.ch0 = FHIE_channelPlan[i].ch0;
                chPlan.channelSpacing = FHIE_channelPlan[i].channelSpacing;
                chPlan.noOfChannels = FHIE_channelPlan[i].noOfChannels;
                pUsIeParams->numChannels = chPlan.noOfChannels;
                break;
            }
        }
    }
    else if(cp == FHIE_CP_VENDOR_SPECIFIED)
    {
        MAP_osal_memcpy(&(chPlan.ch0), pData, 3);
        pData += 3;
        chPlan.channelSpacing = (*pData++)&0xF;
        MAP_osal_memcpy(&chPlan.noOfChannels, pData, sizeof(uint16_t));
        pData += sizeof(uint16_t);
        pUsIeParams->numChannels = chPlan.noOfChannels;
        chPlan.regulatoryDomain = 0;
        chPlan.operatingClass = 0;
        for(i=0; i < NO_OF_CHANNEL_PLAN; i++)
        {
            if((chPlan.ch0 == FHIE_channelPlan[i].ch0)
            && (chPlan.channelSpacing == FHIE_channelPlan[i].channelSpacing)
            && (chPlan.noOfChannels == FHIE_channelPlan[i].noOfChannels))
            {
                chPlan.regulatoryDomain = FHIE_channelPlan[i].regulatoryDomain;
                chPlan.operatingClass = FHIE_channelPlan[i].operatingClass;
                break;
            }
        }
    }
    else
    {
        /* not supported */
        return(0);
    }

    /* check if channel plan matches */
    if(!MAP_FHIE_isChannelPlanValid(cp, &chPlan))
    {
        return(0);
    }

    pUsIeParams->channelFunc = cf;
    if(cf == FHIE_CF_SINGLE_CHANNEL)
    {
        MAP_osal_memcpy(&(pUsIeParams->chInfo.fixedChannel), pData, sizeof(uint16_t));
        pData += sizeof(uint16_t);
    }
    else if(cf == FHIE_CF_DH1CF)
    {
        /* do nothing */
    }
    else // FHIE_CF_TR51CF
    {
        /* not supprted */
        return(0);
    }

    if(ecc == FHIE_ECC_RANGE)
    {
        if(ucFlag == FH_UC)
        {
            rangeLength = MAP_FHIE_genExcludedChannelMask(pData,
                                                pUsIeParams->chInfo.bitMap,
                                                FHIE_PIE_SUB_IE_SUB_ID_US_IE);
        }
        else
        {
            rangeLength = MAP_FHIE_genExcludedChannelMask(pData,
                                                pUsIeParams->chInfo.bitMap,
                                                FHIE_PIE_SUB_IE_SUB_ID_BS_IE);
        }

        noOfRange = *pData;
        rangeLength = 4*noOfRange + 1;
        pData += rangeLength;
    }
    else if(ecc == FHIE_ECC_BIT_MASK)
    {
        MAP_osal_memcpy(pUsIeParams->chInfo.bitMap, pData, (chPlan.noOfChannels+7)>>3);
        pData += ((chPlan.noOfChannels+7)>>3);

        /* consider invalid channels */
        if(chPlan.noOfChannels)
        {
            uint8_t idx;
            uint8_t offset;

            idx = (chPlan.noOfChannels - 1) / 8;
            offset = (chPlan.noOfChannels - 1) % 8;
            pUsIeParams->chInfo.bitMap[idx] &= ((1 << (offset + 1)) - 1);
        }
    }
    else
    {
        /* do nothing */
    }

    return((uint16_t)(pData - pDatStart));
}

uint16_t FHIE_getPie(uint8_t *pData, FHIE_ie_t *pie)
{
    uint16_t ieDescriptor;

    ieDescriptor = MAKE_UINT16(pData[0], pData[1]);
    pie->type = IE_UNPACKING(ieDescriptor,IE_TYPE_S,IE_TYPE_P);
    pie->id = IE_UNPACKING(ieDescriptor,PIE_GROUP_ID_S,PIE_GROUP_ID_P);
    pie->len = IE_UNPACKING(ieDescriptor,PIE_LEN_S,PIE_LEN_P);

    if((pie->type != IE_TYPE_PAYLOAD)
    || (pie->id != PIE_GROUP_ID_WISUN)
    || (pie->len > FH_PIE_MAX_LEN)
    || (pie->len < FH_PIE_MIN_LEN))
    {
        pie->pVal = NULL;
        return(0);
    }
    pie->pVal = pData + IE_DESCRIPTOR_LEN;
    return(pie->len);
}

uint16_t FHIE_getSubPie(uint8_t *pData, FHIE_ie_t *subPie)
{
    uint16_t subIeDescriptor;

    subIeDescriptor = MAKE_UINT16(pData[0], pData[1]);
    subPie->type = IE_UNPACKING(subIeDescriptor,PIE_SUB_IE_TYPE_S,
                                PIE_SUB_IE_TYPE_P);
    if(subPie->type == PIE_SUB_IE_TYPE_LONG)
    {
        subPie->id = IE_UNPACKING(subIeDescriptor,PIE_SUB_IE_SUB_ID_LONG_S,
                                  PIE_SUB_IE_SUB_ID_LONG_P);
        subPie->len = IE_UNPACKING(subIeDescriptor,PIE_SUB_IE_LEN_LONG_S,
                                   PIE_SUB_IE_LEN_LONG_P);
    }
    else
    {
        subPie->id = IE_UNPACKING(subIeDescriptor,PIE_SUB_IE_SUB_ID_SHORT_S,
                                  PIE_SUB_IE_SUB_ID_SHORT_P);
        subPie->len = IE_UNPACKING(subIeDescriptor,PIE_SUB_IE_LEN_SHORT_S,
                                  PIE_SUB_IE_LEN_SHORT_P);
    }

    if((subPie->len > FH_PIE_MAX_LEN) || (subPie->len < FH_PIE_MIN_LEN))
    {
        subPie->pVal = NULL;
        return(0);
    }
#ifndef FHIEPATCH
    if((subPie->id != FHIE_PIE_SUB_IE_SUB_ID_US_IE)
    && (subPie->id != FHIE_PIE_SUB_IE_SUB_ID_BS_IE)
    && (subPie->id != FHIE_PIE_SUB_IE_SUB_ID_PAN_IE)
    && (subPie->id != FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE)
    && (subPie->id != FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE)
    && (subPie->id != FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE))
    {
        subPie->pVal = NULL;
        return(0);
    }
#endif
    subPie->pVal = pData + SUB_IE_DESCRIPTOR_LEN;
    return(subPie->len);
}

uint16_t FHIE_getHie(uint8_t *pData, FHIE_ie_t *hie)
{
    uint16_t ieDescriptor;

    ieDescriptor = MAKE_UINT16(pData[0], pData[1]);
    hie->type = IE_UNPACKING(ieDescriptor,IE_TYPE_S,IE_TYPE_P);
    hie->id = IE_UNPACKING(ieDescriptor,HIE_ELEM_ID_S,HIE_ELEM_ID_P);
    hie->len = IE_UNPACKING(ieDescriptor,HIE_LEN_S,HIE_LEN_P);

    if((hie->type != IE_TYPE_HEADER)
    || (hie->id != HIE_ELEM_ID_WISUN)
    || (hie->len > FH_HIE_MAX_LEN)
    || (hie->len < FH_HIE_MIN_LEN))
    {
        hie->pVal = NULL;
        return(0);
    }

    hie->pVal = pData + IE_DESCRIPTOR_LEN;
    return(hie->len);
}

/******************************************************************************
 Public Functions
 *****************************************************************************/
#if !defined(TIMAC_ROM_PATCH)
/*!
 FHIE_gen

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API uint16_t FHIE_gen(uint8_t *pData, uint32_t bitmap,
                                   macTx_t *pMacTx, FHIE_ieInfo_t *pIeInfo)
{
    uint32_t genPie;
    uint32_t genHie;

    if(!pData)
    {
        return(0);
    }

    genPie = bitmap & FH_WISUN_PIE_BITMAP;
    genHie = bitmap & FH_WISUN_HIE_BITMAP;

    /* each IE generation at a time */
    if(genPie && genHie)
    {
        return(0);
    }

    if(genPie)
    {
        return(MAP_FHIE_genPie(pData, bitmap, pMacTx, pIeInfo));
    }

    if(genHie)
    {
        return(MAP_FHIE_genHie(pData, bitmap, pMacTx, pIeInfo));
    }

    return(0);
}
#endif

/*!
 FHIE_getLen

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API uint16_t FHIE_getLen(uint32_t bitmap)
{
    uint32_t genPie;
    uint32_t genHie;

    genPie = bitmap & FH_WISUN_PIE_BITMAP;
    genHie = bitmap & FH_WISUN_HIE_BITMAP;

    /* each IE generation at a time */
    if(genPie && genHie)
    {
        return(0);
    }

    if(genPie)
    {
        return(MAP_FHIE_getPieLen(bitmap));
    }

    if(genHie)
    {
        return(MAP_FHIE_getHieLen(bitmap));
    }

    return(0);
}

/*!
 FHIE_parsePie

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API FHAPI_status FHIE_parsePie(uint8_t *pData,
                                            sAddrExt_t *pSrcAddr,
                                            uint16_t *pLen1,
                                            uint16_t *pLen2)
{
    uint8_t subIeId;
    uint16_t ieLength;
    uint16_t subIeLength;
    uint16_t parsedLength;
    uint32_t broadcastInterval;
    uint16_t broadcastSchedId;
    uint16_t currBroadcastSchedId;
    uint16_t ieCommonLength;
    uint8_t channelInfo;
    uint8_t excludedChannels;
    sAddrExt_t parentEUI;
    usieParams_t usIeParams;
    FHIE_ie_t pie;
    FHIE_ie_t subPie;
    uint8_t *pBuf = pData;
    NODE_ENTRY_s NodeEntry;
    sAddrExt_t defaultEUI;
    FHAPI_status status;

    parsedLength = 0;

    MAP_osal_memcpy(&defaultEUI,&(FHPIB_defaults.macTrackParentEUI),sizeof(sAddrExt_t));

    ieLength = MAP_FHIE_getPie(pBuf, &pie);
    if(!ieLength)
    {
        *pLen1 = 0;
        *pLen2 = 0;
#ifdef FHIE_DBG
        FHIE_dbg.pie_parse_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }

    parsedLength += IE_DESCRIPTOR_LEN;
    pBuf += IE_DESCRIPTOR_LEN;

    do
    {
        subIeLength = MAP_FHIE_getSubPie(pBuf, &subPie);
        if(!subIeLength)
        {
            break;
        }

#ifdef FHIE_DBG
        FHIE_dbg.pie_parse_try++;
#endif

        MAP_osal_memset(&usIeParams, 0, sizeof(usieParams_t));
        subIeId = subPie.id;
#ifdef FHIEPATCH
        status = MAP_FHIEPATCH_parsePie(subIeId, subIeLength, pBuf, pSrcAddr);
        if(status != FHAPI_STATUS_ERR_NO_PATCH)
        {
            if (status == FHAPI_STATUS_SUCCESS)
            {
                pBuf += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
                parsedLength += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
                continue;
            }
            else
            {
                *pLen1 = 0;
                *pLen2 = 0;
                return(status);
            }
        }
#endif
        switch(subIeId)
        {
        case FHIE_PIE_SUB_IE_SUB_ID_US_IE:
            if(subIeLength <= FH_US_IE_FIXED_PART_LEN)
            {
                *pLen1 = 0;
                *pLen2 = 0;
#ifdef FHIE_DBG
                FHIE_dbg.pie_parse_invalid++;
#endif
                return(FHAPI_STATUS_ERR_INVALID_FORMAT);
            }
            pBuf += SUB_IE_DESCRIPTOR_LEN;
            usIeParams.dwellInterval = *pBuf++;
            usIeParams.clockDrift = *pBuf++;
            usIeParams.timingAccuracy = *pBuf++;
            channelInfo = *pBuf++;
            ieCommonLength = MAP_FHIE_parseCommonInformation(pBuf, channelInfo,
                                                         &usIeParams, FH_UC);
            pBuf += ieCommonLength; /* pBuf incremented by subIeLength */
            /* in case no common lenght or channel plan mismatches
             * or not supported channel function */
            if(!ieCommonLength)
            {
                *pLen1 = 0;
                *pLen2 = 0;
#ifdef FHIE_DBG
                FHIE_dbg.pie_parse_invalid++;
#endif
                return(FHAPI_STATUS_ERR_INVALID_FORMAT);
            }
            //update neighbor table
            MAP_FHNT_getEntry((sAddr_t *)pSrcAddr, &NodeEntry);

            if(NodeEntry.valid == FHNT_NODE_INVALID )
            {
#ifdef FHNT_SPLIT_TABLE
                if(usIeParams.channelFunc == 0)
                {
                    status = MAP_FHNT_createFixedEntry(pSrcAddr,&NodeEntry);
                }
                else
                {
                    status = MAP_FHNT_createEntry(pSrcAddr,&NodeEntry);
                }
#else
                status = MAP_FHNT_createEntry(pSrcAddr,&NodeEntry);
#endif
                if (status != FHAPI_STATUS_SUCCESS)
                {
                    return status;
                }
            }

            NodeEntry.valid |= FHNT_NODE_W_USIE;
            MAP_osal_memcpy(&(NodeEntry.UsieParams_s), &usIeParams,
                    sizeof(usieParams_t));
            /* update numChannels in Node Entry */
            if(usIeParams.channelFunc)  /* no update when fixed channel */
            {
                excludedChannels
                = MAP_FHUTIL_getBitCounts(usIeParams.chInfo.bitMap,
                                      usIeParams.numChannels);
                NodeEntry.UsieParams_s.numChannels = usIeParams.numChannels
                                                        - excludedChannels;
            }
            /* need to save nodeEntry */
            MAP_FHNT_putEntry(&NodeEntry);

            parsedLength += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
            break;

        case FHIE_PIE_SUB_IE_SUB_ID_BS_IE:
            if(subIeLength <= FH_BS_IE_FIXED_PART_LEN)
            {
                *pLen1 = 0;
                *pLen2 = 0;
#ifdef FHIE_DBG
                FHIE_dbg.pie_parse_invalid++;
#endif
                return(FHAPI_STATUS_ERR_INVALID_FORMAT);
            }
            pBuf += SUB_IE_DESCRIPTOR_LEN;
            MAP_osal_memcpy(&broadcastInterval, pBuf, sizeof(uint32_t));
            pBuf += sizeof(uint32_t);
            MAP_osal_memcpy(&broadcastSchedId, pBuf, sizeof(uint16_t));
            pBuf += sizeof(uint16_t);
            MAP_FHPIB_get(FHPIB_BROCAST_SCHED_ID, &currBroadcastSchedId);
            if(broadcastSchedId < currBroadcastSchedId)
            {
                *pLen1 = 0;
                *pLen2 = 0;
#ifdef FHIE_DBG
                FHIE_dbg.pie_parse_invalid++;
#endif
                return(FHAPI_STATUS_ERR_INVALID_FORMAT);
            }
            usIeParams.dwellInterval = *pBuf++;
            usIeParams.clockDrift = *pBuf++;
            usIeParams.timingAccuracy = *pBuf++;
            channelInfo = *pBuf++;
            ieCommonLength = MAP_FHIE_parseCommonInformation(pBuf, channelInfo,
                                                         &usIeParams, FH_BC);
            pBuf += ieCommonLength; /* pBuf incremented by subIeLength */
            /* in case no common lenght or channel plan mismatches
             * or not supported channel function */
            if(!ieCommonLength)
            {
                *pLen1 = 0;
                *pLen2 = 0;
#ifdef FHIE_DBG
                FHIE_dbg.pie_parse_invalid++;
#endif
                return(FHAPI_STATUS_ERR_INVALID_FORMAT);
            }
            //update PIB
            if(!macPanCoordinator)
            {
                MAP_FHPIB_get(FHPIB_TRACK_PARENT_EUI, &parentEUI);
                if(!MAP_memcmp(&parentEUI, &defaultEUI, sizeof(sAddrExt_t))
                || !MAP_memcmp(&parentEUI, pSrcAddr, sizeof(sAddrExt_t)))
                {
                    MAP_FHPIB_set(FHPIB_BC_INTERVAL, &broadcastInterval);
                    MAP_FHPIB_set(FHPIB_BROCAST_SCHED_ID, &broadcastSchedId);
                    MAP_FHPIB_set(FHPIB_BC_DWELL_INTERVAL,
                              &usIeParams.dwellInterval);
                    MAP_FHPIB_set(FHPIB_CLOCK_DRIFT, &usIeParams.clockDrift);
                    MAP_FHPIB_set(FHPIB_TIMING_ACCURACY,
                              &usIeParams.timingAccuracy);
                    MAP_FHPIB_set(FHPIB_BC_CHANNEL_FUNCTION,
                              &usIeParams.channelFunc);
                    if(usIeParams.channelFunc == FHIE_CF_SINGLE_CHANNEL)
                    {
                        MAP_FHPIB_set(FHPIB_BC_FIXED_CHANNEL,
                                  &usIeParams.chInfo.fixedChannel);
                    }
                    else if(usIeParams.channelFunc == FHIE_CF_DH1CF)
                    {
                        MAP_FHPIB_set(FHPIB_BC_EXCLUDED_CHANNELS,
                                  usIeParams.chInfo.bitMap);
                    }
                    FH_hnd.fhBSRcvd = 1;
#if 0
                    if(!FH_hnd.bsStarted)
                    {
                        MAP_FHAPI_startBS();
                        FH_hnd.bsStarted = 1;
                    }
#endif
                }
            }

            parsedLength += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
            break;

        case FHIE_PIE_SUB_IE_SUB_ID_PAN_IE:
        case FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE:
        case FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE:
        case FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE:
        default:
            pBuf += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
#ifdef FHIE_DBG
            FHIE_dbg.pie_parse_not_supported++;
#endif
            break;
        }
    } while(parsedLength < ieLength + IE_DESCRIPTOR_LEN);

    if(parsedLength > IE_DESCRIPTOR_LEN)
    {
        *pLen1 = ieLength + IE_DESCRIPTOR_LEN;
        *pLen2 = ieLength + IE_DESCRIPTOR_LEN - parsedLength;
        return(FHAPI_STATUS_SUCCESS);
    }
    else
    {
        *pLen1 = 0;
        *pLen2 = 0;
#ifdef FHIE_DBG
        FHIE_dbg.pie_parse_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }
}

#if !defined(TIMAC_ROM_PATCH)
/*!
 FHIE_parseHie

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API FHAPI_status FHIE_parseHie(uint8_t *pData,
                                            sAddrExt_t *pSrcAddr,
                                            uint32_t ts,
                                            uint16_t *pLen1,
                                            uint16_t *pLen2)
{
    uint8_t subIeId;
    uint16_t ieLength;
    uint16_t parsedLength;
    uint16_t unsupportLength;
    /*
    uint8_t txFlowControl;
    uint8_t rxFlowControl;
    uint8_t rsl;
    */
    uint32_t ufsi;
    uint8_t frameTypeId;
    uint8_t *pBuf = pData;
    FHIE_ie_t hie;
    NODE_ENTRY_s NodeEntry;
#ifdef FHIEPATCH
    FHAPI_status status;
#endif
    parsedLength = 0;
    unsupportLength = 0;

    do
    {
        ieLength = MAP_FHIE_getHie(pBuf, &hie);
        if(!ieLength)
        {
            break;
        }
#ifdef FHIE_DBG
        FHIE_dbg.hie_parse_try++;
#endif
        parsedLength += IE_DESCRIPTOR_LEN;
        pBuf += IE_DESCRIPTOR_LEN;

        subIeId = pBuf[0];
#ifdef FHIEPATCH
        status = MAP_FHIEPATCH_parseHie(subIeId, ieLength, pBuf, ts, pSrcAddr);
        if (status == FHAPI_STATUS_SUCCESS)
        {
            pBuf += ieLength;
            parsedLength += ieLength;
            continue;
        }
#endif
        switch(subIeId)
        {
        case FHIE_HIE_SUB_ID_FC_IE:
            if(ieLength != HIE_LEN_SUB_ID + HIE_LEN_FC_IE)
            {
                unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
            }
            else
            {
                /*
                txFlowControl = pBuf[1];
                rxFlowControl = pBuf[2];
                */
            }
            break;

        case FHIE_HIE_SUB_ID_UT_IE:
            if(ieLength != HIE_LEN_SUB_ID + HIE_LEN_UT_IE)
            {
                unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
            }
            else
            {
                frameTypeId = (pBuf[1] & 0xF);
                if(frameTypeId > FH_UT_IE_FT_EAPOL)
                {
                    /* do we need to handle this case? */
                }
                ufsi = 0;
                MAP_osal_memcpy(&ufsi, &pBuf[2], HIE_LEN_UT_IE - 1);
                /* update neighbor table */
                MAP_FHNT_getEntry((sAddr_t *)pSrcAddr, &NodeEntry);

                if(NodeEntry.valid)
                {
                    NodeEntry.valid |= FHNT_NODE_W_UTIE;
                    NodeEntry.valid &= ~FHNT_NODE_EXPIRED;
                    NodeEntry.ufsi = ufsi;
                    NodeEntry.ref_timeStamp = FH_hnd.rxSfdTs;

                    /* need to save the entry */
                    MAP_FHNT_putEntry(&NodeEntry);
                }
            }
            break;

        case FHIE_HIE_SUB_ID_RSL_IE:
            if(ieLength != HIE_LEN_SUB_ID + HIE_LEN_RSL_IE)
            {
                unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
            }
            else
            {
                /*
                rsl = pBuf[1];
                */
            }
            break;

        case FHIE_HIE_SUB_ID_BT_IE:
            if(ieLength != HIE_LEN_SUB_ID + HIE_LEN_BT_IE)
            {
                unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
            }
            else
            {
                FH_hnd.rxSlotIdx = MAKE_UINT16(pBuf[1], pBuf[2]);
                MAP_osal_memcpy(&FH_hnd.btie, &pBuf[3], sizeof(uint32_t));
                FH_hnd.btiePresent = 1;
            }
            break;

        default:
            unsupportLength += (IE_DESCRIPTOR_LEN + ieLength);
#ifdef FHIE_DBG
            FHIE_dbg.hie_parse_not_supported++;
#endif
            break;
        }
        pBuf += ieLength;
        parsedLength += ieLength;
    } while(1);

    if(parsedLength > IE_DESCRIPTOR_LEN)
    {
        *pLen1 = parsedLength;
        *pLen2 = unsupportLength;
        return(FHAPI_STATUS_SUCCESS);
    }
    else
    {
        *pLen1 = 0;
        *pLen2 = 0;
#ifdef FHIE_DBG
        FHIE_dbg.hie_parse_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }
}

/*!
 FHIE_extractPie

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API FHAPI_status FHIE_extractPie(uint8_t *pData,
                                              IE_TYPE_t ieType,
                                              uint8_t *pOut)
{
    uint8_t subIeId;
    uint16_t ieLength;
    uint16_t subIeLength;
    uint16_t parsedLength;
    uint16_t currBroadcastSchedId;
    uint16_t ieCommonLength;
    uint8_t channelInfo;
    usieParams_t usIeParams;
    FHIE_ie_t pie;
    FHIE_ie_t subPie;
    uint8_t found;
    uint8_t *pBuf = pData;
    FHIE_usIe_t *pUsIe;
    FHIE_bsIE_t *pBsIe;
    FHIE_panIE_t *pPanIe;
    FHIE_netNameIE_t *pNetNameIe;
    FHIE_panVerIE_t *pPanVerIe;
    FHIE_gtkHashIE_t *pGtkHashIe;
#ifdef FHIEPATCH
    FHAPI_status status;
#endif
#ifdef FHIE_DBG
    FHIE_dbg.pie_extract_try++;
#endif

    found = 0;
    parsedLength = 0;

    ieLength = MAP_FHIE_getPie(pBuf, &pie);
    if(!ieLength)
    {
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.pie_extract_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }

    parsedLength += IE_DESCRIPTOR_LEN;
    pBuf += IE_DESCRIPTOR_LEN;

    do
    {
        subIeLength = MAP_FHIE_getSubPie(pBuf, &subPie);
        if(!subIeLength)
        {
            break;
        }

        subIeId = subPie.id;
        if(subIeId == ieType)
        {
            found = 1;
            pBuf += SUB_IE_DESCRIPTOR_LEN;
            parsedLength += SUB_IE_DESCRIPTOR_LEN;
            break;
        }
        else
        {
            pBuf += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
            parsedLength += (SUB_IE_DESCRIPTOR_LEN + subIeLength);
        }
    } while(parsedLength < ieLength + IE_DESCRIPTOR_LEN);

    if(!found)
    {
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.pie_extract_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }
#ifdef FHIEPATCH
    status = MAP_FHIEPATCH_extractPie(subIeId, subIeLength, pBuf, pOut);
    if(status != FHAPI_STATUS_ERR_NO_PATCH)
    {
        return(status);
    }
#endif
    switch(subIeId)
    {
    case FHIE_PIE_SUB_IE_SUB_ID_US_IE:
        if(subIeLength <= FH_US_IE_FIXED_PART_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        MAP_osal_memset(&usIeParams, 0, sizeof(usieParams_t));
        pUsIe = (FHIE_usIe_t *)pOut;
        pUsIe->schedInfo.dwellInterval = *pBuf++;
        pUsIe->schedInfo.clockDrift = *pBuf++;
        pUsIe->schedInfo.timingAccuracy = *pBuf++;
        channelInfo = *pBuf++;
        pUsIe->schedInfo.channelFunction = GET_CHAN_FUNC(channelInfo);

        ieCommonLength = MAP_FHIE_parseCommonInformation(pBuf, channelInfo,
                                                     &usIeParams, FH_UC);
        /* in case no common lenght or channel plan mismatches
         * or not supported channel function */
        if(!ieCommonLength)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        /* move usIeParams to pUsIe */
        pUsIe->chanInfo.noOfChannels = usIeParams.numChannels;
        pUsIe->chanInfo.fixedChannel = usIeParams.chInfo.fixedChannel;
        MAP_osal_memcpy(pUsIe->chanInfo.excludedChannelMask, usIeParams.chInfo.bitMap,
               FHPIB_MAX_BIT_MAP_SIZE);

        parsedLength += subIeLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_BS_IE:
        if(subIeLength <= FH_BS_IE_FIXED_PART_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        MAP_osal_memset(&usIeParams, 0, sizeof(usieParams_t));
        pBsIe = (FHIE_bsIE_t *)pOut;
        MAP_osal_memcpy(&pBsIe->broadcastInterval, pBuf, sizeof(uint32_t));
        pBuf += sizeof(uint32_t);
        MAP_osal_memcpy(&pBsIe->broadcastSchedId, pBuf, sizeof(uint16_t));
        pBuf += sizeof(uint16_t);
        MAP_FHPIB_get(FHPIB_BROCAST_SCHED_ID, &currBroadcastSchedId);
        if(pBsIe->broadcastSchedId < currBroadcastSchedId)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pBsIe->schedInfo.dwellInterval = *pBuf++;
        pBsIe->schedInfo.clockDrift = *pBuf++;
        pBsIe->schedInfo.timingAccuracy = *pBuf++;
        channelInfo = *pBuf++;
        pBsIe->schedInfo.channelFunction = GET_CHAN_FUNC(channelInfo);
        ieCommonLength = MAP_FHIE_parseCommonInformation(pBuf, channelInfo,
                                                     &usIeParams, FH_BC);
        /* in case no common lenght or channel plan mismatches
         * or not supported channel function */
        if(!ieCommonLength)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return(FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        /* move usIeParams to pUsIe */
        pBsIe->chanInfo.noOfChannels = usIeParams.numChannels;
        pBsIe->chanInfo.fixedChannel = usIeParams.chInfo.fixedChannel;
        MAP_osal_memcpy(pBsIe->chanInfo.excludedChannelMask, usIeParams.chInfo.bitMap,
               FHPIB_MAX_BIT_MAP_SIZE);

        parsedLength += subIeLength;
        break;

    case FHIE_PIE_SUB_IE_SUB_ID_PAN_IE:
        if (subIeLength != FH_PAN_IE_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pPanIe = (FHIE_panIE_t *)pOut;
        MAP_osal_memcpy(&pPanIe->panSize, pBuf, sizeof(uint16_t));
        pBuf += sizeof(uint16_t);
        pPanIe->routingCost = *pBuf++;
        pPanIe->useParentBSIE = ((*pBuf) >> 0) & 1;
        pPanIe->routingMethod = ((*pBuf) >> 1) & 1;
        pPanIe->eapolReady = ((*pBuf) >> 2) & 1;
        pPanIe->fanTpsVersion = ((*pBuf++) >> 5) & 7;

        parsedLength += subIeLength;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_NETNAME_IE:
        if (subIeLength > FH_NETNAME_IE_LEN_MAX)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pNetNameIe = (FHIE_netNameIE_t *)pOut;
        MAP_osal_memset(&pNetNameIe->netName, 0, FH_NETNAME_IE_LEN_MAX);
        MAP_osal_memcpy(&pNetNameIe->netName, pBuf, subIeLength);

        pBuf += subIeLength;
        parsedLength += subIeLength;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_PANVER_IE:
        if (subIeLength != FH_PANVER_IE_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pPanVerIe = (FHIE_panVerIE_t *)pOut;
        MAP_osal_memcpy(&pPanVerIe->panVersion, pBuf, sizeof(uint16_t));
        pBuf += sizeof(uint16_t);
        parsedLength += subIeLength;
        break;
    case FHIE_PIE_SUB_IE_SUB_ID_GTKHASH_IE:
        if (subIeLength != FH_GTKHASH_IE_LEN)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.pie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pGtkHashIe = (FHIE_gtkHashIE_t *)pOut;
        MAP_osal_memcpy(&pGtkHashIe->gtk0Hash, pBuf, FH_GTK0HASH_LEN);
        pBuf += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(&pGtkHashIe->gtk1Hash, pBuf, FH_GTK0HASH_LEN);
        pBuf += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(&pGtkHashIe->gtk2Hash, pBuf, FH_GTK0HASH_LEN);
        pBuf += FH_GTK0HASH_LEN;
        MAP_osal_memcpy(&pGtkHashIe->gtk3Hash, pBuf, FH_GTK0HASH_LEN);
        pBuf += FH_GTK0HASH_LEN;

        parsedLength += subIeLength;
        break;

    default:
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.pie_extract_not_supported++;
#endif
        return (FHAPI_STATUS_ERR_NOT_SUPPORTED_IE);
    }

    return (FHAPI_STATUS_SUCCESS);
}

/*!
 FHIE_extractHie

 Public function defined in fh_ie.h
 */
MAC_INTERNAL_API FHAPI_status FHIE_extractHie(uint8_t *pData,
                                              IE_TYPE_t ieType,
                                              uint8_t *pOut)
{
    uint8_t subIeId;
    uint16_t ieLength;
    uint16_t parsedLength;
    FHIE_ie_t hie;
    uint8_t found;
    uint8_t *pBuf = pData;
    FHIE_utIE_t *pUtIe;
    FHIE_btIE_t *pBtIe;
    FHIE_fcIE_t *pFcIe;
    FHIE_rslIE_t *pRslIe;
#ifdef FHIEPATCH
    FHAPI_status status;
#endif
#ifdef FHIE_DBG
    FHIE_dbg.hie_extract_try++;
#endif

    found = 0;
    parsedLength = 0;

    do
    {
        ieLength = MAP_FHIE_getHie(pBuf, &hie);
        if(!ieLength)
        {
            break;
        }

        pBuf += IE_DESCRIPTOR_LEN;
        parsedLength += IE_DESCRIPTOR_LEN;

        subIeId = *pBuf;
        if(subIeId == ieType)
        {
            found = 1;
            pBuf += HIE_LEN_SUB_ID;
            parsedLength += HIE_LEN_SUB_ID;
            break;
        }
        else
        {
            pBuf += ieLength;
            parsedLength += ieLength;
        }
    } while(1);

    if(!found)
    {
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.hie_extract_invalid++;
#endif
        return(FHAPI_STATUS_ERR_INVALID_FORMAT);
    }
#ifdef FHIEPATCH
    status = MAP_FHIEPATCH_extractHie(subIeId, ieLength, pBuf, pOut);
    if(status != FHAPI_STATUS_ERR_NO_PATCH)
    {
        return(status);
    }
#endif
    switch (subIeId)
    {
    case FHIE_HIE_SUB_ID_FC_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_FC_IE)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pFcIe = (FHIE_fcIE_t *)pOut;
        pFcIe->txFlowControl = *pBuf++;
        pFcIe->rxFlowControl = *pBuf;

        parsedLength += HIE_LEN_FC_IE;
        break;

    case FHIE_HIE_SUB_ID_UT_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_UT_IE)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pUtIe = (FHIE_utIE_t *)pOut;
        pUtIe->frameTypeId = *pBuf++;
        if (pUtIe->frameTypeId > FH_UT_IE_FT_EAPOL)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }
        pUtIe->ufsi = 0;
        MAP_osal_memcpy(&pUtIe->ufsi, pBuf, HIE_LEN_UT_IE - 1);

        parsedLength += HIE_LEN_UT_IE;
        break;

    case FHIE_HIE_SUB_ID_RSL_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_RSL_IE)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pRslIe = (FHIE_rslIE_t *)pOut;
        pRslIe->rsl = *pBuf;

        parsedLength += HIE_LEN_RSL_IE;
        break;

    case FHIE_HIE_SUB_ID_BT_IE:
        if (ieLength != HIE_LEN_SUB_ID + HIE_LEN_BT_IE)
        {
            pOut = NULL;
#ifdef FHIE_DBG
            FHIE_dbg.hie_extract_invalid++;
#endif
            return (FHAPI_STATUS_ERR_INVALID_FORMAT);
        }

        pBtIe = (FHIE_btIE_t *)pOut;
        pBtIe->bsn = MAKE_UINT16(pBuf[0], pBuf[1]);
        MAP_osal_memcpy(&pBtIe->bfio, &pBuf[2], sizeof(uint32_t));

        parsedLength += HIE_LEN_BT_IE;
        break;

    default:
        pOut = NULL;
#ifdef FHIE_DBG
        FHIE_dbg.hie_extract_not_supported++;
#endif
        return (FHAPI_STATUS_ERR_NOT_SUPPORTED_IE);
    }

    return (FHAPI_STATUS_SUCCESS);
}
#endif
#endif
