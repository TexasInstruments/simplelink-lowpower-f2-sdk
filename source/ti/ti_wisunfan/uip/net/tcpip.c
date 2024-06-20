/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Code for tunnelling uIP packets over the Rime mesh routing module
 *
 * \author  Adam Dunkels <adam@sics.se>\author
 * \author  Mathilde Durvy <mdurvy@cisco.com> (IPv6 related code)
 * \author  Julien Abeille <jabeille@cisco.com> (IPv6 related code)
 */
/******************************************************************************
 *
 * Copyright (c) 2014 Texas Instruments Inc.  All rights reserved.
 *
 * DESCRIPTION:
 *
 * HISTORY:
 *
 *
 ******************************************************************************/

#include "uip-conf.h"
#include "net/uip.h"
#include "net/uip-split.h"
#include "net/uip-packetqueue.h"
#include "net/udp-simple-socket.h"

#include "sys/etimer.h"

#if UIP_CONF_IPV6
#include "net/uip-nd6.h"
#include "net/uip-ds6.h"
#endif

#include <string.h>
#include "net/queuebuf.h"
#include "lib/random.h"

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include "uip_rpl_process.h"

#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf[UIP_LLIPH_LEN + uip_ext_len])
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_TCP_BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#if UIP_CONF_ICMP6
process_event_t tcpip_icmp6_event;
#endif /* UIP_CONF_ICMP6 */

process_obj_t tcpip_process_obj;

process_event_t tcpip_event;

/* Periodic check of active connections. */
//static struct etimer periodic;
struct etimer periodic;

#if UIP_CONF_IPV6 && UIP_CONF_IPV6_REASSEMBLY
/* Timer for reassembly. */
extern struct etimer uip_reass_timer;
#endif

#if UIP_TCP
/**
 * \internal Structure for holding a TCP port and a process ID.
 */
struct listenport
{
    uint16_t port;
    process_obj_t *p;
};

static struct internal_state
{
    struct listenport listenports[UIP_LISTENPORTS];
    process_obj_t *p;
}s;
#endif

/* Called on IP packet output. */
#if UIP_CONF_IPV6

static u16_t (*outputfunc)(uip_lladdr_t *a);

u16_t tcpip_output(uip_lladdr_t *a)
{
    int ret;
    if (outputfunc != NULL)
    {
        rimeaddr_copy((rimeaddr_t*) &uip_src_lladdr, (rimeaddr_t*)&uip_lladdr);
        if (a == NULL)
        {
            //Broadcast
            rimeaddr_copy((rimeaddr_t*)&uip_dst_lladdr, (rimeaddr_t*)&rimeaddr_null);
        }
        else
        {
            rimeaddr_copy((rimeaddr_t*)&uip_dst_lladdr, (rimeaddr_t*)a);
        }
        ret = outputfunc(a);
        return ret;
    }
    return 0;
}

void tcpip_set_outputfunc(u16_t (*f)(uip_lladdr_t *))
{
    outputfunc = f;
}
#else

static u16_t (* outputfunc)(void);
u16_t
tcpip_output(void)
{
    if(outputfunc != NULL)
    {
        return outputfunc();
    }
    return 0;
}

void
tcpip_set_outputfunc(u16_t (*f)(void))
{
    outputfunc = f;
}
#endif

#if UIP_CONF_IP_FORWARD
unsigned char tcpip_is_forwarding; /* Forwarding right now? */
#endif /* UIP_CONF_IP_FORWARD */

/*---------------------------------------------------------------------------*/

#if UIP_TCP
static void start_periodic_tcp_timer(void)
{
    if (etimer_expired(&periodic))
    {
        etimer_restart(&periodic);
    }
}
#endif
/*---------------------------------------------------------------------------*/
static void check_for_tcp_syn(void)
{
#if UIP_TCP || UIP_CONF_IP_FORWARD
    /* This is a hack that is needed to start the periodic TCP timer if
     an incoming packet contains a SYN: since uIP does not inform the
     application if a SYN arrives, we have no other way of starting
     this timer.  This function is called for every incoming IP packet
     to check for such SYNs. */
#define TCP_SYN 0x02
    if(UIP_IP_BUF->proto == UIP_PROTO_TCP &&
            (UIP_TCP_BUF->flags & TCP_SYN) == TCP_SYN)
    {
        start_periodic_tcp_timer();
    }
#endif /* UIP_TCP || UIP_CONF_IP_FORWARD */
}

