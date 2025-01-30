/******************************************************************************

 @file  rom_image_linker.cmd

 @brief Linker configuration file for ROM image linker. This command will
        put all functions in ROM area

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

/*
******************************************************************************
    FH module ROM functions
******************************************************************************
*/
/* fh_api.c */
--undef_sym=FHAPI_reset
--undef_sym=FHAPI_start
--undef_sym=FHAPI_startBS
--undef_sym=FHAPI_completeTxCb
--undef_sym=FHAPI_completeRxCb
--undef_sym=FHAPI_sendData
--undef_sym=FHAPI_getTxParams_old
--undef_sym=FHAPI_getTxParams
--undef_sym=FHAPI_getTxChHoppingFunc
--undef_sym=FHAPI_getRemDT
--undef_sym=FHAPI_stopAsync
--undef_sym=FHAPI_setStateCb
--undef_sym=FHAPI_SFDRxCb
--undef_sym=FHAPI_getBitCount

/* fh_data.c */
--undef_sym=FHDATA_checkPktFromQueue
--undef_sym=FHDATA_getPktFromQueue
--undef_sym=FHDATA_purgePktFromQueue
--undef_sym=FHDATA_purgeUnSupportedFrameTypes
--undef_sym=FHDATA_chkASYNCStartReq
--undef_sym=FHDATA_procASYNCReq
--undef_sym=FHDATA_startEDFEReq
--undef_sym=FHDATA_procEDFEReq
--undef_sym=FHDATA_startEDFETimer
--undef_sym=FHDATA_requeuePkt
--undef_sym=FHDATA_updateCCA
--undef_sym=FHDATA_CheckMinTxOffTime
--undef_sym=FHDATA_getASYNCDelay
--undef_sym=FHDATA_asyncMinTxOffTimerCb

/* fh_dhicf.c */
--undef_sym=FHDH1CF_hashword
--undef_sym=FHDH1CF_getCh
--undef_sym=FHDH1CF_getBcCh
--undef_sym=FHDH1CF_mapChIdxChannel
--undef_sym=FHDH1CF_getBCChannelNum
--undef_sym=FHDH1CF_getChannelNum
--undef_sym=FHDH1CF_getTxChannelNum

/* fh_ie.c */
--undef_sym=FHIE_getCcaSfdTime
--undef_sym=FHIE_genUTIE
--undef_sym=FHIE_genBTIE
--undef_sym=FHIE_getChannelPlan
--undef_sym=FHIE_isChannelPlanValid
--undef_sym=FHIE_genExcludedChannelMask
--undef_sym=FHIE_genExcludedChannelRanges
--undef_sym=FHIE_getExcludedChannelControl
--undef_sym=FHIE_genCommonInformation
--undef_sym=FHIE_genPieContent
--undef_sym=FHIE_genPie
--undef_sym=FHIE_genHie
--undef_sym=FHIE_getCommonInformationLen
--undef_sym=FHIE_getPieContentLen
--undef_sym=FHIE_getPieLen
--undef_sym=FHIE_getHieLen
--undef_sym=FHIE_parseCommonInformation
--undef_sym=FHIE_getPie
--undef_sym=FHIE_getSubPie
--undef_sym=FHIE_getHie

--undef_sym=FHIE_gen
--undef_sym=FHIE_getLen
--undef_sym=FHIE_parsePie
--undef_sym=FHIE_parseHie
--undef_sym=FHIE_extractPie
--undef_sym=FHIE_extractHie

/* fh_mgr.c */
--undef_sym=FHMGR_macStartOneShotTimer
--undef_sym=FHMGR_macStartFHTimer
--undef_sym=FHMGR_macGetElapsedFHTime
--undef_sym=FHMGR_ucTimerIsrCb
--undef_sym=FHMGR_BCTimerEventUpd
--undef_sym=FHMGR_bcTimerIsrCb
--undef_sym=FHMGR_updateRadioUCChannel
--undef_sym=FHMGR_updateRadioBCChannel
--undef_sym=FHMGR_pendUcChUpdate
--undef_sym=FHMGR_macCancelFHTimer
--undef_sym=FHMGR_pendBcChUpdate
--undef_sym=FHMGR_updateHopping

