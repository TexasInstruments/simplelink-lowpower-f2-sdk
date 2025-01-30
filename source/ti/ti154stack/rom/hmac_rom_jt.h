/******************************************************************************

 @file hmac_rom_jt.h

 @brief HMAC API directly map the function to FH function jump table

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

#ifndef HMAC_ROM_JT_H
#define HMAC_ROM_JT_H

#include "api_mac.h"
#include "mac_api.h"
#include "mac_main.h"
#include "hal_types.h"
#include "mac_coord.h"
#include "mac_data.h"
#include "mac_device.h"
#include "mac_beacon.h"
#include "mac_beacon_device.h"
#include "mac_beacon_coord.h"
#include "mac_mgmt.h"
#include "mac_pib.h"
#include "mac_scan.h"
#include "mac_spec.h"
#include "mac_radio.h"
#include "mac_security_pib.h"
#include "mac_pwr.h"
#include "mac_timer.h"

#ifdef USE_ICALL
#include "osal_tasks.h"
#endif

#include "rom_jt_def_154.h"

/* HMAC global */

#define macBeaconCoordAction1                   ((macAction_t *)                    ROM_HMAC_JT_OFFSET(0))
#define macBeaconCoord                          (*(macBeaconCoord_t *)              ROM_HMAC_JT_OFFSET(1))

#define macBeaconDeviceAction                   ((macAction_t *)                    ROM_HMAC_JT_OFFSET(2))
#define macBeaconDevice                         (*(macBeaconDevice_t *)             ROM_HMAC_JT_OFFSET(3))
#define macBeaconMargin                         ((uint16 *)                         ROM_HMAC_JT_OFFSET(4))
#define macBeacon                               (*(macBeacon_t *)                   ROM_HMAC_JT_OFFSET(5))

#define macCoordAction1                         ((macAction_t *)                    ROM_HMAC_JT_OFFSET(6))
#define macCoordAction2                         ((macAction_t *)                    ROM_HMAC_JT_OFFSET(7))
#define macCoordAction3                         ((macAction_t *)                    ROM_HMAC_JT_OFFSET(8))
#define macCoord                                (*(macCoord_t *)                    ROM_HMAC_JT_OFFSET(9))

#define macCmdIncludeFHIe                       ((const uint32 *)                   ROM_HMAC_JT_OFFSET(10))
#define macCmdBufSize                           ((const uint8 *)                    ROM_HMAC_JT_OFFSET(11))
#define macCmdLen                               ((const uint8 *)                    ROM_HMAC_JT_OFFSET(12))

#define macTxFrameSuccess                       ((const uint8 *)                    ROM_HMAC_JT_OFFSET(13))
#define macTxFrameFailed                        ((const uint8 *)                    ROM_HMAC_JT_OFFSET(14))

#define macData                                 (*(macData_t *)                     ROM_HMAC_JT_OFFSET(15))
#define pMacDataTx                              (*(macTx_t **)                      ROM_HMAC_JT_OFFSET(16))

#define macDataTxIndirect                       (*(macDataTxFcn_t *)                ROM_HMAC_JT_OFFSET(17))
#define macDataRequeueIndirect                  (*(macDataTxFcn_t *)                ROM_HMAC_JT_OFFSET(18))

#define macDataRxBeaconCritical                 (*(macRxBeaconCritical_t *)         ROM_HMAC_JT_OFFSET(19))
#define macDataTxBeaconComplete                 (*(macTxBeaconComplete_t *)         ROM_HMAC_JT_OFFSET(20))
#define macDataSetSched                         (*(macDataTxFcn_t *)                ROM_HMAC_JT_OFFSET(21))
#define macDataCheckSched                       (*(macDataCheckSched_t *)           ROM_HMAC_JT_OFFSET(22))
#define macDataCheckTxTime                      (*(macDataCheckTxTime_t *)          ROM_HMAC_JT_OFFSET(23))
#define macDataBeaconRequeue                    (*(macDataTxFcn_t *)                ROM_HMAC_JT_OFFSET(24))

#define macDataTxDelay                          (*(macTimer_t *)                    ROM_HMAC_JT_OFFSET(25))
#define macDeviceAction1                        ((macAction_t *)                    ROM_HMAC_JT_OFFSET(26))
#define macDeviceAction2                        ((macAction_t *)                    ROM_HMAC_JT_OFFSET(27))
#define macDevice                               (*(macDevice_t *)                   ROM_HMAC_JT_OFFSET(28))

#define macIdleSt                               ((uint8 [][MAC_NUM_COLS])           ROM_HMAC_JT_OFFSET(29))

