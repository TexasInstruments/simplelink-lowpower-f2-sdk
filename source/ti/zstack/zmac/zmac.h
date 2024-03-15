/******************************************************************************

 @file  zmac.h

 @brief This file contains the ZStack MAC Porting Layer.

 Group: CMCU, LPRF
 Target Device: CC2652

 ******************************************************************************

 Copyright (c) 2004-2017, Texas Instruments Incorporated

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
 Release Name: simplelink_zigbee_sdk_plugin_0_95_00_18_s
 Release Date: 2017-11-10 13:43:40
 *****************************************************************************/

#ifndef ZMAC_H
#define ZMAC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "zmac_internal.h"
#include "mac.h"

/*********************************************************************
 * MACROS
 */

/* Maximum length of the beacon payload */
#ifndef ZMAC_MAX_BEACON_PAYLOAD_LEN
  #define ZMAC_MAX_BEACON_PAYLOAD_LEN    (7 + Z_EXTADDR_LEN)
#endif

/*********************************************************************
 * CONSTANTS
 */

#if defined( MAC_API_H )
 #define ZMAC_CHAN_MASK ( \
   MAC_CHAN_11_MASK | \
   MAC_CHAN_12_MASK | \
   MAC_CHAN_13_MASK | \
   MAC_CHAN_14_MASK | \
   MAC_CHAN_15_MASK | \
   MAC_CHAN_16_MASK | \
   MAC_CHAN_17_MASK | \
   MAC_CHAN_18_MASK | \
   MAC_CHAN_19_MASK | \
   MAC_CHAN_20_MASK | \
   MAC_CHAN_21_MASK | \
   MAC_CHAN_22_MASK | \
   MAC_CHAN_23_MASK | \
   MAC_CHAN_24_MASK | \
   MAC_CHAN_25_MASK | \
   MAC_CHAN_26_MASK | \
   MAC_CHAN_27_MASK | \
   MAC_CHAN_28_MASK )
#else
 #define ZMAC_CHAN_MASK  0x07FFF800
#endif

/* LQI adjustment parameters */
#if !defined( LQI_CORR_MIN )
 #define LQI_CORR_MIN  50  /* Theoretical CORR lower limt */
#endif
#if !defined( LQI_CORR_MAX )
 #define LQI_CORR_MAX  110  /* Theoretical CORR upper limt */
#endif

/*********************************************************************
 * TYPEDEFS
 */

/* ZMAC event header type */
typedef struct
{
  uint8_t   Event;              /* ZMAC event */
  uint8_t   Status;             /* ZMAC status */
} ZMacEventHdr_t;

/* Common security type */
typedef struct
{
  uint8_t KeySource[ZMAC_KEY_SOURCE_MAX_LEN];
  uint8_t SecurityLevel;
  uint8_t KeyIdMode;
  uint8_t KeyIndex;
}ZMacSec_t;

/* PAN descriptor type */
typedef struct
{
  zAddrType_t   CoordAddress;
  uint16_t        CoordPANId;
  uint16_t        SuperframeSpec;
  uint8_t         LogicalChannel;
  uint8_t         ChannelPage;
  uint8_t         GTSPermit;
  uint8_t         LinkQuality;
  uint32_t        TimeStamp;
  uint8_t         SecurityFailure;
  ZMacSec_t     Sec;
} ZMacPanDesc_t;

/* Communication status indication type */
typedef struct
{
  ZMacEventHdr_t hdr;
  zAddrType_t    SrcAddress;
  zAddrType_t    DstAddress;
  uint16_t         PANId;
  uint8_t          Reason;
  ZMacSec_t      Sec;
} ZMacCommStatusInd_t;

/* SYNC */

typedef struct
{
  uint8_t LogicalChannel;     /* The logical channel to use */
  uint8_t ChannelPage;        /* The channel page to use */
  uint8_t TrackBeacon;        /* Set to TRUE to continue tracking beacons after synchronizing with the
                               first beacon.  Set to FALSE to only synchronize with the first beacon */
}ZMacSyncReq_t;

