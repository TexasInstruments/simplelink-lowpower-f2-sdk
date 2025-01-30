/*
 * Copyright (c) 2019 Arm Limited. All rights reserved.
 * Copyright (c) 2020 Nordic Semiconductor ASA. All rights reserved.
 * Copyright (c) 2021 Laird Connectivity. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __RTE_DEVICE_H
#define __RTE_DEVICE_H

#include <nrf-pinctrl.h>

#define RTE_USART0 1

#define RTE_USART0_PINS            \
{                                  \
        NRF_PSEL(UART_TX,  0, 20), \
        NRF_PSEL(UART_RX,  0, 22), \
        NRF_PSEL(UART_RTS, 0, 19), \
        NRF_PSEL(UART_CTS, 0, 21), \
}


#define RTE_USART1 1

#define RTE_USART1_PINS            \
{                                  \
        NRF_PSEL(UART_TX,  1,  8), \
        NRF_PSEL(UART_RX,  1, 10), \
        NRF_PSEL(UART_RTS, 1,  7), \
        NRF_PSEL(UART_CTS, 1,  9), \
}

// <e> TWIM (Two-wire interface master) [Driver_TWIM2]
// <i> Configuration settings for Driver_TWIM2 in component ::Drivers:TWIM
#define RTE_TWIM2                      1
//   <h> Pin Selection (0xFFFFFFFF means Disconnected)
//     <o> SCL
#define   RTE_TWIM2_SCL_PIN            35
//     <o> SDA
#define   RTE_TWIM2_SDA_PIN            34
//   </h> Pin Configuration
// </e> TWIM (Two-wire interface master) [Driver_TWIM2]

// <e> QSPI (Quad serial peripheral interface) [Driver_QSPI0]
// <i> Configuration settings for Driver_QSPI0 in component ::Drivers:QSPI
#define RTE_QSPI0                      1
//   <h> Pin Selection (0xFFFFFFFF means Disconnected)
//     <o> IO0
#define   RTE_QSPI0_IO0_PIN            13
//     <o> IO1
#define   RTE_QSPI0_IO1_PIN            14
//     <o> IO2
#define   RTE_QSPI0_IO2_PIN            15
//     <o> IO3
#define   RTE_QSPI0_IO3_PIN            16
//     <o> SCL
#define   RTE_QSPI0_SCL_PIN            17
//     <o> CSN
#define   RTE_QSPI0_CSN_PIN            18
//   </h> Pin Configuration
// </e> QSPI (Quad serial peripheral interface) [Driver_QSPI0]

// <e> FLASH (Flash Memory) [Driver_FLASH0]
// <i> Configuration settings for Driver_FLASH0 in component ::Drivers:FLASH
#define   RTE_FLASH0                    1
// </e> FLASH (Flash Memory) [Driver_FLASH0]

#endif  /* __RTE_DEVICE_H */
