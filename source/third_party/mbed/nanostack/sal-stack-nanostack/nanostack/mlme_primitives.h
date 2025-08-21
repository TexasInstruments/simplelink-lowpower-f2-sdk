/*
 * Copyright (c) 2013-2018, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** \file mlme_primitives.h
 * \brief MLME primitives
 */

#ifndef MLME_PRIMITIVES_H_
#define MLME_PRIMITIVES_H_

/**
 * @brief MLME primitive error statuses
 *
 * See IEEE standard 802.15.4-2006 for more details
 */
#define MLME_SUCCESS                    0x00 /**< The requested operation was completed successfully*/
#define MLME_BUSY_CHAN                  0xe1 /**< CSMA-CA fail*/
#define MLME_BUSY_RX                    0x01 /**< The radio is asked to change its state while receiving */
#define MLME_BUSY_TX                    0x02 /**< The radio is asked to change its state while transmitting. */
#define MLME_FORCE_TRX_OFF              0x03 /**< The radio is to be switched off immediately */
#define MLME_IDLE                       0x04 /**< The CCA attempt has detected an idle channel */
#define MLME_RX_ON                      0x06 /**< The radio is in or is to be configured into the receiver enabled state. */
#define MLME_TRX_OFF                    0x08 /**< The radio is in or is to be configured into the receiver enabled state. */
#define MLME_TX_ON                      0x09 /**< The radio is in or is to be configured into the receiver enabled state. */
#define MLME_RCP_QUEUE_TIMEOUT          0xd9 /**< The data request timed out in the RCP queue. */
#define MLME_MAC_ALLOCATION_ERROR       0xda /**< MAC failed to allocate memory for the operation. */
#define MLME_COUNTER_ERROR              0xdb /**< Originated messages security counter is not valid */
#define MLME_IMPROPER_KEY_TYPE          0xdc /**< Received Messages key used is agains't key usage policy */
#define MLME_IMPROPER_SECURITY_LEVEL    0xdd /**< Received Messages security level does not meet minimum security level */
#define MLME_UNSUPPORTED_LEGACY         0xde /**< The received frame was purportedly secured using security based on IEEE Std 802.15.4-2003, and such security is not supported by this standard. */
#define MLME_UNSUPPORTED_SECURITY       0xdf /**< The received frame security is not supported */
#define MLME_PACKET_BUILD_ERROR         0xe0 /**< TX packet build error */

#define MLME_SECURITY_FAIL              0xe4 /**< Cryptographic processing of the received secured frame failed. */
#define MLME_FRAME_TOO_LONG             0xe5 /**< Either a frame resulting from processing has a length that is greater than aMaxPHYPacketSize */
#define MLME_INVALID_HANDLE             0xe7 /**< Status for Purge request when Mac not detect proper queued message*/
#define MLME_INVALID_PARAMETER          0xe8 /**< A parameter in the primitive is either not supported or is out of the valid range */
#define MLME_TX_NO_ACK                  0xe9 /**< No ack was received after macMaxFrameRetries */
#define MLME_NO_BEACON                  0xea /**< A scan operation failed to find any network beacons */
#define MLME_NO_DATA                    0xeb /**< No response data were available following a request */
#define MLME_NO_SHORT_ADDRESS           0xec /**< Operation fail because 16-bit address is not allocated */
#define MLME_PAN_ID_CONFLICT            0xee /**< A PAN identifier conflict has been detected and communicated to the PAN coordinator. */
#define MLME_TRANSACTION_EXPIRED        0xf0 /**< The transaction has expired and its information was discarded */
#define MLME_TRANSACTION_OVERFLOW       0xf1 /**< MAC have no capacity to store the transaction */
#define MLME_UNAVAILABLE_KEY            0xf3 /**< Received message use unknown key, or the originating device is unknown or is blacklisted with that particular key */
#define MLME_UNSUPPORTED_ATTRIBUTE      0xf4 /**< A SET/GET request was issued with the unsupported identifier */
#define MLME_INVALID_ADDRESS            0xf5 /**< A request to send data was unsuccessful because neither the source address parameters nor the destination address parameters were present.*/
#define MLME_INVALID_INDEX              0xf9 /**< An attempt to write to a MAC PIB attribute that is in a table failed because the specified table index was out of range. */
#define MLME_LIMIT_REACHED              0xfa /**< A scan operation terminated prematurely because the number of PAN descriptors stored reached an implementation- specified maximum */
#define MLME_READ_ONLY                  0xfb /**< A SET request was issued with the identifier of an attribute that is read only.*/
#define MLME_SCAN_IN_PROGRESS           0xfc /**< Request scan request fail when scan is already active */
#define MLME_NO_NEIGHBOR_DATA           0xfd /**< Neighbor table entry not found */
//NOT-standard
#define MLME_DATA_POLL_NOTIFICATION     0xff /**< Thread requirement feature COMM status status for indicate for successfully data poll event to refresh neighbour data */

