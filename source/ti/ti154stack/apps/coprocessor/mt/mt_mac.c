/******************************************************************************

 @file  mt_mac.c

 @brief Monitor/Test functions for MT MAC commands/callbacks

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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

/******************************************************************************
 Includes
 *****************************************************************************/
#include <string.h>

#include "api_mac.h"

#include "mt_mac.h"
#include "mt_pkt.h"
#include "mt_rpc.h"
#include "mt_util.h"

#if defined(TIMAC_ROM_IMAGE_BUILD)
#include "icall_osal_rom_jt.h"
#else
#include "icall_osal_map_direct.h"
#endif

#if defined(MT_MAC_FUNC)
/******************************************************************************
 * Macros
 *****************************************************************************/
/*! AREQ RPC response for MT_MAC callbacks (indications/confirms) */
#define MT_ARSP_MAC ((uint8_t)MTRPC_CMD_AREQ | (uint8_t)MTRPC_SYS_MAC)

/*! SRSP RPC response for MT_MAC requests (commands/responses) */
#define MT_SRSP_MAC ((uint8_t)MTRPC_CMD_SRSP | (uint8_t)MTRPC_SYS_MAC)

/******************************************************************************
 Constants
 *****************************************************************************/
/*!
 A MAC PIB table of Device-Descriptor entries, each indicating a remote device
 with which this device securely communicates. This PIB attribute should not
 be accessed by the host device.
 */
#define MT_MAC_PIB_DEVICE_TABLE  0x72
/*!
 A MAC PIB table of SecurityLevel-Descriptor entries, each with information
 about the minimum security level expected depending on incoming frame type
 and subtype. This PIB attribute should not be accessed by the host device.
 */
#define MT_MAC_PIB_SECURITY_LEVEL_TABLE  0x73

/*
 TX Options - these should track values in the MAC file: mac_api.h
 */
/*! MAC will attempt to retransmit the frame until it is acknowledged */
#define MT_MAC_TXOPTION_ACK         0x01
/*! GTS transmission (currently unsupported) */
#define MT_MAC_TXOPTION_GTS         0x02
/*! MAC will queue the data and wait for the destination device to poll */
#define MT_MAC_TXOPTION_INDIRECT    0x04
/*! Force the pending bit set for direct transmission */
#define MT_MAC_TXOPTION_PEND_BIT    0x08
/*! Prevent the frame from being retransmitted */
#define MT_MAC_TXOPTION_NO_RETRANS  0x10
/*! Prevent a MAC_MCPS_DATA_CNF event from being sent */
#define MT_MAC_TXOPTION_NO_CNF      0x20
/*! Use PIB value MAC_ALT_BE for the minimum backoff exponent */
#define MT_MAC_TXOPTION_ALT_BE      0x40
/*! Use the power/channel values in DataReq instead of PIB values */
#define MT_MAC_TXOPTION_PWR_CHAN    0x80
/* Special transmit for Green Power. Must be direct */
#define MT_MAC_TXOPTION_GREEN_PWR   0x0100
/* Use Absolute time for Green Power. */
#define MT_MAC_TXOPTION_GP_ABS_TIME 0x0200

/*!
 MT indication/confirm minimum packed serial buffer sizes
 */
/*! Minimum serial packet size - Async Data Ind */
#define MT_MAC_LEN_ASYNC_IND     sizeof(MtPkt_adataInd_t)
/*! Required serial packet size - Associate Cnf */
#define MT_MAC_LEN_ASSOC_CNF     sizeof(MtPkt_assocCnf_t)
/*! Required serial packet size - Associate Ind */
#define MT_MAC_LEN_ASSOC_IND     sizeof(MtPkt_assocInd_t)
/*! Required serial packet size - Associate Ind */
#define MT_MAC_LEN_ASYNC_CNF     sizeof(MtPkt_asyncCnf_t)
/*! Minimum serial packet size - Beacon Ind */
#define MT_MAC_LEN_BEACON_IND    sizeof(MtPkt_beaconInd_t)
/*! Required serial packet size - Comm Status Ind */
#define MT_MAC_LEN_COMMST_IND    sizeof(MtPkt_commstInd_t)
/*! Required serial packet size - Data Cnf */
#define MT_MAC_LEN_DATA_CNF      sizeof(MtPkt_dataCnf_t)
/*! Minimum serial packet size - Data Ind */
#define MT_MAC_LEN_DATA_IND      sizeof(MtPkt_dataInd_t)
/*! Required serial packet size - Disassociate Cnf */
#define MT_MAC_LEN_DISASSOC_CNF  sizeof(MtPkt_dassocCnf_t)
/*! Required serial packet size - Disassociate Ind */
#define MT_MAC_LEN_DISASSOC_IND  sizeof(MtPkt_dassocInd_t)
/*! Required serial packet size - eBeacon Ind */
#define MT_MAC_LEN_EBEACON_IND   sizeof(MtPkt_ebeaconInd_t)
/*! Required serial packet size - Orphan Ind */
#define MT_MAC_LEN_ORPHAN_IND    sizeof(MtPkt_orphanInd_t)
/*! Required serial packet size - Poll Cnf */
#define MT_MAC_LEN_POLL_CNF      sizeof(MtPkt_pollCnf_t)
/*! Required serial packet size - Poll Ind */
#define MT_MAC_LEN_POLL_IND      sizeof(MtPkt_pollInd_t)
/*! Required serial packet size - Purge Cnf */
#define MT_MAC_LEN_PURGE_CNF     sizeof(MtPkt_purgeCnf_t)
/*! Minimum serial packet size - Scan Cnf */
#define MT_MAC_LEN_SCAN_CNF      sizeof(MtPkt_scanCnf_t)
/*! Required serial packet size - Start Cnf */
#define MT_MAC_LEN_START_CNF     sizeof(MtPkt_startCnf_t)
/*! Required serial packet size - Poll Ind */
#define MT_MAC_LEN_SYLOSS_IND    sizeof(MtPkt_sylossInd_t)

/*! Packed Security data block length */
#define MT_MAC_LEN_SECINFO  sizeof(MtPkt_secInfo_t)

/*! Packed Pan Descriptor data block length */
#define MT_MAC_LEN_PANDESC  sizeof(MtPkt_panDesc_t)

/*! Maximum net name size in FHPIB (see fh_pib.h) */
#define FHPIB_NET_NAME_SIZE_MAX  32

/*! Maximum parameter size for FHPIB (see macNetName[] in fh_pib.h) */
#define MAX_PARAM_SIZE_FH  (FHPIB_NET_NAME_SIZE_MAX * sizeof(uint8_t))

/*! Maximum parameter size for MAC PIB (as specified in 15.4 CoP interface guide) */
#define MAX_PARAM_SIZE_MAC  16

/*! Maximum buffer size for MAC PIB to support advanced features */
#define MAX_PARAM_BUF_SIZE_MAC  256

/*! Maximum parameter size for MAC PIB (see mac_security_pib.h) */
#define MAX_PARAM_SIZE_SEC  (APIMAC_KEY_SOURCE_MAX_LEN*sizeof(uint8_t))

/*! Maximum active scan results (to limit allocated buffer size) */
#define MAX_ACTIVE_SCAN_RESULTS  6

/*! Transmit Window for Green Power*/
#define GP_DURATION 255

/******************************************************************************
 Global Variables
 *****************************************************************************/

/*! Transmit Delay for Green Power*/
uint8_t GP_Offset = 20;

/******************************************************************************
 Callback Subscription ID Bit Defintions
 *****************************************************************************/
/* MAC Callback subscription ID bits */
#define CBSID_ASSOCIATE_CNF      0x00000001
#define CBSID_ASSOCIATE_IND      0x00000002
#define CBSID_BEACON_NOTIFY_IND  0x00000004
#define CBSID_COMM_STATUS_IND    0x00000008
#define CBSID_DATA_CNF           0x00000010
#define CBSID_DATA_IND           0x00000020
#define CBSID_DISASSOCIATE_CNF   0x00000040
#define CBSID_DISASSOCIATE_IND   0x00000080
#define CBSID_ORPHAN_IND         0x00000100
#define CBSID_POLL_CNF           0x00000200
#define CBSID_POLL_IND           0x00000400
#define CBSID_PURGE_CNF          0x00000800
#define CBSID_SCAN_CNF           0x00001000
#define CBSID_START_CNF          0x00002000
#define CBSID_SYNC_LOSS_IND      0x00004000
#define CBSID_WS_ASYNC_CNF       0x00008000
#define CBSID_WS_ASYNC_IND       0x00010000

#define CBSID_ALL                0x0001FFFF
#define CBSID_DISABLE_CMD        0x80000000

/* Default callbacks to be enabled at system reset */
#define CBSID_DEFAULT            (CBSID_ALL)

/******************************************************************************
 Local Variables
 *****************************************************************************/
static uint32_t callbackEnables = CBSID_DEFAULT;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
/* MT API request/response functions */
static void macAssociateReq(Mt_mpb_t *pMpb);
static void macAssociateRsp(Mt_mpb_t *pMpb);
static void macAsyncReq(Mt_mpb_t *pMpb);
static void macDataReq(Mt_mpb_t *pMpb);
static void macDisassociateReq(Mt_mpb_t *pMpb);
static void macEnableFhReq(Mt_mpb_t *pMpb);
static void macGetReq(Mt_mpb_t *pMpb);
static void macGetFhReq(Mt_mpb_t *pMpb);
static void macInitReq(Mt_mpb_t *pMpb);
static void macOrphanRsp(Mt_mpb_t *pMpb);
static void macPollReq(Mt_mpb_t *pMpb);
static void macPurgeReq(Mt_mpb_t *pMpb);
static void macResetReq(Mt_mpb_t *pMpb);
static void macScanReq(Mt_mpb_t *pMpb);
static void macSetReq(Mt_mpb_t *pMpb);
static void macSetFhReq(Mt_mpb_t *pMpb);
static void macSetRxGainReq(Mt_mpb_t *pMpb);
static void macStartReq(Mt_mpb_t *pMpb);
static void macStartFhReq(Mt_mpb_t *pMpb);
static void macSyncReq(Mt_mpb_t *pMpb);

/* Security PIB request/response functions */
static void macAddDeviceReq(Mt_mpb_t *pMpb);
static void macDeleteAllDevicesReq(Mt_mpb_t *pMpb);
static void macDeleteDeviceReq(Mt_mpb_t *pMpb);
static void macDeleteKeyReq(Mt_mpb_t *pMpb);
#ifdef FEATURE_MAC_SECURITY
static void macGetSecReq(Mt_mpb_t *pMpb);
static void macSetSecReq(Mt_mpb_t *pMpb);
#endif
static void macReadKeyReq(Mt_mpb_t *pMpb);
static void macSetPanIdReq(Mt_mpb_t *pMpb);
static void macSrcMatchEnable(Mt_mpb_t *pMpb);
static void macWriteKeyReq(Mt_mpb_t *pMpb);

/* MT API callback helper functions */
static void dataInd(uint8_t, ApiMac_mcpsDataInd_t *pInd);
static void eBeaconInd(ApiMac_mlmeBeaconNotifyInd_t *pInd);
static void nBeaconInd(ApiMac_mlmeBeaconNotifyInd_t *pInd);

/* General utility functions */
static uint8_t *copyExtAdr(uint8_t *pDst, uint8_t *pSrc);
static uint8_t *macAdrToSba(uint8_t *pDst, ApiMac_sAddr_t *pSrc);
static void macSbaToAdr(ApiMac_sAddr_t *pDst, uint8_t *pSrc);
static void macSbaToSec(ApiMac_sec_t *pDst, uint8_t *pSrc);
static void macSecToSba(uint8_t *pDst, ApiMac_sec_t *pSrc);
static void parseOptBits(uint16_t optBits, ApiMac_txOptions_t *txOptions);
static void sendCRSP(uint8_t rId, uint16_t rLen, uint8_t *pRsp);
static void sendDRSP(uint8_t rId, uint16_t rLen, uint8_t *pRsp);
static void sendSRSP(uint8_t rId, uint8_t rsp);
#ifdef FEATURE_MAC_SECURITY
/* Security PIB utility functions */
static uint8_t bufferDeviceEntry(uint8_t *pDst, void *pSrc);
static uint8_t bufferKeyDevice(uint8_t *pDst, void *pSrc);
static uint8_t bufferKeyEntry(uint8_t *pDst, void *pSrc);
static uint8_t bufferKeyLookup(uint8_t *pDst, void *pSrc);
static uint8_t bufferKeyUsage(uint8_t *pDst, void *pSrc);
static uint8_t bufferLevelEntry(uint8_t *pDst, void *pSrc);
static uint8_t bufferMiscParam(uint8_t *pDst, void *pSrc, uint8_t sLen);

static uint8_t buildDeviceEntry(Mt_mpb_t *pMpb);
static uint8_t buildKeyDevice(Mt_mpb_t *pMpb);
static uint8_t buildKeyEntry(Mt_mpb_t *pMpb);
static uint8_t buildKeyLookup(Mt_mpb_t *pMpb);
static uint8_t buildKeyTable(Mt_mpb_t *pMpb);
static uint8_t buildKeyUsage(Mt_mpb_t *pMpb);
static uint8_t buildLevelEntry(Mt_mpb_t *pMpb);
static uint8_t buildSecParam(uint8_t attr, Mt_mpb_t *pMpb);
#endif
/******************************************************************************
 External Functions
 *****************************************************************************/
extern ApiMac_status_t
ApiMac_mlmeGetReqArrayLen(ApiMac_attribute_array_t attr,
                          uint8_t* pValue, uint16_t* pLen);

extern ApiMac_status_t
ApiMac_mlmeGetFhReqArrayLen(ApiMac_FHAttribute_array_t attr,
                            uint8_t* pValue, uint16_t* pLen);

extern ApiMac_status_t
ApiMac_mlmeGetSecurityReqArrayLen(ApiMac_securityAttribute_array_t attr,
                                  uint8_t* pValue, uint16_t* pLen);

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Processes MT MAC commands received from the host

 Public function that is defined in mt_mac.h
 */
