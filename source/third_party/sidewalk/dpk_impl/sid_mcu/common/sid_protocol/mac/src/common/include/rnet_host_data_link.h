/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNET_HOST_DATA_LINK_H
#define RNET_HOST_DATA_LINK_H

#include <sid_protocol_opts.h>

#ifdef __cplusplus
extern "C" {
#endif

//Max data size
#define RNET_HOST_DL_DF_HDR_SZ          3
#define RNET_HOST_DL_DF_CRC_SZ          2
//Calculate max payload in XFER dependent on interface MTU.
#define RNET_HOST_DL_PAYLOAD_IN_XFER(x)    ((x) - RNET_HOST_DL_DF_HDR_SZ - RNET_HOST_DL_DF_CRC_SZ)

#define RNET_HOST_DL_FT_DF      0x02
#define RNET_HOST_DL_FT_ACK     0x03
#define RNET_HOST_DL_FT_NACK    0x04
#define RNET_HOST_DL_FT_STATUS  0x05

#define RNET_HOST_DL_FT_LOG     0xD0

#define RNET_HOST_DL_ACK_FRAME_LENGTH 4
#define RNET_HOST_DL_NACK_FRAME_LENGTH 5


#define RNET_HOST_DL_NACK_FLAG_CRC            0x80
#define RNET_HOST_DL_NACK_FLAG_OVERFLOW       0x40
#define RNET_HOST_DL_NACK_FLAG_DUPLICATE      0x20
#define RNET_HOST_DL_NACK_FLAG_OTHER          0x10
#define RNET_HOST_DL_NACK_FLAG_BACK_PRESSURE  0x01

#define RNET_HOST_DL_RESEND_TIMEOUT_FAST    100
#define RNET_HOST_DL_RESEND_TIMEOUT_SLOW    1000

typedef void(*rnet_host_dl_tx_done_cb)(void);
typedef int8_t (*rnet_host_dl_rx_done_cb)(uint8_t*, uint16_t);

/** @brief Structure that keep TX payload, TX state etc */
typedef struct {
    uint8_t frame_buf[SRL_HOST_MAX_FRAME_SZ]; //TODO: move to pointers with dynamic allocation
    uint16_t frame_len;
    uint16_t frame_idx; // deprecated
    uint16_t frame_offset;

    volatile uint8_t tx_in_progress; // Valid transfer to host prepared

    uint8_t frame_to_send; // Data frame needs go out

    uint8_t send_df; // Data frame needs go out
    uint8_t send_ack; // Ack needs to go out
    uint8_t send_nack; // Nack needs to go out
    uint8_t send_status; // status needs to go out: 0x01-send, 0x02-resend
    uint8_t send_log; // Log frame needs go out
    uint8_t send_null;

    uint8_t df_send_seqn;
    uint8_t nack_send_seqn;

    uint8_t nack_flags;
//    uint8_t nack_flag_crc;
//    uint8_t nack_flag_overflow;
//    uint8_t nack_flag_pld_len;    // unused
//    uint8_t nack_flag_duplicate;
//    uint8_t nack_flag_reassembly_overflow;  //unused
//    uint8_t nack_flag_other;            //unused

    uint8_t backpressure_detected;

    uint8_t last_sent_ft;
    uint8_t last_df_pld_len;

    uint8_t waiting_for_ack_nack;
} rnet_host_dl_tx_st_t;

/** @brief Structure that keep RX payload, RX state etc */
typedef struct {
    uint8_t frame_buf[SRL_HOST_MAX_FRAME_SZ]; //TODO: move to pointers with dynamic allocation
    uint16_t frame_len;
    uint8_t frame_vld;
    uint8_t last_rcvd_df_seqn;
} rnet_host_dl_rx_st_t;


/*API*/

/**@brief Init uart.
 *
 * @param[in]   tx_cb                  Callback to handle TX done event.
 * @param[in]   rx_cb                  Callback to handle RX done event Pass payload to upper layer.
 * @param[in]   max_payload_length     Max decoded payload length.
 *
 * @return NRF_SUCCESS on success, otherwise error code.
 */
ret_code_t rnet_host_dl_init(rnet_host_dl_tx_done_cb tx_cb, rnet_host_dl_rx_done_cb rx_cb, uint16_t max_payload_length);

/**@brief Serial interface(s) data processor.
 *
 */
void rnet_host_dl_process(void);

/**@brief Send payload through serail interface (fragmentation and SLIP
 * encoding(option) will be performed inside).
 *
 * @param[in]   frame_buf      Payload to send,
 * @param[in]   len            Payload length.
 *
 * @return 0 on success, otherwise error code.
 */
int8_t rnet_host_dl_send(uint8_t* frame_buf, uint16_t len);

#ifdef __cplusplus
}
#endif


#endif //RNET_HOST_DATA_LINK_H