/* fh_nt.c */
--undef_sym=FHNT_initTempTable
--undef_sym=FHNT_getFixedNTIndexFromTempIndex
--undef_sym=FHNT_getNTIndexFromTempIndex
--undef_sym=FHNT_getRemoveEntryFromTemp
--undef_sym=FHNT_addTempEUI

--undef_sym=FHNT_delTempIndex
--undef_sym=FHNT_assessTime
--undef_sym=FHNT_purgeTimerIsrCb
--undef_sym=FHNT_getRemoveEntry

--undef_sym=FHNT_reset
--undef_sym=FHNT_init
--undef_sym=FHNT_purgeEntry
--undef_sym=FHNT_createEntry
--undef_sym=FHNT_getEntry
--undef_sym=FHNT_putEntry
--undef_sym=FHNT_getEUI
--undef_sym=FHNT_AddDeviceCB
--undef_sym=FHNT_DelDeviceCB
--undef_sym=FHNT_getFixedEntry
--undef_sym=FHNT_putFixedEntry
--undef_sym=FHNT_getRemoveFixedEntry
--undef_sym=FHNT_createFixedEntry
--undef_sym=FHNT_getOptEntry
--undef_sym=FHNT_getAvailEntry
--undef_sym=FHNT_addOptEntry


/* fh_pib.c */
--undef_sym=FHPIB_getIndex
--undef_sym=FHPIB_reset
--undef_sym=FHPIB_getLen
--undef_sym=FHPIB_set
--undef_sym=FHPIB_get

/* fh_sm.c */
--undef_sym=FSM_transaction_debug
--undef_sym=FHSM_event

/* fh_util.c */
--undef_sym=FHUTIL_getTxUfsi
--undef_sym=FHUTIL_adjBackOffDur
--undef_sym=FHUTIL_getUcChannel
--undef_sym=FHUTIL_getTxChannel
--undef_sym=FHUTIL_getBcTxParams
--undef_sym=FHUTIL_getCurUfsi
--undef_sym=FHUTIL_getCurBfio
--undef_sym=FHUTIL_getElapsedTime
--undef_sym=FHUTIL_compBfioTxTime
--undef_sym=FHUTIl_updateBTIE
--undef_sym=FHUTIL_calcUfsi
--undef_sym=FHUTIL_assert
--undef_sym=FHUTIL_noAction
--undef_sym=FHUTIL_getCh0
--undef_sym=FHUTIL_getChannelSpacing
--undef_sym=FHUTIL_getMaxChannels
--undef_sym=FHUTIL_getBitCounts
--undef_sym=FHUTIL_updateUcNumChannels
--undef_sym=FHUTIL_updateBcNumChannels
--undef_sym=FHUTIL_getGuardTime
--undef_sym=FHUTIL_elapsedTime


/*
******************************************************************************
    HMAC module ROM functions
******************************************************************************
*/

/* mac_beacon_coord.c */
--undef_sym=macBeaconCoordReset
--undef_sym=MAC_InitBeaconCoord
--undef_sym=macBeaconSetPrepareTime
--undef_sym=macBeaconCheckStartTime
--undef_sym=macApiBeaconStartReq
--undef_sym=macBeaconPeriodCallback
--undef_sym=macBeaconTxCallback
//--undef_sym=macEBeaconTxCallback
//--undef_sym=macEBeaconPrepareCallback
--undef_sym=macBeaconPrepareCallback
--undef_sym=macStartBeaconPrepareCallback
--undef_sym=macBeaconStartContinue
--undef_sym=macBeaconBattLifeCallback
--undef_sym=macTxBeaconCompleteCallback
--undef_sym=macBeaconSetupBroadcast
--undef_sym=macOutgoingNonSlottedTx
--undef_sym=macBeaconSchedRequested
--undef_sym=macBeaconClearIndirect

