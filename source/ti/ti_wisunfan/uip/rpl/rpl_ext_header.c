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
 *         Management of extension headers for ContikiRPL.
 *
 * \author Vincent Brillault <vincent.brillault@imag.fr>,
 *         Joakim Eriksson <joakime@sics.se>,
 *         Niclas Finne <nfi@sics.se>,
 *         Nicolas Tsiftes <nvt@sics.se>.
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

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include <limits.h>
#include <string.h>

#if USE_RPL_HBHO_HEADER

/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF                ((struct uip_ip_hdr *)&rpl_ip_buf[UIP_LLH_LEN])
#define UIP_EXT_BUF               ((struct uip_ext_hdr *)&rpl_ip_buf[uip_l2_l3_hdr_len])
#define UIP_HBHO_BUF              ((struct uip_hbho_hdr *)&rpl_ip_buf[uip_l2_l3_hdr_len])
#define UIP_HBHO_NEXT_BUF         ((struct uip_ext_hdr *)&rpl_ip_buf[uip_l2_l3_hdr_len + RPL_HOP_BY_HOP_LEN])
#define UIP_EXT_HDR_OPT_BUF       ((struct uip_ext_hdr_opt *)&rpl_ip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])
#define UIP_EXT_HDR_OPT_PADN_BUF  ((struct uip_ext_hdr_opt_padn *)&rpl_ip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])
#define UIP_EXT_HDR_OPT_RPL_BUF   ((struct uip_ext_hdr_opt_rpl *)&rpl_ip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])
/*---------------------------------------------------------------------------*/
//LINUX_GATEWAY CHANGE: issues with static implementation but non-static definition of some functions,
//also using functions before they're declared, not defining functions at all... etc.
#if UIP_CONF_IPV6

/*---------------------------------------------------------------------------*/
void
rpl_remove_header(void)
{
    u16_t temp_len;

    rpl_ip_ext_len = 0;

    PRINTF("RPL: Verifying the presence of the RPL header option\n");
    switch(UIP_IP_BUF->proto)
    {
        case UIP_PROTO_HBHO:
        PRINTF("RPL: Removing the RPL header option\n");
        UIP_IP_BUF->proto = UIP_HBHO_BUF->next;
        temp_len = UIP_IP_BUF->len1;
        rpl_ip_len -= UIP_HBHO_BUF->len + 8;
        UIP_IP_BUF->len1 -= UIP_HBHO_BUF->len + 8;
        if(UIP_IP_BUF->len1 > temp_len)
        {
            UIP_IP_BUF->len0--;
        }
        memmove(UIP_EXT_BUF, UIP_HBHO_NEXT_BUF, CHAR_BIT_UNITS(rpl_ip_len) - UIP_IPH_LEN);
        break;
        default:
        PRINTF("RPL: No hop-by-hop Option found\n");
    }
}
/*---------------------------------------------------------------------------*/
static int
rpl_verify_header(int uip_ext_opt_offset)
{
    rpl_instance_t *instance;
#if !NON_STORING_MODE_ROOT
    int down;
    u16_t sender_closer;
    uip_ds6_route_t *route;
#endif

    if(UIP_EXT_HDR_OPT_RPL_BUF->opt_len != BYTE_UNITS(RPL_HDR_OPT_LEN))
    {
        PRINTF("RPL: Bad header option! (wrong length)\n");
        return 1;
    }

    instance = rpl_get_instance(UIP_EXT_HDR_OPT_RPL_BUF->instance);
    if(instance == NULL)
    {
        PRINTF("RPL: Unknown instance: %u\n",
                UIP_EXT_HDR_OPT_RPL_BUF->instance);
        return 1;
    }

#if NON_STORING_MODE_ROOT //Don't need to verify. Will generate source routing header for downward traffic
    //rpl_remove_header();
    return 0;
#else
    if(UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_FWD_ERR)
    {
        PRINTF("RPL: Forward error!\n");
        /* We should try to repair it by removing the neighbor that caused
         the packet to be forwarded in the first place. We drop any
         routes that go through the neighbor that sent the packet to
         us. */
        route = uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr);
        if(route != NULL)
        {
            if (instance->current_dag->preferred_parent != NULL)
            { //if we have a preferred parent, send him DAO No Path to tell that the route is lost
                //Send this if we have a preferred parent. If we do not have a preferred parent, probably we are the root
                dao_output_target(instance->current_dag->preferred_parent, &route->ipaddr, RPL_ZERO_LIFETIME);
            }
            uip_ds6_route_rm(route);

#if UIP_RPL_ROOT
            /* If we are the root and just needed to remove a DAO route,
             chances are that the network needs to be repaired. The
             rpl_repair_root() function will cause a global repair if we
             happen to be the root node of the dag. */
            PRINTF("RPL: initiate global repair\n");
            rpl_repair_root(instance->instance_id);
#endif

            UIP_STAT_RPL_FORWARDING_ERROR_NUM_INC;
        }

        /* Remove the forwarding error flag and return 0 to let the packet
         be forwarded again. */
        UIP_EXT_HDR_OPT_RPL_BUF->flags &= ~RPL_HDR_OPT_FWD_ERR;
        return 0;
    }

    if(!instance->current_dag->joined)
    {
        PRINTF("RPL: No DAG in the instance\n");
        return 1;
    }

    down = 0;
    if(UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_DOWN)
    {
        down = 1;
    }

    sender_closer = UIP_EXT_HDR_OPT_RPL_BUF->senderrank < instance->current_dag->rank;

    PRINTF("RPL: Packet going %s, sender closer %d (%d < %d)\n", down == 1 ? "down" : "up",
            sender_closer,
            UIP_EXT_HDR_OPT_RPL_BUF->senderrank,
            instance->current_dag->rank
    );

    if((down && !sender_closer) || (!down && sender_closer))
    {
        PRINTF("RPL: Loop detected - senderrank: %d my-rank: %d sender_closer: %d\n",
                UIP_EXT_HDR_OPT_RPL_BUF->senderrank, instance->current_dag->rank,
                sender_closer);
        if(UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_RANK_ERR)
        {
            UIP_STAT_RPL_LOOP_ERROR_NUM_INC;
            PRINTF("RPL: Rank error signalled in RPL option!\n");
            /* Packet must be dropped and dio trickle timer reset, see RFC6550 - 11.2.2.2 */
            rpl_reset_dio_timer(instance);
            return 1;
        }
        PRINTF("RPL: Single error tolerated\n");
        /* RFC 6550 - 11.2.2.2: One inconsistency along the path is not considered a critical error
         and the packet may continue.  However, a second detection along the
         path of the same packet should not occur and the packet MUST be
         dropped. */
        UIP_STAT_RPL_WARNING_ERROR_NUM_INC;
        UIP_EXT_HDR_OPT_RPL_BUF->flags |= RPL_HDR_OPT_RANK_ERR;
        return 0;
    }

    PRINTF("RPL: Rank OK\n");

    return 0;
