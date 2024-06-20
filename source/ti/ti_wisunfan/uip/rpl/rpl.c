/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *         ContikiRPL, an implementation of RPL: IPv6 Routing Protocol
 *         for Low-Power and Lossy Networks (IETF RFC 6550)
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
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

#include "net/uip.h"
#include "net/tcpip.h"
#include "net/uip-ds6.h"
#include "rpl/rpl-private.h"
#include "net/uip-icmp6.h"
#include "rpl/rpl-plusplus.h"
#include "uip_rpl_process.h"

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include <limits.h>
#include <string.h>

#if UIP_CONF_IPV6
#define UIP_IP_BUF                          ((struct uip_ip_hdr *)&rpl_ip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF    					((struct uip_icmp_hdr *)&rpl_ip_buf[uip_l2_l3_hdr_len])

#if RPL_CONF_STATS
rpl_stats_t rpl_stats;
#endif

#if !NON_STORING_MODE_ROOT
/*---------------------------------------------------------------------------*/
#if !RPL_ROUTE_INFINITE_LIFETIME
void rpl_purge_routes(void)
{
#if !NON_STORING_MODE_ROOT
    uip_ds6_route_t *r;
#else
    uip_ds6_route_sourceRouting_t *r;
#endif
    uip_ipaddr_t prefix;
    rpl_dag_t *dag;

    /* First pass, decrement lifetime */
    r = uip_ds6_route_head();

    while (r != NULL)
    {
        if (r->state.lifetime >= 1)
        {
            /*
             * If a route is at lifetime == 1, set it to 0, scheduling it for
             * immediate removal below. This achieves the same as the original code,
             * which would delete lifetime <= 1
             */
            r->state.lifetime--;
        }
        r = uip_ds6_route_next(r);
    }

    /* Second pass, remove dead routes */
    r = uip_ds6_route_head();

    while (r != NULL)
    {
        if (r->state.lifetime < 1)
        {
            /* Routes with lifetime == 1 have only just been decremented from 2 to 1,
             * thus we want to keep them. Hence < and not <= */
            uip_ipaddr_copy(&prefix, &r->ipaddr);
#if !NON_STORING_MODE_ROOT
            uip_ds6_route_rm(r);
#else
            uip_ds6_route_sourceRouting_rm(r);
#endif
            r = uip_ds6_route_head();
            PRINTF("No more routes to "); PRINT6ADDR(&prefix);
            dag = default_instance->current_dag;

            if (default_instance->mop != RPL_MOP_NON_STORING)
            {
                /* Propagate this information with a No-Path DAO to preferred parent if we are not a RPL Root */
                if (dag->rank != ROOT_RANK(default_instance))
                {
                    PRINTF(" -> generate No-Path DAO\n");
                    dao_output_target(dag->preferred_parent, &prefix,
                                      RPL_ZERO_LIFETIME);
                    /* Don't schedule more than 1 No-Path DAO, let next iteration handle that */
                    return;
                }
            } PRINTF("\n");
        }
        else
        {
            r = uip_ds6_route_next(r);
        }
    }
}
#endif //RPL_INFINITE_LIFETIME
/*---------------------------------------------------------------------------*/
void rpl_remove_routes(rpl_dag_t *dag)
{
    uip_ds6_route_t *r;

    r = uip_ds6_route_head();

    while (r != NULL)
    {
        if (r->state.dag == dag)
        {
            uip_ds6_route_rm(r);
            r = uip_ds6_route_head();
        }
        else
        {
            r = uip_ds6_route_next(r);
        }
    }
}
/************************************************************************/
void rpl_remove_routes_by_nexthop(uip_ipaddr_t *nexthop, rpl_dag_t *dag)
{
    uip_ds6_route_t *r;

    r = uip_ds6_route_head();

    while (r != NULL)
    {
        if (uip_ipaddr_cmp(uip_ds6_route_nexthop(r), nexthop)
                && r->state.dag == dag)
        {
            uip_ds6_route_rm(r);
            r = uip_ds6_route_head();
        }
        else
        {
            r = uip_ds6_route_next(r);
        }
    }
    //ANNOTATE("#L %u 0\n",nexthop->u8[sizeof(uip_ipaddr_t) - 1]);
}