uint8_t MtMac_commandProcessing(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_success;

    switch(pMpb->cmd1)
    {
        case MT_MAC_RESET_REQ:
            macResetReq(pMpb);
            break;

        case MT_MAC_INIT_REQ:
            macInitReq(pMpb);
            break;

        case MT_MAC_START_REQ:
            macStartReq(pMpb);
            break;

        case MT_MAC_DATA_REQ:
            macDataReq(pMpb);
            break;

        case MT_MAC_ASSOCIATE_REQ:
            macAssociateReq(pMpb);
            break;

        case MT_MAC_SYNC_REQ:
            macSyncReq(pMpb);
            break;

        case MT_MAC_DISASSOCIATE_REQ:
            macDisassociateReq(pMpb);
            break;

        case MT_MAC_GET_REQ:
            macGetReq(pMpb);
            break;

        case MT_MAC_SET_REQ:
            macSetReq(pMpb);
            break;

        case MT_MAC_POLL_REQ:
            macPollReq(pMpb);
            break;

        case MT_MAC_PURGE_REQ:
            macPurgeReq(pMpb);
            break;

        case MT_MAC_SET_RX_GAIN_REQ:
            macSetRxGainReq(pMpb);
            break;

        case MT_MAC_SCAN_REQ:
            macScanReq(pMpb);
            break;
#ifdef FEATURE_MAC_SECURITY
        case MT_MAC_SECURITY_GET_REQ:
            macGetSecReq(pMpb);
            break;

        case MT_MAC_SECURITY_SET_REQ:
            macSetSecReq(pMpb);
            break;
#endif
        case MT_MAC_UPDATE_PANID_REQ:
            macSetPanIdReq(pMpb);
            break;

        case MT_MAC_SRC_MATCH_EN_REQ:
            macSrcMatchEnable(pMpb);
            break;

        case MT_MAC_ADD_DEVICE_REQ:
            macAddDeviceReq(pMpb);
            break;

        case MT_MAC_DELETE_DEVICE_REQ:
            macDeleteDeviceReq(pMpb);
            break;

        case MT_MAC_DELETE_ALL_REQ:
            macDeleteAllDevicesReq(pMpb);
            break;

        case MT_MAC_DELETE_KEY_REQ:
            macDeleteKeyReq(pMpb);
            break;

        case MT_MAC_READ_KEY_REQ:
            macReadKeyReq(pMpb);
            break;

        case MT_MAC_WRITE_KEY_REQ:
            macWriteKeyReq(pMpb);
            break;

        case MT_MAC_WS_ASYNC_REQ:
            macAsyncReq(pMpb);
            break;

        case MT_MAC_FH_ENABLE_REQ:
            macEnableFhReq(pMpb);
            break;

        case MT_MAC_FH_START_REQ:
            macStartFhReq(pMpb);
            break;

        case MT_MAC_FH_GET_REQ:
            macGetFhReq(pMpb);
            break;

        case MT_MAC_FH_SET_REQ:
            macSetFhReq(pMpb);
            break;

        case MT_MAC_ASSOCIATE_RSP:
            macAssociateRsp(pMpb);
            break;

        case MT_MAC_ORPHAN_RSP:
            macOrphanRsp(pMpb);
            break;

        default:
            status = ApiMac_status_commandIDError;
            break;
    }

    return(status);
}

/*!
 Update enabled MT MAC callbacks

 Public function that is defined in mt_mac.h
 */
uint32_t MtMac_setCallbacks(uint32_t cbBits)
{
    if(cbBits & CBSID_DISABLE_CMD)
    {
        /* Disable callbacks for bits that are set */
        callbackEnables &= ~(cbBits ^ CBSID_DISABLE_CMD);
    }
    else
    {
        /* Enable callbacks for bits that are set */
        callbackEnables |= (cbBits & CBSID_ALL);
    }

    /* Tell caller which callbacks are enabled */
    return(callbackEnables);
}

/******************************************************************************
 * Local API Request Functions
 *****************************************************************************/
/*!
 * @brief   Process MAC_ADD_DEVICE_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macAddDeviceReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_addDevReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_secAddDevice_t aReq;

        /* New device's Pan ID */
        aReq.panID = Util_parseUint16(pBuf);
        pBuf += 2;

        /* New device's short address */
        aReq.shortAddr = Util_parseUint16(pBuf);
        pBuf += 2;

        /* New device's extended address */
        (void)copyExtAdr(aReq.extAddr, pBuf);
        pBuf += APIMAC_SADDR_EXT_LEN;

        /* Frame counter */
        aReq.frameCounter = Util_parseUint32(pBuf);
        pBuf += 4;

        /* Minimum security override indicator */
        aReq.exempt = *pBuf++;

        /* Key device descriptor uniqueDevice indicator */
        aReq.uniqueDevice = *pBuf++;

        /* Duplicate device entry indicator */
        aReq.duplicateDevFlag = *pBuf++;

        /* Key ID lookup data size indicator, 0=5 bytes, 1=9 bytes */
        aReq.keyIdLookupDataSize = *pBuf++;

        /* Key ID lookup data */
        memcpy(&aReq.keyIdLookupData, pBuf, APIMAC_MAX_KEY_LOOKUP_LEN);
#ifdef FEATURE_MAC_SECURITY
        /* Send request to the MAC task */
        status = ApiMac_secAddDevice(&aReq);
#endif
    }

    /* Send host a response */
    sendSRSP(MT_MAC_ADD_DEVICE_REQ, status);
}

/*!
 * @brief   Process MAC_ASSOCIATE_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macAssociateReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_assocReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_mlmeAssociateReq_t aReq;

        /* Logical Channel */
        aReq.logicalChannel = *pBuf++;

        /* Channel Page */
        aReq.channelPage = *pBuf++;

        /* PHY ID */
        aReq.phyID = *pBuf++;

        /* Coordinator Address Mode */
        aReq.coordAddress.addrMode = (ApiMac_addrType_t)*pBuf++;

        /* Coordinator Address */
        macSbaToAdr(&aReq.coordAddress, pBuf);
        pBuf += APIMAC_SADDR_EXT_LEN;

        /* Coordinator Pan ID */
        aReq.coordPanId = Util_parseUint16(pBuf);
        pBuf += 2;

        /* Capability Info bit mask */
        ApiMac_buildMsgCapInfo(*pBuf++, &aReq.capabilityInformation);

        /* Security parameters */
        macSbaToSec(&aReq.sec, pBuf);

        /* Send request to the MAC task */
        status = ApiMac_mlmeAssociateReq(&aReq);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_ASSOCIATE_REQ, status);
}

/*!
 * @brief   Process MAC_ASSOCIATE_RSP command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macAssociateRsp(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_assocRsp_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_mlmeAssociateRsp_t aRsp;

        /* Extended address of requesting device */
        (void)copyExtAdr(aRsp.deviceAddress, pBuf);
        pBuf += APIMAC_SADDR_EXT_LEN;

        /* Short address allocated to the associated device */
        aRsp.assocShortAddress = Util_parseUint16(pBuf);
        pBuf += 2;

        /* Status of the association */
        aRsp.status = (ApiMac_assocStatus_t)*pBuf++;

        /* Security parameters */
        macSbaToSec(&aRsp.sec, pBuf);

        /* Send request to the MAC task */
        status = ApiMac_mlmeAssociateRsp(&aRsp);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_ASSOCIATE_RSP, status);
}

/*!
 * @brief   Process MAC_WS_ASYNC_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming parameter block
 */
static void macAsyncReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_asyncReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_mlmeWSAsyncReq_t aReq;

        /* Async operation start/stop */
        aReq.operation = (ApiMac_wisunAsycnOperation_t)*pBuf++;

        /* Async frame type */
        aReq.frameType = (ApiMac_wisunAsyncFrame_t)*pBuf++;

        /* Security parameters */
        macSbaToSec(&aReq.sec, pBuf);
        pBuf += MT_MAC_LEN_SECINFO;

        /* Channel bitmask array */
        memcpy(&aReq.channels, pBuf, APIMAC_154G_CHANNEL_BITMAP_SIZ);

        /* Send request to the MAC task */
        status = ApiMac_mlmeWSAsyncReq(&aReq);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_WS_ASYNC_REQ, status);
}

/*!
 * @brief   Process MAC_DATA_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macDataReq(Mt_mpb_t *pMpb)
{
    extern bool ApiMac_mtDataReqTxOptionGp;
    uint16_t dLen;
    uint8_t status;
    ApiMac_mcpsDataReq_t dReq;
    uint8_t *pBuf = pMpb->pData;

    /* Destination address mode */
    dReq.dstAddr.addrMode = (ApiMac_addrType_t)*pBuf++;

    /* Destination address */
    macSbaToAdr(&dReq.dstAddr, pBuf);
    pBuf += APIMAC_SADDR_EXT_LEN;

    /* Destination Pan ID */
    dReq.dstPanId = Util_parseUint16(pBuf);
    pBuf += 2;

    /* Source address mode */
    dReq.srcAddrMode = (ApiMac_addrType_t)*pBuf++;

    /* MSDU handle */
    dReq.msduHandle = *pBuf++;

    /* TX options bit mask */
    if(ApiMac_mtDataReqTxOptionGp == true)
    {
        /* Check and see if we're using Absolute or Relative Time */
        parseOptBits(Util_parseUint16(pBuf), &dReq.txOptions);
        pBuf += 2;
    }
    else
    {
        parseOptBits(*pBuf++, &dReq.txOptions);
    }

    /* TX channel */
    dReq.channel = *pBuf++;

    /* TX power */
    dReq.power = *pBuf++;

    /* Security parameters */
    macSbaToSec(&dReq.sec, pBuf);
    pBuf += MT_MAC_LEN_SECINFO;

    /* Frequency hopping Protocol Dispatch - HARDWIRED FOR NOW */
    dReq.fhProtoDispatch = ApiMac_fhDispatchType_none;

    /* Frequency hopping bitmask for IEs to include */
    dReq.includeFhIEs = Util_parseUint32(pBuf);
    pBuf += 4;

    /* FHIE is overloaded to timestamp for green power */
    if(ApiMac_mtDataReqTxOptionGp == true)
    {
        /*timestamps are in units of 10us, but gpOffset should be in units of milliseconds */
        uint8_t conversionFactor = 100;

        if(dReq.txOptions.useAbsoluteTime)
        {
            uint32_t currTime = MAP_ICall_getTicks();
            uint32_t desiredTime = dReq.includeFhIEs;
            /* Even if one overflows it'll still calculate the difference properly */
            int32_t desiredOffset = desiredTime - currTime;

            if (desiredOffset > 0)
            {
                if (desiredOffset <= UINT8_MAX * conversionFactor)
                {
                    /* set the offset to milliseconds and always round up */
                    dReq.gpOffset = (desiredOffset + (conversionFactor - 1)) / conversionFactor;
                }
                else
                {
                    /* Requesting to wait more than 255ms, which isn't possible with an 8 byte offset field */
                    sendSRSP(MT_MAC_DATA_REQ, ApiMac_status_onTimeTooLong);
                    return;
                }
            }
            else
            {
                /* Message took to long to get here, can't send packet out in time */
                sendSRSP(MT_MAC_DATA_REQ, ApiMac_status_noTime);
                return;
            }
        }
        else
        {
            /* Relative Time */
            if(dReq.includeFhIEs)
            {
                uint32_t desiredOffset = dReq.includeFhIEs;
                if (desiredOffset < conversionFactor)
                {
                    /* Not long enough delay, don't want the user to think they can delay <1ms*/
                    sendSRSP(MT_MAC_DATA_REQ, ApiMac_status_noTime);
                    return;
                }
                else if (desiredOffset > UINT8_MAX * conversionFactor)
                {
                    /* Too long of a delay, can't delay that long */
                    sendSRSP(MT_MAC_DATA_REQ, ApiMac_status_onTimeTooLong);
                    return;
                }
                /* convert and round up */
                dReq.gpOffset = (dReq.includeFhIEs + (conversionFactor - 1)) / conversionFactor;
            }
            else
            {
                /* if not set in the request, use the default */
                dReq.gpOffset = GP_Offset;
            }
        }

        /* After doing everything else, reset includeFhIEs back to 0 since we don't
         want anything later on to think that we're actually using those options
         instead of just overloading the field */
        dReq.includeFhIEs = 0;
    }

    /* MSDU length */
    dReq.msdu.len = Util_parseUint16(pBuf);
    dLen = dReq.msdu.len;
    pBuf += 2;

    /* Payload IE length */
    dReq.payloadIELen = Util_parseUint16(pBuf);
    dLen += dReq.payloadIELen;
    pBuf += 2;

    /* MSDU - just put MT buffer pointer in ApiMac structure */
    dReq.msdu.p = pBuf;
    pBuf += dReq.msdu.len;

    /* IE List - just put MT buffer pointer in ApiMac structure */
    dReq.pIEList = pBuf;

    dReq.gpDuration = GP_DURATION;

    if((pMpb->length == (sizeof(MtPkt_dataReq_t) + dLen)) ||
       ( ApiMac_mtDataReqTxOptionGp && (pMpb->length == (sizeof(MtPkt_dataReq_t) + dLen + 1))))
    {
        /* Send request to the MAC task */
        status = ApiMac_mcpsDataReq(&dReq);
    }
    else
    {
        /* Invalid incoming message length */
        status = ApiMac_status_lengthError;
    }

    /* Send host a response */
    sendSRSP(MT_MAC_DATA_REQ, status);
}

/*!
 * @brief   Process MAC_DELETE_ALLDEVS_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macDeleteAllDevicesReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == 0)
    {
#ifdef FEATURE_MAC_SECURITY
        /* Send request to the MAC task */
        status = ApiMac_secDeleteAllDevices();
#endif
    }

    /* Send host a response */
    sendSRSP(MT_MAC_DELETE_ALL_REQ, status);
}

/*!
 * @brief   Process MAC_DELETE_DEVICE_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macDeleteDeviceReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_delDevReq_t))
    {
#ifdef FEATURE_MAC_SECURITY
        uint8_t *pBuf = pMpb->pData;

        /* Send request to the MAC task */
        status = ApiMac_secDeleteDevice((ApiMac_sAddrExt_t *)pBuf);
#endif
    }

    /* Send host a response */
    sendSRSP(MT_MAC_DELETE_DEVICE_REQ, status);
}

