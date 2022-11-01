/*
 * Copyright (c) 2016-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "ti_drivers_config.h"
#include "ti_radio_config.h"

/* This file defines data types and variables for the application configuration */

/* PER version */
#define PER_VERSION "Ver 5.30.00"

#if !((defined CONFIG_CC26X2R1_LAUNCHXL) || (defined CONFIG_CC2652R1FRGZ) || \
      (defined CONFIG_LP_CC2652RSIP)     || (defined CONFIG_CC2652R1FSIP) || \
      (defined CONFIG_LP_CC2652PSIP)     || (defined CONFIG_CC2652P1FSIP) || \
      (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
      (defined CONFIG_LP_CC2651P3)       || (defined CONFIG_CC2651P3RGZ)  || \
      (defined CONFIG_LP_CC2651R3)       || (defined CONFIG_CC2651R3RGZ)  || \
      (defined CONFIG_LP_CC2651R3SIPA)   || (defined CONFIG_CC2651R3SIPA) || \
      (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
      (defined CONFIG_LP_CC2674R10_FPGA) || (defined CONFIG_CC2674R10RGZ) || \
      (defined CONFIG_LP_CC2653P10)      || (defined CONFIG_CC2653P10RGZ) || \
      (defined CONFIG_LP_CC2651R3SIPA)   || (defined CONFIG_CC2651R3SIPA))
#define SUPPORT_FSK_50KBPS
#if !((defined DeviceFamily_CC26X4) || (defined DeviceFamily_CC13X4) || (defined DeviceFamily_CC26X3))
#define SUPPORT_SLR
#endif
#endif
#if ((defined CONFIG_CC1352R1F3RGZ)      || (defined CONFIG_CC2652R1FRGZ)       || \
     (defined CONFIG_CC2652R1FSIP)       || (defined CONFIG_CC2652P1FSIP)       || \
     (defined CONFIG_CC2652R7RGZ)        || (defined CONFIG_CC1352P7RGZ)        || \
     (defined CONFIG_CC1352R1_LAUNCHXL)  || (defined CONFIG_CC26X2R1_LAUNCHXL)  || \
     (defined LAUNCHXL_CC1352P1)         || (defined LAUNCHXL_CC1352P_2)        || \
     (defined LAUNCHXL_CC1352P_4)        || (defined CONFIG_LP_CC2652RSIP)      || \
     (defined CONFIG_LP_CC2652PSIP)      || (defined CONFIG_LP_CC2652R7)        || \
     (defined LP_CC1352P7_1)             || (defined LP_CC1352P7_4)             || \
     (defined CONFIG_LP_CC2651P3)        || (defined CONFIG_CC2651P3RGZ)        || \
     (defined CONFIG_LP_CC2651R3)        || (defined CONFIG_CC2651R3RGZ)        || \
     (defined LP_CC1352P7_1)             || (defined LP_CC1352P7_4)             || \
     (defined CONFIG_LP_CC2674R10_FPGA)  || (defined CONFIG_CC2674R10RGZ)       || \
     (defined CONFIG_LP_CC2651R3SIPA)    || (defined CONFIG_CC2651R3SIPA)       || \
     (defined CONFIG_LP_CC2653P10)       || (defined CONFIG_CC2653P10RGZ))
#define SUPPORT_PROP_2_4_GHZ
#endif
#if !((defined CONFIG_CC1312R1_LAUNCHXL) || (defined CONFIG_CC1312R1F3RGZ)      || \
       (defined CONFIG_LP_CC1312R7)      || (defined CONFIG_CC1312R7RGZ)        || \
       (defined CONFIG_LP_CC1311P3)      || (defined CONFIG_CC1311P3RGZ)        || \
       (defined CONFIG_LP_EM_CC1314R10)  || (defined CONFIG_CC1314R10RSK)       || \
       (defined CONFIG_LP_EM_CC1312PSIP) || (defined CONFIG_CC1312PSIP))
#define SUPPORT_BLE
#endif
#if ((defined CONFIG_CC1312R1_LAUNCHXL) || (defined CONFIG_CC1352R1_LAUNCHXL)  || \
     (defined CONFIG_CC1352R1F3RGZ)     || (defined CONFIG_CC1312R1F3RGZ)      || \
     (defined LAUNCHXL_CC1352P1)        || (defined LAUNCHXL_CC1352P_2)        || \
     (defined CONFIG_LP_CC1312R7)       || (defined CONFIG_CC1312R7RGZ)        || \
     (defined LP_CC1352P7_1)            || (defined CONFIG_LP_CC1311P3)        || \
     (defined CONFIG_CC1311P3RGZ)       || (defined CONFIG_LP_EM_CC1312PSIP)   || \
     (defined CONFIG_CC1312PSIP))
#define SUPPORT_FSK_200KBPS
#endif
#if ((defined LAUNCHXL_CC1352P1) || (defined LAUNCHXL_CC1352P_2) || \
     (defined LAUNCHXL_CC1352P_4)|| (defined LP_CC1352P7_1)      || \
     (defined LP_CC1352P7_4)     || (defined LP_CC1311P3)        || \
     (defined LP_EM_CC1312PSIP))
#define SUPPORT_HIGH_PA
#endif

