/*
 * Copyright (c) 2019-2020 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_uart_simplelink_transport.h"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<UartSimpleLinkTransport> s_transport;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_uart_simplelink_init(const uint8_t uart_index, uint32_t baud_rate)
{
    erpc_transport_t t = NULL;

    s_transport.construct();
    if (s_transport->init(uart_index, baud_rate) == kErpcStatus_Success) {
        t = reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    return (t);
}
