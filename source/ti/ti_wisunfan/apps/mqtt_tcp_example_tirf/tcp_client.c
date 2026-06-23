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
 *  ======== tcp_client.c ========
 *  TCP Application Implementation for Wi-SUN
 *
 *  The tcp_object_info_t structure is kept entirely internal.
 *  Callers receive a socket index (int32_t) from tcp_client_connect()
 *  and pass it to all subsequent operations.
 *
 *  Hooks are registered at connect time; TCP fires them without
 *  knowing which upper-layer protocol registered them.
 */

#include "tcp_client.h"

#include <string.h>

/* Nanostack Headers */
#include "socket_api.h"
#include "net_interface.h"
#include "ip6string.h"
#include "ns_trace.h"
#include "mbed_config_app.h"

#define TRACE_GROUP "tcpa"

/******************************************************************************
 Internal types
 *****************************************************************************/

typedef struct
{
    uint16_t          port;
    bool              isAllocated;
    rx_data_cb_t      rx_data_cb;      /* registered at connect time */
    socket_event_cb_t socket_event_cb; /* registered at connect time */
    int8_t            tcp_socket_id;
    bool              tcp_connected;
    ns_address_t      tcp_peer_addr;
    uint8_t           tcp_recv_buffer[TCP_RECV_BUF_SIZE];
    uint8_t           tcp_send_buffer[TCP_SEND_BUF_SIZE];
} socket_info_t;

typedef struct
{
    socket_info_t socketInfo[MAX_TCP_SOCKETS];
} tcp_object_info_t;

/******************************************************************************
 Internal state
 Explicitly pre-initialised so every field starts in a known safe state.
 *****************************************************************************/

static tcp_object_info_t g_tcp_object = {
    .socketInfo = {
        {
            .port            = 0,
            .isAllocated     = false,
            .rx_data_cb      = NULL,
            .socket_event_cb = NULL,
            .tcp_socket_id   = NOT_INITIALIZED,
            .tcp_connected   = false,
        }
    }
};

/* ---- Forward declarations for all static (internal) functions ---- */
static int32_t tcp_allocate_slot(void);
static void    tcp_deallocate_slot(int32_t ind);
static int32_t tcp_find_slot_by_socket_id(int8_t socket_id);
static void    tcp_socket_callback(void *cb);
static bool    tcp_app_is_connected(int32_t ind);
static bool    tcp_app_init(void);

/******************************************************************************
 Internal helpers
 *****************************************************************************/

/*!
 * Allocate a free socket slot and initialise it.
 * @return slot index (>=0) or -1 if pool is full
 */
static int32_t tcp_allocate_slot(void)
{
    int32_t i;
    for (i = 0; i < MAX_TCP_SOCKETS; i++)
    {
        if (!g_tcp_object.socketInfo[i].isAllocated)
        {
            socket_info_t *s;
            s = &g_tcp_object.socketInfo[i];
            s->isAllocated     = true;
            s->tcp_socket_id   = NOT_INITIALIZED;
            s->tcp_connected   = false;
            s->rx_data_cb      = NULL;
            s->socket_event_cb = NULL;
            return i;
        }
    }
    return -1;
}

/*!
 * Close the socket (if open) and return the slot to the free pool.
 * Clears all fields so the slot is ready for re-use.
 * @param ind  Slot index to deallocate
 */
static void tcp_deallocate_slot(int32_t ind)
{
    if (ind < 0 || ind >= MAX_TCP_SOCKETS)
        return;

    socket_info_t *s;
    s = &g_tcp_object.socketInfo[ind];

    if (s->tcp_socket_id >= 0)
    {
        socket_close(s->tcp_socket_id);
        s->tcp_socket_id = NOT_INITIALIZED;
    }

    s->tcp_connected   = false;
    s->rx_data_cb      = NULL;
    s->socket_event_cb = NULL;
    s->port            = 0;
    s->isAllocated     = false;
}

/*!
 * Find the socket slot that owns the given nanostack socket fd.
 * socket_callback_t.interface_id is the *network* interface — it cannot be
 * used as a slot index. Match on socket_id instead.
 * @param  socket_id  Nanostack socket fd from socket_callback_t
 * @return Slot index (>=0) if found, -1 if not allocated
 */
static int32_t tcp_find_slot_by_socket_id(int8_t socket_id)
{
    int32_t i;
    for (i = 0; i < MAX_TCP_SOCKETS; i++)
    {
        if (g_tcp_object.socketInfo[i].isAllocated &&
            g_tcp_object.socketInfo[i].tcp_socket_id == socket_id)
        {
            return i;
        }
    }
    return -1;
}

/******************************************************************************
 Socket callback — dispatches events to the registered hooks
 *****************************************************************************/

