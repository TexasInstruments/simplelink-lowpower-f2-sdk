/**
 * \addtogroup uip6
 * @{
 */

/*
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 *
 */

/**
 * \file
 *         IPv6 Neighbor cache (link-layer/IPv6 address mapping)
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 * \author Simon Duquennoy <simonduq@sics.se>
 *
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

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "lib/list.h"
#include "rime/rimeaddr.h"
#include "net/packetbuf.h"
#include "net/uip-ds6-nbr.h"
#include "lib/memb.h"

#include "uip_rpl_process.h"

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

NBR_TABLE_GLOBAL(uip_ds6_nbr_t, ds6_neighbors);

LIST(secondary_ipaddr_nbr_list);
MEMB(secondary_ipaddr_nbr_memb, secondary_ipaddr_nbr_t,
     NBR_TABLE_MAX_NEIGHBORS);

/*---------------------------------------------------------------------------*/
void uip_ds6_neighbors_init(void)
{

    nbr_table_register(ds6_neighbors, (nbr_table_callback *) uip_ds6_nbr_rm);

    memb_init(&secondary_ipaddr_nbr_memb);
    list_init(secondary_ipaddr_nbr_list);

}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_add(const uip_ipaddr_t *ipaddr, const uip_lladdr_t *lladdr,
                u16_t isrouter, u16_t state)
{
    //search for an entry of this ll address in the nbr table
    uip_ds6_nbr_t *nbr;
    if (!(nbr = uip_ds6_nbr_ll_lookup(lladdr)))
    {

        //Add new entry to neighbor table, same as it used to be
        nbr = nbr_table_add_lladdr(ds6_neighbors, (rimeaddr_t*) lladdr);
        if (nbr)
        {
            uip_ipaddr_copy(&nbr->ipaddr, ipaddr);
            nbr->isrouter = isrouter;
            nbr->state = state;
#if UIP_CONF_IPV6_QUEUE_PKT
            uip_packetqueue_new(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
            /* timers are set separately, for now we put them in expired state */
            stimer_set(&nbr->reachable, 0);
            stimer_set(&nbr->sendns, 0);
            nbr->nscount = 0;
            PRINTF("Adding neighbor with ip addr "); PRINT6ADDR(ipaddr); PRINTF(" link addr "); PRINTLLADDR(lladdr); PRINTF(" state %u\n", state);
            UIP_DS6_NOTIFICATION(UIP_DS6_NBR_ADD, nbr);
            UIP_STAT_NEIGHBOR_ADDED_NUM_INC;
            return nbr;
        }
        else
        {
            PRINTF("uip_ds6_nbr_add drop ip addr "); PRINT6ADDR(ipaddr); PRINTF(" link addr (%p) ", lladdr); PRINTLLADDR(lladdr); PRINTF(" state %u\n", state);
            UIP_STAT_NEIGHBOR_NOT_ADDED_NUM_INC;
            return NULL;
        }
//in the case we found the nbr with this ll address in the table, add ip address to the secondary list and point to this nbr
    }
    else
    {
        /* Allocate an entry for the secondary ip address list*/
        secondary_ipaddr_nbr_t* entry = memb_alloc(&secondary_ipaddr_nbr_memb);
        if (entry == NULL)
        {
            PRINTF("uip_ds6_nbr_add: could not allocate secondary ip address for neighbor table\n");
            UIP_STAT_NEIGHBOR_SECONDARY_NOT_ADDED_NUM_INC;
            return NULL;
        }
        entry->nbr = nbr;
        uip_ipaddr_copy(&entry->ipaddr, ipaddr);
        list_add(secondary_ipaddr_nbr_list, entry);
        UIP_STAT_NEIGHBOR_SECONDARY_ADDED_NUM_INC;
        return nbr;
    }

}

void remove_secondary_entries(uip_ds6_nbr_t *nbr)
{
    secondary_ipaddr_nbr_t* sec_ipaddr_nbr_entry = list_head(
            secondary_ipaddr_nbr_list);
    while (sec_ipaddr_nbr_entry)
    {
        if (sec_ipaddr_nbr_entry->nbr == nbr)
        {
            //this entry points to the same neighbor, remove the entry
            list_remove(secondary_ipaddr_nbr_list, sec_ipaddr_nbr_entry);
            memb_free(&secondary_ipaddr_nbr_memb, sec_ipaddr_nbr_entry);
            //restart from the beginning
            sec_ipaddr_nbr_entry = list_head(secondary_ipaddr_nbr_list);
        }
        else
        {
            sec_ipaddr_nbr_entry = list_item_next(sec_ipaddr_nbr_entry);
        }
    }
}

/*---------------------------------------------------------------------------*/
void uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr)
{
    if (nbr != NULL)
    {
#if UIP_CONF_IPV6_QUEUE_PKT
        uip_packetqueue_free(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
        //need to check if the secondary ip address nbr list has any entries for this nbr and remove them too
        remove_secondary_entries(nbr);

        UIP_DS6_NOTIFICATION(UIP_DS6_NBR_RM, nbr);
        nbr_table_remove(ds6_neighbors, nbr);
        UIP_STAT_NEIGHBOR_DELETED_NUM_INC;
    }
    return;
}

/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_nbr_get_ipaddr(uip_ds6_nbr_t *nbr)
{
    return (nbr != NULL) ? &nbr->ipaddr : NULL;
}

/*---------------------------------------------------------------------------*/
uip_lladdr_t *
uip_ds6_nbr_get_ll(uip_ds6_nbr_t *nbr)
{
    return (uip_lladdr_t *) nbr_table_get_lladdr(ds6_neighbors, nbr);
}
/*---------------------------------------------------------------------------*/
int uip_ds6_nbr_num(void)
{
    uip_ds6_nbr_t *nbr;
    int num;

    num = 0;
    for (nbr = nbr_table_head(ds6_neighbors); nbr != NULL;
            nbr = nbr_table_next(ds6_neighbors, nbr))
    {
        num++;
    }
    return num;
}

secondary_ipaddr_nbr_t* secondary_ipaddr_nbr_head()
{
    return list_head(secondary_ipaddr_nbr_list);
}

secondary_ipaddr_nbr_t* secondary_ipaddr_nbr_next(secondary_ipaddr_nbr_t* entry)
{
    return list_item_next(entry);
}

uip_ds6_nbr_t *lookup_secondary_table(uip_ipaddr_t *ipaddr)
{
    secondary_ipaddr_nbr_t* sec_ipaddr_nbr_entry = secondary_ipaddr_nbr_head();
    while (sec_ipaddr_nbr_entry)
    {
        if (uip_ipaddr_cmp(&sec_ipaddr_nbr_entry->ipaddr, ipaddr))
        {
            return sec_ipaddr_nbr_entry->nbr;
        }
        else
        {
            sec_ipaddr_nbr_entry = secondary_ipaddr_nbr_next(
                    sec_ipaddr_nbr_entry);
        }
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_lookup(uip_ipaddr_t *ipaddr)
{
    uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
    if (ipaddr != NULL)
    {
        while (nbr != NULL)
        {
            if (uip_ipaddr_cmp(&nbr->ipaddr, ipaddr))
            {
                return nbr;
            }
            nbr = nbr_table_next(ds6_neighbors, nbr);
        }
        //if we did not find it in the nbr table, take a look in the secondary ip address table
        return lookup_secondary_table(ipaddr);
    }
    return NULL;
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_ll_lookup(const uip_lladdr_t *lladdr)
{
    return nbr_table_get_from_lladdr(ds6_neighbors, (rimeaddr_t*) lladdr);
}

/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_nbr_ipaddr_from_lladdr(uip_lladdr_t *lladdr)
{
    uip_ds6_nbr_t *nbr = uip_ds6_nbr_ll_lookup(lladdr);
    return nbr ? &nbr->ipaddr : NULL;
}

/*---------------------------------------------------------------------------*/
uip_lladdr_t *
uip_ds6_nbr_lladdr_from_ipaddr(uip_ipaddr_t *ipaddr)
{
    uip_ds6_nbr_t *nbr = uip_ds6_nbr_lookup(ipaddr);
    return nbr ? uip_ds6_nbr_get_ll(nbr) : NULL;
}
/*---------------------------------------------------------------------------*/
void uip_ds6_link_neighbor_callback(int status, int numtx)
{
    const rimeaddr_t *dest;

    dest = (rimeaddr_t *) &uip_dst_lladdr;

    if (rimeaddr_cmp(dest, &rimeaddr_null))
    { //this means that the message was a broadcast
        return;
    }

    LINK_NEIGHBOR_CALLBACK(dest, status, numtx);

#if UIP_DS6_LL_NUD
    if (status == MAC_TX_OK)
    {
        uip_ds6_nbr_t *nbr;
        nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *) dest);
        if (nbr != NULL
                && (nbr->state == NBR_STALE || nbr->state == NBR_DELAY
                        || nbr->state == NBR_PROBE
                        || nbr->state == NBR_REACHABLE))
        {
            nbr->state = NBR_REACHABLE;
            stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
            PRINTF("uip-ds6-neighbor : received a link layer ACK : "); PRINTLLADDR((uip_lladdr_t *)dest); PRINTF(" is reachable.\n");
        }
    }
#endif /* UIP_DS6_LL_NUD */

}
/*---------------------------------------------------------------------------*/
void uip_ds6_neighbor_periodic(void)
{
    /* Periodic processing on neighbors */
    uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
    while (nbr != NULL)
    {
        switch (nbr->state)
        {
        case NBR_REACHABLE:
            if (stimer_expired(&nbr->reachable))
            {
                PRINTF("REACHABLE: moving to STALE ("); PRINT6ADDR(&nbr->ipaddr); PRINTF(")\n");
                nbr->state = NBR_STALE;
            }
            break;
#if UIP_ND6_SEND_NA
        case NBR_INCOMPLETE:
            if (nbr->nscount >= UIP_ND6_MAX_MULTICAST_SOLICIT)
            {
                uip_ds6_nbr_rm(nbr);
            }
            else if (stimer_expired(&nbr->sendns) && (uip_len== 0))
            {
                nbr->nscount++;
                PRINTF("NBR_INCOMPLETE: NS %u\n", nbr->nscount);
                uip_nd6_ns_output(NULL, NULL, &nbr->ipaddr);
                stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);
                //Needs to be sent immediately
                UIP_STAT_TX_ROUTE_ONLINK_NUM_INC;
                UIP_STAT_TX_IP_NUM_INC;
                tcpip_output(NULL);
                uip_len = 0;
            }
            break;
            case NBR_DELAY:
            if(stimer_expired(&nbr->reachable))
            {
                nbr->state = NBR_PROBE;
                nbr->nscount = 0;
                PRINTF("DELAY: moving to PROBE\n");
                stimer_set(&nbr->sendns, 0);
            }
            break;
            case NBR_PROBE:
            if(nbr->nscount >= UIP_ND6_MAX_UNICAST_SOLICIT)
            {
                uip_ds6_defrt_t *locdefrt;
                PRINTF("PROBE END\n");
                if((locdefrt = uip_ds6_defrt_lookup(&nbr->ipaddr)) != NULL)
                {
                    if (!locdefrt->isinfinite)
                    {
                        uip_ds6_defrt_rm(locdefrt);
                    }
                }
                uip_ds6_nbr_rm(nbr);
            }
            else if(stimer_expired(&nbr->sendns) && (uip_len == 0))
            {
                nbr->nscount++;
                PRINTF("PROBE: NS %u\n", nbr->nscount);
                uip_nd6_ns_output(NULL, &nbr->ipaddr, &nbr->ipaddr);
                stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);
                //Needs to be sent immediately
                UIP_STAT_TX_ROUTE_ONLINK_NUM_INC;
                UIP_STAT_TX_IP_NUM_INC;
                tcpip_output(uip_ds6_nbr_get_ll(nbr));
                uip_len = 0;
            }
            break;
#endif /* UIP_ND6_SEND_NA */
            default:
            break;
        }
        nbr = nbr_table_next(ds6_neighbors, nbr);
    }
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_get_least_lifetime_neighbor(void)
{
    uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
    uip_ds6_nbr_t *nbr_expiring = NULL;
    while (nbr != NULL)
    {
        if (nbr_expiring != NULL)
        {
            clock_time_t curr = stimer_remaining(&nbr->reachable);
            if (curr < stimer_remaining(&nbr->reachable))
            {
                nbr_expiring = nbr;
            }
        }
        else
        {
            nbr_expiring = nbr;
        }
        nbr = nbr_table_next(ds6_neighbors, nbr);
    }
    return nbr_expiring;
}
/*---------------------------------------------------------------------------*/
void uip_ds6_nbr_lock(uip_ds6_nbr_t *nbr)
{
    nbr_table_lock(ds6_neighbors, nbr);
}
/*---------------------------------------------------------------------------*/
void uip_ds6_nbr_unlock(uip_ds6_nbr_t *nbr)
{
    nbr_table_unlock(ds6_neighbors, nbr);
}
