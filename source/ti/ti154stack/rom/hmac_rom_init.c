/******************************************************************************

 @file  hmac_rom_init.c

 @brief This file contains the externs for ROM API initialization.

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
#include "mac_mcu.h"
#include "mac_data.h"
#include <crypto_mac_api.h>
#include "mac_hl_patch.h"

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
 * GLOBAL VARIABLES
 */


// ROM Flash Jump Table

#if defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(HMAC_ROM_Flash_JT, 4)
#elif defined(__GNUC__) || defined(__clang__)
__attribute__ ((aligned (4)))
#else
#pragma data_alignment=4
#endif

//#ifdef FEATURE_BEACON_MODE
/* Coordinator action set 1 */
extern macAction_t macBeaconCoordAction1[2];

extern macBeaconCoord_t macBeaconCoord;
//#endif

//#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
#if 1
/* Device beacon action function table */
extern macAction_t macBeaconDeviceAction[4];

extern macBeaconDevice_t macBeaconDevice;
#if defined (FEATURE_BEACON_MODE)
extern uint16 macBeaconMargin[];
#endif
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

/* Action set 1 */
extern macAction_t macMgmtAction1[12];

/* Action set 2 */
extern macAction_t macMgmtAction2[5];
/**
  Flag which identifies whether frequency hopping is enabled or
  not.
*/
extern bool freqHopEnabled ;

/**
  frequency hopping object. Holds the function pointers related
  to frequency hopping module.
*/
extern fhObject_t fhObject ;

extern bool csmPhy ;

extern const macPib_t macPibDefaults;
extern const macPibTbl_t macPibTbl[];
extern macSecurityPib_t macSecurityPib;
extern const macSecurityPibTbl_t macSecurityPibTbl[] ;

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

extern macCfg_t macCfg;

/* Crypto driver handle */
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC26X2X7) && !defined(DeviceFamily_CC13X1) && !defined(DeviceFamily_CC26X1) && !defined(DeviceFamily_CC13X4) && !defined(DeviceFamily_CC26X4) && !defined(DeviceFamily_CC26X3)
extern CryptoCC26XX_Handle Crypto_handle;
#else
extern AESCCM_Handle AESCCM_handle;
#endif

extern CODE const uint8 macAuthTagLen[];

/* Length of key source indexed by key identifier mode */
extern CODE const uint8 macKeySourceLen[];

#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)

extern uint8 macBeaconPayload[MAC_PIB_MAX_BEACON_PAYLOAD_LEN];

#endif
extern uint16 *macTasksEvents;
/*******************************************************************************
 * FUNCTION
 */

/* mac_beacon_coord.c */
extern void macBeaconCoordReset(void);
extern void MAC_InitBeaconCoord(void);
extern void macBeaconSetPrepareTime(void);
extern uint8 macBeaconCheckStartTime(macEvent_t *pEvent);
extern void macApiBeaconStartReq(macEvent_t *pEvent);
extern void macBeaconPeriodCallback(uint8 param);
extern void macBeaconTxCallback(uint8 param);
//extern void macEBeaconTxCallback(uint8 param);
//extern void macEBeaconPrepareCallback(uint8 param);
extern void macBeaconPrepareCallback(uint8 param);
extern void macStartBeaconPrepareCallback(uint8 param);
extern void macBeaconStartContinue(macEvent_t *pEvent);
extern void macBeaconBattLifeCallback(uint8 param);
extern void macTxBeaconCompleteCallback(uint8 status);
extern void macBeaconSetupBroadcast(void);
extern void macOutgoingNonSlottedTx(void);
extern void macBeaconSchedRequested(void);
extern void macBeaconClearIndirect(void);

/* mac_beacon_device.c */
extern void macBroadcastPendCallback(uint8 param);
extern void macBeaconStopTrack(void);
extern void macIncomingNonSlottedTx(void);
extern void macBeaconSyncLoss(void);
extern void macSyncTimeoutCallback(uint8 param);
extern void macTrackTimeoutCallback(uint8 param);
extern void macTrackStartCallback(uint8 param);
extern void macTrackPeriodCallback(uint8 param);
extern void macRxBeaconCritical(macRx_t *pBuf);
extern void MAC_InitBeaconDevice(void);
extern uint8 macCheckPendAddr(uint8 pendAddrSpec, uint8 *pAddrList);
extern void macAutoPoll(macEvent_t *pEvent);
extern void macApiSyncReq(macEvent_t *pEvent);
extern void macBeaconStartFrameResponseTimer(macEvent_t *pEvent);
extern void macStartBroadcastPendTimer(macEvent_t *pEvent);

/* mac_beacon.c */
extern void macBeaconReset(void);
extern void macBeaconInit(void);
extern void macBeaconSetSched(macTx_t *pMsg);
extern uint8 macBeaconCheckSched(void);
extern void macBeaconRequeue(macTx_t *pMsg);
extern uint16 macBeaconCheckTxTime(void);
extern void macBeaconSetupCap(uint8 sched, uint8 superframeOrder, uint16 beaconLen);

/* mac_coord.c */
extern void macCoordReset(void);
extern uint8 macBuildAssociateRsp(macEvent_t *pEvent);
extern uint8 macBuildRealign(macTx_t *pMsg, sAddr_t *pDstAddr, uint16 dstShortAddr, uint16 newPanId, uint8 logicalChannel);
extern uint8 macPendAddrLen(uint8 *maxPending);
extern uint8* macBuildPendAddr(uint8 *p, uint8 maxPending, bool *pBroadcast);
//extern macTx_t* macBuildEnhancedBeacon(uint8 beaconOrder, uint8 superframeOrder, bool battLifeExt, uint8 coexistIe);
extern macTx_t* macBuildBeacon(uint8 beaconOrder, uint8 superframeOrder, bool battLifeExt);
extern void macIndirectMark(macTx_t *pMsg);
extern bool macIndirectSend(sAddr_t *pAddr, uint16 panId);
extern void macIndirectExpire(macEvent_t *pEvent);
extern void macIndirectTxFrame(macTx_t *pMsg);
extern void macIndirectRequeueFrame(macTx_t *pMsg);
extern void macRxAssociateReq(macEvent_t *pEvent);
extern void macApiAssociateRsp(macEvent_t *pEvent);
extern void macRxOrphan(macEvent_t *pEvent);
extern void macApiOrphanRsp(macEvent_t *pEvent);
extern void macApiPurgeReq(macEvent_t *pEvent);
extern void macApiStartReq(macEvent_t *pEvent);
extern uint8 macStartBegin(macEvent_t *pEvent);
extern void macStartSetParams(ApiMac_mlmeStartReq_t *pStartReq);
extern void macStartContinue(macEvent_t *pEvent);
extern void macStartComplete(macEvent_t *pEvent);
extern void macRxBeaconReq(macEvent_t *pEvent);
//extern bool macProcessEBeaconFilterIe( macEvent_t* pEvent, uint8* pContent, uint32* pAttribIds );
//extern void macRxEnhancedBeaconReq(macEvent_t *pEvent);
extern void macRxDataReq(macEvent_t *pEvent);
extern void macRxPanConflict(macEvent_t *pEvent);
extern void MAC_InitCoord(void);
extern uint8 MAC_MlmeAssociateRsp(ApiMac_mlmeAssociateRsp_t *pData);
extern uint8 MAC_MlmeOrphanRsp(ApiMac_mlmeOrphanRsp_t *pData);
extern uint8 MAC_McpsPurgeReq(uint8 msduHandle);
extern uint8 MAC_MlmeStartReq(ApiMac_mlmeStartReq_t *pData);

