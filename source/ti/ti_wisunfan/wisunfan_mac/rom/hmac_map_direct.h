/******************************************************************************

 @file hmac_map_direct.h

 @brief HMAC API directly map the function implementation (declaration)

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

#ifndef HMAC_MAP_DIRECT_H
#define HMAC_MAP_DIRECT_H

#include "api_mac.h"
#include "timac_api.h"
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
#include <crypto_mac_api.h>

#ifdef FEATURE_BEACON_MODE
/* Coordinator action set 1 */
extern macAction_t macBeaconCoordAction1[2];

extern macBeaconCoord_t macBeaconCoord;
#endif

//#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
#if 1
/* Device beacon action function table */
extern macAction_t macBeaconDeviceAction[4];

extern macBeaconDevice_t macBeaconDevice;

extern uint16 macBeaconMargin[];
/* mac_beacon data */
extern macBeacon_t macBeacon;
#endif

/* Coordinator action set 1 */
extern macAction_t macCoordAction1[2];

/* Coordinator action set 2 */
extern macAction_t macCoordAction2[7];

/* Coordinator action set 3 */
extern macAction_t macCoordAction3[4];

/* Coordinator info */
extern macCoord_t  macCoord;

/* Table of which IE's to be included for the specific command indexed by command identifier */
extern const uint32 macCmdIncludeFHIe[];
/* Table of command frame sizes indexed by command identifier */
extern const uint8 macCmdBufSize[] ;
/* Table of MAC command payload length by command identifier */
extern const uint8 macCmdLen[];


/* TX frame success to event */
extern const uint8 macTxFrameSuccess[] ;
/* TX frame failure to event */
extern const uint8 macTxFrameFailed[] ;

/* mac_data data */
extern macData_t macData;

/* pointer to current tx frame */
extern macTx_t *pMacDataTx ;

/* indirect data handling functions */
extern macDataTxFcn_t macDataTxIndirect;
extern macDataTxFcn_t macDataRequeueIndirect;

/* critical beacon handling function */
extern macRxBeaconCritical_t macDataRxBeaconCritical ;

/* beacon tx complete function */
extern macTxBeaconComplete_t macDataTxBeaconComplete ;

/* tx frame set schedule function */
extern macDataTxFcn_t macDataSetSched ;

/* tx frame check schedule function */
extern macDataCheckSched_t macDataCheckSched ;

/* tx frame check tx time function */
extern macDataCheckTxTime_t macDataCheckTxTime ;

/* tx frame beacon requeue function */
extern macDataTxFcn_t macDataBeaconRequeue ;

/* tx timer for Green Power delayed transmission */
extern macTimer_t macDataTxDelay;

extern macAction_t macDeviceAction1[14];

/* Device action set 2 */
extern macAction_t macDeviceAction2[5];

#ifdef FEATURE_WISUN_SUPPORT
/* Device action set 3 */
extern macAction_t macDeviceAction3[1];
/* Device action set 4 */
extern macAction_t macDeviceAction4[5];
#endif

/* MAC device info */
extern macDevice_t macDevice;


/* State table for MAC_IDLE_ST */
extern const uint8 macIdleSt[][MAC_NUM_COLS] ;
/* State table for MAC_ASSOCIATING_ST */
extern const uint8 macAssociatingSt[][MAC_NUM_COLS] ;
/* State table for MAC_POLLING_ST */
extern const uint8 macPollingSt[][MAC_NUM_COLS] ;

#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
/* State table for MAC_SCANNING_ST */
extern const uint8 macScanningSt[][MAC_NUM_COLS] ;
#endif /* (FEATURE_BEACON_MODE) || (FEATURE_NON_BEACON_MODE) */

/* State table for MAC_STARTING_ST */
extern const uint8 macStartingSt[][MAC_NUM_COLS];
/* Message buffer size table used by macSendMsg */
extern const uint16 macSendMsgBufSize[] ;
/* Parameter data size table used by macSendMsg */
extern const uint8 macSendMsgDataSize[] ;

/* Lookup callback event based on MAC event */
extern const uint8 macCbackEventTbl[] ;

/* OSAL task id */
extern uint8 macTaskId;

/* mac_main data */
extern macMain_t macMain;

/* State table */
extern macStateTbl_t macStateTbl[];


/* initialization functions */
extern macReset_t macReset[MAC_FEAT_MAX];

