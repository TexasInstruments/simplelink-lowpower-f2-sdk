/* ------Don't edit or modify this file------ */

FHAPI_reset = 0x10030809;
FHAPI_start = 0x10030101;
FHAPI_startBS = 0x10030989;
FHAPI_completeTxCb = 0x1003035d;
FHAPI_completeRxCb = 0x10030501;
FHAPI_sendData = 0x10030689;
FHAPI_getTxParams_old = 0x100308d1;
//FHAPI_getTxParams = 0x10030a21;
FHAPI_getTxChHoppingFunc = 0x10030b81;
FHAPI_getRemDT = 0x10030a81;
FHAPI_stopAsync = 0x10030b11;
FHAPI_setStateCb = 0x10030b51;
FHAPI_SFDRxCb = 0x10030ad1;
FHAPI_getBitCount = 0x10030bad;
FHDATA_checkPktFromQueue = 0x10031029;
FHDATA_getPktFromQueue = 0x10030d61;
FHDATA_purgePktFromQueue = 0x10030e89;
FHDATA_purgeUnSupportedFrameTypes = 0x10030e05;
FHDATA_chkASYNCStartReq = 0x10030fb5;
FHDATA_procASYNCReq = 0x10030bd1;
FHDATA_startEDFEReq = 0x10031051;
FHDATA_procEDFEReq = 0x1003103d;
FHDATA_startEDFETimer = 0x10031065;
FHDATA_requeuePkt = 0x10030efd;
FHDATA_updateCCA = 0x10030fed;
//FHDATA_CheckMinTxOffTime = 0x10030cb5;
//FHDATA_getASYNCDelay = 0x10030f65;
FHDATA_asyncMinTxOffTimerCb = 0x10031011;
FHDH1CF_hashword = 0x10031069;
FHDH1CF_getCh = 0x10031209;
FHDH1CF_getBcCh = 0x100312f5;
FHDH1CF_mapChIdxChannel = 0x100311ad;
FHDH1CF_getBCChannelNum = 0x10031129;
FHDH1CF_getChannelNum = 0x1003125d;
FHDH1CF_getTxChannelNum = 0x100312b1;
//FHIE_getCcaSfdTime = 0x10032519;
FHIE_genUTIE = 0x10032755;
//FHIE_genBTIE = 0x1003248d;
FHIE_getChannelPlan = 0x100326f1;
FHIE_isChannelPlanValid = 0x10032805;
FHIE_genExcludedChannelMask = 0x10032689;
FHIE_genExcludedChannelRanges = 0x100323cd;
FHIE_getExcludedChannelControl = 0x10032859;
FHIE_genCommonInformation = 0x10031fed;
FHIE_genPieContent = 0x10031325;
FHIE_genPie = 0x1003210d;
FHIE_genHie = 0x10031d91;
FHIE_getCommonInformationLen = 0x100327b1;
FHIE_getPieContentLen = 0x1003220d;
FHIE_getPieLen = 0x10032599;
FHIE_getHieLen = 0x10032619;
FHIE_parseCommonInformation = 0x10031ba9;
FHIE_getPie = 0x100328cf;
FHIE_getSubPie = 0x10032895;
FHIE_getHie = 0x10032939;
FHIE_gen = 0x10032905;
FHIE_getLen = 0x1003296d;
FHIE_parsePie = 0x10031699;
FHIE_parseHie = 0x10031ec9;
FHIE_extractPie = 0x1003197d;
FHIE_extractHie = 0x1003230d;
FHMGR_macStartOneShotTimer = 0x10032cc9;
FHMGR_macStartFHTimer = 0x10032c1d;
FHMGR_macGetElapsedFHTime = 0x10032d29;
FHMGR_ucTimerIsrCb = 0x10032bb5;
FHMGR_BCTimerEventUpd = 0x10032cf5;
FHMGR_bcTimerIsrCb = 0x1003299d;
FHMGR_updateRadioUCChannel = 0x10032acd;
FHMGR_updateRadioBCChannel = 0x10032b45;
FHMGR_pendUcChUpdate = 0x10032d47;
FHMGR_macCancelFHTimer = 0x10032d11;
FHMGR_pendBcChUpdate = 0x10032d41;
FHMGR_updateHopping = 0x10032c75;
FHNT_initTempTable = 0x10033429;
FHNT_getFixedNTIndexFromTempIndex = 0x100338c1;
FHNT_getNTIndexFromTempIndex = 0x10033671;
FHNT_getRemoveEntryFromTemp = 0x10032ea1;
FHNT_addTempEUI = 0x10033381;
//FHNT_delTempIndex = 0x1003386d;
FHNT_assessTime = 0x10033aa9;
FHNT_purgeTimerIsrCb = 0x10033a3d;
FHNT_getRemoveEntry = 0x100334c5;
FHNT_reset = 0x10033811;
FHNT_init = 0x10033a75;
FHNT_purgeEntry = 0x10032d4d;
FHNT_createEntry = 0x10033209;
FHNT_getEntry = 0x10032fe1;
FHNT_putEntry = 0x10033915;
FHNT_getEUI = 0x100339f9;
FHNT_AddDeviceCB = 0x10033119;
FHNT_DelDeviceCB = 0x100337b1;
FHNT_getFixedEntry = 0x100335f5;
FHNT_putFixedEntry = 0x100339b1;
FHNT_getRemoveFixedEntry = 0x1003355d;
FHNT_createFixedEntry = 0x100332c5;
FHNT_getOptEntry = 0x100336e5;
FHNT_getAvailEntry = 0x10033969;
FHNT_addOptEntry = 0x1003374d;
FHPIB_getIndex = 0x10033d25;
FHPIB_reset = 0x10033d41;
FHPIB_getLen = 0x10033cf5;
FHPIB_set = 0x10033ac9;
FHPIB_get = 0x10033ca5;
FSM_transaction_debug = 0x10033d9d;
FHSM_event = 0x10033d59;
FHUTIL_getTxUfsi = 0x10034319;
FHUTIL_adjBackOffDur = 0x10033dcd;
FHUTIL_getUcChannel = 0x100343a1;
FHUTIL_getTxChannel = 0x1003404d;
FHUTIL_getBcTxParams = 0x10033f09;
FHUTIL_getCurUfsi = 0x10034261;
FHUTIL_getCurBfio = 0x100340cd;
FHUTIL_getElapsedTime = 0x100343c9;
FHUTIL_compBfioTxTime = 0x100343e5;
FHUTIl_updateBTIE = 0x10033fc9;
FHUTIL_calcUfsi = 0x10034411;
FHUTIL_assert = 0x100343fd;
FHUTIL_noAction = 0x1003441f;
FHUTIL_getCh0 = 0x100342a9;
FHUTIL_getChannelSpacing = 0x10034219;
FHUTIL_getMaxChannels = 0x100342e1;
FHUTIL_getBitCounts = 0x100341c1;
FHUTIL_updateUcNumChannels = 0x10034375;
FHUTIL_updateBcNumChannels = 0x10034349;
FHUTIL_getGuardTime = 0x10034149;
FHUTIL_elapsedTime = 0x1003441b;
macBeaconCoordReset = 0x10034a01;
MAC_InitBeaconCoord = 0x10034849;
macBeaconSetPrepareTime = 0x100349d5;
macBeaconCheckStartTime = 0x10034715;
//macApiBeaconStartReq = 0x10034625;
macBeaconPeriodCallback = 0x1003495d;
macBeaconTxCallback = 0x10034781;
macBeaconPrepareCallback = 0x100346a1;
macStartBeaconPrepareCallback = 0x10034895;
macBeaconStartContinue = 0x10034545;
macBeaconBattLifeCallback = 0x10034a25;
macTxBeaconCompleteCallback = 0x10034421;
macBeaconSetupBroadcast = 0x100347ed;
macOutgoingNonSlottedTx = 0x10034999;
macBeaconSchedRequested = 0x100348e1;
macBeaconClearIndirect = 0x10034921;
macBroadcastPendCallback = 0x10035355;
macBeaconStopTrack = 0x10035289;
macIncomingNonSlottedTx = 0x100352cd;
macBeaconSyncLoss = 0x10035065;
macSyncTimeoutCallback = 0x10034e8d;
//macTrackTimeoutCallback = 0x10034fd1;
//macTrackStartCallback = 0x10034f3d;
//macTrackPeriodCallback = 0x100351c1;
//macRxBeaconCritical = 0x10034a39;
MAC_InitBeaconDevice = 0x10035159;
macCheckPendAddr = 0x100350ed;
//macAutoPoll = 0x10034d49;
macApiSyncReq = 0x10034bcd;
macBeaconStartFrameResponseTimer = 0x10035229;
macStartBroadcastPendTimer = 0x10035311;
macBeaconReset = 0x100355a9;
macBeaconInit = 0x10035575;
macBeaconSetSched = 0x10035369;
macBeaconCheckSched = 0x100355dd;
macBeaconRequeue = 0x10035535;
//macBeaconCheckTxTime = 0x100354b5;
macBeaconSetupCap = 0x10035421;
macCoordReset = 0x10035c85;
macBuildAssociateRsp = 0x1003560d;
macBuildRealign = 0x10035d4d;
macPendAddrLen = 0x100364a9;
//macBuildPendAddr = 0x10035bb5;
macBuildBeacon = 0x1003576d;
macIndirectMark = 0x100361e5;
macIndirectSend = 0x10035e09;
macIndirectExpire = 0x100358c5;
macIndirectTxFrame = 0x10036085;
macIndirectRequeueFrame = 0x10036171;
//macRxAssociateReq = 0x100362b1;
macApiAssociateRsp = 0x10036399;
macRxOrphan = 0x10036355;
macApiOrphanRsp = 0x100360fd;
macApiPurgeReq = 0x10035eb1;
//macApiStartReq = 0x10035ff5;
macStartBegin = 0x10035ad5;
macStartSetParams = 0x100359d5;
macStartContinue = 0x10036309;
macStartComplete = 0x100363d9;
macRxBeaconReq = 0x10036419;
macRxDataReq = 0x10035f55;
macRxPanConflict = 0x100364c5;
MAC_InitCoord = 0x1003624d;
MAC_MlmeAssociateRsp = 0x10036471;
MAC_MlmeOrphanRsp = 0x1003648d;
MAC_McpsPurgeReq = 0x10036455;
MAC_MlmeStartReq = 0x100364e1;
macDataTxDelayCallback = 0x10037529;
macDataReset = 0x10037279;
macAllocTxBuffer = 0x10036ba1;
macFrameDurationSubG = 0x100372f5;
macFrameDurationIeee = 0x1003749d;
macFrameDuration = 0x10037479;
macRxCheckPendingCallback = 0x1003742d;
macCheckIndPacket = 0x10037361;
macRxCheckMACPendingCallback = 0x100373fd;
macDataRxMemAlloc = 0x10037455;
macDataRxMemFree = 0x10037515;
macRxCompleteCallback = 0x10036cd9;
macDataTxTimeAvailable = 0x100374fd;
//macBuildDataFrame = 0x10036a61;
macCheckSched = 0x100374e1;
macSetSched = 0x1003753d;
//macDataTxComplete = 0x10037079;
//macTxCompleteCallback = 0x100364f9;
macDataTxSend = 0x100368cd;
macDataTxEnqueue = 0x1003712d;
macDataSend = 0x100374c1;
macApiDataReq = 0x10036ef5;
macDataRxInd = 0x100373b9;
//MAC_McpsDataReq = 0x10036729;
MAC_McpsDataAlloc = 0x10036e05;
//macAutoPendMaintainSrcMatchTable = 0x10036fc5;
//macAutoPendAddSrcMatchTableEntry = 0x100371e1;
macDeviceReset = 0x100377a1;
macAssociateCnf = 0x10037d79;
macPollCnf = 0x10037e3d;
macBuildAssociateReq = 0x10037549;
macStartResponseTimer = 0x10037e15;
macStartFrameResponseTimer = 0x10037d35;
macApiAssociateReq = 0x10037699;
macAssocDataReq = 0x10037c99;
macAssocDataReqComplete = 0x10037b25;
macRxAssocRsp = 0x10037871;
macAssocFrameResponseTimeout = 0x10037db5;
macAssocFailed = 0x10037ced;
macAssocRxDisassoc = 0x10037e85;
macAssocDataRxInd = 0x1003792d;
macApiPollReq = 0x10037ab9;
macPollDataReqComplete = 0x10037b91;
macPollDataRxInd = 0x100379b1;
macPollFrameResponseTimeout = 0x10037bed;
macPollRxDisassoc = 0x10037de9;
macPollRxAssocRsp = 0x10037e61;
macRxCoordRealign = 0x10037a35;
macPanConflictComplete = 0x10037ed5;
MAC_InitDevice = 0x10037c45;
MAC_MlmeAssociateReq = 0x10037ea5;
MAC_MlmePollReq = 0x10037ebd;
// macBuildHeaderIes = 0x10038005;
macBuildPayloadCoexistIe = 0x100380d9;
macBuildPayloadIes = 0x10037ee9;
macUpdateHeaderIEPtr = 0x10038349;
macUpdatePayloadIEPtr = 0x10038489;
macParseHeaderIes = 0x1003818d;
macParsePayloadIes = 0x1003827d;
macIeFreeList = 0x100384e5;
macIeCoexistIePresent = 0x1003842b;
macMlmeSubIePresent = 0x100383c5;
macMainReset = 0x10038791;
macSetEvent = 0x10038861;
macSetEvent16 = 0x10038845;
// macCbackForEvent = 0x100385f1;
macStateIdle = 0x100388b1;
macStateIdleOrPolling = 0x1003887d;
macStateScanning = 0x100388c9;
macExecute = 0x1003871d;
macSendMsg = 0x100386a1;
macSendDataMsg = 0x10038501;
macMainSetTaskId = 0x100388e1;
macMainReserve = 0x10038899;
mac_msg_deallocate = 0x100387f9;
//macMgmtReset = 0x1003967d;
macGetCoordAddress = 0x10039c35;
macGetMyAddrMode = 0x10039d81;
macDestAddrCmp = 0x10039ae9;
macDestSAddrCmp = 0x10039b35;
macCoordAddrCmp = 0x10039bc1;
macCoordDestAddrCmp = 0x10039a41;
macBuildHeader = 0x100388f1;
macBuildCommonReq = 0x10038c1d;
macBuildDisassociateReq = 0x10038f61;
macBuildBeaconNotifyInd = 0x100394f1;
macSecCpy = 0x10039e91;
macNoAction = 0x10039ec1;
macTxCsmaDelay = 0x10039e3d;
macBcTimerHandler = 0x10039e69;
macApiUnsupported = 0x10039e25;
macDefaultAction = 0x10039d61;
macApiBadState = 0x10039e0d;
macApiPending = 0x10039b81;
macCommStatusInd = 0x100393ed;
macApiDisassociateReq = 0x10039955;
macDisassocComplete = 0x100397c9;
macRxDisassoc = 0x100398d9;
macRxBeacon = 0x100395bd;
macConflictSyncLossInd = 0x10039c95;
MAC_MlmeDisassociateReq = 0x10039dc1;
macUpdateCsmPhyParam = 0x10039ea5;
macIsCsmOperational = 0x10039e7d;
//MAC_MlmeResetReq = 0x100391c9;
//MAC_ResumeReq = 0x10039729;
MAC_YieldReq = 0x10039c69;
MAC_MlmeSyncReq = 0x10039ddd;
macSetDefaultsByPhyID = 0x10039095;
macSetDefaultsByRE = 0x10039bfd;
MAC_Init = 0x100399cd;
//MAC_InitFH = 0x10038e15;
MAC_MlmeFHGetReqSize = 0x10039d01;
MAC_MlmeFHGetReq = 0x10039ce1;
MAC_MlmeFHSetReq = 0x10039d21;
MAC_StartFH = 0x10039d41;
MAC_EnableFH = 0x10039cbd;
MAC_MlmeWSAsyncReq = 0x10039df5;
macWSAsyncCnf = 0x10039da1;
macBuildWSAsyncMsg = 0x10039855;
macBuildWSAsyncReq = 0x100392e1;
macApiWSAsyncReq = 0x10039a99;
macGetCSMPhy = 0x10039eb9;
MAC_RandomByte = 0x10039e55;
macPibReset = 0x1003a239;
MAC_MlmeGetReq = 0x1003a169;
MAC_MlmeGetReqSize = 0x1003a275;
MAC_MlmeSetReq = 0x10039ec5;
MAC_GetPHYParamReq = 0x1003a1b9;
MAC_SetPHYParamReq = 0x1003a1f9;
macMRFSKGetPhyDesc = 0x1003a115;
macIEEEGetPhyDesc = 0x1003a29d;
macScanCnfInit = 0x1003a931;
macApiScanReq = 0x1003a2ad;
//macScanNextChan = 0x1003a489;
macScanStartTimer = 0x1003a8b9;
macScanRxBeacon = 0x1003a599;
//macScanRxCoordRealign = 0x1003a761;
macScanComplete = 0x1003a839;
macScanFailedInProgress = 0x1003a96d;
//MAC_MlmeScanReq = 0x1003a689;
macSecurityPibReset = 0x1003ac6d;
macSecurityPibIndex = 0x1003b199;
MAC_MlmeGetSecurityReq = 0x1003af09;
MAC_MlmeGetPointerSecurityReq = 0x1003b159;
MAC_MlmeGetSecurityReqSize = 0x1003b101;
MAC_MlmeSetSecurityReq = 0x1003a99d;
//macKeyDescriptorLookup = 0x1003b965;
//macIncomingSecurityLevelChecking = 0x1003b8d1;
//macDeviceDescriptorLookup = 0x1003bad5;
//macBlacklistChecking = 0x1003b789;
//macIncomingFrameSecurityMaterialRetrieval = 0x1003b1ad;
//macIncomingKeyUsagePolicyChecking = 0x1003bb2d;
//macCcmStarInverseTransform = 0x1003b82d;
//macIncomingFrameSecurity = 0x1003b34d;
//macOutgoingFrameKeyDescRetrieval = 0x1003b4d5;
//macOutgoingFrameSecurity = 0x1003b5cd;
//macCcmStarTransform = 0x1003b6b1;
macUpdatePanId = 0x1003b9f1;
macGetEUIIndex = 0x1003ba75;
macGetEUI = 0x1003bb81;
//macWrapper8ByteUnused = 0x1003c7c1;
//macWrapperAddDevice = 0x1003bbbd;
//macWrapperDeleteDevice = 0x1003c3b1;
//macWrapperDeleteKeyAndAssociatedDevices = 0x1003c1f9;
//macWrapperDeleteAllDevices = 0x1003c565;
macWrapperGetDefaultSourceKey = 0x1003c6e9;
//macWrapperAddKeyInitFCtr = 0x1003c02d;


-u RAM_MAC_BASE_ADDR
SECTIONS
{
.data:RAM_MAC_BASE_ADDR: LOAD > 0x2000010c
}
