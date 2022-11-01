/*
 * Copyright (c) 2016-2018, Texas Instruments Incorporated
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

/***** Includes *****/

/* Application specific Header files */
#include "config.h"
#include "ti_radio_config.h"

/*
The values have been generated
with RF studio and are used to overwrite the CMD_FS
from ti_radio_config.c
*/
FrequencyTableEntry config_frequencyTable_2_4[] =
{
    { "2405  ", 0x0965, 0x0000, 0x0 },
    { "2410  ", 0x096a, 0x0000, 0x0 },
    { "2415  ", 0x096f, 0x0000, 0x0 },
    { "2420  ", 0x0974, 0x0000, 0x0 },
    { "2425  ", 0x0979, 0x0000, 0x0 },
    { "2430  ", 0x097e, 0x0000, 0x0 },
    { "2435  ", 0x0983, 0x0000, 0x0 },
    { "2440  ", 0x0988, 0x0000, 0x0 },
    { "2445  ", 0x098d, 0x0000, 0x0 },
    { "2450  ", 0x0992, 0x0000, 0x0 },
    { "2455  ", 0x0997, 0x0000, 0x0 },
    { "2460  ", 0x099c, 0x0000, 0x0 },
    { "2465  ", 0x09a1, 0x0000, 0x0 },
    { "2470  ", 0x09a6, 0x0000, 0x0 },
    { "2475  ", 0x09ab, 0x0000, 0x0 },
    { "2480  ", 0x09b0, 0x0000, 0x0 },
    { "", 0xFFFF, 0, 0 }, // Freq of 0xFFFF indicates the end of the table
};

FrequencyTableEntry config_frequencyTable_ble[] =
{
    { "2402  ", 0x0962, 0x0000, 0x65 },
    { "2404  ", 0x0964, 0x0000, 0x40 },
    { "2406  ", 0x0966, 0x0000, 0x41 },
    { "2408  ", 0x0968, 0x0000, 0x42 },
    { "2410  ", 0x096a, 0x0000, 0x43 },
    { "2412  ", 0x096c, 0x0000, 0x44 },
    { "2414  ", 0x096e, 0x0000, 0x45 },
    { "2416  ", 0x0970, 0x0000, 0x46 },
    { "2418  ", 0x0972, 0x0000, 0x47 },
    { "2420  ", 0x0974, 0x0000, 0x48 },
    { "2422  ", 0x0976, 0x0000, 0x49 },
    { "2424  ", 0x0978, 0x0000, 0x4a },
    { "2426  ", 0x097a, 0x0000, 0x66 },
    { "2428  ", 0x097c, 0x0000, 0x4b },
    { "2430  ", 0x097e, 0x0000, 0x4c },
    { "2432  ", 0x0980, 0x0000, 0x4d },
    { "2434  ", 0x0982, 0x0000, 0x4e },
    { "2436  ", 0x0984, 0x0000, 0x4f },
    { "2438  ", 0x0986, 0x0000, 0x50 },
    { "2440  ", 0x0988, 0x0000, 0x51 },
    { "2442  ", 0x098a, 0x0000, 0x52 },
    { "2444  ", 0x098c, 0x0000, 0x53 },
    { "2446  ", 0x098e, 0x0000, 0x54 },
    { "2448  ", 0x0990, 0x0000, 0x55 },
    { "2450  ", 0x0992, 0x0000, 0x56 },
    { "2452  ", 0x0994, 0x0000, 0x57 },
    { "2454  ", 0x0996, 0x0000, 0x58 },
    { "2456  ", 0x0998, 0x0000, 0x59 },
    { "2458  ", 0x099a, 0x0000, 0x5a },
    { "2460  ", 0x099c, 0x0000, 0x5b },
    { "2462  ", 0x099e, 0x0000, 0x5c },
    { "2464  ", 0x09a0, 0x0000, 0x5d },
    { "2466  ", 0x09a2, 0x0000, 0x5e },
    { "2468  ", 0x09a4, 0x0000, 0x5f },
    { "2470  ", 0x09a6, 0x0000, 0x60 },
    { "2472  ", 0x09a8, 0x0000, 0x61 },
    { "2474  ", 0x09aa, 0x0000, 0x62 },
    { "2476  ", 0x09ac, 0x0000, 0x63 },
    { "2478  ", 0x09ae, 0x0000, 0x64 },
    { "2480  ", 0x09b0, 0x0000, 0x67 },
    { "", 0xFFFF, 0, 0 }, //Freq of 0xFFFF indicates the end of the table
};