/* mac_data.c */
extern void macDataTxDelayCallback(uint8 param);
extern void macDataReset(void);
extern macTx_t* macAllocTxBuffer(uint8 cmd, ApiMac_sec_t *sec);
extern uint16 macFrameDurationSubG(uint16 len, uint16 txOptions);
extern uint16 macFrameDurationIeee(uint16 len, uint16 txOptions);
extern uint16 macFrameDuration(uint16 len, uint16 txOptions);
extern bool macRxCheckPendingCallback(void);
extern uint8_t macCheckIndPacket(macRx_t  *pRxBuf);
extern bool macRxCheckMACPendingCallback(void);
extern uint8* macDataRxMemAlloc(uint16 len);
extern uint8 macDataRxMemFree(uint8 **pMsg);
extern void macRxCompleteCallback(macRx_t *pMsg);
extern uint16 macDataTxTimeAvailable(void);
extern uint8 macBuildDataFrame(macEvent_t *pEvent);
extern uint8 macCheckSched(void);
extern void macSetSched(macTx_t *pMsg);
extern void macDataTxComplete(macTx_t *pMsg);
extern void macTxCompleteCallback(uint8 status);
extern void macDataTxSend(void);
extern void macDataTxEnqueue(macTx_t *pMsg);
extern void macDataSend(macEvent_t *pEvent);
extern void macApiDataReq(macEvent_t *pEvent);
extern void macDataRxInd(macEvent_t *pEvent);
extern uint8 MAC_McpsDataReq(ApiMac_mcpsDataReq_t* pData);
extern macMcpsDataReq_t* MAC_McpsDataAlloc(uint16 len, uint8 securityLevel, uint8 keyIdMode, uint32 includeFhIEs, uint16 payloadIeLen);
extern void macAutoPendMaintainSrcMatchTable(macTx_t *pCurr);
extern void macAutoPendAddSrcMatchTableEntry(macTx_t *pMsg);

/* mac_device.c */
extern void macDeviceReset(void);
extern void macAssociateCnf(uint8 status, uint16 addr);
extern void macPollCnf(uint8 status, uint8 framePending);
extern uint8 macBuildAssociateReq(macEvent_t *pEvent);
extern void macStartResponseTimer(macEvent_t *pEvent);
extern void macStartFrameResponseTimer(macEvent_t *pEvent);
extern void macApiAssociateReq(macEvent_t *pEvent);
extern void macAssocDataReq(macEvent_t *pEvent);
extern void macAssocDataReqComplete(macEvent_t *pEvent);
extern void macRxAssocRsp(macEvent_t *pEvent);
extern void macAssocFrameResponseTimeout(macEvent_t *pEvent);
extern void macAssocFailed(macEvent_t *pEvent);
extern void macAssocRxDisassoc(macEvent_t *pEvent);
extern void macAssocDataRxInd(macEvent_t *pEvent);
extern void macApiPollReq(macEvent_t *pEvent);
extern void macPollDataReqComplete(macEvent_t *pEvent);
extern void macPollDataRxInd(macEvent_t *pEvent);
extern void macPollFrameResponseTimeout(macEvent_t *pEvent);
extern void macPollRxDisassoc(macEvent_t *pEvent);
extern void macPollRxAssocRsp(macEvent_t *pEvent);
extern void macRxCoordRealign(macEvent_t *pEvent);
extern void macPanConflictComplete(macEvent_t *pEvent);
extern void MAC_InitDevice(void);
extern uint8 MAC_MlmeAssociateReq(ApiMac_mlmeAssociateReq_t *pData);
extern uint8 MAC_MlmePollReq(ApiMac_mlmePollReq_t *pData);

/* mac_ie_build.c */
extern uint8 macBuildHeaderIes(macTx_t* pBuf, headerIeInfo_t *pHeaderIes );
extern uint16 macBuildPayloadCoexistIe( macTx_t* pBuf, bool ieIdOnly );
extern uint8 macBuildPayloadIes( macTx_t* pBuf, payloadIeInfo_t *pPayloadIeInfo, bool includeTermIe, bool ieIdOnly );

/* mac_ie_parse.c */
extern void macUpdateHeaderIEPtr(macRx_t *pMsg);
extern void macUpdatePayloadIEPtr(macRx_t *pMsg);
extern uint8 macParseHeaderIes(macRx_t *pMsg);
extern uint8 macParsePayloadIes( macRx_t *pMsg );
extern void macIeFreeList(macPayloadIeInfo_t* pList);
extern uint8* macIeCoexistIePresent( macPayloadIeInfo_t* pPayloadIe, uint8* pIePresent );
extern uint8* macMlmeSubIePresent( macPayloadIeInfo_t* pPayloadIe, uint8 mlmeSubIeType, uint8* pIePresent );

/* mac_main.c */
extern void macMainReset(void);
extern void macSetEvent(uint8 event);
extern void macSetEvent16(uint16 event);
extern void macCbackForEvent(macEvent_t *pEvent, uint8 status);
extern bool macStateIdle(void);
extern bool macStateIdleOrPolling(void);
extern bool macStateScanning(void);
extern void macExecute(macEvent_t *pEvent);
extern uint8 macSendMsg(uint8 event, void *pData);
extern uint8 macSendDataMsg(uint8 event, void *pData, ApiMac_sec_t *sec);
extern void macMainSetTaskId(uint8 taskId);
extern void macMainReserve( uint8 *ptr );
extern void mac_msg_deallocate(uint8 **msg_ptr);

/* mac_mgmt.c */
extern void macMgmtReset(void);
extern void macGetCoordAddress(sAddr_t *pAddr);
extern uint8 macGetMyAddrMode(void);
extern bool macDestAddrCmp(uint8 *p1, uint8 *p2);
extern bool macDestSAddrCmp(sAddr_t *pAddr, uint16 panId, uint8 *p);
extern bool macCoordAddrCmp(sAddr_t *pAddr);
extern bool macCoordDestAddrCmp(uint8 *p);
extern uint8 macBuildHeader(macTx_t *pBuf, uint8 srcAddrMode, sAddr_t *pDstAddr, uint16 dstPanId, uint8 elideSeqNo);
extern uint8 macBuildCommonReq(uint8 cmd, uint8 srcAddrMode, sAddr_t *pDstAddr, uint16 dstPanId, uint16 txOptions, ApiMac_sec_t *sec);
//extern uint8 macBuildEnhanceBeaconReq(macTx_t *pMsg, sAddr_t *pDstAddr, ApiMac_sec_t *sec, uint8 requestField, uint8 linkQuality, uint8 percentFilter);
//extern uint8 macBuildCoexistEBeaconReq(macTx_t *pMsg, sAddr_t *pDstAddr, ApiMac_sec_t *sec);
extern uint8 macBuildDisassociateReq(macEvent_t *pEvent);
//extern uint8 macBuildEBeaconNotifyInd(macMlmeBeaconNotifyInd_t *pNotifyInd, macEvent_t *pEvent);
extern void macBuildBeaconNotifyInd(macMlmeBeaconNotifyInd_t *pNotifyInd, macEvent_t *pEvent);
extern void macSecCpy (ApiMac_sec_t *pDst, ApiMac_sec_t *pSrc);
extern void macNoAction(macEvent_t *pEvent);
extern void macTxCsmaDelay(macEvent_t *pEvent);
extern void macBcTimerHandler(macEvent_t *pEvent);
extern void macApiUnsupported(macEvent_t *pEvent);
extern void macDefaultAction(macEvent_t *pEvent);
extern void macApiBadState(macEvent_t *pEvent);
extern void macApiPending(macEvent_t *pEvent);
extern void macCommStatusInd(macEvent_t *pEvent);
extern void macApiDisassociateReq(macEvent_t *pEvent);
extern void macDisassocComplete(macEvent_t *pEvent);
extern void macRxDisassoc(macEvent_t *pEvent);
extern void macRxBeacon(macEvent_t *pEvent);
extern void macConflictSyncLossInd(void);
extern uint8 MAC_MlmeDisassociateReq(ApiMac_mlmeDisassociateReq_t *pData);
extern void macUpdateCsmPhyParam(bool val);
extern bool macIsCsmOperational(void);
extern uint8 MAC_MlmeResetReq(bool setDefaultPib);
extern void MAC_ResumeReq(void);
extern uint8 MAC_YieldReq(void);
extern uint8 MAC_MlmeSyncReq(ApiMac_mlmeSyncReq_t *pData);
extern void macSetDefaultsByPhyID(void);
extern void macSetDefaultsByRE(void);
extern void MAC_Init(void);
extern void MAC_InitFH(void);
extern uint8 MAC_MlmeFHGetReqSize( uint16 pibAttribute );
extern uint8 MAC_MlmeFHGetReq(uint16 pibAttribute, void *pValue);
extern uint8 MAC_MlmeFHSetReq(uint16 pibAttribute, void *pValue);
extern uint8 MAC_StartFH(void);
extern uint8 MAC_EnableFH(void);
extern uint8 MAC_MlmeWSAsyncReq( ApiMac_mlmeWSAsyncReq_t* pData );
extern void macWSAsyncCnf(uint8 status);
extern uint8 macBuildWSAsyncMsg(macEvent_t *pEvent );
extern uint8 macBuildWSAsyncReq(macEvent_t *pEvent);
extern void macApiWSAsyncReq(macEvent_t *pEvent);
extern uint8 macGetCSMPhy( uint8 currPhyId );
extern uint8 MAC_RandomByte(void);
extern uint8 macCheckPhyRate(uint8 phyID);
extern uint8 macCheckPhyMode(uint8 phyID);

