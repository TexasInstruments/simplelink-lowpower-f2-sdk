/******************************************************************************

 @file  fh_rom_init.c

 @brief This file contains the externs for FH ROM API initialization.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated

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

/*******************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "fh_api.h"
#include "fh_data.h"
#include "fh_dh1cf.h"
#include "fh_ie.h"
#include "fh_mgr.h"
#include "fh_nt.h"
#include "fh_pib.h"
#include "fh_sm.h"
#include "fh_util.h"
#ifdef FEATURE_FREQ_HOP_MODE
extern uint32 FHIE_getCcaSfdTime(uint8 fhFrameType);
extern void FHIE_genUTIE(uint8_t *pBuf, uint8_t fhFrameType);
extern void FHIE_genBTIE(uint8_t *pBuf, uint8_t frameType);
extern uint8_t FHIE_getChannelPlan(FHIE_channelPlan_t *pChPlan);
extern uint8_t FHIE_isChannelPlanValid(uint8_t channelPlan,FHIE_channelPlan_t *pChPlan);
extern uint16_t FHIE_genExcludedChannelMask(uint8_t *pData,uint8_t *pExcludedChannelMask, uint8_t id);
extern uint16_t FHIE_genExcludedChannelRanges(uint8_t *pData, uint8_t id);
extern uint8_t FHIE_getExcludedChannelControl(uint8_t id);
extern uint16_t FHIE_genCommonInformation(uint8_t *pData, uint8_t cp, uint8_t cf, uint8_t ecc,
                                          FHIE_channelPlan_t *pChPlan, uint8_t ucFlag);
extern uint16_t FHIE_genPieContent(uint8_t *pData, uint8_t id);
extern uint16_t FHIE_genPie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx, FHIE_ieInfo_t *pIeInfo);
extern uint16_t FHIE_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx, FHIE_ieInfo_t *pIeInfo);
extern uint16_t FHIE_getCommonInformationLen(uint8_t cp, uint8_t cf,uint8_t ecc, FHIE_channelPlan_t *pChPlan, uint8_t ucFlag);
extern uint16_t FHIE_getPieContentLen(uint8_t id);
extern uint16_t FHIE_getPieLen(uint32_t bitmap);
extern uint16_t FHIE_getHieLen(uint32_t bitmap);
extern uint16_t FHIE_parseCommonInformation(uint8_t *pData, uint8_t chinfo,
                                            usieParams_t *pUsIeParams,
                                            uint8_t ucFlag);
extern uint16_t FHIE_getPie(uint8_t *pData, FHIE_ie_t *pie);
extern uint16_t FHIE_getSubPie(uint8_t *pData, FHIE_ie_t *subPie);
extern uint16_t FHIE_getHie(uint8_t *pData, FHIE_ie_t *hie);

extern uint16_t FHNT_getFixedNTIndexFromTempIndex(uint16_t tempIndex);
extern uint16_t FHNT_getNTIndexFromTempIndex(uint16_t tempIndex,uint8_t *pTableType);
extern void FHNT_getRemoveEntryFromTemp(void);
extern NODE_FIXED_ENTRY_s *FHNT_getRemoveFixedEntry(void);
extern uint16_t FHNT_addTempEUI(sAddrExt_t *peui);
extern uint8_t FHNT_assessTime(uint32_t curTime,
                               uint32_t oldTime,
                               uint32_t validTime);

extern uint16_t FHNT_delTempIndex(uint16_t eui_index);
extern NODE_OPT_ENTRY_s *FHNT_getAvailEntry(sAddr_t *pSrcAddr);
extern NODE_ENTRY_s *FHNT_getRemoveEntry(void);
extern void FHNT_purgeTimerIsrCb(uint8_t parameter);
extern uint16_t FHPIB_getIndex(uint16_t fhPibId);

extern uint8_t FHUTIL_getUcChannel(NODE_ENTRY_s *pEntry, uint32_t txUfsi);
extern uint32_t FHUTIL_compBfioTxTime(void);

#ifdef FHIEPATCH
extern FHAPI_status FHIEPATCH_extractHie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint8_t *pOut);
extern FHAPI_status FHIEPATCH_extractPie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint8_t *pOut);
extern uint16_t FHIEPATCH_getHieLen(uint32_t ieBitmap);
extern uint8_t FHIEPATCH_getPieId(uint32_t ieBitmap, uint8_t *pType);
extern uint16_t FHIEPATCH_getPieContentLen(uint8_t id);
extern uint16_t FHIEPATCH_genPieContent(uint8_t *pData, uint8_t id);
extern uint16_t FHIEPATCH_genHie(uint8_t *pData, uint32_t bitmap, macTx_t *pMacTx,
                            FHIE_ieInfo_t *pIeInfo);
extern FHAPI_status FHIEPATCH_parsePie(uint8_t subIeId, uint16_t subIeLength, uint8_t *pBuf, sAddrExt_t *pSrcAddr);
extern FHAPI_status FHIEPATCH_parseHie(uint8_t subIeId, uint16_t ieLength, uint8_t *pBuf, uint32_t ts, sAddrExt_t *pSrcAddr);
#endif

#include "rom_jt_def_154.h"

/*******************************************************************************
 * EXTERNS
 */

