/******************************************************************************

 @file fh_rom_jt.h

 @brief FH API directly map the function to FH function jump table

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

#ifndef FH_ROM_JT_H
#define FH_ROM_JT_H

#include "rom_jt_def_154.h"

//#include <string.h>
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
#include "mac_high_level.h"


/* 
** FH Module global variables
*/

#define FH_hnd                              (*(FH_HND_s *)                    ROM_FH_JT_OFFSET(0))
#define FHNT_hnd                            (*(FHNT_HND_s *)                  ROM_FH_JT_OFFSET(1))
#define FH_txTiming                         (*(FH_TX_TIMING_TABLE_s *)        ROM_FH_JT_OFFSET(2))
#define FHNT_tempTable                      (*(FHNT_TEMP_TABLE_s *)           ROM_FH_JT_OFFSET(3))
#define FHIE_channelPlan                    ((FHIE_channelPlan_t *)           ROM_FH_JT_OFFSET(4))
#define FHNT_Dbg                            (*(FHNT_DEBUG_s *)                ROM_FH_JT_OFFSET(5))
#define FHPIB_defaults                      (*(FHPIB_DB_t *)                  ROM_FH_JT_OFFSET(6))
#define FH_PibTbl                           ((FH_PibTbl_t *)                  ROM_FH_JT_OFFSET(7))
#define FHPIB_db                            (*(FHPIB_DB_t *)                  ROM_FH_JT_OFFSET(8))
#define FHNT_tableOpt                       (*(FHNT_OPT_Table_s *)            ROM_FH_JT_OFFSET(9))
#define FHFSM_Dbg                           (*(FHFSM_debugBuffer_t *)         ROM_FH_JT_OFFSET(10))
#define FH_FSM                              (*(FSM_stateMachine_t *)          ROM_FH_JT_OFFSET(11))
#define macAddrSize                         ((uint8 *)                        ROM_FH_JT_OFFSET(12))
#define fhSupFrameTypes                     ((uint8 *)                        ROM_FH_JT_OFFSET(13))
/*
** FH API Proxy
** ROM-to ROM or ROM-to-Flash function
** if there is any patch function, replace the corresponding entries
*/
#define FH_API_BASE_INDEX                   (14)
/* file: fh_api.c */
#define MAP_FHAPI_reset                     ((void     (*)(void ))             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+0))                                           
#define MAP_FHAPI_start                     ((void     (*)(void ))             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+1))  
#define MAP_FHAPI_startBS                   ((void     (*)(void ))             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+2))  
#define MAP_FHAPI_completeTxCb              ((void     (*)(uint8_t))           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+3))  
#define MAP_FHAPI_completeRxCb              ((void     (*)(macRx_t *))         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+4))  
#define MAP_FHAPI_sendData                  ((void     (*)(void))              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+5))  
#define MAP_FHAPI_getTxParams_old           ((FHAPI_status     (*)(uint32_t *,uint8_t *))  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+6))  
#define MAP_FHAPI_getTxParams               ((FHAPI_status     (*)(uint32_t *,uint8_t *))  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+7)) 
#define MAP_FHAPI_getTxChHoppingFunc        ((uint8_t  (*)(void))              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+8))
#define MAP_FHAPI_getRemDT                  ((uint32_t  (*)(void))             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+9))
#define MAP_FHAPI_stopAsync                 ((FHAPI_status  (*)(void))         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+10))
#define MAP_FHAPI_setStateCb                ((void  (*)(uint8_t))              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+11))
#define MAP_FHAPI_SFDRxCb                   ((void  (*)(uint8_t))              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+12))