/* mac_pib.c */
//extern void MAC_MlmeSetActivePib( void* pPib );
extern void macPibReset(void);
extern uint8 macPibIndex(uint8 pibAttribute);
extern uint8 MAC_MlmeGetReq(uint8 pibAttribute, void *pValue);
extern uint8 MAC_MlmeGetReqSize( uint8 pibAttribute );
extern uint8 MAC_MlmeSetReq(uint8 pibAttribute, void *pValue);
extern uint8 MAC_GetPHYParamReq(uint8 phyAttribute, uint8 phyID, void *pValue);
extern uint8 MAC_SetPHYParamReq(uint8 phyAttribute, uint8 phyID, void *pValue);
extern macMRFSKPHYDesc_t* macMRFSKGetPhyDesc(uint8 phyID);
#if defined(COMBO_MAC) || defined(FREQ_2_4G)
extern macIEEEPHYDesc_t* macIEEEGetPhyDesc(uint8 phyID);
#endif
/* mac_pwr.c */
extern void macPwrReset(void);
extern void macApiPwrOnReq(macEvent_t *pEvent);
extern uint8 MAC_PwrOffReq(uint8 mode);
extern void MAC_PwrOnReq(void);
extern uint8 MAC_PwrMode(void);
extern void macPwrVote(bool pwrUp, bool bypassRAT);

/* mac_scan.c */
extern void macScanCnfInit(macMlmeScanCnf_t *pScanCnf, macEvent_t *pEvent);
extern void macApiScanReq(macEvent_t *pEvent);
extern void macScanNextChan(macEvent_t *pEvent);
extern void macScanStartTimer(macEvent_t *pEvent);
extern void macScanRxBeacon(macEvent_t *pEvent);
extern void macScanRxCoordRealign(macEvent_t *pEvent);
extern void macScanComplete(macEvent_t *pEvent);
extern void macScanFailedInProgress(macEvent_t *pEvent);
extern ApiMac_status_t MAC_MlmeScanReq(ApiMac_mlmeScanReq_t *pData);

/* mac_security_pib.c */
//extern void MAC_MlmeSetActiveSecurityPib( void* pSecPib);
extern void macSecurityPibReset(void);
extern uint8 macSecurityPibIndex(uint8 pibAttribute);
extern uint8 MAC_MlmeGetSecurityReq(uint8 pibAttribute, void *pValue);
extern uint8 MAC_MlmeGetPointerSecurityReq(uint8 pibAttribute, void **pValue);
extern uint8 MAC_MlmeGetSecurityReqSize( uint8 pibAttribute );
extern uint8 MAC_MlmeSetSecurityReq(uint8 pibAttribute, void *pValue);

/* mac_security.c */
extern uint8 macKeyDescriptorLookup( uint8 *pLookupData, uint8 lookupDataSize, keyDescriptor_t **ppKeyDescriptor );
extern uint8 macIncomingSecurityLevelChecking( uint8 securityLevel, uint8 frameType, uint8 cmdFrameId );
extern uint8 macDeviceDescriptorLookup( deviceDescriptor_t *deviceDescriptor, uint8 *deviceLookupData, uint8 deviceLookupSize );
extern uint8 macBlacklistChecking( keyDescriptor_t *pKeyDescriptor, uint8 *deviceLookupData, uint8 deviceLookupSize, deviceDescriptor_t **ppDeviceDescriptor, keyDeviceDescriptor_t **ppKeyDeviceDescriptor );
extern uint8 macIncomingFrameSecurityMaterialRetrieval( macRx_t *pMsg, keyDescriptor_t **ppKeyDescriptor, deviceDescriptor_t **ppDeviceDescriptor, keyDeviceDescriptor_t **ppKeyDeviceDescriptor);
extern uint8 macIncomingKeyUsagePolicyChecking( keyDescriptor_t *keyDescriptor, uint8 frameType, uint8 cmdFrameId );
extern uint8 macCcmStarInverseTransform( uint8    *pKey, uint32   frameCounter, uint8    securityLevel, uint8    *pAData, uint16   aDataLen, uint8    *pCData, uint16   cDataLen, uint8    *pExtAddr );
extern uint8 macIncomingFrameSecurity( macRx_t *pMsg );
extern keyDescriptor_t* macOutgoingFrameKeyDescRetrieval( ApiMac_sec_t  *pSec, sAddr_t  *pDstAddr, uint16   dstPanId );
extern uint8 macOutgoingFrameSecurity( macTx_t  *pBuf, sAddr_t  *pDstAddr, uint16   dstPanId, keyDescriptor_t **ppKeyDesc );
extern uint8 macCcmStarTransform( uint8    *pKey, uint32   frameCounter, uint8    securityLevel, uint8    *pAData, uint16   aDataLen, uint8    *pMData, uint16   mDataLen );
extern void macUpdatePanId(uint16 panId);
extern uint16_t macGetEUIIndex(sAddrExt_t *pEUI);
extern sAddrExt_t* macGetEUI(uint16_t euiIndex);

/* mac_timer.c */
extern void macTimerInit(void);
extern void macTimerUpdateSymbolTimer(void);
extern void macTimerGetNextTimer(void);
extern void macSymbolTimerTriggerCallback(void);
extern void macTimerRecalcUnaligned(int32 adjust, macTimer_t *pTimer);
extern void macSymbolTimerRolloverCallback(void);
extern bool macTimerCheckUnAlignedQ(macTimer_t *pTimer);
extern void macTimerAddTimer(macTimer_t *pTimer, macTimerHeader_t *pList);
extern void macTimerAligned(macTimer_t *pTimer, uint32 symbols);
extern void macTimer(macTimer_t *pTimer, uint32 symbols);
extern uint8 macTimerRemoveTimer(macTimer_t *pTimer, macTimerHeader_t *pList);
extern void macTimerCancel(macTimer_t * pTimer);
extern void macTimerUpdActive(macTimer_t * pTimer);
extern uint32 macTimerGetTime(void);
extern void macTimerStart(uint32 initTime, uint8 beaconOrder);
extern void macTimerRealign(macRx_t *pRxBeacon, uint8 beaconOrder);
extern void macTimerSyncRollover(uint8 beaconOrder);
extern void macTimerSetRollover(uint8 beaconOrder);
extern uint32 MAC_PwrNextTimeout(void);

/* macwrapper.c */
extern int8 macWrapper8ByteUnused(const uint8 *extaddr);
extern uint8 macWrapperAddDevice(ApiMac_secAddDevice_t *param);
extern uint8 macWrapperDeleteDevice(ApiMac_sAddrExt_t *param);
extern uint8 macWrapperDeleteKeyAndAssociatedDevices( uint16 keyIndex );
extern uint8 macWrapperDeleteAllDevices(void);
extern uint8 macWrapperGetDefaultSourceKey(uint8 keyid, uint32 *pFrameCounter);
extern uint8 macWrapperAddKeyInitFCtr( ApiMac_secAddKeyInitFrameCounter_t *param );

