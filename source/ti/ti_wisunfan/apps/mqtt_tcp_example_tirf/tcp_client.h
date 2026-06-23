/*
 * Copyright (c) 2015-2026, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== tcp_client.h ========
 *  TCP Application API for Wi-SUN
 *
 *  Internal state (tcp_object_info_t) is hidden inside tcp_client.c.
 *  Callers identify their connection by the socket index returned
 *  from tcp_client_connect().
 */

#ifndef TCP_APP_H_
#define TCP_APP_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 Defines
 *****************************************************************************/
#define TCP_SEND_BUF_SIZE  (2048)
#define TCP_PORT           (5678)
#define TCP_RECV_BUF_SIZE  (2048)
#define TCP_BACKLOG        (1)
#define NOT_INITIALIZED    (-1)
#define MAX_TCP_SOCKETS    (1)

/******************************************************************************
 Type Definitions
 *****************************************************************************/

/* Event values passed to socket_event_cb_t */
#define TCP_EVENT_CONNECTED     (0)
#define TCP_EVENT_DISCONNECTED  (1)

/* Callbacks registered at connect time. TCP layer fires them without
 * knowing which upper-layer protocol registered them. */
typedef void (*rx_data_cb_t)(const uint8_t *data, uint16_t len);
typedef void (*socket_event_cb_t)(int event);

/******************************************************************************
 Public API
 *****************************************************************************/

/*!
 * @brief  Connect TCP client to a remote peer and register callbacks.
 *         Internal socket state is hidden — callers use the returned index.
 * @param  ip_addr    Broker/peer IPv6 address string
 * @param  port       Destination port
 * @param  rx_data_cb    Called with received bytes (NULL if unused)
 * @param  socket_event_cb Called on connect(0) or disconnect(1) (NULL if unused)
 * @return Socket index (>=0) on success, -1 on failure
 */
int32_t tcp_client_connect(const char *ip_addr, uint16_t port, rx_data_cb_t rx_data_cb, socket_event_cb_t socket_event_cb);

/*!
 * @brief  Send data on the socket identified by ind.
 * @param  ind   Socket index from tcp_client_connect
 * @param  data  Data buffer
 * @param  len   Number of bytes to send
 * @return true if sent successfully, false otherwise
 */
bool tcp_send_data(int32_t ind, const uint8_t *data, uint16_t len);

/*!
 * @brief  Disconnect and free the socket slot identified by ind.
 *         Hooks are cleared internally.
 * @param  ind  Socket index from tcp_client_connect
 */
void tcp_disconnect(int32_t ind);


#ifdef __cplusplus
}
#endif

#endif /* TCP_APP_H_ */
