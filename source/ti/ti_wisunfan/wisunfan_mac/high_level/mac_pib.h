/******************************************************************************

 @file  mac_pib.h

 @brief Internal interface file for the MAC PIB module.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2025, Texas Instruments Incorporated

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

#ifndef MAC_PIB_H
#define MAC_PIB_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "timac_api.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */
#if !defined(MAC_PIB_MAX_BEACON_PAYLOAD_LEN)
#define MAC_PIB_MAX_BEACON_PAYLOAD_LEN  52
#endif

/* Attribute index constants, based on attribute ID values from spec */
#define MAC_ATTR_SET1_START       0x40
#define MAC_ATTR_SET1_END         0x64
#define MAC_ATTR_SET1_OFFSET      0
#define MAC_ATTR_SET2_START       0xE0
#define MAC_ATTR_SET2_END         0xF8
#define MAC_ATTR_SET2_OFFSET      (MAC_ATTR_SET1_END - MAC_ATTR_SET1_START + MAC_ATTR_SET1_OFFSET + 1)

/* frame response values */
#define MAC_MAX_FRAME_RESPONSE_MIN  143
#define MAC_MAX_FRAME_RESPONSE_MAX  32001

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)

/* MR-FSK channel center frequencies */
#define MAC_915MHZ_BAND_MODE_1_CENTER_FREQ_KHZ          902200
#define MAC_915MHZ_BAND_MODE_2_CENTER_FREQ_KHZ          902400
#define MAC_868MHZ_BAND_MODE_1_CENTER_FREQ_KHZ          863125
#define MAC_433MHZ_BAND_MODE_1_CENTER_FREQ_KHZ          433300
#define MAC_50KBPS_866MHZ_BAND_MODE_1_CENTER_FREQ_KHZ   865100
#define MAC_100KBPS_866MHZ_BAND_MODE_2_CENTER_FREQ_KHZ  865100
#define MAC_5KBPS_915MHZ_BAND_MODE_1_CENTER_FREQ_KHZ    902200
#define MAC_5KBPS_433MHZ_BAND_MODE_1_CENTER_FREQ_KHZ    433300
#define MAC_5KBPS_868MHZ_BAND_MODE_1_CENTER_FREQ_KHZ    863125
#define MAC_915MHZ_BAND_MODE_3_CENTER_FREQ_KHZ          902400
#define MAC_868MHZ_BAND_MODE_2_CENTER_FREQ_KHZ          863225

/* MR-FSK channel spacing */
#define MAC_915MHZ_BAND_MODE_1_CHAN_SPACING_KHZ         200
#define MAC_915MHZ_BAND_MODE_2_CHAN_SPACING_KHZ         400
#define MAC_868MHZ_BAND_MODE_1_CHAN_SPACING_KHZ         200
#define MAC_433MHZ_BAND_MODE_1_CHAN_SPACING_KHZ         200
#define MAC_50KBPS_866MHZ_BAND_MODE_1_CHAN_SPACING_KHZ  100
#define MAC_100KBPS_866MHZ_BAND_MODE_2_CHAN_SPACING_KHZ 200
#define MAC_5KBPS_915MHZ_BAND_MODE_1_CHAN_SPACING_KHZ   200
#define MAC_5KBPS_433MHZ_BAND_MODE_1_CHAN_SPACING_KHZ   200
#define MAC_5KBPS_868MHZ_BAND_MODE_1_CHAN_SPACING_KHZ   200
#define MAC_915MHZ_BAND_MODE_3_CHAN_SPACING_KHZ         400
#define MAC_868MHZ_BAND_MODE_2_CHAN_SPACING_KHZ         400

/* MR-FSK Number of Channels */
#define MAC_915MHZ_BAND_MODE_1_TOTAL_CHANNELS           129
#define MAC_915MHZ_BAND_MODE_2_TOTAL_CHANNELS           64
#define MAC_868MHZ_BAND_MODE_1_TOTAL_CHANNELS           34
#define MAC_433MHZ_BAND_MODE_1_TOTAL_CHANNELS           7
#define MAC_50KBPS_866MHZ_BAND_MODE_1_TOTAL_CHANNELS    19
#define MAC_100KBPS_866MHZ_BAND_MODE_2_TOTAL_CHANNELS   10
#define MAC_5KBPS_915MHZ_BAND_MODE_1_TOTAL_CHANNELS     129
#define MAC_5KBPS_433MHZ_BAND_MODE_1_TOTAL_CHANNELS     7
#define MAC_5KBPS_868MHZ_BAND_MODE_1_TOTAL_CHANNELS     34
#define MAC_915MHZ_BAND_MODE_3_TOTAL_CHANNELS           64
#define MAC_868MHZ_BAND_MODE_2_TOTAL_CHANNELS           17

