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

#ifndef RNET_UART_H
#define RNET_UART_H

#include <sid_protocol_opts.h>
#include "rnet_host_interface.h"
#include <sid_protocol_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Structure for UART event. */
typedef struct
{
    uint8_t      * p_data; ///< Pointer to memory used for transfer
    uint16_t     length;   ///< Number of bytes transfered.
    uint8_t      flags;     // bit 0 - rx done, bit 1 - tx done, both set for syncronous (SPIS), bits 4-7 - rfu for error handling
} rnet_uart_evt_t;

/**
 * @brief Uart event handler.
 *
 * @param[in] p_evt  Pointer to event structure. Event is allocated on the stack so it is available
 *                   only within the context of the event handler.
 */
typedef void (*rnet_uart_evt_handler_t)(rnet_uart_evt_t * p_evt);


/*API*/

/**@brief Init uart.
 *
 * @param[in]   evt_handler     rnet_uart_evt_handler_t handler to handle  rnet_uart_evt_t events by upper layer.
 *
 * @return NRF_SUCCESS on success, otherwise error code.
 */
ret_code_t rnet_uart_init(rnet_uart_evt_handler_t evt_handler);

/**@brief Send payload through UART.
 *
 * @param[in]   buffer      Payload to send,
 * @param[in]   length      Payload length.
 * @param[in]   retry       Resend data or not.
 *
 * @return NRF_SUCCESS on success, otherwise error code.
 */
ret_code_t rnet_uart_start_xfer(uint8_t* buffer, uint8_t length, bool retry);


/**@brief Get UART DMA MTU size.
 *
 * @param[out]  mtu    UART MTU size.
 */
void rnet_uart_get_mtu(uint8_t *mtu);

/**@brief UART data processor.
 *
 */
void rnet_uart_process(void);

/**@brief Resend last sent data. (Need validation)
 *
 */
ret_code_t rnet_host_uart_resend(void);


/**@brief Deinit UART.
 *
 * @return NRF_SUCCESS on success, otherwise error code.
 */
ret_code_t rnet_uart_deinit(void);

#ifdef __cplusplus
}
#endif


#endif