/*---------------------------------------------------------------------------*/
static void packet_input(void)
{
    uipBuffer_GetInputPacket();
#if UIP_CONF_IP_FORWARD
    if(uip_len > 0)
    {
        tcpip_is_forwarding = 1;
        if(uip_fw_forward() == UIP_FW_LOCAL)
        {
            tcpip_is_forwarding = 0;
            check_for_tcp_syn();
            uip_input();
            if(uip_len > 0)
            {
#if UIP_CONF_TCP_SPLIT
                uip_split_output();
#else /* UIP_CONF_TCP_SPLIT */
#if UIP_CONF_IPV6
                tcpip_ipv6_output();
#else
                PRINTF("tcpip packet_input forward output len %d\n", uip_len);
                tcpip_output();
#endif
#endif /* UIP_CONF_TCP_SPLIT */
            }
        }
        tcpip_is_forwarding = 0;
    }
#else /* UIP_CONF_IP_FORWARD */
    if (uip_len> 0)
    {
        check_for_tcp_syn();

        uip_input();

        if (uip_len > 0)
        {
#if UIP_CONF_TCP_SPLIT
            uip_split_output();
#else /* UIP_CONF_TCP_SPLIT */
#if UIP_CONF_IPV6
            tcpip_ipv6_output();
#else
            PRINTF("tcpip packet_input output len %d\n", uip_len);
            tcpip_output();
#endif
#endif /* UIP_CONF_TCP_SPLIT */
        }
    }
#endif /* UIP_CONF_IP_FORWARD */

    uip_len= 0;
#if UIP_CONF_IPV6
    uip_ext_len = 0;
#endif /*UIP_CONF_IPV6*/
}
/*---------------------------------------------------------------------------*/
#if UIP_TCP
#if UIP_ACTIVE_OPEN
struct uip_conn *
tcp_connect(uip_ipaddr_t *ripaddr, uint16_t port, void *appstate, process_obj_t *process_obj)
{
    struct uip_conn *c;

    c = uip_connect(ripaddr, port);
    if(c == NULL)
    {
        return NULL;
    }

    c->appstate.p = process_obj;
    c->appstate.state = appstate;

    tcpip_poll_tcp(c);

    return c;
}
#endif /* UIP_ACTIVE_OPEN */
/*---------------------------------------------------------------------------*/
void
tcp_unlisten(uint16_t port, process_obj_t *process_obj)
{
    static unsigned char i;
    struct listenport *l;

    l = s.listenports;
    for(i = 0; i < UIP_LISTENPORTS; ++i)
    {
        if(l->port == port &&
                l->p == process_obj)
        {
            l->port = 0;
            uip_unlisten(port);
            break;
        }
        ++l;
    }
}
/*---------------------------------------------------------------------------*/
void
tcp_listen(uint16_t port, process_obj_t *process_obj)
{
    static unsigned char i;
    struct listenport *l;

    l = s.listenports;
    for(i = 0; i < UIP_LISTENPORTS; ++i)
    {
        if(l->port == 0)
        {
            l->port = port;
            l->p = process_obj;
            uip_listen(port);
            break;
        }
        ++l;
    }
}
/*---------------------------------------------------------------------------*/
void
tcp_attach(struct uip_conn *conn,
        void *appstate, process_obj_t *process_obj)
{
    uip_tcp_appstate_t *s;

    s = &conn->appstate;
    s->p = process_obj;
    s->state = appstate;
}