/* Table of state machine action function table pointers */
extern macActionSet_t macActionSet[MAC_ACTION_SET_MAX] ;


/* MAC mangement info */
extern macMgmt_t macMgmt;

/* TRUE if operating as a PAN coordinator */
extern bool macPanCoordinator;

#ifdef FEATURE_WISUN_EDFE_SUPPORT
extern macEdfeInfo_t macEdfeInfo;
extern uint8_t edfeEfrmBuf[MAC_EDFE_MAX_FRAME];
#endif

/* Action set 1 */
extern macAction_t macMgmtAction1[12];

/* Action set 2 */
extern macAction_t macMgmtAction2[5];

/**
  Flag which identifies whether frequency hopping is enabled or
  not.
*/
//extern bool freqHopEnabled ;

/**
  frequency hopping object. Holds the function pointers related
  to frequency hopping module.
*/
extern fhObject_t fhObject ;

extern bool csmPhy ;

extern const macPib_t macPibDefaults;
extern const macPibTbl_t macPibTbl[];

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)

/* MAC 15.4g Standard PHY Descriptor Table */
extern const macMRFSKPHYDesc_t macMRFSKStdPhyTable[MAC_STANDARD_PHY_DESCRIPTOR_ENTRIES];
extern macMRFSKPHYDesc_t macMRFSKGenPhyTable[MAC_GENERIC_PHY_DESCRIPTOR_ENTRIES];

#endif

#if defined(COMBO_MAC) || defined(FREQ_2_4G)

/* MAC 15.4 IEEE PHY Descriptor Table */
extern const macIEEEPHYDesc_t macIEEEPhyTable ;
#endif

/* MAC PIB */
extern macPib_t macPib;
#define pMacPib (&macPib)

extern macTimerHeader_t timerUnalignedQ;
extern macTimerHeader_t timerAlignedQ;

extern int32 macTimerRollover;
extern int32 macTimerNewRollover;
extern macTimer_t *macTimerActive;

/* timer rollover table indexed by beacon order */
extern const uint32 CODE macTimerRolloverValue[] ;

//#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
#if 1
extern macAction_t macScanAction[6];

extern macScan_t macScan;
#endif

#ifdef FEATURE_MAC_SECURITY
/* Security related MAC PIB */
extern macSecurityPib_t macSecurityPib;

#ifdef STATIC_MAC_SECURITY_DEFAULTS
/* Security related PIB default values */
extern const macSecurityPib_t macSecurityPibDefaults ;
#endif

/* PIB access and min/max table.  min/max of 0/0 means not checked; if min/max are
 * equal, element is read-only
 */
extern const macSecurityPibTbl_t macSecurityPibTbl[] ;

#if defined( FEATURE_MAC_PIB_PTR )

/* Pointer to the mac security PIB */
extern macSecurityPib_t* pMacSecurityPib ;

#endif

#endif

#define pMacSecurityPib (&macSecurityPib)

/* configurable parameters */
extern macCfg_t macCfg;

#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC13X4)
extern CryptoCC26XX_Handle Crypto_handle;
#else
extern AESCCM_Handle AESCCM_handle;
#endif

/* Crypto driver function table */
extern uint32_t *macCryptoDrvTblPtr;

extern CODE const uint8 macAuthTagLen[];

/* Length of key source indexed by key identifier mode */
extern CODE const uint8 macKeySourceLen[];

#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)

extern uint8 macBeaconPayload[MAC_PIB_MAX_BEACON_PAYLOAD_LEN];

#endif

extern uint16 *macTasksEvents;

extern void assertHandler(void);
/*
**  HMAC API directly map to implementation
**  This is used in CC131X/CC135X project with using any TIMAC 15.4 ROM image
*/