#define macAssociatingSt                        ((macStateTbl_t)                   ROM_HMAC_JT_OFFSET(30))
#define macPollingSt                            ((macStateTbl_t)                   ROM_HMAC_JT_OFFSET(31))

#define macScanningSt                           ((macStateTbl_t)                   ROM_HMAC_JT_OFFSET(32))
#define macStartingSt                           ((macStateTbl_t)                   ROM_HMAC_JT_OFFSET(33))

#define macSendMsgBufSize                       ((uint16 * )                        ROM_HMAC_JT_OFFSET(34))
#define macSendMsgDataSize                      ((uint8 * )                         ROM_HMAC_JT_OFFSET(35))
#define macCbackEventTbl                        ((uint8 * )                         ROM_HMAC_JT_OFFSET(36))

#define macTaskId                               (*(uint8 * )                        ROM_HMAC_JT_OFFSET(37))
#define macMain                                 (*(macMain_t * )                    ROM_HMAC_JT_OFFSET(38))

#define macStateTbl                             ((macStateTbl_t * )                 ROM_HMAC_JT_OFFSET(39))
#define macReset                                ((macReset_t * )                    ROM_HMAC_JT_OFFSET(40))

#define macActionSet                            ((macActionSet_t * )                ROM_HMAC_JT_OFFSET(41))

#define macMgmt                                  (*(macMgmt_t * )                   ROM_HMAC_JT_OFFSET(42))

#define macPanCoordinator                       (*(bool * )                         ROM_HMAC_JT_OFFSET(43))

#define macMgmtAction1                          ((macAction_t * )                   ROM_HMAC_JT_OFFSET(44))
#define macMgmtAction2                          ((macAction_t * )                   ROM_HMAC_JT_OFFSET(45))

#define fhObject                                (*(fhObject_t * )                   ROM_HMAC_JT_OFFSET(46))
#define csmPhy                                  (*(bool * )                         ROM_HMAC_JT_OFFSET(47))

#define macPibDefaults                          (*(macPib_t * )                     ROM_HMAC_JT_OFFSET(48))
#define macPibTbl                               ((macPibTbl_t * )                   ROM_HMAC_JT_OFFSET(49))

#define macMRFSKStdPhyTable             ((macMRFSKPHYDesc_t * )                     ROM_HMAC_JT_OFFSET(50))
#define macMRFSKGenPhyTable             ((macMRFSKPHYDesc_t * )                     ROM_HMAC_JT_OFFSET(51))
#define macIEEEPhyTable                 (*(macIEEEPHYDesc_t * )                     ROM_HMAC_JT_OFFSET(52))

#define macPib                          (*(macPib_t * )                             ROM_HMAC_JT_OFFSET(53))
#define pMacPib (&macPib)

#define timerUnalignedQ                 (*(macTimerHeader_t * )                     ROM_HMAC_JT_OFFSET(54))
#define timerAlignedQ                   (*(macTimerHeader_t * )                     ROM_HMAC_JT_OFFSET(55))

#define macTimerRollover                (*(int32 * )                                ROM_HMAC_JT_OFFSET(56))
#define macTimerNewRollover             (*(int32 * )                                ROM_HMAC_JT_OFFSET(57))

#define macTimerActive                  (*(macTimer_t ** )                          ROM_HMAC_JT_OFFSET(58))

#define macTimerRolloverValue           ((uint32 * )                                ROM_HMAC_JT_OFFSET(59))

#define macScanAction                   ((macAction_t * )                           ROM_HMAC_JT_OFFSET(60))
#define macScan                         (*(macScan_t * )                            ROM_HMAC_JT_OFFSET(61))

#define macSecurityPib                  (*(macSecurityPib_t * )                     ROM_HMAC_JT_OFFSET(62))

#define macSecurityPibTbl               ( (macSecurityPibTbl_t * )                  ROM_HMAC_JT_OFFSET(63))

#define pMacSecurityPib (&macSecurityPib)

/* configurable parameters */
#define macCfg                          ( * (macCfg_t * )                           ROM_HMAC_JT_OFFSET(64))

#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC26X2X7) && !defined(DeviceFamily_CC13X1) && !defined(DeviceFamily_CC26X1) && !defined(DeviceFamily_CC13X4) && !defined(DeviceFamily_CC26X4) && !defined(DeviceFamily_CC26X3)
#define Crypto_handle                   ( *(CryptoCC26XX_Handle  *)                ROM_HMAC_JT_OFFSET(65))
#else
#define AESCCM_handle                   ( *(AESCCM_Handle  *)                      ROM_HMAC_JT_OFFSET(65))
#endif