/**
 * @brief enum mlme_attr_t MLME attributes used with GET and SET primitives
 *
 * See IEEE standard 802.15.4-2006 (table 86) for more details
 */
typedef enum {
    phyCurrentChannel = 0x00,       /*<Current RF channel*/
    macAckWaitDuration = 0x40,      /*<Integer, n. of symbols*/
    macAssociationPermit = 0x41,    /*<Boolean, if association is allowed (in coordinator)*/
    macAutoRequest = 0x42,          /*<Boolean, if device automatically sends data request on beacon*/
    macBattLifeExt = 0x43,          /*<Boolean, if BLE is enabled*/
    macBattLifeExtPeriods = 0x44,   /*<Integer 6-41, BLE back off periods */
    macBeaconPayload = 0x45,        /*<Set of bytes, beacon payload*/
    macBeaconPayloadLength = 0x46,  /*<Integer 0-MaxPayLoadLen*/
    macBeaconOrder = 0x47,          /*<Integer 0–15, Beacon tx period, 15 = no periodic beacon*/
    macBeaconTxTime = 0x48,         /*<Integer 0x000000–0xffffff, symbols, when last beacon was transmitted*/
    macBSN = 0x49,                  /*<Integer 0x00-0xff, Beacon sequence number*/
    macCoordExtendedAddress = 0x4a, /*<64-bit IEEE of coordinator*/
    macCoordShortAddress = 0x4b,    /*<16-bit addr of coordinator*/
    macDSN = 0x4c,                  /*<Integer 0x00-0xff, Data frame sequence number*/
    macGTSPermit = 0x4d,            /*<Boolean, GTS allowed?*/
    macMaxCSMABackoffs = 0x4e,      /*<Integer 0-5*/
    macMinBE = 0x4f,                /*<Integer 0-macMaxBE*/
    macPANId = 0x50,                /*<PAN ID, 16 bits*/
    macPromiscuousMode = 0x51,      /*<Boolean*/
    macRxOnWhenIdle = 0x52,         /*<Boolean*/
    macShortAddress = 0x53,         /*<Short address, 16 bits*/
    macSuperframeOrder = 0x54,      /*<Integer 0-15, The length of the active portion of the outgoing super frame, 15 = none*/
    macTransactionPersistenceTime = 0x55, /*<Integer 0x0000-0xffff (unit periods)*/
    macAssociatedPANCoord = 0x56,   /*<Boolean, associated to PAN coordinator*/
    macMaxBE = 0x57,                /*<Integer 3-8, max value of back off exponent*/
    macMaxFrameTotalWaitTime = 0x58,/*<Integer, max of CAP symbols while waiting for data requested by DREQ or PEND*/
    macMaxFrameRetries = 0x59,      /*<Integer 0-7*/
    macResponseWaitTime = 0x5a,     /*<Integer 2-64  The maximum time in SuperFrameDurations to wait for responses*/
    macSyncSymbolOffset = 0x5b,     /*<Integer 0x000-0x100 (symbols) time stamp offset*/
    macTimestampSupported = 0x5c,   /*<Boolean*/
    macSecurityEnabled = 0x5d,      /*<Boolean*/
    macKeyTable = 0x71,             /*<A table of KeyDescriptor entries, each containing keys and related information required for secured communications.*/
    macDeviceTable = 0x72,          /*<List of Descriptor entries, each indicating a remote device*/
    macSecurityLevelTable = 0x73,   /*<A table of SecurityLevelDescriptor entries*/
    macKeyTableEntries = 0x81,      /*<The number of entries in macKeyTable.*/
    macDeviceTableEntries = 0x82,   /*<The number of entries in macDeviceTable.*/
    macSecurityLevelTableEntries = 0x83,    /*<The number of entries in macSecurityLevelTable*/
    macFrameCounter = 0x84,         /*<The outgoing frame counter*/
    macAutoRequestSecurityLevel = 0x85, /*<0x00-0x07  The security level used for automatic data requests.*/
    macAutoRequestKeyIdMode = 0x86,      /*< The key identifier mode used for automatic data requests.*/
    macAutoRequestKeySource = 0x87, /*<Key source for automatic data*/
    macAutoRequestKeyIndex = 0x88,  /*<The index of the key used for automatic data*/
    macDefaultKeySource = 0x89,      /*<Default key source*/

    macPanCoordExtendedAddress = 0x8a,
    macPanCoordShortAddress = 0x8b,
    macDeviceTableFrameCount = 0x8c,  /* Set only the frame count entry at the provided index of the device table */

    /* Proprietary Security PIB */
    macKeyIdLookupEntry = 0xd0,
    macKeyDeviceEntry = 0xd1,
    macKeyUsageEntry = 0xd2,
    macKeyEntry = 0xd3,
    macDeviceEntry = 0xd4,
    macSecurityLevelEntry = 0xd5,

    /* Proprietary PIB */
    macPhyTransmitPowerSigned = 0xe0,
    macLogicalChannel = 0xe1,
    macExtendedAddress = 0xe2,
    macAltbe = 0xe3,
    macdeviceBeaconOrder = 0xe4,
    macRf4cePowerSavings = 0xe5,
    macFrameVersionSupport = 0xe6,
    macChannelPage = 0xe7,
    macPhyCurrentDescriptorId = 0xe8,
    macFcsType = 0xe9,
    macRegDomain = 0x63,

    /* Diagnostic PIB */
#if defined(MAC_DUTY_CYCLE_CHECKING) || defined(MAC_OVERRIDE_TX_DELAY)
    macDutyCycleEnable = 0xea,
    macDutyCycleRegulated = 0xeb,
    macCustomMinTxOffEnabled = 0xec,
    macMinTxOffTime = 0xed,
#else
    macDiagsRxCrcPass = 0xea,
    macDiagsRxCrcFail = 0xeb,
    macDiagsRxBcast = 0xec,
    macDiagsTxBcast = 0xed,
    macDiagsRxUcast = 0xee,
    macDiagsTxUcast = 0xef,
    macDiagsTxUcastRetry = 0xf0,
#endif

#if 0
    macDiagsTxUcastFail = 0xf1, //#if 0ed this as NS code macRequestRestart got introduced in NS code
    macDiagsRxSecFail = 0xf2,
    macDiagsTxSecFail = 0xf3,

    /* LBT RSSI Threshold */
    macRssiThreshold = 0xf4,
    macEnDataAckPending = 0xf6,
    macRfFreq = 0xf7,
#else
    //NON standard extension
    macRequestRestart = 0xf1,       /*< Configure failed packet data request restart */
    macEdfeForceStop = 0xf2,          /*< Use this command for Data wait timeout at LLC: Mac stop Edfe session data wait and enable normal FHSS mode */
    macSetDataWhitening = 0xf3,     /*< Enable or disable data whitening, boolean true for enable, false for disable */
    macCCAThresholdStart = 0xf4,    /*< Start automatic CCA threshold */
    macCCAThreshold, //macRssiThreshold,         /*<CCA threshold*/
    mac802_15_4Mode = 0xf7,         /*<IEEE 802.15.4 mode*/
#endif

    //NON standard extension
    macDeviceDescriptionPanIDUpdate = 0xf8, /*<Thread pending link update case this will update device descrioton list pan-id to new one*/
    macTXPower = macPhyTransmitPowerSigned,              /*<TX output power*/
    macMultiCSMAParameters = 0xfa,  /*<Multi CSMA parameters*/
    macRfConfiguration = 0xfb,      /*<RF channel configuration parameters*/
    macAcceptByPassUnknowDevice = 0xfc,  /*< Accept data trough MAC if packet is data can be authenticated by group key nad MIC. Security enforsment point must be handled carefully these packets */
    macLoadBalancingBeaconTx = 0xfd,  /*< Trig Beacon from load balance module periodic */
    macLoadBalancingAcceptAnyBeacon = 0xfe, /*<Beacon accept state control from other network. Value size bool, data true=Enable, false=disable .*/
    macThreadForceLongAddressForBeacon = 0xff /*<Thread standard force beacon source address for extended 64-bit*/
} mlme_attr_t;


#endif // MLME_PRIMITIVES_H_