#if ((defined LAUNCHXL_CC1352P_4) || (defined LP_CC1352P7_4))
FrequencyTableEntry config_frequencyTable_sub1[] =
{
    { "433.92 ", 0x01B1, 0xEB9A, 0x0 },
    { "", 0xFFFF, 0, 0 }, // Freq of 0xFFFF indicates the end of the table
};
#else
FrequencyTableEntry config_frequencyTable_sub1[] =
{
    { "868.0  ", 0x0364, 0x0000, 0x0 },
    { "915.0  ", 0x0393, 0x0000, 0x0 },
    { "", 0xFFFF, 0, 0 }, // Freq of 0xFFFF indicates the end of the table
};
#endif


FrequencyTableEntry config_frequencyTable_custom[] =
{
    { "Custom  ", 1, 0x0000, 0x0 }, // Custom Phy mode only support freq defined in
                                    // ti_radio_config.c
    { "", 0xFFFF, 0, 0 }, // Freq of 0xFFFF indicates the end of the table
};

/*
Frequency and data rate table look up. Maps Frequency table and data rate table
to specific RF Mode
*/
#if ((defined CONFIG_CC26X2R1_LAUNCHXL) || (defined CONFIG_CC2652R1FRGZ) || \
     (defined CONFIG_LP_CC2652RSIP)     || (defined CONFIG_CC2652R1FSIP) || \
     (defined CONFIG_LP_CC2652PSIP)     || (defined CONFIG_CC2652P1FSIP) || \
     (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
     (defined CONFIG_LP_CC2651P3)       || (defined CONFIG_CC2651P3RGZ)  || \
     (defined CONFIG_LP_CC2651R3)       || (defined CONFIG_CC2651R3RGZ)  || \
     (defined CONFIG_LP_CC2674R10_FPGA) || (defined CONFIG_CC2674R10RGZ) || \
     (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
     (defined CONFIG_LP_CC2651R3SIPA)   || (defined CONFIG_CC2651R3SIPA) || \
     (defined CONFIG_LP_CC2653P10)      || (defined CONFIG_CC2653P10RGZ))
FrequencyTableEntry*  config_frequencyTable_Lut[] =
{
    config_frequencyTable_custom, // RfSetup_Custom - only frequency in custom command can be used.
    config_frequencyTable_2_4,    // RfSetup_2_4_Fsk_250
    config_frequencyTable_2_4,    // RfSetup_2_4_Fsk_100
    config_frequencyTable_ble,    // RfSetup_Ble
    config_frequencyTable_ble     // RfSetup_Ble5
};

uint32_t config_dataRateTable_Lut[] =
{
    250000,   // RfSetup_Custom 250 Kbps
    250000,   // RfSetup_2_4_Fsk_250 Kbps
    100000,   // RfSetup_2_4_Fsk_100 Kbps
    1000000,  // RfSetup_Ble 1 Mbps
    1000000   // RfSetup_Ble5 1 Mbps
};

const char* const config_rfSetupLabels[] =
{
    "Custom   ",
    "GFSK 250K",
    "GFSK 100K",
    "BLE Mode ",
    "BLE5 Mode"
};
#elif ((defined CONFIG_CC1352R1F3RGZ)     || (defined CONFIG_CC1312R1F3RGZ)     || \
       (defined CONFIG_CC1312R7RGZ)       || (defined CONFIG_CC1352P7RGZ)       || \
       (defined CONFIG_CC1312R1_LAUNCHXL) || (defined CONFIG_CC1352R1_LAUNCHXL) || \
       (defined LAUNCHXL_CC1352P1)        || (defined LAUNCHXL_CC1352P_2)       || \
       (defined LAUNCHXL_CC1352P_4)       || (defined CONFIG_LP_CC1312R7)       || \
       (defined LP_CC1352P7_1)            || (defined LP_CC1352P7_4)            || \
       (defined CONFIG_LP_CC1311P3)       || (defined CONFIG_CC1311P3RGZ)       || \
       (defined CONFIG_LP_CC1354R10_RGZ)  || (defined CONFIG_CC1354R10RGZ)      || \
       (defined CONFIG_LP_EM_CC1314R10)   || (defined CONFIG_CC1314R10RSK)      || \
       (defined LP_EM_CC1354P10_1)        || (defined LP_EM_CC1354P10_6)        || \
       (defined CONFIG_CC1354P10RSK)      || (defined CONFIG_LP_EM_CC1312PSIP)  || \
       (defined CONFIG_CC1312PSIP))
