/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef DIAG_IFC_H
#define DIAG_IFC_H

//Write
#define TM_ADDR_SEMT_CW_ENA             0xFFFFF010UL
#define TM_ADDR_SEMT_MODEM              0xFFFFF014UL
#define TM_ADDR_SEMT_TX_PWR             0xFFFFF018UL
#define TM_ADDR_SEMT_MOD_ENA            0xFFFFF01CUL
#define TM_ADDR_SEMT_PING               0xFFFFF020UL
#define TM_ADDR_SEMT_RCV_PONG           0xFFFFF024UL
#define TM_ADDR_SEMT_RCV_PKT            0xFFFFF028UL
#define TM_ADDR_SEMT_RCV_PING           0xFFFFF029UL
#define TM_ADDR_SEMT_TX_RAW             0xFFFFF02CUL
#define TM_ADDR_SEMT_FREQ               0xFFFFF038UL
#define TM_ADDR_SEMT_RX_ENA             0xFFFFF03CUL
#define TM_ADDR_SEMT_GET_COUNTERS       0xFFFFF03DUL
#define TM_ADDR_SEMT_RESET_COUNTERS     0xFFFFF03EUL
#define TM_ADDR_SEMT_LORA_MOD           0xFFFFF041UL
#define TM_ADDR_SEMT_LORA_PKT           0xFFFFF042UL
#define TM_ADDR_SEMT_FSK_MOD            0xFFFFF043UL
#define TM_ADDR_SEMT_FSK_PKT            0xFFFFF044UL
#define TM_ADDR_SEMT_FHOP               0xFFFFF045UL
#define TM_ADDR_SEMT_LORA_TX_TIMEOUT    0xFFFFF046UL
#define TM_ADDR_SEMT_FSK_TX_TIMEOUT     0xFFFFF047UL
#define TM_ADDR_SEMT_LORA_CAD           0xFFFFF048UL
#define TM_ADDR_SEMT_SCAN               0xFFFFF049UL
#define TM_ADDR_SEMT_TIMONAIR           0xFFFFF050UL
#define TM_ADDR_SEMT_STATE              0xFFFFF055UL

//Read
#define TM_ADDR_FW_VERSION              0xFFFFF400UL
#define TM_ADDR_FW_VARIANT              0xFFFFF401UL
#define TM_ADDR_SERIAL_NUM              0xFFFFF422UL
#define TM_ADDR_RNET_DEV_ID             0xFFFFF423UL

// parameters length values
#define TM_ADDR_ARG_MODE_LEN                    4

#define TM_ADDR_SEMT_MODEM_PLD_LEN              1
#define TM_ADDR_SEMT_MODEM_ARG_NUM              1

#define TM_ADDR_SEMT_FREQ_PLD_LEN               4
#define TM_ADDR_SEMT_FREQ_ARG_NUM               1
#define TM_ADDR_SEMT_FREQ_ARG_FREQ_LEN          16

#define TM_ADDR_SEMT_TX_PWR_PLD_LEN             1
#define TM_ADDR_SEMT_TX_PWR_ARG_NUM             1
#define TM_ADDR_SEMT_TX_PWR_ARG_PWR_LEN         4

#define TM_ADDR_SEMT_LORA_MOD_PLD_LEN           3
#define TM_ADDR_SEMT_LORA_MOD_ARG_NUM           4
#define TM_ADDR_SEMT_LORA_MOD_ARG_LEN           4

#define TM_ADDR_SEMT_FSK_MOD_PLD_LEN            10
#define TM_ADDR_SEMT_FSK_MOD_ARG_NUM            5
#define TM_ADDR_SEMT_FSK_MOD_ARG_BR_LEN         16 // bit rate
#define TM_ADDR_SEMT_FSK_MOD_ARG_FD_LEN         16 // frequency division
#define TM_ADDR_SEMT_FSK_MOD_ARG_MS_LEN         4  // mod shaping
#define TM_ADDR_SEMT_FSK_MOD_ARG_BW_LEN         4  // bandwith

#define TM_ADDR_SEMT_LORA_PKT_PLD_LEN           6
#define TM_ADDR_SEMT_LORA_PKT_ARG_NUM           6
#define TM_ADDR_SEMT_LORA_PKT_ARG_PREAMBLE_LEN  8  // preamble length
#define TM_ADDR_SEMT_LORA_PKT_ARG_HT_LEN        4  // header type
#define TM_ADDR_SEMT_LORA_PKT_ARG_CM_LEN        4  // crc mode
#define TM_ADDR_SEMT_LORA_PKT_ARG_IQ_LEN        4  // invert IQ
#define TM_ADDR_SEMT_LORA_PKT_ARG_PAYLOAD_LEN   4