/* macstack.c */
extern void MAC_CbackEvent(macCbackEvent_t *pData);
extern uint8 MAC_CbackCheckPending(void);

/*Api_mac.c */
extern uint16_t convertTxOptions(ApiMac_txOptions_t txOptions);

/* main.c */
extern void assertHandler(void);

/* mac_hl_patch.c */
extern uint8 macPibCheckByPatch(uint8 pibAttribute, void *pValue);
extern void macSetDefaultsByPatch(uint8 pibAttribute);
extern uint8 macCcmEncrypt(AESCCM_Transaction *trans);
extern uint8 macCcmDecrypt(AESCCM_Transaction *trans);
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
 * GLOBAL VARIABLES
 */

// ROM Flash Jump Table

const uint32 HMAC_ROM_Flash_JT[] =
{
/* global variables */
   (uint32)&macBeaconCoordAction1,                       //ROM_HMAC_JT_OFFSET[0]
   (uint32)&macBeaconCoord,                              //ROM_HMAC_JT_OFFSET[1]
   (uint32)&macBeaconDeviceAction,                       //ROM_HMAC_JT_OFFSET[2]
   (uint32)&macBeaconDevice,                             //ROM_HMAC_JT_OFFSET[3]
#if defined (FEATURE_BEACON_MODE)
   (uint32)&macBeaconMargin,                             //ROM_HMAC_JT_OFFSET[4]
#else
   (uint32)NULL,
#endif
   (uint32)&macBeacon,                                   //ROM_HMAC_JT_OFFSET[5]
   (uint32)&macCoordAction1,                             //ROM_HMAC_JT_OFFSET[6]
   (uint32)&macCoordAction2,                             //ROM_HMAC_JT_OFFSET[7]
   (uint32)&macCoordAction3,                             //ROM_HMAC_JT_OFFSET[8]
   (uint32)&macCoord,                                    //ROM_HMAC_JT_OFFSET[9]
   (uint32)&macCmdIncludeFHIe,                           //ROM_HMAC_JT_OFFSET[10]
   (uint32)&macCmdBufSize,                               //ROM_HMAC_JT_OFFSET[11]
   (uint32)&macCmdLen,                                   //ROM_HMAC_JT_OFFSET[12]

   (uint32)&macTxFrameSuccess,                           //ROM_HMAC_JT_OFFSET[13]
   (uint32)&macTxFrameFailed,                            //ROM_HMAC_JT_OFFSET[14]
   (uint32)&macData,                                     //ROM_HMAC_JT_OFFSET[15]
   (uint32)&pMacDataTx,                                  //ROM_HMAC_JT_OFFSET[16]
   (uint32)&macDataTxIndirect,                           //ROM_HMAC_JT_OFFSET[17]
   (uint32)&macDataRequeueIndirect,                      //ROM_HMAC_JT_OFFSET[18]
   (uint32)&macDataRxBeaconCritical,                     //ROM_HMAC_JT_OFFSET[19]

   (uint32)&macDataTxBeaconComplete,                     //ROM_HMAC_JT_OFFSET[20]
   (uint32)&macDataSetSched,                             //ROM_HMAC_JT_OFFSET[21]
   (uint32)&macDataCheckSched,                           //ROM_HMAC_JT_OFFSET[22]

   (uint32)&macDataCheckTxTime,                          //ROM_HMAC_JT_OFFSET[23]
   (uint32)&macDataBeaconRequeue,                        //ROM_HMAC_JT_OFFSET[24]
   (uint32)&macDataTxDelay,                              //ROM_HMAC_JT_OFFSET[25]

   (uint32)&macDeviceAction1,                            //ROM_HMAC_JT_OFFSET[26]
   (uint32)&macDeviceAction2,                            //ROM_HMAC_JT_OFFSET[27]
   (uint32)&macDevice,                                   //ROM_HMAC_JT_OFFSET[28]

   (uint32)&macIdleSt,                                   //ROM_HMAC_JT_OFFSET[29]
   (uint32)&macAssociatingSt,                            //ROM_HMAC_JT_OFFSET[30]
   (uint32)&macPollingSt,                                //ROM_HMAC_JT_OFFSET[31]
#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
   (uint32)&macScanningSt,                               //ROM_HMAC_JT_OFFSET[32]
#else
   (uint32)NULL,
#endif
   (uint32)&macStartingSt,                               //ROM_HMAC_JT_OFFSET[33]
   (uint32)&macSendMsgBufSize,                           //ROM_HMAC_JT_OFFSET[34]

   (uint32)&macSendMsgDataSize,                          //ROM_HMAC_JT_OFFSET[35]
   (uint32)&macCbackEventTbl,                            //ROM_HMAC_JT_OFFSET[36]
   (uint32)&macTaskId,                                   //ROM_HMAC_JT_OFFSET[37]

   (uint32)&macMain,                                     //ROM_HMAC_JT_OFFSET[38]
   (uint32)&macStateTbl,                                 //ROM_HMAC_JT_OFFSET[39]
   (uint32)&macReset,                                    //ROM_HMAC_JT_OFFSET[40]

   (uint32)&macActionSet,                                //ROM_HMAC_JT_OFFSET[41]
   (uint32)&macMgmt,                                     //ROM_HMAC_JT_OFFSET[42]
   (uint32)&macPanCoordinator,                           //ROM_HMAC_JT_OFFSET[43]
   (uint32)&macMgmtAction1,                              //ROM_HMAC_JT_OFFSET[44]
   (uint32)&macMgmtAction2,                              //ROM_HMAC_JT_OFFSET[45]

  // (uint32)&freqHopEnabled,                              //ROM_HMAC_JT_OFFSET[46]
   (uint32)&fhObject,                                    //ROM_HMAC_JT_OFFSET[46]
   (uint32)&csmPhy,                                      //ROM_HMAC_JT_OFFSET[47]
   (uint32)&macPibDefaults,                              //ROM_HMAC_JT_OFFSET[48]
   (uint32)&macPibTbl,                                   //ROM_HMAC_JT_OFFSET[49]
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
   (uint32)&macMRFSKStdPhyTable,                         //ROM_HMAC_JT_OFFSET[50]
   (uint32)&macMRFSKGenPhyTable,                         //ROM_HMAC_JT_OFFSET[51]
#else
   (uint32)NULL,                                         //ROM_HMAC_JT_OFFSET[50]
   (uint32)NULL,                                         //ROM_HMAC_JT_OFFSET[51]
#endif
   (uint32)&macIEEEPhyTable,                             //ROM_HMAC_JT_OFFSET[52]
   (uint32)&macPib,                                      //ROM_HMAC_JT_OFFSET[53]

   (uint32)&timerUnalignedQ,                             //ROM_HMAC_JT_OFFSET[54]
   (uint32)&timerAlignedQ,                               //ROM_HMAC_JT_OFFSET[55]
   (uint32)&macTimerRollover,                            //ROM_HMAC_JT_OFFSET[56]
   (uint32)&macTimerNewRollover,                         //ROM_HMAC_JT_OFFSET[57]
   (uint32)&macTimerActive,                              //ROM_HMAC_JT_OFFSET[58]
   (uint32)&macTimerRolloverValue,                       //ROM_HMAC_JT_OFFSET[59]

   (uint32)&macScanAction,                               //ROM_HMAC_JT_OFFSET[60]
   (uint32)&macScan,                                     //ROM_HMAC_JT_OFFSET[61]
#if defined (FEATURE_MAC_SECURITY)
   (uint32)&macSecurityPib,                              //ROM_HMAC_JT_OFFSET[62]
   (uint32)&macSecurityPibTbl,                           //ROM_HMAC_JT_OFFSET[63]
#else
   (uint32)NULL,                                        //ROM_HMAC_JT_OFFSET[62]
   (uint32)NULL,                                        //ROM_HMAC_JT_OFFSET[63]
#endif
   (uint32)&macCfg,                                      //ROM_HMAC_JT_OFFSET[64]
#if defined (FEATURE_MAC_SECURITY)
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC26X2X7) && !defined(DeviceFamily_CC13X1) && !defined(DeviceFamily_CC26X1) && !defined(DeviceFamily_CC13X4) && !defined(DeviceFamily_CC26X4) && !defined(DeviceFamily_CC26X3)
   (uint32)&Crypto_handle,                               //ROM_HMAC_JT_OFFSET[65]