/* DATA TYPES */

/* Data request parameters type */
typedef struct
{
  zAddrType_t   DstAddr;
  uint16_t        DstPANId;
  uint8_t         SrcAddrMode;
  uint8_t         Handle;
  uint16_t        TxOptions;
  uint8_t         Channel;
  uint8_t         Power;
  uint8_t         GpOffset;
  uint8_t         GpDuration;
  ZMacSec_t     Sec;
  uint8_t         msduLength;
  uint8_t        *msdu;
} ZMacDataReq_t;

/* Data confirm type */
typedef struct
{
  ZMacEventHdr_t hdr;
  uint8_t          msduHandle;
  ZMacDataReq_t  *pDataReq;
  uint32_t         Timestamp;
  uint16_t         Timestamp2;
  uint8_t          retries;
  uint8_t          mpduLinkQuality;
  uint8_t          correlation;
  int8_t           rssi;
} ZMacDataCnf_t;


/* ASSOCIATION TYPES */

/* Associate request type */
typedef struct
{
  ZMacSec_t     Sec;
  uint8_t         LogicalChannel;
  uint8_t         ChannelPage;
  zAddrType_t   CoordAddress;
  uint16_t        CoordPANId;
  uint8_t         CapabilityFlags;
} ZMacAssociateReq_t;

/* Associate response type */
typedef struct
{
  ZMacSec_t      Sec;
  ZLongAddr_t    DeviceAddress;
  uint16_t         AssocShortAddress;
  uint8_t          Status;
} ZMacAssociateRsp_t;

/* Associate indication parameters type */
typedef struct
{
  ZMacEventHdr_t hdr;
  ZLongAddr_t    DeviceAddress;
  uint8_t          CapabilityFlags;
  ZMacSec_t      Sec;
} ZMacAssociateInd_t;

/* Associate confim type */
typedef struct
{
  ZMacEventHdr_t hdr;
  uint16_t         AssocShortAddress;
  ZMacSec_t      Sec;
} ZMacAssociateCnf_t;

/* Disassociate request type */
typedef struct
{
  ZMacSec_t     Sec;
  zAddrType_t   DeviceAddress;
  uint16_t        DevicePanId;
  uint8_t         DisassociateReason;
  uint8_t         TxIndirect;
} ZMacDisassociateReq_t;

/* Rx enable confirm type */
typedef struct
{
  ZMacEventHdr_t hdr;
} ZMacRxEnableCnf_t;

/* SCAN */
/* Scan request type */
typedef struct
{
  uint32_t         ScanChannels;
  uint8_t          ScanType;
  uint8_t          ScanDuration;
  uint8_t          ChannelPage;
  uint8_t          MaxResults;
  /* Adding fields for enhanced active scan request */
  bool           PermitJoining;
  uint8_t          LinkQuality;
  uint8_t          PercentFilter;
  ZMacSec_t      Sec;
  union
  {
    uint8_t        *pEnergyDetect;
    ZMacPanDesc_t *pPanDescriptor;
  }Result;
} ZMacScanReq_t;

/* Scan confirm type */
typedef struct
{
  ZMacEventHdr_t hdr;
  uint8_t          ScanType;
  uint8_t          ChannelPage;
  uint32_t         UnscannedChannels;
  uint8_t          ResultListSize;
  union
  {
    uint8_t         *pEnergyDetect;
    ZMacPanDesc_t *pPanDescriptor;
  }Result;
} ZMacScanCnf_t;


/* START */
/* Start request type */
typedef struct
{
  uint32_t        StartTime;
  uint16_t        PANID;
  uint8_t         LogicalChannel;
  uint8_t         ChannelPage;
  uint8_t         BeaconOrder;
  uint8_t         SuperframeOrder;
  uint8_t         PANCoordinator;
  uint8_t         BatteryLifeExt;
  uint8_t         CoordRealignment;
  ZMacSec_t     RealignSec;
  ZMacSec_t     BeaconSec;
} ZMacStartReq_t;