#define TM_ADDR_SEMT_FSK_PKT_PLD_LEN            9
#define TM_ADDR_SEMT_FSK_PKT_ARG_NUM            9
#define TM_ADDR_SEMT_FSK_PKT_ARG_PRAMBLE_LEN    8
#define TM_ADDR_SEMT_FSK_PKT_ARG_PMD_LEN        4  // preamble min detect
#define TM_ADDR_SEMT_FSK_PKT_ARG_SW_LEN         4  // sync word
#define TM_ADDR_SEMT_FSK_PKT_ARG_AC_LEN         4  // addr comp
#define TM_ADDR_SEMT_FSK_PKT_ARG_HT_LEN         4  // header type
#define TM_ADDR_SEMT_FSK_PKT_PAYLOAD_LEN        4
#define TM_ADDR_SEMT_FSK_PKT_CRC_TYPE_LEN       4
#define TM_ADDR_SEMT_FSK_PKT_RWM_LEN            4  // radio whitening mode

#define TM_ADDR_SEMT_LORA_CAD_PLD_LEN           8
#define TM_ADDR_SEMT_LORA_CAD_ARG_NUM           5
#define TM_ADDR_SEMT_LORA_CAD_ARG_SYM_NUM_LEN   4  // symbol number
#define TM_ADDR_SEMT_LORA_CAD_ARG_DET_PEAK_LEN  4  // detecet peak
#define TM_ADDR_SEMT_LORA_CAD_ARG_DET_MIN_LEN   4  // detecet minumium
#define TM_ADDR_SEMT_LORA_CAD_ARG_EXIT_MODE_LEN 4
#define TM_ADDR_SEMT_LORA_CAD_ARG_TMO_LEN       16 // timeout

#define TM_ADDR_SEMT_FSK_TX_TIMEOUT_PLD_LEN     1
#define TM_ADDR_SEMT_FSK_TX_TIMEOUT_ARG_NUM     2
#define TM_ADDR_SEMT_FSK_TX_TIMEOUT_ARG_TMO_LEN 4

#define TM_ADDR_SEMT_LORA_TX_TIMEOUT_PLD_LEN     1
#define TM_ADDR_SEMT_LORA_TX_TIMEOUT_ARG_NUM     2
#define TM_ADDR_SEMT_LORA_TX_TIMEOUT_ARG_TMO_LEN 4

#define TM_ADDR_SEMT_MOD_ENA_PLD_LEN_SHORT      1
#define TM_ADDR_SEMT_MOD_ENA_PLD_LEN_MED        9
#define TM_ADDR_SEMT_MOD_ENA_PLD_LEN_LONG       11
#define TM_ADDR_SEMT_MOD_ENA_PLD_ARG_NUM_SHORT  1
#define TM_ADDR_SEMT_MOD_ENA_PLD_ARG_NUM_LONG   4
#define TM_ADDR_SEMT_MOD_ENA_PLD_ARG_EN_LEN     4  // enable
#define TM_ADDR_SEMT_MOD_ENA_PLD_ARG_DELAY_LEN  16
#define TM_ADDR_SEMT_MOD_ENA_PLD_ARG_THR_NUM    8  // threshold
#define TM_ADDR_SEMT_MOD_ENA_PLD_ARG_ST_NUM     16 // sense time

#define TM_ADDR_SEMT_STATE_PLD_LEN              1
#define TM_ADDR_SEMT_STATE_ARG_NUM              1
#define TM_ADDR_SEMT_STATE_ARG_STATE_LEN        4

#define TM_ADDR_SEMT_RX_ENA_PLD_LEN             1
#define TM_ADDR_SEMT_RX_ENA_ARG_NUM             1
#define TM_ADDR_SEMT_RX_ENA_ARG_STATE_LEN       4

#define TM_ADDR_SEMT_PING_PLD_LEN               7
#define TM_ADDR_SEMT_PING_ARG_NUM               4
#define TM_ADDR_SEMT_PING_ARG_LEN               8

#define TM_ADDR_SEMT_FHOP_PLD_LEN_SHORT         1
#define TM_ADDR_SEMT_FHOP_PLD_LEN_LONG          15
#define TM_ADDR_SEMT_FHOP_ARG_NUM_MIN           1
#define TM_ADDR_SEMT_FHOP_ARG_NUM_MAX           5
#define TM_ADDR_SEMT_FHOP_ARG_EN_LEN            4
#define TM_ADDR_SEMT_FHOP_ARG_SF_LEN            16 // start frequency
#define TM_ADDR_SEMT_FHOP_ARG_EF_LEN            16 // end frequency
#define TM_ADDR_SEMT_FHOP_ARG_SEP_LEN           16
#define TM_ADDR_SEMT_FHOP_ARG_DW_TIME_LEN       8

void diag_ifc_init(const sid_pal_serial_client_factory_t *serial_ifc);
void diag_ifc_process(void);

#endif