#else
   (uint32)&AESCCM_handle,                               //ROM_HMAC_JT_OFFSET[65]
#endif
   (uint32)&macCryptoDrvTblPtr,                             //ROM_HMAC_JT_OFFSET[66]

   (uint32)&macAuthTagLen,                               //ROM_HMAC_JT_OFFSET[67]
   (uint32)&macKeySourceLen,                             //ROM_HMAC_JT_OFFSET[68]
#else
   (uint32)NULL,                                        //ROM_HMAC_JT_OFFSET[65]
   (uint32)NULL,                                        //ROM_HMAC_JT_OFFSET[66]

   (uint32)NULL,                                        //ROM_HMAC_JT_OFFSET[67]
   (uint32)NULL,                                        //ROM_HMAC_JT_OFFSET[68]
#endif
#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
   (uint32)&macBeaconPayload,                            //ROM_HMAC_JT_OFFSET[69]
#else
   (uint32)NULL,
#endif
   (uint32)&enhancedAckBuf,                              //ROM_HMAC_JT_OFFSET[70]
   (uint32)&macTasksEvents,                                 //ROM_HMAC_JT_OFFSET[71]

/* mac_beacon_coord.c */
   (uint32)&macBeaconCoordReset,                         //ROM_HMAC_JT_OFFSET[0]
   (uint32)&MAC_InitBeaconCoord,                         //ROM_HMAC_JT_OFFSET[1]
   (uint32)&macBeaconSetPrepareTime,                     //ROM_HMAC_JT_OFFSET[2]
   (uint32)&macBeaconCheckStartTime,                     //ROM_HMAC_JT_OFFSET[3]
   (uint32)&macApiBeaconStartReq,                        //ROM_HMAC_JT_OFFSET[4]
   (uint32)&macBeaconPeriodCallback,                     //ROM_HMAC_JT_OFFSET[5]
   (uint32)&macBeaconTxCallback,                         //ROM_HMAC_JT_OFFSET[6]
//   (uint32)&macEBeaconTxCallback,                        //ROM_HMAC_JT_OFFSET[7]
//   (uint32)&macEBeaconPrepareCallback,                   //ROM_HMAC_JT_OFFSET[8]
   (uint32)&macBeaconPrepareCallback,                    //ROM_HMAC_JT_OFFSET[7]
   (uint32)&macStartBeaconPrepareCallback,               //ROM_HMAC_JT_OFFSET[8]
   (uint32)&macBeaconStartContinue,                      //ROM_HMAC_JT_OFFSET[9]
   (uint32)&macBeaconBattLifeCallback,                   //ROM_HMAC_JT_OFFSET[10]
   (uint32)&macTxBeaconCompleteCallback,                 //ROM_HMAC_JT_OFFSET[11]
   (uint32)&macBeaconSetupBroadcast,                     //ROM_HMAC_JT_OFFSET[12]
   (uint32)&macOutgoingNonSlottedTx,                     //ROM_HMAC_JT_OFFSET[13]
   (uint32)&macBeaconSchedRequested,                     //ROM_HMAC_JT_OFFSET[14]
   (uint32)&macBeaconClearIndirect,                      //ROM_HMAC_JT_OFFSET[15]

/* mac_beacon_device.c */
   (uint32)&macBroadcastPendCallback,                    //ROM_HMAC_JT_OFFSET[16]
   (uint32)&macBeaconStopTrack,                          //ROM_HMAC_JT_OFFSET[17]
   (uint32)&macIncomingNonSlottedTx,                     //ROM_HMAC_JT_OFFSET[18]
   (uint32)&macBeaconSyncLoss,                           //ROM_HMAC_JT_OFFSET[19]
   (uint32)&macSyncTimeoutCallback,                      //ROM_HMAC_JT_OFFSET[20]
   (uint32)&macTrackTimeoutCallback,                     //ROM_HMAC_JT_OFFSET[21]
   (uint32)&macTrackStartCallback,                       //ROM_HMAC_JT_OFFSET[22]
   (uint32)&macTrackPeriodCallback,                      //ROM_HMAC_JT_OFFSET[23]
#if defined (FEATURE_BEACON_MODE)
   (uint32)&macRxBeaconCritical,                         //ROM_HMAC_JT_OFFSET[24]
#else
   (uint32)NULL,
#endif
   (uint32)&MAC_InitBeaconDevice,                        //ROM_HMAC_JT_OFFSET[25]
   (uint32)&macCheckPendAddr,                            //ROM_HMAC_JT_OFFSET[26]
   (uint32)&macAutoPoll,                                 //ROM_HMAC_JT_OFFSET[27]
   (uint32)&macApiSyncReq,                               //ROM_HMAC_JT_OFFSET[28]
   (uint32)&macBeaconStartFrameResponseTimer,            //ROM_HMAC_JT_OFFSET[29]
   (uint32)&macStartBroadcastPendTimer,                  //ROM_HMAC_JT_OFFSET[30]

/* mac_beacon.c */
   (uint32)&macBeaconReset,                              //ROM_HMAC_JT_OFFSET[31]
   (uint32)&macBeaconInit,                               //ROM_HMAC_JT_OFFSET[32]
   (uint32)&macBeaconSetSched,                           //ROM_HMAC_JT_OFFSET[33]
   (uint32)&macBeaconCheckSched,                         //ROM_HMAC_JT_OFFSET[34]
   (uint32)&macBeaconRequeue,                            //ROM_HMAC_JT_OFFSET[35]
   (uint32)&macBeaconCheckTxTime,                        //ROM_HMAC_JT_OFFSET[36]
   (uint32)&macBeaconSetupCap,                           //ROM_HMAC_JT_OFFSET[37]

/* mac_coord.c */
   (uint32)&macCoordReset,                               //ROM_HMAC_JT_OFFSET[38]
   (uint32)&macBuildAssociateRsp,                        //ROM_HMAC_JT_OFFSET[39]
   (uint32)&macBuildRealign,                             //ROM_HMAC_JT_OFFSET[40]
   (uint32)&macPendAddrLen,                              //ROM_HMAC_JT_OFFSET[41]
   (uint32)&macBuildPendAddr,                            //ROM_HMAC_JT_OFFSET[42]
//   (uint32)&macBuildEnhancedBeacon,                      //ROM_HMAC_JT_OFFSET[46]
   (uint32)&macBuildBeacon,                              //ROM_HMAC_JT_OFFSET[43]
   (uint32)&macIndirectMark,                             //ROM_HMAC_JT_OFFSET[44]
   (uint32)&macIndirectSend,                             //ROM_HMAC_JT_OFFSET[45]
   (uint32)&macIndirectExpire,                           //ROM_HMAC_JT_OFFSET[46]
   (uint32)&macIndirectTxFrame,                          //ROM_HMAC_JT_OFFSET[47]
   (uint32)&macIndirectRequeueFrame,                     //ROM_HMAC_JT_OFFSET[48]
   (uint32)&macRxAssociateReq,                           //ROM_HMAC_JT_OFFSET[49]
   (uint32)&macApiAssociateRsp,                          //ROM_HMAC_JT_OFFSET[50]
   (uint32)&macRxOrphan,                                 //ROM_HMAC_JT_OFFSET[51]
   (uint32)&macApiOrphanRsp,                             //ROM_HMAC_JT_OFFSET[52]
   (uint32)&macApiPurgeReq,                              //ROM_HMAC_JT_OFFSET[53]
   (uint32)&macApiStartReq,                              //ROM_HMAC_JT_OFFSET[54]
   (uint32)&macStartBegin,                               //ROM_HMAC_JT_OFFSET[55]
   (uint32)&macStartSetParams,                           //ROM_HMAC_JT_OFFSET[56]
   (uint32)&macStartContinue,                            //ROM_HMAC_JT_OFFSET[57]
   (uint32)&macStartComplete,                            //ROM_HMAC_JT_OFFSET[58]
   (uint32)&macRxBeaconReq,                              //ROM_HMAC_JT_OFFSET[59]
