/*
 * Copyright (c) 2019-2020 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UART_SIMPLELINK_TRANSPORT_H_
#define _EMBEDDED_RPC__UART_SIMPLELINK_TRANSPORT_H_

#include <stdlib.h>

#include "erpc_framed_transport.h"
#include <ti/drivers/UART.h>

/*!
 * @addtogroup uart_simplelink
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Transport to send and receive messages via UART with TI-Drivers
 *
 * @ingroup uart_simplelink
 */
class UartSimpleLinkTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor
     */
    UartSimpleLinkTransport(void)
    : m_uart(NULL)
    {
        UART_init();
    }

    /*!
     * @brief Destructor
     */
    virtual ~UartSimpleLinkTransport(void)
    {
        UART_close(m_uart);
    }

    /*!
     * @brief Initialize SimpleLink UART
     *
     * @param[in] uart_index Index to the UART attribute array
     * @param[in] baud_rate Baud rate for UART
     *
     * @retval kErpcStatus_InitFailed When init function failed
     * @retval kErpcStatus_Success When init function succeeded
     */
    virtual erpc_status_t init(uint8_t uart_index, uint32_t baud_rate);

protected:
    /*!
     * @brief Receive data from UART
     *
     * @param[out] data Pre-allocated buffer for receiving data
     * @param[in] size Size of data to read
     *
     * @retval kErpcStatus_ReceiveFailed UART failed to receive data
     * @retval kErpcStatus_Success Successfully received all data
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

    /*!
     * @brief Write data to UART
     *
     * @param[in] data Buffer to send
     * @param[in] size Size of data to send
     *
     * @retval kErpcStatus_Success Always returns success
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size);

private:
    UART_Handle m_uart;         /*!< UART driver Handle */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__UART_SIMPLELINK_TRANSPORT_H_
