/******************************************************************************

 @file  cc13xx.h

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

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

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef __CC13XX_H__
#define __CC13XX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Packet RX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8    /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             2047 /* Max length byte the radio will accept 2+2045 */
#define NUM_DATA_ENTRIES       1    /* NOTE: upto two data entries supported by ti driver. at the moment using one entry for rx */
#define NUM_APPENDED_BYTES     10 /* The Data Entries data field will contain:
                                   * 1 Header byte (RF_cmdPropRxAdv.rxConf.bIncludeHdr = 0x1)
                                   * Max 2047 payload bytes
                                   * 1 status byte (RF_cmdPropRxAdv.rxConf.bIncludeCrc = 0x1) 2 or 4
                                   * 1 status byte (RF_cmdPropRxAdv.rxConf.bAppendRssi = 0x1) 1
                                   * 1 status byte (RF_cmdPropRxAdv.rxConf.bAppendTimestamp = 0x1) 4
                                   * 1 status byte (RF_cmdPropRxAdv.rxConf.bAppendStatus = 0x1) 1 */

#define REAR_APPENDED_8_BYTES                   8
#define REAR_APPENDED_10_BYTES                  10
#define REAR_APPENDED_RSSI_OFFSET               6

#define PHR_PAYLOAD_CRC_LEN_LOW                 0xff
#define PHR_PAYLOAD_CRC_LEN_HIG                 0x07
#define PHR_FIELD_MODE                          0x80
#define PHR_FIELD_FCS_TYPE                      0x10
#define PHR_FIELD_WHITEN                        0X08
#define PHR_FIELD_PAYCRC_LEN_HIG_SHIFT          8
#define PHR_FIELD_MODE_SHIFT                    7
#define PHR_FIELD_FCS_TYPE_SHIFT                4
#define PHR_FIELD_WHITEN_SHIFT                  3
#define PHR_FCS_LEN_2_BYTES                     2
#define PHR_FCS_LEN_4_BYTES                     4
#define PHR_LEN_IN_BYTES                        2
#define PHR_TYPE_OFFSET                         0
#define PHR_LENGTH_OFFSET                       1
#define PHR_LENGTH_LBYTE(x)                     (x & 0xFF)
#define PHR_LENGTH_HBYTE(x)                     ((x >> 8) & 0x07)

#define FRAME_CRC_BYTES_ZERO                    0
#define FRAME_CRC_BYTES_ONE                     1
#define FRAME_CRC_BYTES_TWO                     2
#define FRAME_HDR_LEN_IN_BITS                   16

#define BIT_0                                   (1<<0)
#define BIT_IN_BYTE                             8
#define DIV_TO_MS                               64000
#define MS_IN_SEC                               1000U
#define NS_IN_MS                                1000000U
#define HZ_IN_MHZ                               1000000
#define HZ_IN_KHZ                               1000

#define HZ_TO_MHZ(x)                            (x / HZ_IN_MHZ)
#define HZ_TO_KHZ(x)                            (x / HZ_IN_MHZ)
#define MHZ_TO_HZ(x)                            (x * HZ_IN_MHZ)
#define KHZ_TO_HZ(x)                            (x * HZ_IN_KHZ)

#define RADIO_FSK_BR_50KBPS                     50000
#define RADIO_FSK_BR_150KBPS                    150000
#define RADIO_FSK_BR_250KBPS                    250000

#define RADIO_FSK_FDEV_25KHZ                    25000
#define RADIO_FSK_FDEV_37_5KHZ                  37500
#define RADIO_FSK_FDEV_62_5KHZ                  62500

#define TI_FSK_BR_50KBPS                        0x8000
#define TI_FSK_BR_150KBPS                       0x18000
#define TI_FSK_BR_250KBPS                       0x28000

#define TI_FSK_BR_50KBPS_DEVI_19KHZ             0x4C
#define TI_FSK_BR_50KBPS_DEVI_25KHZ             0x64
#define TI_FSK_BR_150KBPS_DEVI_57KHZ            0xE4
#define TI_FSK_BR_250KBPS_DEVI_62_5KHZ          0xFA

#define TI_FSK_BR_50KBPS_RXBW_98KHZ             0x52
#define TI_FSK_BR_50KBPS_RXBW_155_4KHZ          0x55
#define TI_FSK_BR_150KBPS_RXBW_310_8KHZ         0x59
#define TI_FSK_BR_250KBPS_RXBW_470_9KHZ         0x5B
#define TI_FSK_BR_250KBPS_RXBW_546_3KHZ         0x5C

#define TX_PWR_PLUS_20                          20
#define TX_PWR_PLUS_14                          14
#define MIN_CHANNEL_FREE_DELAY_US               1
#define NOISE_SAMPLE_SIZE                       32
#define RF_RANDOM_TIMES                         8