/* file: fh_data.c */
#define MAP_FHDATA_checkPktFromQueue        ((osal_msg_q_t  (*)(osal_msg_q_t *,uint8_t))    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+13))
#define MAP_FHDATA_getPktFromQueue          ((osal_msg_q_t  (*)(osal_msg_q_t *,uint8_t))    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+14))
#define MAP_FHDATA_purgePktFromQueue        ((FHAPI_status  (*)(osal_msg_q_t *,uint8_t))    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+15))
#define MAP_FHDATA_purgeUnSupportedFrameTypes ((void  (*)(osal_msg_q_t *))                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+16))
#define MAP_FHDATA_chkASYNCStartReq         ((void  (*)(void))                              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+17))
#define MAP_FHDATA_procASYNCReq             ((void  (*)(void *))                            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+18))
#define MAP_FHDATA_startEDFEReq             ((void  (*)(void *))                            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+19))
#define MAP_FHDATA_procEDFEReq              ((void  (*)(void *))                            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+20))
#define MAP_FHDATA_startEDFETimer           ((void  (*)(void *))                            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+21))
#define MAP_FHDATA_requeuePkt               ((void  (*)(void *))                            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+22))
#define MAP_FHDATA_updateCCA                ((void  (*)(uint32_t))                          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+23))
#define MAP_FHDATA_CheckMinTxOffTime        ((uint32_t  (*)(uint8_t, uint32_t))             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+24))
#define MAP_FHDATA_getASYNCDelay            ((uint32_t  (*)(void))                          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+25))
#define MAP_FHDATA_asyncMinTxOffTimerCb     ((void  (*)(uint8_t))                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+26))

/* file fh_dh1cf.c */
#define MAP_FHDH1CF_hashword                ((uint32_t  (*)(uint32_t *,uint32_t,uint32_t))    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+27))
#define MAP_FHDH1CF_getCh                   ((uint8_t  (*)(uint16_t,sAddrExt_t,uint16_t))     ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+28))
#define MAP_FHDH1CF_getBcCh                 ((uint8_t  (*)(uint32_t ,uint16_t,uint16_t))      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+29))
#define MAP_FHDH1CF_mapChIdxChannel         ((uint8_t  (*)(uint8_t ,uint8_t *,uint8_t))       ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+30))
#define MAP_FHDH1CF_getBCChannelNum         ((uint8_t  (*)(uint16_t,uint8_t))                 ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+31))
#define MAP_FHDH1CF_getChannelNum           ((uint8_t  (*)(FH_HND_s *))                       ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+32))
#define MAP_FHDH1CF_getTxChannelNum         ((uint8_t  (*)(uint16_t,NODE_ENTRY_s *))          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+33))

/* fh_ie.c */
#define MAP_FHIE_getCcaSfdTime              ((uint32_t  (*)(uint8_t))                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+34))
#define MAP_FHIE_genUTIE                    ((void  (*)(uint8_t *, uint8_t))                    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+35))
#define MAP_FHIE_genBTIE                    ((void  (*)(uint8_t *, uint8_t))                    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+36))
#define MAP_FHIE_getChannelPlan             ((uint8_t  (*)(FHIE_channelPlan_t *))               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+37))
#define MAP_FHIE_isChannelPlanValid         ((uint8_t  (*)(uint8_t, FHIE_channelPlan_t *))      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+38))
#define MAP_FHIE_genExcludedChannelMask     ((uint16_t  (*)(uint8_t *, uint8_t *,uint8_t))      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+39))
#define MAP_FHIE_genExcludedChannelRanges   ((uint16_t  (*)(uint8_t *, uint8_t))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+40))
#define MAP_FHIE_getExcludedChannelControl  ((uint8_t  (*)(uint8_t ))                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+41))
#define MAP_FHIE_genCommonInformation       ((uint16_t  (*)(uint8_t *, uint8_t, uint8_t, uint8_t ,FHIE_channelPlan_t *,uint8_t ))    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+42))
#define MAP_FHIE_genPieContent              ((uint16_t  (*)(uint8_t *, uint8_t))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+43))
#define MAP_FHIE_genPie                     ((uint16_t  (*)(uint8_t *, uint32_t, macTx_t *,FHIE_ieInfo_t *))          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+44))
#define MAP_FHIE_genHie                     ((uint16_t  (*)(uint8_t *, uint32_t, macTx_t *,FHIE_ieInfo_t *))          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+45))
#define MAP_FHIE_getCommonInformationLen    ((uint16_t  (*)(uint8_t ,uint8_t,  uint32_t, FHIE_channelPlan_t *,uint8_t))              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+46))
#define MAP_FHIE_getPieContentLen           ((uint16_t  (*)(uint8_t ))                          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+47))
#define MAP_FHIE_getPieLen                  ((uint16_t  (*)(uint32_t ))                         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+48))
#define MAP_FHIE_getHieLen                  ((uint16_t  (*)(uint32_t ))                         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+49))
#define MAP_FHIE_parseCommonInformation     ((uint16_t  (*)(uint8_t *,uint8_t,usieParams_t *,uint8_t ))               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+50))
#define MAP_FHIE_getPie                     ((uint16_t  (*)(uint8_t *,FHIE_ie_t * ))            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+51))
#define MAP_FHIE_getSubPie                  ((uint16_t  (*)(uint8_t *,FHIE_ie_t * ))            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+52))
#define MAP_FHIE_getHie                     ((uint16_t  (*)(uint8_t *,FHIE_ie_t * ))            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+53))