/* Start confirm type */
typedef struct
{
  ZMacEventHdr_t hdr;
} ZMacStartCnf_t;

/* POLL */
/* Roll request type */
typedef struct
{
  zAddrType_t CoordAddress;
  uint16_t      CoordPanId;
  ZMacSec_t   Sec;
} ZMacPollReq_t;

/* Poll confirm type */
typedef struct
{
  ZMacEventHdr_t hdr;
} ZMacPollCnf_t;

/* MAC_MLME_POLL_IND type */
typedef struct
{
  ZMacEventHdr_t  hdr;
  sAddr_t         srcAddr;        /* Short address of the device sending the data request */
  uint16_t          srcPanId;       /* Pan ID of the device sending the data request */
  uint8_t           noRsp;          /* indication that no MAP_MAC_McpsDataReq() is required.
                                   * It is set when MAC_MLME_POLL_IND is generated,
                                   * to simply indicate that a received data request frame
                                   * was acked with pending bit cleared. */
} ZMacPollInd_t;

/* ORPHAN */
/* Orphan response type */
typedef struct
{
  ZMacSec_t      Sec;
  ZLongAddr_t    OrphanAddress;
  uint16_t         ShortAddress;
  uint8_t          AssociatedMember;
} ZMacOrphanRsp_t;

/* Orphan indication type */
typedef struct
{
  ZMacEventHdr_t hdr;
  ZLongAddr_t    OrphanAddress;
  ZMacSec_t      Sec;
} ZMacOrphanInd_t;

#if defined (MT_MAC_FUNC) || defined (MT_MAC_CB_FUNC)

/* Sync loss indication type */
typedef struct
{
  ZMacEventHdr_t hdr;
  uint16_t         PANId;
  uint8_t          LogicalChannel;
  uint8_t          ChannelPage;
  ZMacSec_t      Sec;
} ZMacSyncLossInd_t;

/* Data indication parameters type */
typedef struct
{
  ZMacEventHdr_t hdr;
  ZMacSec_t      Sec;
  zAddrType_t    SrcAddr;
  zAddrType_t    DstAddr;
  uint32_t         Timestamp;
  uint16_t         Timestamp2;
  uint16_t         SrcPANId;
  uint16_t         DstPANId;
  uint8_t          mpduLinkQuality;
  uint8_t          Correlation;
  uint8_t          Rssi;
  uint8_t          Dsn;
  uint8_t          msduLength;
  uint8_t         *msdu;
} ZMacDataInd_t;

/* Disassociate indication type */
typedef struct
{
  ZMacEventHdr_t hdr;
  ZLongAddr_t    DeviceAddress;
  uint8_t          DisassociateReason;
  ZMacSec_t      Sec;
} ZMacDisassociateInd_t;

/* Disassociate confirm type */
typedef struct
{
  ZMacEventHdr_t hdr;
  zAddrType_t    DeviceAddress;
  uint16_t         panID;
} ZMacDisassociateCnf_t;

/* Beacon notify indication type */
typedef struct
{
  ZMacEventHdr_t hdr;
  uint8_t          BSN;
  ZMacPanDesc_t *pPanDesc;
  uint8_t          PendAddrSpec;
  uint8_t         *AddrList;
  uint8_t          sduLength;
  uint8_t         *sdu;
} ZMacBeaconNotifyInd_t;

/* Purge confirm type */
typedef struct
{
  ZMacEventHdr_t hdr;
  uint8_t          msduHandle;
} ZMacPurgeCnf_t;
#endif