/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES declaration
 */

extern FH_HND_s FH_hnd;
extern FHNT_HND_s FHNT_hnd;
extern FH_TX_TIMING_TABLE_s FH_txTiming;
extern FHNT_TEMP_TABLE_s FHNT_tempTable;
extern FHIE_channelPlan_t FHIE_channelPlan[];
extern FHNT_DEBUG_s FHNT_Dbg;
extern const FHPIB_DB_t FHPIB_defaults;
extern const FH_PibTbl_t FH_PibTbl[];
extern FHPIB_DB_t FHPIB_db;
extern FHFSM_debugBuffer_t FHFSM_Dbg;
extern FHNT_OPT_Table_s FHNT_tableOpt;
extern FSM_stateMachine_t FH_FSM;
extern uint8 macAddrSize[4];
extern const uint8_t  fhSupFrameTypes[FH_SUPPORTED_FRAME_TYPES];

uint32 FH_test_var;

// ROM Flash Jump Table

#if defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(FH_ROM_Flash_JT, 4)
#elif defined(__GNUC__) || defined(__clang__)
__attribute__ ((aligned (4)))
#else
#pragma data_alignment=4
#endif

const uint32 FH_ROM_Flash_JT[] =
{
	/* global variable jump table */
#if !defined(FREQ_2_4G)
#if defined(FEATURE_FREQ_HOP_MODE)
    (uint32)&FH_hnd,                                            //ROM_FH_JT_OFFSET[0]
	(uint32)&FHNT_hnd,											//ROM_FH_JT_OFFSET[1]
    (uint32)&FH_txTiming,                                       //ROM_FH_JT_OFFSET[2]
    (uint32)&FHNT_tempTable,                                    //ROM_FH_JT_OFFSET[3]
    (uint32)&FHIE_channelPlan,                                  //ROM_FH_JT_OFFSET[4]
    (uint32)&FHNT_Dbg,                                    		//ROM_FH_JT_OFFSET[5]
    (uint32)&FHPIB_defaults,                                    //ROM_FH_JT_OFFSET[6]
    (uint32)&FH_PibTbl,                                    		//ROM_FH_JT_OFFSET[7]
    (uint32)&FHPIB_db,                                    		//ROM_FH_JT_OFFSET[8]
    (uint32)&FHNT_tableOpt,                                     //ROM_FH_JT_OFFSET[9]
    (uint32)&FHFSM_Dbg,                                         //ROM_FH_JT_OFFSET[10]
    (uint32)&FH_FSM,                                            //ROM_FH_JT_OFFSET[11]
    (uint32)&macAddrSize,                                       //ROM_FH_JT_OFFSET[12]
    (uint32)&fhSupFrameTypes,                                   //ROM_FH_JT_OFFSET[13]
#else
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[0]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[1]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[2]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[3]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[4]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[5]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[6]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[7]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[8]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[9]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[10]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[11]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[12]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[13]
#endif

#else
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[0]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[1]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[2]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[3]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[4]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[5]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[6]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[7]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[8]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[9]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[10]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[11]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[12]
    (uint32)NULL,                                               //ROM_FH_JT_OFFSET[13]
#endif
    /* FH API.c */
    (uint32)&FHAPI_reset,                                                    //ROM_FH_JT_OFFSET[0]
    (uint32)&FHAPI_start,                                                    //ROM_FH_JT_OFFSET[1]
    (uint32)&FHAPI_startBS,                                                  //ROM_FH_JT_OFFSET[2]
    (uint32)&FHAPI_completeTxCb,                                             //ROM_FH_JT_OFFSET[3]
    (uint32)&FHAPI_completeRxCb,                                             //ROM_FH_JT_OFFSET[4]
    (uint32)&FHAPI_sendData,                                                 //ROM_FH_JT_OFFSET[5]
    (uint32)&FHAPI_getTxParams_old,                                          //ROM_FH_JT_OFFSET[6]
    (uint32)&FHAPI_getTxParams,                                              //ROM_FH_JT_OFFSET[7]
    (uint32)&FHAPI_getTxChHoppingFunc,                                       //ROM_FH_JT_OFFSET[8]
    (uint32)&FHAPI_getRemDT,                                                 //ROM_FH_JT_OFFSET[9]
    (uint32)&FHAPI_stopAsync,                                                //ROM_FH_JT_OFFSET[10]
    (uint32)&FHAPI_setStateCb,                                               //ROM_FH_JT_OFFSET[11]
    (uint32)&FHAPI_SFDRxCb,                                                  //ROM_FH_JT_OFFSET[12]

    /* fh_data.c */
    (uint32)&FHDATA_checkPktFromQueue,                               //ROM_FH_JT_OFFSET[13]
    (uint32)&FHDATA_getPktFromQueue,                                 //ROM_FH_JT_OFFSET[14]
    (uint32)&FHDATA_purgePktFromQueue,                               //ROM_FH_JT_OFFSET[15]
    (uint32)&FHDATA_purgeUnSupportedFrameTypes,                      //ROM_FH_JT_OFFSET[16]
    (uint32)&FHDATA_chkASYNCStartReq,                                //ROM_FH_JT_OFFSET[17]
    (uint32)&FHDATA_procASYNCReq,                                    //ROM_FH_JT_OFFSET[18]
    (uint32)&FHDATA_startEDFEReq,                                    //ROM_FH_JT_OFFSET[19]
    (uint32)&FHDATA_procEDFEReq,                                     //ROM_FH_JT_OFFSET[20]
    (uint32)&FHDATA_startEDFETimer,                                  //ROM_FH_JT_OFFSET[21]
    (uint32)&FHDATA_requeuePkt,                                      //ROM_FH_JT_OFFSET[22]
    (uint32)&FHDATA_updateCCA,                                       //ROM_FH_JT_OFFSET[23]
    (uint32)&FHDATA_CheckMinTxOffTime,                               //ROM_FH_JT_OFFSET[24]
    (uint32)&FHDATA_getASYNCDelay,                                   //ROM_FH_JT_OFFSET[25]
    (uint32)&FHDATA_asyncMinTxOffTimerCb,                            //ROM_FH_JT_OFFSET[26]

    /* fh_dh1cf.c */
    (uint32)&FHDH1CF_hashword,                                   //ROM_FH_JT_OFFSET[27]
    (uint32)&FHDH1CF_getCh,                                      //ROM_FH_JT_OFFSET[28]
    (uint32)&FHDH1CF_getBcCh,                                    //ROM_FH_JT_OFFSET[29]
    (uint32)&FHDH1CF_mapChIdxChannel,                            //ROM_FH_JT_OFFSET[30]
    (uint32)&FHDH1CF_getBCChannelNum,                            //ROM_FH_JT_OFFSET[31]
    (uint32)&FHDH1CF_getChannelNum,                              //ROM_FH_JT_OFFSET[32]
    (uint32)&FHDH1CF_getTxChannelNum,                            //ROM_FH_JT_OFFSET[33]

    /* fh_ie.c */
    (uint32)&FHIE_getCcaSfdTime,                         //ROM_FH_JT_OFFSET[34]
    (uint32)&FHIE_genUTIE,                               //ROM_FH_JT_OFFSET[35]
    (uint32)&FHIE_genBTIE,                               //ROM_FH_JT_OFFSET[36]
    (uint32)&FHIE_getChannelPlan,                        //ROM_FH_JT_OFFSET[37]
    (uint32)&FHIE_isChannelPlanValid,                    //ROM_FH_JT_OFFSET[38]
    (uint32)&FHIE_genExcludedChannelMask,                //ROM_FH_JT_OFFSET[39]
    (uint32)&FHIE_genExcludedChannelRanges,              //ROM_FH_JT_OFFSET[40]
    (uint32)&FHIE_getExcludedChannelControl,             //ROM_FH_JT_OFFSET[41]
    (uint32)&FHIE_genCommonInformation,                  //ROM_FH_JT_OFFSET[42]
    (uint32)&FHIE_genPieContent,                         //ROM_FH_JT_OFFSET[43]
    (uint32)&FHIE_genPie,                                //ROM_FH_JT_OFFSET[44]
    (uint32)&FHIE_genHie,                                //ROM_FH_JT_OFFSET[45]
    (uint32)&FHIE_getCommonInformationLen,               //ROM_FH_JT_OFFSET[46]
    (uint32)&FHIE_getPieContentLen,                      //ROM_FH_JT_OFFSET[47]
    (uint32)&FHIE_getPieLen,                             //ROM_FH_JT_OFFSET[48]
    (uint32)&FHIE_getHieLen,                             //ROM_FH_JT_OFFSET[49]
    (uint32)&FHIE_parseCommonInformation,                //ROM_FH_JT_OFFSET[50]
    (uint32)&FHIE_getPie,                                //ROM_FH_JT_OFFSET[51]
    (uint32)&FHIE_getSubPie,                             //ROM_FH_JT_OFFSET[52]
    (uint32)&FHIE_getHie,                                //ROM_FH_JT_OFFSET[53]
    (uint32)&FHIE_gen,                                   //ROM_FH_JT_OFFSET[54]
    (uint32)&FHIE_getLen,                                //ROM_FH_JT_OFFSET[55]
    (uint32)&FHIE_parsePie,                              //ROM_FH_JT_OFFSET[56]
    (uint32)&FHIE_parseHie,                              //ROM_FH_JT_OFFSET[57]
    (uint32)&FHIE_extractPie,                            //ROM_FH_JT_OFFSET[58]
    (uint32)&FHIE_extractHie,                            //ROM_FH_JT_OFFSET[59]

    /* fh_mgr.c */
    (uint32)&FHMGR_macStartOneShotTimer,                 //ROM_FH_JT_OFFSET[60]
    (uint32)&FHMGR_macStartFHTimer,                      //ROM_FH_JT_OFFSET[61]
    (uint32)&FHMGR_macGetElapsedFHTime,                  //ROM_FH_JT_OFFSET[62]
    (uint32)&FHMGR_ucTimerIsrCb,                         //ROM_FH_JT_OFFSET[63]
    (uint32)&FHMGR_BCTimerEventUpd,                      //ROM_FH_JT_OFFSET[64]
    (uint32)&FHMGR_bcTimerIsrCb,                         //ROM_FH_JT_OFFSET[65]
    (uint32)&FHMGR_updateRadioUCChannel,                 //ROM_FH_JT_OFFSET[66]
    (uint32)&FHMGR_updateRadioBCChannel,                 //ROM_FH_JT_OFFSET[67]
    (uint32)&FHMGR_pendUcChUpdate,                       //ROM_FH_JT_OFFSET[68]
    (uint32)&FHMGR_macCancelFHTimer,                     //ROM_FH_JT_OFFSET[69]
    (uint32)&FHMGR_pendBcChUpdate,                       //ROM_FH_JT_OFFSET[70]
    (uint32)&FHMGR_updateHopping,                        //ROM_FH_JT_OFFSET[71]
    /* fh_nt.c */
    (uint32)&FHNT_initTempTable,                     //ROM_FH_JT_OFFSET[72]
    (uint32)&FHNT_getFixedNTIndexFromTempIndex,      //ROM_FH_JT_OFFSET[73]
    (uint32)&FHNT_getNTIndexFromTempIndex,           //ROM_FH_JT_OFFSET[74]
    (uint32)&FHNT_getRemoveEntryFromTemp,            //ROM_FH_JT_OFFSET[75]
    (uint32)&FHNT_addTempEUI,                        //ROM_FH_JT_OFFSET[76]
    (uint32)&FHNT_delTempIndex,                      //ROM_FH_JT_OFFSET[77]
    (uint32)&FHNT_assessTime,                        //ROM_FH_JT_OFFSET[78]
    (uint32)&FHNT_purgeTimerIsrCb,                   //ROM_FH_JT_OFFSET[79]
    (uint32)&FHNT_getRemoveEntry,                    //ROM_FH_JT_OFFSET[80]
    (uint32)&FHNT_reset,                             //ROM_FH_JT_OFFSET[81]
    (uint32)&FHNT_init,                              //ROM_FH_JT_OFFSET[82]
    (uint32)&FHNT_purgeEntry,                        //ROM_FH_JT_OFFSET[83]
    (uint32)&FHNT_createEntry,                       //ROM_FH_JT_OFFSET[84]
    (uint32)&FHNT_getEntry,                          //ROM_FH_JT_OFFSET[85]
    (uint32)&FHNT_putEntry,                          //ROM_FH_JT_OFFSET[86]
    (uint32)&FHNT_getEUI,                            //ROM_FH_JT_OFFSET[87]
    (uint32)&FHNT_AddDeviceCB,                       //ROM_FH_JT_OFFSET[88]
    (uint32)&FHNT_DelDeviceCB,                       //ROM_FH_JT_OFFSET[89]
    (uint32)&FHNT_getFixedEntry,                     //ROM_FH_JT_OFFSET[90]
    (uint32)&FHNT_putFixedEntry,                     //ROM_FH_JT_OFFSET[91]
    (uint32)&FHNT_getRemoveFixedEntry,               //ROM_FH_JT_OFFSET[92]
    (uint32)&FHNT_createFixedEntry,                  //ROM_FH_JT_OFFSET[93]
    (uint32)&FHNT_getOptEntry,                       //ROM_FH_JT_OFFSET[94]
    (uint32)&FHNT_getAvailEntry,                     //ROM_FH_JT_OFFSET[95]
    (uint32)&FHNT_addOptEntry,                       //ROM_FH_JT_OFFSET[96]
    /* pib.c */
    (uint32)&FHPIB_getIndex,         //ROM_FH_JT_OFFSET[97]
    (uint32)&FHPIB_reset,            //ROM_FH_JT_OFFSET[98]
    (uint32)&FHPIB_getLen,           //ROM_FH_JT_OFFSET[99]
    (uint32)&FHPIB_set,              //ROM_FH_JT_OFFSET[100]
    (uint32)&FHPIB_get,              //ROM_FH_JT_OFFSET[101]
    /* fh_sm.c */
    (uint32)&FSM_transaction_debug,   //ROM_FH_JT_OFFSET[102]
    (uint32)&FHSM_event,              //ROM_FH_JT_OFFSET[103]
    /* fh_util.c */
    (uint32)&FHUTIL_getTxUfsi,                               //ROM_FH_JT_OFFSET[104]
    (uint32)&FHUTIL_adjBackOffDur,                           //ROM_FH_JT_OFFSET[105]
    (uint32)&FHUTIL_getUcChannel,                            //ROM_FH_JT_OFFSET[106]
    (uint32)&FHUTIL_getTxChannel,                            //ROM_FH_JT_OFFSET[107]
    (uint32)&FHUTIL_getBcTxParams,                           //ROM_FH_JT_OFFSET[108]
    (uint32)&FHUTIL_getCurUfsi,                              //ROM_FH_JT_OFFSET[109]
    (uint32)&FHUTIL_getCurBfio,                              //ROM_FH_JT_OFFSET[110]
    (uint32)&FHUTIL_getElapsedTime,                          //ROM_FH_JT_OFFSET[111]
    (uint32)&FHUTIL_compBfioTxTime,                          //ROM_FH_JT_OFFSET[112]
    (uint32)&FHUTIl_updateBTIE,                              //ROM_FH_JT_OFFSET[113]
    (uint32)&FHUTIL_calcUfsi,                                //ROM_FH_JT_OFFSET[114]
    (uint32)&FHUTIL_assert,                                  //ROM_FH_JT_OFFSET[115]
    (uint32)&FHUTIL_noAction,                                //ROM_FH_JT_OFFSET[116]
    (uint32)&FHUTIL_getCh0,                                  //ROM_FH_JT_OFFSET[117]
    (uint32)&FHUTIL_getChannelSpacing,                       //ROM_FH_JT_OFFSET[118]
    (uint32)&FHUTIL_getMaxChannels,                          //ROM_FH_JT_OFFSET[119]
    (uint32)&FHUTIL_getBitCounts,                            //ROM_FH_JT_OFFSET[120]
    (uint32)&FHUTIL_updateUcNumChannels,                     //ROM_FH_JT_OFFSET[121]
    (uint32)&FHUTIL_updateBcNumChannels,                     //ROM_FH_JT_OFFSET[122]
    (uint32)&FHUTIL_getGuardTime,                            //ROM_FH_JT_OFFSET[123]
    (uint32)&FHUTIL_elapsedTime,                             //ROM_FH_JT_OFFSET[124]

    (uint32)&FHAPI_getBitCount,                              //ROM_FH_JT_OFFSET[125]
    /* fh_ie_patch.c */
#ifdef FHIEPATCH
    (uint32)&FHIEPATCH_extractHie,                           //ROM_FH_JT_OFFSET[126]
    (uint32)&FHIEPATCH_extractPie,                           //ROM_FH_JT_OFFSET[127]
    (uint32)&FHIEPATCH_getHieLen,                            //ROM_FH_JT_OFFSET[128]
    (uint32)&FHIEPATCH_getPieId,                             //ROM_FH_JT_OFFSET[129]
    (uint32)&FHIEPATCH_getPieContentLen,                     //ROM_FH_JT_OFFSET[130]
    (uint32)&FHIEPATCH_genPieContent,                        //ROM_FH_JT_OFFSET[131]
    (uint32)&FHIEPATCH_genHie,                               //ROM_FH_JT_OFFSET[132]
    (uint32)&FHIEPATCH_parsePie,                             //ROM_FH_JT_OFFSET[133]
    (uint32)&FHIEPATCH_parseHie,                             //ROM_FH_JT_OFFSET[134]
#else
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[126]
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[127]
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[128]
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[129]
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[130]
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[131]
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[132]
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[133]
    (uint32)NULL,                                            //ROM_FH_JT_OFFSET[134]
#endif
    (uint32)&FH_test_var,
};  

void FH_ROM_Init(void)
{   
    /* assign the FH ROM JT table */
    RAM_MAC_BASE_ADDR[ROM_RAM_FH_TABLE_INDEX] = (uint32)(FH_ROM_Flash_JT);
}
#endif