#define macCryptoDrvTblPtr                 ( *(uint32_t **  )                         ROM_HMAC_JT_OFFSET(66))

#define macAuthTagLen                   ((CODE const uint8 *)                      ROM_HMAC_JT_OFFSET(67))

#define macKeySourceLen                 ((CODE const uint8 *)                      ROM_HMAC_JT_OFFSET(68))

#define macBeaconPayload                ((uint8 *)                                 ROM_HMAC_JT_OFFSET(69))

#define enhancedAckBuf                  ((uint8 *)                                 ROM_HMAC_JT_OFFSET(70))

#define macTasksEvents                     ((uint16 *)                                ROM_HMAC_JT_OFFSET(71))
/*
** HMAC API Proxy
** ROM-to ROM or ROM-to-Flash function
** if there is any patch function, replace the corresponding entries
*/


#define HMAC_API_BASE_INDEX                   (72)

/* mac_beacon_coord.c */
#define MAP_macBeaconCoordReset                                               ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+0))
#define MAP_MAC_InitBeaconCoord                                               ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+1))
#define MAP_macBeaconSetPrepareTime                                           ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+2))
#define MAP_macBeaconCheckStartTime                                           ((uint8                     (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+3))
#define MAP_macApiBeaconStartReq                                              ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+4))
#define MAP_macBeaconPeriodCallback                                           ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+5))
#define MAP_macBeaconTxCallback                                               ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+6))
//#define MAP_macEBeaconTxCallback                                              ((void                      (*) ( uint8))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+7))
//#define MAP_macEBeaconPrepareCallback                                         ((void                      (*) ( uint8))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+8))
#define MAP_macBeaconPrepareCallback                                          ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+7))
#define MAP_macStartBeaconPrepareCallback                                     ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+8))
#define MAP_macBeaconStartContinue                                            ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+9))
#define MAP_macBeaconBattLifeCallback                                         ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+10))
#define MAP_macTxBeaconCompleteCallback                                       ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+11))
#define MAP_macBeaconSetupBroadcast                                           ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+12))
#define MAP_macOutgoingNonSlottedTx                                           ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+13))
#define MAP_macBeaconSchedRequested                                           ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+14))
#define MAP_macBeaconClearIndirect                                            ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+15))

/* mac_beacon_device.c */
#define MAP_macBroadcastPendCallback                                          ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+16))
#define MAP_macBeaconStopTrack                                                ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+17))
#define MAP_macIncomingNonSlottedTx                                           ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+18))
#define MAP_macBeaconSyncLoss                                                 ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+19))
#define MAP_macSyncTimeoutCallback                                            ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+20))
#define MAP_macTrackTimeoutCallback                                           ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+21))
#define MAP_macTrackStartCallback                                             ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+22))
#define MAP_macTrackPeriodCallback                                            ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+23))
#define MAP_macRxBeaconCritical                                               ((void                      (*) ( macRx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+24))
#define MAP_MAC_InitBeaconDevice                                              ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+25))
#define MAP_macCheckPendAddr                                                  ((uint8                     (*) ( uint8, uint8 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+26))
#define MAP_macAutoPoll                                                       ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+27))
#define MAP_macApiSyncReq                                                     ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+28))
#define MAP_macBeaconStartFrameResponseTimer                                  ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+29))
#define MAP_macStartBroadcastPendTimer                                        ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+30))

/* mac_beacon.c */
#define MAP_macBeaconReset                                                    ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+31))
#define MAP_macBeaconInit                                                     ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+32))
#define MAP_macBeaconSetSched                                                 ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+33))
#define MAP_macBeaconCheckSched                                               ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+34))
#define MAP_macBeaconRequeue                                                  ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+35))
#define MAP_macBeaconCheckTxTime                                              ((uint16                    (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+36))
#define MAP_macBeaconSetupCap                                                 ((void                      (*) ( uint8, uint8, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+37))