#endif //!NON_STORING_MODE_ROOT
}
/*
 int uip_rpl_verify_header(u16_t type, u16_t *uip_ext_opt_offset_ptr){
 u16_t uip_ext_opt_offset = *uip_ext_opt_offset_ptr;
 if (type == UIP_EXT_HDR_OPT_RPL){
 if (rpl_verify_header(*uip_ext_opt_offset_ptr)){
 PRINTF("RPL Option Error: Dropping Packet\n");
 return 1;
 }
 *uip_ext_opt_offset_ptr += CHAR_BIT_UNITS((UIP_EXT_HDR_OPT_BUF->len) + 2);
 return 0;
 }
 return -1;
 }
 */
int rpl_ipv6_verify_header(u16_t uip_ext_opt_offset)
{
    if (UIP_EXT_HDR_OPT_BUF->type == UIP_EXT_HDR_OPT_RPL)
    {
        if (rpl_verify_header(uip_ext_opt_offset))
        {
            PRINTF("RPL Option Error: Dropping Packet\n");
            return -1;
        }
        return 1;
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
#if !NON_STORING_MODE_ROOT
static void
set_rpl_opt(unsigned uip_ext_opt_offset)
{
    u16_t temp_len;

    memmove(UIP_HBHO_NEXT_BUF, UIP_EXT_BUF, CHAR_BIT_UNITS(rpl_ip_len) - UIP_IPH_LEN);
    memset(UIP_HBHO_BUF, 0, RPL_HOP_BY_HOP_LEN);
    UIP_HBHO_BUF->next = UIP_IP_BUF->proto;
    UIP_IP_BUF->proto = UIP_PROTO_HBHO;
    UIP_HBHO_BUF->len = BYTE_UNITS(RPL_HOP_BY_HOP_LEN) - 8;
    UIP_EXT_HDR_OPT_RPL_BUF->opt_type = UIP_EXT_HDR_OPT_RPL;
    UIP_EXT_HDR_OPT_RPL_BUF->opt_len = BYTE_UNITS(RPL_HDR_OPT_LEN);
    UIP_EXT_HDR_OPT_RPL_BUF->flags = 0;
    UIP_EXT_HDR_OPT_RPL_BUF->instance = 0;
    UIP_EXT_HDR_OPT_RPL_BUF->senderrank = 0;
    rpl_ip_len += BYTE_UNITS(RPL_HOP_BY_HOP_LEN);
    temp_len = UIP_IP_BUF->len1;
    UIP_IP_BUF->len1 += UIP_HBHO_BUF->len + 8;
    if(UIP_IP_BUF->len1 < temp_len)
    {
        UIP_IP_BUF->len0++;
    }
}
#endif
/*---------------------------------------------------------------------------*/
void
rpl_update_header_empty(void)
{
#if !NON_STORING_MODE_ROOT //Non storing mode Root will never insert or update a HBHO
    rpl_instance_t *instance;
    int uip_ext_opt_offset;
    int last_uip_ext_len;

    last_uip_ext_len = rpl_ip_ext_len;
    rpl_ip_ext_len = 0;
    uip_ext_opt_offset = CHAR_BIT_UNITS(2);

    PRINTF("RPL: Verifying the presence of the RPL header option\n");

    switch(UIP_IP_BUF->proto)
    {
        case UIP_PROTO_HBHO:
        if(UIP_HBHO_BUF->len != BYTE_UNITS(RPL_HOP_BY_HOP_LEN) - 8)
        {
            PRINTF("RPL: Non RPL Hop-by-hop options support not implemented\n");
            rpl_ip_ext_len = last_uip_ext_len;
            return;
        }
        instance = rpl_get_instance(UIP_EXT_HDR_OPT_RPL_BUF->instance);
        if(instance == NULL || !instance->used || !instance->current_dag->joined)
        {
            PRINTF("RPL: Unable to add hop-by-hop extension header: incorrect instance\n");
            return;
        }
        break;
        default:
        PRINTF("RPL: No hop-by-hop option found, creating it\n");
        if(CHAR_BIT_UNITS(rpl_ip_len) + RPL_HOP_BY_HOP_LEN > RPL_BUFSIZE)
        {
            PRINTF("RPL: Packet too long: impossible to add hop-by-hop option\n");
            rpl_ip_ext_len = last_uip_ext_len;
            return;
        }
        set_rpl_opt(uip_ext_opt_offset);
        rpl_ip_ext_len = last_uip_ext_len + RPL_HOP_BY_HOP_LEN;
        return;
    }

    switch(UIP_EXT_HDR_OPT_BUF->type)
    {
        case UIP_EXT_HDR_OPT_RPL:
        PRINTF("RPL: Updating RPL option\n");
        UIP_EXT_HDR_OPT_RPL_BUF->senderrank = instance->current_dag->rank;

        /* Check the direction of the down flag, as per Section 11.2.2.3,
         which states that if a packet is going down it should in
         general not go back up again. If this happens, a
         RPL_HDR_OPT_FWD_ERR should be flagged. */
        if((UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_DOWN))
        {
            if(uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr) == NULL)
            {
                UIP_EXT_HDR_OPT_RPL_BUF->flags |= RPL_HDR_OPT_FWD_ERR;
                PRINTF("RPL forwarding error\n");
                UIP_STAT_RPL_ADDING_FORWARDING_ERROR_NUM_INC;
            }
        }
        else
        {
            /* Set the down extension flag correctly as described in Section
             11.2 of RFC6550. If the packet progresses along a DAO route,
             the down flag should be set. */
            if(uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr) == NULL)
            {
                /* No route was found, so this packet will go towards the RPL
                 root. If so, we should not set the down flag. */
                UIP_EXT_HDR_OPT_RPL_BUF->flags &= ~RPL_HDR_OPT_DOWN;
                PRINTF("RPL option going up\n");
            }
            else
            {
                /* A DAO route was found so we set the down flag. */
                UIP_EXT_HDR_OPT_RPL_BUF->flags |= RPL_HDR_OPT_DOWN;
                PRINTF("RPL option going down\n");
            }
        }

        rpl_ip_ext_len = last_uip_ext_len;
        return;
        default:
        PRINTF("RPL: Multi Hop-by-hop options not implemented\n");
        rpl_ip_ext_len = last_uip_ext_len;
        return;
    }
#endif
}
/*---------------------------------------------------------------------------*/
int
rpl_update_header_final(uip_ipaddr_t *addr)
{
#if !NON_STORING_MODE_ROOT
    rpl_parent_t *parent;
#endif
    int uip_ext_opt_offset;
    int last_uip_ext_len;

    last_uip_ext_len = rpl_ip_ext_len;
    rpl_ip_ext_len = 0;
    uip_ext_opt_offset = CHAR_BIT_UNITS(2);

    if(UIP_IP_BUF->proto == UIP_PROTO_HBHO)
    {
        if(UIP_HBHO_BUF->len != BYTE_UNITS(RPL_HOP_BY_HOP_LEN) - 8)
        {
            PRINTF("RPL: Non RPL Hop-by-hop options support not implemented\n");
            rpl_ip_ext_len = last_uip_ext_len;
            return 0;
        }

        if(UIP_EXT_HDR_OPT_BUF->type == UIP_EXT_HDR_OPT_RPL)
        {
#if NON_STORING_MODE_ROOT
            //Always remove it! Do not need a HBHO if we are the Root.
            rpl_remove_header();
            return 0;//Try to send the packet anyway
#else

            if(UIP_EXT_HDR_OPT_RPL_BUF->senderrank == 0)
            {
                PRINTF("RPL: Updating RPL option\n");
                if(default_instance == NULL || !default_instance->used || !default_instance->current_dag->joined)
                {
                    PRINTF("RPL: Unable to add hop-by-hop extension header: incorrect default instance\n");
                    return 1;
                }
                parent = rpl_find_parent(default_instance->current_dag, addr);
                if(parent == NULL || parent != parent->dag->preferred_parent)
                {
#if (RPL_ROUTING_MODE == RPL_NO_ROUTING_MODE)
                    //In source routing mode we only use the header if packet goes UP (for nodes)
                    rpl_remove_header();
                    return 0;//Try to send the packet anyway
#else
                    UIP_EXT_HDR_OPT_RPL_BUF->flags = RPL_HDR_OPT_DOWN;
#endif
                }
                UIP_EXT_HDR_OPT_RPL_BUF->instance = default_instance->instance_id;
                UIP_EXT_HDR_OPT_RPL_BUF->senderrank = default_instance->current_dag->rank;
                rpl_ip_ext_len = last_uip_ext_len;
            }

#endif //!NON_STORING_MODE_ROOT
        }
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
uint16_t
rpl_invert_header(void)
{
    u16_t uip_ext_opt_offset;
    u16_t last_uip_ext_len;

    last_uip_ext_len = rpl_ip_ext_len;
    rpl_ip_ext_len = 0;
    uip_ext_opt_offset = CHAR_BIT_UNITS(2);

    PRINTF("RPL: Verifying the presence of the RPL header option\n");
    switch(UIP_IP_BUF->proto)
    {
        case UIP_PROTO_HBHO:
        break;
        default:
        PRINTF("RPL: No hop-by-hop Option found\n");
        rpl_ip_ext_len = last_uip_ext_len;
        return 0;
    }

    switch (UIP_EXT_HDR_OPT_BUF->type)
    {
        case UIP_EXT_HDR_OPT_RPL:
        PRINTF("RPL: Updating RPL option (switching direction)\n");
        UIP_EXT_HDR_OPT_RPL_BUF->flags &= RPL_HDR_OPT_DOWN;
        UIP_EXT_HDR_OPT_RPL_BUF->flags ^= RPL_HDR_OPT_DOWN;
        UIP_EXT_HDR_OPT_RPL_BUF->senderrank = rpl_get_instance(UIP_EXT_HDR_OPT_RPL_BUF->instance)->current_dag->rank;
        rpl_ip_ext_len = last_uip_ext_len;
        return RPL_HOP_BY_HOP_LEN;
        default:
        PRINTF("RPL: Multi Hop-by-hop options not implemented\n");
        rpl_ip_ext_len = last_uip_ext_len;
        return 0;
    }
}
/*---------------------------------------------------------------------------*/
void
rpl_insert_header(void)
{
#if !NON_STORING_MODE_ROOT //Root node for non-storing mode will never insert a HBHO header
    if(default_instance != NULL && !uip_is_addr_mcast(&UIP_IP_BUF->destipaddr))
    {
        rpl_update_header_empty();
    }
#endif
}
/*---------------------------------------------------------------------------*/
int rpl_ipv6_hbho_header_extension(int action, void *data)
{
    uip_ipaddr_t *ipaddr;
    int opt_offset;
    switch(action)
    {
        case RPL_HBHO_HDR_UPDATE_EMPTY:
        rpl_update_header_empty();
        return 0;
        case RPL_HBHO_HDR_UPDATE_NEXT_HOP:
        ipaddr = (uip_ipaddr_t *) data;
        return rpl_update_header_final(ipaddr);
        case RPL_HBHO_HDR_VERIFY:
        opt_offset = *((int*)data);
        return rpl_ipv6_verify_header(opt_offset);
        case RPL_HBHO_HDR_INSERT:
        rpl_insert_header();
        return 0;
        case RPL_HBHO_HDR_INVERT:
        return rpl_invert_header();
    }
    return 0;
}

#endif /* UIP_CONF_IPV6 */

#endif