/*!
 * @brief   Process MAC_DELETE_KEY_ID_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macDeleteKeyReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_delKeyReq_t))
    {
#ifdef FEATURE_MAC_SECURITY
        uint8_t *pBuf = pMpb->pData;

        /* Send request to the MAC task */
        status = ApiMac_secDeleteKeyAndAssocDevices(*pBuf);
#endif
    }

    /* Send host a response */
    sendSRSP(MT_MAC_DELETE_KEY_REQ, status);
}

/*!
 * @brief   Process MAC_DISASSOCIATE_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macDisassociateReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_disAssReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_mlmeDisassociateReq_t dReq;

        /* Coordinator address mode */
        dReq.deviceAddress.addrMode = (ApiMac_addrType_t)*pBuf++;

        /* Coordinator address */
        macSbaToAdr(&dReq.deviceAddress, pBuf);
        pBuf += APIMAC_SADDR_EXT_LEN;

        /* Coordinator Pan ID */
        dReq.devicePanId = Util_parseUint16(pBuf);
        pBuf += 2;

        /* Disassociate reason */
        dReq.disassociateReason = (ApiMac_disassocateReason_t)*pBuf++;

        /* TxIndirect */
        dReq.txIndirect = *pBuf++;

        /* Security parameters */
        macSbaToSec(&dReq.sec, pBuf);

        /* Send request to the MAC task */
        status = ApiMac_mlmeDisassociateReq(&dReq);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_DISASSOCIATE_REQ, status);
}

/*!
 * @brief   Process MAC_FH_ENABLE_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macEnableFhReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == 0)
    {
        /* Try to enable frequency hopping */
        status = ApiMac_enableFH();
    }

    /* Send host a response */
    sendSRSP(MT_MAC_FH_ENABLE_REQ, status);
}

/*!
 * @brief   Process MAC_GET_REQ command issued by host
 *
 * @param   pBuf - pointer to incoming parameter buffer
 */
static void macGetReq(Mt_mpb_t *pMpb)
{
    uint16_t len = 0;
    uint8_t* pRsp;
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_getReq_t))
    {
        ApiMac_attribute_array_t attr;

        /* 8-bit MAC PIB attribute ID */
        attr = *(ApiMac_attribute_array_t*)pMpb->pData;

        /* special case for beacon payload */
        if(ApiMac_attribute_beaconPayload == attr)
        {
            /* Get the correct length value */
            ApiMac_mlmeGetReqUint8(ApiMac_attribute_beaconPayloadLength,
                                   (uint8_t*)&len);
        }
        else
        {
            /* set our length to the maximum buffer size */
            len = MAX_PARAM_BUF_SIZE_MAC;
        }

        if(len > 0)
        {
            /* allocate memory */
            pRsp = MAP_ICall_malloc(1 + len);

            /* send get request */
            if(NULL != pRsp)
            {
                /* Clear allocated memory */
                memset(pRsp, 0, 1 + len);

                if(ApiMac_attribute_beaconPayload == attr)
                {
                  uint32_t pValue;
                  uint16_t len2;
                  status = ApiMac_mlmeGetReqArrayLen(attr, (uint8_t*)&pValue,
                                                     &len2);
                  memcpy(&pRsp[1], (uint8_t*)pValue, len);
                }
                else
                {
                  status = ApiMac_mlmeGetReqArrayLen(attr, &pRsp[1], &len);
                }

                /* Status code in first byte of response */
                pRsp[0] = status;
                /* If error, don't send back any data */
                len = (status != ApiMac_status_success) ? 1 : len + 1;

                /* Send host a response */
                sendDRSP(MT_MAC_GET_REQ, len, pRsp);

                MAP_ICall_free(pRsp);
                return;
            }
            else
            {
                status = ApiMac_status_noResources;
            }
        }
    }

    /* If error, don't send any data */
    len = (status != ApiMac_status_success) ? 1 : len + 1;

    /* Send host a response */
    sendDRSP(MT_MAC_GET_REQ, len, &status);
}

/*!
 * @brief   Process MAC_FH_GET_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macGetFhReq(Mt_mpb_t *pMpb)
{
    uint16_t len = 0;
    uint8_t rsp[1 + MAX_PARAM_SIZE_FH];
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_getFhReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_FHAttribute_array_t attr;

        /* 16-bit frequency hopping PIB attribute ID */
        attr = (ApiMac_FHAttribute_array_t)Util_parseUint16(pBuf);

        /* Send request to the MAC task */
        status = ApiMac_mlmeGetFhReqArrayLen(attr, &rsp[1], &len);
    }

    /* Status code in first byte of response */
    rsp[0] = status;

    /* If error, don't send any data */
    len = (status != ApiMac_status_success) ? 1 : len + 1;

    /* Send host a response */
    sendDRSP(MT_MAC_FH_GET_REQ, len, rsp);
}

#ifdef FEATURE_MAC_SECURITY
#ifdef SEC_API_256
/*!
 * @brief   Process MAC_SECURITY_GET_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macGetSecReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_getSecReq_t))
    {
        uint16_t sLen;
        uint8_t *pBuf = pMpb->pData;
        uint8_t attr;

        status = ApiMac_status_noResources;

        /* 8-bit security PIB attribute ID */
        attr = pBuf[0];

        switch(attr)
        {
            /* These security PIB items are Write-Only */
            case MT_MAC_PIB_DEVICE_TABLE:
            case MT_MAC_PIB_SECURITY_LEVEL_TABLE:
            case ApiMac_securityAttribute_keyTable:
                status = ApiMac_status_unsupportedAttribute;
                break;

            case ApiMac_securityAttribute_keyIdLookupEntry:
                sLen = sizeof(ApiMac_securityPibKeyIdLookupEntry_t);
                break;
            case ApiMac_securityAttribute_keyDeviceEntry:
                sLen = sizeof(ApiMac_securityPibKeyDeviceEntry_t);
                break;
            case ApiMac_securityAttribute_keyUsageEntry:
                sLen = sizeof(ApiMac_securityPibKeyUsageEntry_t);
                break;
            case ApiMac_securityAttribute_keyEntry:
                sLen = sizeof(ApiMac_securityPibKeyEntry_t);
                break;
            case ApiMac_securityAttribute_deviceEntry:
                sLen = sizeof(ApiMac_securityPibDeviceEntry_t);
                break;
            case ApiMac_securityAttribute_securityLevelEntry:
                sLen = sizeof(ApiMac_securityPibSecurityLevelEntry_t);
                break;
            default:
                sLen = MAX_PARAM_SIZE_SEC;
                break;
        }

        if(status == ApiMac_status_noResources)
        {
            uint8_t *pGet = ICall_malloc(sLen);

            if(pGet != NULL)
            {
                uint16_t gLen;

                switch(attr)
                {
                    case ApiMac_securityAttribute_keyIdLookupEntry:
                    case ApiMac_securityAttribute_keyDeviceEntry:
                    case ApiMac_securityAttribute_keyUsageEntry:
                        pGet[1] = pBuf[2];
                        /* Intentionally falling through */
                    case ApiMac_securityAttribute_keyEntry:
                    case ApiMac_securityAttribute_securityLevelEntry:
                        pGet[0] = pBuf[1];
                        break;
                    case ApiMac_securityAttribute_deviceEntry:
                        pGet[0] = pBuf[1];
                        ((ApiMac_securityPibDeviceEntry_t *)pGet)->deviceEntry.frameCntr = ICall_malloc(sizeof(ApiMac_frameCntr_t)*API_MAX_NODE_KEY_ENTRIES);
                        break;
                }
                /* Send request to the MAC task */
                status = ApiMac_mlmeGetSecurityReqArrayLen(
                         (ApiMac_securityAttribute_array_t)attr, pGet, &gLen);
                if(status == ApiMac_status_success)
                {
                    uint8_t *pRsp = ICall_malloc(1 + gLen);

                    if(pRsp != NULL)
                    {
                        uint8_t rLen;
                        uint8_t *pTmp = pRsp;

                        /* Start with parameter read status */
                        *pTmp++ = status;

                        /* Handle specific structures/data types */
                        switch(attr)
                        {
                            case ApiMac_securityAttribute_keyIdLookupEntry:
                                rLen = bufferKeyLookup(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_keyDeviceEntry:
                                rLen = bufferKeyDevice(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_keyUsageEntry:
                                rLen = bufferKeyUsage(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_keyEntry:
                                rLen = bufferKeyEntry(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_deviceEntry:
                                rLen = bufferDeviceEntry(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_securityLevelEntry:
                                rLen = bufferLevelEntry(pTmp, pGet);
                                break;
                            default:
                                rLen = bufferMiscParam(pTmp, pGet, gLen);
                                break;
                        }

                        /* Send back the requested parameter(s) */
                        sendDRSP(MT_MAC_SECURITY_GET_REQ, rLen + 1, pRsp);
                        /* Done with the 'rsp' buffer */
                        ICall_free(pRsp);
                    }
                    else
                    {
                        /* Not enough memory for response buffer */
                        status = ApiMac_status_noResources;
                    }
                }

                /* Done with the 'get' buffer */
                if(attr == ApiMac_securityAttribute_deviceEntry)
                {
                    if(((ApiMac_securityPibDeviceEntry_t *)pGet)->deviceEntry.frameCntr != NULL)
                    {
                        ICall_free(((ApiMac_securityPibDeviceEntry_t *)pGet)->deviceEntry.frameCntr);
                    }
                }
                ICall_free(pGet);
            }
        }
    }

    if(status != ApiMac_status_success)
    {
        /* In case there was an error, just send that */
        sendSRSP(MT_MAC_SECURITY_GET_REQ, status);
    }
}
#else
/*!
 * @brief   Process MAC_SECURITY_GET_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macGetSecReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_getSecReq_t))
    {
        uint16_t sLen, gLen;
        uint8_t *pBuf = pMpb->pData;
        uint8_t attr;

        status = ApiMac_status_noResources;

        /* 8-bit security PIB attribute ID */
        attr = pBuf[0];

        switch(attr)
        {
            /* These security PIB items are Write-Only */
            case MT_MAC_PIB_DEVICE_TABLE:
            case MT_MAC_PIB_SECURITY_LEVEL_TABLE:
            case ApiMac_securityAttribute_keyTable:
                status = ApiMac_status_unsupportedAttribute;
                break;

            case ApiMac_securityAttribute_keyIdLookupEntry:
                sLen = sizeof(ApiMac_securityPibKeyIdLookupEntry_t);
                gLen = sizeof(MtPkt_secKeyLookup_t);
                break;
            case ApiMac_securityAttribute_keyDeviceEntry:
                sLen = sizeof(ApiMac_securityPibKeyDeviceEntry_t);
                gLen = sizeof(MtPkt_secKeyDevice_t);
                break;
            case ApiMac_securityAttribute_keyUsageEntry:
                sLen = sizeof(ApiMac_securityPibKeyUsageEntry_t);
                gLen = sizeof(MtPkt_secKeyUsage_t);
                break;
            case ApiMac_securityAttribute_keyEntry:
                sLen = sizeof(ApiMac_securityPibKeyEntry_t);
                gLen = sizeof(MtPkt_secKeyEntry_t);
                break;
            case ApiMac_securityAttribute_deviceEntry:
                sLen = sizeof(ApiMac_securityPibDeviceEntry_t);
                gLen = sizeof(MtPkt_secDevEntry_t);
                break;
            case ApiMac_securityAttribute_securityLevelEntry:
                sLen = sizeof(ApiMac_securityPibSecurityLevelEntry_t);
                gLen = sizeof(MtPkt_secLevelEntry_t);
                break;
            default:
                sLen = gLen = MAX_PARAM_SIZE_SEC;
                break;
        }

        if(status == ApiMac_status_noResources)
        {
            uint8_t *pGet = MAP_ICall_malloc(sLen);

            if(pGet != NULL)
            {
                switch(attr)
                {
                    case ApiMac_securityAttribute_keyIdLookupEntry:
                    case ApiMac_securityAttribute_keyDeviceEntry:
                    case ApiMac_securityAttribute_keyUsageEntry:
                        pGet[2] = pBuf[3];
                        pGet[3] = pBuf[4];
                        /* Intentionally falling through */
                    case ApiMac_securityAttribute_keyEntry:
                    case ApiMac_securityAttribute_securityLevelEntry:
                        pGet[0] = pBuf[1];
                        pGet[1] = pBuf[2];
                        break;
                    case ApiMac_securityAttribute_deviceEntry:
                        pGet[0] = pBuf[1];
                        pGet[1] = pBuf[2];
                        ((ApiMac_securityPibDeviceEntry_t *)pGet)->deviceEntry.frameCntr = \
                                MAP_ICall_malloc(sizeof(ApiMac_frameCntr_t)*API_MAX_NODE_KEY_ENTRIES);
                        break;
                }
                /* Send request to the MAC task */
                status = ApiMac_mlmeGetSecurityReqArrayLen(
                         (ApiMac_securityAttribute_array_t)attr, pGet, &sLen);
                if(status == ApiMac_status_success)
                {
                    uint8_t *pRsp = MAP_ICall_malloc(1 + gLen);

                    if(pRsp != NULL)
                    {
                        uint8_t rLen;
                        uint8_t *pTmp = pRsp;

                        /* Start with parameter read status */
                        *pTmp++ = status;

                        /* Handle specific structures/data types */
                        switch(attr)
                        {
                            case ApiMac_securityAttribute_keyIdLookupEntry:
                                rLen = bufferKeyLookup(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_keyDeviceEntry:
                                rLen = bufferKeyDevice(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_keyUsageEntry:
                                rLen = bufferKeyUsage(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_keyEntry:
                                rLen = bufferKeyEntry(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_deviceEntry:
                                rLen = bufferDeviceEntry(pTmp, pGet);
                                break;
                            case ApiMac_securityAttribute_securityLevelEntry:
                                rLen = bufferLevelEntry(pTmp, pGet);
                                break;
                            default:
                                rLen = bufferMiscParam(pTmp, pGet, gLen);
                                break;
                        }

                        /* Send back the requested parameter(s) */
                        sendDRSP(MT_MAC_SECURITY_GET_REQ, rLen + 1, pRsp);
                        /* Done with the 'rsp' buffer */
                        MAP_ICall_free(pRsp);
                    }
                    else
                    {
                        /* Not enough memory for response buffer */
                        status = ApiMac_status_noResources;
                    }
                }
                /* Done with the 'get' buffer */
                if(attr == ApiMac_securityAttribute_deviceEntry)
                {
                    if(((ApiMac_securityPibDeviceEntry_t *)pGet)->deviceEntry.frameCntr != NULL)
                    {
                        MAP_ICall_free(((ApiMac_securityPibDeviceEntry_t *)pGet)->deviceEntry.frameCntr);
                    }
                }

                MAP_ICall_free(pGet);
            }
        }
    }

    if(status != ApiMac_status_success)
    {
        /* In case there was an error, just send that */
        sendSRSP(MT_MAC_SECURITY_GET_REQ, status);
    }
}
#endif
#endif /*FEATURE_SECURITY */

/*!
 * @brief   Process MAC_INIT_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macInitReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_success;

    if(pMpb->length != 0)
    {
        status = ApiMac_status_lengthError;
    }

    /* Send host a response */
    sendSRSP(MT_MAC_INIT_REQ, status);
}

/*!
 * @brief   Process MAC_ORPAN_RSP command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macOrphanRsp(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_orphanRsp_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_mlmeOrphanRsp_t oRsp;

        /* Extended address of requesting device */
        (void)copyExtAdr(oRsp.orphanAddress, pBuf);
        pBuf += APIMAC_SADDR_EXT_LEN;

        /* Short address allocated to the associated device */
        oRsp.shortAddress = Util_parseUint16(pBuf);
        pBuf += 2;

        /* Associated to this coordinator indicator */
        oRsp.associatedMember = *pBuf++;

        /* Security parameters */
        macSbaToSec(&oRsp.sec, pBuf);

        /* Send request to the MAC task */
        status = ApiMac_mlmeOrphanRsp(&oRsp);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_ORPHAN_RSP, status);
}

/*!
 * @brief   Process MAC_POLL_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macPollReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_pollReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_mlmePollReq_t pReq;

        /* Coordinator address mode */
        pReq.coordAddress.addrMode = (ApiMac_addrType_t)*pBuf++;

        /* Coordinator address */
        macSbaToAdr(&pReq.coordAddress, pBuf);
        pBuf += APIMAC_SADDR_EXT_LEN;

        /* Coordinator Pan ID */
        pReq.coordPanId = Util_parseUint16(pBuf);
        pBuf += 2;

        /* Security parameters */
        macSbaToSec(&pReq.sec, pBuf);

        /* Send request to the MAC task */
        status = ApiMac_mlmePollReq(&pReq);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_POLL_REQ, status);
}