//   (uint32)&macProcessEBeaconFilterIe,                   //ROM_HMAC_JT_OFFSET[64]
//   (uint32)&macRxEnhancedBeaconReq,                      //ROM_HMAC_JT_OFFSET[65]
   (uint32)&macRxDataReq,                                //ROM_HMAC_JT_OFFSET[60]
   (uint32)&macRxPanConflict,                            //ROM_HMAC_JT_OFFSET[61]
   (uint32)&MAC_InitCoord,                               //ROM_HMAC_JT_OFFSET[62]
   (uint32)&MAC_MlmeAssociateRsp,                        //ROM_HMAC_JT_OFFSET[63]
   (uint32)&MAC_MlmeOrphanRsp,                           //ROM_HMAC_JT_OFFSET[64]
   (uint32)&MAC_McpsPurgeReq,                            //ROM_HMAC_JT_OFFSET[65]
   (uint32)&MAC_MlmeStartReq,                            //ROM_HMAC_JT_OFFSET[66]

/* mac_data.c */
   (uint32)&macDataTxDelayCallback,                      //ROM_HMAC_JT_OFFSET[67]
   (uint32)&macDataReset,                                //ROM_HMAC_JT_OFFSET[68]
   (uint32)&macAllocTxBuffer,                            //ROM_HMAC_JT_OFFSET[69]
   (uint32)&macFrameDurationSubG,                        //ROM_HMAC_JT_OFFSET[70]
   (uint32)&macFrameDurationIeee,                        //ROM_HMAC_JT_OFFSET[71]
   (uint32)&macFrameDuration,                            //ROM_HMAC_JT_OFFSET[72]
   (uint32)&macRxCheckPendingCallback,                   //ROM_HMAC_JT_OFFSET[73]
   (uint32)&macCheckIndPacket,                           //ROM_HMAC_JT_OFFSET[74]
   (uint32)&macRxCheckMACPendingCallback,                //ROM_HMAC_JT_OFFSET[75]
   (uint32)&macDataRxMemAlloc,                           //ROM_HMAC_JT_OFFSET[76]
   (uint32)&macDataRxMemFree,                            //ROM_HMAC_JT_OFFSET[77]
   (uint32)&macRxCompleteCallback,                       //ROM_HMAC_JT_OFFSET[78]
   (uint32)&macDataTxTimeAvailable,                      //ROM_HMAC_JT_OFFSET[79]
   (uint32)&macBuildDataFrame,                           //ROM_HMAC_JT_OFFSET[80]
   (uint32)&macCheckSched,                               //ROM_HMAC_JT_OFFSET[81]
   (uint32)&macSetSched,                                 //ROM_HMAC_JT_OFFSET[82]
   (uint32)&macDataTxComplete,                           //ROM_HMAC_JT_OFFSET[83]
   (uint32)&macTxCompleteCallback,                       //ROM_HMAC_JT_OFFSET[84]
   (uint32)&macDataTxSend,                               //ROM_HMAC_JT_OFFSET[85]
   (uint32)&macDataTxEnqueue,                            //ROM_HMAC_JT_OFFSET[86]
   (uint32)&macDataSend,                                 //ROM_HMAC_JT_OFFSET[87]
   (uint32)&macApiDataReq,                               //ROM_HMAC_JT_OFFSET[88]
   (uint32)&macDataRxInd,                                //ROM_HMAC_JT_OFFSET[89]
   (uint32)&MAC_McpsDataReq,                             //ROM_HMAC_JT_OFFSET[90]
   (uint32)&MAC_McpsDataAlloc,                           //ROM_HMAC_JT_OFFSET[91]
   (uint32)&macAutoPendMaintainSrcMatchTable,            //ROM_HMAC_JT_OFFSET[92]
   (uint32)&macAutoPendAddSrcMatchTableEntry,            //ROM_HMAC_JT_OFFSET[93]

/* mac_device.c */
   (uint32)&macDeviceReset,                              //ROM_HMAC_JT_OFFSET[94]
   (uint32)&macAssociateCnf,                             //ROM_HMAC_JT_OFFSET[95]
   (uint32)&macPollCnf,                                  //ROM_HMAC_JT_OFFSET[96]
   (uint32)&macBuildAssociateReq,                        //ROM_HMAC_JT_OFFSET[97]
   (uint32)&macStartResponseTimer,                       //ROM_HMAC_JT_OFFSET[98]
   (uint32)&macStartFrameResponseTimer,                  //ROM_HMAC_JT_OFFSET[99]
   (uint32)&macApiAssociateReq,                          //ROM_HMAC_JT_OFFSET[100]
   (uint32)&macAssocDataReq,                             //ROM_HMAC_JT_OFFSET[101]
   (uint32)&macAssocDataReqComplete,                     //ROM_HMAC_JT_OFFSET[102]
   (uint32)&macRxAssocRsp,                               //ROM_HMAC_JT_OFFSET[103]
   (uint32)&macAssocFrameResponseTimeout,                //ROM_HMAC_JT_OFFSET[104]
   (uint32)&macAssocFailed,                              //ROM_HMAC_JT_OFFSET[105]
   (uint32)&macAssocRxDisassoc,                          //ROM_HMAC_JT_OFFSET[106]
   (uint32)&macAssocDataRxInd,                           //ROM_HMAC_JT_OFFSET[107]
   (uint32)&macApiPollReq,                               //ROM_HMAC_JT_OFFSET[108]
   (uint32)&macPollDataReqComplete,                      //ROM_HMAC_JT_OFFSET[109]
   (uint32)&macPollDataRxInd,                            //ROM_HMAC_JT_OFFSET[110]
   (uint32)&macPollFrameResponseTimeout,                 //ROM_HMAC_JT_OFFSET[111]
   (uint32)&macPollRxDisassoc,                           //ROM_HMAC_JT_OFFSET[112]
   (uint32)&macPollRxAssocRsp,                           //ROM_HMAC_JT_OFFSET[113]
   (uint32)&macRxCoordRealign,                           //ROM_HMAC_JT_OFFSET[114]
   (uint32)&macPanConflictComplete,                      //ROM_HMAC_JT_OFFSET[115]
   (uint32)&MAC_InitDevice,                              //ROM_HMAC_JT_OFFSET[116]
   (uint32)&MAC_MlmeAssociateReq,                        //ROM_HMAC_JT_OFFSET[117]
   (uint32)&MAC_MlmePollReq,                             //ROM_HMAC_JT_OFFSET[118]
/* mac_ie_build.c */
   (uint32)&macBuildHeaderIes,                           //ROM_HMAC_JT_OFFSET[129]
   (uint32)&macBuildPayloadCoexistIe,                    //ROM_HMAC_JT_OFFSET[120]
   (uint32)&macBuildPayloadIes,                          //ROM_HMAC_JT_OFFSET[121]

/* mac_ie_parse.c */
   (uint32)&macUpdateHeaderIEPtr,                        //ROM_HMAC_JT_OFFSET[122]
   (uint32)&macUpdatePayloadIEPtr,                       //ROM_HMAC_JT_OFFSET[123]
   (uint32)&macParseHeaderIes,                           //ROM_HMAC_JT_OFFSET[124]
   (uint32)&macParsePayloadIes,                          //ROM_HMAC_JT_OFFSET[125]
   (uint32)&macIeFreeList,                               //ROM_HMAC_JT_OFFSET[126]
   (uint32)&macIeCoexistIePresent,                       //ROM_HMAC_JT_OFFSET[127]
   (uint32)&macMlmeSubIePresent,                         //ROM_HMAC_JT_OFFSET[128]

