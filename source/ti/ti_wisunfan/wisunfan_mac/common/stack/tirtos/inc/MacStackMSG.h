/******************************************************************************

 @file  MacStackMSG.h

 @brief MAC stack messaging API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2013-2025, Texas Instruments Incorporated

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

#ifndef MACSTACKMSG_H
#define MACSTACKMSG_H

#include <stdbool.h>
#include <stdint.h>
#include "mac_api.h"

/* C API is included only in the case that stack owner decides
 * to support legacy C API.
 * That is API definition is split into C API header file and
 * message API header file.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Key source maximum length in bytes */
#define MAC_KEY_SOURCE_MAX_LEN      8
#define SADDR_EXT_LEN   8

/**
 * Event message header.
 * This is to support ICALL_CMD_EVENT
 */
typedef struct _timacmsg_hdr_t
{
  /** event */
  uint_least8_t event;
  /** status */
  uint_least8_t status;
} TimacMSG_HDR;


  /**
 * MAC_InitCommon command message common header.
 */
typedef struct _timacmsg_maccommoncmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
} TimacMSG_MacCommonCmd;


/**
 * MAC_StackInitParams command message common header.
 */
typedef struct _timacmsg_macstackinitparamscmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /**
   * Source task ID internally used by stack.
   * Because of this field, the message has to be
   * sent to TIMAC stack using @ref ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID.
   */
  uint_least8_t srctaskid;
  /** retransmit option, 0 do not continue retx, 1 continue */
  uint_least8_t retransmit;
  /** pending indirect MAC msg, 0 no pending msg. */
  uint_least8_t pendingMsg;
  /** query retransmit callback */
  uint_least8_t (*pMacCbackQueryRetransmit)();
  /** check pending callback */
  uint_least8_t (*pMacCbackCheckPending)();
} TimacMSG_MacStackInitParamsCmd;


/**
 * MAC_Init command message common header.
 */
typedef struct _timacmsg_macresetreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** default Pib */
  uint_least8_t setDefaultPib;
  /** result status */
  uint_least8_t status;
} TimacMSG_MacResetReqCmd;

/**
 * sdata structure .
 */
typedef struct _timac_sdata_t
{
  uint_least8_t       *p;
  uint_least16_t      len;
} Timac_sData_t;

/**
 * timac security structure .
 */
typedef struct _timac_macsec_t
{
  /** Key source */
  uint_least8_t  keySource[MAC_KEY_SOURCE_MAX_LEN];
  /** Security level */
  uint_least8_t  securityLevel;
  /** Key identifier mode */
  uint_least8_t  keyIdMode;
  /** Key index */
  uint_least8_t  keyIndex;
} Timac_MacSec_t;


/**
 * short/extended address array .
 */
typedef uint_least8_t  Timac_sAddrExt_t[SADDR_EXT_LEN];


/**
 * Combined short/extended device address
 */
typedef struct _timac_saddr_t
{
  union
  {
    /** Short address */
    uint_least16_t shortAddr;
    /** Extended address */
    Timac_sAddrExt_t extAddr;
  } addr;
  /** Address mode */
  uint_least8_t addrMode;
} Timac_sAddr_t;

/**
 * For internal use only
 */
typedef struct _timac_payloadIeInfo
{
  uint_least8_t         gId;
  union
  {
    uint_least8_t       id[MAC_PAYLOAD_SUB_IE_MAX];
    uint_least32_t      subIdBMap;
  } subId;
} Timac_PayloadIeInfo_t;

typedef struct _timac_headerIeInfo
{
  uint_least8_t         elementId;
  uint_least32_t        subIdBitMap;

} Timac_HeaderIeInfo_t;

typedef struct _timac_mactxtIntdata_t
{
  uint_least32_t        timestamp;
  uint_least16_t        timestamp2;
  uint_least16_t        timeToLive;
  uint_least8_t         frameType;
  uint_least16_t        txOptions;
  uint_least8_t         txMode;
  uint_least8_t         txSched;
  uint_least8_t         retries;
  uint_least8_t         channel;
  uint_least8_t         power;
  uint_least8_t         mpduLinkQuality;
  uint_least8_t         correlation;
  int_least8_t          rssi;
#ifdef FEATURE_MAC_SECURITY
  uint_least8_t*        ptrMData;       /* pointer to MData */
  keyDescriptor_t*      pKeyDesc;       /* pointer to the key descriptor */
  uint_least32_t        frameCntr;      /* frame counter */
#endif
  Timac_PayloadIeInfo_t payloadIeInfo[MAC_PAYLOAD_IE_MAX];
  Timac_HeaderIeInfo_t  headerIeInfo[MAC_HEADER_IE_MAX];
  uint_least16_t        totalLen;       /* total length of the allocated msdu buffer */
  uint_least8_t*        ptrHeaderIE;    /* pointer to the start of header IE field */
  uint_least16_t        headerIesLen;   /* length of header ie's */
  uint_least8_t*        ptrPayloadIE;   /* pointer to the start of Payload IE field */
  uint_least8_t*        ptrUTIE;        /* pointer to the UT IE Content */
  uint_least8_t*        ptrBTIE;        /* pointer to the BT IE Content */
  uint_least8_t*        ptrFrameCnt;    /* pointer to the frameCnt */
  uint_least8_t         rsl;            /* update with the information from RSL-IE */
  uint_least8_t         nb;             /* number of backoffs */
  uint_least8_t         be;             /* backoff exponent */
  uint_least8_t         fhFrameType;    /* Type of WiSun frame */
  union
  {
    Timac_sAddrExt_t    destAddr;       /* Destination address, extended */
    uint_least8_t       chList[MAC_154G_CHANNEL_BITMAP_SIZ]; /* Channel List as a bitmap */
  } dest;

} Timac_MacTxIntData_t;


