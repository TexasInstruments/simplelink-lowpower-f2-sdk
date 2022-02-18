/*
 * Copyright (c) 2019-2020 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_uart_simplelink_transport.h"
#include <cstdio>
#if ERPC_THREADS_IS(ERPC_THREADS_NONE)
#include <ti/drivers/dpl/ClockP.h>
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
#define ERPC_READ_TIMEOUT 1000  // usecs

erpc_status_t UartSimpleLinkTransport::init(uint8_t uart_index, uint32_t baud_rate)
{
    UART_Params params;

    UART_Params_init(&params);

#if ERPC_THREADS_IS(ERPC_THREADS_NONE)
    // add one tick to ensure at least one full tick wait period
    params.readTimeout = (ClockP_tickPeriod > ERPC_READ_TIMEOUT
        ? 1 : ERPC_READ_TIMEOUT / ClockP_tickPeriod) + 1;
#endif

    params.baudRate = baud_rate;
    params.readDataMode = UART_DATA_BINARY;
    params.readEcho = UART_ECHO_OFF;
    params.writeDataMode = UART_DATA_BINARY;

    m_uart = UART_open(uart_index, &params);

    return (m_uart == NULL ? kErpcStatus_InitFailed : kErpcStatus_Success);
}

erpc_status_t UartSimpleLinkTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    uint32_t nbytes;

    nbytes = UART_read(m_uart, data, size);

    // caller expects to receive the exact number of bytes
    return (nbytes == size ? kErpcStatus_Success : kErpcStatus_ReceiveFailed);
}

erpc_status_t UartSimpleLinkTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    uint32_t nbytes;

    nbytes = UART_write(m_uart, data, size);
    return (nbytes == size ? kErpcStatus_Success : kErpcStatus_SendFailed);
}
