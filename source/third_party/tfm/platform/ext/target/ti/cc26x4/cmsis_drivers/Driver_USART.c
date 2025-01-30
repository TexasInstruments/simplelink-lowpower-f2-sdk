/*
 * Copyright (c) 2013-2018 ARM Limited. All rights reserved.
 * Copyright (c) 2018-2023, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This is a virtual USART driver which writes to a circular RAM buffer which is
 * read out using CCS Runtime Object Viewer (ROV).
 */

#include <stdio.h>

#include "Driver_USART.h"

#include "cmsis.h"
#include "platform_retarget_dev.h"
#include "RTE_Device.h"

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  (void)arg
#endif

/* Driver version */
#define ARM_USART_DRV_VERSION  ARM_DRIVER_VERSION_MAJOR_MINOR(2, 2)

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
    0, /* Transmit completed event: \ref ARM_USARTx_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USARTx_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USARTx_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USARTx_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USARTx_EVENT_DCD */
    0, /* Signal RI change event: \ref ARM_USARTx_EVENT_RI */
    0  /* Reserved */
};

static ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
    return DriverVersion;
}

static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
    return DriverCapabilities;
}

typedef struct {
    struct arm_uart_dev_t* dev;        /* UART device structure */
    uint32_t tx_nbr_bytes;             /* Number of bytes transfered */
    uint32_t rx_nbr_bytes;             /* Number of bytes recevied */
    ARM_USART_SignalEvent_t cb_event;  /* Callback function for events */
} UARTx_Resources;

static int32_t ARM_USARTx_PowerControl(UARTx_Resources* uart_dev,
                                       ARM_POWER_STATE state)
{
    ARG_UNUSED(uart_dev);

    switch (state) {
    case ARM_POWER_OFF:
    case ARM_POWER_LOW:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    case ARM_POWER_FULL:
        /* Nothing to be done */
        return ARM_DRIVER_OK;
    /* default:  The default is not defined intentionally to force the
     *           compiler to check that all the enumeration values are
     *           covered in the switch.*/
    }
}

static int32_t ARM_USARTx_Receive(UARTx_Resources* uart_dev,
                                  void *data, uint32_t num)
{
    uint8_t* p_data = (uint8_t*)data;

    if ((data == NULL) || (num == 0U)) {
        // Invalid parameters
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Resets previous RX counter */
    uart_dev->rx_nbr_bytes = 0;

    while(uart_dev->rx_nbr_bytes != num) {
        /* Waits until one character is received */
        while (!arm_uart_rx_ready(uart_dev->dev)){};

        /* As UART has received one byte, the read can not
         * return any receive error at this point */
        (void)arm_uart_read(uart_dev->dev, p_data);

        uart_dev->rx_nbr_bytes++;
        p_data++;
    }

    return ARM_DRIVER_OK;
}

static uint32_t ARM_USARTx_GetTxCount(UARTx_Resources* uart_dev)
{
    return uart_dev->tx_nbr_bytes;
}

static uint32_t ARM_USARTx_GetRxCount(UARTx_Resources* uart_dev)
{
    return uart_dev->rx_nbr_bytes;
}

#if (RTE_USART0)
/* USART0 Driver wrapper functions */
static UARTx_Resources USART0_DEV = {
#if (__DOMAIN_NS == 1)
    .dev = &ARM_UART0_DEV_NS,
#else
    .dev = &ARM_UART0_DEV_S,
#endif
    .tx_nbr_bytes = 0,
    .rx_nbr_bytes = 0,
    .cb_event = NULL,
};

#if (__DOMAIN_NS == 1)
    /* Non-Secure domain */
    #define uartOutputBuf      uartOutputBuf_ns
    #define uartOutputBufIndex uartOutputBufIndex_ns
    #define uartOutputBufCount uartOutputBufCount_ns
    #if defined(USART_BUFFER_FOR_TEST)
char uartOutputBuf_ns[16 * 1024];
    #else
char uartOutputBuf_ns[256];
    #endif
uint_fast16_t uartOutputBufIndex_ns;
uint_fast16_t uartOutputBufCount_ns;
#else
    /* Secure domain */
    #define uartOutputBuf      uartOutputBuf_s
    #define uartOutputBufIndex uartOutputBufIndex_s
    #define uartOutputBufCount uartOutputBufCount_s
    #if defined(USART_BUFFER_FOR_TEST)
char uartOutputBuf_s[20 * 1024];
    #else
char uartOutputBuf_s[256];
    #endif
uint_fast16_t uartOutputBufIndex_s;
uint_fast16_t uartOutputBufCount_s;
#endif

static int32_t ARM_USART0_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART0_Uninitialize(void)
{
    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART0_PowerControl(ARM_POWER_STATE state)
{
    return ARM_USARTx_PowerControl(&USART0_DEV, state);
}

static int32_t ARM_USART0_Send(const void *data, uint32_t num)
{
    while (num--) {
        uartOutputBuf[uartOutputBufIndex++] = *(char *)data++;
        uartOutputBufCount++;
        if (uartOutputBufIndex == sizeof(uartOutputBuf)) {
            uartOutputBufIndex = 0;
        }
    }
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART0_Receive(void *data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART0_DEV, data, num);
}

static int32_t ARM_USART0_Transfer(const void *data_out, void *data_in,
                                   uint32_t num)
{
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART0_GetTxCount(void)
{
    return ARM_USARTx_GetTxCount(&USART0_DEV);
}

static uint32_t ARM_USART0_GetRxCount(void)
{
    return ARM_USARTx_GetRxCount(&USART0_DEV);
}
static int32_t ARM_USART0_Control(uint32_t control, uint32_t arg)
{
    return ARM_DRIVER_OK;
}

static ARM_USART_STATUS ARM_USART0_GetStatus(void)
{
    ARM_USART_STATUS status = {0, 0, 0, 0, 0, 0, 0, 0};
    return status;
}

static int32_t ARM_USART0_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    ARG_UNUSED(control);
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART0_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem_status = {0, 0, 0, 0, 0};
    return modem_status;
}

extern ARM_DRIVER_USART Driver_USART0;
ARM_DRIVER_USART Driver_USART0 = {
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    ARM_USART0_Initialize,
    ARM_USART0_Uninitialize,
    ARM_USART0_PowerControl,
    ARM_USART0_Send,
    ARM_USART0_Receive,
    ARM_USART0_Transfer,
    ARM_USART0_GetTxCount,
    ARM_USART0_GetRxCount,
    ARM_USART0_Control,
    ARM_USART0_GetStatus,
    ARM_USART0_SetModemControl,
    ARM_USART0_GetModemStatus
};
#endif /* RTE_USART0 */