FrequencyTableEntry* config_frequencyTable_Lut[] =
{
    config_frequencyTable_custom, // RfSetup_Custom - only frequency in custom command can be used
#if (defined SUPPORT_FSK_50KBPS)
    config_frequencyTable_sub1,   // RfSetup_Fsk
#endif
#if (defined SUPPORT_FSK_200KBPS)
    config_frequencyTable_sub1,   // RfSetup_Fsk_200kbps
#endif
#if (defined SUPPORT_SLR)
    config_frequencyTable_sub1,   // RfSetup_Sl_lr
#endif
#if (defined SUPPORT_BLE)
    config_frequencyTable_ble,    // RfSetup_Ble
    config_frequencyTable_ble,    // RfSetup_Ble5
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
    config_frequencyTable_2_4,    // RfSetup_2_4_Fsk_250
    config_frequencyTable_2_4     // RfSetup_2_4_Fsk_100
#endif
};

uint32_t config_dataRateTable_Lut[] =
{
    50000,    // RfSetup_Custom 50 Kbps
#if (defined SUPPORT_FSK_50KBPS)
    50000,    // RfSetup_Fsk 50 Kbps
#endif
#if (defined SUPPORT_FSK_200KBPS)
    200000,   // RfSetup_Fsk_200kbps 200 Kbps
#endif
#if (defined SUPPORT_SLR)
    5000,     // RfSetup_Sl_lr 5 Kbps
#endif
#if (defined SUPPORT_BLE)
    1000000,  // RfSetup_Ble 1 Mbps
    1000000,  // RfSetup_Ble5 1 Mbps
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
    250000,   // RfSetup_2_4_Fsk_250 Kbps
    100000   // RfSetup_2_4_Fsk_100 Kbps
#endif
};

const char* const config_rfSetupLabels[] =
{
    "Custom   ",
#if (defined SUPPORT_FSK_50KBPS)
    "2-GFSK   ",
#endif
#if (defined SUPPORT_FSK_200KBPS)
    "GFSK 200K",
#endif
#if (defined SUPPORT_SLR)
    "SL LR    ",
#endif
#if (defined SUPPORT_BLE)
    "BLE Mode ",
    "BLE5 Mode",
#endif
#if (defined SUPPORT_PROP_2_4_GHZ)
    "GFSK 250K",
    "GFSK 100K"
#endif
};
#else
#error "Unsupported board or device being used"
#endif

const char* const config_testmodeLabels[] =
{
    "Rx",
    "Tx"
};

const uint32_t config_packetCountTable[] =
{
    10,
    100,
    1000,
    10000
};

const uint32_t config_payloadLengthTable[] =
{
    30,
    60,
    100,
    254
};

const char* const config_intervalLabels[] =
{
    "No ",
    "Yes"
};

#if (defined SUPPORT_HIGH_PA)
const char* const config_highPaLabels[] =
{
    "Disabled",
    "Enabled "
};
#endif

const uint8_t config_NrOfPacketCounts = (sizeof(config_packetCountTable)/sizeof(uint32_t));
const uint8_t config_NrOfPayloadLengths = (sizeof(config_payloadLengthTable)/sizeof(uint32_t));