/**
 * For internal use only
 */
typedef struct _timac_macrxintdata_t
{
  uint_least16_t  fcf;
  uint_least8_t   fhFrameType;
  uint_least8_t   cmdFrameId;
  uint_least8_t   flags;
#ifdef FEATURE_MAC_SECURITY
  uint_least8_t*  ptrCData;
#endif
  uint32          frameCntr;
} Timac_MacRxIntData_t;


/**
 * Data request parameters type
 */
typedef struct _timac_macdatareq_t
{
  /** The address of the destination device */
  Timac_sAddr_t dstAddr;
  /** The PAN ID of the destination device */
  uint_least16_t dstPanId;
  /** The source address mode */
  uint_least8_t srcAddrMode;
  /** Application-defined handle value associated with this data request */
  uint_least8_t msduHandle;
  /** TX options bit mask */
  uint_least16_t txOptions;
  /** Transmit the data frame on this channel */
  uint_least8_t channel;
  /** Transmit the data frame at this power level */
  uint_least8_t power;
  /** Pointer to the payload IE list, excluding termination IEs */
  uint_least8_t* pIEList;
  /** length of the payload IEs */
  uint_least16_t payloadIELen;
  /** Not used, RESERVED for future. The value shall be set to zero(0) */
  uint_least8_t fhProtoDispatch;
  /** Bitmap indicates which FH IE's need to be included */
  uint_least32_t includeFhIEs;
} Timac_MacDataReq_t;


/**
 * MCPS data request type
 */
typedef struct _timac_macmcpsdatareq_t
{
  /** Internal use only */
  TimacMSG_HDR hdr;
  /** Data pointer and length */
  Timac_sData_t msdu;
  /** Internal use only */
  Timac_MacTxIntData_t internal;
  /** Security parameters */
  Timac_MacSec_t sec;
  /** Data request parameters */
  Timac_MacDataReq_t mac;
} Timac_MacMcpsDataReq_t;


/**
 * Data indication parameters type
 */
typedef struct _timac_macdataind_t
{
  /** The address of the sending device */
  Timac_sAddr_t srcAddr;
  /** The address of the destination device */
  Timac_sAddr_t  dstAddr;
  /** The time, in backoffs, at which the data were received */
  uint_least32_t timestamp;
  /** The time, in internal MAC timer units, at which the
   *  data were received */
  uint_least16_t timestamp2;
  /** The PAN ID of the sending device */
  uint_least16_t srcPanId;
  /** The PAN ID of the destination device */
  uint_least16_t dstPanId;
  /** The link quality of the received data frame */
  uint_least8_t mpduLinkQuality;
  /** The raw correlation value of the received data frame */
  uint_least8_t correlation;
  /** The received RF power in units dBm */
  int_least8_t rssi;
  /** The data sequence number of the received frame */
  uint_least8_t dsn;
  /** Pointer to the start of payload IE's */
  uint_least8_t*    pPayloadIE;
  /** total length of the payload IE's */
  uint_least16_t   payloadIeLen;
  /** Not used, RESERVED for future. */
  uint_least8_t     fhProtoDispatch;
  /* Frame counter value (if used) of the received data frame */
  uint_least32_t    frameCntr;
} Timac_MacDataInd_t;


/**
 * MCPS data indication type
 */
typedef struct _timacmsg_macmcpsdataind_t
{
  /** Internal use only */
  TimacMSG_HDR hdr;
  /** Data pointer and length */
  Timac_sData_t msdu;
  /** Internal use only */
  Timac_MacRxIntData_t internal;
  /** Security parameters */
  Timac_MacSec_t sec;
  /** Data indication parameters */
  Timac_MacDataInd_t mac;
} TimacMSG_MacMcpsDataInd_t;


/**
 * MCPS data confirm type
 */