/* mac_coord.c */
#define MAP_macCoordReset                                                     ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+38))
#define MAP_macBuildAssociateRsp                                              ((uint8                     (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+39))
#define MAP_macBuildRealign                                                   ((uint8                     (*) ( macTx_t *, sAddr_t *, uint16, uint16, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+40))
#define MAP_macPendAddrLen                                                    ((uint8                     (*) ( uint8 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+41))
#define MAP_macBuildPendAddr                                                  ((uint8*                    (*) ( uint8 *, uint8, bool *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+42))
//#define MAP_macBuildEnhancedBeacon                                            ((macTx_t*                  (*) ( uint8, uint8, bool, uint8))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+0))
#define MAP_macBuildBeacon                                                    ((macTx_t*                  (*) ( uint8, uint8, bool))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+43))
#define MAP_macIndirectMark                                                   ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+44))
#define MAP_macIndirectSend                                                   ((bool                      (*) ( sAddr_t *, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+45))
#define MAP_macIndirectExpire                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+46))
#define MAP_macIndirectTxFrame                                                ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+47))
#define MAP_macIndirectRequeueFrame                                           ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+48))
#define MAP_macRxAssociateReq                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+49))
#define MAP_macApiAssociateRsp                                                ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+50))
#define MAP_macRxOrphan                                                       ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+51))
#define MAP_macApiOrphanRsp                                                   ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+52))
#define MAP_macApiPurgeReq                                                    ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+53))
#define MAP_macApiStartReq                                                    ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+54))
#define MAP_macStartBegin                                                     ((uint8                     (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+55))
#define MAP_macStartSetParams                                                 ((void                      (*) ( ApiMac_mlmeStartReq_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+56))
#define MAP_macStartContinue                                                  ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+57))
#define MAP_macStartComplete                                                  ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+58))
#define MAP_macRxBeaconReq                                                    ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+59))
//#define MAP_macProcessEBeaconFilterIe                                         ((bool                      (*) ( macEvent_t*, uint8*, uint32*))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+62))
//#define MAP_macRxEnhancedBeaconReq                                            ((void                      (*) ( macEvent_t *))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+0))
#define MAP_macRxDataReq                                                      ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+60))
#define MAP_macRxPanConflict                                                  ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+61))
#define MAP_MAC_InitCoord                                                     ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+62))
#define MAP_MAC_MlmeAssociateRsp                                              ((uint8                     (*) ( ApiMac_mlmeAssociateRsp_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+63))
#define MAP_MAC_MlmeOrphanRsp                                                 ((uint8                     (*) ( ApiMac_mlmeOrphanRsp_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+64))
#define MAP_MAC_McpsPurgeReq                                                  ((uint8                     (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+65))
#define MAP_MAC_MlmeStartReq                                                  ((uint8                     (*) ( ApiMac_mlmeStartReq_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+66))

/* mac_data.c */
#define MAP_macDataTxDelayCallback                                            ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+67))
#define MAP_macDataReset                                                      ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+68))
#define MAP_macAllocTxBuffer                                                  ((macTx_t*                  (*) ( uint8, ApiMac_sec_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+69))
#define MAP_macFrameDurationSubG                                              ((uint16                    (*) ( uint16, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+70))
#define MAP_macFrameDurationIeee                                              ((uint16                    (*) ( uint16, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+71))
#define MAP_macFrameDuration                                                  ((uint16                    (*) ( uint16, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+72))
#define MAP_macRxCheckPendingCallback                                         ((bool                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+73))
#define MAP_macCheckIndPacket                                                 ((uint8_t                   (*) ( macRx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+74))
#define MAP_macRxCheckMACPendingCallback                                      ((bool                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+75))
#define MAP_macDataRxMemAlloc                                                 ((uint8*                    (*) ( uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+76))
#define MAP_macDataRxMemFree                                                  ((uint8                     (*) ( uint8 **))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+77))
#define MAP_macRxCompleteCallback                                             ((void                      (*) ( macRx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+78))
#define MAP_macDataTxTimeAvailable                                            ((uint16                    (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+79))
#define MAP_macBuildDataFrame                                                 ((uint8                     (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+80))
#define MAP_macCheckSched                                                     ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+81))
#define MAP_macSetSched                                                       ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+82))
#define MAP_macDataTxComplete                                                 ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+83))
#define MAP_macTxCompleteCallback                                             ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+84))
#define MAP_macDataTxSend                                                     ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+85))
#define MAP_macDataTxEnqueue                                                  ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+86))
#define MAP_macDataSend                                                       ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+87))
#define MAP_macApiDataReq                                                     ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+88))
#define MAP_macDataRxInd                                                      ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+89))
#define MAP_MAC_McpsDataReq                                                   ((uint8                     (*) ( ApiMac_mcpsDataReq_t*))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+90))
#define MAP_MAC_McpsDataAlloc                                                 ((macMcpsDataReq_t*         (*) ( uint16, uint8, uint8, uint32, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+91))
#define MAP_macAutoPendMaintainSrcMatchTable                                  ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+92))
#define MAP_macAutoPendAddSrcMatchTableEntry                                  ((void                      (*) ( macTx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+93))