/*!
 * @brief   Process MAC_PURGE_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macPurgeReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_purgeReq_t))
    {
        uint8_t *pBuf = pMpb->pData;

        /* Send request to the MAC task */
        status = ApiMac_mcpsPurgeReq(*pBuf);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_PURGE_REQ, status);
}

/*!
 * @brief   Process MAC_READ_KEY_ID_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macReadKeyReq(Mt_mpb_t *pMpb)
{
#ifdef FEATURE_MAC_SECURITY
    uint32_t frameCounter = 0x00000000;

    uint8_t rsp[1 + sizeof(frameCounter)];
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_rdKeyReq_t))
    {
        uint8_t *pBuf = pMpb->pData;

        /* Send request to the MAC task */
        status = ApiMac_secGetDefaultSourceKey(*pBuf, &frameCounter);

        (void)Util_bufferUint32(&rsp[1], frameCounter);
    }

    /* Status code in first byte of response */
    rsp[0] = status;

    /* Send host a response */
    sendDRSP(MT_MAC_READ_KEY_REQ, sizeof(rsp), rsp);
#endif
}

/*!
 * @brief   Process MAC_RESET_REQ command issued by host
 *          There is no need to have an async response to this
 *          command, because SRSP is sent after the reset sequence
 *          is executed. This MAC API is executed in the app task
 *          context itself
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macResetReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_resetReq_t))
    {
        uint8_t *pBuf = pMpb->pData;

        /* Send request to the MAC task */
        status = ApiMac_mlmeResetReq(*pBuf);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_RESET_REQ, status);
}

/*!
 * @brief   Process MAC_SCAN_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macScanReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_scanReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_mlmeScanReq_t sReq;

        /* Scan type */
        sReq.scanType = (ApiMac_scantype_t)*pBuf++;

        /* Scan duration */
        sReq.scanDuration = *pBuf++;

        /* Channel page */
        sReq.channelPage = *pBuf++;

        /* PHY ID */
        sReq.phyID = *pBuf++;

        /* Max scan results */
        sReq.maxResults = *pBuf++;
        if((sReq.maxResults > MAX_ACTIVE_SCAN_RESULTS) &&
           (sReq.scanType != ApiMac_scantype_orphan) &&
           (sReq.scanType != ApiMac_scantype_energyDetect))
        {
            /* Limit results list size for active scans */
            sReq.maxResults = MAX_ACTIVE_SCAN_RESULTS;
        }

        /* Permit join indicator */
        sReq.permitJoining = *pBuf++;

        /* Enhanced Beacon min link quality */
        sReq.linkQuality = *pBuf++;

        /* Enhanced Beacon random filter percentage */
        sReq.percentFilter = *pBuf++;

        /* MPM scan mode */
        sReq.MPMScan = *pBuf++;

        /* MPM scan type */
        sReq.MPMScanType = *pBuf++;

        /* MPM scan duration */
        sReq.MPMScanDuration = Util_parseUint16(pBuf);
        pBuf += 2;

        /* Security parameters */
        macSbaToSec(&sReq.sec, pBuf);
        pBuf += MT_MAC_LEN_SECINFO;

        /* Channel bitmask array */
        memcpy(&sReq.scanChannels, pBuf, APIMAC_154G_CHANNEL_BITMAP_SIZ);

        /* Send request to the MAC task */
        status = ApiMac_mlmeScanReq(&sReq);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_SCAN_REQ, status);
}

/*!
 * @brief   Process MAC_SET_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macSetReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length >= sizeof(MtPkt_setReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_attribute_array_t attr;

        /* 8-bit MAC PIB attribute ID */
        attr = (ApiMac_attribute_array_t)*pBuf++;

        /* Send request to the MAC task */
        status = ApiMac_mlmeSetReqArray(attr, pBuf);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_SET_REQ, status);
}

/*!
 * @brief   Process MAC_FH_SET_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macSetFhReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length >= sizeof(MtPkt_setFhReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_FHAttribute_array_t attr;

        /* 16-bit frequency hopping PIB attribute ID */
        attr = (ApiMac_FHAttribute_array_t)Util_parseUint16(pBuf);

        /* Send request to the MAC task */
        status = ApiMac_mlmeSetFhReqArray(attr, &pBuf[2]);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_FH_SET_REQ, status);
}

/*!
 * @brief   Process MAC_UPDATE_PAN_ID_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macSetPanIdReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_panidReq_t))
    {
        uint16_t panId;
        uint8_t *pBuf = pMpb->pData;

        /* New Pan ID */
        panId = Util_parseUint16(pBuf);

        /* Send request to the MAC task */
        status = ApiMac_updatePanId(panId);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_UPDATE_PANID_REQ, status);
}

/*!
 * @brief   Process MT_MAC_SRC_MATCH_EN_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void macSrcMatchEnable(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

#if defined(COMBO_MAC) || defined(FREQ_2_4G)
    if(pMpb->length == 0)
    {
        /* Send request to the MAC task */
        status = ApiMac_srcMatchEnable();
    }
#else
    status = ApiMac_status_commandIDError;
#endif
    /* Send host a response */
    sendSRSP(MT_MAC_SRC_MATCH_EN_REQ, status);
}

#ifdef FEATURE_MAC_SECURITY
/*!
 * @brief   Process MAC_SECURITY_SET_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming parameter block
 */
static void macSetSecReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length >= sizeof(MtPkt_setSecReq_t))
    {
        uint8_t attr;

        /* 8-bit security PIB attribute ID */
        attr = *(uint8_t*)pMpb->pData;

        switch(attr)
        {
            case MT_MAC_PIB_DEVICE_TABLE:
            case MT_MAC_PIB_SECURITY_LEVEL_TABLE:
                status = ApiMac_status_unsupportedAttribute;
                break;

            /* These security PIB items need 'special handling' */
            case ApiMac_securityAttribute_keyTable:
                status = buildKeyTable(pMpb);
                break;
            case ApiMac_securityAttribute_keyIdLookupEntry:
                status = buildKeyLookup(pMpb);
                break;
            case ApiMac_securityAttribute_keyDeviceEntry:
                status = buildKeyDevice(pMpb);
                break;
            case ApiMac_securityAttribute_keyUsageEntry:
                status = buildKeyUsage(pMpb);
                break;
            case ApiMac_securityAttribute_keyEntry:
                status = buildKeyEntry(pMpb);
                break;
            case ApiMac_securityAttribute_deviceEntry:
                status = buildDeviceEntry(pMpb);
                break;
            case ApiMac_securityAttribute_securityLevelEntry:
                status = buildLevelEntry(pMpb);
                break;

            /* Other Security PIB items are single parameters */
            default:
                status = buildSecParam(attr, pMpb);
                break;
        }

        if(status == ApiMac_status_success)
        {
            /* Ptr to parameter data structure */
            uint8_t *sBuf = (uint8_t*)pMpb->pData;

            /* Send request to the MAC task */
           status = ApiMac_mlmeSetSecurityReqArray(
                    (ApiMac_securityAttribute_array_t)attr, sBuf);

           if(sBuf != NULL)
           {
               /* Done with allocated structure */
               if(attr == ApiMac_securityAttribute_deviceEntry)
               {
                   if(((ApiMac_securityPibDeviceEntry_t *)pMpb->pData)->deviceEntry.frameCntr != NULL)
                   {
                       MAP_ICall_free(((ApiMac_securityPibDeviceEntry_t *)pMpb->pData)->deviceEntry.frameCntr);
                   }
               }
               MAP_ICall_free(sBuf);
           }

        }
    }

    /* Send host a response */
    sendSRSP(MT_MAC_SECURITY_SET_REQ, status);
}
#endif

/*!
 * @brief   Process MAC_SET_RX_GAIN_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming parameter block
 */
static void macSetRxGainReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_rxgainReq_t))
    {
#ifdef HAL_PA_LNA
        uint8_t *pBuf = pMpb->pData;

        if(*pBuf)
        {
            /* Enable high-gain mode */
            HAL_PA_LNA_RX_HGM();
        }
        else
        {
            /* Enable low-gain mode */
            HAL_PA_LNA_RX_LGM();
        }
        status = ApiMac_status_success;
#else
        /* PA/LNA control not available */
        status = ApiMac_status_commandIDError;
#endif
    }

    /* Send host a response */
    sendSRSP(MT_MAC_SET_RX_GAIN_REQ, status);
}

/*!
 * @brief   Process MAC_START_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming parameter block
 */
static void macStartReq(Mt_mpb_t *pMpb)
{
    uint8_t status;
    uint8_t *pBuf = pMpb->pData;
    ApiMac_mlmeStartReq_t sReq;

    /* Beacon TX start time */
    sReq.startTime = Util_parseUint32(pBuf);
    pBuf += 4;

    /* Coordinator Pan ID */
    sReq.panId = Util_parseUint16(pBuf);
    pBuf += 2;

    /* Logical Channel */
    sReq.logicalChannel = *pBuf++;

    /* Channel Page */
    sReq.channelPage = *pBuf++;

    /* PHY ID */
    sReq.phyID = *pBuf++;

    /* Beacon order */
    sReq.beaconOrder = *pBuf++;

    /* Superframe order */
    sReq.superframeOrder = *pBuf++;

    /* PAN coordinator indicator */
    sReq.panCoordinator = *pBuf++;

    /* Battery life extension indicator */
    sReq.batteryLifeExt = *pBuf++;

    /* Coordinator realignment TX indicator */
    sReq.coordRealignment = *pBuf++;

    /* Realignment frame security parameters */
    macSbaToSec(&sReq.realignSec, pBuf);
    pBuf += MT_MAC_LEN_SECINFO;

    /* Beacon frame security parameters */
    macSbaToSec(&sReq.beaconSec, pBuf);
    pBuf += MT_MAC_LEN_SECINFO;

    /* Frequency hopping enable indicator */
    sReq.startFH = *pBuf++;

    /* Enhanced beacon interval exponent */
    sReq.mpmParams.eBeaconOrder = *pBuf++;

    /* Enhanced beacon time offset */
    sReq.mpmParams.offsetTimeSlot = *pBuf++;

    /* Enhanced beacon interval for non-beacon PAN */
    sReq.mpmParams.NBPANEBeaconOrder = Util_parseUint16(pBuf);
    pBuf += 2;

    /* Numbers of IEs */
    sReq.mpmParams.numIEs = *pBuf++;

    /* IE List - just put MT bufffer pointer in ApiMac structure */
    sReq.mpmParams.pIEIDs = pBuf;

    if(pMpb->length == (sizeof(MtPkt_startReq_t) + sReq.mpmParams.numIEs))
    {
        /* Send request to the MAC task */
        status = ApiMac_mlmeStartReq(&sReq);
    }
    else
    {
        /* Invalid incoming message length */
        status = ApiMac_status_lengthError;
    }

    /* Send host a response */
    sendSRSP(MT_MAC_START_REQ, status);
}

/*!
 * @brief   Process MAC_FH_START_REQ command issued by host
  *
 * @param   pMpb - pointer to incoming parameter block
 */
static void macStartFhReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == 0)
    {
        /* Try to start frequency hopping */
        status = ApiMac_startFH();
    }

    /* Send host a response */
    sendSRSP(MT_MAC_FH_START_REQ, status);
}

/*!
 * @brief   Process MAC_SYNC_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming parameter block
 */
static void macSyncReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_syncReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_mlmeSyncReq_t sReq;

        /* LogicalChannel */
        sReq.logicalChannel = *pBuf++;

        /* ChannelPage */
        sReq.channelPage = *pBuf++;

        /* TrackBeacon */
        sReq.trackBeacon = *pBuf++;

        /* PHY ID */
        sReq.phyID = *pBuf;

        /* Send request to the MAC task */
        status = ApiMac_mlmeSyncReq(&sReq);
    }

    /* Send host a response */
    sendSRSP(MT_MAC_SYNC_REQ, status);
}