typedef struct _timacmsg_macmcpsdatacnf_t
{
  /** Contains the status of the data request operation */
  TimacMSG_HDR  hdr;
  /** Application-defined handle value associated with the data request */
  uint_least8_t msduHandle;
   /** Pointer to the data request buffer for this data confirm */
  macMcpsDataReq_t *pDataReq;
  /** The time, in backoffs, at which the frame was transmitted */
  uint_least32_t timestamp;
  /** The time, in internal MAC timer units, at which the
   * frame was transmitted */
  uint_least16_t timestamp2;
  /** The number of retries required to transmit the data frame */
  uint_least8_t retries;
  /** The link quality of the received ack frame */
  uint_least8_t mpduLinkQuality;
  /** The raw correlation value of the received ack frame */
  uint_least8_t correlation;
  /** The RF power of the received ack frame in units dBm */
  int_least8_t rssi;
  /* Frame counter value (if used) of the received data frame */
  uint_least32_t    frameCntr;
} TimacMSG_MacMcpsDataCnf_t;


/**
 * MCPS purge confirm type
 */
typedef struct _timacmsg_macmcpspurgecnf_t
{
  TimacMSG_HDR        hdr;         /** Contains the status of the purge request operation */
  uint_least8_t       msduHandle;  /** Application-defined handle value associated with the data request */
} TimacMSG_MacMcpsPurgeCnf_t;


/**
 * Pan descriptor structure
 */
typedef struct _timac_macpanDesc_t
{
  /** The address of the coordinator sending the beacon */
  Timac_sAddr_t  coordAddress;
  /** The PAN ID of the network */
  uint_least16_t coordPanId;
  /** The superframe specification of the network */
  uint_least16_t superframeSpec;
  /** The logical channel of the network */
  uint_least8_t  logicalChannel;
  /** The current channel page occupied by the network */
  uint_least8_t  channelPage;
  /** TRUE if coordinator accepts GTS requests */
  bool           gtsPermit;
  /** The link quality of the received beacon */
  uint_least8_t  linkQuality;
  /** The time at which the beacon was received, in backoffs */
  uint_least32_t timestamp;
  /** Set to TRUE if there was an error in the security processing */
  bool           securityFailure;
  /** The security parameters for the received beacon frame */
  Timac_MacSec_t sec;
} Timac_MacPanDesc_t;


/**
 * MLME poll request structure
 */
typedef struct _timac_macmlmepollreq_t
{
  /** he address of the coordinator device to poll */
  Timac_sAddr_t   coordAddress;
  /** The PAN ID of the coordinator */
  uint_least16_t  coordPanId;
  /** The security parameters for this message */
  Timac_MacSec_t  sec;
} Timac_MacMlmePollReq_t;


/**
 * MAC_MlmePollReqMsg command message common header.
 */
typedef struct _timacmsg_pollreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME Poll Request structure */
  Timac_MacMlmePollReq_t macMlmePollReq;
} TimacMSG_PollReqCmd;


/**
 * MCPS purge request structure
 */
typedef struct _timac_macmcpspurgereq_t
{
  /** application-defined handle value associated with the data request */
  uint_least8_t msduHandle;
} Timac_MacMcpsPurgeReq_t;


/**
 * MAC_McpsPurgeReqMsg command message common header.
 */
typedef struct _timacmsg_purgereqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MCPS Purge Request structure */
  Timac_MacMcpsPurgeReq_t macMcpsPurgeReq;
} TimacMSG_PurgeReqCmd;


/**
 * MLME Scan Request structure
 */
typedef struct _timac_macmlmescanreq_t
{
  /** Bit mask indicating which channels to scan */
  uint_least8_t  scanChannels[MAC_154G_CHANNEL_BITMAP_SIZ];
  /** The type of scan */
  uint_least8_t  scanType;
  /** The exponent used in the scan duration calculation */
  uint_least8_t  scanDuration;
  /** The channel page on which to perform the scan */
  uint_least8_t  channelPage;
  /* PHY ID corresponding to the PHY descriptor to use */
  uint_least8_t  phyID;
  /** The maximum number of PAN descriptor results */
  uint_least8_t  maxResults;
  /** Only devices with permit joining on respond to the
   * enahnced beacon request */
  bool           permitJoining;
  /** The device will respond to the enhanced beacon request
   * if mpduLinkQuality is equal or higher than this value */
  uint_least8_t  linkQuality;
  /** The device will then randomly determine if it is to
   * respond to the enhanced beacon request based on meeting
   * this probability (0 to 100%). */
  uint_least8_t  percentFilter;
  /** The security parameters for orphan scan */
  Timac_MacSec_t sec;
  /** When TRUE, scanDuration is ignored. When FALSE, scan duration
   *  shall be set to scanDuration; MPMScanDuration is ignored */
  bool           MPMScan;
  /** BPAN or NBPAN */
  uint_least8_t  MPMScanType;
  /** If MPMScanType is BPAN, MPMScanDuration values are 0-14.
   *  It is used in determining the max time spent scanning
   *  for an EB in a beacon enabled PAN on the channel.
   *  [aBaseSuperframeDuration * 2^n symbols], where
   *   n is the MPMScanDuration.
   *  If MPMScanType is NBPAN, valid values are 1 - 16383.
   *  It is used in determining the max time spent scanning for an
   *  EB in nonbeacon-enabled PAN on the channel.
   *  [aBaseSlotDuration * n] symbols, where n
   *  is MPMScanDuration. */
  uint_least16_t MPMScanDuration;
  union {
    /** Pointer to a buffer to store energy detect measurements */
    uint_least8_t *pEnergyDetect;
     /** Pointer to a buffer to store PAN descriptors */
    Timac_MacPanDesc_t *pPanDescriptor;
  } result;
} Timac_MacMlmeScanReq_t;