/* mac_beacon_device.c */
--undef_sym=macBroadcastPendCallback
--undef_sym=macBeaconStopTrack
--undef_sym=macIncomingNonSlottedTx
--undef_sym=macBeaconSyncLoss
--undef_sym=macSyncTimeoutCallback
--undef_sym=macTrackTimeoutCallback
--undef_sym=macTrackStartCallback
--undef_sym=macTrackPeriodCallback
--undef_sym=macRxBeaconCritical
--undef_sym=MAC_InitBeaconDevice
--undef_sym=macCheckPendAddr
--undef_sym=macAutoPoll
--undef_sym=macApiSyncReq
--undef_sym=macBeaconStartFrameResponseTimer
--undef_sym=macStartBroadcastPendTimer

/* mac_beacon.c */
--undef_sym=macBeaconReset
--undef_sym=macBeaconInit
--undef_sym=macBeaconSetSched
--undef_sym=macBeaconCheckSched
--undef_sym=macBeaconRequeue
--undef_sym=macBeaconCheckTxTime
--undef_sym=macBeaconSetupCap

/* mac_coord.c */
--undef_sym=macCoordReset
--undef_sym=macBuildAssociateRsp
--undef_sym=macBuildRealign
--undef_sym=macPendAddrLen
--undef_sym=macBuildPendAddr
//--undef_sym=macBuildEnhancedBeacon
--undef_sym=macBuildBeacon
--undef_sym=macIndirectMark
--undef_sym=macIndirectSend
--undef_sym=macIndirectExpire
--undef_sym=macIndirectTxFrame
--undef_sym=macIndirectRequeueFrame
--undef_sym=macRxAssociateReq
--undef_sym=macApiAssociateRsp
--undef_sym=macRxOrphan
--undef_sym=macApiOrphanRsp
--undef_sym=macApiPurgeReq
--undef_sym=macApiStartReq
--undef_sym=macStartBegin
--undef_sym=macStartSetParams
--undef_sym=macStartContinue
--undef_sym=macStartComplete
--undef_sym=macRxBeaconReq
//--undef_sym=macProcessEBeaconFilterIe
//--undef_sym=macRxEnhancedBeaconReq
--undef_sym=macRxDataReq
--undef_sym=macRxPanConflict
--undef_sym=MAC_InitCoord
--undef_sym=MAC_MlmeAssociateRsp
--undef_sym=MAC_MlmeOrphanRsp
--undef_sym=MAC_McpsPurgeReq
--undef_sym=MAC_MlmeStartReq

/* mac_data.c */
--undef_sym=macDataTxDelayCallback
--undef_sym=macDataReset
--undef_sym=macAllocTxBuffer
--undef_sym=macFrameDurationSubG
--undef_sym=macFrameDurationIeee
--undef_sym=macFrameDuration
--undef_sym=macRxCheckPendingCallback
--undef_sym=macCheckIndPacket
--undef_sym=macRxCheckMACPendingCallback
--undef_sym=macDataRxMemAlloc
--undef_sym=macDataRxMemFree
--undef_sym=macRxCompleteCallback
--undef_sym=macDataTxTimeAvailable
--undef_sym=macBuildDataFrame
--undef_sym=macCheckSched
--undef_sym=macSetSched
--undef_sym=macDataTxComplete
--undef_sym=macTxCompleteCallback
--undef_sym=macDataTxSend
--undef_sym=macDataTxEnqueue
--undef_sym=macDataSend
--undef_sym=macApiDataReq
--undef_sym=macDataRxInd
--undef_sym=MAC_McpsDataReq
--undef_sym=MAC_McpsDataAlloc
--undef_sym=macAutoPendMaintainSrcMatchTable
--undef_sym=macAutoPendAddSrcMatchTableEntry