/*!
 * @brief   Process MAC_WRITE_KEY_ID_REQ command issued by host
 *
 * @param   pMpb - pointer to incoming parameter block
 */
static void macWriteKeyReq(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_wrKeyReq_t))
    {
        uint8_t *pBuf = pMpb->pData;
        ApiMac_secAddKeyInitFrameCounter_t wReq;

        /* Duplicate key table entries indicator */
        wReq.newKeyFlag = *pBuf++;
#ifndef SEC_API_256
        /* Index into the mac security key table */
        wReq.replaceKeyIndex = Util_parseUint16(pBuf);
        pBuf += 2;
#else
        /* Index into the mac security key table */
        wReq.replaceKeyIndex = *pBuf++;
#endif
        /* Key data */
        memcpy(&wReq.key, pBuf, APIMAC_KEY_MAX_LEN);
        pBuf += APIMAC_KEY_MAX_LEN;

        /* Frame counter */
        wReq.frameCounter = Util_parseUint32(pBuf);
        pBuf += 4;

        /* Key ID lookup data size indicator, 0=5 bytes, 1=9 bytes */
        wReq.lookupDataSize = *pBuf++;

        /* Key ID lookup data */
        memcpy(&wReq.lookupData, pBuf, APIMAC_MAX_KEY_LOOKUP_LEN);
#ifdef FEATURE_MAC_SECURITY
        /* Send request to the MAC task */
        status = ApiMac_secAddKeyInitFrameCounter(&wReq);
#endif
    }

    /* Send host a response */
    sendSRSP(MT_MAC_WRITE_KEY_REQ, status);
}

/******************************************************************************
 Public API Callback Functions
 *****************************************************************************/