/* mac_device.c */
#define MAP_macDeviceReset                                                    ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+94))
#define MAP_macAssociateCnf                                                   ((void                      (*) ( uint8, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+95))
#define MAP_macPollCnf                                                        ((void                      (*) ( uint8, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+96))
#define MAP_macBuildAssociateReq                                              ((uint8                     (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+97))
#define MAP_macStartResponseTimer                                             ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+98))
#define MAP_macStartFrameResponseTimer                                        ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+99))
#define MAP_macApiAssociateReq                                                ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+100))
#define MAP_macAssocDataReq                                                   ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+101))
#define MAP_macAssocDataReqComplete                                           ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+102))
#define MAP_macRxAssocRsp                                                     ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+103))
#define MAP_macAssocFrameResponseTimeout                                      ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+104))
#define MAP_macAssocFailed                                                    ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+105))
#define MAP_macAssocRxDisassoc                                                ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+106))
#define MAP_macAssocDataRxInd                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+107))
#define MAP_macApiPollReq                                                     ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+108))
#define MAP_macPollDataReqComplete                                            ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+109))
#define MAP_macPollDataRxInd                                                  ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+110))
#define MAP_macPollFrameResponseTimeout                                       ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+111))
#define MAP_macPollRxDisassoc                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+112))
#define MAP_macPollRxAssocRsp                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+113))
#define MAP_macRxCoordRealign                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+114))
#define MAP_macPanConflictComplete                                            ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+115))
#define MAP_MAC_InitDevice                                                    ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+116))
#define MAP_MAC_MlmeAssociateReq                                              ((uint8                     (*) ( ApiMac_mlmeAssociateReq_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+117))
#define MAP_MAC_MlmePollReq                                                   ((uint8                     (*) ( ApiMac_mlmePollReq_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+118))

/* mac_ie_build.c */
#define MAP_macBuildHeaderIes                                                 ((uint8                     (*) ( macTx_t*, headerIeInfo_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+119))
#define MAP_macBuildPayloadCoexistIe                                          ((uint16                    (*) ( macTx_t*, bool))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+120))
#define MAP_macBuildPayloadIes                                                ((uint8                     (*) ( macTx_t*, payloadIeInfo_t *, bool, bool))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+121))

/* mac_ie_parse.c */
#define MAP_macUpdateHeaderIEPtr                                              ((void                      (*) ( macRx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+122))
#define MAP_macUpdatePayloadIEPtr                                             ((void                      (*) ( macRx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+123))
#define MAP_macParseHeaderIes                                                 ((uint8                     (*) ( macRx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+124))
#define MAP_macParsePayloadIes                                                ((uint8                     (*) ( macRx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+125))
#define MAP_macIeFreeList                                                     ((void                      (*) ( macPayloadIeInfo_t*))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+126))
#define MAP_macIeCoexistIePresent                                             ((uint8*                    (*) ( macPayloadIeInfo_t*, uint8*))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+127))
#define MAP_macMlmeSubIePresent                                               ((uint8*                    (*) ( macPayloadIeInfo_t*, uint8, uint8*))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+128))

/* mac_main.c */
#define MAP_macMainReset                                                      ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+129))
#define MAP_macSetEvent                                                       ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+130))
#define MAP_macSetEvent16                                                     ((void                      (*) ( uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+131))
#define MAP_macCbackForEvent                                                  ((void                      (*) ( macEvent_t *, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+132))
#define MAP_macStateIdle                                                      ((bool                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+133))
#define MAP_macStateIdleOrPolling                                             ((bool                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+134))
#define MAP_macStateScanning                                                  ((bool                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+135))
#define MAP_macExecute                                                        ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+136))
#define MAP_macSendMsg                                                        ((uint8                     (*) ( uint8, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+137))
#define MAP_macSendDataMsg                                                    ((uint8                     (*) ( uint8, void *, ApiMac_sec_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+138))
#define MAP_macMainSetTaskId                                                  ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+139))
#define MAP_macMainReserve                                                    ((void                      (*) ( uint8 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+140))
#define MAP_mac_msg_deallocate                                                ((void                      (*) ( uint8 **))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+141))