#endif /* UIP_TCP */
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void udp_attach(struct uip_udp_conn *conn, void *appstate,
                process_obj_t *process_obj)
{
    uip_udp_appstate_t *s;

    s = &conn->appstate;
    s->p = process_obj; //PROCESS_CURRENT();
    s->state = appstate;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_new(const uip_ipaddr_t *ripaddr, u16_t port, void *appstate,
        process_obj_t *process_obj)
{
    struct uip_udp_conn *c;
    uip_udp_appstate_t *s;

    c = uip_udp_new(ripaddr, port);
    if (c == NULL)
    {
        return NULL;
    }

    s = &c->appstate;
    s->p = process_obj; //PROCESS_CURRENT();
    s->state = appstate;

    return c;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_broadcast_new(u16_t port, void *appstate, process_obj_t *process_obj)
{
    uip_ipaddr_t addr;
    struct uip_udp_conn *conn;

#if UIP_CONF_IPV6
    uip_create_linklocal_allnodes_mcast(&addr);
#else
    uip_ipaddr(&addr, 255,255,255,255);
#endif /* UIP_CONF_IPV6 */

    conn = udp_new(&addr, port, appstate, process_obj);
    if (conn != NULL)
    {
        udp_bind(conn, port);
    }
    return conn;
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
#if UIP_CONF_ICMP6
uint8_t
icmp6_new(void *appstate)
{
    if(uip_icmp6_conns.appstate.p == PROCESS_NONE)
    {
        uip_icmp6_conns.appstate.p = PROCESS_CURRENT();
        uip_icmp6_conns.appstate.state = appstate;
        return 0;
    }
    return 1;
}

void
tcpip_icmp6_call(uint8_t type)
{
    if(uip_icmp6_conns.appstate.p != PROCESS_NONE)
    {
        /* XXX: This is a hack that needs to be updated. Passing a pointer (&type)
         like this only works with process_post_synch. */
        process_post_synch(uip_icmp6_conns.appstate.p, tcpip_icmp6_event, &type);
    }
    return;
}
#endif /* UIP_CONF_ICMP6 */
/*---------------------------------------------------------------------------*/
void eventhandler(process_event_t ev, process_data_t data)
{
#if UIP_TCP
    static unsigned char i;
    //register struct listenport *l;
#endif /*UIP_TCP*/

    switch (ev)
    {
    /* TODO: PROCESS_EVENT_EXITED
     We don't have this case, the task is killed or exit
     */

    case EVENT_TIMER:
        /* We get this event if one of our timers have expired. */
    {
        /* Check the clock so see if we should call the periodic uIP
         processing. */
        if (data == &periodic && etimer_expired(&periodic))
        {

#if UIP_TCP
            for(i = 0; i < UIP_CONNS; ++i)
            {
                if(uip_conn_active(i))
                {
                    /* Only restart the timer if there are active
                     connections. */
                    etimer_restart(&periodic);
                    uip_periodic(i);
#if UIP_CONF_IPV6
                    tcpip_ipv6_output();
#else
                    if(uip_len > 0)
                    {
                        LOG_INFO1("tcpip_output from periodic len %d\n", uip_len);
                        tcpip_output();
                        LOG_INFO1("tcpip_output after periodic len %d\n", uip_len);
                    }
#endif /* UIP_CONF_IPV6 */
                }
            }
#endif /* UIP_TCP */
#if UIP_CONF_IP_FORWARD
            uip_fw_periodic();
#endif /* UIP_CONF_IP_FORWARD */
        }

#if UIP_CONF_IPV6
#if UIP_CONF_IPV6_REASSEMBLY

        /*
         * check the timer for reassembly
         */
        if(data == &uip_reass_timer &&
                etimer_expired(&uip_reass_timer))
        {
            uip_reass_over();
            tcpip_ipv6_output();
        }

#endif /* UIP_CONF_IPV6_REASSEMBLY */
#if !UIP_CONF_ROUTER

        if(data == &uip_ds6_timer_rs &&
                etimer_expired(&uip_ds6_timer_rs))
        {
            uip_ds6_send_rs();
            tcpip_ipv6_output();
        }

#endif /* !UIP_CONF_ROUTER */

        if (data == &uip_ds6_timer_periodic
                && etimer_expired(&uip_ds6_timer_periodic))
        {
            uip_ds6_periodic();
            tcpip_ipv6_output();
        }

#endif /* UIP_CONF_IPV6 */

    }
        break;

#if UIP_TCP
        case TCP_POLL:
        if(data != NULL)
        {
            uip_poll_conn(data);
#if UIP_CONF_IPV6
            tcpip_ipv6_output();
#else /* UIP_CONF_IPV6 */
            if(uip_len > 0)
            {
                LOG_INFO1("tcpip_output from tcp poll len %d\n", uip_len);
                tcpip_output();
            }
#endif /* UIP_CONF_IPV6 */
            /* Start the periodic polling, if it isn't already active. */
            start_periodic_tcp_timer();
        }
        break;
#endif /* UIP_TCP */
#if UIP_UDP
    case UDP_POLL:
        if (data != NULL)
        {
            uip_udp_periodic_conn(data);
#if UIP_CONF_IPV6
            tcpip_ipv6_output();
#else
            if(uip_len > 0)
            {
                tcpip_output();
            }
#endif /* UIP_UDP */
        }
        break;
#endif /* UIP_UDP */

    case PACKET_INPUT:
        packet_input();
        break;
    };
}
/*---------------------------------------------------------------------------*/
void tcpip_input(process_data_t data)
{
    //tcpip_process(PACKET_INPUT, NULL);
    tcpip_process_obj.process_post(PACKET_INPUT, data);
}
/*---------------------------------------------------------------------------*/
#if UIP_CONF_IPV6
void tcpip_ipv6_output(void)
{
    uip_ds6_nbr_t *nbr = NULL;
    uip_ipaddr_t *nexthop = NULL;
    uip_ipaddr_t nexthop_addr;

    if (uip_len== 0)
    {
        return;
    }
    UIP_STAT_TX_IP_NUM_INC;

    if (uip_len> UIP_LINK_MTU)
    {
        LOG_INFO("tcpip_ipv6_output: Packet to big");
        UIP_STAT_TX_IP_DROP_TOO_BIG_INC;
        UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_PACKET_TOO_BIG);
        uip_len = 0;
        uip_ext_len = 0;
        return;
    }

    if (uip_is_addr_unspecified(&UIP_IP_BUF->destipaddr))
    {
        PRINTF("tcpip_ipv6_output: Destination address unspecified"); UIP_STAT_TX_IP_DROP_DEST_UNSPEC_INC; UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_DESTINATION_UNSPECIFIED);
        uip_len= 0;
        uip_ext_len = 0;
        return;
    }

    if (uip_ds6_is_my_addr(&UIP_IP_BUF->destipaddr))
    {
        PRINTF("tcpip_ipv6_output: Destination address is our own's"); UIP_STAT_TX_IP_DROP_DEST_MY_ADDR_INC; UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_DESTINATION_IS_OWN_ADDR);
        uip_len= 0;
        uip_ext_len = 0;
        return;
    }

    if (!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr))
    {
        /* Next hop determination */
        nbr = NULL;

        /* We first check if the destination address is on our immediate
         link. If so, we simply use the destination address as our
         nexthop address. */
        if (uip_ds6_is_addr_onlink(&UIP_IP_BUF->destipaddr)
                || (UIP_IP_BUF->proto == UIP_PROTO_ROUTING))
        {
            if (uip_ds6_is_addr_onlink(&UIP_IP_BUF->destipaddr))
            {
                UIP_STAT_TX_ROUTE_ONLINK_NUM_INC;
            }
            nexthop = &nexthop_addr;
            uip_ipaddr_copy(nexthop, &UIP_IP_BUF->destipaddr);
        }
        else
        {
            //If the address is not onlink we need to search for a route
#if (UIP_DS6_ROUTING_MODE != UIP_DS6_SOURCE_ROUTING)
            uip_ds6_route_t *route;
            /* Check if we have a route to the destination address. */
            route = uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr);

            /* No route was found - we send to the default route instead. */
            if (route == NULL)
            {
                UIP_STAT_TX_ROUTE_NONE_NUM_INC;
                PRINTF("tcpip_ipv6_output: no route found, using default route\n");
                nexthop = uip_ds6_defrt_choose();
                if (nexthop == NULL)
                {
#ifdef UIP_FALLBACK_INTERFACE
                    PRINTF("FALLBACK: removing ext hdrs & setting proto %d %d\n",
                            uip_ext_len, *((u8_t *)UIP_IP_BUF + UIP_IPH_LEN));
                    if(uip_ext_len > 0)
                    {
                        extern void remove_ext_hdr(void);
                        u8_t proto = *((u8_t *)UIP_IP_BUF + UIP_IPH_LEN);
                        remove_ext_hdr();
                        /* This should be copied from the ext header... */
                        UIP_IP_BUF->proto = proto;
                    }
                    UIP_FALLBACK_INTERFACE.output();
#else
                    PRINTF("tcpip_ipv6_output: Destination off-link but no route\n");
#endif /* !UIP_FALLBACK_INTERFACE */
                    UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_NO_ROUTE_TO_HOST);
                    uip_len= 0;
                    uip_ext_len = 0;
                    return;
                }
                UIP_STAT_TX_ROUTE_DEFAULT_NUM_INC;
            }
            else
            {
                UIP_STAT_TX_ROUTE_FOUND_NUM_INC;
                /* A route was found, so we look up the nexthop neighbor for
                 the route. */
                nexthop = uip_ds6_route_nexthop(route);

                /* If the nexthop is dead, for example because the neighbor
                 never responded to link-layer acks, we drop its route. */
                if (nexthop == NULL)
                {
                    uip_ds6_route_rm(route);
                    UIP_STAT_TX_NBR_NONE_FOR_ROUTE_NUM_INC;
                    UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_NO_NBR_FOR_NEXT_HOP);
                    /* We don't have a nexthop to send the packet to, so we drop
                     it. */
                    uip_len= 0;
                    uip_ext_len = 0;
                    return;
                }
            }