#define MAP_FHIE_gen                        ((uint16_t  (*)(uint8_t *,uint32_t,macTx_t *,FHIE_ieInfo_t * ))         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+54))
#define MAP_FHIE_getLen                     ((uint16_t  (*)(uint32_t ))                                             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+55))
#define MAP_FHIE_parsePie                   ((FHAPI_status  (*)(uint8_t *,sAddrExt_t *,uint16_t *,uint16_t * ))     ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+56))
#define MAP_FHIE_parseHie                   ((FHAPI_status  (*)(uint8_t *,sAddrExt_t *,uint32_t,uint16_t *,uint16_t * ))          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+57))
#define MAP_FHIE_extractPie                 ((FHAPI_status  (*)(uint8_t *,IE_TYPE_t , uint8_t * ))                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+58))
#define MAP_FHIE_extractHie                 ((FHAPI_status  (*)(uint8_t *,IE_TYPE_t , uint8_t * ))                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+59))

/* fh_mgr.c */
#define MAP_FHMGR_macStartOneShotTimer      ((void  (*)(FH_macFHTimer_t *))                     ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+60))
#define MAP_FHMGR_macStartFHTimer           ((void  (*)(FH_macFHTimer_t *,bool))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+61))
#define MAP_FHMGR_macGetElapsedFHTime       ((uint32_t  (*)(FH_macFHTimer_t *))                 ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+62))
#define MAP_FHMGR_ucTimerIsrCb              ((void  (*)(uint8_t))                               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+63))
#define MAP_FHMGR_BCTimerEventUpd           ((void  (*)(void))                                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+64))
#define MAP_FHMGR_bcTimerIsrCb              ((void  (*)(uint8_t))                               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+65))

#define MAP_FHMGR_updateRadioUCChannel      ((void  (*)(void *))                                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+66))
#define MAP_FHMGR_updateRadioBCChannel      ((void  (*)(void *))                                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+67))
#define MAP_FHMGR_pendUcChUpdate            ((void  (*)(void *))                                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+68))
#define MAP_FHMGR_macCancelFHTimer          ((void  (*)(FH_macFHTimer_t *))                     ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+69))
#define MAP_FHMGR_pendBcChUpdate            ((void  (*)(void *))                                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+70))
#define MAP_FHMGR_updateHopping             ((void  (*)(void *))                                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+71))

/* fh_nt.c */

