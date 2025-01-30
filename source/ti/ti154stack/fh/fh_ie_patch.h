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

/******************************************************************************
 Prototypes
 *****************************************************************************/
uint16_t FHNT_delTempIndex(uint16_t eui_index);
FHAPI_status FHIEPATCH_extractHie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint8_t *pOut);
FHAPI_status FHIEPATCH_extractPie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint8_t *pOut);
uint16_t FHIEPATCH_getHieLen(uint32_t ieBitmap);
uint8_t FHIEPATCH_getPieId(uint32_t ieBitmap, uint8_t *pType);
uint16_t FHIEPATCH_getPieContentLen(uint8_t id);
uint16_t FHIEPATCH_genPieContent(uint8_t *pData, uint8_t id);
uint16_t FHIEPATCH_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo);
FHAPI_status FHIEPATCH_parsePie(uint8_t subIeId, uint16_t subIeLength, uint8_t *pBuf, sAddrExt_t *pSrcAddr);
FHAPI_status FHIEPATCH_parseHie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint32_t ts, sAddrExt_t *pSrcAddr);
uint32 FHIE_getCcaSfdTime(uint8 fhFrameType);
void FHIE_genBTIE(uint8_t *pBuf, uint8_t frameType);
MAC_INTERNAL_API FHAPI_status FHAPI_getTxParams(uint32_t *pBackOffDur, uint8_t *pChIdx);
uint32_t FHDATA_CheckMinTxOffTime(uint8_t txChan,uint32_t csmaDelay);
uint32_t FHDATA_getASYNCDelay(void);
#endif