/* mac_device.c */
--undef_sym=macDeviceReset
--undef_sym=macAssociateCnf
--undef_sym=macPollCnf
--undef_sym=macBuildAssociateReq
--undef_sym=macStartResponseTimer
--undef_sym=macStartFrameResponseTimer
--undef_sym=macApiAssociateReq
--undef_sym=macAssocDataReq
--undef_sym=macAssocDataReqComplete
--undef_sym=macRxAssocRsp
--undef_sym=macAssocFrameResponseTimeout
--undef_sym=macAssocFailed
--undef_sym=macAssocRxDisassoc
--undef_sym=macAssocDataRxInd
--undef_sym=macApiPollReq
--undef_sym=macPollDataReqComplete
--undef_sym=macPollDataRxInd
--undef_sym=macPollFrameResponseTimeout
--undef_sym=macPollRxDisassoc
--undef_sym=macPollRxAssocRsp
--undef_sym=macRxCoordRealign
--undef_sym=macPanConflictComplete
--undef_sym=MAC_InitDevice
--undef_sym=MAC_MlmeAssociateReq
--undef_sym=MAC_MlmePollReq

/* mac_ie_build.c */
--undef_sym=macBuildHeaderIes
--undef_sym=macBuildPayloadCoexistIe
--undef_sym=macBuildPayloadIes

/* mac_ie_parse.c */
--undef_sym=macUpdateHeaderIEPtr
--undef_sym=macUpdatePayloadIEPtr
--undef_sym=macParseHeaderIes
--undef_sym=macParsePayloadIes
--undef_sym=macIeFreeList
--undef_sym=macIeCoexistIePresent
--undef_sym=macMlmeSubIePresent

/* mac_main.c */
--undef_sym=macMainReset
--undef_sym=macSetEvent
--undef_sym=macSetEvent16
--undef_sym=macCbackForEvent
--undef_sym=macStateIdle
--undef_sym=macStateIdleOrPolling
--undef_sym=macStateScanning
--undef_sym=macExecute
--undef_sym=macSendMsg
--undef_sym=macSendDataMsg
--undef_sym=macMainSetTaskId
--undef_sym=macMainReserve
--undef_sym=mac_msg_deallocate

/* mac_mgmt.c */
--undef_sym=macMgmtReset
--undef_sym=macGetCoordAddress
--undef_sym=macGetMyAddrMode
--undef_sym=macDestAddrCmp
--undef_sym=macDestSAddrCmp
--undef_sym=macCoordAddrCmp
--undef_sym=macCoordDestAddrCmp
--undef_sym=macBuildHeader
--undef_sym=macBuildCommonReq
//--undef_sym=macBuildEnhanceBeaconReq
//--undef_sym=macBuildCoexistEBeaconReq
--undef_sym=macBuildDisassociateReq
//--undef_sym=macBuildEBeaconNotifyInd
--undef_sym=macBuildBeaconNotifyInd
--undef_sym=macSecCpy
--undef_sym=macNoAction
--undef_sym=macTxCsmaDelay
--undef_sym=macBcTimerHandler
--undef_sym=macApiUnsupported
--undef_sym=macDefaultAction
--undef_sym=macApiBadState
--undef_sym=macApiPending
--undef_sym=macCommStatusInd
--undef_sym=macApiDisassociateReq
--undef_sym=macDisassocComplete
--undef_sym=macRxDisassoc
--undef_sym=macRxBeacon
--undef_sym=macConflictSyncLossInd
--undef_sym=MAC_MlmeDisassociateReq
--undef_sym=macUpdateCsmPhyParam
--undef_sym=macIsCsmOperational
--undef_sym=MAC_MlmeResetReq
--undef_sym=MAC_ResumeReq
--undef_sym=MAC_YieldReq
--undef_sym=MAC_MlmeSyncReq
--undef_sym=macSetDefaultsByPhyID
--undef_sym=macSetDefaultsByRE
--undef_sym=MAC_Init
--undef_sym=MAC_InitFH
--undef_sym=MAC_MlmeFHGetReqSize
--undef_sym=MAC_MlmeFHGetReq
--undef_sym=MAC_MlmeFHSetReq
--undef_sym=MAC_StartFH
--undef_sym=MAC_EnableFH
--undef_sym=MAC_MlmeWSAsyncReq
--undef_sym=macWSAsyncCnf
--undef_sym=macBuildWSAsyncMsg
--undef_sym=macBuildWSAsyncReq
--undef_sym=macApiWSAsyncReq
--undef_sym=macGetCSMPhy
--undef_sym=MAC_RandomByte
//--undef_sym=macCheckPhyLRMMode
//--undef_sym=macCheckPhyFSKMode

