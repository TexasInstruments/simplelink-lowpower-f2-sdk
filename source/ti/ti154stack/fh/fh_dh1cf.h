/******************************************************************************

 @file fh_dh1cf.h

 @brief TIMAC 2.0 FH DH1CF API

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

#ifndef _FH_DH1CF_H
#define _FH_DH1CF_H

#include "hal_types.h"
#include "fh_nt.h"

#define FH_DH1CF
/******************************************************************************
 * MACROS
 ******************************************************************************/
/*
 * The defines below are from WiSUN spec. They thus do not follow
 * TI Coding guidelines
 */
#define hashsize(n) ((uint32_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/*!
 * @brief       This function calculates hash index.
 *              Taken from the WiSUN standard.
 *
 * @param       k - pointer to key, an array of uint32_t values
 * @param       length -  the length of the key
 * @param       initval - the previous hash, or an arbitrary value
 *
 * @return       uint32_t  - hash value
 */
uint32_t FHDH1CF_hashword(
        uint32_t        *k,
        uint32_t        length,
        uint32_t        initval);

/*!
 * @brief     This function calculates the channel at a given slot.
 *            Taken from WiSUN standard.
 *
 * @param       slotNum - The slot at which channel has to be computed
 * @param       devExtAddr - The EUI of the node for whom tart channel is to
 *              be computed
 * @param       numChannels - The total number of channels
 *
 * @return      uint8_t - Channel number
 */
uint8_t FHDH1CF_getCh(uint16_t slotNum,
                      sAddrExt_t  devExtAddr,
                      uint16_t numChannels);

/*!
 * @brief   This function maps hash index to actual channel number
 *
 * @param   chIdx - the index for the channel
 * @param   exclChList - The exclude channel list
 * @param   nChannels - total number of possible channels
 *
 * @return  uint8_t - mapped channel number
 */
uint8_t FHDH1CF_mapChIdxChannel(uint8_t  chIdx,
                                uint8_t  *exclChList,
                                uint8_t  nChannels);


/*!
 * @brief       This function calculates broadcast channel at a given slot
 *
 * @param       slotNum - the broadcast slot number
 * @param       bsi - broadcast slot index
 * @param       numChannels - total number of broadcast channels
 *
 * @return      uint8_t - the broadcast channel number
 */
uint8_t FHDH1CF_getBcCh(uint32_t slotNum,
                          uint16_t bsi,
	                      uint16_t numChannels);

/*!
 * @brief       Wrapper function to get broadcast channel for a given slot
 *
 * @param       slotIdx - Slot index
 * @param       numChannels - total number of channels
 *
 * @return      uint8_t - correspoding broadcast channel number.
 */
uint8_t FHDH1CF_getBCChannelNum(uint16_t slotIdx, uint8_t numChannels);

/*!
 * @brief       Wrapper function to get node's channel at a given time
 *
 * @param       pFH_hnd - pointer to FH handle
 *
 * @return      uint8_t - correspoding channel number.
 */
uint8_t FHDH1CF_getChannelNum(FH_HND_s *pFH_hnd);

/*!
 * @brief       Wrapper function to get target's channel at a given slot
 *
 * @param       slotIdx - slot index
 * @param       pEntry - pointer to target node's neighbor table entry
 *
 * @return      uint8_t - correspoding channel number.
 */
uint8_t FHDH1CF_getTxChannelNum(uint16_t slotIdx, NODE_ENTRY_s *pEntry);

#endif