#define MAP_FHNT_initTempTable              ((void  (*)(void ))                                 ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+72))
#define MAP_FHNT_getFixedNTIndexFromTempIndex ((uint16_t  (*)(uint16_t ))                       ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+73))
#define MAP_FHNT_getNTIndexFromTempIndex    ((uint16_t  (*)(uint16_t,uint8_t * ))               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+74))
#define MAP_FHNT_getRemoveEntryFromTemp     ((void  (*)(void))                                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+75))
#define MAP_FHNT_addTempEUI                 ((uint16_t  (*)(sAddrExt_t *))                      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+76))
#define MAP_FHNT_delTempIndex               ((uint16_t  (*)(uint16_t))                          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+77))
#define MAP_FHNT_assessTime                 ((uint8_t  (*)(uint32_t,uint32_t,uint32_t))         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+78))
#define MAP_FHNT_purgeTimerIsrCb            ((void  (*)(uint8_t))                               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+79))
#define MAP_FHNT_getRemoveEntry             ((NODE_ENTRY_s *  (*)(void))                        ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+80))
#define MAP_FHNT_reset                      ((void  (*)(void))                                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+81))
#define MAP_FHNT_init                       ((void  (*)(void))                                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+82))
#define MAP_FHNT_purgeEntry                 ((void  (*)(uint32_t))                              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+83))
#define MAP_FHNT_createEntry                ((FHAPI_status  (*)(sAddrExt_t *,NODE_ENTRY_s *))   ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+84))
#define MAP_FHNT_getEntry                   ((FHAPI_status  (*)(sAddr_t *,NODE_ENTRY_s *))      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+85))
#define MAP_FHNT_putEntry                   ((FHAPI_status  (*)(const NODE_ENTRY_s *))          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+86))
#define MAP_FHNT_getEUI                     ((sAddrExt_t *  (*)(uint16_t))                      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+87))
#define MAP_FHNT_AddDeviceCB                ((uint8_t (*)(sAddrExt_t *,uint8_t))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+88))
#define MAP_FHNT_DelDeviceCB                ((uint8_t (*)(sAddrExt_t *,uint8_t))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+89))
#define MAP_FHNT_getFixedEntry              ((FHAPI_status (*)(sAddrExt_t *,NODE_ENTRY_s *))    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+90))
#define MAP_FHNT_putFixedEntry              ((FHAPI_status (*)(const NODE_ENTRY_s *))           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+91))
#define MAP_FHNT_getRemoveFixedEntry        ((NODE_FIXED_ENTRY_s * (*)(void))                   ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+92))
#define MAP_FHNT_createFixedEntry           ((FHAPI_status (*)(sAddrExt_t *,NODE_ENTRY_s *))    ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+93))
#define MAP_FHNT_getOptEntry                ((FHAPI_status (*)(sAddr_t *,NODE_ENTRY_s *))       ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+94))
#define MAP_FHNT_getAvailEntry              ((NODE_OPT_ENTRY_s * (*)(sAddr_t *))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+95))
#define MAP_FHNT_addOptEntry                ((void (*)(sAddr_t *,uint8_t ))                     ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+96))

/* fh_pib.c */
#define MAP_FHPIB_getIndex                  ((uint16_t (*)(uint16_t))                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+97))
#define MAP_FHPIB_reset                     ((void (*)(void))                                   ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+98))
#define MAP_FHPIB_getLen                    ((uint8_t (*)(uint16_t))                            ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+99))
#define MAP_FHPIB_set                       ((FHAPI_status (*)(uint16_t,void *))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+100))
#define MAP_FHPIB_get                       ((FHAPI_status (*)(uint16_t,void *))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+101))

/* fh_sm.c */
#define MAP_FSM_transaction_debug           ((FHAPI_status (*)(uint16_t,uint16_t,void *))       ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+102))
#define MAP_FHSM_event                      ((FHAPI_status (*)(FSM_stateMachine_t *,uint16_t , void *))     ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+103))

