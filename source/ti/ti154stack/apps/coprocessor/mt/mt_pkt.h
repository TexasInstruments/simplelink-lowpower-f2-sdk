/******************************************************************************

 @file  mt_pkt.h

 @brief Definitions of serialized MT packets

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
#ifndef MTPKT_H
#define MTPKT_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include "mt.h"
#include "api_mac.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SEC_API_256
#define MT_SEC_INDX_SIZE  2
#else
#define MT_SEC_INDX_SIZE  1
#endif

/******************************************************************************
 Typedefs - MT_MAC Requests/Responses Packed Structures
 *****************************************************************************/
/*! Packed serial command packet - AddDevice Req */
typedef struct
{
    /*! New device PAN ID */
    uint8_t panId[2];
    /*! New device short address */
    uint8_t shortAddr[2];
    /*! New device extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Initial frame counter */
    uint8_t frameCntr[4];
    /*! Minimum security override indicator */
    uint8_t exempt[1];
    /*! Key device descriptor uniqueDevice indicator */
    uint8_t unique[1];
    /*! Duplicate device entry indicator */
    uint8_t duplicate[1];
    /*! Key ID lookup data size indicator, 0=5 bytes, 1=9 bytes */
    uint8_t lookupSize[1];
    /*! Key ID lookup data */
    uint8_t lookupData[APIMAC_MAX_KEY_LOOKUP_LEN];
} MtPkt_addDevReq_t;