/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
rpl_add_route(rpl_dag_t *dag, uip_ipaddr_t *prefix, int prefix_len,
              uip_ipaddr_t *next_hop)
{
    uip_ds6_route_t *rep;

    if ((rep = uip_ds6_route_add(prefix, prefix_len, next_hop)) == NULL)
    {
        PRINTF("RPL: No space for more route entries\n");
        return NULL;
    }

    rep->state.dag = dag;
#if !RPL_ROUTE_INFINITE_LIFETIME
    rep->state.lifetime = RPL_LIFETIME(dag->instance,
                                       dag->instance->default_lifetime);
#endif
    rep->state.learned_from = RPL_ROUTE_FROM_INTERNAL;

    PRINTF("RPL: Added a route to "); PRINT6ADDR(prefix); PRINTF("/%d via ", prefix_len); PRINT6ADDR(next_hop); PRINTF("\n");

    return rep;
}
#else
/*---------------------------------------------------------------------------*/
#if !RPL_ROUTE_INFINITE_LIFETIME
void
rpl_purge_routes(void)
{
    uip_ds6_route_sourceRouting_t *r;

    /* First pass, decrement lifetime */
    r = uip_ds6_route_sourceRouting_head();

    while(r != NULL)
    {
        if(r->state.lifetime >= 1)
        {
            /*
             * If a route is at lifetime == 1, set it to 0, scheduling it for
             * immediate removal below. This achieves the same as the original code,
             * which would delete lifetime <= 1
             */
            r->state.lifetime--;
        }
        r = uip_ds6_route_sourceRouting_next(r);
    }

    /* Second pass, remove dead routes */
    r = uip_ds6_route_sourceRouting_head();

    while(r != NULL)
    {
        if(r->state.lifetime < 1)
        {
            uip_ds6_route_sourceRouting_rm(r);
            r = uip_ds6_route_sourceRouting_head();
        }
        else
        {
            r = uip_ds6_route_sourceRouting_next(r);
        }
    }
}
#endif //RPL_ROUTE_INFINITE_LIFETIME
/*---------------------------------------------------------------------------*/
void
rpl_remove_routes(rpl_dag_t *dag)
{
    uip_ds6_route_sourceRouting_t *r;

    r = uip_ds6_route_sourceRouting_head();

    while(r != NULL)
    {
        if(r->state.dag == dag)
        {
            uip_ds6_route_sourceRouting_rm(r);
            r = uip_ds6_route_sourceRouting_head();
        }
        else
        {
            r = uip_ds6_route_sourceRouting_next(r);
        }
    }
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_sourceRouting_t *
rpl_add_route_nonstoring(rpl_dag_t *dag, uip_ipaddr_t *prefix, int prefix_len,
        uip_ipaddr_t *target_parent)
{
    uip_ds6_route_sourceRouting_t *rep;

    if((rep = uip_ds6_route_sourceRouting_add(prefix, prefix_len, target_parent)) == NULL)
    {
        PRINTF("RPL: No space for more route entries\n");
        UIP_STAT_ROUTE_NOT_ADDED_NUM_INC;
        return NULL;
    }

    rep->state.dag = dag;
#if !RPL_ROUTE_INFINITE_LIFETIME
    rep->state.lifetime = RPL_LIFETIME(dag->instance, dag->instance->default_lifetime);
#endif
    rep->state.learned_from = RPL_ROUTE_FROM_INTERNAL;

    PRINTF("RPL: Added a route to ");
    PRINT6ADDR(prefix);
    PRINTF("/%d parent ", prefix_len);
    PRINT6ADDR(target_parent);
    PRINTF("\n");

    UIP_STAT_ROUTE_ADDED_NUM_INC;

    return rep;
}
#endif
/*---------------------------------------------------------------------------*/
void rpl_ipv6_tx_confirmation(const rimeaddr_t *addr, int status, int numtx)
{
    uip_ipaddr_t ipaddr;
    rpl_parent_t *parent;
    rpl_instance_t *instance;
#if !RPL_SINGLE_INSTANCE
    rpl_instance_t *end;
#endif

    uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, (uip_lladdr_t *) addr);

#if RPL_SINGLE_INSTANCE
    instance = &single_instance;
#else
    for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES; instance < end; ++instance)