/*
RF basic settings as found in the typical settings section of Smart RF Studio.
Each one defines a test case in this application.
*/
#if ((defined CONFIG_CC26X2R1_LAUNCHXL) || (defined CONFIG_CC2652R1FRGZ) || \
     (defined CONFIG_LP_CC2652RSIP)     || (defined CONFIG_CC2652R1FSIP) || \
     (defined CONFIG_LP_CC2652PSIP)     || (defined CONFIG_CC2652P1FSIP) || \
     (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
     (defined CONFIG_LP_CC2651P3)       || (defined CONFIG_CC2651P3RGZ)  || \
     (defined CONFIG_LP_CC2651R3)       || (defined CONFIG_CC2651R3RGZ)  || \
     (defined CONFIG_LP_CC2651R3SIPA)   || (defined CONFIG_CC2651R3SIPA) || \
     (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
     (defined CONFIG_LP_CC2674R10_FPGA) || (defined CONFIG_CC2674R10RGZ) || \
     (defined CONFIG_LP_CC2653P10)      || (defined CONFIG_CC2653P10RGZ))
typedef enum
{
    RfSetup_Custom = 0,  // Imported from SmartRF Studio
    RfSetup_2_4_Fsk_250, // Frequency shift keying
    RfSetup_2_4_Fsk_100, // Frequency shift keying
    RfSetup_Ble,
    RfSetup_Ble5,
    NrOfRfSetups
} RfSetup;
#elif ((defined CONFIG_CC1352R1F3RGZ)     || (defined CONFIG_CC1312R1F3RGZ)     || \
       (defined CONFIG_CC1312R7RGZ)       || (defined CONFIG_CC1352P7RGZ)       || \
       (defined CONFIG_CC1312R1_LAUNCHXL) || (defined CONFIG_CC1352R1_LAUNCHXL) || \
       (defined LAUNCHXL_CC1352P1)        || (defined LAUNCHXL_CC1352P_2)       || \
       (defined LAUNCHXL_CC1352P_4)       || (defined CONFIG_LP_CC1312R7)       || \
       (defined LP_CC1352P7_4)            || (defined LP_CC1352P7_1)            || \
       (defined CONFIG_LP_CC1311P3)       || (defined CONFIG_CC1311P3RGZ)       || \
       (defined CONFIG_LP_CC1354R10_RGZ)  || (defined CONFIG_CC1354R10RGZ)      || \
       (defined CONFIG_LP_EM_CC1314R10)   || (defined CONFIG_CC1314R10RSK)      || \
       (defined CONFIG_LP_EM_CC1354P10_6) || (defined CONFIG_LP_EM_CC1354P10_1) || \
       (defined CONFIG_CC1354P10RSK)      || (defined CONFIG_LP_EM_CC1312PSIP)  || \
       (defined CONFIG_CC1312PSIP))

typedef enum
{
    RfSetup_Custom = 0,  // Imported from SmartRF Studio
#if (defined SUPPORT_FSK_50KBPS)
    RfSetup_Fsk,         // Frequency shift keying
#endif
#if (defined SUPPORT_FSK_200KBPS)
    RfSetup_Fsk_200kbps, // Frequency shift keying 200 Kbps
#endif
#if (defined SUPPORT_SLR)
    RfSetup_Sl_lr,       // SimpleLink Long Range
#endif
#if (defined SUPPORT_BLE)
    RfSetup_Ble,         // BLE
    RfSetup_Ble5,        // BLE5
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
    RfSetup_2_4_Fsk_250, // Frequency shift keying
    RfSetup_2_4_Fsk_100, // Frequency shift keying
#endif
    NrOfRfSetups
} RfSetup;
#else
#error "Unsupported board or device being used"
#endif

#if (defined SUPPORT_HIGH_PA)
typedef enum
{
    HighPa_Dis = 0,
    HighPa_En,
    NrOfHighPa
}HighPa;
#endif

/* Whether packets are sent back-to-back or with predefined intervals */
typedef enum
{
    IntervalMode_No = 0,
    IntervalMode_Yes,
    NrOfIntervalModes,
} IntervalMode;

/* Whether the application works as sender or receiver */
typedef enum
{
    TestMode_Rx = 0,
    TestMode_Tx,
    NrOfTestModes
} TestMode;

/* Contains a pre-defined setting for frequency selection */
typedef struct
{
    const char* const label;
    const uint16_t frequency;
    const uint16_t fractFreq;
    const uint8_t whitening; // BLE has freq dependent whitening settings
} FrequencyTableEntry;

/*
Holds the application config that is prepared in the menu and
used in the rx and tx functions.
*/
typedef struct
{
    RfSetup rfSetup;                     // Test case index
    IntervalMode intervalMode;           // Packet interval
    TestMode testMode;                   // TX/RX mode index
    uint32_t packetCount;                // Desired packet count
    uint32_t  payloadLength;             // Desired payload length (bytes)
    FrequencyTableEntry* frequencyTable; // FrequencyTable for this test case
    uint8_t frequency;                   // Index in config_frequencyTable
#if (defined SUPPORT_HIGH_PA)
    HighPa highPa;                       // CC1352P devices with built-in PA
#endif
} ApplicationConfig;

extern FrequencyTableEntry*  config_frequencyTable_Lut[]; // Lookup table for freq table
extern uint32_t config_dataRateTable_Lut[];               // Lookup table for data rates
extern const char* const config_testmodeLabels[];         // Lookup table for operation mode labels
extern const uint32_t config_packetCountTable[];          // Lookup table for different packet count options
extern const uint32_t config_payloadLengthTable[];        // Lookup table for different payload length options
extern const char* const config_rfSetupLabels[];          // Lookup table for RfSetup labels
extern const char* const config_intervalLabels[];         // Lookup table for interval mode labels
extern const char* const config_rangeExtenderLabels[];    // Lookup table for range extender labels
extern const char* const config_highPaLabels[];           // Lookup table for High PA labels
extern const uint8_t config_NrOfPacketCounts;             // Total amount of packet count options
extern const uint8_t config_NrOfPayloadLengths;           // Total size of payload options
#endif