/**
 * MAC_MlmeScanReqMsg command message common header.
 */
typedef struct _timacmsg_scanreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME Scan Request structure */
  Timac_MacMlmeScanReq_t macMlmeScanReq;
} TimacMSG_ScanReqCmd;

/* MPM(Multi-PHY layer management) parameters */
typedef struct _timac_mpmParams
{
  uint8       eBeaconOrder;       /* The exponent used to calculate the enhanced beacon interval.
                                     A value of 15 indicates no EB in a beacon enabled PAN */
  uint8       offsetTimeSlot;     /* Indicates the time diff between the EB and the preceeding periodic Beacon */
  uint16      NBPANEBeaconOrder;  /* Indicates how often the EB to tx in a non-beacon enabled PAN. A value of
                                     16383 indicates no EB in a non-beacon enabled PAN */
  uint8*      pIEIDs;             /* pointer to the buffer containing the Information element IDs which needs
                                     to be sent in Enhanced Beacon */
  uint8       numIEs;             /* number of Information Elements in the buffer */

} Timac_macMpmParams_t;


/**
 * MLME start request type
 */
typedef struct _timac_macmlmestartreq_t
{
  /** The time to begin transmitting beacons relative to the received beacon */
  uint_least32_t startTime;
  /** The PAN ID to use.  This parameter is ignored if panCoordinator is FALSE */
  uint_least16_t panId;
  /** The logical channel to use.  This parameter is ignored if panCoordinator is FALSE */
  uint_least8_t  logicalChannel;
  /** The channel page to use.  This parameter is ignored if panCoordinator is FALSE */
  uint_least8_t  channelPage;
  /** PHY ID corresponding to the PHY descriptor to use */
  uint_least8_t  phyID;
  /** The exponent used to calculate the beacon interval */
  uint_least8_t  beaconOrder;
  /** The exponent used to calculate the superframe duration */
  uint_least8_t  superframeOrder;
  /** Set to TRUE to start a network as PAN coordinator */
  bool  panCoordinator;
  /** If this value is TRUE, the receiver is disabled after MAC_BATT_LIFE_EXT_PERIODS
   * full backoff periods following the interframe spacing period of the beacon frame */
  bool  batteryLifeExt;
  /** Set to TRUE to transmit a coordinator realignment prior to changing
   * the superframe configuration */
  bool  coordRealignment;
  /** Security parameters for the coordinator realignment frame */
  Timac_MacSec_t    realignSec;
  /** Security parameters for the beacon frame */
  Timac_MacSec_t    beaconSec;
  /** MPM (multi-PHY layer management) parameters */
  Timac_macMpmParams_t mpmparams;
  /** Indicates whether frequency hopping needs to be started */
  bool startFH;
} Timac_MacMlmeStartReq_t;


/**
 * MAC_MlmeStartReqMsg command message common header.
 */
typedef struct _timacmsg_startreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME Start Request structure */
  Timac_MacMlmeStartReq_t macMlmeStartReq;
} TimacMSG_StartReqCmd;


/**
 * MLME Sync request type
 */
typedef struct
{
  /** The logical channel to use */
  uint_least8_t logicalChannel;
  /** The channel page to use */
  uint_least8_t channelPage;
  /** PHY ID corresponding to the PHY descriptor to use */
  uint8       phyID;
  /** Set to TRUE to continue tracking beacons after synchronizing with the
    * first beacon.  Set to FALSE to only synchronize with the first beacon */
  uint_least8_t trackBeacon;
} Timac_MacMlmeSyncReq_t;


/**
 * MAC_MlmeSyncReqMsg command message common header.
 */
typedef struct _timacmsg_syncreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME Sync Request structure */
  Timac_MacMlmeSyncReq_t macMlmeSyncReq;
} TimacMSG_SyncReqCmd;

/**
 *  MLME WiSUN Async request type
 */
typedef struct _timac_macMlmeWSAsyncReq
{
  Timac_MacSec_t sec;    /* The security parameters */
  uint_least8_t operation; /* Start or Stop Async operation */
  uint_least8_t frameType; /* Async frame type */
  uint_least8_t channels[MAC_154G_CHANNEL_BITMAP_SIZ]; /* Bit Mask indicating which channels to send
                                                          the Async frames for the start operation */
} Timac_MacMlmeWSAsyncReq_t;

/**
 * MAC_MlmeWSAsyncReqMsg command message common header.
 */
typedef struct _timacmsg_WSAsyncreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME WiSUN Async Request structure */
  Timac_MacMlmeWSAsyncReq_t macMlmeWSAsyncReq;
} TimacMSG_WSAsyncReqCmd;


/**
 * MAC_SetReqMsg command message common header.
 */