/* mac_mgmt.c */
#define MAP_macMgmtReset                                                      ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+142))
#define MAP_macGetCoordAddress                                                ((void                      (*) ( sAddr_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+143))
#define MAP_macGetMyAddrMode                                                  ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+144))
#define MAP_macDestAddrCmp                                                    ((bool                      (*) ( uint8 *, uint8 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+145))
#define MAP_macDestSAddrCmp                                                   ((bool                      (*) ( sAddr_t *, uint16, uint8 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+146))
#define MAP_macCoordAddrCmp                                                   ((bool                      (*) ( sAddr_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+147))
#define MAP_macCoordDestAddrCmp                                               ((bool                      (*) ( uint8 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+148))
#define MAP_macBuildHeader                                                    ((uint8                     (*) ( macTx_t *, uint8, sAddr_t *, uint16, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+149))
#define MAP_macBuildCommonReq                                                 ((uint8                     (*) ( uint8, uint8, sAddr_t *, uint16, uint16, ApiMac_sec_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+150))
//#define MAP_macBuildEnhanceBeaconReq                                          ((uint8                     (*) ( macTx_t *, sAddr_t *, ApiMac_sec_t *, uint8, uint8, uint8))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+0))
//#define MAP_macBuildCoexistEBeaconReq                                         ((uint8                     (*) ( macTx_t *, sAddr_t *, ApiMac_sec_t *))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+0))
#define MAP_macBuildDisassociateReq                                           ((uint8                     (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+151))
//#define MAP_macBuildEBeaconNotifyInd                                          ((uint8                     (*) ( macMlmeBeaconNotifyInd_t *, macEvent_t *))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+0))
#define MAP_macBuildBeaconNotifyInd                                           ((void                      (*) ( macMlmeBeaconNotifyInd_t *, macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+152))
#define MAP_macSecCpy                                                         ((void                      (*) ( ApiMac_sec_t *, ApiMac_sec_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+153))
#define MAP_macNoAction                                                       ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+154))
#define MAP_macTxCsmaDelay                                                    ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+155))
#define MAP_macBcTimerHandler                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+156))
#define MAP_macApiUnsupported                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+157))
#define MAP_macDefaultAction                                                  ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+158))
#define MAP_macApiBadState                                                    ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+159))
#define MAP_macApiPending                                                     ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+160))
#define MAP_macCommStatusInd                                                  ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+161))
#define MAP_macApiDisassociateReq                                             ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+162))
#define MAP_macDisassocComplete                                               ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+163))
#define MAP_macRxDisassoc                                                     ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+164))
#define MAP_macRxBeacon                                                       ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+165))
#define MAP_macConflictSyncLossInd                                            ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+166))
#define MAP_MAC_MlmeDisassociateReq                                           ((uint8                     (*) ( ApiMac_mlmeDisassociateReq_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+167))
#define MAP_macUpdateCsmPhyParam                                              ((void                      (*) ( bool))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+168))
#define MAP_macIsCsmOperational                                               ((bool                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+169))
#define MAP_MAC_MlmeResetReq                                                  ((uint8                     (*) ( bool))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+170))
#define MAP_MAC_ResumeReq                                                     ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+171))
#define MAP_MAC_YieldReq                                                      ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+172))
#define MAP_MAC_MlmeSyncReq                                                   ((uint8                     (*) ( ApiMac_mlmeSyncReq_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+173))
#define MAP_macSetDefaultsByPhyID                                             ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+174))
#define MAP_macSetDefaultsByRE                                                ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+175))
#define MAP_MAC_Init                                                          ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+176))
#define MAP_MAC_InitFH                                                        ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+177))
#define MAP_MAC_MlmeFHGetReqSize                                              ((uint8                     (*) ( uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+178))
#define MAP_MAC_MlmeFHGetReq                                                  ((uint8                     (*) ( uint16, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+179))
#define MAP_MAC_MlmeFHSetReq                                                  ((uint8                     (*) ( uint16, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+180))
#define MAP_MAC_StartFH                                                       ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+181))
#define MAP_MAC_EnableFH                                                      ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+182))
#define MAP_MAC_MlmeWSAsyncReq                                                ((uint8                     (*) ( ApiMac_mlmeWSAsyncReq_t*))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+183))
#define MAP_macWSAsyncCnf                                                     ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+184))
#define MAP_macBuildWSAsyncMsg                                                ((uint8                     (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+185))
#define MAP_macBuildWSAsyncReq                                                ((uint8                     (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+186))
#define MAP_macApiWSAsyncReq                                                  ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+187))
#define MAP_macGetCSMPhy                                                      ((uint8                     (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+188))
#define MAP_MAC_RandomByte                                                    ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+189))
#define MAP_macCheckPhyRate                                                   ((uint8                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+190))
#define MAP_macCheckPhyMode                                                   ((uint8                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+191))