/*!
 * Process MAC_ASSOCIATE_CNF callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_AssociateCnf(ApiMac_mlmeAssociateCnf_t *pCnf)
{
    uint8_t rspBuf[MT_MAC_LEN_ASSOC_CNF];
    uint8_t *pBuf = rspBuf;

    /* Status */
    *pBuf++ = pCnf->status;

    /* Short address */
    pBuf = Util_bufferUint16(pBuf, pCnf->assocShortAddress);

    /* Security */
    macSecToSba(pBuf, &pCnf->sec);

    /* Send host a response */
    sendCRSP(MT_MAC_ASSOCIATE_CNF, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_ASSOCIATE_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_AssociateInd(ApiMac_mlmeAssociateInd_t *pInd)
{
    uint8_t rspBuf[MT_MAC_LEN_ASSOC_IND];
    uint8_t *pBuf = rspBuf;

    /* Extended address */
    pBuf = copyExtAdr(pBuf, pInd->deviceAddress);

    /* Capabilities */
    *pBuf++ = ApiMac_convertCapabilityInfo(&pInd->capabilityInformation);

    /* Security */
    macSecToSba(pBuf, &pInd->sec);

    /* Send host a response */
    sendCRSP(MT_MAC_ASSOCIATE_IND, sizeof(rspBuf), rspBuf);
}

/*!
 * Proess MAC_ASYNC_CNF callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_AsyncCnf(ApiMac_mlmeWsAsyncCnf_t *pCnf)
{
    uint8_t rspBuf[MT_MAC_LEN_ASYNC_CNF];
    uint8_t *pBuf = rspBuf;

    /* Status */
    *pBuf = pCnf->status;

    /* Send host a response */
    sendCRSP(MT_MAC_ASYNC_CNF, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_ASYNC_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_AsyncInd(ApiMac_mlmeWsAsyncInd_t *pInd)
{
    /* Hand over to shared DATA_IND processor */
    dataInd(MT_MAC_ASYNC_IND, pInd);
}

/*!
 * Process MAC_BEACON_NOTIFY_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_BeaconNotifyInd(ApiMac_mlmeBeaconNotifyInd_t *pInd)
{
    if(pInd->beaconType == ApiMac_beaconType_enhanced)
    {
        eBeaconInd(pInd);
    }
    else if(pInd->beaconType == ApiMac_beaconType_normal)
    {
        nBeaconInd(pInd);
    }
}

/*!
 * Process MAC_COMM_STATUS_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_CommStatusInd(ApiMac_mlmeCommStatusInd_t *pInd)
{
    uint8_t rspBuf[MT_MAC_LEN_COMMST_IND];
    uint8_t *pBuf = rspBuf;

    /* Status */
    *pBuf++ = pInd->status;

    /* Source address */
    pBuf = macAdrToSba(pBuf, &pInd->srcAddr);

    /* Destination address */
    pBuf = macAdrToSba(pBuf, &pInd->dstAddr);

    /* PAN ID */
    pBuf = Util_bufferUint16(pBuf, pInd->panId);

    /* Reason */
    *pBuf++ = pInd->reason;

    /* Security */
    macSecToSba(pBuf, &pInd->sec);

    /* Send host a response */
    sendCRSP(MT_MAC_COMM_STATUS_IND, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_DATA_CNF callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_DataCnf(ApiMac_mcpsDataCnf_t *pCnf)
{
    uint8_t rspBuf[MT_MAC_LEN_DATA_CNF];
    uint8_t *pBuf = rspBuf;

    /* Status */
    *pBuf++ = pCnf->status;

    /* MSDU handle */
    *pBuf++ = pCnf->msduHandle;

    /* Timestamp (backoffs) */
    pBuf = Util_bufferUint32(pBuf, pCnf->timestamp);

    /* Timestamp2 (MAC units) */
    pBuf = Util_bufferUint16(pBuf, pCnf->timestamp2);

    /* Retries */
    *pBuf++ = pCnf->retries;

    /* Link quality */
    *pBuf++ = pCnf->mpduLinkQuality;

    /* Correlation */
    *pBuf++ = pCnf->correlation;

    /* RSSI */
    *pBuf++ = pCnf->rssi;

    /* Frame counter */
    (void)Util_bufferUint32(pBuf, pCnf->frameCntr);

    /* Send host a response */
    sendCRSP(MT_MAC_DATA_CNF, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_DATA_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_DataInd(ApiMac_mlmeWsAsyncInd_t *pInd)
{
    /* Hand over to shared DATA_IND processor */
    dataInd(MT_MAC_DATA_IND, pInd);
}

/*!
 * Process MAC_DISASSOCIATE_CNF callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_DisassociateCnf(ApiMac_mlmeDisassociateCnf_t *pCnf)
{
    uint8_t rspBuf[MT_MAC_LEN_DISASSOC_CNF];
    uint8_t *pBuf = rspBuf;

    /* Status */
    *pBuf++ = pCnf->status;

    /* Extended address */
    pBuf = macAdrToSba(pBuf, &pCnf->deviceAddress);

    /* PAN ID */
    (void)Util_bufferUint16(pBuf, pCnf->panId);

    /* Send host a response */
    sendCRSP(MT_MAC_DISASSOCIATE_CNF, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_DISASSOCIATE_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_DisassociateInd(ApiMac_mlmeDisassociateInd_t *pInd)
{
    uint8_t rspBuf[MT_MAC_LEN_DISASSOC_IND];
    uint8_t *pBuf = rspBuf;

    /* Extended address */
    pBuf = copyExtAdr(pBuf, pInd->deviceAddress);

    /* Reason */
    *pBuf++ = pInd->disassociateReason;

    /* Security */
    macSecToSba(pBuf, &pInd->sec);

    /* Send host a response */
    sendCRSP(MT_MAC_DISASSOCIATE_IND, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_ORPHAN_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_OrphanInd(ApiMac_mlmeOrphanInd_t *pInd)
{
    uint8_t rspBuf[MT_MAC_LEN_ORPHAN_IND];
    uint8_t *pBuf = rspBuf;

    /* Extended address */
    pBuf = copyExtAdr(pBuf, pInd->orphanAddress);

    /* Security */
    macSecToSba(pBuf, &pInd->sec);

    /* Send host a response */
    sendCRSP(MT_MAC_ORPHAN_IND, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_POLL_CNF callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_PollCnf(ApiMac_mlmePollCnf_t *pCnf)
{
    uint8_t rspBuf[MT_MAC_LEN_POLL_CNF];
    uint8_t *pBuf = rspBuf;

    /* Status */
    *pBuf++ = pCnf->status;

    /* Frame pending indicator */
    *pBuf = pCnf->framePending;

    /* Send host a response */
    sendCRSP(MT_MAC_POLL_CNF, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_POLL_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_PollInd(ApiMac_mlmePollInd_t *pInd)
{
    uint8_t rspBuf[MT_MAC_LEN_POLL_IND];
    uint8_t *pBuf = rspBuf;

    /* Address of device sending data request */
    pBuf = macAdrToSba(pBuf, &pInd->srcAddr);

    /* PAN ID */
    pBuf = Util_bufferUint16(pBuf, pInd->srcPanId);

    /* No response required indicator */
    *pBuf = pInd->noRsp;

    /* Send host a response */
    sendCRSP(MT_MAC_POLL_IND, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_PURGE_CNF callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_PurgeCnf(ApiMac_mcpsPurgeCnf_t *pCnf)
{
    uint8_t rspBuf[MT_MAC_LEN_PURGE_CNF];
    uint8_t *pBuf = rspBuf;

    /* Status */
    *pBuf++ = pCnf->status;

    /* MSDU handle */
    *pBuf = pCnf->msduHandle;

    /* Send host a response */
    sendCRSP(MT_MAC_PURGE_CNF, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_SCAN_CNF callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_ScanCnf(ApiMac_mlmeScanCnf_t *pCnf)
{
    uint8_t listLen;
    uint16_t rspLen;
    uint8_t *pRspBuf;

    /* Results buffer size depends on type of scan */
    if(pCnf->scanType == ApiMac_scantype_orphan)
    {
        /* No results list for oprhan scans */
        rspLen = listLen = 0;
    }
    else
    {
        /* Number of items in results list */
        rspLen = listLen = pCnf->resultListSize;

        if(pCnf->scanType != ApiMac_scantype_energyDetect)
        {
            /* List of PAN descriptors if not energy detect */
            rspLen *= MT_MAC_LEN_PANDESC;
        }
    }

    /* Add fixed length to variable list length */
    rspLen += MT_MAC_LEN_SCAN_CNF;

    pRspBuf = MAP_ICall_malloc(rspLen);
    if(pRspBuf != NULL)
    {
        uint8_t *pBuf = pRspBuf;

        /* Status */
        *pBuf++ = pCnf->status;

        /* Scan type */
        *pBuf++ = pCnf->scanType;

        /* Channel page */
        *pBuf++ = pCnf->channelPage;

        /* PHY ID */
        *pBuf++ = pCnf->phyId;

        /* Unscanned channel bit mask */
        memcpy(pBuf, &pCnf->unscannedChannels, APIMAC_154G_CHANNEL_BITMAP_SIZ);
        pBuf += APIMAC_154G_CHANNEL_BITMAP_SIZ;

        /* Number of items in result list */
        *pBuf++ = pCnf->resultListSize;

        if(pCnf->scanType == ApiMac_scantype_energyDetect)
        {
            /* Copy list of detected energy for scanned channels */
            memcpy(pBuf, pCnf->result.pEnergyDetect, listLen);
        }
        else
        {
            ApiMac_panDesc_t *pDesc = pCnf->result.pPanDescriptor;

            /* Copy the list of PAN descriptors */
            while(listLen > 0)
            {
                /* Coordinator address */
                pBuf = macAdrToSba(pBuf, &pDesc->coordAddress);

                /* Coordinator PAN ID */
                pBuf = Util_bufferUint16(pBuf, pDesc->coordPanId);

                /* Network superframe spec */
                pBuf = Util_bufferUint16(pBuf, pDesc->superframeSpec);

                /* Logical channel */
                *pBuf++ = pDesc->logicalChannel;

                /* Channel page */
                *pBuf++ = pDesc->channelPage;

                /* GTS permit indicatior */
                *pBuf++ = pDesc->gtsPermit;

                /* Link quality */
                *pBuf++ = pDesc->linkQuality;

                /* Timestamp (backoffs) */
                pBuf = Util_bufferUint32(pBuf, pDesc->timestamp);

                /* Security fail indicator */
                *pBuf++ = pDesc->securityFailure;

                /* Security */
                macSecToSba(pBuf, &pDesc->sec);
                pBuf += MT_MAC_LEN_SECINFO;

                pDesc++;
                listLen--;
            }
        }

        /* Send host a response */
        sendCRSP(MT_MAC_SCAN_CNF, rspLen, pRspBuf);

        /* Give back memory */
        MAP_ICall_free(pRspBuf);
    }
}

/*!
 * Process MAC_START_CNF callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_StartCnf(ApiMac_mlmeStartCnf_t *pCnf)
{
    uint8_t rspBuf[MT_MAC_LEN_START_CNF];
    uint8_t *pBuf = rspBuf;

    /* Status */
    *pBuf++ = pCnf->status;

    /* Send host a response */
    sendCRSP(MT_MAC_START_CNF, sizeof(rspBuf), rspBuf);
}

/*!
 * Process MAC_SYNC_LOSS_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtMac_SyncLossInd(ApiMac_mlmeSyncLossInd_t *pInd)
{
    uint8_t rspBuf[MT_MAC_LEN_SYLOSS_IND];
    uint8_t *pBuf = rspBuf;

    /* Reason */
    *pBuf++ = pInd->reason;

    /* PAN ID */
    pBuf = Util_bufferUint16(pBuf, pInd->panId);

    /* Logical channel */
    *pBuf++ = pInd->logicalChannel;

    /* Channel page */
    *pBuf++ = pInd->channelPage;

    /* PHY ID */
    *pBuf++ = pInd->phyID;

    /* Security */
    macSecToSba(pBuf, &pInd->sec);

    /* Send host a response */
    sendCRSP(MT_MAC_SYNC_LOSS_IND, sizeof(rspBuf), rspBuf);
}

/******************************************************************************
 Local Callback Utility Functions
 *****************************************************************************/
/*!
 * @brief   Process ASYNC/DATA_IND callback issued by MAC
 *
 * @param   indType - MT_MAC_DATA_IND or MT_MAC_ASYNC_IND
 * @param   pInd - pointer to incoming callback data structure
 */
static void dataInd(uint8_t indType, ApiMac_mcpsDataInd_t *pInd)
{
    uint16_t rspLen;
    uint8_t *pRspBuf;

    if(indType == MT_MAC_DATA_IND)
    {
        /* Fixed length of data indication */
        rspLen = MT_MAC_LEN_DATA_IND;
    }
    else
    {
        /* Fixed length of async indication */
        rspLen = MT_MAC_LEN_ASYNC_IND;
    }

    /* Add in the variable MSDU and IE data lengths */
    rspLen += pInd->msdu.len + pInd->payloadIeLen;

    pRspBuf = MAP_ICall_malloc(rspLen);
    if(pRspBuf != NULL)
    {
        uint8_t *pBuf = pRspBuf;

        /* Source device address */
        pBuf = macAdrToSba(pBuf, &pInd->srcAddr);

        /* Destination device address */
        pBuf = macAdrToSba(pBuf, &pInd->dstAddr);

        /* Timestamp (backoffs) */
        pBuf = Util_bufferUint32(pBuf, pInd->timestamp);

        /* Timestamp2 (MAC units) */
        pBuf = Util_bufferUint16(pBuf, pInd->timestamp2);

        /* Source PAN ID */
        pBuf = Util_bufferUint16(pBuf, pInd->srcPanId);

        /* Destination PAN ID */
        pBuf = Util_bufferUint16(pBuf, pInd->dstPanId);

        /* Link quality */
        *pBuf++ = pInd->mpduLinkQuality;

        /* Correlation */
        *pBuf++ = pInd->correlation;

        /* RSSI */
        *pBuf++ = pInd->rssi;

        /* Data sequence number */
        *pBuf++ = pInd->dsn;

        /* Security */
        macSecToSba(pBuf, &pInd->sec);
        pBuf += MT_MAC_LEN_SECINFO;

        /* Frame counter */
        pBuf = Util_bufferUint32(pBuf, pInd->frameCntr);

        if(indType == MT_MAC_ASYNC_IND)
        {
            /* Frequency hopping frame type */
            *pBuf++ = pInd->fhFrameType;
        }

        /* Payload MDSU data length */
        pBuf = Util_bufferUint16(pBuf, pInd->msdu.len);

        /* Payload IE data length */
        pBuf = Util_bufferUint16(pBuf, pInd->payloadIeLen);

        /* MSDU data block */
        memcpy(pBuf, pInd->msdu.p, pInd->msdu.len);
        pBuf += pInd->msdu.len;

        /* IE data block */
        memcpy(pBuf, pInd->pPayloadIE, pInd->payloadIeLen);

        /* Send host a response */
        sendCRSP(indType, rspLen, pRspBuf);

        /* Give back memory */
        MAP_ICall_free(pRspBuf);
    }
}

/*!
 * @brief   Process Enhanced BEACON_NOTIFY_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
static void eBeaconInd(ApiMac_mlmeBeaconNotifyInd_t *pInd)
{
    uint8_t rspBuf[MT_MAC_LEN_EBEACON_IND];
    uint8_t *pBuf = rspBuf;

    /* Beacon type */
    *pBuf++ = ApiMac_beaconType_enhanced;

    /* Beacon sequence number */
    *pBuf++ = pInd->bsn;

    /* Beacon order */
    *pBuf++ = pInd->beaconData.eBeacon.coexist.beaconOrder;

    /* Superframe order */
    *pBuf++ = pInd->beaconData.eBeacon.coexist.superFrameOrder;

    /* Final CAP slot */
    *pBuf++ = pInd->beaconData.eBeacon.coexist.finalCapSlot;

    /* Enhanced beacon order */
    *pBuf++ = pInd->beaconData.eBeacon.coexist.eBeaconOrder;

    /* Periodic/enhanced beacon time offset */
    *pBuf++ = pInd->beaconData.eBeacon.coexist.offsetTimeSlot;

    /* TX slot position for enhabced beacon */
    *pBuf++ = pInd->beaconData.eBeacon.coexist.capBackOff;

    /* Non-beacon PAN enhanced beacon order */
    (void)Util_bufferUint16(pBuf, pInd->beaconData.eBeacon.coexist.eBeaconOrderNBPAN);

    /* Send host a response */
    sendCRSP(MT_MAC_BEACON_NOTIFY_IND, sizeof(rspBuf), rspBuf);
}

/*!
 * @brief   Process Normal BEACON_NOTIFY_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
static void nBeaconInd(ApiMac_mlmeBeaconNotifyInd_t *pInd)
{
    uint16_t pLen;
    uint16_t sLen;
    uint16_t xLen;
    uint16_t rspLen;
    uint8_t *pRspBuf;

    /* Length of beacon payload */
    pLen = pInd->beaconData.beacon.sduLength * sizeof(uint8_t);

    /* Length of pending short address list */
    sLen = pInd->beaconData.beacon.numPendShortAddr * sizeof(uint16_t);

    /* Length of pending extended address list */
    xLen = pInd->beaconData.beacon.numPendExtAddr * APIMAC_SADDR_EXT_LEN;

    /* Length of beacon indication response */
    rspLen = MT_MAC_LEN_BEACON_IND + sLen + xLen + pLen;

    pRspBuf = MAP_ICall_malloc(rspLen);
    if(pRspBuf != NULL)
    {
        uint8_t *pBuf = pRspBuf;

        /* Beacon type */
        *pBuf++ = ApiMac_beaconType_normal;

        /* Beacon sequence number */
        *pBuf++ = pInd->bsn;

        /* Timestamp (backoffs) */
        pBuf = Util_bufferUint32(pBuf, pInd->panDesc.timestamp);

        /* Coordinator address */
        pBuf = macAdrToSba(pBuf, &pInd->panDesc.coordAddress);

        /* Coordinator PAN ID */
        pBuf = Util_bufferUint16(pBuf, pInd->panDesc.coordPanId);

        /* Network superframe spec */
        pBuf = Util_bufferUint16(pBuf, pInd->panDesc.superframeSpec);

        /* Logical channel */
        *pBuf++ = pInd->panDesc.logicalChannel;

        /* Channel page */
        *pBuf++ = pInd->panDesc.channelPage;

        /* GTS permit indicator */
        *pBuf++ = pInd->panDesc.gtsPermit;

        /* Limk quality */
        *pBuf++ = pInd->panDesc.linkQuality;

        /* Security fail indicator */
        *pBuf++ = pInd->panDesc.securityFailure;

        /* Security */
        macSecToSba(pBuf, &pInd->panDesc.sec);
        pBuf += MT_MAC_LEN_SECINFO;

        /* Number of pending short addresses */
        *pBuf++ = pInd->beaconData.beacon.numPendShortAddr;

        /* Number of pending extended addresses */
        *pBuf++ = pInd->beaconData.beacon.numPendExtAddr;

        /* Length of beacon payload */
        *pBuf++ = pInd->beaconData.beacon.sduLength;

        /* List of pending short addresses */
        memcpy(pBuf, pInd->beaconData.beacon.pShortAddrList, sLen);
        pBuf += sLen;

        /* List of pending extended addresses */
        memcpy(pBuf, pInd->beaconData.beacon.pExtAddrList, xLen);
        pBuf += xLen;

        /* Beacon payload */
        memcpy(pBuf, pInd->beaconData.beacon.pSdu, pLen);

        /* Send host a response */
        sendCRSP(MT_MAC_BEACON_NOTIFY_IND, rspLen, pRspBuf);

        /* Give back memory */
        MAP_ICall_free(pRspBuf);
    }
}
#ifdef FEATURE_MAC_SECURITY
/******************************************************************************
 Local Security PIB Utility Functions
 *****************************************************************************/
/*!
 * @brief   Serialize a ApiMac_securityPibDeviceEntry_t structure
 *
 * @param   pDst - pointer to destination buffer
 * @param   pSrc - pointer to source structure
 *
 * @return  Number of bytes copied to destination buffer
 */
static uint8_t bufferDeviceEntry(uint8_t *pDst, void *pSrc)
{
    uint8_t i;
    uint8_t *pBuf = pDst;
    ApiMac_securityPibDeviceEntry_t *pSec = pSrc;

#ifndef SEC_API_256
    /* Index into macDeviceTable */
    pBuf = Util_bufferUint16(pBuf, pSec->deviceIndex);
#else
    /* Index into macDeviceTable */
    *pBuf++ = pSec->deviceIndex;
#endif
    /* Security Device Descriptor: device PAN ID */
    pBuf = Util_bufferUint16(pBuf, pSec->deviceEntry.devInfo.panID);

    /* Security Device Descriptor: device 16-bit address */
    pBuf = Util_bufferUint16(pBuf, pSec->deviceEntry.devInfo.shortAddress);

    /* Security Device Descriptor: device 64-bit address */
    pBuf = copyExtAdr(pBuf, (uint8_t*)&pSec->deviceEntry.devInfo.extAddress);

    /* Security Device Descriptor: minimum security override indicator */
    *pBuf++ = pSec->deviceEntry.exempt;
#ifndef SEC_API_256
    /* Security Device Descriptor: device incoming frame counter list */
    for(i = 0; i < API_MAX_NODE_KEY_ENTRIES;  i++)
    {
        pBuf = Util_bufferUint32(pBuf, pSec->deviceEntry.frameCntr[i].frameCounter);
        pBuf = Util_bufferUint16(pBuf, pSec->deviceEntry.frameCntr[i].keyIdx);
    }
#else
    /* Security Device Descriptor: device incoming frame counter list */
    for(i = 0; i < API_MAX_NODE_KEY_ENTRIES;  i++)
    {
        pBuf = Util_bufferUint32(pBuf, pSec->deviceEntry.frameCntr[i].frameCounter);
    }
#endif
    /* Number of bytes serialized */
    return(pBuf - pDst);
}

/*!
 * @brief   Serialize a ApiMac_securityPibKeyDeviceEntry_t structure
 *
 * @param   pDst - pointer to destination buffer
 * @param   pSrc - pointer to source structure
 *
 * @return  Number of bytes copied to destination buffer
 */
static uint8_t bufferKeyDevice(uint8_t *pDst, void *pSrc)
{
    uint8_t *pBuf = pDst;
    ApiMac_securityPibKeyDeviceEntry_t *pSec = pSrc;

#ifndef SEC_API_256
    /* Index into macKeyDeviceList */
    pBuf = Util_bufferUint16(pBuf, pSec->keyIndex);

    /* Index into macKeyDeviceList[keyIndex] */
    pBuf = Util_bufferUint16(pBuf, pSec->keyDeviceIndex);

    /* Key Device Descriptor: handle of the device descriptor */
    pBuf = Util_bufferUint16(pBuf, pSec->deviceEntry.deviceDescriptorHandle);
#else
    /* Index into macKeyDeviceList */
    *pBuf++ = pSec->keyIndex;

    /* Index into macKeyDeviceList[keyIndex] */
    *pBuf++ = pSec->keyDeviceIndex;

    /* Key Device Descriptor: handle of the device descriptor */
    *pBuf++ = pSec->deviceEntry.deviceDescriptorHandle;
#endif
    /* Key Device Descriptor: unique device indicator */
    *pBuf++ = pSec->deviceEntry.uniqueDevice;

    /* Key Device Descriptor: key has exhausted the frame counter */
    *pBuf++ = pSec->deviceEntry.blackListed;

    /* Number of bytes serialized */
    return(pBuf - pDst);
}

/*!
 * @brief   Serialize a ApiMac_securityPibKeyEntry_t structure
 *
 * @param   pDst - pointer to destination buffer
 * @param   pSrc - pointer to source structure
 *
 * @return  Number of bytes copied to destination buffer
 */
static uint8_t bufferKeyEntry(uint8_t *pDst, void *pSrc)
{
    uint8_t *pBuf = pDst;
    ApiMac_securityPibKeyEntry_t *pSec = pSrc;

#ifndef SEC_API_256
    /* Index into macKeyTable */
    pBuf = Util_bufferUint16(pBuf, pSec->keyIndex);
    /*skip over unused byte */
    pBuf += 2;
#else
    /* Index into macKeyTable */
    *pBuf++ = pSec->keyIndex;
#endif
    memcpy(pBuf, pSec->keyEntry, APIMAC_KEY_MAX_LEN);
    pBuf += APIMAC_KEY_MAX_LEN;

    pBuf = Util_bufferUint32(pBuf, pSec->frameCounter);

    /* Number of bytes serialized */
    return(pBuf - pDst);
}

/*!
 * @brief   Serialize a ApiMac_securityPibKeyIdLookupEntry_t structure
 *
 * @param   pDst - pointer to destination buffer
 * @param   pSrc - pointer to source structure
 *
 * @return  Number of bytes copied to destination buffer
 */
static uint8_t bufferKeyLookup(uint8_t *pDst, void *pSrc)
{
    uint8_t *pBuf = pDst;
    ApiMac_securityPibKeyIdLookupEntry_t *pSec = pSrc;

#ifndef SEC_API_256
    /* Index into macKeyIdLookupList */
    pBuf = Util_bufferUint16(pBuf, pSec->keyIndex);

    /* Index into macKeyIdLookupList[keyIndex] */
    *pBuf++ = pSec->keyIdLookupIndex;
    /* skip over unused byte */
    pBuf++;
#else
    /* Index into macKeyIdLookupList */
    *pBuf++ = pSec->keyIndex;

    /* Index into macKeyIdLookupList[keyIndex] */
    *pBuf++ = pSec->keyIdLookupIndex;
#endif
    /* Key ID Lookup Descriptor: data used to identify key */
    memcpy(pBuf, pSec->lookupEntry.lookupData, APIMAC_MAX_KEY_LOOKUP_LEN);
    pBuf += APIMAC_MAX_KEY_LOOKUP_LEN;

    /* Key ID Lookup Descriptor: lookup data size */
    *pBuf++ = pSec->lookupEntry.lookupDataSize;

    /* Number of bytes serialized */
    return(pBuf - pDst);
}

/*!
 * @brief   Serialize a ApiMac_securityPibKeyUsageEntry_t structure
 *
 * @param   pDst - pointer to destination buffer
 * @param   pSrc - pointer to source structure
 *
 * @return  Number of bytes copied to destination buffer
 */
static uint8_t bufferKeyUsage(uint8_t *pDst, void *pSrc)
{
    uint8_t *pBuf = pDst;
    ApiMac_securityPibKeyUsageEntry_t *pSec = pSrc;

#ifndef SEC_API_256
    /* Index into macKeyUsageList */
    pBuf = Util_bufferUint16(pBuf, pSec->keyIndex);

    /* Index into macKeyUsageList[keyIndex] */
    *pBuf++ = pSec->keyUsageIndex;
    /* skip over unused byte */
    pBuf++;
#else
    /* Index into macKeyUsageList */
    *pBuf++ = pSec->keyIndex;

    /* Index into macKeyUsageList[keyIndex] */
    *pBuf++ = pSec->keyUsageIndex;
#endif
    /* Key Usage Descriptor: frame type */
    *pBuf++ = pSec->usageEntry.frameType;

    /* Key Usage Descriptor: command frame ID */
    *pBuf++ = pSec->usageEntry.cmdFrameId;

    /* Number of bytes serialized */
    return(pBuf - pDst);
}

/*!
 * @brief   Serialize a ApiMac_securityPibSecurityLevelEntry_t structure
 *
 * @param   pDst - pointer to destination buffer
 * @param   pSrc - pointer to source structure
 *
 * @return  Number of bytes copied to destination buffer
 */
static uint8_t bufferLevelEntry(uint8_t *pDst, void *pSrc)
{
    uint8_t *pBuf = pDst;
    ApiMac_securityPibSecurityLevelEntry_t *pSec = pSrc;

#ifndef SEC_API_256
    /* Index into macSecurityLevelTable */
    *pBuf++ = pSec->levelIndex;
    /*skip over unused bytes */
    *pBuf++ = 0;
    *pBuf++ = 0;
    *pBuf++ = 0;
#else
    /* Index into macSecurityLevelTable */
    *pBuf++ = pSec->levelIndex;
    /*skip over unused byte */
    *pBuf++ = 0;
#endif
    /* Security Level Descriptor: frame type */
    *pBuf++ = pSec->levelEntry.frameType;

    /* Security Level Descriptor: command frame ID */
    *pBuf++ = pSec->levelEntry.commandFrameIdentifier;

    /* Security Level Descriptor: minimal required/expected security level */
    *pBuf++ = pSec->levelEntry.securityMinimum;

    /* Security Level Descriptor: incoming security level override indicator */
    *pBuf++ = pSec->levelEntry.securityOverrideSecurityMinimum;

    /* Number of bytes serialized */
    return(pBuf - pDst);
}

/*!
 * @brief   Serialize 'miscellaneous' parameter
 *
 * @param   pDst - pointer to destination buffer
 * @param   pSrc - pointer to source structure
 * @param   sLen - length of parameter in source buffer
 *
 * @return  Number of bytes copied to destination buffer
 */
static uint8_t bufferMiscParam(uint8_t *pDst, void *pSrc, uint8_t sLen)
{
    uint8_t *pBuf = pDst;

    if(sLen == sizeof(uint8_t))
    {
        *pBuf++ = *(uint8_t*)pSrc;
    }
    else if(sLen == sizeof(uint16_t))
    {
        pBuf = Util_bufferUint16(pBuf, *(uint16_t*)pSrc);
    }
    else if(sLen == sizeof(uint32_t))
    {
        pBuf = Util_bufferUint32(pBuf, *(uint32_t*)pSrc);
    }
    else
    {
        /* Assume an array of bytes */
        memcpy(pBuf, pSrc, sLen);
        pBuf += sLen;
    }

    /* Number of bytes serialized */
    return(pBuf - pDst);
}

/*!
 * @brief   Build an ApiMac_securityPibDeviceEntry_t structure
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  status - command processing error code
 */
static uint8_t buildDeviceEntry(Mt_mpb_t *pMpb)
{
    uint8_t i;
    uint8_t *pSrc;
    ApiMac_securityPibDeviceEntry_t *pSec;

    if(pMpb->length != sizeof(MtPkt_secDevEntry_t))
    {
        /* Incorrect incoming MT buffer length */
        return(ApiMac_status_lengthError);
    }

    pSec = MAP_ICall_malloc(sizeof(ApiMac_securityPibDeviceEntry_t));
    if(pSec == NULL)
    {
        /* Not enough memory to build structure */
        return(ApiMac_status_noResources);
    }

    ((ApiMac_securityPibDeviceEntry_t *)pSec)->deviceEntry.frameCntr = \
            MAP_ICall_malloc(sizeof(ApiMac_frameCntr_t)*API_MAX_NODE_KEY_ENTRIES);
    if(((ApiMac_securityPibDeviceEntry_t *)pSec)->deviceEntry.frameCntr == NULL)
    {
        /* Not enough memory to build structure */
        return(ApiMac_status_noResources);
    }

    /* Skip over attribute id */
    pSrc = (uint8_t*)pMpb->pData + 1;
#ifndef SEC_API_256
    /* Index into macDeviceTable */
    pSec->deviceIndex = Util_parseUint16(pSrc);
    pSrc += 2;
    /* Skip over unused 2nd index */
    pSrc += 2;
#else
    /* Index into macDeviceTable */
    pSec->deviceIndex = *pSrc;
    /* Skip over unused 2nd index */
    pSrc += 2;
#endif
    /* Security Device Descriptor: device PAN ID */
    pSec->deviceEntry.devInfo.panID = Util_parseUint16(pSrc);
    pSrc += 2;

    /* Security Device Descriptor: device 16-bit address */
    pSec->deviceEntry.devInfo.shortAddress = Util_parseUint16(pSrc);
    pSrc += 2;

    /* Security Device Descriptor: device 64-bit address */
    copyExtAdr((uint8_t*)&pSec->deviceEntry.devInfo.extAddress, pSrc);
    pSrc += APIMAC_SADDR_EXT_LEN;

    /* Security Device Descriptor: minimum security override indicator */
    pSec->deviceEntry.exempt = *pSrc++;

#ifndef SEC_API_256
    /* Security Device Descriptor: device incoming frame counter list */
    for(i = 0; i < API_MAX_NODE_KEY_ENTRIES;  i++)
    {
        pSec->deviceEntry.frameCntr[i].frameCounter = Util_parseUint32(pSrc);
        pSrc += 4;
        pSec->deviceEntry.frameCntr[i].keyIdx = Util_parseUint16(pSrc);
        pSrc += 2;
    }
#else
    /* Security Device Descriptor: device incoming frame counter list */
    for(i = 0; i < API_MAX_NODE_KEY_ENTRIES;  i++)
    {
        pSec->deviceEntry.frameCntr[i].frameCounter = Util_parseUint32(pSrc);
        pSrc += 4;
        pSec->deviceEntry.frameCntr[i].keyIdx = i;
    }
#endif
    /* Return ptr to allocated/filled data structure */
    pMpb->pData = pSec;

    return(ApiMac_status_success);
}

/*!
 * @brief   Build an ApiMac_securityPibKeyDeviceEntry_t structure
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  status - command processing error code
 */
static uint8_t buildKeyDevice(Mt_mpb_t *pMpb)
{
    uint8_t *pSrc;
    ApiMac_securityPibKeyDeviceEntry_t *pSec;

    if(pMpb->length != sizeof(MtPkt_secKeyDevice_t))
    {
        /* Incorrect incoming MT buffer length */
        return(ApiMac_status_lengthError);
    }

    pSec = MAP_ICall_malloc(sizeof(ApiMac_securityPibKeyDeviceEntry_t));
    if(pSec == NULL)
    {
        /* Not enough memory to build structure */
        return(ApiMac_status_noResources);
    }

    /* Skip over attribute id */
    pSrc = (uint8_t*)pMpb->pData + 1;

#ifndef SEC_API_256
    /* Index into macKeyDeviceList */
    pSec->keyIndex = Util_parseUint16(pSrc);
    pSrc += 2;

    /* Index into macKeyDeviceList[keyIndex] */
    pSec->keyDeviceIndex = Util_parseUint16(pSrc);
    pSrc += 2;

    /* Key Device Descriptor: handle of the device descriptor */
    pSec->deviceEntry.deviceDescriptorHandle = Util_parseUint16(pSrc);
    pSrc += 2;
#else
    /* Index into macKeyDeviceList */
    pSec->keyIndex = *pSrc++;

    /* Index into macKeyDeviceList[keyIndex] */
    pSec->keyDeviceIndex = *pSrc++;

    /* Key Device Descriptor: handle of the device descriptor */
    pSec->deviceEntry.deviceDescriptorHandle = *pSrc++;
#endif
    /* Key Device Descriptor: unique device indicator */
    pSec->deviceEntry.uniqueDevice = *pSrc++;

    /* Key Device Descriptor: key has exhausted the frame counter */
    pSec->deviceEntry.blackListed = *pSrc;

    /* Return ptr to allocated/filled data structure */
    pMpb->pData = pSec;

    return(ApiMac_status_success);
}

/*!
 * @brief   Build an ApiMac_securityPibKeyEntry_t structure
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  status - command processing error code
 */
static uint8_t buildKeyEntry(Mt_mpb_t *pMpb)
{
    uint8_t *pSrc;
    ApiMac_securityPibKeyEntry_t *pSec;

    if(pMpb->length != sizeof(MtPkt_secKeyEntry_t))
    {
        /* Incorrect incoming MT buffer length */
        return(ApiMac_status_lengthError);
    }

    pSec = MAP_ICall_malloc(sizeof(ApiMac_securityPibKeyEntry_t));
    if(pSec == NULL)
    {
        /* Not enough memory to build structure */
        return(ApiMac_status_noResources);
    }

    /* Skip over attribute id */
    pSrc = (uint8_t*)pMpb->pData + 1;
#ifndef SEC_API_256
    /* Index into macKeyTable */
    pSec->keyIndex = Util_parseUint16(pSrc);
    pSrc += 2;
    /* Skip over unused 2nd index */
    pSrc += 2;
#else
    /* Index into macKeyTable */
    pSec->keyIndex = *pSrc;
    /* Skip over unused 2nd index */
    pSrc += 2;
#endif
    /* Key entry */
    memcpy(pSec->keyEntry, pSrc, APIMAC_KEY_MAX_LEN);
    pSrc += APIMAC_KEY_MAX_LEN;

    /* Frame counter */
    pSec->frameCounter = Util_parseUint32(pSrc);

    /* Return ptr to allocated/filled data structure */
    pMpb->pData = pSec;

    return(ApiMac_status_success);
}

/*!
 * @brief   Build an ApiMac_securityPibKeyIdLookupEntry_t structure
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  status - command processing error code
 */
static uint8_t buildKeyLookup(Mt_mpb_t *pMpb)
{
    uint8_t *pSrc;
    ApiMac_securityPibKeyIdLookupEntry_t *pSec;

    if(pMpb->length != sizeof(MtPkt_secKeyLookup_t))
    {
        /* Incorrect incoming MT buffer length */
        return(ApiMac_status_lengthError);
    }

    pSec = MAP_ICall_malloc(sizeof(ApiMac_securityPibKeyIdLookupEntry_t));
    if(pSec == NULL)
    {
        /* Not enough memory to build structure */
        return(ApiMac_status_noResources);
    }

    /* Skip over attribute id */
    pSrc = (uint8_t*)pMpb->pData + 1;
#ifndef SEC_API_256
    /* Index into macKeyIdLookupList */
    pSec->keyIndex = Util_parseUint16(pSrc);
    pSrc += 2;

    /* Index into macKeyIdLookupList[keyIndex] */
    pSec->keyIdLookupIndex = *pSrc++;
    /*skip over unused byte */
    pSrc += 1;
#else
    /* Index into macKeyIdLookupList */
    pSec->keyIndex = *pSrc++;

    /* Index into macKeyIdLookupList[keyIndex] */
    pSec->keyIdLookupIndex = *pSrc++;
#endif
    /* Key ID Lookup Descriptor: data used to identify key */
    memcpy(pSec->lookupEntry.lookupData, pSrc, APIMAC_MAX_KEY_LOOKUP_LEN);
    pSrc += APIMAC_MAX_KEY_LOOKUP_LEN;

    /* Key ID Lookup Descriptor: lookup data size */
    pSec->lookupEntry.lookupDataSize = *pSrc;

    /* Return ptr to allocated/filled data structure */
    pMpb->pData = pSec;

    return(ApiMac_status_success);
}

/*!
 * @brief   Process an ApiMac_securityAttribute_keyTable build request
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  status - command processing error code
 */
static uint8_t buildKeyTable(Mt_mpb_t *pMpb)
{
    if(pMpb->length != sizeof(MtPkt_secKeyDesc_t))
    {
        /* Incorrect incoming MT buffer length */
        return(ApiMac_status_lengthError);
    }

    /* No data for this request */
    pMpb->pData = NULL;

    return(ApiMac_status_success);
}

/*!
 * @brief   Build an ApiMac_securityPibKeyUsageEntry_t structure
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  status - command processing error code
 */
static uint8_t buildKeyUsage(Mt_mpb_t *pMpb)
{
    uint8_t *pSrc;
    ApiMac_securityPibKeyUsageEntry_t *pSec;

    if(pMpb->length != sizeof(MtPkt_secKeyUsage_t))
    {
        /* Incorrect incoming MT buffer length */
        return(ApiMac_status_lengthError);
    }

    pSec = MAP_ICall_malloc(sizeof(ApiMac_securityPibKeyUsageEntry_t));
    if(pSec == NULL)
    {
        /* Not enough memory to build structure */
        return(ApiMac_status_noResources);
    }

    /* Skip over attribute id */
    pSrc = (uint8_t*)pMpb->pData + 1;
#ifndef SEC_API_256
    /* Index into macKeyUsageList */
    pSec->keyIndex = Util_parseUint16(pSrc);
    pSrc += 2;

    /* Index into macKeyUsageList[keyIndex] */
    pSec->keyUsageIndex = *pSrc++;
    /*skip over unused byte */
    pSrc += 1;
#else
    /* Index into macKeyUsageList */
    pSec->keyIndex = *pSrc++;

    /* Index into macKeyUsageList[keyIndex] */
    pSec->keyUsageIndex = *pSrc++;

#endif
    /* Key Usage Descriptor: frame type */
    pSec->usageEntry.frameType = *pSrc++;

    /* Key Usage Descriptor: command frame ID */
    pSec->usageEntry.cmdFrameId = *pSrc;

    /* Return ptr to allocated/filled data structure */
    pMpb->pData = pSec;

    return(ApiMac_status_success);
}

/*!
 * @brief   Build an ApiMac_securityPibSecurityLevelEntry_t structure
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  status - command processing error code
 */
static uint8_t buildLevelEntry(Mt_mpb_t *pMpb)
{
    uint8_t *pSrc;
    ApiMac_securityPibSecurityLevelEntry_t *pSec;

    if(pMpb->length != sizeof(MtPkt_secLevelEntry_t))
    {
        /* Incorrect incoming MT buffer length */
        return(ApiMac_status_lengthError);
    }

    pSec = MAP_ICall_malloc(sizeof(ApiMac_securityPibSecurityLevelEntry_t));
    if(pSec == NULL)
    {
        /* Not enough memory to build structure */
        return(ApiMac_status_noResources);
    }

    /* Skip over attribute id */
    pSrc = (uint8_t*)pMpb->pData + 1;
#ifndef SEC_API_256
    /* Index into macSecurityLevelTable */
    pSec->levelIndex = *pSrc;
    /* Skip over unused byte */
    pSrc += 2;

    /* Skip over unused 2nd index */
    pSrc += 2;
#else
    /* Index into macSecurityLevelTable */
    pSec->levelIndex = *pSrc;

    /* Skip over unused 2nd index */
    pSrc += 2;
#endif
    /* Security Level Descriptor: frame type */
    pSec->levelEntry.frameType = *pSrc++;

    /* Security Level Descriptor: command frame ID */
    pSec->levelEntry.commandFrameIdentifier = *pSrc++;

    /* Security Level Descriptor: min required/expected security level */
    pSec->levelEntry.securityMinimum = *pSrc++;

    /* Security Level Descriptor: security level override indicator */
    pSec->levelEntry.securityOverrideSecurityMinimum = *pSrc;

    /* Return ptr to allocated/filled data structure */
    pMpb->pData = pSec;

    return(ApiMac_status_success);
}

/*!
 * @brief   Build a single PIB item packed serial array
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  status - command processing error code
 */
static uint8_t buildSecParam(uint8_t attr, Mt_mpb_t *pMpb)
{
    uint8_t len;
    uint8_t *pSrc;
    void *pSec;

    switch(attr)
    {
        case ApiMac_securityAttribute_securityLevelTableEntries:
        case ApiMac_securityAttribute_autoRequestSecurityLevel:
        case ApiMac_securityAttribute_autoRequestKeyIdMode:
        case ApiMac_securityAttribute_autoRequestKeyIndex:
#ifdef SEC_API_256
        case ApiMac_securityAttribute_keyTableEntries:
        case ApiMac_securityAttribute_deviceTableEntries:
#endif
            len = sizeof(uint8_t);
            break;
        case ApiMac_securityAttribute_panCoordShortAddress:
#ifndef SEC_API_256
        case ApiMac_securityAttribute_keyTableEntries:
        case ApiMac_securityAttribute_deviceTableEntries:
#endif
            len = sizeof(uint16_t);
            break;
        case ApiMac_securityAttribute_panCoordExtendedAddress:
            len = APIMAC_SADDR_EXT_LEN;
            break;
        case ApiMac_securityAttribute_autoRequestKeySource:
        case ApiMac_securityAttribute_defaultKeySource:
            len = APIMAC_KEY_SOURCE_MAX_LEN;
            break;
        default:
            /* Unexpected attribute */
            len = 0xFF;
            break;
    }

    if(len != (pMpb->length - sizeof(MtPkt_setSecReq_t)))
    {
        /* Incorrect incoming MT buffer length */
        return(ApiMac_status_lengthError);
    }

    pSec = MAP_ICall_malloc(len);
    if(pSec == NULL)
    {
        /* Not enough memory to build structure */
        return(ApiMac_status_noResources);
    }

    /* Skip over attribute id and both indexes */
#ifdef SEC_API_256
    pSrc = (uint8_t*)pMpb->pData + 3;
#else
    pSrc = (uint8_t*)pMpb->pData + 5;
#endif
    switch(attr)
    {
        case ApiMac_securityAttribute_securityLevelTableEntries:
        case ApiMac_securityAttribute_autoRequestSecurityLevel:
        case ApiMac_securityAttribute_autoRequestKeyIdMode:
        case ApiMac_securityAttribute_autoRequestKeyIndex:
            *(uint8_t*)pSec = *pSrc;
            break;

        case ApiMac_securityAttribute_keyTableEntries:
        case ApiMac_securityAttribute_deviceTableEntries:
#ifdef SEC_API_256
            *(uint16_t*)pSec = *pSrc;
#else
            *(uint16_t*)pSec = Util_parseUint16(pSrc);
#endif
            break;
        case ApiMac_securityAttribute_panCoordShortAddress:
            *(uint16_t*)pSec = Util_parseUint16(pSrc);
            break;
        case ApiMac_securityAttribute_panCoordExtendedAddress:
            memcpy(pSec, pSrc, APIMAC_SADDR_EXT_LEN);
            break;
        case ApiMac_securityAttribute_autoRequestKeySource:
        case ApiMac_securityAttribute_defaultKeySource:
            memcpy(pSec, pSrc, APIMAC_SADDR_EXT_LEN);
            break;
    }

    /* Return ptr to allocated/filled data structure */
    pMpb->pData = pSec;

    return(ApiMac_status_success);
}
#endif
/******************************************************************************
 Local Utility Functions
 *****************************************************************************/
/*!
 * @brief   Copy extended address, return ptr to next destination byte
 *
 * @param   pDst - Pointer to destination address byte array
 * @param   pSrc - Pointer to source address byte array
 *
 * @return  pDst - Pointer to next location in destination byte array
 */
static uint8_t* copyExtAdr(uint8_t *pDst, uint8_t *pSrc)
{
    memcpy(pDst, pSrc, APIMAC_SADDR_EXT_LEN);
    pDst += APIMAC_SADDR_EXT_LEN;

    /* Ptr to next location in destination byte array */
    return(pDst);
}

/*!
 * @brief   Copy an ApiMac address structure to serial byte array
 *
 * @param   pSrc - Pointer to source address structure
 * @param   pDst - Pointer to destination serial byte array
 *
 * @return  pDst - Pointer to next location in destination byte array
 */
static uint8_t* macAdrToSba(uint8_t *pDst, ApiMac_sAddr_t *pSrc)
{
    *pDst++ = pSrc->addrMode;

    if(pSrc->addrMode == ApiMac_addrType_short)
    {
        memset(pDst, 0, APIMAC_SADDR_EXT_LEN);
        (void)Util_bufferUint16(pDst, pSrc->addr.shortAddr);
        pDst += APIMAC_SADDR_EXT_LEN;
    }
    else
    {
        pDst = copyExtAdr(pDst, pSrc->addr.extAddr);
    }

    return(pDst);
}

/*!
 * @brief   Copy an address from an serial byte array to an ApiMac struct
 *          The addrMode in pDst must already be set.
 *
 * @param   pDst - Pointer to destination address structure
 * @param   pSrc - Pointer to source address byte array
 */
static void macSbaToAdr(ApiMac_sAddr_t *pDst, uint8_t *pSrc)
{
    if(pDst->addrMode == ApiMac_addrType_short)
    {
        pDst->addr.shortAddr = Util_parseUint16(pSrc);
    }
    else if(pDst->addrMode == ApiMac_addrType_extended)
    {
        (void)copyExtAdr(pDst->addr.extAddr, pSrc);
    }
}

/*!
 * @brief   Copy security parameters from ApiMac struct to serial byte array
 *
 * @param   pSba - Pointer to serial byte array
 * @param   pSec - Pointer to security structure
 */
static void macSecToSba(uint8_t *pSba, ApiMac_sec_t *pSec)
{
    /* Key source */
    memcpy(pSba, pSec->keySource, APIMAC_KEY_SOURCE_MAX_LEN);
    pSba += APIMAC_KEY_SOURCE_MAX_LEN;

    /* Security level */
    *pSba++ = pSec->securityLevel;

    /* Key identifier mode */
    *pSba++ = pSec->keyIdMode;

    /* Key index */
    *pSba = pSec->keyIndex;
}

/*!
 * @brief   Copy security parameters from serial byte array to ApiMac struct
 *
 * @param   pSec - Pointer to security structure
 * @param   pSba - Pointer to serial byte array
 */
static void macSbaToSec(ApiMac_sec_t *pSec, uint8_t *pSba)
{
    /* Key source */
    memcpy(pSec->keySource, pSba, APIMAC_KEY_SOURCE_MAX_LEN);
    pSba += APIMAC_KEY_SOURCE_MAX_LEN;

    /* Security level */
    pSec->securityLevel = *pSba++;

    /* Key identifier mode */
    pSec->keyIdMode = *pSba++;

    /* Key index */
    pSec->keyIndex = *pSba;
}

/*!
 * @brief   Wrapper for MT_sendResponse() for MT_MAC Callback AREQ
 *
 * @param   rspId  - response ID
 * @param   rspLen - length of response buffer
 * @param   pRsp   - pointer to response buffer
 */
static void sendCRSP(uint8_t rspId, uint16_t rspLen, uint8_t *pRsp)
{
    uint32_t cbkBit;

    switch(rspId)
    {
        case MT_MAC_ASSOCIATE_CNF:
            cbkBit = CBSID_ASSOCIATE_CNF;
            break;
        case MT_MAC_ASSOCIATE_IND:
            cbkBit = CBSID_ASSOCIATE_IND;
            break;
        case MT_MAC_BEACON_NOTIFY_IND:
            cbkBit = CBSID_BEACON_NOTIFY_IND;
            break;
        case MT_MAC_COMM_STATUS_IND:
            cbkBit = CBSID_COMM_STATUS_IND;
            break;
        case MT_MAC_DATA_CNF:
            cbkBit = CBSID_DATA_CNF;
            break;
        case MT_MAC_DATA_IND:
            cbkBit = CBSID_DATA_IND;
            break;
        case MT_MAC_DISASSOCIATE_CNF:
            cbkBit = CBSID_DISASSOCIATE_CNF;
            break;
        case MT_MAC_DISASSOCIATE_IND:
            cbkBit = CBSID_DISASSOCIATE_IND;
            break;
        case MT_MAC_ORPHAN_IND:
            cbkBit = CBSID_ORPHAN_IND;
            break;
        case MT_MAC_POLL_CNF:
            cbkBit = CBSID_POLL_CNF;
            break;
        case MT_MAC_POLL_IND:
            cbkBit = CBSID_POLL_IND;
            break;
        case MT_MAC_PURGE_CNF:
            cbkBit = CBSID_PURGE_CNF;
            break;
        case MT_MAC_SCAN_CNF:
            cbkBit = CBSID_SCAN_CNF;
            break;
        case MT_MAC_START_CNF:
            cbkBit = CBSID_START_CNF;
            break;
        case MT_MAC_SYNC_LOSS_IND:
            cbkBit = CBSID_SYNC_LOSS_IND;
            break;
        case MT_MAC_ASYNC_CNF:
            cbkBit = CBSID_WS_ASYNC_CNF;
            break;
        case MT_MAC_ASYNC_IND:
            cbkBit = CBSID_WS_ASYNC_IND;
            break;
        default:
            cbkBit = 0;
            break;
    }

    if(callbackEnables & cbkBit)
    {
        /* Callback is enabled - send it */
        (void)MT_sendResponse(MT_ARSP_MAC, rspId, rspLen, pRsp);
    }
}

/*!
 * @brief   Wrapper for MT_sendResponse() for data SRSP
 *
 * @param   rspId  - response ID
 * @param   rspLen - length of response buffer
 * @param   pRsp   - pointer to response buffer
 */
static void sendDRSP(uint8_t rspId, uint16_t rspLen, uint8_t *pRsp)
{
    (void)MT_sendResponse(MT_SRSP_MAC, rspId, rspLen, pRsp);
}

/*!
 * @brief   Wrapper for MT_sendResponse() for status SRSP
 *
 * @param   rspId - response ID
 * @param   rsp   - response status
 */
static void sendSRSP(uint8_t rspId, uint8_t rsp)
{
    (void)MT_sendResponse(MT_SRSP_MAC, rspId, 1, &rsp);
}

/*!
 * @brief   Parse txOptions bits to ApiMac txOptions structure
 *
 * @param   optBits - MT command TX options bit mask
 * @param   txOptions - ptr to output TX options structure
 */
static void parseOptBits(uint16_t optBits, ApiMac_txOptions_t *txOptions)
{
    /* Start with no options enabled */
    memset(txOptions, false, sizeof(ApiMac_txOptions_t));

    if(optBits & MT_MAC_TXOPTION_ACK)
    {
        txOptions->ack = true;
    }
    if(optBits & MT_MAC_TXOPTION_GTS)
    {
        /* Not used */
    }
    if(optBits & MT_MAC_TXOPTION_INDIRECT)
    {
        txOptions->indirect = true;
    }
    if(optBits & MT_MAC_TXOPTION_PEND_BIT)
    {
        txOptions->pendingBit = true;
    }
    if(optBits & MT_MAC_TXOPTION_NO_RETRANS)
    {
        txOptions->noRetransmits = true;
    }
    if(optBits & MT_MAC_TXOPTION_NO_CNF)
    {
        txOptions->noConfirm = true;
    }
    if(optBits & MT_MAC_TXOPTION_ALT_BE)
    {
        txOptions->useAltBE = true;
    }
    if(optBits & MT_MAC_TXOPTION_PWR_CHAN)
    {
        txOptions->usePowerAndChannel = true;
    }
    if(optBits & MT_MAC_TXOPTION_GREEN_PWR)
    {
        txOptions->useGreenPower = true;
        if(optBits & MT_MAC_TXOPTION_GP_ABS_TIME)
        {
            txOptions->useAbsoluteTime = true;
        }
        else
        {
            txOptions->useAbsoluteTime = false;
        }
    }
}
#endif /* MT_MAC_FUNC */