/*
**  HMAC API directly map to implementation
**  This is used in CC131X/CC135X project with using any TIMAC 15.4 ROM image
*/
/* mac_beacon_coord.c */
#define MAP_macBeaconCoordReset                        macBeaconCoordReset
#define MAP_MAC_InitBeaconCoord                        MAC_InitBeaconCoord
#define MAP_macBeaconSetPrepareTime                    macBeaconSetPrepareTime
#define MAP_macBeaconCheckStartTime                    macBeaconCheckStartTime
#define MAP_macApiBeaconStartReq                       macApiBeaconStartReq
#define MAP_macBeaconPeriodCallback                    macBeaconPeriodCallback
#define MAP_macBeaconTxCallback                        macBeaconTxCallback
//#define MAP_macEBeaconTxCallback                       macEBeaconTxCallback
//#define MAP_macEBeaconPrepareCallback                  macEBeaconPrepareCallback
#define MAP_macBeaconPrepareCallback                   macBeaconPrepareCallback
#define MAP_macStartBeaconPrepareCallback              macStartBeaconPrepareCallback
#define MAP_macBeaconStartContinue                     macBeaconStartContinue
#define MAP_macBeaconBattLifeCallback                  macBeaconBattLifeCallback
#define MAP_macTxBeaconCompleteCallback                macTxBeaconCompleteCallback
#define MAP_macBeaconSetupBroadcast                    macBeaconSetupBroadcast
#define MAP_macOutgoingNonSlottedTx                    macOutgoingNonSlottedTx
#define MAP_macBeaconSchedRequested                    macBeaconSchedRequested
#define MAP_macBeaconClearIndirect                     macBeaconClearIndirect

/* mac_beacon_device.c */
#define MAP_macBroadcastPendCallback                   macBroadcastPendCallback
#define MAP_macBeaconStopTrack                         macBeaconStopTrack
#define MAP_macIncomingNonSlottedTx                    macIncomingNonSlottedTx
#define MAP_macBeaconSyncLoss                          macBeaconSyncLoss
#define MAP_macSyncTimeoutCallback                     macSyncTimeoutCallback
#define MAP_macTrackTimeoutCallback                    macTrackTimeoutCallback
#define MAP_macTrackStartCallback                      macTrackStartCallback
#define MAP_macTrackPeriodCallback                     macTrackPeriodCallback
#define MAP_macRxBeaconCritical                        macRxBeaconCritical
#define MAP_MAC_InitBeaconDevice                       MAC_InitBeaconDevice
#define MAP_macCheckPendAddr                           macCheckPendAddr
#define MAP_macAutoPoll                                macAutoPoll
#define MAP_macApiSyncReq                              macApiSyncReq
#define MAP_macBeaconStartFrameResponseTimer           macBeaconStartFrameResponseTimer
#define MAP_macStartBroadcastPendTimer                 macStartBroadcastPendTimer

/* mac_beacon.c */
#define MAP_macBeaconReset                             macBeaconReset
#define MAP_macBeaconInit                              macBeaconInit
#define MAP_macBeaconSetSched                          macBeaconSetSched
#define MAP_macBeaconCheckSched                        macBeaconCheckSched
#define MAP_macBeaconRequeue                           macBeaconRequeue
#define MAP_macBeaconCheckTxTime                       macBeaconCheckTxTime
#define MAP_macBeaconSetupCap                          macBeaconSetupCap

/* mac_coord.c */
#define MAP_macCoordReset                              macCoordReset
#define MAP_macBuildAssociateRsp                       macBuildAssociateRsp
#define MAP_macBuildRealign                            macBuildRealign
#define MAP_macPendAddrLen                             macPendAddrLen
#define MAP_macBuildPendAddr                           macBuildPendAddr
//#define MAP_macBuildEnhancedBeacon                     macBuildEnhancedBeacon
#define MAP_macBuildBeacon                             macBuildBeacon
#define MAP_macIndirectMark                            macIndirectMark
#define MAP_macIndirectSend                            macIndirectSend
#define MAP_macIndirectExpire                          macIndirectExpire
#define MAP_macIndirectTxFrame                         macIndirectTxFrame
#define MAP_macIndirectRequeueFrame                    macIndirectRequeueFrame
#define MAP_macRxAssociateReq                          macRxAssociateReq
#define MAP_macApiAssociateRsp                         macApiAssociateRsp
#define MAP_macRxOrphan                                macRxOrphan
#define MAP_macApiOrphanRsp                            macApiOrphanRsp
#define MAP_macApiPurgeReq                             macApiPurgeReq
#define MAP_macApiStartReq                             macApiStartReq
#define MAP_macStartBegin                              macStartBegin
#define MAP_macStartSetParams                          macStartSetParams
#define MAP_macStartContinue                           macStartContinue
#define MAP_macStartComplete                           macStartComplete
#define MAP_macRxBeaconReq                             macRxBeaconReq
//#define MAP_macProcessEBeaconFilterIe                  macProcessEBeaconFilterIe
//#define MAP_macRxEnhancedBeaconReq                     macRxEnhancedBeaconReq
#define MAP_macRxDataReq                               macRxDataReq
#define MAP_macRxPanConflict                           macRxPanConflict
#define MAP_MAC_InitCoord                              MAC_InitCoord
#define MAP_MAC_MlmeAssociateRsp                       MAC_MlmeAssociateRsp
#define MAP_MAC_MlmeOrphanRsp                          MAC_MlmeOrphanRsp
#define MAP_MAC_McpsPurgeReq                           MAC_McpsPurgeReq
#define MAP_MAC_MlmeStartReq                           MAC_MlmeStartReq