/* mac_pib.c */
//#define MAP_MAC_MlmeSetActivePib                                              ((void                      (*) ( void*))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+0))
#define MAP_macPibReset                                                       ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+192))
#define MAP_macPibIndex                                                       ((uint8                     (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+193))
#define MAP_MAC_MlmeGetReq                                                    ((uint8                     (*) ( uint8, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+194))
#define MAP_MAC_MlmeGetReqSize                                                ((uint8                     (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+195))
#define MAP_MAC_MlmeSetReq                                                    ((uint8                     (*) ( uint8, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+196))
#define MAP_MAC_GetPHYParamReq                                                ((uint8                     (*) ( uint8, uint8, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+197))
#define MAP_MAC_SetPHYParamReq                                                ((uint8                     (*) ( uint8, uint8, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+198))
#define MAP_macMRFSKGetPhyDesc                                                ((macMRFSKPHYDesc_t*        (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+199))
#define MAP_macIEEEGetPhyDesc                                                 ((macIEEEPHYDesc_t*         (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+200))

/* mac_pwr.c */
#define MAP_macPwrReset                                                       ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+201))
#define MAP_macApiPwrOnReq                                                    ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+202))
#define MAP_MAC_PwrOffReq                                                     ((uint8                     (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+203))
#define MAP_MAC_PwrOnReq                                                      ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+204))
#define MAP_MAC_PwrMode                                                       ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+205))
#define MAP_macPwrVote                                                        ((void                      (*) ( bool, bool))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+206))

/* mac_scan.c */
#define MAP_macScanCnfInit                                                    ((void                      (*) ( macMlmeScanCnf_t *, macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+207))
#define MAP_macApiScanReq                                                     ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+208))
#define MAP_macScanNextChan                                                   ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+209))
#define MAP_macScanStartTimer                                                 ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+210))
#define MAP_macScanRxBeacon                                                   ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+211))
#define MAP_macScanRxCoordRealign                                             ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+212))
#define MAP_macScanComplete                                                   ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+213))
#define MAP_macScanFailedInProgress                                           ((void                      (*) ( macEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+214))
#define MAP_MAC_MlmeScanReq                                                   ((ApiMac_status_t           (*) ( ApiMac_mlmeScanReq_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+215))

/* mac_security_pib.c */
//#define MAP_MAC_MlmeSetActiveSecurityPib                                      ((void                      (*) ( void*))   \
//                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+0))
#define MAP_macSecurityPibReset                                               ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+216))
#define MAP_macSecurityPibIndex                                               ((uint8                     (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+217))
#define MAP_MAC_MlmeGetSecurityReq                                            ((uint8                     (*) ( uint8, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+218))
#define MAP_MAC_MlmeGetPointerSecurityReq                                     ((uint8                     (*) ( uint8, void **))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+219))
#define MAP_MAC_MlmeGetSecurityReqSize                                        ((uint8                     (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+220))
#define MAP_MAC_MlmeSetSecurityReq                                            ((uint8                     (*) ( uint8, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+221))

/* mac_security.c */
#define MAP_macKeyDescriptorLookup                                            ((uint8                     (*) ( uint8 *, uint8, keyDescriptor_t **))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+222))
#define MAP_macIncomingSecurityLevelChecking                                  ((uint8                     (*) ( uint8, uint8, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+223))
#define MAP_macDeviceDescriptorLookup                                         ((uint8                     (*) ( deviceDescriptor_t *, uint8 *, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+224))
#define MAP_macBlacklistChecking                                              ((uint8                     (*) ( keyDescriptor_t *, uint8 *, uint8, deviceDescriptor_t **, keyDeviceDescriptor_t **))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+225))
#define MAP_macIncomingFrameSecurityMaterialRetrieval                         ((uint8                     (*) ( macRx_t *, keyDescriptor_t **, deviceDescriptor_t **, keyDeviceDescriptor_t **))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+226))
#define MAP_macIncomingKeyUsagePolicyChecking                                 ((uint8                     (*) ( keyDescriptor_t *, uint8, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+227))
#define MAP_macCcmStarInverseTransform                                        ((uint8                     (*) ( uint8 *, uint32, uint8, uint8 *, uint16, uint8 *, uint16, uint8 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+228))
#define MAP_macIncomingFrameSecurity                                          ((uint8                     (*) ( macRx_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+229))
#define MAP_macOutgoingFrameKeyDescRetrieval                                  ((keyDescriptor_t*          (*) ( ApiMac_sec_t *, sAddr_t *, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+230))
#define MAP_macOutgoingFrameSecurity                                          ((uint8                     (*) ( macTx_t *, sAddr_t *, uint16, keyDescriptor_t **))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+231))
#define MAP_macCcmStarTransform                                               ((uint8                     (*) ( uint8 *, uint32, uint8, uint8 *, uint16, uint8 *, uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+232))
#define MAP_macUpdatePanId                                                    ((void                      (*) ( uint16))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+233))
#define MAP_macGetEUIIndex                                                    ((uint16_t                  (*) ( sAddrExt_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+234))
#define MAP_macGetEUI                                                         ((sAddrExt_t*               (*) ( uint16_t))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+235))

