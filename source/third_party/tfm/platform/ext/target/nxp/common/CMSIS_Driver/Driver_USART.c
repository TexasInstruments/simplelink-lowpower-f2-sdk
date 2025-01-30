/*
 * Copyright (c) 2013-2021 Arm Limited. All rights reserved.
 * Copyright 2019-2022 NXP. All rights reserved.
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

#include "fsl_common.h"
#include "fsl_usart.h"
#include "fsl_clock.h"

#include "Driver_USART.h"
#include "board.h"
#include "platform_base_address.h"

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  (void)arg
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

typedef struct {
    USART_Type      *base;          /* USART base */
    usart_config_t  config;         /* USART configuration structure */
    uint32_t        tx_nbr_bytes;   /* Number of bytes transfered */
    uint32_t        rx_nbr_bytes;   /* Number of bytes recevied */
} UARTx_Resources;

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

static int32_t ARM_USARTx_Initialize(UARTx_Resources* uart_dev)
{
    
#if (__ARM_FEATURE_CMSE & 0x2) /* Initialize once in S */
    uint32_t usartClkFreq;

    usartClkFreq = BOARD_DEBUG_UART_CLK_FREQ;

    USART_Init(uart_dev->base, &uart_dev->config, usartClkFreq);
#endif

    return ARM_DRIVER_OK;
}

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
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }
}

static int32_t ARM_USARTx_Deinitialize(UARTx_Resources* uart_dev)
{
    USART_Deinit(uart_dev->base);

    return ARM_DRIVER_OK;
}

static int32_t ARM_USARTx_Send(UARTx_Resources* uart_dev, const uint8_t *data, size_t length)
{
    USART_WriteBlocking(uart_dev->base, data, length);
    
    uart_dev->tx_nbr_bytes = length;
    
    return ARM_DRIVER_OK;
}

static int32_t ARM_USARTx_Receive(UARTx_Resources* uart_dev, uint8_t *data, size_t length)
{
    status_t status;

    status = USART_ReadBlocking(uart_dev->base, data, length);

    if (status == kStatus_Success)
    {
        uart_dev->rx_nbr_bytes = length;
        return ARM_DRIVER_OK;
    }
    else
    {
        return ARM_DRIVER_ERROR;
    }
}

static uint32_t ARM_USARTx_GetTxCount(UARTx_Resources* uart_dev)
{
    return uart_dev->tx_nbr_bytes;
}

static uint32_t ARM_USARTx_GetRxCount(UARTx_Resources* uart_dev)
{
    return uart_dev->rx_nbr_bytes;
}


/* USART_BASE Driver wrapper functions */
static UARTx_Resources USART_DEV = {
    .base = USART_BASE,
};
static int32_t ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);

    /*
    *   usartConfig->baudRate_Bps = 115200U;
    *   usartConfig->parityMode = kUSART_ParityDisabled;
    *   usartConfig->stopBitCount = kUSART_OneStopBit;
    *   usartConfig->bitCountPerChar = kUSART_8BitsPerChar;
    *   usartConfig->loopback = false;
    *   usartConfig->enableTx = false;
    *   usartConfig->enableRx = false;
    */
    USART_GetDefaultConfig(&USART_DEV.config);
    USART_DEV.config.enableRx = true;
    USART_DEV.config.enableTx = true;

    return ARM_USARTx_Initialize(&USART_DEV);
}

static int32_t ARM_USART_Uninitialize(void)
{
    return ARM_USARTx_Deinitialize(&USART_DEV);
}

static int32_t ARM_USART_PowerControl(ARM_POWER_STATE state)
{
    return ARM_USARTx_PowerControl(&USART_DEV, state);
}

static int32_t ARM_USART_Send(const void *data, uint32_t num)
{
    return ARM_USARTx_Send(&USART_DEV, data, num);
}

static int32_t ARM_USART_Receive(void *data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART_DEV, data, num);
}

static int32_t ARM_USART_Transfer(const void *data_out, void *data_in,
                                   uint32_t num)
{
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART_GetTxCount(void)
{
    return ARM_USARTx_GetTxCount(&USART_DEV);
}

static uint32_t ARM_USART_GetRxCount(void)
{
    return ARM_USARTx_GetRxCount(&USART_DEV);
}

static int32_t ARM_USART_Control(uint32_t control, uint32_t arg)
{
    return ARM_DRIVER_OK; //ARM_USARTx_Control(&USART_DEV, control, arg);
}

static ARM_USART_STATUS ARM_USART_GetStatus(void)
{
    ARM_USART_STATUS status = {0, 0, 0, 0, 0, 0, 0, 0};
    return status;
}

static int32_t ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    ARG_UNUSED(control);
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem_status = {0, 0, 0, 0, 0};
    return modem_status;
}

extern ARM_DRIVER_USART Driver_USART;
ARM_DRIVER_USART Driver_USART = {
    .GetVersion = ARM_USART_GetVersion,
    .GetCapabilities = ARM_USART_GetCapabilities,
    .Initialize = ARM_USART_Initialize,
    .Uninitialize = ARM_USART_Uninitialize,
    .PowerControl = ARM_USART_PowerControl,
    .Send = ARM_USART_Send,
    .Receive = ARM_USART_Receive,
    .Transfer = ARM_USART_Transfer,
    .GetTxCount = ARM_USART_GetTxCount,
    .GetRxCount = ARM_USART_GetRxCount,
    .Control = ARM_USART_Control,
    .GetStatus = ARM_USART_GetStatus,
    .SetModemControl = ARM_USART_SetModemControl,
    .GetModemStatus = ARM_USART_GetModemStatus
};