typedef struct _timacmsg_setreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** parameter ID */
  uint_least8_t  paramID;
  /** Value for the parameter ID */
  void  *paramValue;
} TimacMSG_MacSetReqCmd;



/**
 * MAC_SetReqMsg command message common header.
 */
typedef struct _timacmsg_getreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** parameter ID */
  uint_least8_t paramID;
  /** Length in bytes of parameter Value */
  uint_least8_t len;
  /** Value for the parameter ID */
  uint_least8_t *pValue;
} TimacMSG_MacGetReqCmd;

/**
 * MAC_MlmeGetPointerSecurityReq command message common header.
 */
typedef struct _timacmsg_getsecurityptrreqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** parameter ID */
  uint_least8_t paramID;
  /** Length in bytes of parameter Value */
  uint_least8_t len;
  /** Value for the parameter ID */
  void **pValue;
} TimacMSG_MacSecurityGetPtrReqCmd;


/**
 * MAC_RandomByte command message common header.
 */
typedef struct _timacmsg_randbytecmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Random Byte */
  uint_least8_t  randByte;
} TimacMSG_MacRandByteCmd;


/**
 * MLME associate request type
 */
typedef struct _timac_macmlmeassociatereq_t
{
  /** The security parameters for this message */
  Timac_MacSec_t sec;
  /** The channel on which to attempt association */
  uint_least8_t  logicalChannel;
  /** The channel page on which to attempt association */
  uint_least8_t channelPage;
  /** Identifier for the PHY descriptor */
  uint_least8_t phyID;
  /** Address of the coordinator with which to associate */
  Timac_sAddr_t coordAddress;
  /** The identifier of the PAN with which to associate */
  uint_least16_t coordPanId;
  /** The operational capabilities of this device */
  uint_least8_t capabilityInformation;
} Timac_MacMlmeAssociateReq_t;


/**
 * MAC_MlmeAssociateReqMsg command message common header.
 */
typedef struct _timacmsg_associatereqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME Sync Request structure */
  Timac_MacMlmeAssociateReq_t macMlmeAssociateReq;
} TimacMSG_AssociateReqCmd;


/**
 * MLME associate response type
 */
typedef struct _timac_macmlmeassociatersp_t
{
  /** The security parameters for this message */
  Timac_MacSec_t sec;
  /** The address of the device requesting association */
  Timac_sAddrExt_t deviceAddress;
  /** The short address allocated to the device */
  uint_least16_t assocShortAddress;
  /** The status of the association attempt */
  uint_least8_t status;
} Timac_MacMlmeAssociateRsp_t;


/**
 * MAC_MlmeAssociateRspMsg command message common header.
 */
typedef struct _timacmsg_associaterspcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME Association response structure */
  Timac_MacMlmeAssociateRsp_t macMlmeAssociateRsp;
} TimacMSG_AssociateRspCmd;


/**
 * MAC_MLME_ASSOCIATE_IND type
 */
typedef struct _timacmsg_macmlmeassociateind_t
{
  /** The event header */
  TimacMSG_HDR hdr;
  /** The address of the device requesting association */
  Timac_sAddrExt_t deviceAddress;
  /** The operational capabilities of the device requesting association */
  uint_least8_t capabilityInformation;
  /** The security parameters for this message */
  Timac_MacSec_t sec;
} TimacMSG_MacMlmeAssociateInd_t;


/**
 * MAC_MLME_ASSOCIATE_CNF type
 */
typedef struct _timacmsg_macmlmeassociatecnf_t
{
  /** Event header contains the status of the associate attempt */
  TimacMSG_HDR hdr;
  /** If successful, the short address allocated to this device */
  uint_least16_t assocShortAddress;
  /** The security parameters for this message */
  Timac_MacSec_t sec;
} TimacMSG_MacMlmeAssociateCnf_t;


/**
 * MLME disassociate request type
 */
typedef struct _timac_macmlmedisassociatereq_t
{
  /** The security parameters for this message */
  Timac_MacSec_t sec;
  /** The address of the device with which to disassociate */
  Timac_sAddr_t  deviceAddress;
  /** The PAN ID of the device */
  uint_least16_t devicePanId;
  /** The disassociate reason */
  uint_least8_t  disassociateReason;
  /** Transmit Indirect */
  bool           txIndirect;
} Timac_MacMlmeDisassociateReq_t;


/**
 * MAC_MlmeDisassociateReqMsg command message common header.
 */
typedef struct _timacmsg_disassociatereqcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME Sync Request structure */
  Timac_MacMlmeDisassociateReq_t macMlmeDisassociateReq;
} TimacMSG_DisassociateReqCmd;


/**
 * MAC_MLME_DISASSOCIATE_IND type
 */
typedef struct _timacmsg_macmlmedisassociateind_t
{
  /** The event header */
  TimacMSG_HDR hdr;
  /** The address of the device sending the disassociate command */
  Timac_sAddrExt_t deviceAddress;
  /** The disassociate reason */
  uint_least8_t disassociateReason;
   /** The security parameters for this message */
  Timac_MacSec_t sec;
} TimacMSG_MacMlmeDisassociateInd_t;