#endif
    {
        if (instance->used == 1)
        {
            parent = rpl_find_parent_any_dag(instance, &ipaddr);
            if (parent != NULL)
            {
                /* Trigger DAG rank recalculation. */
                PRINTF("RPL: rpl_link_neighbor_callback triggering update\n");
                parent->updated = 1;
                if (instance->of->neighbor_link_callback != NULL)
                {
                    instance->of->neighbor_link_callback(parent, status, numtx);
                }
            }
            if (status == MAC_TX_NOACK)
            {
#if RPL_PLUSPLUS_PROBE_IF_NO_ACK
                rpl_no_ack(&ipaddr);
#endif

#if !NON_STORING_MODE_ROOT
                //For NON-STORING mode, if the packet sent has the routing header and it failed to be sent, send a icmpv6 message to the root to inform this
                if (UIP_IP_BUF->proto == UIP_PROTO_ROUTING)
                {
                    if (instance->current_dag != NULL)
                    {
                        uip_icmp6_error_output_destination(
                                ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_ERRORSHR,
                                0, &instance->current_dag->dag_id); //UIP_IPH_LEN + uip_ext_len + 2);

                        tcpip_ipv6_output();
                    }
                }
#endif

                //RPL++
#if RPL_PLUSPLUS_MODIFY_DAO_INTERVAL  
                if (UIP_ICMP_BUF->type == ICMP6_RPL && UIP_ICMP_BUF->icode == RPL_CODE_DAO)
                {
                    if (ctimer_expired(&instance->dao_timer))
                    {
                        //If the packet sent is a SCHEDULED DAO, increase the interval for sending it
                        rpl_modify_dao_interval(instance);
                    }
                }
#endif
            }
        }
    }
}
/*---------------------------------------------------------------------------*/
void rpl_ipv6_neighbor_callback(int event, uip_ds6_nbr_t *nbr)
{
    rpl_parent_t *p;
    rpl_instance_t *instance;
#if !RPL_SINGLE_INSTANCE
    rpl_instance_t *end;
#endif
#if NON_STORING_MODE_ROOT
    uip_ds6_route_sourceRouting_t* locrt;
#endif

    if (event == RPL_DS6_NBR_RM)
    {
        PRINTF("RPL: Removing neighbor "); PRINT6ADDR(&nbr->ipaddr); PRINTF("\n");

#if RPL_SINGLE_INSTANCE
        instance = &single_instance;
#else
        for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES; instance < end; ++instance)
#endif
        {
            if (instance->used == 1)
            {
                p = rpl_find_parent_any_dag(instance, &nbr->ipaddr);
                if (p != NULL)
                {
                    p->rank = INFINITE_RANK; //rpl periodic function will handle the removal of this parent from the parent set
                    /* Trigger DAG rank recalculation. */
                    PRINTF("RPL: rpl_ipv6_neighbor_callback infinite rank\n");
                    p->updated = 1;
                    //This neighbor has been removed so put it to NULL
                    p->nbr = NULL;
                }
                else
                {
#if !NON_STORING_MODE_ROOT
                    //if Non-Storing mode is used, we do not have routes so no need to check the routes related to the neighbor
#if (RPL_CONF_MOP != RPL_MOP_NON_STORING)
                    //RPL++ Also remove routes related with the neighbor
#if RPL_PLUSPLUS_NEXT_HOP_DEAD
                    rpl_nexthop_dead(nbr);
#endif
#endif
#else
                    //We are the dodag root in non-storing mode, we should erase the route of the node deleted
                    if (locrt = uip_ds6_sourceRouting_route_lookup(&nbr->ipaddr))
                    {
                        uip_ds6_route_sourceRouting_rm(locrt);
                    }
#endif
                }
            }
        }
    }
}
/*---------------------------------------------------------------------------*/
#if !UIP_RPL_ROOT || NON_STORING_MODE_ROOT
void rpl_route_callback(int event, void* route)
{
}
#else
void rpl_route_callback(int event, void* route)
{
    uip_ds6_route_t* ds6_route = (uip_ds6_route_t*) route;
    if (event == RPL_DS6_ROUTE_RM)
    {
        //A route was deleted. If it was deleted because the next hop is not in the table, trigger a global repair (if we are root, of course)
        if (uip_ds6_route_nexthop(ds6_route) == NULL)
        {
            rpl_dag_t *dag;
            rpl_instance_t *instance;
            dag = (rpl_dag_t *)ds6_route->state.dag;
            if(dag != NULL)
            {
                instance = dag->instance;
                rpl_repair_root(instance->instance_id);
            }
        }
    }
}
#endif
/*---------------------------------------------------------------------------*/
void rpl_ipv6_ds6_notification(int event, void *data)
{
    uip_ds6_nbr_t *nbr;
    switch (event)
    {
    case RPL_DS6_NBR_ADD:
    case RPL_DS6_NBR_RM:
        nbr = (uip_ds6_nbr_t *) data;
        rpl_ipv6_neighbor_callback(event, nbr);
        break;
    case RPL_DS6_ROUTE_DEFRT_ADD:
    case RPL_DS6_ROUTE_DEFRT_RM:
    case RPL_DS6_ROUTE_ADD:
    case RPL_DS6_ROUTE_RM:
        rpl_route_callback(event, data);
        break;
    }
}
/*---------------------------------------------------------------------------*/
void rpl_periodic_timer_init();
void rpl_instance_init();
void rpl_dag_init();