/* mac_main.c */
   (uint32)&macMainReset,                                //ROM_HMAC_JT_OFFSET[129]
   (uint32)&macSetEvent,                                 //ROM_HMAC_JT_OFFSET[130]
   (uint32)&macSetEvent16,                               //ROM_HMAC_JT_OFFSET[131]
   (uint32)&macCbackForEvent,                            //ROM_HMAC_JT_OFFSET[132]
   (uint32)&macStateIdle,                                //ROM_HMAC_JT_OFFSET[133]
   (uint32)&macStateIdleOrPolling,                       //ROM_HMAC_JT_OFFSET[134]
   (uint32)&macStateScanning,                            //ROM_HMAC_JT_OFFSET[135]
   (uint32)&macExecute,                                  //ROM_HMAC_JT_OFFSET[136]
   (uint32)&macSendMsg,                                  //ROM_HMAC_JT_OFFSET[137]
   (uint32)&macSendDataMsg,                              //ROM_HMAC_JT_OFFSET[138]
   (uint32)&macMainSetTaskId,                                 //ROM_HMAC_JT_OFFSET[139]
   (uint32)&macMainReserve,                              //ROM_HMAC_JT_OFFSET[140]
   (uint32)&mac_msg_deallocate,                          //ROM_HMAC_JT_OFFSET[141]

/* mac_mgmt.c */
   (uint32)&macMgmtReset,                                //ROM_HMAC_JT_OFFSET[142]
   (uint32)&macGetCoordAddress,                          //ROM_HMAC_JT_OFFSET[143]
   (uint32)&macGetMyAddrMode,                            //ROM_HMAC_JT_OFFSET[144]
   (uint32)&macDestAddrCmp,                              //ROM_HMAC_JT_OFFSET[145]
   (uint32)&macDestSAddrCmp,                             //ROM_HMAC_JT_OFFSET[146]
   (uint32)&macCoordAddrCmp,                             //ROM_HMAC_JT_OFFSET[147]
   (uint32)&macCoordDestAddrCmp,                         //ROM_HMAC_JT_OFFSET[148]
   (uint32)&macBuildHeader,                              //ROM_HMAC_JT_OFFSET[149]
   (uint32)&macBuildCommonReq,                           //ROM_HMAC_JT_OFFSET[150]
//   (uint32)&macBuildEnhanceBeaconReq,                    //ROM_HMAC_JT_OFFSET[158]
//   (uint32)&macBuildCoexistEBeaconReq,                   //ROM_HMAC_JT_OFFSET[159]
   (uint32)&macBuildDisassociateReq,                     //ROM_HMAC_JT_OFFSET[151]
//   (uint32)&macBuildEBeaconNotifyInd,                    //ROM_HMAC_JT_OFFSET[161]
   (uint32)&macBuildBeaconNotifyInd,                     //ROM_HMAC_JT_OFFSET[152]
   (uint32)&macSecCpy,                                   //ROM_HMAC_JT_OFFSET[153]
   (uint32)&macNoAction,                                 //ROM_HMAC_JT_OFFSET[154]
   (uint32)&macTxCsmaDelay,                              //ROM_HMAC_JT_OFFSET[155]
   (uint32)&macBcTimerHandler,                           //ROM_HMAC_JT_OFFSET[156]
   (uint32)&macApiUnsupported,                           //ROM_HMAC_JT_OFFSET[157]
   (uint32)&macDefaultAction,                            //ROM_HMAC_JT_OFFSET[158]
   (uint32)&macApiBadState,                              //ROM_HMAC_JT_OFFSET[159]
   (uint32)&macApiPending,                               //ROM_HMAC_JT_OFFSET[160]
   (uint32)&macCommStatusInd,                            //ROM_HMAC_JT_OFFSET[161]
   (uint32)&macApiDisassociateReq,                       //ROM_HMAC_JT_OFFSET[162]
   (uint32)&macDisassocComplete,                         //ROM_HMAC_JT_OFFSET[163]
   (uint32)&macRxDisassoc,                               //ROM_HMAC_JT_OFFSET[164]
   (uint32)&macRxBeacon,                                 //ROM_HMAC_JT_OFFSET[165]
   (uint32)&macConflictSyncLossInd,                      //ROM_HMAC_JT_OFFSET[166]
   (uint32)&MAC_MlmeDisassociateReq,                     //ROM_HMAC_JT_OFFSET[167]
   (uint32)&macUpdateCsmPhyParam,                        //ROM_HMAC_JT_OFFSET[168]
   (uint32)&macIsCsmOperational,                         //ROM_HMAC_JT_OFFSET[169]
   (uint32)&MAC_MlmeResetReq,                            //ROM_HMAC_JT_OFFSET[170]
   (uint32)&MAC_ResumeReq,                               //ROM_HMAC_JT_OFFSET[171]
   (uint32)&MAC_YieldReq,                                //ROM_HMAC_JT_OFFSET[172]
   (uint32)&MAC_MlmeSyncReq,                             //ROM_HMAC_JT_OFFSET[173]
   (uint32)&macSetDefaultsByPhyID,                       //ROM_HMAC_JT_OFFSET[174]
   (uint32)&macSetDefaultsByRE,                          //ROM_HMAC_JT_OFFSET[175]
   (uint32)&MAC_Init,                                    //ROM_HMAC_JT_OFFSET[176]
   (uint32)&MAC_InitFH,                                  //ROM_HMAC_JT_OFFSET[177]
   (uint32)&MAC_MlmeFHGetReqSize,                        //ROM_HMAC_JT_OFFSET[178]
   (uint32)&MAC_MlmeFHGetReq,                            //ROM_HMAC_JT_OFFSET[179]
   (uint32)&MAC_MlmeFHSetReq,                            //ROM_HMAC_JT_OFFSET[180]
   (uint32)&MAC_StartFH,                                 //ROM_HMAC_JT_OFFSET[181]
   (uint32)&MAC_EnableFH,                                //ROM_HMAC_JT_OFFSET[182]
   (uint32)&MAC_MlmeWSAsyncReq,                          //ROM_HMAC_JT_OFFSET[183]
   (uint32)&macWSAsyncCnf,                               //ROM_HMAC_JT_OFFSET[184]
   (uint32)&macBuildWSAsyncMsg,                          //ROM_HMAC_JT_OFFSET[185]
   (uint32)&macBuildWSAsyncReq,                          //ROM_HMAC_JT_OFFSET[186]
   (uint32)&macApiWSAsyncReq,                            //ROM_HMAC_JT_OFFSET[187]
   (uint32)&macGetCSMPhy,                                //ROM_HMAC_JT_OFFSET[188]
   (uint32)&MAC_RandomByte,                              //ROM_HMAC_JT_OFFSET[189]
   (uint32)&macCheckPhyRate,                             //ROM_HMAC_JT_OFFSET[190]
   (uint32)&macCheckPhyMode,                             //ROM_HMAC_JT_OFFSET[191]

/* mac_pib.c */
//   (uint32)&MAC_MlmeSetActivePib,                        //ROM_HMAC_JT_OFFSET[202]
   (uint32)&macPibReset,                                 //ROM_HMAC_JT_OFFSET[192]
   (uint32)&macPibIndex,                                 //ROM_HMAC_JT_OFFSET[193]
   (uint32)&MAC_MlmeGetReq,                              //ROM_HMAC_JT_OFFSET[194]
   (uint32)&MAC_MlmeGetReqSize,                          //ROM_HMAC_JT_OFFSET[195]
   (uint32)&MAC_MlmeSetReq,                              //ROM_HMAC_JT_OFFSET[196]
   (uint32)&MAC_GetPHYParamReq,                          //ROM_HMAC_JT_OFFSET[197]
   (uint32)&MAC_SetPHYParamReq,                          //ROM_HMAC_JT_OFFSET[198]
   (uint32)&macMRFSKGetPhyDesc,                          //ROM_HMAC_JT_OFFSET[199]
   (uint32)&macIEEEGetPhyDesc,                           //ROM_HMAC_JT_OFFSET[200]

/* mac_pwr.c */
   (uint32)&macPwrReset,                                 //ROM_HMAC_JT_OFFSET[201]
   (uint32)&macApiPwrOnReq,                              //ROM_HMAC_JT_OFFSET[202]
   (uint32)&MAC_PwrOffReq,                               //ROM_HMAC_JT_OFFSET[203]
   (uint32)&MAC_PwrOnReq,                                //ROM_HMAC_JT_OFFSET[204]
   (uint32)&MAC_PwrMode,                                 //ROM_HMAC_JT_OFFSET[205]
   (uint32)&macPwrVote,                                  //ROM_HMAC_JT_OFFSET[206]

