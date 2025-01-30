/******************************************************************************

 @file  mac_hl_patch.h

 @brief This file includes typedefs and functions required for high level mac patches.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated

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

#ifndef MAC_HL_PATCH_H
#define MAC_HL_PATCH_H
#if defined(TIMAC_ROM_PATCH)
#include "mac_main.h"
#include "mac_mgmt.h"
#include "mac_data.h"

#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
#include "mac_timer.h"
#include "mac_beacon_device.h"
#include "mac_symbol_timer.h"
#endif //defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
#endif

#ifdef FEATURE_EDFE_TEST_MODE
extern uint8_t switchToEDFE;
#endif
/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define NUM_PHY_FSK_RATES       (5)
#define PHY_MODE_UNDEF          (0)
#define PHY_MODE_STD            (1) //not used in wi-sun
#define PHY_MODE_GEN            (2) //not used in wi-sun
#define PHY_MODE_FSK_50K        (1)
#define PHY_MODE_FSK_100K       (2)
#define PHY_MODE_FSK_150K       (3)
#define PHY_MODE_FSK_200K       (4)
#define PHY_MODE_FSK_300K       (5)
#define PHY_MODE_SLR_5K         (6) //not used in wi-sun

extern uint32_t phyModeFSKdataRate[NUM_PHY_FSK_RATES];

#define MAC_RSSI_NOT_MEASURED   (0x80)
#define MAC_RSL_THERMAL_NOISE   (174)
#define MAC_RSL_MAX             (80)
#define MAC_RSL_NOT_MEASURED    (MAC_RSL_MAX + 1)
/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */

/*--------------------------------------------------------------------------------------------------
*           Typedef
*---------------------------------------------------------------------------------------------------
*/
typedef struct AESCCM_Transaction {
    uint8 securityLevel;
    uint8 *pKey;
    uint8 *header;
    uint8 *data;
    uint8 *mic;
    uint8 *nonce;
    uint16 headerLength;
    uint16 dataLength;
    uint16 micLength;
} AESCCM_Transaction;

/******************************************************************************
 Global Externs
 *****************************************************************************/
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
extern macEdfeInfo_t macEdfeInfo;
extern uint8_t edfeEfrmBuf[MAC_EDFE_MAX_FRAME];
#endif

/*--------------------------------------------------------------------------------------------------
*           Functions
*---------------------------------------------------------------------------------------------------
*/

uint8 macCheckPhyRate(uint8 phyID);
uint8 macCheckPhyMode(uint8 phyID);
uint8 macPibIndex(uint8 pibAttribute);
uint8 macPibCheckByPatch(uint8 pibAttribute, void *pValue);
void macSetDefaultsByPatch(uint8 pibAttribute);
uint8 macCcmEncrypt(AESCCM_Transaction *trans);
uint8 macCcmDecrypt(AESCCM_Transaction *trans);

#if defined(TIMAC_ROM_PATCH)

#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
void macTrackTimeoutCallback(uint8 param);
void macTrackStartCallback(uint8 param);
void macBeaconStopTrack(void);
#endif

MAC_INTERNAL_API void macScanRxCoordRealign(macEvent_t *pEvent);
MAC_INTERNAL_API void macScanNextChan(macEvent_t *pEvent);
MAC_INTERNAL_API void macEdScanISR(void *arg);
MAC_INTERNAL_API void macBeaconSyncLoss(void);

uint8 macWrapperAddDevice(ApiMac_secAddDevice_t *param);
uint8 macWrapperDeleteDevice(ApiMac_sAddrExt_t *param);
uint8 macWrapperAddKeyInitFCtr( ApiMac_secAddKeyInitFrameCounter_t *param );

MAC_INTERNAL_API uint8 macIncomingFrameSecurity( macRx_t *pMsg );
MAC_INTERNAL_API uint8 macCcmStarTransform( uint8    *pKey,
                                            uint32   frameCounter,
                                            uint8    securityLevel,
                                            uint8    *pAData,
                                            uint16   aDataLen,
                                            uint8    *pMData,
                                            uint16   mDataLen );
MAC_INTERNAL_API uint8 macOutgoingFrameSecurity( macTx_t  *pBuf,
                                                 sAddr_t  *pDstAddr,
                                                 uint16   dstPanId,
                                                 keyDescriptor_t **ppKeyDesc );

MAC_INTERNAL_API uint8 macBeaconCheckStartTime(macEvent_t *pEvent);

MAC_INTERNAL_API uint8_t getLostBeaconCount(void);

uint8 macCheckPendAddr(uint8 pendAddrSpec, uint8 *pAddrList);

void MAC_ResumeReq(void);

MAC_INTERNAL_API macTx_t *macAllocTxBuffer(uint8 cmd, ApiMac_sec_t *sec);
MAC_INTERNAL_API uint8 macBuildDataFrame(macEvent_t *pEvent);
MAC_INTERNAL_API void macDataTxComplete(macTx_t *pMsg);
MAC_INTERNAL_API uint8 macBuildHeaderIes(macTx_t* pBuf, headerIeInfo_t *pHeaderIes );
void macUpdateHeaderIEPtr(macRx_t *pMsg);
void macMgmtReset(void);
MAC_INTERNAL_API uint8 macBuildHeader(macTx_t *pBuf, uint8 srcAddrMode, sAddr_t *pDstAddr, uint16 dstPanId, uint8 elideSeqNo);
MAC_INTERNAL_API uint8 macBuildCommonReq(uint8 cmd, uint8 srcAddrMode, sAddr_t *pDstAddr,
                                         uint16 dstPanId, uint16 txOptions, ApiMac_sec_t *sec);
MAC_INTERNAL_API void macApiDataReq(macEvent_t *pEvent);
uint8 MAC_McpsDataReq(ApiMac_mcpsDataReq_t* pData);
macMcpsDataReq_t *MAC_McpsDataAlloc(uint16 len, uint8 securityLevel, uint8 keyIdMode,
                                    uint32 includeFhIEs, uint16 payloadIeLen);
MAC_INTERNAL_API void macTxCsmaDelay(macEvent_t *pEvent);
void MAC_InitFH(void);
void macUpdatePayloadIEPtr(macRx_t *pMsg);

#ifdef FEATURE_FREQ_HOP_MODE
extern MAC_INTERNAL_API void macApiWSAsyncReq(macEvent_t *pEvent);
#endif /* FEATURE_FREQ_HOP_MODE */

MAC_INTERNAL_API void macTxBackoffHandler(macEvent_t *pEvent);
MAC_INTERNAL_API void macRxBackoffHandler(macEvent_t *pEvent);

#ifdef FEATURE_WISUN_SUPPORT
MAC_INTERNAL_API void macApiNodeStartReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macNodeStartComplete(macEvent_t *pEvent);
#endif

#endif /* defined(TIMAC_ROM_PATCH) */

#endif /* MAC_HL_PATCH_H */