typedef enum
{
  TX_PWR_MINUS_22 = -22,
  TX_PWR_MINUS_21,
  TX_PWR_MINUS_20,
  TX_PWR_MINUS_19,
  TX_PWR_MINUS_18,
  TX_PWR_MINUS_17,
  TX_PWR_MINUS_16,
  TX_PWR_MINUS_15,
  TX_PWR_MINUS_14,
  TX_PWR_MINUS_13,
  TX_PWR_MINUS_12,
  TX_PWR_MINUS_11,
  TX_PWR_MINUS_10,
  TX_PWR_MINUS_9,
  TX_PWR_MINUS_8,
  TX_PWR_MINUS_7,
  TX_PWR_MINUS_6,
  TX_PWR_MINUS_5,
  TX_PWR_MINUS_4,
  TX_PWR_MINUS_3,
  TX_PWR_MINUS_2,
  TX_PWR_MINUS_1,
  TX_PWR_ZERO,
  TX_PWR_PLUS_1,
  TX_PWR_PLUS_2,
  TX_PWR_PLUS_3,
  TX_PWR_PLUS_4,
  TX_PWR_PLUS_5,
  TX_PWR_PLUS_6,
  TX_PWR_PLUS_7,
  TX_PWR_PLUS_8,
  TX_PWR_PLUS_9,
  TX_PWR_PLUS_10,
  TX_PWR_PLUS_11,
  TX_PWR_PLUS_12,
  TX_PWR_PLUS_13,
  TX_PWR_PLUS_14,
  TX_PWR_PLUS_15,
  TX_PWR_PLUS_16,
  TX_PWR_PLUS_17,
  TX_PWR_PLUS_18,
  TX_PWR_PLUS_19
} ZMacTransmitPower_t;  // The transmit power in units of -1 dBm.

typedef struct
{
  byte protocolID;
  byte stackProfile;    // 4 bit in native
  byte protocolVersion; // 4 bit in native
  byte reserved;        // 2 bit in native
  byte routerCapacity;  // 1 bit in native
  byte deviceDepth;     // 4 bit in native
  byte deviceCapacity;  // 1 bit in native
  byte extendedPANID[Z_EXTADDR_LEN];
  byte txOffset[3];
  byte updateId;
} beaconPayload_t;

typedef uint8_t (*applySecCB_t)( uint8_t len, uint8_t *msdu );

typedef enum
{
  LQI_ADJ_OFF = 0,
  LQI_ADJ_MODE1,
  LQI_ADJ_MODE2,
  LQI_ADJ_GET = 0xFF
} ZMacLqiAdjust_t;  // Mode settings for lqi adjustment

/*********************************************************************
 * GLOBAL VARIABLES
 */
#define NWK_CMD_ID_LEN sizeof( byte )

/*********************************************************************
 * FUNCTIONS
 */

  /*
   * Initialize.
   */
  extern uint32_t ZMacEventLoop(uint8_t taskId, uint32_t events);

  /*
   * Initialize.
   */
  extern ZMacStatus_t ZMacInit( void );

  /*
   * Send a MAC Data Frame packet.
   */
  extern ZMacStatus_t ZMacDataReq( ZMacDataReq_t *param );

  /*
   * Send a MAC Data Frame packet and apply application security to the packet.
   */
  extern uint8_t ZMacDataReqSec( ZMacDataReq_t *pData, applySecCB_t secCB );

  /*
   * Request an association with a coordinator.
   */
  extern ZMacStatus_t ZMacAssociateReq( ZMacAssociateReq_t *param );

  /*
   * Request to send an association response message.
   */
  extern ZMacStatus_t ZMacAssociateRsp( ZMacAssociateRsp_t *param );

  /*
   * Request to send a disassociate request message.
   */
  extern ZMacStatus_t ZMacDisassociateReq( ZMacDisassociateReq_t *param );

  /*
   * Gives the MAC extra processing time.
   * Returns false if its OK to sleep.
   */
  extern byte ZMacUpdate( void );

  /*
   * Read a MAC PIB attribute.
   */
  extern ZMacStatus_t ZMacGetReq( ZMacAttributes_t attr, byte *value );

  /*
   * This function allows the next higher layer to respond to
   * an orphan indication message.
   */
  extern ZMacStatus_t ZMacOrphanRsp( ZMacOrphanRsp_t *param );

  /*
   * This function is called to request MAC data request poll.
   */
  extern ZMacStatus_t ZMacPollReq( ZMacPollReq_t *param );

  /*
   * Reset the MAC.
   */
  extern ZMacStatus_t ZMacReset( byte SetDefaultPIB );

  /*
   * This function is called to perform a network scan.
   */
  extern ZMacStatus_t ZMacScanReq( ZMacScanReq_t *param );

  /*
   * Write a MAC PIB attribute.
   */
  extern ZMacStatus_t ZMacSetReq( ZMacAttributes_t attr, byte *value );