/* mac_timer.c */
#define MAP_macTimerInit                                                      ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+236))
#define MAP_macTimerUpdateSymbolTimer                                         ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+237))
#define MAP_macTimerGetNextTimer                                              ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+238))
#define MAP_macSymbolTimerTriggerCallback                                     ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+239))
#define MAP_macTimerRecalcUnaligned                                           ((void                      (*) ( int32, macTimer_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+240))
#define MAP_macSymbolTimerRolloverCallback                                    ((void                      (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+241))
#define MAP_macTimerCheckUnAlignedQ                                           ((bool                      (*) ( macTimer_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+242))
#define MAP_macTimerAddTimer                                                  ((void                      (*) ( macTimer_t *, macTimerHeader_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+243))
#define MAP_macTimerAligned                                                   ((void                      (*) ( macTimer_t *, uint32))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+244))
#define MAP_macTimer                                                          ((void                      (*) ( macTimer_t *, uint32))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+245))
#define MAP_macTimerRemoveTimer                                               ((uint8                     (*) ( macTimer_t *, macTimerHeader_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+246))
#define MAP_macTimerCancel                                                    ((void                      (*) ( macTimer_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+247))
#define MAP_macTimerUpdActive                                                 ((void                      (*) ( macTimer_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+248))
#define MAP_macTimerGetTime                                                   ((uint32                    (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+249))
#define MAP_macTimerStart                                                     ((void                      (*) ( uint32, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+250))
#define MAP_macTimerRealign                                                   ((void                      (*) ( macRx_t *, uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+251))
#define MAP_macTimerSyncRollover                                              ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+252))
#define MAP_macTimerSetRollover                                               ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+253))
#define MAP_MAC_PwrNextTimeout                                                ((uint32                    (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+254))

/* macwrapper.c */
#define MAP_macWrapper8ByteUnused                                             ((int8                      (*) ( const uint8 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+255))
#define MAP_macWrapperAddDevice                                               ((uint8                     (*) ( ApiMac_secAddDevice_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+256))
#define MAP_macWrapperDeleteDevice                                            ((uint8                     (*) ( ApiMac_sAddrExt_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+257))
#define MAP_macWrapperDeleteKeyAndAssociatedDevices                           ((uint8                     (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+258))
#define MAP_macWrapperDeleteAllDevices                                        ((uint8                     (*) ( void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+259))
#define MAP_macWrapperGetDefaultSourceKey                                     ((uint8                     (*) ( uint8, uint32 *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+260))
#define MAP_macWrapperAddKeyInitFCtr                                          ((uint8                     (*) ( ApiMac_secAddKeyInitFrameCounter_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+261))

/* macstack.c */
#define MAP_MAC_CbackEvent                                                    ((void                      (*) (macCbackEvent_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+262))
#define MAP_MAC_CbackQueryRetransmit                                          ((uint8                     (*) (void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+263))

/*Api_mac.c */
#define MAP_ApiMac_convertCapabilityInfo                                      ((uint8_t                   (*) (ApiMac_capabilityInfo_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+264))
#define MAP_copyApiMacAddrToMacAddr                                           ((void                      (*) (sAddr_t *, ApiMac_sAddr_t *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+265))
#define MAP_convertTxOptions                                                  ((uint16_t                  (*) (ApiMac_txOptions_t))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+266))
/*assert*/
#define MAP_assertHandler                                                     ((void                      (*) (void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+267))

#define MAP_macMcuLongDiv                                                     ((uint32                    (*) (uint64_t dividend, uint32 divisor))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+268))

#define MAP_MAC_CbackCheckPending                                             ((uint8                     (*) (void))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+269))

/* mac_hl_patch.c */
#define MAP_macPibCheckByPatch                                                ((uint8                     (*) ( uint8, void *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+270))
#define MAP_macSetDefaultsByPatch                                             ((void                      (*) ( uint8))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+271))
#define MAP_macCcmEncrypt                                                     ((uint8                     (*) ( AESCCM_Transaction *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+272))
#define MAP_macCcmDecrypt                                                     ((uint8                     (*) ( AESCCM_Transaction *))   \
                   ROM_HMAC_JT_OFFSET(HMAC_API_BASE_INDEX+273))
#endif