/* mac_pib.c */
//--undef_sym=MAC_MlmeSetActivePib
--undef_sym=macPibReset
//--undef_sym=macPibIndex
--undef_sym=MAC_MlmeGetReq
--undef_sym=MAC_MlmeGetReqSize
--undef_sym=MAC_MlmeSetReq
--undef_sym=MAC_GetPHYParamReq
--undef_sym=MAC_SetPHYParamReq
--undef_sym=macMRFSKGetPhyDesc
--undef_sym=macIEEEGetPhyDesc

/* mac_pwr.c */
//--undef_sym=macPwrReset
//--undef_sym=macApiPwrOnReq
//--undef_sym=MAC_PwrOffReq
//--undef_sym=MAC_PwrOnReq
//--undef_sym=MAC_PwrMode
//--undef_sym=macPwrVote

/* mac_scan.c */
--undef_sym=macScanCnfInit
--undef_sym=macApiScanReq
--undef_sym=macScanNextChan
--undef_sym=macScanStartTimer
--undef_sym=macScanRxBeacon
--undef_sym=macScanRxCoordRealign
--undef_sym=macScanComplete
--undef_sym=macScanFailedInProgress
--undef_sym=MAC_MlmeScanReq

/* mac_security_pib.c */
//--undef_sym=MAC_MlmeSetActiveSecurityPib
--undef_sym=macSecurityPibReset
--undef_sym=macSecurityPibIndex
--undef_sym=MAC_MlmeGetSecurityReq
--undef_sym=MAC_MlmeGetPointerSecurityReq
--undef_sym=MAC_MlmeGetSecurityReqSize
--undef_sym=MAC_MlmeSetSecurityReq

/* mac_security.c */
--undef_sym=macKeyDescriptorLookup
--undef_sym=macIncomingSecurityLevelChecking
--undef_sym=macDeviceDescriptorLookup
--undef_sym=macBlacklistChecking
--undef_sym=macIncomingFrameSecurityMaterialRetrieval
--undef_sym=macIncomingKeyUsagePolicyChecking
--undef_sym=macCcmStarInverseTransform
--undef_sym=macIncomingFrameSecurity
--undef_sym=macOutgoingFrameKeyDescRetrieval
--undef_sym=macOutgoingFrameSecurity
--undef_sym=macCcmStarTransform
--undef_sym=macUpdatePanId
--undef_sym=macGetEUIIndex
--undef_sym=macGetEUI

/* mac_timer.c */
// --undef_sym=macTimerInit
// --undef_sym=macTimerUpdateSymbolTimer
// --undef_sym=macTimerGetNextTimer
// --undef_sym=macSymbolTimerTriggerCallback
// --undef_sym=macTimerRecalcUnaligned
// --undef_sym=macSymbolTimerRolloverCallback
// --undef_sym=macTimerCheckUnAlignedQ
// --undef_sym=macTimerAddTimer
// --undef_sym=macTimerAligned
// --undef_sym=macTimer
// --undef_sym=macTimerRemoveTimer
// --undef_sym=macTimerCancel
// --undef_sym=macTimerUpdActive
// --undef_sym=macTimerGetTime
// --undef_sym=macTimerStart
// --undef_sym=macTimerRealign
// --undef_sym=macTimerSyncRollover
// --undef_sym=macTimerSetRollover
// --undef_sym=MAC_PwrNextTimeout

/* macwrapper.c */
--undef_sym=macWrapper8ByteUnused
--undef_sym=macWrapperAddDevice
--undef_sym=macWrapperDeleteDevice
--undef_sym=macWrapperDeleteKeyAndAssociatedDevices
--undef_sym=macWrapperDeleteAllDevices
--undef_sym=macWrapperGetDefaultSourceKey
--undef_sym=macWrapperAddKeyInitFCtr

/* macstack.c */
//--undef_sym=MAC_CbackEvent
--undef_sym=TIMAC_ROM_FW_VERSION_NUMBER
