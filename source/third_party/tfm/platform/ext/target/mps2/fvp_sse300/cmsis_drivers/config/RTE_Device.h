/*
 * Copyright (c) 2019 Arm Limited. All rights reserved.
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

// <e> USART (Universal synchronous - asynchronous receiver transmitter) [Driver_USART0]
// <i> Configuration settings for Driver_USART0 in component ::Drivers:USART
#define   RTE_USART0                     1
// </e> USART (Universal synchronous - asynchronous receiver transmitter) [Driver_USART0]

// <e> USART (Universal synchronous - asynchronous receiver transmitter) [Driver_USART1]
// <i> Configuration settings for Driver_USART1 in component ::Drivers:USART
#define   RTE_USART1                     1
// </e> USART (Universal synchronous - asynchronous receiver transmitter) [Driver_USART1]

// <e> MPC (Memory Protection Controller) [Driver_VM0_MPC]
// <i> Configuration settings for Driver_VM0_MPC in component ::Drivers:MPC
#define   RTE_VM0_MPC                 1
// </e> MPC (Memory Protection Controller) [Driver_VM0_MPC]

// <e> MPC (Memory Protection Controller) [Driver_VM1_MPC]
// <i> Configuration settings for Driver_VM1_MPC in component ::Drivers:MPC
#define   RTE_VM1_MPC                 1
// </e> MPC (Memory Protection Controller) [Driver_VM1_MPC]

// <e> MPC (Memory Protection Controller) [Driver_SSRAM2_MPC]
// <i> Configuration settings for Driver_SSRAM2_MPC in component ::Drivers:MPC
#define   RTE_SSRAM2_MPC                 1
// </e> MPC (Memory Protection Controller) [Driver_SSRAM2_MPC]

// <e> MPC (Memory Protection Controller) [Driver_SSRAM3_MPC]
// <i> Configuration settings for Driver_SSRAM3_MPC in component ::Drivers:MPC
#define   RTE_SSRAM3_MPC                 1
// </e> MPC (Memory Protection Controller) [Driver_SSRAM3_MPC]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_MAIN0]
// <i> Configuration settings for Driver_PPC_SSE300_MAIN0 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_MAIN0             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_MAIN0]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_MAIN_EXP0]
// <i> Configuration settings for Driver_PPC_SSE300_MAIN_EXP0 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_MAIN_EXP0             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_MAIN_EXP0]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_MAIN_EXP1]
// <i> Configuration settings for Driver_PPC_SSE300_MAIN_EXP1 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_MAIN_EXP1             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_MAIN_EXP1]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_MAIN_EXP2]
// <i> Configuration settings for Driver_PPC_SSE300_MAIN_EXP2 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_MAIN_EXP2             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_MAIN_EXP2]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_MAIN_EXP3]
// <i> Configuration settings for Driver_PPC_SSE300_MAIN_EXP3 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_MAIN_EXP3             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_MAIN_EXP3]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_PERIPH0]
// <i> Configuration settings for Driver_PPC_SSE300_PERIPH0 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_PERIPH0             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_PERIPH0]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_PERIPH1]
// <i> Configuration settings for Driver_PPC_SSE300_PERIPH1 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_PERIPH1             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_PERIPH1]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_PERIPH_EXP0]
// <i> Configuration settings for Driver_PPC_SSE300_PERIPH_EXP0 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_PERIPH_EXP0             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_PERIPH_EXP0]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_PERIPH_EXP1]
// <i> Configuration settings for Driver_PPC_SSE300_PERIPH_EXP1 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_PERIPH_EXP1             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_PERIPH_EXP1]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_PERIPH_EXP2]
// <i> Configuration settings for Driver_PPC_SSE300_PERIPH_EXP2 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_PERIPH_EXP2             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_PERIPH_EXP2]

// <e> PPC (Peripheral Protection Controller) [PPC_SSE300_PERIPH_EXP3]
// <i> Configuration settings for Driver_PPC_SSE300_PERIPH_EXP3 in component ::Drivers:PPC
#define   RTE_PPC_SSE300_PERIPH_EXP3             1
// </e> PPC (Peripheral Protection Controller) [Driver_PPC_SSE300_PERIPH_EXP3]

// <e> Flash device emulated by BRAM [Driver_Flash0]
// <i> Configuration settings for Driver_Flash0 in component ::Drivers:Flash
#define   RTE_FLASH0                     1
// </e> Flash device emulated by BRAM [Driver_Flash0]

#endif  /* __RTE_DEVICE_H */