/* fh_util.c */
#define MAP_FHUTIL_getTxUfsi                ((uint32_t (*)(NODE_ENTRY_s *))                                 ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+104))
#define MAP_FHUTIL_adjBackOffDur            ((FHAPI_status (*)(NODE_ENTRY_s *,uint32_t *,uint32_t *))       ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+105))
#define MAP_FHUTIL_getUcChannel             ((uint8_t (*)(NODE_ENTRY_s *,uint32_t))                         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+106))
#define MAP_FHUTIL_getTxChannel             ((uint8_t (*)(NODE_ENTRY_s *,uint32_t,uint32_t))                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+107))
#define MAP_FHUTIL_getBcTxParams            ((FHAPI_status (*)(uint32_t *,uint8_t *))                       ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+108))
#define MAP_FHUTIL_getCurUfsi               ((FHAPI_status (*)(uint32_t *))                                 ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+109))
#define MAP_FHUTIL_getCurBfio               ((FHAPI_status (*)(uint32_t *,uint16_t *))                      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+110))
#define MAP_FHUTIL_getElapsedTime           ((uint32_t (*)(uint32_t ))                                      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+111))
#define MAP_FHUTIL_compBfioTxTime           ((uint32_t (*)(void))                                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+112))
#define MAP_FHUTIl_updateBTIE               ((uint8_t (*)(uint32_t,uint16_t))                               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+113))
#define MAP_FHUTIL_calcUfsi                 ((uint32_t (*)(uint32_t,uint8_t))                               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+114))
#define MAP_FHUTIL_assert                   ((void (*)(void *))                                             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+115))
#define MAP_FHUTIL_noAction                 ((void (*)(void *))                                             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+116))
#define MAP_FHUTIL_getCh0                   ((uint32_t (*)(void ))                                          ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+117))
#define MAP_FHUTIL_getChannelSpacing        ((uint8_t (*)(void ))                                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+118))
#define MAP_FHUTIL_getMaxChannels           ((uint8_t (*)(void ))                                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+119))
#define MAP_FHUTIL_getBitCounts             ((uint8_t (*)(uint8_t *, uint8_t ))                             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+120))
#define MAP_FHUTIL_updateUcNumChannels      ((void (*)(uint8_t * ))                                         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+121))
#define MAP_FHUTIL_updateBcNumChannels      ((void (*)(uint8_t * ))                                         ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+122))
#define MAP_FHUTIL_getGuardTime             ((uint32_t (*)(NODE_ENTRY_s * ))                                ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+123))
#define MAP_FHUTIL_elapsedTime              ((uint32_t (*)(uint32_t,uint32_t))                              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+124))

/* fh_api.c */
#define MAP_FHAPI_getBitCount               ((void (*)(void ))                                              ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+125))

#ifdef FHIEPATCH
/* fh_ie_patch.c */
#define MAP_FHIEPATCH_extractHie            ((FHAPI_status  (*)(uint8_t, uint16_t, uint8_t *, uint8_t * ))                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+126))
#define MAP_FHIEPATCH_extractPie            ((FHAPI_status  (*)(uint8_t, uint16_t, uint8_t *, uint8_t * ))                  ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+127))
#define MAP_FHIEPATCH_getHieLen             ((uint16_t  (*)(uint32_t ))                                                     ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+128))
#define MAP_FHIEPATCH_getPieId              ((uint8_t  (*)(uint32_t, uint8_t * ))                                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+129))
#define MAP_FHIEPATCH_getPieContentLen      ((uint16_t  (*)(uint8_t ))                                                      ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+130))
#define MAP_FHIEPATCH_genPieContent         ((uint16_t  (*)(uint8_t *, uint8_t ))                                           ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+131))
#define MAP_FHIEPATCH_genHie                ((uint16_t  (*)(uint8_t *, uint32_t , macTx_t *, FHIE_ieInfo_t * ))             ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+132))
#define MAP_FHIEPATCH_parsePie              ((FHAPI_status  (*)(uint8_t, uint16_t, uint8_t *, sAddrExt_t * ))               ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+133))
#define MAP_FHIEPATCH_parseHie              ((FHAPI_status  (*)(uint8_t, uint16_t, uint8_t *, uint32_t, sAddrExt_t * ))     ROM_FH_JT_OFFSET(FH_API_BASE_INDEX+134))
#endif

#endif