/* mac_data.c */
#define MAP_macDataTxDelayCallback                     macDataTxDelayCallback
#define MAP_macDataReset                               macDataReset
#define MAP_macAllocTxBuffer                           macAllocTxBuffer
#define MAP_macFrameDurationSubG                       macFrameDurationSubG
#define MAP_macFrameDurationIeee                       macFrameDurationIeee
#define MAP_macFrameDuration                           macFrameDuration
#define MAP_macRxCheckPendingCallback                  macRxCheckPendingCallback
#define MAP_macCheckIndPacket                          macCheckIndPacket
#define MAP_macRxCheckMACPendingCallback               macRxCheckMACPendingCallback
#define MAP_macDataRxMemAlloc                          macDataRxMemAlloc
#define MAP_macDataRxMemFree                           macDataRxMemFree
#define MAP_macRxCompleteCallback                      macRxCompleteCallback
#define MAP_macDataTxTimeAvailable                     macDataTxTimeAvailable
#define MAP_macBuildDataFrame                          macBuildDataFrame
#define MAP_macCheckSched                              macCheckSched
#define MAP_macSetSched                                macSetSched
#define MAP_macDataTxComplete                          macDataTxComplete
#define MAP_macTxCompleteCallback                      macTxCompleteCallback
#define MAP_macDataTxSend                              macDataTxSend
#define MAP_macDataTxEnqueue                           macDataTxEnqueue
#define MAP_macDataSend                                macDataSend
#define MAP_macApiDataReq                              macApiDataReq
#define MAP_macDataRxInd                               macDataRxInd
#define MAP_MAC_McpsDataReq                            MAC_McpsDataReq
#define MAP_MAC_McpsDataAlloc                          MAC_McpsDataAlloc
#define MAP_macAutoPendMaintainSrcMatchTable           macAutoPendMaintainSrcMatchTable
#define MAP_macAutoPendAddSrcMatchTableEntry           macAutoPendAddSrcMatchTableEntry

/* mac_device.c */
#define MAP_macDeviceReset                             macDeviceReset
#define MAP_macAssociateCnf                            macAssociateCnf
#define MAP_macPollCnf                                 macPollCnf
#define MAP_macBuildAssociateReq                       macBuildAssociateReq
#define MAP_macStartResponseTimer                      macStartResponseTimer
#define MAP_macStartFrameResponseTimer                 macStartFrameResponseTimer
#define MAP_macApiAssociateReq                         macApiAssociateReq
#define MAP_macAssocDataReq                            macAssocDataReq
#define MAP_macAssocDataReqComplete                    macAssocDataReqComplete
#define MAP_macRxAssocRsp                              macRxAssocRsp
#define MAP_macAssocFrameResponseTimeout               macAssocFrameResponseTimeout
#define MAP_macAssocFailed                             macAssocFailed
#define MAP_macAssocRxDisassoc                         macAssocRxDisassoc
#define MAP_macAssocDataRxInd                          macAssocDataRxInd
#define MAP_macApiPollReq                              macApiPollReq
#define MAP_macPollDataReqComplete                     macPollDataReqComplete
#define MAP_macPollDataRxInd                           macPollDataRxInd
#define MAP_macPollFrameResponseTimeout                macPollFrameResponseTimeout
#define MAP_macPollRxDisassoc                          macPollRxDisassoc
#define MAP_macPollRxAssocRsp                          macPollRxAssocRsp
#define MAP_macRxCoordRealign                          macRxCoordRealign
#define MAP_macPanConflictComplete                     macPanConflictComplete
#define MAP_MAC_InitDevice                             MAC_InitDevice
#define MAP_MAC_MlmeAssociateReq                       MAC_MlmeAssociateReq
#define MAP_MAC_MlmePollReq                            MAC_MlmePollReq