/**
 * MAC_MLME_DISASSOCIATE_CNF type
 */
typedef struct _timacmsg_macmlmedisassociatecnf_t
{
  /** Event header contains the status of the disassociate attempt */
  TimacMSG_HDR hdr;
  /** The address of the device that has either requested disassociation
   * or been instructed to disassociate by its coordinator */
  Timac_sAddr_t deviceAddress;
  /** The pan ID of the device that has either requested disassociation
   * or been instructed to disassociate by its coordinator */
  uint_least16_t panId;
} TimacMSG_MacMlmeDisassociateCnf_t;


/* MAC Beacon data type */
typedef struct _timac_macBeaconData
{
  Timac_MacPanDesc_t   *pPanDesc;       /* The PAN descriptor for the received beacon */
  uint_least8_t  pendAddrSpec;    /* The beacon pending address specification */
  uint_least8_t  *pAddrList;      /* The list of device addresses for which the sender of the beacon has data */
  uint_least8_t  sduLength;       /* The number of bytes in the beacon payload of the beacon frame */
  uint_least8_t  *pSdu;           /* The beacon payload */
} Timac_MacBeaconData_t;

/* Coexistence Information element content type */
typedef struct _timac_macCoexist
{
  uint_least8_t beaconOrder;       /* Beacon Order field shall specify the transmission interval of the beacon */
  uint_least8_t superFrameOrder;   /* Superframe Order field shall specify the length of time during which the superframe
                                      is active (i.e., receiver enabled), including the Beacon frame transmission time*/
  uint_least8_t finalCapSlot;
  uint_least8_t eBeaconOrder;      /* Enhanced Beacon Order field specifies the transmission interval
                                      of the Enhanced Beacon frames in a beacon enabled network */
  uint_least8_t offsetTimeSlot;    /* time offset between periodic beacon and the Enhanced Beacon */
  uint_least8_t capBackOff;        /* actual slot position in which the Enhanced Beacon frame is
                                      transmitted due to the backoff procedure in the CAP.*/
  uint_least16_t eBeaconOrderNBPAN; /* NBPAN Enhanced Beacon Order field specifies the transmission interval between consecutive
                                      Enhanced Beacon frames in the nonbeacon-enabled mode */
} Timac_MacCoexist_t;

/* MAC Enhanced beacon data type */
typedef struct _timac_macEBeaconData
{
  Timac_MacPanDesc_t *pPanDesc;       /* The PAN descriptor for the received beacon */
  Timac_MacCoexist_t coexist;         /* Coexistence IE data */
} Timac_macEBeaconData_t;

/**
 * MAC_MLME_BEACON_NOTIFY_IND type
 */
typedef struct _timacmsg_macmlmebeaconnotifyind_t
{
  /** The event header */
  TimacMSG_HDR hdr;
  /** Indicates the beacon type: beacon or enhanced beacon */
  uint_least8_t beaconType;
  /** The beacon sequence number or enhanced beacon sequence
   *  number */
  uint_least8_t bsn;
  union
  {
    Timac_MacBeaconData_t beaconData;
    Timac_macEBeaconData_t eBeaconData;
  } info;
} TimacMSG_MacMlmeBeaconNotifyInd_t;


/**
 * MAC_MLME_ORPHAN_IND type
 */
typedef struct _timacmsg_macmlmeorphanind_t
{
  /** The event header */
  TimacMSG_HDR  hdr;
  /** The address of the orphaned device */
  Timac_sAddrExt_t orphanAddress;
  /** The security parameters for this message */
  Timac_MacSec_t sec;
} TimacMSG_MacMlmeOrphanInd_t;


/**
 * MLME orphone response type
 */
typedef struct _timac_macmorphanresponse_t
{
  /** The security parameters for this message */
  Timac_MacSec_t    sec;
  /** The extended address of the device sending the orphan notification */
  Timac_sAddrExt_t  orphanAddress;
  /** The short address of the orphaned device */
  uint_least16_t    shortAddress;
  /** Set to TRUE if the orphaned device is associated with this coordinator */
  bool              associatedMember;
} Timac_MacMlmeOrphanRsp_t;


/**
 * MAC_MlmeOrphanRsp command message common header.
 */
typedef struct _timacmsg_orphanrspcmd_t
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** Refer to MLME Sync Request structure */
  Timac_MacMlmeOrphanRsp_t macMlmeOrphanRsp;
} TimacMSG_OrphanRspCmd;


/**
 * MAC_MLME_SCAN_CNF type
 */
typedef struct _timacmsg_macmlmescancnf_t
{
  /** Event header contains the status of the scan request */
  TimacMSG_HDR hdr;
  /** The type of scan requested */
  uint_least8_t scanType;
  /** The channel page of the scan */
  uint_least8_t channelPage;
  /** PHY ID corresponding to the PHY descriptor used during scan */
  uint_least8_t phyID;
  /** Bit mask of channels that were not scanned */
  uint_least8_t unscannedChannels[MAC_154G_CHANNEL_BITMAP_SIZ];
  /** The number of PAN descriptors returned in the results list */
  uint_least8_t resultListSize;
  union
  {
    /** The list of energy measurements, one for each channel scanned */
    uint_least8_t *pEnergyDetect;
    /** The list of PAN descriptors, one for each beacon found */
    Timac_MacPanDesc_t *pPanDescriptor;
  } result;
} TimacMSG_MacMlmeScanCnf_t;