static void tcp_socket_callback(void *cb)
{
    socket_callback_t *sock_cb = (socket_callback_t *)cb;
    int16_t       len;
    socket_info_t *s;
    int32_t        ind;

    /* Look up by socket fd — interface_id is the *network* interface ID,
     * not our slot index, so it cannot be used directly. */
    ind = tcp_find_slot_by_socket_id(sock_cb->socket_id);
    if (ind < 0)
    {
        tr_warn("tcp_socket_callback: unknown socket %d", sock_cb->socket_id);
        return;
    }

    s = &g_tcp_object.socketInfo[ind];

    switch (sock_cb->event_type & SOCKET_EVENT_MASK)
    {
    case SOCKET_DATA:
    {
        tr_info("SOCKET_DATA sock=%d bytes=%d", sock_cb->socket_id, sock_cb->d_len);
        len = socket_recv(sock_cb->socket_id, s->tcp_recv_buffer, TCP_RECV_BUF_SIZE, 0);
        if (len > 0)
        {
            if (s->rx_data_cb != NULL){
                s->rx_data_cb(s->tcp_recv_buffer,(uint16_t)len);
            }
        }
        else if (len == 0)
        {
            tr_info("TCP: connection closed by peer (ind=%d)", ind);
            if (s->socket_event_cb != NULL){
                s->socket_event_cb(TCP_EVENT_DISCONNECTED);
            }
            tcp_deallocate_slot(ind);
        }
        break;
    }
    case SOCKET_CONNECT_DONE:
    {
        tr_info("SOCKET_CONNECT_DONE ind=%d", ind);
        s->tcp_connected = true;
        if (s->socket_event_cb != NULL){
            s->socket_event_cb(TCP_EVENT_CONNECTED);
        }
        break;
    }
    case SOCKET_CONNECT_FAIL:
    case SOCKET_CONNECT_CLOSED:
    case SOCKET_CONNECTION_RESET:
    {
        tr_info("event=%d", sock_cb->event_type);
        if (s->socket_event_cb != NULL){
            s->socket_event_cb(TCP_EVENT_DISCONNECTED);
        }
        tcp_deallocate_slot(ind);
        break;
    }
    case SOCKET_INCOMING_CONNECTION:
    case SOCKET_TX_FAIL:
    case SOCKET_NO_ROUTE:
    case SOCKET_TX_DONE:
    case SOCKET_NO_RAM:
    case SOCKET_CONNECTION_PROBLEM:
    default:
    {
        tr_warn("tcp_socket_callback: unhandled event 0x%x ind=%d",sock_cb->event_type, ind);
        break;
    }
    }
}

/******************************************************************************
 Public API
 *****************************************************************************/

int32_t tcp_client_connect(const char *ip_addr, uint16_t port, rx_data_cb_t rx_data_cb, socket_event_cb_t socket_event_cb)
{
    int8_t        ret;
    ns_address_t  connect_addr;
    socket_info_t *s;

    int32_t ind = tcp_allocate_slot();
    if (ind < 0)
    {
        tr_error("TCP: no free socket slots");
        return -1;
    }

    s = &g_tcp_object.socketInfo[ind];

    /* Store connection parameters and hooks before opening the socket */
    s->port            = port;
    s->rx_data_cb      = rx_data_cb;
    s->socket_event_cb = socket_event_cb;

    /* Close stale socket if present */
    if (s->tcp_socket_id >= 0)
    {
        socket_close(s->tcp_socket_id);
        s->tcp_socket_id = NOT_INITIALIZED;
    }

    tr_info("TCP CLIENT CONNECT START ind=%d addr=%s port=%d", ind, ip_addr, port);

    /* socket_open uses ind as the interface_id so the callback can route events */
    s->tcp_socket_id = socket_open(SOCKET_TCP, (uint16_t)ind, tcp_socket_callback);
    if (s->tcp_socket_id < 0)
    {
        tr_error("TCP socket open failed: %d", s->tcp_socket_id);
        tcp_deallocate_slot(ind);
        return -1;
    }
    tr_info("TCP socket opened socket_id=%d", s->tcp_socket_id);

    /* Parse and store peer address */
    connect_addr.type       = ADDRESS_IPV6;
    connect_addr.identifier = port;
    if (!stoip6(ip_addr, strlen(ip_addr), connect_addr.address))
    {
        tr_error("Invalid IPv6 address: %s", ip_addr);
        tcp_deallocate_slot(ind);
        return -1;
    }
    memcpy(s->tcp_peer_addr.address, connect_addr.address, 16);

    tr_info("TCP: Connecting to %s port %d", ip_addr, port);
    ret = socket_connect(s->tcp_socket_id, &connect_addr, 0);
    /* -4 = already connected / in progress */
    if (ret < 0 && ret != -4)   
    {
        tr_error("TCP socket connect failed: %d", ret);
        tcp_deallocate_slot(ind);
        return -1;
    }
    /* caller stores this to identify the connection */
    return ind;  
}

bool tcp_send_data(int32_t ind, const uint8_t *data, uint16_t len)
{
    int16_t ret;

    if (ind < 0 || ind >= MAX_TCP_SOCKETS)
    {
        tr_warn("TCP: invalid socket index %d", ind);
        return false;
    }
    if (data == NULL || len == 0)
    {
        tr_warn("TCP: invalid data or length");
        return false;
    }
    if (!g_tcp_object.socketInfo[ind].tcp_connected)
    {
        tr_warn("TCP: not connected (ind=%d)", ind);
        return false;
    }
    if (g_tcp_object.socketInfo[ind].tcp_socket_id < 0)
    {
        tr_warn("TCP: no socket (ind=%d)", ind);
        return false;
    }

    tr_debug("TCP: Sending %d bytes on socket %d", len, g_tcp_object.socketInfo[ind].tcp_socket_id);
    ret = socket_send(g_tcp_object.socketInfo[ind].tcp_socket_id, data, len);
    if (ret < 0)
    {
        tr_error("TCP: socket_send failed: %d", ret);
        return false;
    }
    tr_debug("TCP: Sent %d bytes", ret);
    return true;
}

void tcp_disconnect(int32_t ind)
{
    if (ind < 0 || ind >= MAX_TCP_SOCKETS)
        return;
    tr_info("TCP: Disconnecting ind=%d", ind);
    tcp_deallocate_slot(ind);
}

static bool tcp_app_is_connected(int32_t ind)
{
    if (ind < 0 || ind >= MAX_TCP_SOCKETS)
        return false;
    return g_tcp_object.socketInfo[ind].tcp_connected;
}
