/******************************************************************************

 @file fh_dh1cf.c

 @brief TIMAC 2.0 FH DH1CF API

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

#include "fh_api.h"
#include "fh_pib.h"
#include "fh_data.h"
#include "fh_dh1cf.h"
#include "fh_nt.h"
#include "fh_util.h"
#include "fh_ie.h"
/* ----------------------------------------------------------------------------
 *                                            Defines
 * ----------------------------------------------------------------------------
 */

#ifdef FEATURE_FREQ_HOP_MODE

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/*!
 * This function calculates hash index. Taken from the WiSUN standard.
 *
 * Public function defined in fh_dh1cf.h
 */

uint32_t FHDH1CF_hashword(
        uint32_t        *k,
        uint32_t        length,
        uint32_t        initval)
{
    uint32_t a,b,c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((uint32_t)length) << 2) + initval;

    /*----------------------------------------- handle most of the key */
    while (length > 3)
    {
        a += k[0];
        b += k[1];
        c += k[2];
        mix(a,b,c);
        length -= 3;
        k += 3;
    }

    /*--------------------------------------- handle the last 3 uint32_t's */
    switch(length)                  /* all the case statements fall through */
    {
        case 3 :
            c+=k[2];
        case 2 :
            b+=k[1];
        case 1 :
            a+=k[0];
            final(a, b, c);
        case 0:     /* case 0: nothing left to add */
            break;
    }
    /*--------------------------------------------------report the result */
    return c;
}

/*!
 * This function calculates the channel at a given slot.
 * Taken from WiSUN standard.
 *
 * Public function defined in fh_dh1cf.h
 */
uint8_t FHDH1CF_getCh(uint16_t slotNum,
                      sAddrExt_t  devExtAddr,
                      uint16_t numChannels)
{
    uint32_t channelNumber;
    uint32_t x[3];
    uint8_t channel;

   /* numChannels = FH_hnd.ucNumChannels; */
#ifdef FH_DH1CF_TEST
    uint8_t temp8[8] = {0x0, 0x13, 0x50, 0x04, 0x0, 0x0, 0x05, 0xf8};

    MAP_osal_memcpy(devExtAddr, temp8, sizeof(temp8));
    slotNum = 10738;
    numChannels = 129;
#endif
    /* slot # */
    x[0] = (uint32_t)slotNum;
    /* EUI */
    x[1] = (devExtAddr[4] << 24) | (devExtAddr[5] << 16) |
           (devExtAddr[6] << 8) | (devExtAddr[7]);

    x[2] = (devExtAddr[0] << 24) | (devExtAddr[1] << 16) |
           (devExtAddr[2] << 8) | (devExtAddr[3]);

    /* modulo nChannels */
    channelNumber = MAP_FHDH1CF_hashword(x, 3, 0);

    channel = channelNumber % numChannels;

    return channel;
}

/*!
 * This function calculates broadcast channel at a given slot
 *
 * Public function defined in fh_dh1cf.h
 */
uint8_t FHDH1CF_getBcCh(uint32_t slotNum,
                          uint16_t bsi,
                          uint16_t numChannels)
{
    uint32_t channelNumber;
    uint32_t x[3];
    uint8_t channel;

    x[0] = slotNum;

    x[1] = (uint32_t)(bsi << 16);

    x[2] = 0;

    channelNumber = MAP_FHDH1CF_hashword(x, 3, 0);

    channel = channelNumber % numChannels;

    return channel;

}

/*!
 * This function maps hash index to actual channel number
 *
 * Public function defined in fh_dh1cf.h
 */
uint8_t FHDH1CF_mapChIdxChannel(uint8_t  chIdx,
                                uint8_t  *exclChList,
                                uint8_t  nChannels)
{
    uint8_t channel = 0, chCnt = 0, i;
    uint8_t temp, found = 0;

    while(((chCnt < nChannels) && !found))
    {
        temp = *exclChList++;
        for(i = 0; i < 8; i++)
        {
            /* check for channels present */
            channel += ((temp & 1) ^ 1);
            temp >>= 1;
            if(chIdx == (channel - 1))
            {
                found = 1;
                break;
            }
            chCnt += 1;
        }

    }
    if(!found)
    {
        MAP_FHUTIL_assert(NULL);
    }


    return chCnt;

}