#endif

#if defined(COMBO_MAC) || defined(FREQ_2_4G)
/* MR-FSK channel center frequencies */
#define MAC_IEEE_CENTER_FREQ_KHZ              2405000
/* MR-FSK channel spacing */
#define MAC_IEEE_CHAN_SPACING_KHZ             5000
/* MR-FSK Number of Channels */
#define MAC_IEEE_TOTAL_CHANNELS               16

#endif

/* Invalid PIB table index used for error code */
//#define MAC_PIB_INVALID     ((uint8) (sizeof(macPibTbl) / sizeof(macPibTbl[0])))
#define MAC_PIB_INVALID     (0xFF)
/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* PIB access and min/max table type */
typedef struct
{
  uint8     offset;
  uint8     len;
  uint8     min;
  uint8     max;
} macPibTbl_t;

/* MAC PIB type */
typedef struct
{
  uint8             ackWaitDuration;
  bool              associationPermit;
  bool              autoRequest;
  bool              battLifeExt;
  uint8             battLifeExtPeriods;

  uint8             *pBeaconPayload;
  uint8             beaconPayloadLength;
  uint8             beaconOrder;
  uint32            beaconTxTime;
  uint8             bsn;

  sAddr_t           coordExtendedAddress;
  uint16            coordShortAddress;
  uint8             dsn;
  bool              gtsPermit;
  uint8             maxCsmaBackoffs;

  uint8             minBe;
  uint16            panId;
  bool              promiscuousMode;
  bool              rxOnWhenIdle;
  uint16            shortAddress;

  uint8             superframeOrder;
  uint16            transactionPersistenceTime;
  bool              associatedPanCoord;
  uint8             maxBe;
  uint16            maxFrameTotalWaitTime;

  uint8             maxFrameRetries;
  uint8             responseWaitTime;
  uint8             syncSymbolOffset;
  bool              timeStampSupported;
  bool              securityEnabled;

  uint8             ebsn;
  uint8             eBeaconOrder;
  uint16            eBeaconOrderNBPAN;
  uint8             offsetTimeSlot;
  bool              includeMPMIE;

  uint8             fskPreambleLen;
  uint8             mrFSKSFD;

  /* Proprietary */
  int8              phyTransmitPower;
  uint8             logicalChannel;
  sAddr_t           extendedAddress;
  uint8             altBe;
  uint8             deviceBeaconOrder;
  uint8             rf4cepowerSavings;
  uint8             frameVersionSupport;
  uint8             channelPage;
  uint8             curPhyID;
  bool              fcsType;

  /* Diagnostics */
  uint32            diagsRxCrcPass;
  uint32            diagsRxCrcFail;
  uint32            diagsRxBcast;
  uint32            diagsTxBcast;
  uint32            diagsRxUcast;
  uint32            diagsTxUcast;
  uint32            diagsTxUcastRetry;
  uint32            diagsTxUcastFail;
  uint32            diagsRxSecFail;
  uint32            diagsTxSecFail;

  /* LBT RSSI threshold */
  int8              rssiThreshold;

  /* Range Extender */
  uint8             rangeExt;
  /* enable Ack Pending for Data Pkts */
  uint8             enDataAckPending;

  /* RF freq */
  uint8             rfFreq;

  /* PA type */
  uint8             paType;
} macPib_t;


/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern uint8_t regDomain;


/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macPibReset(void);

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macMRFSKGetPhyDesc
 *
 * @brief       This function retrieves PHY descriptor based on PHY ID
 *
 * input parameters
 *
 * @param       phyID - phy attribute identifier.
 *
 * @return      Pointer to macMRFSKPHYDescriptor for the particular PHY ID
 *
 **************************************************************************************************/
MAC_INTERNAL_API macMRFSKPHYDesc_t* macMRFSKGetPhyDesc(uint8 phyID);
#endif
#if defined(COMBO_MAC) || defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macIEEEGetPhyDesc
 *
 * @brief       This function selects the IEEE PHY descriptor
 *
 * input parameters
 *
 * @param       phyID - index of the PHY descriptor
 *
 * @return      pValue - pointer to the PHY descriptor
 *
 **************************************************************************************************
 */
MAC_INTERNAL_API macIEEEPHYDesc_t* macIEEEGetPhyDesc(uint8 phyID);
#endif
/**************************************************************************************************
*/

#endif /* MAC_PIB_H */

