/******************************************************************************

 @file  mac_settings.h

 @brief Describe the purpose and contents of the file.

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
#ifndef MAC_SETTINGS_H
#define MAC_SETTINGS_H

#if defined(COMBO_MAC) || defined(FREQ_2_4G)
#include <driverlib/rf_ieee_mailbox.h>
#include <driverlib/rf_ieee_cmd.h>
#endif
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
#include <driverlib/rf_prop_cmd.h>
#endif
#include <driverlib/rf_mailbox.h>
#include <driverlib/rf_common_cmd.h>
#include <ti/drivers/rf/RF.h>
/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
//To be removed when solution to RFCORE49 is available
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
#define RFCORE49
#endif

/**
 * Channel page.
 */
#define CONFIG_CHANNEL_PAGE APIMAC_CHANNEL_PAGE_9

/**
 * (IEEE 802.15.4-2006 7.4.1) MAC constants.
 */
#define IEEE802154_A_UINT_BACKOFF_PERIOD  (20)

/**
 * (IEEE 802.15.4-2006 6.4.1) PHY constants.
 */
#define IEEE802154_A_TURNAROUND_TIME      (12)

/**
 * (IEEE 802.15.4-2006 6.4.2) PHY PIB attribute, specifically the O-QPSK PHY.
 */
#define IEEE802154_PHY_SHR_DURATION       (10)

/**
 * (IEEE 802.15.4-2006 6.4.2) PHY PIB attribute, specifically the O-QPSK PHY.
 */
#define IEEE802154_PHY_SYMBOLS_PER_OCTET  (2)

/**
 * (IEEE 802.15.4-2006 7.4.2) macAckWaitDuration PIB attribute.
 */
#define IEEE802154_MAC_ACK_WAIT_DURATION                                       \
        (IEEE802154_A_UINT_BACKOFF_PERIOD + IEEE802154_A_TURNAROUND_TIME       \
         + IEEE802154_PHY_SHR_DURATION                                         \
         + ( 6 * IEEE802154_PHY_SYMBOLS_PER_OCTET))
/**
 * (IEEE 802.15.4-2006 6.5.3.2) O-QPSK symbol rate.
 */
#define IEEE802154_SYMBOLS_PER_SEC        (62500)

/**
 * Frequency of the Radio Timer module.
 *
 * 4MHz clock.
 */
#define PLATFORM_RADIO_RAT_TICKS_PER_SEC  (4000000)
/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */
// RF Core variables

// RF Core API commands
#if defined(COMBO_MAC) || defined(FREQ_2_4G)
extern rfc_CMD_RADIO_SETUP_PA_t RF_cmdRadioSetup;
extern rfc_CMD_IEEE_TX_t RF_cmdIEEETx;
extern rfc_CMD_IEEE_RX_t RF_cmdIEEERx;
extern rfc_CMD_IEEE_CSMA_t RF_cmdIEEECsma;
extern rfc_CMD_IEEE_RX_ACK_t RF_cmdRxAckCmd;
#endif
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
extern rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioDivSetup;
extern rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv;
extern rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv;
extern rfc_CMD_PROP_CS_t RF_cmdPropCs;
extern rfc_CMD_PROP_CS_t RF_cmdPropCsSlotted;
#endif
extern rfc_CMD_FS_t RF_cmdFsRx;
extern rfc_CMD_FS_t RF_cmdFsTx;
extern rfc_CMD_NOP_t RF_cmdNop;
extern rfc_CMD_NOP_t RF_cmdNopFg;
extern rfc_CMD_SCH_IMM_t RF_cmdScheduleImmediate;
extern rfc_CMD_SET_RAT_CMP_t RF_cmdRat;
extern rfc_CMD_SET_RAT_CMP_t RF_cmdRat1;
#endif // MAC_SETTINGS_H