#define FSK_MICRO_SECS_PER_SYMBOL               250
#define MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_0      251
#define MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_1      253
#define CC13XX_FSK_MAX_PAYLOAD_LEN              255
#define CC13XX_RF_NOISE_FLOOR                   -90
#define CC13XX_PHR_TYPE_OFFSET                  1
#define CC13XX_PHR_LENGTH_OFFSET                0
#define CC13XX_MAX_SYNCWORD_BYTES               4
#define CC13XX_RAT_TICKS_PER_US                 4
#define CC13XX_SHR_LONG_PREAMBLE_LEN            200000
#define CC13XX_CS_RSSI_MODE                     0
#define CC13XX_CS_PREAMBLE_MODE                 1
#define CC13XX_CS_NEXT_RX                       0
#define CC13XX_CS_NEXT_TX                       1
#define CC13XX_RX_FOR_SHORT_WINDOW              1
#define CC13XX_SYMBOL_PERIOD_US(rate)           (1000000 / rate)
/**
 * @brief CC13XX APIs return status enumeration definition
 */
typedef enum cc13xx_status_e
{
    CC13XX_STATUS_OK = 0,
    CC13XX_STATUS_UNSUPPORTED_FEATURE,
    CC13XX_STATUS_UNKNOWN_VALUE,
    CC13XX_STATUS_ERROR,
} cc13xx_status_t;

/**
 * @brief CC13XX GFSK modulation shaping enumeration definition
 */
typedef enum cc13xx_gfsk_mod_shapes_e
{
    CC13XX_GFSK_MOD_SHAPE_OFF   = 0x00,
    CC13XX_GFSK_MOD_SHAPE_BT_03 = 0x08,
    CC13XX_GFSK_MOD_SHAPE_BT_05 = 0x09,
    CC13XX_GFSK_MOD_SHAPE_BT_07 = 0x0A,
    CC13XX_GFSK_MOD_SHAPE_BT_1  = 0x0B,
} cc13xx_gfsk_mod_shapes_t;

/**
 * @brief CC13XX GFSK Rx bandwidth enumeration definition
 */
typedef enum cc13xx_gfsk_bw_e
{
    CC13XX_GFSK_BW_4800   = 0x1F,
    CC13XX_GFSK_BW_5800   = 0x17,
    CC13XX_GFSK_BW_7300   = 0x0F,
    CC13XX_GFSK_BW_9700   = 0x1E,
    CC13XX_GFSK_BW_11700  = 0x16,
    CC13XX_GFSK_BW_14600  = 0x0E,
    CC13XX_GFSK_BW_19500  = 0x1D,
    CC13XX_GFSK_BW_23400  = 0x15,
    CC13XX_GFSK_BW_29300  = 0x0D,
    CC13XX_GFSK_BW_39000  = 0x1C,
    CC13XX_GFSK_BW_46900  = 0x14,
    CC13XX_GFSK_BW_58600  = 0x0C,
    CC13XX_GFSK_BW_78200  = 0x1B,
    CC13XX_GFSK_BW_93800  = 0x13,
    CC13XX_GFSK_BW_117300 = 0x0B,
    CC13XX_GFSK_BW_156200 = 0x1A,
    CC13XX_GFSK_BW_187200 = 0x12,
    CC13XX_GFSK_BW_234300 = 0x0A,
    CC13XX_GFSK_BW_312000 = 0x19,
    CC13XX_GFSK_BW_373600 = 0x11,
    CC13XX_GFSK_BW_467000 = 0x09,
} cc13xx_gfsk_bw_t;

/**
 * @brief CC13XX GFSK modulation parameters structure definition
 */
typedef struct cc13xx_mod_params_gfsk_s
{
    uint32_t                 br_in_bps;
    uint32_t                 fdev_in_hz;
    cc13xx_gfsk_mod_shapes_t mod_shape;
    cc13xx_gfsk_bw_t         bw_dsb_param;
} cc13xx_mod_params_gfsk_t;


/**
 * @brief CC13XX GFSK preamble length Rx detection size enumeration definition
 */
typedef enum cc13xx_gfsk_pbl_det_e
{
    CC13XX_GFSK_PBL_DET_OFF     = 0x00,
    CC13XX_GFSK_PBL_DET_08_BITS = 0x04,
    CC13XX_GFSK_PBL_DET_16_BITS = 0x05,
    CC13XX_GFSK_PBL_DET_24_BITS = 0x06,
    CC13XX_GFSK_PBL_DET_32_BITS = 0x07,
} cc13xx_gfsk_pbl_det_t;

/**
 * @brief CC13XX GFSK address filtering configuration enumeration definition
 */
typedef enum cc13xx_gfsk_addr_cmp_e
{
    CC13XX_GFSK_ADDR_CMP_FILT_OFF        = 0x00,
    CC13XX_GFSK_ADDR_CMP_FILT_NODE       = 0x01,
    CC13XX_GFSK_ADDR_CMP_FILT_NODE_BROAD = 0x02,
} cc13xx_gfsk_addr_cmp_t;