/* mac_scan.c */
   (uint32)&macScanCnfInit,                              //ROM_HMAC_JT_OFFSET[207]
   (uint32)&macApiScanReq,                               //ROM_HMAC_JT_OFFSET[208]
   (uint32)&macScanNextChan,                             //ROM_HMAC_JT_OFFSET[209]
   (uint32)&macScanStartTimer,                           //ROM_HMAC_JT_OFFSET[210]
   (uint32)&macScanRxBeacon,                             //ROM_HMAC_JT_OFFSET[211]
   (uint32)&macScanRxCoordRealign,                       //ROM_HMAC_JT_OFFSET[212]
   (uint32)&macScanComplete,                             //ROM_HMAC_JT_OFFSET[213]
   (uint32)&macScanFailedInProgress,                     //ROM_HMAC_JT_OFFSET[214]
   (uint32)&MAC_MlmeScanReq,                             //ROM_HMAC_JT_OFFSET[215]

/* mac_security_pib.c */
//   (uint32)&MAC_MlmeSetActiveSecurityPib,                //ROM_HMAC_JT_OFFSET[227]
   (uint32)&macSecurityPibReset,                         //ROM_HMAC_JT_OFFSET[216]
   (uint32)&macSecurityPibIndex,                         //ROM_HMAC_JT_OFFSET[217]
   (uint32)&MAC_MlmeGetSecurityReq,                      //ROM_HMAC_JT_OFFSET[218]
   (uint32)&MAC_MlmeGetPointerSecurityReq,               //ROM_HMAC_JT_OFFSET[219]
   (uint32)&MAC_MlmeGetSecurityReqSize,                  //ROM_HMAC_JT_OFFSET[220]
   (uint32)&MAC_MlmeSetSecurityReq,                      //ROM_HMAC_JT_OFFSET[221]

/* mac_security.c */
   (uint32)&macKeyDescriptorLookup,                      //ROM_HMAC_JT_OFFSET[222]
   (uint32)&macIncomingSecurityLevelChecking,            //ROM_HMAC_JT_OFFSET[223]
   (uint32)&macDeviceDescriptorLookup,                   //ROM_HMAC_JT_OFFSET[224]
   (uint32)&macBlacklistChecking,                        //ROM_HMAC_JT_OFFSET[225]
   (uint32)&macIncomingFrameSecurityMaterialRetrieval,   //ROM_HMAC_JT_OFFSET[226]
   (uint32)&macIncomingKeyUsagePolicyChecking,           //ROM_HMAC_JT_OFFSET[227]
   (uint32)&macCcmStarInverseTransform,                  //ROM_HMAC_JT_OFFSET[228]
   (uint32)&macIncomingFrameSecurity,                    //ROM_HMAC_JT_OFFSET[229]
   (uint32)&macOutgoingFrameKeyDescRetrieval,            //ROM_HMAC_JT_OFFSET[230]
   (uint32)&macOutgoingFrameSecurity,                    //ROM_HMAC_JT_OFFSET[231]
   (uint32)&macCcmStarTransform,                         //ROM_HMAC_JT_OFFSET[232]
   (uint32)&macUpdatePanId,                              //ROM_HMAC_JT_OFFSET[233]
   (uint32)&macGetEUIIndex,                              //ROM_HMAC_JT_OFFSET[234]
   (uint32)&macGetEUI,                                   //ROM_HMAC_JT_OFFSET[235]

/* mac_timer.c */
   (uint32)&macTimerInit,                                //ROM_HMAC_JT_OFFSET[236]
   (uint32)&macTimerUpdateSymbolTimer,                   //ROM_HMAC_JT_OFFSET[237]
   (uint32)&macTimerGetNextTimer,                        //ROM_HMAC_JT_OFFSET[238]
   (uint32)&macSymbolTimerTriggerCallback,               //ROM_HMAC_JT_OFFSET[239]
   (uint32)&macTimerRecalcUnaligned,                     //ROM_HMAC_JT_OFFSET[240]
   (uint32)&macSymbolTimerRolloverCallback,              //ROM_HMAC_JT_OFFSET[241]
   (uint32)&macTimerCheckUnAlignedQ,                     //ROM_HMAC_JT_OFFSET[242]
   (uint32)&macTimerAddTimer,                            //ROM_HMAC_JT_OFFSET[243]
   (uint32)&macTimerAligned,                             //ROM_HMAC_JT_OFFSET[244]
   (uint32)&macTimer,                                    //ROM_HMAC_JT_OFFSET[245]
   (uint32)&macTimerRemoveTimer,                         //ROM_HMAC_JT_OFFSET[246]
   (uint32)&macTimerCancel,                              //ROM_HMAC_JT_OFFSET[247]
   (uint32)&macTimerUpdActive,                           //ROM_HMAC_JT_OFFSET[248]
   (uint32)&macTimerGetTime,                             //ROM_HMAC_JT_OFFSET[249]
   (uint32)&macTimerStart,                               //ROM_HMAC_JT_OFFSET[250]
   (uint32)&macTimerRealign,                             //ROM_HMAC_JT_OFFSET[251]
   (uint32)&macTimerSyncRollover,                        //ROM_HMAC_JT_OFFSET[252]
   (uint32)&macTimerSetRollover,                         //ROM_HMAC_JT_OFFSET[253]
   (uint32)&MAC_PwrNextTimeout,                          //ROM_HMAC_JT_OFFSET[254]

/* macwrapper.c */
   (uint32)&macWrapper8ByteUnused,                       //ROM_HMAC_JT_OFFSET[255]
   (uint32)&macWrapperAddDevice,                         //ROM_HMAC_JT_OFFSET[256]
   (uint32)&macWrapperDeleteDevice,                      //ROM_HMAC_JT_OFFSET[257]
   (uint32)&macWrapperDeleteKeyAndAssociatedDevices,     //ROM_HMAC_JT_OFFSET[258]
   (uint32)&macWrapperDeleteAllDevices,                  //ROM_HMAC_JT_OFFSET[259]
   (uint32)&macWrapperGetDefaultSourceKey,               //ROM_HMAC_JT_OFFSET[260]
   (uint32)&macWrapperAddKeyInitFCtr,                    //ROM_HMAC_JT_OFFSET[261]

/* macstack.c */
   (uint32)&MAC_CbackEvent,                              //ROM_HMAC_JT_OFFSET[262]
   (uint32)&MAC_CbackQueryRetransmit,                    //ROM_HMAC_JT_OFFSET[263]

/*Api_mac.c */
   (uint32)&ApiMac_convertCapabilityInfo,                //ROM_HMAC_JT_OFFSET[264]
   (uint32)&copyApiMacAddrToMacAddr,                     //ROM_HMAC_JT_OFFSET[265]
   (uint32)&convertTxOptions,                            //ROM_HMAC_JT_OFFSET[266]

/*Assert */
   (uint32)&assertHandler,                               //ROM_HMAC_JT_OFFSET[267]
   (uint32)&macMcuLongDiv,                               //ROM_HMAC_JT_OFFSET[268]

   (uint32)&MAC_CbackCheckPending,                       //ROM_HMAC_JT_OFFSET[269]

/* mac_hl_patch.c */
   (uint32)&macPibCheckByPatch,                          //ROM_HMAC_JT_OFFSET[270]
   (uint32)&macSetDefaultsByPatch,                       //ROM_HMAC_JT_OFFSET[271]
   (uint32)&macCcmEncrypt,                               //ROM_HMAC_JT_OFFSET[272]
   (uint32)&macCcmDecrypt,                               //ROM_HMAC_JT_OFFSET[273]
};



void HMAC_ROM_Init(void)
{
    /* assign the FH ROM JT table */
    RAM_MAC_BASE_ADDR[ROM_RAM_HMAC_TABLE_INDEX] = (uint32)(HMAC_ROM_Flash_JT);
}

