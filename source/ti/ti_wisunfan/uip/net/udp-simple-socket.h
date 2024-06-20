/*
 * Copyright (c) 2012-2014, Thingsquare, http://www.thingsquare.com/.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/******************************************************************************
 *
 * Copyright (c) 2015 Texas Instruments Inc.  All rights reserved.
 *
 * DESCRIPTION:
 *
 * HISTORY:
 *
 *
 ******************************************************************************/

#ifndef UDP_SIMPLE_SOCKET_H
#define UDP_SIMPLE_SOCKET_H

#include "net/uip.h"

struct udp_simple_socket;

void udp_simple_socket_init();

/**
 * \brief      A UDP socket callback function
 * \param c    A pointer to the struct udp_simple_socket that received the data
 * \param ptr  An opaque pointer that was specified when the UDP socket was opened
 * \param source_addr The IP address from which the datagram was sent
 * \param source_port The UDP port number, in host byte order, from which the datagram was sent
 * \param dest_addr The IP address that this datagram was sent to
 * \param dest_port The UDP port number, in host byte order, that the datagram was sent to
 * \param data A pointer to the data contents of the UDP datagram
 * \param datalen The length of the data being pointed to by the data pointer
 *
 *             Each UDP socket has a callback function that gets
 *             called every time a UDP packet is received.
 */
typedef void (*udp_simple_socket_input_callback_t)(
        struct udp_simple_socket *c, void *ptr, const uip_ipaddr_t *source_addr,
        uint16_t source_port, const uip_ipaddr_t *dest_addr, uint16_t dest_port,
        const char *data, uint16_t datalen);

struct udp_simple_socket
{
    udp_simple_socket_input_callback_t input_callback;
    void *ptr;
    struct uip_udp_conn *udp_conn;

};

/**
 * \brief      Open a UDP socket connection
 * \param c    A pointer to the struct udp_simple_socket that should be connected
 * \param remote_addr The IP address of the remote host, or NULL if the UDP socket should only be connected to a specific port
 * \param remote_port The UDP port number, in host byte order, to which the UDP socket should be connected
 * \param local_port The UDP port number, in host byte order, to bind the UDP socket to
 * \param ptr  An opaque pointer that will be passed to callbacks
 * \param receive_callback A function pointer to the callback function that will be called when data arrives
 * \retval -1  Opening the UDP socket failed
 * \retval 1   Opening the UDP socket succeeded
 *
 *             This function connects the UDP socket to a specific
 *             remote port and optional remote IP address. When a UDP
 *             socket is connected to a remote port and address, it
 *             will only receive packets that are sent from the remote
 *             port and address. When sending data over a connected
 *             UDP socket, the data will be sent to the connected
 *             remote address.
 *
 *             A UDP socket can be connected to a remote port, but not
 *             a remote IP address, by providing a NULL parameter as
 *             the remote_addr parameter. This lets the UDP socket
 *             receive data from any IP address on the specified port. 
 *             The same happens if we specify the remote port 0.
 *
 *             This function also binds the UDP socket to a local port so
 *             that it will begin to receive data that arrives on the
 *             specified port. A UDP socket will receive data
 *             addressed to the specified port number on any IP
 *             address of the host.
 *
 *             A UDP socket that is bound to a local port will use
 *             this port number as a source port in outgoing UDP
 *             messages. If the local_port value UIP_UDP_LISTEN_ALL_PORT (0xFFFF)
 *             is used, all ports are listened by this socket.
 *
 *             The receive_callback will be called when data arrives for the
 *             socket and the ptr data will be available.
 *
 */
int udp_simple_socket_open(struct udp_simple_socket *c,
                           uip_ipaddr_t *remote_addr, uint16_t remote_port,
                           uint16_t local_port, void *ptr,
                           udp_simple_socket_input_callback_t receive_callback);

/**
 * \brief      Send data on a UDP socket
 * \param c    A pointer to the struct udp_simple_socket on which the data should be sent
 * \param data A pointer to the data that should be sent
 * \param datalen The length of the data to be sent
 * \return     The number of bytes sent, or -1 if an error occurred
 *
 *             This function sends data over a UDP socket. The UDP
 *             socket must have been connected to a remote address and
 *             port with udp_simple_socket_open().
 *
 */
int udp_simple_socket_send(struct udp_simple_socket *c, const void *data,
                           uint16_t datalen);

/**
 * \brief      Send data on a UDP socket to a specific address and port
 * \param c    A pointer to the struct udp_simple_socket on which the data should be sent
 * \param data A pointer to the data that should be sent
 * \param datalen The length of the data to be sent
 * \param addr The IP address to which the data should be sent
 * \param port The UDP port number, in host byte order, to which the data should be sent
 * \return     The number of bytes sent, or -1 if an error occurred
 *
 *             This function sends data over a UDP socket to a
 *             specific address and port.
 *
 */
int udp_simple_socket_sendto(struct udp_simple_socket *c, const void *data,
                             uint16_t datalen, const uip_ipaddr_t *addr,
                             uint16_t port);

/**
 * \brief      Close a UDP socket
 * \param c    A pointer to the struct udp_simple_socket to be closed
 * \retval -1  If closing the UDP socket failed
 * \retval 1   If closing the UDP socket succeeded
 *
 *             This function closes a UDP socket that has previously
 *             been opened with udp_simple_socket_open(). 
 *
 */
int udp_simple_socket_close(struct udp_simple_socket *c);

#endif /* UDP_SOCKET_H */