/* mac_ie_build.c */
#define MAP_macBuildHeaderIes                          macBuildHeaderIes
#define MAP_macBuildPayloadCoexistIe                   macBuildPayloadCoexistIe
#define MAP_macBuildPayloadIes                         macBuildPayloadIes

/* mac_ie_parse.c */
#define MAP_macUpdateHeaderIEPtr                       macUpdateHeaderIEPtr
#define MAP_macUpdatePayloadIEPtr                      macUpdatePayloadIEPtr
#define MAP_macParseHeaderIes                          macParseHeaderIes
#define MAP_macParsePayloadIes                         macParsePayloadIes
#define MAP_macIeFreeList                              macIeFreeList
#define MAP_macIeCoexistIePresent                      macIeCoexistIePresent
#define MAP_macMlmeSubIePresent                        macMlmeSubIePresent

/* mac_main.c */
#define MAP_macMainReset                               macMainReset
#define MAP_macSetEvent                                macSetEvent
#define MAP_macSetEvent16                              macSetEvent16
#define MAP_macCbackForEvent                           macCbackForEvent
#define MAP_macStateIdle                               macStateIdle
#define MAP_macStateIdleOrPolling                      macStateIdleOrPolling
#define MAP_macStateScanning                           macStateScanning
#define MAP_macExecute                                 macExecute
#define MAP_macSendMsg                                 macSendMsg
#define MAP_macSendDataMsg                             macSendDataMsg
#define MAP_macMainSetTaskId                           macMainSetTaskId
#define MAP_macMainReserve                             macMainReserve
#define MAP_mac_msg_deallocate                         mac_msg_deallocate

/* mac_mgmt.c */ //not done
#define MAP_macMgmtReset                               macMgmtReset
#define MAP_macGetCoordAddress                         macGetCoordAddress
#define MAP_macGetMyAddrMode                           macGetMyAddrMode
#define MAP_macDestAddrCmp                             macDestAddrCmp
#define MAP_macDestSAddrCmp                            macDestSAddrCmp
#define MAP_macCoordAddrCmp                            macCoordAddrCmp
#define MAP_macCoordDestAddrCmp                        macCoordDestAddrCmp
#define MAP_macBuildHeader                             macBuildHeader
#define MAP_macBuildCommonReq                          macBuildCommonReq
//#define MAP_macBuildEnhanceBeaconReq                   macBuildEnhanceBeaconReq
//#define MAP_macBuildCoexistEBeaconReq                  macBuildCoexistEBeaconReq
#define MAP_macBuildDisassociateReq                    macBuildDisassociateReq
//#define MAP_macBuildEBeaconNotifyInd                   macBuildEBeaconNotifyInd
#define MAP_macBuildBeaconNotifyInd                    macBuildBeaconNotifyInd
#define MAP_macSecCpy                                  macSecCpy
#define MAP_macNoAction                                macNoAction
#define MAP_macTxCsmaDelay                             macTxCsmaDelay
#define MAP_macBcTimerHandler                          macBcTimerHandler
#define MAP_macApiUnsupported                          macApiUnsupported
#define MAP_macDefaultAction                           macDefaultAction
#define MAP_macApiBadState                             macApiBadState
#define MAP_macApiPending                              macApiPending
#define MAP_macCommStatusInd                           macCommStatusInd
#define MAP_macApiDisassociateReq                      macApiDisassociateReq
#define MAP_macDisassocComplete                        macDisassocComplete
#define MAP_macRxDisassoc                              macRxDisassoc
#define MAP_macRxBeacon                                macRxBeacon
#define MAP_macConflictSyncLossInd                     macConflictSyncLossInd
#define MAP_MAC_MlmeDisassociateReq                    MAC_MlmeDisassociateReq
#define MAP_macUpdateCsmPhyParam                       macUpdateCsmPhyParam
#define MAP_macIsCsmOperational                        macIsCsmOperational
#define MAP_MAC_MlmeResetReq                           MAC_MlmeResetReq
#define MAP_MAC_ResumeReq                              MAC_ResumeReq
#define MAP_MAC_YieldReq                               MAC_YieldReq
#define MAP_MAC_MlmeSyncReq                            MAC_MlmeSyncReq
#define MAP_macSetDefaultsByPhyID                      macSetDefaultsByPhyID
#define MAP_macSetDefaultsByRE                         macSetDefaultsByRE
#define MAP_MAC_Init                                   MAC_Init
#define MAP_MAC_InitFH                                 MAC_InitFH
#define MAP_MAC_MlmeFHGetReqSize                       MAC_MlmeFHGetReqSize
#define MAP_MAC_MlmeFHGetReq                           MAC_MlmeFHGetReq
#define MAP_MAC_MlmeFHSetReq                           MAC_MlmeFHSetReq
#define MAP_MAC_StartFH                                MAC_StartFH
#define MAP_MAC_EnableFH                               MAC_EnableFH
#define MAP_MAC_MlmeWSAsyncReq                         MAC_MlmeWSAsyncReq
#define MAP_macWSAsyncCnf                              macWSAsyncCnf
#define MAP_macBuildWSAsyncMsg                         macBuildWSAsyncMsg
#define MAP_macBuildWSAsyncReq                         macBuildWSAsyncReq
#define MAP_macApiWSAsyncReq                           macApiWSAsyncReq
#define MAP_macGetCSMPhy                               macGetCSMPhy
#define MAP_MAC_RandomByte                             MAC_RandomByte
#define MAP_macLowLevelYield                           macLowLevelYield
//#define MAP_macCheckPhyLRMMode                         macCheckPhyLRMMode
//#define MAP_macCheckPhyFSKMode                         macCheckPhyFSKMode