/**
 * @brief CC13XX GFSK packet length enumeration definition
 */
typedef enum cc13xx_gfsk_pkt_len_modes_e
{
    CC13XX_GFSK_PKT_FIX_LEN = 0x00,  //!< The packet length is known on both sides, no header included
    CC13XX_GFSK_PKT_VAR_LEN = 0x01,  //!< The packet length is variable, header included
} cc13xx_gfsk_pkt_len_modes_t;

/**
 * @brief CC13XX GFSK CRC type enumeration definition
 */
typedef enum cc13xx_gfsk_crc_types_e
{
    CC13XX_GFSK_CRC_OFF         = 0x01,
    CC13XX_GFSK_CRC_1_BYTE      = 0x00,
    CC13XX_GFSK_CRC_2_BYTES     = 0x02,
    CC13XX_GFSK_CRC_1_BYTE_INV  = 0x04,
    CC13XX_GFSK_CRC_2_BYTES_INV = 0x06,
} cc13xx_gfsk_crc_types_t;

/**
 * @brief CC13XX GFSK whitening control enumeration definition
 */
typedef enum cc13xx_gfsk_dc_free_e
{
    CC13XX_GFSK_DC_FREE_OFF       = 0x00,
    CC13XX_GFSK_DC_FREE_WHITENING = 0x01,
} cc13xx_gfsk_dc_free_t;

/**
 * @brief CC13XX Radio Operation State
 */
typedef enum {
    CC13XX_RADIO_UNKNOWN = (0 << 0),    // 0x00
    CC13XX_RADIO_TX      = (1 << 0),    // 0x01
    CC13XX_RADIO_RX      = (1 << 1),    // 0x02
    CC13XX_RADIO_CW      = (1 << 2),    // 0x04
    CC13XX_RADIO_SLEEP   = (1 << 3),    // 0x08
    CC13XX_RADIO_STANDBY = (1 << 4),    // 0x10
    CC13XX_RADIO_CAD     = (1 << 5),    // 0x20
} cc13xx_radio_state_t;

/**
 * @brief CC13XX GFSK packet parameters structure definition
 */
typedef struct cc13xx_pkt_params_gfsk_s
{
    uint16_t                    pbl_len_in_bits;        //!< Preamble length in bits
    cc13xx_gfsk_pbl_det_t       pbl_min_det;            //!< Preamble detection length
    uint8_t                     sync_word_len_in_bits;  //!< Sync word length in bits
    cc13xx_gfsk_addr_cmp_t      addr_cmp;               //!< Address filtering configuration
    cc13xx_gfsk_pkt_len_modes_t hdr_type;               //!< Header type
    uint8_t                     pld_len_in_bytes;       //!< Payload length in bytes
    cc13xx_gfsk_crc_types_t     crc_type;               //!< CRC type configuration
    cc13xx_gfsk_dc_free_t       dc_free;                //!< Whitening configuration
} cc13xx_pkt_params_gfsk_t;

uint16_t cc13xx_get_rx_entry( uint8_t *phr, uint8_t *buffer, int8_t *rssi);
uint32_t cc13xx_get_gfsk_time_on_air_in_ms( const cc13xx_pkt_params_gfsk_t* pkt_p,
                                            const cc13xx_mod_params_gfsk_t* mod_p );

cc13xx_status_t cc13xx_platform_init(void);
cc13xx_status_t cc13xx_set_sleep(void);
cc13xx_status_t cc13xx_set_standby(void);
cc13xx_status_t cc13xx_radio_start_rx(uint32_t timeout, bool continuous);
cc13xx_status_t cc13xx_set_tx_payload(const uint8_t *buffer, uint8_t size);
cc13xx_status_t cc13xx_radio_start_tx(uint32_t timeout);
cc13xx_status_t cc13xx_set_tx_power(const int8_t tx_power);
cc13xx_status_t cc13xx_set_rf_frequency(const uint32_t freq);
cc13xx_status_t cc13xx_get_rssi_inst(int16_t *rssi);
cc13xx_status_t cc13xx_get_random_numbers(uint32_t* numbers, unsigned int n);
cc13xx_status_t cc13xx_set_gfsk_sync_word(const uint8_t* sync_word, const uint8_t sync_word_len);
cc13xx_status_t cc13xx_set_gfsk_mod_params(const cc13xx_mod_params_gfsk_t* params);
cc13xx_status_t cc13xx_set_gfsk_pkt_params(const cc13xx_pkt_params_gfsk_t* params);
cc13xx_status_t cc13xx_set_tx_cw(void);
cc13xx_status_t cc13xx_radio_start_carrier_sense(uint32_t timeout, sid_pal_radio_cad_param_exit_mode_t exit_mode);
void cc13xx_radio_process(void);

#ifdef __cplusplus
}
#endif

#endif  // __CC13XX_H__