void rpl_init(
        void (*rpl_process_post_func)(process_event_t event,
                                      process_data_t data))
{
    uip_ipaddr_t rplmaddr;
    PRINTF("RPL started\n");
    default_instance = NULL;

    etimer_init();
    ctimer_init(rpl_process_post_func);

    rpl_periodic_timer_init();
    rpl_instance_init();
    rpl_dag_init();
    rpl_reset_periodic_timer();

    /* add rpl multicast address */
    uip_create_linklocal_rplnodes_mcast(&rplmaddr);
    uip_ds6_maddr_add(&rplmaddr);

#if RPL_CONF_STATS
    memset(&rpl_stats, 0, sizeof(rpl_stats));
#endif
}

#if UIP_RPL_ROOT
void rpl_start_instance(uip_ipaddr_t *prefix)
{
    uip_ipaddr_t ipaddr;
    rpl_dag_t *dag;

    //Set manually the global address only for the ROOT
    memcpy(&ipaddr, &prefix->u16, sizeof(ipaddr));
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);

    //Now start the instance and dag
    uip_ds6_set_addr_iid((uip_ipaddr_t*)&prefix->u16, &uip_lladdr);
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&prefix->u16);
    rpl_set_prefix(dag, (uip_ipaddr_t*)&prefix->u16, 64);
}
#endif

void rpl_dtsn_increment(rpl_instance_t *instance)
{
    RPL_LOLLIPOP_INCREMENT(instance->dtsn_out);
#if RPL_PLUSPLUS_DTSN_STRATEGY 
    //The strategy states that whenever we increment the DTSN, we reset the DIO timer
    rpl_reset_dio_timer(instance);
#endif
}
/*---------------------------------------------------------------------------*/
#endif /* UIP_CONF_IPV6 */