/* mac_pib.c */
//#define MAP_MAC_MlmeSetActivePib                       MAC_MlmeSetActivePib
#define MAP_macPibReset                                macPibReset
#define MAP_macPibIndex                                macPibIndex
#define MAP_MAC_MlmeGetReq                             MAC_MlmeGetReq
#define MAP_MAC_MlmeGetReqSize                         MAC_MlmeGetReqSize
#define MAP_MAC_MlmeSetReq                             MAC_MlmeSetReq
#define MAP_MAC_GetPHYParamReq                         MAC_GetPHYParamReq
#define MAP_MAC_SetPHYParamReq                         MAC_SetPHYParamReq
#define MAP_macMRFSKGetPhyDesc                         macMRFSKGetPhyDesc
#define MAP_macIEEEGetPhyDesc                          macIEEEGetPhyDesc

/* mac_pwr.c */
#define MAP_macPwrReset                                macPwrReset
#define MAP_macApiPwrOnReq                             macApiPwrOnReq
#define MAP_MAC_PwrOffReq                              MAC_PwrOffReq
#define MAP_MAC_PwrOnReq                               MAC_PwrOnReq
#define MAP_MAC_PwrMode                                MAC_PwrMode
#define MAP_macPwrVote                                 macPwrVote

/* mac_scan.c */
#define MAP_macScanCnfInit                             macScanCnfInit
#define MAP_macApiScanReq                              macApiScanReq
#define MAP_macScanNextChan                            macScanNextChan
#define MAP_macScanStartTimer                          macScanStartTimer
#define MAP_macScanRxBeacon                            macScanRxBeacon
#define MAP_macScanRxCoordRealign                      macScanRxCoordRealign
#define MAP_macScanComplete                            macScanComplete
#define MAP_macScanFailedInProgress                    macScanFailedInProgress
#define MAP_MAC_MlmeScanReq                            MAC_MlmeScanReq

/* mac_security_pib.c */
//#define MAP_MAC_MlmeSetActiveSecurityPib               MAC_MlmeSetActiveSecurityPib
#define MAP_macSecurityPibReset                        macSecurityPibReset
#define MAP_macSecurityPibIndex                        macSecurityPibIndex
#define MAP_MAC_MlmeGetSecurityReq                     MAC_MlmeGetSecurityReq
#define MAP_MAC_MlmeGetPointerSecurityReq              MAC_MlmeGetPointerSecurityReq
#define MAP_MAC_MlmeGetSecurityReqSize                 MAC_MlmeGetSecurityReqSize
#define MAP_MAC_MlmeSetSecurityReq                     MAC_MlmeSetSecurityReq

