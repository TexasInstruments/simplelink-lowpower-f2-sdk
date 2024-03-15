/*
 *  Copyright (c) 2019, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes compile-time configurations for the NCP.
 *
 */

#ifndef CONFIG_NCP_H_
#define CONFIG_NCP_H_

/**
 * @def CONFIG_NCP_TX_BUFFER_SIZE
 *
 * The size of NCP message buffer in bytes.
 *
 */
#ifndef CONFIG_NCP_TX_BUFFER_SIZE
#define CONFIG_NCP_TX_BUFFER_SIZE 512
#endif

/**
 * @def CONFIG_NCP_UART_TX_CHUNK_SIZE
 *
 * The size of NCP UART TX chunk in bytes.
 *
 */
#ifndef CONFIG_NCP_UART_TX_CHUNK_SIZE
#define CONFIG_NCP_UART_TX_CHUNK_SIZE 128
#endif

/**
 * @def CONFIG_NCP_UART_RX_BUFFER_SIZE
 *
 * The size of NCP UART RX buffer in bytes.
 *
 */
#ifndef CONFIG_NCP_UART_RX_BUFFER_SIZE
#define CONFIG_NCP_UART_RX_BUFFER_SIZE 1300
#endif // CONFIG_NCP_UART_RX_BUFFER_SIZE

/**
 * @def CONFIG_NCP_SPINEL_ENCRYPTER_EXTRA_DATA_SIZE
 *
 * The size of extra data to be allocated in UART buffer,
 * needed by NCP Spinel Encrypter.
 *
 */
#ifndef CONFIG_NCP_SPINEL_ENCRYPTER_EXTRA_DATA_SIZE
#define CONFIG_NCP_SPINEL_ENCRYPTER_EXTRA_DATA_SIZE 0
#endif

/**
 * @def CONFIG_NCP_SPINEL_LOG_MAX_SIZE
 *
 * The maximum OpenThread log string size (number of chars) supported by NCP using Spinel `StreamWrite`.
 *
 */
#ifndef CONFIG_NCP_SPINEL_LOG_MAX_SIZE
#define CONFIG_NCP_SPINEL_LOG_MAX_SIZE 150
#endif

/**
 * @def CONFIG_NCP_ENABLE_PEEK_POKE
 *
 * Define as 1 to enable peek/poke functionality on NCP.
 *
 * Peek/Poke allows the host to read/write to memory addresses on NCP. This is intended for debugging.
 *
 */
#ifndef CONFIG_NCP_ENABLE_PEEK_POKE
#define CONFIG_NCP_ENABLE_PEEK_POKE 0
#endif

/**
 * @def CONFIG_NCP_SPINEL_RESPONSE_QUEUE_SIZE
 *
 * Size of NCP Spinel command response queue.
 *
 * NCP guarantees that it can respond up to `CONFIG_NCP_SPINEL_RESPONSE_QUEUE_SIZE` spinel commands at the
 * same time. The spinel protocol defines a Transaction ID (TID) as part of spinel command frame (the TID can be
 * a value 0-15 where TID 0 is used for frames which require no response). Spinel protocol can support at most support
 * 15 simultaneous commands.
 *
 * The host driver implementation may further limit the number of simultaneous Spinel command frames (e.g., wpantund
 * limits this to two). This configuration option can be used to reduce the response queue size.
 *
 */
#ifndef CONFIG_NCP_SPINEL_RESPONSE_QUEUE_SIZE
#define CONFIG_NCP_SPINEL_RESPONSE_QUEUE_SIZE 15
#endif

#endif // CONFIG_NCP_H_