#ifdef FEATURE_MAC_SECURITY
  /*
   * Read a MAC Security PIB attribute.
   */
  extern ZMacStatus_t ZMacSecurityGetReq( ZMacAttributes_t attr, byte *value );

  /*
   * Write a MAC Security PIB attribute.
   */
  extern ZMacStatus_t ZMacSecuritySetReq( ZMacAttributes_t attr, byte *value );
#endif /* FEATURE_MAC_SECURITY */

  /*
   * This function is called to tell the MAC to transmit beacons
   * and become a coordinator.
   */
  extern ZMacStatus_t ZMacStartReq( ZMacStartReq_t *param );

  /*
   * This function is called to request a sync to the current
   * networks beacons.
   */
  extern ZMacStatus_t ZMacSyncReq( ZMacSyncReq_t *param );

  /*
   * This function requests to reset mac state machine and
   * transaction.
   */
  extern ZMacStatus_t ZMacCleanReq( void );

  /*
   * This function is called to request MAC to purge a message.
   */
  extern ZMacStatus_t ZMacPurgeReq( byte msduHandle );

  /*
   * This function is called to enable AUTOPEND and source address matching.
   */
  extern ZMacStatus_t ZMacSrcMatchEnable ( void );

 /*
  * This function is called to add a short or extended address to source address table.
  */
  extern ZMacStatus_t ZMacSrcMatchAddEntry (zAddrType_t *addr, uint16_t panID);

  /*
   * This function is called to delete a short or extended address from source address table.
   */
  extern ZMacStatus_t ZMacSrcMatchDeleteEntry (zAddrType_t *addr, uint16_t panID);

  /*
   * This funciton is called to enabled/disable acknowledging all packets with pending bit set
   */
  extern ZMacStatus_t ZMacSrcMatchAckAllPending (uint8_t option);

  /*
   * This function is called to check if acknowledging all packets with pending bit set is enabled.
   */
  extern ZMacStatus_t ZMacSrcMatchCheckAllPending (void);

  /*
   * This function is called to request MAC to power on the radio hardware and wake up.
   */
  extern void ZMacPwrOnReq ( void );

  /*
   * This function returns the current power mode of the MAC.
   */
  extern uint8_t ZMac_PwrMode(void);

  /*
   * This function is called to request MAC to set the transmit power level.
   */
  extern ZMacStatus_t ZMacSetTransmitPower( ZMacTransmitPower_t level );

  /*
   * This function is called to send out an empty msg
   */
  extern void ZMacSendNoData( uint16_t DstAddr, uint16_t DstPANId );

  /*
   * This callback function is called for every MAC message that is received
   * over-the-air or generated locally by MAC for the application.
   */
  extern uint8_t (*pZMac_AppCallback)( uint8_t *msgPtr );

  /*
   * This function returns true if the MAC state is idle.
   */
  extern uint8_t ZMacStateIdle( void );

  /*
   * This function sets/returns LQI adjust mode.
   */
  extern ZMacLqiAdjust_t ZMacLqiAdjustMode( ZMacLqiAdjust_t mode );

  /*
   * This function sends out an enhanced active scan request
   */
  extern ZMacStatus_t ZMacEnhancedActiveScanReq( ZMacScanReq_t *param );

  /*
   * Sets MAC PIB values specific for Z-Stack (after calling ZMacReset)
   */
  extern void ZMacSetZigbeeMACParams(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZMAC_H */