/*!
 * Wrapper function to get broadcast channel for a given slot
 *
 * Public function defined in fh_dh1cf.h
 */
uint8_t FHDH1CF_getBCChannelNum(uint16_t slotIdx, uint8_t numChannels)
{
    uint8_t  channel;
    uint8_t  chFunc;
    uint16_t bcFixedChannel;
    uint8_t  exclChList[FHPIB_MAX_BIT_MAP_SIZE];

    MAP_FHPIB_get(FHPIB_BC_CHANNEL_FUNCTION, &chFunc);
    if(chFunc == FHIE_CF_SINGLE_CHANNEL)
    {   /* fixed BC channel */
        MAP_FHPIB_get(FHPIB_BC_FIXED_CHANNEL, &bcFixedChannel);
        return bcFixedChannel;
    }

#ifdef FH_DH1CF
    uint16_t bcSchedId;
    MAP_FHPIB_get(FHPIB_BROCAST_SCHED_ID , &bcSchedId);

    channel = MAP_FHDH1CF_getBcCh(slotIdx, bcSchedId, numChannels);
#else
    channel = slotIdx % numChannels;
#endif

    MAP_FHPIB_get(FHPIB_BC_EXCLUDED_CHANNELS, exclChList);
    channel = MAP_FHDH1CF_mapChIdxChannel(channel,
                                      exclChList,
                                      MAP_FHUTIL_getMaxChannels());

    return channel;
}

/*!
 * Wrapper function to get node's channel at a given time
 *
 * Public function defined in fh_dh1cf.h
 */
uint8_t FHDH1CF_getChannelNum(FH_HND_s *pFH_hnd)
{
    uint8_t channel;
    uint8_t exclChList[FHPIB_MAX_BIT_MAP_SIZE];

#ifdef FH_DH1CF
    sAddrExt_t    extAddr;

    MAP_MAC_MlmeGetReq(MAC_EXTENDED_ADDRESS, &extAddr);
    channel = MAP_FHDH1CF_getCh(pFH_hnd->ucSlotIdx, extAddr,
                            pFH_hnd->ucNumChannels);
#else
    channel = pFH_hnd->ucSlotIdx % pFH_hnd->ucNumChannels;
#endif

    MAP_FHPIB_get(FHPIB_UC_EXCLUDED_CHANNELS, exclChList);
    channel = MAP_FHDH1CF_mapChIdxChannel(channel,
                                      exclChList,
                                      MAP_FHUTIL_getMaxChannels());

    return channel;
}

/*!
 * Wrapper function to get target's channel at a given slot
 *
 * Public function defined in fh_dh1cf.h
 */
uint8_t FHDH1CF_getTxChannelNum(uint16_t slotIdx, NODE_ENTRY_s *pEntry)
{
    uint8_t channel;

#ifdef FH_DH1CF
    sAddrExt_t *pEUI;
    pEUI = MAP_FHNT_getEUI(pEntry->EUI_index);
#ifdef FEATURE_WISUN_SUPPORT
    sAddrExt_t eui;
    memcpy(&eui, pEUI, 8);
    channel = MAP_FHDH1CF_getCh(slotIdx, (uint8_t *)&eui,
                            pEntry->UsieParams_s.numChannels);
#else
    //channel = FHDH1CF_getCh(slotIdx, (uint8_t *)&pEntry->dstAddr,
    channel = MAP_FHDH1CF_getCh(slotIdx, (uint8_t *)pEUI,
                            pEntry->UsieParams_s.numChannels);
#endif
#else
    channel = slotIdx % pEntry->UsieParams_s.numChannels;
#endif

    channel = MAP_FHDH1CF_mapChIdxChannel(channel,
                                      pEntry->UsieParams_s.chInfo.bitMap,
                                      MAP_FHUTIL_getMaxChannels());

    return channel;
}

#endif
