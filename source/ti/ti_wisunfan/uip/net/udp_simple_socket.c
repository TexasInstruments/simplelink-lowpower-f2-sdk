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

#include "net/udp-simple-socket.h"
#include "net/uip-udp-packet.h"

#include <string.h>

#define UIP_IP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

process_obj_t udp_simple_socket_process_obj;

void udp_simple_socket_process_post_func(process_event_t event,
                                         process_data_t data)
{
    struct udp_simple_socket *c = (struct udp_simple_socket *) data;
    if (uip_newdata())
    {
        if (c != NULL)
        {
            if (c->input_callback != NULL)
            {
                c->input_callback(c, c->ptr, &(UIP_IP_BUF->srcipaddr),
                                  UIP_HTONS(UIP_IP_BUF->srcport),
                                  &(UIP_IP_BUF->destipaddr),
                                  UIP_HTONS(UIP_IP_BUF->destport), uip_appdata,
                                  uip_datalen());
            }
        }
    }
}

void udp_simple_socket_init()
{
    process_obj_init(&udp_simple_socket_process_obj,
                     udp_simple_socket_process_post_func);
}

/*---------------------------------------------------------------------------*/
int udp_simple_socket_close(struct udp_simple_socket *c)
{
    if (c == NULL)
    {
        return -1;
    }
    if (c->udp_conn != NULL)
    {
        uip_udp_remove(c->udp_conn);
        return 1;
    }
    return -1;
}

/*---------------------------------------------------------------------------*/
int udp_simple_socket_open(struct udp_simple_socket *c,
                           uip_ipaddr_t *remote_addr, uint16_t remote_port,
                           uint16_t local_port, void *ptr,
                           udp_simple_socket_input_callback_t receive_callback)
{
    if (c == NULL)
    {
        return -1;
    }
    c->ptr = ptr;
    c->input_callback = receive_callback;
    c->udp_conn = udp_new(remote_addr, UIP_HTONS(remote_port), c,
                          &udp_simple_socket_process_obj);

    if (c->udp_conn == NULL)
    {
        return -1;
    }
    udp_bind(c->udp_conn, UIP_HTONS(local_port));

    return 1;
}
/*---------------------------------------------------------------------------*/
int udp_simple_socket_send(struct udp_simple_socket *c, const void *data,
                           uint16_t datalen)
{
    if (c == NULL || c->udp_conn == NULL)
    {
        return -1;
    }

    uip_udp_packet_send(c->udp_conn, data, datalen);
    return datalen;
}
/*---------------------------------------------------------------------------*/
int udp_simple_socket_sendto(struct udp_simple_socket *c, const void *data,
                             uint16_t datalen, const uip_ipaddr_t *to,
                             uint16_t port)
{
    if (c == NULL || c->udp_conn == NULL)
    {
        return -1;
    }

    if (c->udp_conn != NULL)
    {
        uip_udp_packet_sendto(c->udp_conn, data, datalen, to, UIP_HTONS(port));
        return datalen;
    }
    return -1;
}
/*---------------------------------------------------------------------------*/
/** @} */
