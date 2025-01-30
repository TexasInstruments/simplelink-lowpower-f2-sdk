/******************************************************************************

 @file fh_ie_patch.h

 @brief TIMAC 2.0 FH IE PATCH API

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

#ifndef _FHIE_PATCH_H_
#define _FHIE_PATCH_H_

/******************************************************************************
 Includes
 *****************************************************************************/

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Typedefs
 *****************************************************************************/

/******************************************************************************
 Global Externs
 *****************************************************************************/
#ifdef FEATURE_WISUN_SUPPORT
extern const FHPIB_DB_new_t FHPIB_defaults_new;
extern FHPIB_DB_new_t FHPIB_db_new;
#endif
/******************************************************************************
 Prototypes
 *****************************************************************************/

FHAPI_status FHIEPATCH_extractHie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint8_t *pOut);
FHAPI_status FHIEPATCH_extractPie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint8_t *pOut);
uint16_t FHIEPATCH_getHieLen(uint32_t ieBitmap);
uint8_t FHIEPATCH_getPieId(uint32_t ieBitmap, uint8_t *pType);
uint16_t FHIEPATCH_getPieContentLen(uint8_t id);
uint16_t FHIEPATCH_genPieContent(uint8_t *pData, uint8_t id);
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
uint16_t FHIEPATCH_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_new_t *pIeInfo);
#else
uint16_t FHIEPATCH_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo);
#endif
FHAPI_status FHIEPATCH_parsePie(uint8_t subIeId, uint16_t subIeLength, uint8_t *pBuf, sAddrExt_t *pSrcAddr);
FHAPI_status FHIEPATCH_parseHie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint32_t ts, sAddrExt_t *pSrcAddr);

#if defined(TIMAC_ROM_PATCH)
MAC_INTERNAL_API void  FHAPI_sendData(void);
void FHDATA_startEDFEReq(void *pData);
void FHDATA_procEDFEReq(void *pData);
void FHDATA_compEDFEReq(uint8_t status);
uint16_t FHIE_getPieContentLen(uint8_t id);
uint16_t FHIE_getHieLen(uint32_t bitmap);
#ifdef FEATURE_WISUN_EDFE_SUPPORT
void FHIE_genUTIE(uint8_t *pBuf, uint8_t fhFrameType, uint32_t offset);
void FHIE_genBTIE(uint8_t *pBuf, uint8_t frameType, uint32_t offset);
#else
void FHIE_genUTIE(uint8_t *pBuf, uint8_t fhFrameType);
void FHIE_genBTIE(uint8_t *pBuf, uint8_t frameType);
#endif
uint16_t FHIE_genPieContent(uint8_t *pData, uint8_t id);

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
uint16_t FHIE_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_new_t *pIeInfo);
uint16_t FHIE_genPie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx, FHIE_ieInfo_new_t *pIeInfo);
MAC_INTERNAL_API uint16_t FHIE_gen(uint8_t *pData, uint32_t bitmap,
                                   macTx_t *pMacTx, FHIE_ieInfo_new_t *pIeInfo);
#else
uint16_t FHIE_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo);
uint16_t FHIE_genPie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx, FHIE_ieInfo_t *pIeInfo);
MAC_INTERNAL_API uint16_t FHIE_gen(uint8_t *pData, uint32_t bitmap,
                                   macTx_t *pMacTx, FHIE_ieInfo_t *pIeInfo);
#endif
MAC_INTERNAL_API FHAPI_status FHIE_parseHie(uint8_t *pData,
                                            sAddrExt_t *pSrcAddr,
                                            uint32_t ts,
                                            uint16_t *pLen1,
                                            uint16_t *pLen2);
MAC_INTERNAL_API FHAPI_status FHIE_extractPie(uint8_t *pData,
                                              IE_TYPE_t ieType,
                                              uint8_t *pOut);
MAC_INTERNAL_API FHAPI_status FHIE_extractHie(uint8_t *pData,
                                              IE_TYPE_t ieType,
                                              uint8_t *pOut);
uint8_t FHUTIl_updateBTIE(uint32_t bfio, uint16_t slotIdx);
MAC_INTERNAL_API void FHAPI_completeTxCb(uint8_t status);
void FHMGR_bcTimerIsrCb(uint8_t parameter);
void FHMGR_updateRadioUCChannel(void *pData);

MAC_INTERNAL_API void FHPIB_reset(void);

extern uint16_t FHIE_getCommonInformationLen(uint8_t cp, uint8_t cf,
                                             uint8_t ecc,
                                             FHIE_channelPlan_t *pChPlan,
                                             uint8_t ucFlag);
extern uint16_t FHIE_genCommonInformation(uint8_t *pData, uint8_t cp, uint8_t cf, uint8_t ecc,
                                          FHIE_channelPlan_t *pChPlan, uint8_t ucFlag);
extern uint32 FHIE_getCcaSfdTime(uint8 fhFrameType);
extern uint8_t FHIE_getChannelPlan(FHIE_channelPlan_t *pChPlan);
extern uint8_t FHIE_getExcludedChannelControl(uint8_t id);
extern uint16_t FHIE_getSubPie(uint8_t *pData, FHIE_ie_t *subPie);
uint16_t FHIE_getHie(uint8_t *pData, FHIE_ie_t *hie);
extern uint16_t FHIE_parseCommonInformation(uint8_t *pData, uint8_t chinfo,
                                            usieParams_t *pUsIeParams,
                                            uint8_t ucFlag);
extern uint32_t FHUTIL_compBfioTxTime(void);
extern uint16_t FHIE_getPie(uint8_t *pData, FHIE_ie_t *pie);
#endif

#endif