#else //(UIP_DS6_ROUTING == SOURCE ROUTING)
            //in the case of NON-STORING ROOT, the header needs to be generated for the routing
            uip_ds6_sourceRouting_path_t *path;
            /* Check if we have a route to the destination address. */
            path = uip_ds6_sourceRouting_path_lookup(&UIP_IP_BUF->destipaddr);

            /* No route was found - we send to the default route instead. */
            if(path == NULL)
            {
                UIP_STAT_TX_ROUTE_NONE_NUM_INC;
                PRINTF("tcpip_ipv6_output: no path found for non-storing root\n");
#ifdef UIP_FALLBACK_INTERFACE
                PRINTF("FALLBACK: removing ext hdrs & setting proto %d %d\n",
                        uip_ext_len, *((u8_t *)UIP_IP_BUF + UIP_IPH_LEN));
                if(uip_ext_len > 0)
                {
                    extern void remove_ext_hdr(void);
                    u8_t proto = *((u8_t *)UIP_IP_BUF + UIP_IPH_LEN);
                    remove_ext_hdr();
                    /* This should be copied from the ext header... */
                    UIP_IP_BUF->proto = proto;
                }
                UIP_FALLBACK_INTERFACE.output();
#else
                PRINTF("tcpip_ipv6_output: Destination off-link but no route\n");
#endif /* !UIP_FALLBACK_INTERFACE */
                UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_NO_ROUTE_TO_HOST);
                uip_len = 0;
                uip_ext_len = 0;
                return;

            }
            else
            {
                /* A path was found, so we generate the source routing header and send to next hop */
                //If path is not valid (has loops), discard!
                if (!uip_ds6_route_sourceRouting_isValidPath(path))
                {
                    UIP_STAT_TX_PATH_INVALID_NUM_INC;
                    UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_INVALID_PATH);
                    uip_len = 0;
                    uip_ext_len = 0;
                    return;
                }
                UIP_STAT_TX_PATH_FOUND_NUM_INC;
                //We get the nexthop for the IPv6 packet. This is the first address of the path.
                if (uip_ds6_sourceRouting_path_num_nodes()>1)
                {
                    if (!UIP_SRC_ROUTING_HDR_EXTENSION(UIP_SOURCE_ROUTING_HDR_INSERT, path))
                    {
                        UIP_STAT_TX_CANNOT_INSERT_SRH_INC;
                        UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_CANNOT_INSERT_SRH);
                        uip_len = 0;
                        uip_ext_len = 0;
                        return;
                    }
                    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, path->ipaddr);
                }
                nexthop = &nexthop_addr;
                uip_ipaddr_copy(nexthop, path->ipaddr);
            }