/* mac_security.c */
#define MAP_macKeyDescriptorLookup                     macKeyDescriptorLookup
#define MAP_macIncomingSecurityLevelChecking           macIncomingSecurityLevelChecking
#define MAP_macDeviceDescriptorLookup                  macDeviceDescriptorLookup
#define MAP_macBlacklistChecking                       macBlacklistChecking
#define MAP_macIncomingFrameSecurityMaterialRetrieval  macIncomingFrameSecurityMaterialRetrieval
#define MAP_macIncomingKeyUsagePolicyChecking          macIncomingKeyUsagePolicyChecking
#define MAP_macCcmStarInverseTransform                 macCcmStarInverseTransform
#define MAP_macIncomingFrameSecurity                   macIncomingFrameSecurity
#define MAP_macOutgoingFrameKeyDescRetrieval           macOutgoingFrameKeyDescRetrieval
#define MAP_macOutgoingFrameSecurity                   macOutgoingFrameSecurity
#define MAP_macCcmStarTransform                        macCcmStarTransform
#define MAP_macUpdatePanId                             macUpdatePanId
#define MAP_macGetEUIIndex                             macGetEUIIndex
#define MAP_macGetEUI                                  macGetEUI

/* mac_timer.c */
#define MAP_macTimerInit                               macTimerInit
#define MAP_macTimerUpdateSymbolTimer                  macTimerUpdateSymbolTimer
#define MAP_macTimerGetNextTimer                       macTimerGetNextTimer
#define MAP_macSymbolTimerTriggerCallback              macSymbolTimerTriggerCallback
#define MAP_macTimerRecalcUnaligned                    macTimerRecalcUnaligned
#define MAP_macSymbolTimerRolloverCallback             macSymbolTimerRolloverCallback
#define MAP_macTimerCheckUnAlignedQ                    macTimerCheckUnAlignedQ
#define MAP_macTimerAddTimer                           macTimerAddTimer
#define MAP_macTimerAligned                            macTimerAligned
#define MAP_macTimer                                   macTimer
#define MAP_macTimerRemoveTimer                        macTimerRemoveTimer
#define MAP_macTimerCancel                             macTimerCancel
#define MAP_macTimerUpdActive                          macTimerUpdActive
#define MAP_macTimerGetTime                            macTimerGetTime
#define MAP_macTimerStart                              macTimerStart
#define MAP_macTimerRealign                            macTimerRealign
#define MAP_macTimerSyncRollover                       macTimerSyncRollover
#define MAP_macTimerSetRollover                        macTimerSetRollover
#define MAP_MAC_PwrNextTimeout                         MAC_PwrNextTimeout

/* macwrapper.c */
#define MAP_macWrapper8ByteUnused                      macWrapper8ByteUnused
#define MAP_macWrapperAddDevice                        macWrapperAddDevice
#define MAP_macWrapperDeleteDevice                     macWrapperDeleteDevice
#define MAP_macWrapperDeleteKeyAndAssociatedDevices    macWrapperDeleteKeyAndAssociatedDevices
#define MAP_macWrapperDeleteAllDevices                 macWrapperDeleteAllDevices
#define MAP_macWrapperGetDefaultSourceKey              macWrapperGetDefaultSourceKey
#define MAP_macWrapperAddKeyInitFCtr                   macWrapperAddKeyInitFCtr

/* mac_stack.c */
#define MAP_MAC_CbackEvent                             MAC_CbackEvent
#define MAP_MAC_CbackQueryRetransmit                   MAC_CbackQueryRetransmit
/*Api_mac.c */
#define MAP_ApiMac_convertCapabilityInfo               ApiMac_convertCapabilityInfo
#define MAP_copyApiMacAddrToMacAddr                    copyApiMacAddrToMacAddr
#define MAP_convertTxOptions                           convertTxOptions

#define MAP_assertHandler                              assertHandler

#define MAP_macMcuLongDiv                              macMcuLongDiv

#define MAP_MAC_CbackCheckPending                      MAC_CbackCheckPending

/* mac_hl_patch.c */
#define MAP_macCheckPhyRate                            macCheckPhyRate
#define MAP_macCheckPhyMode                            macCheckPhyMode
#define MAP_macPibCheckByPatch                         macPibCheckByPatch
#define MAP_macSetDefaultsByPatch                      macSetDefaultsByPatch
#define MAP_macCcmEncrypt                              macCcmEncrypt
#define MAP_macCcmDecrypt                              macCcmDecrypt
#endif