/**
 * MAC_MLME_START_CNF type
 */
typedef struct
{
   /** Event header contains the status of the start request */
  TimacMSG_HDR hdr;
} TimacMSG_MacMlmeStartCnf_t;


/**
 * MAC_MLME_SYNC_LOSS_IND type
 */
typedef struct _timacmsg_macmlmesynclossind_t
{
  /** Event header contains the reason that synchronization was lost */
  TimacMSG_HDR hdr;
  /** The PAN ID of the realignment */
  uint_least16_t panId;
  /** The logical channel of the realignment */
  uint_least8_t logicalChannel;
  /** The channel page of the realignment */
  uint_least8_t channelPage;
  /* PHY ID corresponding to the PHY descriptor of the realignment */
  uint_least8_t phyID;
  /** The security parameters for this message */
  Timac_MacSec_t sec;
} TimacMSG_MacMlmeSyncLossInd_t;


/**
 * MAC_MLME_POLL_CNF type
 */
typedef struct _timacmsg_macmlmepollcnf_t
{
  /** Event header contains the status of the poll request */
  TimacMSG_HDR hdr;
  uint_least8_t framePending;
} TimacMSG_MacMlmePollCnf_t;


/**
 * MAC_MLME_COMM_STATUS_IND type
 */
typedef struct _timacmsg_macmlmecommstatusind_t
{
  /** Event header contains the status for this event */
  TimacMSG_HDR hdr;
  /** The source address associated with the event */
  Timac_sAddr_t srcAddr;
  /** The destination address associated with the event */
  Timac_sAddr_t dstAddr;
  /** The PAN ID associated with the event */
  uint_least16_t panId;
  /** The reason the event was generated */
  uint_least8_t reason;
  /** The security parameters for this message */
  Timac_MacSec_t sec;
} TimacMSG_MacMlmeCommStatusInd_t;


/**
 * MAC_MLME_POLL_IND type
 */
typedef struct _timacmsg_macmlmepollind_t
{
  /** Event header contains the status for this event */
  TimacMSG_HDR hdr;
  /** Short address of the device sending the data request */
  Timac_sAddr_t srcAddr;
  /** Pan ID of the device sending the data request */
  uint_least16_t srcPanId;
  /** indication that no MAC_McpsDataReq() is required.
   * It is set when MAC_MLME_POLL_IND is generated,
   * to simply indicate that a received data request frame
   * was acked with pending bit cleared. */
  uint_least8_t noRsp;
} TimacMSG_MacMlmePollInd_t;

/**
 * MAC_FHSetReqMsg command message common header.
 */
typedef struct _timacmsg_fhsetreqcmd
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** parameter ID */
  uint_least16_t  paramID;
  /** Value for the parameter ID */
  void  *paramValue;
} TimacMSG_MacFHSetReqCmd;



/**
 * MAC_FHSetReqMsg command message common header.
 */
typedef struct _timacmsg_fhgetreqcmd
{
  /** message header<br>
   * event field must be set to @ref ICALL_CMD_EVENT. */
  TimacMSG_HDR hdr;
  /** parameter ID */
  uint_least16_t paramID;
  /** Length in bytes of parameter Value */
  uint_least8_t len;
  /** Value for the parameter ID */
  uint_least8_t *pValue;
} TimacMSG_MacFHGetReqCmd;

/**
 * MAC_MLME_WS_ASYNC_FRAME_IND type
 */
typedef TimacMSG_MacMcpsDataInd_t TimacMSG_MacMlmeWSAsyncInd_t;

/* MAC_MLME_WS_ASYNC_FRAME_CNF type */
typedef struct _timacmsg_macMlmeWSAsyncCnf
{
  /** Event header contains the status for this event */
  TimacMSG_HDR     hdr;

} TimacMSG_MacMlmeWSAsyncCnf_t;



/**
 * Union of callback structures
 */