#endif //(UIP_DS6_ROUTING_MODE == UIP_DS6_SOURCE_ROUTING)

#if TCPIP_CONF_ANNOTATE_TRANSMISSIONS
            if(nexthop != NULL)
            {
                static u16_t annotate_last;
                static u16_t annotate_has_last = 0;

                if(annotate_has_last)
                {
                    printf("#L %u 0; red\n", annotate_last);
                }
                printf("#L %u 1; red\n", nexthop->u8[sizeof(uip_ipaddr_t) - 1]);
                annotate_last = nexthop->u8[sizeof(uip_ipaddr_t) - 1];
                annotate_has_last = 1;
            }
#endif /* TCPIP_CONF_ANNOTATE_TRANSMISSIONS */
        }

        /* End of next hop determination */

        if (UIP_HBHO_HDR_EXTENSION(UIP_HBHO_HDR_UPDATE_NEXT_HOP, nexthop))
        {
            uip_len= 0;
            uip_ext_len = 0;
            return;
        }

        nbr = uip_ds6_nbr_lookup(nexthop);
        if (nbr == NULL)
        {
            UIP_STAT_TX_NBR_UNKNOWN_NUM_INC;
            UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_NO_NBR_FOR_NEXT_HOP);
#if UIP_ND6_SEND_NA

            if ((nbr = uip_ds6_nbr_add(nexthop, NULL, 0, NBR_INCOMPLETE))
                    == NULL)
            {
                uip_len= 0;
                uip_ext_len = 0;
                return;
            }
            else
            {

#if UIP_CONF_IPV6_QUEUE_PKT
                /* Copy outgoing pkt in the queuing buffer for later transmit. */
                if(uip_packetqueue_alloc(&nbr->packethandle, UIP_DS6_NBR_PACKET_LIFETIME) != NULL)
                {
                    memcpy(uip_packetqueue_buf(&nbr->packethandle), UIP_IP_BUF, uip_len);
                    uip_packetqueue_set_buflen(&nbr->packethandle, uip_len);
                }
#endif
                /* RFC4861, 7.2.2:
                 * "If the source address of the packet prompting the solicitation is the
                 * same as one of the addresses assigned to the outgoing interface, that
                 * address SHOULD be placed in the IP Source Address of the outgoing
                 * solicitation.  Otherwise, any one of the addresses assigned to the
                 * interface should be used."*/

                if(uip_ds6_is_my_addr(&UIP_IP_BUF->srcipaddr))
                {
                    uip_nd6_ns_output(&UIP_IP_BUF->srcipaddr, NULL, &nbr->ipaddr);
                }
                else
                {
                    uip_nd6_ns_output(NULL, NULL, &nbr->ipaddr);
                }

                tcpip_output(NULL);
                uip_len = 0;

                stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);
                nbr->nscount = 1;
            }