/*! Packed serial command packet - Associate Req */
typedef struct
{
    /*! Channel on which to associate */
    uint8_t logicalChan[1];
    /*! Channel page on which to associate */
    uint8_t channelPage[1];
    /*! PHY descriptor identifier */
    uint8_t phyId[1];
    /*! Coordinator address mode */
    uint8_t addrMode[1];
    /*! Coordinator extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Coordinator PAN ID */
    uint8_t panId[2];
    /*! Device operational capabilities bitmask */
    uint8_t capInfo[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_assocReq_t;

/*! Packed serial response packet - Associate Rsp */
typedef struct
{
    /*! Requestor extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Allocated short address */
    uint8_t shortAddr[2];
    /*! Association status */
    uint8_t status[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_assocRsp_t;

/*! Packed serial command packet - Async Req */
typedef struct
{
    /*! Async operation start/stop */
    uint8_t operation[1];
    /*! Async frame type */
    uint8_t frameType[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
    /*! Scan channels bitmask */
    uint8_t scanChannels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
} MtPkt_asyncReq_t;

/*! Packed serial command packet - Data Req */
typedef struct
{
    /*! Destination device address mode */
    uint8_t dstAddrMode[1];
    /*! Destination device address */
    uint8_t dstAddr[APIMAC_SADDR_EXT_LEN];
    /*! Destination device PAN ID */
    uint8_t dstPanId[2];
    /*! Source device address mode */
    uint8_t srcAddrMode[1];
    /*! Application-defined handle associated with this data frame */
    uint8_t msduHandle[1];
    /*! TX options bitmask */
    uint8_t txOptions[1];
    /*! Transmit channel for this data frame */
    uint8_t channel[1];
    /*! Transmit power for this data frame */
    uint8_t power[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
    /*! Bitmask indicates which FH IEs to include */
    uint8_t includeFhIEs[4];
    /*! Length of the MSDU payload */
    uint8_t msduLen[2];
    /*! Length of the payload IEs */
    uint8_t payloadIELen[2];
    /*! MSDU data block followed by IE data block */
    uint8_t data[];
} MtPkt_dataReq_t;

/*! Packed serial command packet - Delete Device Req */
typedef struct
{
    /*! Extended address of device to delete */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
} MtPkt_delDevReq_t;

/*! Packed serial command packet - Delete Key Req */
typedef struct
{
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_delKeyReq_t;

/*! Packed serial command packet - Disassociate Req */
typedef struct
{
    /*! Device address mode */
    uint8_t addrMode[1];
    /*! Device address */
    uint8_t dstAddr[APIMAC_SADDR_EXT_LEN];
    /*! Device PAN ID */
    uint8_t panId[2];
    /*! Reason for disassociate */
    uint8_t reason[1];
    /*! TX indirect indicator */
    uint8_t indirect[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_disAssReq_t;

/*! Packed serial command packet - Get MAC PIB Req */
typedef struct
{
    /*! MAC PIB attribute id */
    uint8_t attrId[1];
} MtPkt_getReq_t;

/*! Packed serial command packet - Get FH PIB Req */
typedef struct
{
    /*! MAC FH PIB attribute id */
    uint8_t attrId[2];
} MtPkt_getFhReq_t;

/*! Packed serial command packet - Get Security PIB Req */
typedef struct
{
    /*! MAC Security PIB attribute id */
    uint8_t attrId[1];
    /*! PIB attribute array index 1 */
    uint8_t index1[MT_SEC_INDX_SIZE];
    /*! PIB attribute array index 2 */
    uint8_t index2[MT_SEC_INDX_SIZE];
} MtPkt_getSecReq_t;

/*! Packed serial response packet - Orphan Rsp */
typedef struct
{
    /*! Orphaned device extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Orphaned device short address */
    uint8_t shortAddr[2];
    /*! Orphaned device associated to this coordinator indicaor */
    uint8_t member[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_orphanRsp_t;

/*! Packed serial command packet - Update PAN ID Req */
typedef struct
{
    /*! New PAN ID */
    uint8_t panid[2];
} MtPkt_panidReq_t;

/*! Packed serial command packet - Poll Req */
typedef struct
{
    /*! Coordinator address mode */
    uint8_t addrMode[1];
    /*! Coordinator extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Coordinator PAN ID */
    uint8_t panId[2];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_pollReq_t;

/*! Packed serial command packet - Purge Req */
typedef struct
{
    /*! Application-defined handle for previous data frame */
    uint8_t msduHandle[1];
} MtPkt_purgeReq_t;

/*! Packed serial command packet - Read Key Req */
typedef struct
{
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_rdKeyReq_t;

/*! Packed serial command packet - Reset Req */
typedef struct
{
    /*! Set default PIB indicator */
    uint8_t defaultPib[1];
} MtPkt_resetReq_t;

/*! Packed serial command packet - Reset Req */
typedef struct
{
    /*!  */
    uint8_t rxGain[1];
} MtPkt_rxgainReq_t;

/*! Packed serial command packet - Scan Req */
typedef struct
{
    /*! Requested scan type */
    uint8_t scanType[1];
    /*! Exponent used in the scan duration calculation */
    uint8_t scanDuration[1];
    /*! Channel page on which to perform the scan */
    uint8_t channelPage[1];
    /*! Set to current PHY ID in MAC layer */
    uint8_t phyId[1];
    /*! Maximum scan results to return */
    uint8_t maxResults[1];
    /*! Permit join indicator */
    uint8_t permitJoining[1];
    /*! Enhanced Beacon min link quality */
    uint8_t linkQuality[1];
    /*! Enhanced Beacon random filter percentage */
    uint8_t percentFilter[1];
    /*! MPM scan mode */
    uint8_t mpmScan[1];
    /*! MPM scan type */
    uint8_t mpmScanType[1];
    /*! Scan duration parameter based on MPM scan type */
    uint8_t mpmScanDuration[2];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
    /*! Scan channels bitmask */
    uint8_t scanChannels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
} MtPkt_scanReq_t;

/*! Packed serial command packet - Set MAC PIB Req */
typedef struct
{
    /*! MAC PIB attribute id */
    uint8_t attrId[1];
    /*! MAC PIB data block */
    uint8_t data[];
} MtPkt_setReq_t;

/*! Packed serial command packet - Set FH PIB Req */
typedef struct
{
    /*! MAC FH PIB attribute id */
    uint8_t attrId[2];
    /*! FH PIB data block */
    uint8_t data[];
} MtPkt_setFhReq_t;

/*! Packed serial command packet - Set Security PIB Req */
typedef struct
{
    /*! MAC Security PIB attribute id */
    uint8_t attrId[1];
    /*! PIB attribute array index 1 */
    uint8_t index1[MT_SEC_INDX_SIZE];
    /*! PIB attribute array index 2 */
    uint8_t index2[MT_SEC_INDX_SIZE];
    /*! Security PIB data block */
    uint8_t data[];
} MtPkt_setSecReq_t;

/*! Packed serial command packet - Start Req */
typedef struct
{
    /*! Time to begin transmitting beacons */
    uint8_t startTime[4];
    /*! PAN ID to use */
    uint8_t panId[2];
    /*! Logical channel to use */
    uint8_t logicalChannel[1];
    /*! Channel page to use */
    uint8_t channelPage[1];
    /*! PHY ID for the PHY descriptor to use */
    uint8_t phyId[1];
    /*! Beacon order to use */
    uint8_t beaconOrder[1];
    /*! Superframe order to use */
    uint8_t superframeOrder[1];
    /*! PAN coordinator indicator */
    uint8_t panCoordinator[1];
    /*! Battery life extension indicator */
    uint8_t batteryLifeExt[1];
    /*! Coordinator realignment TX indicator */
    uint8_t coordRealignment[1];
    /*! Realignment frame security key source */
    uint8_t keySourceRealign[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Realignment frame security Level */
    uint8_t securityLevelRealign[1];
    /*! Realignment frame security key identifier mode */
    uint8_t keyIdModeRealign[1];
    /*! Realignment frame security key index */
    uint8_t keyIndexRealign[1];
    /*! Beacon frame security key source */
    uint8_t keySourceBeacon[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Beacon frame security Level */
    uint8_t securityLevelBeacon[1];
    /*! Beacon frame security key identifier mode */
    uint8_t keyIdModeBeacon[1];
    /*! Beacon frame security key index */
    uint8_t keyIndexBeacon[1];
    /*! Frequency hopping enable indicator */
    uint8_t startFH[1];
    /*! Enhanced beacon interval exponent */
    uint8_t eBeaconOrder[1];
    /*! Enhanced beacon time offset */
    uint8_t ofsTimeSlot[1];
    /*! NBPAN enhanced beacon interval exponent */
    uint8_t eBeaconOrderNbPan[2];
    /*! Numbers of IEs */
    uint8_t numIEs[1];
    /*! List of IEs to include */
    uint8_t listIEs[];
} MtPkt_startReq_t;

/*! Packed serial command packet - Start Req */
typedef struct
{
    /*! Logical channel to use */
    uint8_t logicalChannel[1];
    /*! Channel page to use */
    uint8_t channelPage[1];
    /*! Continue tracking beacons indicator */
    uint8_t trackBeacon[1];
    /*! PHY ID for the PHY descriptor to use */
    uint8_t phyId[1];
} MtPkt_syncReq_t;

/*! Packed serial command packet - Write Key Req */
typedef struct
{
    /*! Duplicate key table entries indicator */
    uint8_t newKey[1];
    /*! Key index into security key table */
    uint8_t keyIndex[MT_SEC_INDX_SIZE];
    /*! Key data */
    uint8_t keyData[APIMAC_KEY_MAX_LEN];
    /*! Initial frame counter */
    uint8_t frameCounter[4];
    /*! Key ID lookup data size indicator, 0=5 bytes, 1=9 bytes */
    uint8_t lookupSize[1];
    /*! Key ID lookup data */
    uint8_t lookupData[APIMAC_MAX_KEY_LOOKUP_LEN];
} MtPkt_wrKeyReq_t;

/******************************************************************************
 Typedefs - MT_MAC Indications/Confirms Packed Structures
 *****************************************************************************/
/*! Packed serial indication packet - Async Data Ind */
typedef struct
{
    /*! Source address mode */
    uint8_t srcMode[1];
    /*! Source address */
    uint8_t srcAddr[APIMAC_SADDR_EXT_LEN];
    /*! Destination address mode */
    uint8_t dstMode[1];
    /*! Destination address */
    uint8_t dstAddr[APIMAC_SADDR_EXT_LEN];
    /*! Timestamp (backoffs) */
    uint8_t timestamp[4];
    /*! Timestamp (MAC) */
    uint8_t timestamp2[2];
    /*! Source PAN ID */
    uint8_t srcPanId[2];
    /*! Destination PAN ID */
    uint8_t dstPanId[2];
    /*!  MPDU link quality */
    uint8_t linkQuality[1];
    /*! MPDU correlation */
    uint8_t correlation[1];
    /*! MPDU rssi */
    uint8_t rssi[1];
    /*! Data sequence number */
    uint8_t dsn[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
    /*! Frame counter */
    uint8_t frameCounter[4];
    /*! FH frame type */
    uint8_t frameType[1];
    /*! MSDU data length */
    uint8_t dataLen[2];
    /*! IE data length */
    uint8_t ieLen[2];
    /*! MSDU data block followed by IE data block */
    uint8_t data[];
} MtPkt_adataInd_t;

/*! Packed serial confirm packet - Associate Cnf */
typedef struct
{
    /*! Association status */
    uint8_t status[1];
    /*! Short address */
    uint8_t shortAddr[2];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_assocCnf_t;

/*! Packed serial indication packet - Associate Ind */
typedef struct
{
    /*! Extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Operational capabilities */
    uint8_t capInfo[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_assocInd_t;

/*! Packed serial confirm packet - Async Cnf */
typedef struct
{
    /*! Async request status */
    uint8_t status[1];
} MtPkt_asyncCnf_t;

/*! Packed serial confirm packet - Beacon Ind */
typedef struct
{
    /*! Beacon type */
    uint8_t type[1];
    /*! Beacon sequence number */
    uint8_t bsn[1];
    /*! Timestamp (backoffs) */
    uint8_t timestamp[4];
    /*! Coordinator address mode */
    uint8_t addrMode[1];
    /*! Coordinator address */
    uint8_t coordAddr[APIMAC_SADDR_EXT_LEN];
    /*! Coordinator PAN ID */
    uint8_t coordPanId[2];
    /*! Network superframe spec */
    uint8_t superframeSpec[2];
    /*! Logical channel */
    uint8_t logicalChannel[1];
    /*! Channel page */
    uint8_t channelPage[1];
    /*! GTS permit indicator */
    uint8_t gtsPermit[1];
    /*! Limk quality */
    uint8_t linkQuality[1];
    /*! Security fail indicator */
    uint8_t securityFail[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
    /*! Number of pending short addresses  */
    uint8_t numShortAddr[1];
    /*! Number of pending extended addresses  */
    uint8_t numExtAddr[1];
    /*! Length of beacon payload  */
    uint8_t lenBeacon[1];
    /*! Short addr block, then ext addr block, then beacon payload */
    uint8_t data[];
} MtPkt_beaconInd_t;

/*! Packed serial confirm packet - Beacon Ind */
typedef struct
{
    /*! Status of comm event */
    uint8_t status[1];
    /*! Source address mode */
    uint8_t srcMode[1];
    /*! Source address */
    uint8_t srcAddr[APIMAC_SADDR_EXT_LEN];
    /*! Destination address mode */
    uint8_t dstMode[1];
    /*! Destination address */
    uint8_t dstAddr[APIMAC_SADDR_EXT_LEN];
    /*! Source PAN ID */
    uint8_t panId[2];
    /*! Comm status reason */
    uint8_t reason[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_commstInd_t;

/*! Packed serial confirm packet - Data Cnf */
typedef struct
{
    /*! Status of data request */
    uint8_t status[1];
    /*! MSDU handle */
    uint8_t handle[1];
    /*! Timestamp (backoffs) */
    uint8_t timestamp[4];
    /*! Timestamp (MAC) */
    uint8_t timestamp2[2];
    /*! Retries to transmit the data frame */
    uint8_t retries[1];
    /*! MPDU link quality */
    uint8_t linkQuality[1];
    /*! MPDU correlation */
    uint8_t correlation[1];
    /*! MPDU rssi */
    uint8_t rssi[1];
    /*! Frame counter */
    uint8_t frameCounter[4];
} MtPkt_dataCnf_t;

/*! Packed serial indication packet - Data Ind */
typedef struct
{
    /*! Source address mode */
    uint8_t srcMode[1];
    /*! Source address */
    uint8_t srcAddr[APIMAC_SADDR_EXT_LEN];
    /*! Destination address mode */
    uint8_t dstMode[1];
    /*! Destination address */
    uint8_t dstAddr[APIMAC_SADDR_EXT_LEN];
    /*! Timestamp (backoffs) */
    uint8_t timestamp[4];
    /*! Timestamp (MAC) */
    uint8_t timestamp2[2];
    /*! Source PAN ID */
    uint8_t srcPanId[2];
    /*! Destination PAN ID */
    uint8_t dstPanId[2];
    /*! MPDU link quality */
    uint8_t linkQuality[1];
    /*!  MPDU correlation */
    uint8_t correlation[1];
    /*! MPDU rssi */
    uint8_t rssi[1];
    /*! Data sequence number */
    uint8_t dsn[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
    /*! Frame counter */
    uint8_t frameCounter[4];
    /*! MSDU data length */
    uint8_t dataLen[2];
    /*! IE data length */
    uint8_t ieLen[2];
    /*! MSDU data block followed by IE data block */
    uint8_t data[];
} MtPkt_dataInd_t;

/*! Packed serial confirm packet - Disassociate Cnf */
typedef struct
{
    /*! Status of the dis-associate */
    uint8_t status[1];
    /*! Device address mode */
    uint8_t addrMode[1];
    /*! Device address */
    uint8_t devAddr[APIMAC_SADDR_EXT_LEN];
    /*! Device PAN ID */
    uint8_t panId[2];
} MtPkt_dassocCnf_t;

/*! Packed serial indication packet - Disassociate Ind */
typedef struct
{
    /*! Device extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Reason for dis-associating */
    uint8_t reason[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_dassocInd_t;

/*! Packed serial indication packet - eBeacon Ind */
typedef struct
{
    /*! Beacon type */
    uint8_t type[1];
    /*! Beacon sequence number */
    uint8_t bsn[1];
    /*! Beacon order */
    uint8_t beaconOrder[1];
    /*! Superframe order */
    uint8_t superframeOrder[1];
    /*! Final CAP slot */
    uint8_t finalCapSlot[1];
    /*! Enhanced beacon interval exponent */
    uint8_t eBeaconOrder[1];
    /*! Enhanced beacon time offset */
    uint8_t ofsTimeSlot[1];
    /*! TX slot position for enhabced beacon */
    uint8_t capBackOff[1];
    /*! NBPAN enhanced beacon interval exponent */
    uint8_t eBeaconOrderNbPan[2];
} MtPkt_ebeaconInd_t;

/*! Packed serial indication packet - Orphan Ind */
typedef struct
{
    /*! Device extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_orphanInd_t;

/*! Packed serial confirm packet - Poll Cnf */
typedef struct
{
    /*! Status of poll request */
    uint8_t status[1];
    /*! Frame pending indicator */
    uint8_t framePending[1];
} MtPkt_pollCnf_t;

/*! Packed serial indication packet - Poll Ind */
typedef struct
{
    /*! Device address mode */
    uint8_t addrMode[1];
    /*! Device address */
    uint8_t devAddr[APIMAC_SADDR_EXT_LEN];
    /*! Device PAN ID */
    uint8_t panId[2];
    /*! No response required indicator */
    uint8_t noResponse[1];
} MtPkt_pollInd_t;

/*! Packed serial confirm packet - Purge Cnf */
typedef struct
{
    /*! Status of purge request */
    uint8_t status[1];
    /*! MSDU handle */
    uint8_t handle[1];
} MtPkt_purgeCnf_t;

/*! Packed serial confirm packet - Scan Cnf */
typedef struct
{
    /*! Status of scan request */
    uint8_t status[1];
    /*! Scan type */
    uint8_t scanType[1];
    /*! Channel page */
    uint8_t channelPage[1];
    /*! PHY ID */
    uint8_t phyId[1];
    /*! Unscanned channel bitmask */
    uint8_t unscanned[APIMAC_154G_CHANNEL_BITMAP_SIZ];
    /*! Number of items in result list */
    uint8_t resultListSize[1];
    /*! Result list */
    uint8_t data[];
} MtPkt_scanCnf_t;

/*! Packed serial confirm packet - Start Cnf */
typedef struct
{
    /*! Status of start request */
    uint8_t status[1];
} MtPkt_startCnf_t;

/*! Packed serial indication packet - Sync Loss Ind */
typedef struct
{
    /*! Reason for sync loss */
    uint8_t reason[1];
    /*! Device PAN ID */
    uint8_t panId[2];
    /*! Logical channel */
    uint8_t logicalChannel[1];
    /*! Channel page */
    uint8_t channelPage[1];
    /*! PHY ID for the PHY descriptor */
    uint8_t phyId[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_sylossInd_t;

/******************************************************************************
 Typedefs - MT_MAC Common Packed Structures
 *****************************************************************************/
/*! Packed serial data packet - PAN Descriptor */
typedef struct
{
    /*! Address mode */
    uint8_t addrMode[1];
    /*! Device address */
    uint8_t devAddr[APIMAC_SADDR_EXT_LEN];
    /*! PAN ID */
    uint8_t panId[2];
    /*! Network superframe spec */
    uint8_t superframeSpec[2];
    /*! Logical channel */
    uint8_t logicalChannel[1];
    /*! Channel page */
    uint8_t channelPage[1];
    /*! GTS permit indicator */
    uint8_t gtsPermit[1];
    /*! Limk quality */
    uint8_t linkQuality[1];
    /*! Timestamp (backoffs) */
    uint8_t timestamp[4];
    /*! Security fail indicator */
    uint8_t securityFail[1];
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_panDesc_t;

/*! Packed serial data packet - Security Device Descriptor */
typedef struct
{
    /*! PIB attribute id */
    uint8_t attrId[1];
    /*! Index into macDeviceTable */
    uint8_t deviceIndex[MT_SEC_INDX_SIZE];
    /*! Unused can be used as the extra byte for device Index*/
    uint8_t index2[MT_SEC_INDX_SIZE];
    /*! Device PAN ID */
    uint8_t panId[2];
    /*! Device short address */
    uint8_t shortAddr[2];
    /*! Device extended address */
    uint8_t extAddr[APIMAC_SADDR_EXT_LEN];
    /*! Minimum security override indicator */
    uint8_t exempt[1];
    /*! Initial frame counters */
#ifndef SEC_API_256
    uint8_t frameCntr[6 * API_MAX_NODE_KEY_ENTRIES];
#else
    uint8_t frameCntr[4 * API_MAX_NODE_KEY_ENTRIES];
#endif
} MtPkt_secDevEntry_t;

/*! Packed serial data packet - Security Info */
typedef struct
{
    /*! Security key source */
    uint8_t keySource[APIMAC_KEY_SOURCE_MAX_LEN];
    /*! Security Level */
    uint8_t securityLevel[1];
    /*! Security key identifier mode */
    uint8_t keyIdMode[1];
    /*! Security key index */
    uint8_t keyIndex[1];
} MtPkt_secInfo_t;

/*! Packed serial data packet - Security Key Descriptor */
typedef struct
{
    /*! PIB attribute id */
    uint8_t attrId[1];
    /*! PIB table index 1 */
    uint8_t index1[MT_SEC_INDX_SIZE];
    /*! PIB table index 2 */
    uint8_t index2[MT_SEC_INDX_SIZE];
    /*! NULL data - writes to this PIB item initialize Key Descriptor Table */
    uint8_t data[];
} MtPkt_secKeyDesc_t;

/*! Packed serial data packet - Security Key Device */
typedef struct
{
    /*! PIB attribute id */
    uint8_t attrId[1];
    /*! Index into macKeyDeviceList */
    uint8_t keyIndex[MT_SEC_INDX_SIZE];
    /*! Index into macKeyDeviceList[keyIndex] */
    uint8_t keyDeviceIndex[MT_SEC_INDX_SIZE];
    /*! Handle of the DeviceDescriptor */
    uint8_t handle[MT_SEC_INDX_SIZE];
    /*! DeviceDescriptor unique device indicator */
    uint8_t unique[1];
    /*! Key exhausted the frame counter indicator */
    uint8_t blacklisted[1];
} MtPkt_secKeyDevice_t;

/*! Packed serial data packet - Security Key Entry */
typedef struct
{
    /*! PIB attribute id */
    uint8_t attrId[1];
    /*! Index into macKeyTable */
    uint8_t index1[MT_SEC_INDX_SIZE];
    /*! Unused */
    uint8_t index2[MT_SEC_INDX_SIZE];
    /*! Key entry data */
    uint8_t keyEntry[APIMAC_KEY_MAX_LEN];
    /*! Frame counter */
    uint8_t frameCounter[4];
} MtPkt_secKeyEntry_t;

/*! Packed serial data packet - Security Key ID Lookup */
typedef struct
{
    /*! PIB attribute id */
    uint8_t attrId[1];
    /*! Index into the macKeyIdLookupList */
    uint8_t index1[MT_SEC_INDX_SIZE];
    /*! Index into macKeyIdLookupList[keyIndex] */
    uint8_t index2[MT_SEC_INDX_SIZE];
    /*! Key ID lookup data */
    uint8_t lookupData[APIMAC_MAX_KEY_LOOKUP_LEN];
    /*! Key ID lookup data size indicator, 0=5 bytes, 1=9 bytes */
    uint8_t lookupSize[1];
} MtPkt_secKeyLookup_t;

/*! Packed serial data packet - Security Key Usage Entry */
typedef struct
{
    /*! PIB attribute id */
    uint8_t attrId[1];
    /*! Index into macKeyUsageList */
    uint8_t keyIndex[MT_SEC_INDX_SIZE];
    /*! Index into macKeyUsageList[keyIndex] */
    uint8_t keyUsageIndex[MT_SEC_INDX_SIZE];
    /*! Frame Type */
    uint8_t frameType[1];
    /*! Command Frame Identifier */
    uint8_t cmdFrameId[1];
} MtPkt_secKeyUsage_t;

/*! Packed serial data packet - Security Level */
typedef struct
{
    /*! PIB attribute id */
    uint8_t attrId[1];
#ifndef SEC_API_256
    /*! Index into macSecurityLevelTable */
    uint8_t levelIndex[1];
    /*! Unused */
    uint8_t index1[1];
    /*! Unused */
    uint8_t index2[2];
#else
    /*! Index into macSecurityLevelTable */
    uint8_t levelIndex[1];
    /*! Unused */
    uint8_t index2[1];
#endif
    /*! Frame Type */
    uint8_t frameType[1];
    /*! Command Frame Identifier */
    uint8_t cmdFrameId[1];
    /*! Min required/expected security level */
    uint8_t secMinimum[1];
    /*! Security level override indicator */
    uint8_t secOverride[1];
} MtPkt_secLevelEntry_t;

/******************************************************************************
 Typedefs - MT_SYS Command Packed Structures
 *****************************************************************************/
/*! Packed serial command packet - NV Compact */
typedef struct
{
    /*! Security key source */
    uint8_t threshold[2];
} MtPkt_nvCompact_t;

/*! Packed serial command packet - NV Create */
typedef struct
{
    /*! System ID */
    uint8_t sysId[1];
    /*! Item ID */
    uint8_t itemId[2];
    /*! Sub ID */
    uint8_t subId[2];
    /*! Data length */
    uint8_t length[4];
} MtPkt_nvCreate_t;

/*! Packed serial command packet - NV Delete */
typedef struct
{
    /*! System ID */
    uint8_t sysId[1];
    /*! Item ID */
    uint8_t itemId[2];
    /*! Sub ID */
    uint8_t subId[2];
} MtPkt_nvDelete_t;

/*! Packed serial command packet - NV Length */
typedef struct
{
    /*! System ID */
    uint8_t sysId[1];
    /*! Item ID */
    uint8_t itemId[2];
    /*! Sub ID */
    uint8_t subId[2];
} MtPkt_nvLength_t;

/*! Packed serial command packet - NV Read */
typedef struct
{
    /*! System ID */
    uint8_t sysId[1];
    /*! Item ID */
    uint8_t itemId[2];
    /*! Sub ID */
    uint8_t subId[2];
    /*! Data offset */
    uint8_t offset[2];
    /*! Data length */
    uint8_t length[1];
} MtPkt_nvRead_t;

/*! Packed serial command packet - NV Write */
typedef struct
{
    /*! System ID */
    uint8_t sysId[1];
    /*! Item ID */
    uint8_t itemId[2];
    /*! Sub ID */
    uint8_t subId[2];
    /*! Data offset */
    uint8_t offset[2];
    /*! Data length */
    uint8_t length[1];
    /*! Data block */
    uint8_t data[];
} MtPkt_nvWrite_t;

/*! Packed serial command packet - NV Update */
typedef struct
{
    /*! System ID */
    uint8_t sysId[1];
    /*! Item ID */
    uint8_t itemId[2];
    /*! Sub ID */
    uint8_t subId[2];
    /*! Data length */
    uint8_t length[1];
    /*! Data block */
    uint8_t data[];
} MtPkt_nvUpdate_t;

/******************************************************************************
 Typedefs - MT_UTIL Command Packed Structures
 *****************************************************************************/
/*! Packed serial command packet - Set Callback */
typedef struct
{
    /*! MT sub-system */
    uint8_t subSystem[1];
    /*! Callback bit mask */
    uint8_t callbackBits[4];
} MtPkt_callBack_t;

/*! Packed serial command packet - Extended Address */
typedef struct
{
    /*! Type of extended address */
    uint8_t addrType[1];
} MtPkt_extAddr_t;

/*! Packed serial command packet - Loopback Test */
typedef struct
{
    /*! Number of AREQs */
    uint8_t loopCount[1];
    /*! Interval between AEQs */
    uint8_t timeout[4];
} MtPkt_loopBack_t;

#ifdef __cplusplus
}
#endif

#endif /* MTPKT_H */
