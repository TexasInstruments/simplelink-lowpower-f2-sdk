/******************************************************************************

 @file advanced_config.h

 @brief Advanced Configuration to Modify CSMA Parameters

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
#ifndef ADVANCED_H
#define ADVANCED_H

/******************************************************************************
 Includes
 *****************************************************************************/

/* Enable or disable EDFE Mode */
#define ENABLE_WISUN_EDFE_APPL false

/* Number of max data retries */
#define CONFIG_MAX_RETRIES 5

/* Max number of message failure to indicate sync loss */
#define CONFIG_MAX_DATA_FAILURES 5

/*! MAC MAX CSMA Backoffs */
#define CONFIG_MAC_MAX_CSMA_BACKOFFS 5

/* Coordinator short address when operating with FH enabled */
#define FH_COORD_SHORT_ADDR 0xAABB

/*!
 Maximum number of attempts for association in FH mode
 after reception of a PAN Config frame
 */
#define CONFIG_FH_MAX_ASSOCIATION_ATTEMPTS 3

/*!
 The number of non sleepy channel hopping end devices to be supported.
 The stack will allocate memory proportional to the number of end devices
 requested.
 */
#define FH_NUM_NON_SLEEPY_HOPPING_NEIGHBORS MAX_DEVICE_TABLE_ENTRIES

/*!
 The number of non sleepy fixed channel end devices to be supported.
 It is to be noted that the total number of non sleepy devices supported
 must be less than 50. Stack will allocate memory proportional
 to the number of end devices requested.
 */
#define FH_NUM_NON_SLEEPY_FIXED_CHANNEL_NEIGHBORS 2


#if (FH_NUM_NON_SLEEPY_HOPPING_NEIGHBORS < 2) || (FH_NUM_NON_SLEEPY_FIXED_CHANNEL_NEIGHBORS < 2)
#error "You have an invalid value for FH neighbors. Set the values \
        for FH_NUM_NON_SLEEPY_HOPPING_NEIGHBORS and \
        FH_NUM_NON_SLEEPY_FIXED_CHANNEL_NEIGHBORS to at least 2"
#endif

/*!
 Dwell time (ms): The duration for which the collector will
 stay on a specific channel before hopping to next channel.
 */
#define CONFIG_DWELL_TIME 250

/*! FH Poll/Sensor msg start time randomization window */
#define CONFIG_FH_START_POLL_DATA_RAND_WINDOW 10000

#if (((CONFIG_PHY_ID >= APIMAC_MRFSK_STD_PHY_ID_BEGIN) && (CONFIG_PHY_ID <= APIMAC_MRFSK_GENERIC_PHY_ID_BEGIN)) || \
    ((CONFIG_PHY_ID >= APIMAC_GENERIC_US_915_PHY_132) && (CONFIG_PHY_ID <= APIMAC_GENERIC_ETSI_863_PHY_133)))
/*! PAN Advertisement Solicit trickle timer duration in milliseconds */
#define CONFIG_PAN_ADVERT_SOLICIT_CLK_DURATION    6000
/*! PAN Config Solicit trickle timer duration in milliseconds */
#define CONFIG_PAN_CONFIG_SOLICIT_CLK_DURATION    6000
#else
/*! PAN Advertisement Solicit trickle timer duration in milliseconds */
#define CONFIG_PAN_ADVERT_SOLICIT_CLK_DURATION    60000
/*! PAN Config Solicit trickle timer duration in milliseconds */
#define CONFIG_PAN_CONFIG_SOLICIT_CLK_DURATION    60000
#endif

//FIXME
#define CONFIG_ROUTE_COST 0xFF

#endif /* ADVANCED_H */