typedef union
{
  TimacMSG_HDR                      hdr;
  /** MAC_MLME_ASSOCIATE_IND */
  TimacMSG_MacMlmeAssociateInd_t    associateInd;
  /** MAC_MLME_ASSOCIATE_CNF */
  TimacMSG_MacMlmeAssociateCnf_t    associateCnf;
  /** MAC_MLME_DISASSOCIATE_IND */
  TimacMSG_MacMlmeDisassociateInd_t disassociateInd;
  /** MAC_MLME_DISASSOCIATE_CNF */
  TimacMSG_MacMlmeDisassociateCnf_t disassociateCnf;
  /** MAC_MLME_BEACON_NOTIFY_IND */
  TimacMSG_MacMlmeBeaconNotifyInd_t beaconNotifyInd;
  /** MAC_MLME_ORPHAN_IND */
  TimacMSG_MacMlmeOrphanInd_t       orphanInd;
  /** MAC_MLME_SCAN_CNF */
  TimacMSG_MacMlmeScanCnf_t         scanCnf;
  /** MAC_MLME_START_CNF */
  TimacMSG_MacMlmeStartCnf_t        startCnf;
  /** MAC_MLME_SYNC_LOSS_IND */
  TimacMSG_MacMlmeSyncLossInd_t     syncLossInd;
  /** MAC_MLME_POLL_CNF */
  TimacMSG_MacMlmePollCnf_t         pollCnf;
  /** MAC_MLME_COMM_STATUS_IND */
  TimacMSG_MacMlmeCommStatusInd_t   commStatusInd;
  /** MAC_MLME_POLL_IND */
  TimacMSG_MacMlmePollInd_t         pollInd;
  /** MAC_MCPS_DATA_CNF */
  TimacMSG_MacMcpsDataCnf_t         dataCnf;
  /** MAC_MCPS_DATA_IND */
  TimacMSG_MacMcpsDataInd_t         dataInd;
  /** MAC_MCPS_PURGE_CNF */
  TimacMSG_MacMcpsPurgeCnf_t        purgeCnf;
  /** MAC_MLME_WS_ASYNC_FRAME_IND */
  TimacMSG_MacMlmeWSAsyncInd_t      asyncInd;
  /** MAC_MLME_WS_ASYNC_FRAME_CNF */
  TimacMSG_MacMlmeWSAsyncCnf_t      asyncCnf;
} TimacMSG_MacCbackEvent_t;


/**
 * SourceMatch Add or delete command message common header.
 */
typedef struct _timacmsg_macsrcmatchentry_t
{
  /** Event header contains the status for this event */
  TimacMSG_HDR hdr;
  /** Source Match Address */
  Timac_sAddr_t *addr;
  /** PanId */
  uint_least16_t  panID;
} TimacMSG_MacSrcMatchEntry_t;

/**
 * SourceMatch Ack all Pending
 */
typedef struct _timacmsg_macsrcmatackallpending_t
{
  /** Event header contains the status for this event */
  TimacMSG_HDR hdr;
  /** Ack all frames as pending since source matching table is full */
  uint_least8_t option;
} TimacMSG_MacSrcMatchAckAllPending_t;


/**
 * SourceMatch Check all Pending
 */
typedef struct _timacmsg_macsrcmatcheckallpending_t
{
  /** Event header contains the status for this event */
  TimacMSG_HDR hdr;
  /** Status of Pend */
  uint_least8_t pend;
} TimacMSG_MacSrcMatchCheckAllPending_t;


/**
 * MCPS Data Alloc command message common header.
 */
typedef struct _timacmsg_macmcpsdataalloc_t
{
  /** Event header contains the status for this event */
  TimacMSG_HDR hdr;
  uint_least16_t len;
  uint_least8_t securityLevel;
  uint_least8_t keyIdMode;
  uint_least32_t includeFhIEs;
  uint_least16_t payloadIeLen;
  Timac_MacMcpsDataReq_t *pDataReq;
} TimacMSG_MacMcpsDataAlloc_t;


void MAC_MlmeScanReqMsg(Timac_MacMlmeScanReq_t *pData);


void MAC_MlmeStartReqMsg(Timac_MacMlmeStartReq_t *pData);


void MAC_MlmeSyncReqMsg(Timac_MacMlmeSyncReq_t *pData);


void MAC_McpsPurgeReqMsg(Timac_MacMcpsPurgeReq_t *pData);


void MAC_MlmeAssociateReqMsg(Timac_MacMlmeAssociateReq_t *pData);


void MAC_MlmeDisassociateReqMsg(Timac_MacMlmeDisassociateReq_t *pData);


void MAC_MlmeOrphanRspMsg(Timac_MacMlmeOrphanRsp_t *pData);


void MAC_MlmeAssociateRspMsg(Timac_MacMlmeAssociateRsp_t *pData);


void MAC_MlmePollReqMsg(Timac_MacMlmePollReq_t *pData);


void MAC_MlmeWSAsyncReqMsg(Timac_MacMlmeWSAsyncReq_t *pData);


void *sAddrExtCpy(uint_least8_t * pDest, const uint_least8_t * pSrc);


Timac_MacMcpsDataReq_t *MAC_McpsDataAllocMsg(uint_least16_t len,
                                             uint_least8_t securityLevel,
                                             uint_least8_t keyIdMode,
                                             uint_least32_t includeFhIEs,
                                             uint_least16_t payloadIeLen);


void MAC_McpsDataReqMsg(Timac_MacMcpsDataReq_t *pData);


void MAC_MacMsgDeallocate(TimacMSG_HDR *pData);

uint8 MAC_MlmeGetPointerSecurityReq(uint8 pibAttribute, void **pValue);

void MAC_StackInitParamsMsg( void );

#ifdef __cplusplus
}
#endif

#endif /** MACSTACKMSG_H *