#endif /* UIP_ND6_SEND_NA */
        }
        else
        {
#if UIP_ND6_SEND_NA
            if(nbr->state == NBR_INCOMPLETE)
            {
                UIP_STAT_TX_NBR_INCOMPLETE_NUM_INC;
                UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_NBR_STATE_INCOMPLETE);
                PRINTF("tcpip_ipv6_output: nbr cache entry incomplete\n");
#if UIP_CONF_IPV6_QUEUE_PKT
                /* Copy outgoing pkt in the queuing buffer for later transmit and set
                 the destination nbr to nbr. */
                if(uip_packetqueue_alloc(&nbr->packethandle, UIP_DS6_NBR_PACKET_LIFETIME) != NULL)
                {
                    memcpy(uip_packetqueue_buf(&nbr->packethandle), UIP_IP_BUF, uip_len);
                    uip_packetqueue_set_buflen(&nbr->packethandle, uip_len);
                }
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/
                uip_len = 0;
                uip_ext_len = 0;
                return;
            }
            /* Send in parallel if we are running NUD (nbc state is either STALE,
             DELAY, or PROBE). See RFC 4861, section 7.3.3 on node behavior. */
            if(nbr->state == NBR_STALE)
            {
                UIP_STAT_TX_NBR_STALE_NUM_INC;
                nbr->state = NBR_DELAY;
                stimer_set(&nbr->reachable, UIP_ND6_DELAY_FIRST_PROBE_TIME);
                nbr->nscount = 0;
                PRINTF("tcpip_ipv6_output: nbr cache entry stale moving to delay\n");
            }
#endif /* UIP_ND6_SEND_NA */

            if (tcpip_output(uip_ds6_nbr_get_ll(nbr)))
            {
                UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_SCHEDULED);
            }
            else
            {
                UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_SCHEDULED_ERROR);
            }

#if UIP_CONF_IPV6_QUEUE_PKT
#error "No Support!"
            /*
             * Send the queued packets from here, may not be 100% perfect though.
             * This happens in a few cases, for example when instead of receiving a
             * NA after sendiong a NS, you receive a NS with SLLAO: the entry moves
             * to STALE, and you must both send a NA and the queued packet.
             */
            if(uip_packetqueue_buflen(&nbr->packethandle) != 0)
            {
                uip_len = uip_packetqueue_buflen(&nbr->packethandle);
                memcpy(UIP_IP_BUF, uip_packetqueue_buf(&nbr->packethandle), uip_len);
                uip_packetqueue_free(&nbr->packethandle);
                tcpip_output(uip_ds6_nbr_get_ll(nbr));
            }
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/
            uip_len = 0;
            uip_ext_len = 0;
            return;
        }
        return;
    }
    UIP_STAT_TX_MCAST_NUM_INC;
    /* Multicast IP destination address. */
    if (tcpip_output(NULL))
    {
        UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_SCHEDULED);
    }
    else
    {
        UIP_TX_STATUS_NOTIFICATION(UIP_TX_STATUS_SCHEDULED_ERROR);
    }
    uip_len= 0;
    uip_ext_len = 0;
}
#endif /* UIP_CONF_IPV6 */
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void tcpip_poll_udp(struct uip_udp_conn *conn)
{
    //tcpip_process(UDP_POLL, conn);
    tcpip_process_obj.process_post(UDP_POLL, conn);
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
#if UIP_TCP
void
tcpip_poll_tcp(struct uip_conn *conn)
{
    //tcpip_process(TCP_POLL, conn);
    tcpip_process_obj.process_post(TCP_POLL, conn);
}
#endif /* UIP_TCP */
/*---------------------------------------------------------------------------*/
void tcpip_uipcall(void)
{
    uip_udp_appstate_t *ts;

#if UIP_UDP
    if (uip_conn != NULL)
    {
        ts = &uip_conn->appstate;
    }
    else
    {

        ts = &uip_udp_conn->appstate;
    }
#else /* UIP_UDP */
    ts = &uip_conn->appstate;
#endif /* UIP_UDP */

#if UIP_TCP
    {
        static unsigned char i;
        register struct listenport *l;

        /* If this is a connection request for a listening port, we must
         mark the connection with the right process ID. */
        if(uip_connected())
        {
            l = &s.listenports[0];
            for(i = 0; i < UIP_LISTENPORTS; ++i)
            {
                if(l->port == uip_conn->lport &&
                        l->p != PROCESS_NONE)
                {
                    ts->p = l->p;
                    ts->state = NULL;
                    break;
                }
                ++l;
            }

            /* Start the periodic polling, if it isn't already active. */
            start_periodic_tcp_timer();
        }
    }
#endif /* UIP_TCP */
    tcpip_event = UIP_APP_EVENT;

    //Store the application data length of the last packet received
    uip_app_datalen = uip_datalen();

    if (ts->p != PROCESS_NONE)
    {
        if (ts->p->process_post != NULL)
        {
            ts->p->process_post(tcpip_event, ts->state);
        }
    }

}

int tcpip_initialized = 0;

int tcpip_isInitialized()
{
    return tcpip_initialized;
}

void tcpip_init(uip_lladdr_t* my_lladdr,
                process_post_func_t tcpip_process_post_func)
{

    rimeaddr_copy((rimeaddr_t*) &uip_lladdr, (rimeaddr_t*) my_lladdr);

    process_obj_init(&tcpip_process_obj, tcpip_process_post_func);

#if UIP_TCP
    {
        static unsigned char i;

        for(i = 0; i < UIP_LISTENPORTS; ++i)
        {
            s.listenports[i].port = 0;
        }
        s.p = PROCESS_CURRENT();
    }
#endif

#if UIP_CONF_ICMP6
    tcpip_icmp6_event = process_alloc_event();
#endif /* UIP_CONF_ICMP6 */

    clock_init();

    uint16_t seed = 0;
    int i;
    for (i = 0; i < UIP_LLADDR_LEN; ++i)
    {
        seed += my_lladdr->addr16[i];
    }

#ifdef LINUX_GATEWAY
    random_init(seed);
#endif

    etimer_init();

#ifdef UIP_TCP
    etimer_timer_init(&periodic);
    etimer_set(&periodic, &tcpip_process_obj, TCP_PERIODIC_TIMER_TICKS); //(4 * CLOCK_SECOND / 2));
#endif
    nbr_table_init();

    uip_init();
#ifdef UIP_FALLBACK_INTERFACE
    UIP_FALLBACK_INTERFACE.init();
#endif

    udp_simple_socket_init();

    tcpip_initialized = 1;
}

void tcpip_clean()
{
#ifdef UIP_TCP
    etimer_stop(&periodic);
#endif
    uip_ds6_stop_periodic_timer();
}

