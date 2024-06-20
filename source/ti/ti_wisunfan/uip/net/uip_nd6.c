/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         Neighbor discovery (RFC 4861)
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 */

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
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
#include "net/uip-icmp6.h"
#include "net/uip-nd6.h"
#include "net/uip-ds6.h"
#include "lib/random.h"

#include "uip_rpl_process.h"

#if UIP_CONF_IPV6
/*------------------------------------------------------------------*/

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "uip-debug.h"

#if UIP_LOGGING
#include <stdio.h>
void uip_log(char *msg);

#define UIP_LOG(m) uip_log(m)
#else
#define UIP_LOG(m)
#endif /* UIP_LOGGING == 1 */

/*------------------------------------------------------------------*/
/** @{ */
/** \name Pointers to the header structures.
 *  All pointers except UIP_IP_BUF depend on uip_ext_len, which at
 *  packet reception, is the total length of the extension headers.
 *  
 *  The pointer to ND6 options header also depends on nd6_opt_offset,
 *  which we set in each function.
 *
 *  Care should be taken when manipulating these buffers about the
 *  value of these length variables
 */

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])  /**< Pointer to IP header */
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])  /**< Pointer to ICMP header*/
/**@{  Pointers to messages just after icmp header */
#define UIP_ND6_RS_BUF            ((uip_nd6_rs *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_RA_BUF            ((uip_nd6_ra *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_NS_BUF            ((uip_nd6_ns *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_NA_BUF            ((uip_nd6_na *)&uip_buf[uip_l2_l3_icmp_hdr_len])
/** @} */
/** Pointer to ND option */
#define UIP_ND6_OPT_HDR_BUF  ((uip_nd6_opt_hdr *)&uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset])
#define UIP_ND6_OPT_PREFIX_BUF ((uip_nd6_opt_prefix_info *)&uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset])
#define UIP_ND6_OPT_MTU_BUF ((uip_nd6_opt_mtu *)&uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset])
/** @} */

/* TODO: CHECK
 #if WITH_ULPSMAC
 #define UIP_ND6_IS_LLADDR_NULL(m) \
  ( ((m)->addr[0] == 0x00) && \
    ((m)->addr[1] == 0x00) && \
    ((m)->addr[2] == 0x00) && \
    ((m)->addr[3] == 0x00) && \
    ((m)->addr[4] == 0x00) && \
    ((m)->addr[5] == 0x00) && \
    ((m)->addr[6] == 0x00) && \
    ((m)->addr[7] == 0x00) ) 
 #endif
 */

static u16_t nd6_opt_offset; /** Offset from the end of the icmpv6 header to the option in uip_buf*/
static char *nd6_opt_llao; /**  Pointer to llao option in uip_buf */

#if !UIP_CONF_ROUTER            // TBD see if we move it to ra_input
static uip_nd6_opt_prefix_info *nd6_opt_prefix_info; /**  Pointer to prefix information option in uip_buf */
static uip_ipaddr_t ipaddr;
static uip_ds6_prefix_t *prefix; /**  Pointer to a prefix list entry */
#endif

static uip_ds6_nbr_t *nbr; /**  Pointer to a nbr cache entry*/
static uip_ds6_defrt_t *defrt; /**  Pointer to a router list entry */
static uip_ds6_addr_t *addr; /**  Pointer to an interface address */

/*------------------------------------------------------------------*/
/* create a llao */
static void create_llao(char *llao, u16_t type)
{
    //llao[UIP_ND6_OPT_TYPE_OFFSET] = type;
    pltfrm_byte_set(llao, UIP_ND6_OPT_TYPE_OFFSET, type);
    //llao[UIP_ND6_OPT_LEN_OFFSET] = UIP_ND6_OPT_LLAO_LEN >> 3;
    pltfrm_byte_set(llao, UIP_ND6_OPT_LEN_OFFSET,
                    BYTE_UNITS(UIP_ND6_OPT_LLAO_LEN) >> 3);
    /*
     memcpy(&llao[UIP_ND6_OPT_DATA_OFFSET], &uip_lladdr, UIP_LLADDR_LEN);
     */
    pltfrm_byte_memcpy(&llao[UIP_ND6_OPT_DATA_OFFSET], 0, &uip_lladdr, 0,
                       BYTE_UNITS(UIP_LLADDR_LEN));
    /* padding on some */
    pltfrm_byte_memset(
            &llao[UIP_ND6_OPT_DATA_OFFSET], BYTE_UNITS(UIP_LLADDR_LEN), 0,
            BYTE_UNITS(UIP_ND6_OPT_LLAO_LEN) - 2 - BYTE_UNITS(UIP_LLADDR_LEN));
}

/*------------------------------------------------------------------*/

void uip_nd6_ns_input(void)
{
    u16_t flags;
    PRINTF("Received NS from ");PRINT6ADDR(&UIP_IP_BUF->srcipaddr);PRINTF(" to ");PRINT6ADDR(&UIP_IP_BUF->destipaddr);PRINTF(" with target address");PRINT6ADDR((uip_ipaddr_t *) (&UIP_ND6_NS_BUF->tgtipaddr));UIP_STAT(++uip_stat.nd6.recv);

#if UIP_CONF_IPV6_CHECKS
    if ((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT)
            || (uip_is_addr_mcast(&UIP_ND6_NS_BUF->tgtipaddr))
            || (UIP_ICMP_BUF->icode != 0))
    {
        LOG_INFO("NS received is bad\n");
        goto discard;
    }
#endif /* UIP_CONF_IPV6_CHECKS */

    /* Options processing */
    nd6_opt_llao = NULL;
    nd6_opt_offset = UIP_ND6_NS_LEN;
    while (uip_l3_icmp_hdr_len + nd6_opt_offset < CHAR_BIT_UNITS(uip_len))
    {
#if UIP_CONF_IPV6_CHECKS
        if (UIP_ND6_OPT_HDR_BUF->len == 0)
        {
            LOG_INFO("NS received is bad\n");
            goto discard;
        }
#endif /* UIP_CONF_IPV6_CHECKS */
        switch (UIP_ND6_OPT_HDR_BUF->type)
        {
        case UIP_ND6_OPT_SLLAO:
            nd6_opt_llao = &uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset];
#if UIP_CONF_IPV6_CHECKS
            /* There must be NO option in a DAD NS */
            if (uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr))
            {
                LOG_INFO("NS received is bad\n");
                goto discard;
            }
            else
            {
#endif /*UIP_CONF_IPV6_CHECKS */
                nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->srcipaddr);
                if (nbr == NULL)
                {
                    uip_ds6_nbr_add(
                            &UIP_IP_BUF->srcipaddr,
                            (uip_lladdr_t *) &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET],
                            0, NBR_STALE);
                }
                else
                {
                    uip_lladdr_t *lladdr = uip_ds6_nbr_get_ll(nbr);
                    if (pltfrm_byte_memcmp(&nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0,
                            lladdr, 0, BYTE_UNITS(UIP_LLADDR_LEN)) != 0)
                    {
                        //if this is true, the ll address we have in the neighbor table is 0, not resolved yet. We need to check if it is needed to fusion this entry with an existing one
                        uip_ds6_nbr_t* nbr_existing_entry =
                                uip_ds6_nbr_ll_lookup(
                                        (uip_lladdr_t*) &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET]);
                        //If this entry already existed, but the neighbor elements are not the same, we fusion them
                        if (nbr_existing_entry)
                        {
                            if (nbr != nbr_existing_entry)
                            {
                                //remove the nbr with ll address equal 0 (null)
                                uip_ds6_nbr_rm(nbr);
                                //add the new entry (the only values that matter are IP address and LL address because the entry exists so only a new possible IP address for the neighbor will be added)
                                nbr =
                                        uip_ds6_nbr_add(
                                                &UIP_IP_BUF->srcipaddr,
                                                (uip_lladdr_t*) &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET],
                                                0, NBR_STALE);
                            } //else: this cannot happen
                        }
                        else
                        {
                            //if there is no existing item in the table with the ll address indicated, just update lladdr value
                            pltfrm_byte_memcpy(
                                    lladdr, 0,
                                    &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0,
                                    BYTE_UNITS(UIP_LLADDR_LEN));
                            nbr->state = NBR_STALE;
                        }
                    }
                    else
                    {
                        if (nbr->state == NBR_INCOMPLETE)
                        {
                            nbr->state = NBR_STALE;
                        }
                    }
                }
#if UIP_CONF_IPV6_CHECKS
            }
#endif /*UIP_CONF_IPV6_CHECKS */
            break;
        default:
            LOG_INFO("ND option not supported in NS");
            break;
        }
        nd6_opt_offset += CHAR_BIT_UNITS(UIP_ND6_OPT_HDR_BUF->len << 3);
    }

    addr = uip_ds6_addr_lookup(&UIP_ND6_NS_BUF->tgtipaddr);
    if (addr != NULL)
    {
#if UIP_ND6_DEF_MAXDADNS > 0
        if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr))
        {
            /* DAD CASE */
#if UIP_CONF_IPV6_CHECKS
            if(!uip_is_addr_solicited_node(&UIP_IP_BUF->destipaddr))
            {
                LOG_INFO("NS received is bad\n");
                goto discard;
            }
#endif /* UIP_CONF_IPV6_CHECKS */
            if(addr->state != ADDR_TENTATIVE)
            {
                uip_create_linklocal_allnodes_mcast(&UIP_IP_BUF->destipaddr);
                uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
                flags = UIP_ND6_NA_FLAG_OVERRIDE;
                goto create_na;
            }
            else
            {
                /** \todo if I sent a NS before him, I win */
                uip_ds6_dad_failed(addr);
                goto discard;
            }
#else /* UIP_ND6_DEF_MAXDADNS > 0 */
        if (uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr))
        {
            /* DAD CASE */
            goto discard;
#endif /* UIP_ND6_DEF_MAXDADNS > 0 */
        }
#if UIP_CONF_IPV6_CHECKS
        if (uip_ds6_is_my_addr(&UIP_IP_BUF->srcipaddr))
        {
            /**
             * \NOTE do we do something here? we both are using the same address.
             * If we are doing dad, we could cancel it, though we should receive a
             * NA in response of DAD NS we sent, hence DAD will fail anyway. If we
             * were not doing DAD, it means there is a duplicate in the network!
             */
            LOG_INFO("NS received is bad\n");
            goto discard;
        }
#endif /*UIP_CONF_IPV6_CHECKS */

        /* Address resolution case */
        if (uip_is_addr_solicited_node(&UIP_IP_BUF->destipaddr))
        {
            uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
            uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_ND6_NS_BUF->tgtipaddr);
            flags = UIP_ND6_NA_FLAG_SOLICITED | UIP_ND6_NA_FLAG_OVERRIDE;
            goto create_na;
        }

        /* NUD CASE */
        if (uip_ds6_addr_lookup(&UIP_IP_BUF->destipaddr) == addr)
        {
            uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
            uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_ND6_NS_BUF->tgtipaddr);
            flags = UIP_ND6_NA_FLAG_SOLICITED | UIP_ND6_NA_FLAG_OVERRIDE;
            goto create_na;
        }
        else
        {
#if UIP_CONF_IPV6_CHECKS
            LOG_INFO("NS received is bad\n");
            goto discard;
#endif /* UIP_CONF_IPV6_CHECKS */
        }
    }
    else
    {
        goto discard;
    }

    create_na:
    /* If the node is a router it should set R flag in NAs */
#if UIP_CONF_ROUTER
    flags = flags | UIP_ND6_NA_FLAG_ROUTER;
#endif
    uip_ext_len = 0;
    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 0;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->len0 = 0; /* length will not be more than 255 */
    UIP_IP_BUF->len1 = BYTE_UNITS(
            UIP_ICMPH_LEN + UIP_ND6_NA_LEN + UIP_ND6_OPT_LLAO_LEN);
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;

    UIP_ICMP_BUF->type = ICMP6_NA;
    UIP_ICMP_BUF->icode = 0;

    memset(UIP_ND6_NA_BUF, 0, sizeof(uip_nd6_na));

    UIP_ND6_NA_BUF->flagsreserved = flags;
    memcpy(&UIP_ND6_NA_BUF->tgtipaddr, &addr->ipaddr, sizeof(uip_ipaddr_t));

    create_llao(&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_NA_LEN],
    UIP_ND6_OPT_TLLAO);

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

    uip_len= BYTE_UNITS(UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NA_LEN + UIP_ND6_OPT_LLAO_LEN);

    UIP_STAT(++uip_stat.nd6.sent);

    UIP_STAT_TX_NA_NUM_INC;
    UIP_STAT_TX_ICMP_NUM_INC;

    PRINTF("Sending NA to");PRINT6ADDR(&UIP_IP_BUF->destipaddr);PRINTF("from");PRINT6ADDR(&UIP_IP_BUF->srcipaddr);PRINTF("with target address");PRINT6ADDR(&UIP_ND6_NA_BUF->tgtipaddr);PRINTF("");
    return;

    discard:
    uip_len= 0;
    return;
}

/*------------------------------------------------------------------*/
void uip_nd6_ns_output(uip_ipaddr_t * src, uip_ipaddr_t * dest,
                       uip_ipaddr_t * tgt)
{
    uip_ext_len = 0;
    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 0;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;

    if (dest == NULL)
    {
        uip_create_solicited_node(tgt, &UIP_IP_BUF->destipaddr);
    }
    else
    {
        uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, dest);
    }
    UIP_ICMP_BUF->type = ICMP6_NS;
    UIP_ICMP_BUF->icode = 0;
    UIP_ND6_NS_BUF->reserved1 = 0;
    UIP_ND6_NS_BUF->reserved2 = 0;
    uip_ipaddr_copy((uip_ipaddr_t *) &UIP_ND6_NS_BUF->tgtipaddr, tgt);
    UIP_IP_BUF->len0 = 0; /* length will not be more than 255 */
    /*
     * check if we add a SLLAO option: for DAD, MUST NOT, for NUD, MAY
     * (here yes), for Address resolution , MUST
     */
    if (!(uip_ds6_is_my_addr(tgt)))
    {
        if (src != NULL)
        {
            uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, src);
        }
        else
        {
            uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
        }
        if (uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr))
        {
            LOG_INFO("Dropping NS due to no suitable source address\n");
            uip_len= 0;
            return;
        }
        UIP_IP_BUF->len1 = BYTE_UNITS(
                UIP_ICMPH_LEN + UIP_ND6_NS_LEN + UIP_ND6_OPT_LLAO_LEN);

        create_llao(&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_NS_LEN],
        UIP_ND6_OPT_SLLAO);

        uip_len= BYTE_UNITS(UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NS_LEN + UIP_ND6_OPT_LLAO_LEN);
    }
    else
    {
        uip_create_unspecified(&UIP_IP_BUF->srcipaddr);
        UIP_IP_BUF->len1 = BYTE_UNITS(UIP_ICMPH_LEN + UIP_ND6_NS_LEN);
        uip_len = BYTE_UNITS(UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NS_LEN);
    }

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

    UIP_STAT(++uip_stat.nd6.sent);

    UIP_STAT_TX_NS_NUM_INC;
    UIP_STAT_TX_ICMP_NUM_INC;

    PRINTF("Sending NS to");PRINT6ADDR(&UIP_IP_BUF->destipaddr);PRINTF("from");PRINT6ADDR(&UIP_IP_BUF->srcipaddr);PRINTF("with target address");PRINT6ADDR(tgt);PRINTF("");

    return;
}

/*------------------------------------------------------------------*/
void uip_nd6_na_input(void)
{
    char is_llchange;
    char is_router;
    char is_solicited;
    char is_override;

    PRINTF("Received NA from");PRINT6ADDR(&UIP_IP_BUF->srcipaddr);PRINTF("to");PRINT6ADDR(&UIP_IP_BUF->destipaddr);PRINTF("with target address");PRINT6ADDR((uip_ipaddr_t *) (&UIP_ND6_NA_BUF->tgtipaddr));PRINTF("");UIP_STAT(++uip_stat.nd6.recv);

    /*
     * booleans. the three last one are not 0 or 1 but 0 or 0x80, 0x40, 0x20
     * but it works. Be careful though, do not use tests such as is_router == 1
     */
    is_llchange = 0;
    is_router = ((UIP_ND6_NA_BUF->flagsreserved & UIP_ND6_NA_FLAG_ROUTER));
    is_solicited =
            ((UIP_ND6_NA_BUF->flagsreserved & UIP_ND6_NA_FLAG_SOLICITED));
    is_override = ((UIP_ND6_NA_BUF->flagsreserved & UIP_ND6_NA_FLAG_OVERRIDE));

#if UIP_CONF_IPV6_CHECKS
    if ((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) || (UIP_ICMP_BUF->icode != 0)
            || (uip_is_addr_mcast(&UIP_ND6_NA_BUF->tgtipaddr))
            || (is_solicited && uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)))
    {
        LOG_INFO("NA received is bad\n");
        goto discard;
    }
#endif /*UIP_CONF_IPV6_CHECKS */

    /* Options processing: we handle TLLAO, and must ignore others */
    nd6_opt_offset = UIP_ND6_NA_LEN;
    nd6_opt_llao = NULL;
    while (uip_l3_icmp_hdr_len + nd6_opt_offset < CHAR_BIT_UNITS(uip_len))
    {
#if UIP_CONF_IPV6_CHECKS
        if (UIP_ND6_OPT_HDR_BUF->len == 0)
        {
            LOG_INFO("NA received is bad\n");
            goto discard;
        }
#endif /*UIP_CONF_IPV6_CHECKS */
        switch (UIP_ND6_OPT_HDR_BUF->type)
        {
        case UIP_ND6_OPT_TLLAO:
            nd6_opt_llao = (char *) UIP_ND6_OPT_HDR_BUF;
            break;
        default:
            LOG_INFO("ND option not supported in NA\n");
            break;
        }
        nd6_opt_offset += CHAR_BIT_UNITS(UIP_ND6_OPT_HDR_BUF->len << 3);
    }
    addr = uip_ds6_addr_lookup(&UIP_ND6_NA_BUF->tgtipaddr);
    /* Message processing, including TLLAO if any */
    if (addr != NULL)
    {
#if UIP_ND6_DEF_MAXDADNS > 0
        if(addr->state == ADDR_TENTATIVE)
        {
            uip_ds6_dad_failed(addr);
        }
#endif /*UIP_ND6_DEF_MAXDADNS > 0 */
        LOG_INFO("NA received is bad\n");
        goto discard;
    }
    else
    {
        uip_lladdr_t *lladdr;
        nbr = uip_ds6_nbr_lookup(&UIP_ND6_NA_BUF->tgtipaddr);
        lladdr = uip_ds6_nbr_get_ll(nbr);
        if (nbr == NULL)
        {
            goto discard;
        }
        if (nd6_opt_llao != 0)
        {
            is_llchange = pltfrm_byte_memcmp(
                    &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0, (void * )lladdr,
                    0, BYTE_UNITS(UIP_LLADDR_LEN));
        }
        if (nbr->state == NBR_INCOMPLETE)
        {
            if (nd6_opt_llao == NULL)
            {
                goto discard;
            }
            //need to check if the ll address of the NA received exists in our nbr table.
            uip_ds6_nbr_t* nbr_existing_entry = uip_ds6_nbr_ll_lookup(
                    (uip_lladdr_t*) &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET]);
            //If this entry already exists, fusion the new ip address with the old entry
            if (nbr_existing_entry)
            {
                //remove the nbr with ll address equal 0 (null)
                uip_ds6_nbr_rm(nbr);
                //add the new entry (the only values that matter are IP address and LL address because the entry exists so only a new possible IP address for the neighbor will be added)
                nbr = uip_ds6_nbr_add(
                        &UIP_ND6_NA_BUF->tgtipaddr,
                        (uip_lladdr_t*) &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET],
                        is_router, NBR_STALE);
            }
            else
            { //The entry with this MAC did not exist, so replace the neighbor table entry with the correct MAC address
                pltfrm_byte_memcpy(lladdr, 0,
                                   &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0,
                                   BYTE_UNITS(UIP_LLADDR_LEN));
            }
            if (is_solicited)
            {
                nbr->state = NBR_REACHABLE;
                nbr->nscount = 0;

                /* reachable time is stored in ms */
                stimer_set(&(nbr->reachable), uip_ds6_if.reachable_time / 1000);

            }
            else
            {
                nbr->state = NBR_STALE;
            }
            nbr->isrouter = is_router;
        }
        else
        {
            if (!is_override && is_llchange)
            {
                if (nbr->state == NBR_REACHABLE)
                {
                    nbr->state = NBR_STALE;
                }
                goto discard;
            }
            else
            {
                if (is_override
                        || (!is_override && nd6_opt_llao != 0 && !is_llchange)
                        || nd6_opt_llao == 0)
                {
                    if (nd6_opt_llao != 0)
                    {
                        pltfrm_byte_memcpy(
                                lladdr, 0,
                                &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0,
                                BYTE_UNITS(UIP_LLADDR_LEN));
                    }
                    if (is_solicited)
                    {
                        nbr->state = NBR_REACHABLE;
                        /* reachable time is stored in ms */
                        stimer_set(&(nbr->reachable),
                                   uip_ds6_if.reachable_time / 1000);
                    }
                    else
                    {
                        if (nd6_opt_llao != 0 && is_llchange)
                        {
                            nbr->state = NBR_STALE;
                        }
                    }
                }
            }
            if (nbr->isrouter && !is_router)
            {
                defrt = uip_ds6_defrt_lookup(&UIP_IP_BUF->srcipaddr);
                if (defrt != NULL)
                {
                    uip_ds6_defrt_rm(defrt);
                }
            }
            nbr->isrouter = is_router;
        }
    }
#if UIP_CONF_IPV6_QUEUE_PKT
    /* The nbr is now reachable, check if we had buffered a pkt for it */
    /*if(nbr->queue_buf_len != 0) {
     uip_len = nbr->queue_buf_len;
     memcpy(UIP_IP_BUF, nbr->queue_buf, uip_len);
     nbr->queue_buf_len = 0;
     return;
     }*/
    if(uip_packetqueue_buflen(&nbr->packethandle) != 0)
    {
        uip_len = uip_packetqueue_buflen(&nbr->packethandle);
        memcpy(UIP_IP_BUF, uip_packetqueue_buf(&nbr->packethandle), uip_len);
        uip_packetqueue_free(&nbr->packethandle);
        return;
    }

#endif /*UIP_CONF_IPV6_QUEUE_PKT */

    discard:
    uip_len= 0;
    return;
}

#if UIP_CONF_ROUTER
#if UIP_ND6_SEND_RA
#error no support
/*---------------------------------------------------------------------------*/
void
uip_nd6_rs_input(void)
{

    LOG_INFO("Received RS from");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    LOG_INFO("to");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    LOG_INFO("\n");
    UIP_STAT(++uip_stat.nd6.recv);

#if UIP_CONF_IPV6_CHECKS
    /*
     * Check hop limit / icmp code
     * target address must not be multicast
     * if the NA is solicited, dest must not be multicast
     */
    if((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) || (UIP_ICMP_BUF->icode != 0))
    {
        LOG_INFO("RS received is bad\n");
        goto discard;
    }
#endif /*UIP_CONF_IPV6_CHECKS */

    /* Only valid option is Source Link-Layer Address option any thing
     else is discarded */
    nd6_opt_offset = UIP_ND6_RS_LEN;
    nd6_opt_llao = NULL;

    while(uip_l3_icmp_hdr_len + nd6_opt_offset < uip_len)
    {
#if UIP_CONF_IPV6_CHECKS
        if(UIP_ND6_OPT_HDR_BUF->len == 0)
        {
            LOG_INFO("RS received is bad\n");
            goto discard;
        }
#endif /*UIP_CONF_IPV6_CHECKS */
        switch (UIP_ND6_OPT_HDR_BUF->type)
        {
            case UIP_ND6_OPT_SLLAO:
            nd6_opt_llao = (u8_t *)UIP_ND6_OPT_HDR_BUF;
            break;
            default:
            LOG_INFO("ND option not supported in RS\n");
            break;
        }
        nd6_opt_offset += (UIP_ND6_OPT_HDR_BUF->len << 3);
    }
    /* Options processing: only SLLAO */
    if(nd6_opt_llao != NULL)
    {
#if UIP_CONF_IPV6_CHECKS
        if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr))
        {
            LOG_INFO("RS received is bad\n");
            goto discard;
        }
        else
        {
#endif /*UIP_CONF_IPV6_CHECKS */
            if((nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->srcipaddr)) == NULL)
            {
                /* we need to add the neighbor */
                uip_ds6_nbr_add(&UIP_IP_BUF->srcipaddr,
                        (uip_lladdr_t *)&nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0, NBR_STALE);
            }
            else
            {
                /* If LL address changed, set neighbor state to stale */
                if(pltfrm_byte_memcmp(&nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0,
                                uip_ds6_nbr_get_ll(nbr), 0, BYTE_UNITS(UIP_LLADDR_LEN)) != 0)
                {
                    uip_ds6_nbr_t nbr_data = *nbr;
                    uip_ds6_nbr_rm(nbr);
                    nbr = uip_ds6_nbr_add(&UIP_IP_BUF->srcipaddr,
                            (uip_lladdr_t *)&nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0, NBR_STALE);
                    nbr->reachable = nbr_data.reachable;
                    nbr->sendns = nbr_data.sendns;
                    nbr->nscount = nbr_data.nscount;
                }
                nbr->isrouter = 0;
            }
#if UIP_CONF_IPV6_CHECKS
        }
#endif /*UIP_CONF_IPV6_CHECKS */
    }

    /* Schedule a sollicited RA */
    uip_ds6_send_ra_sollicited();

    discard:
    uip_len = 0;
    return;
}

/*---------------------------------------------------------------------------*/
void
uip_nd6_ra_output(uip_ipaddr_t * dest)
{

    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 0;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;

    if(dest == NULL)
    {
        uip_create_linklocal_allnodes_mcast(&UIP_IP_BUF->destipaddr);
    }
    else
    {
        /* For sollicited RA */
        uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, dest);
    }
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

    UIP_ICMP_BUF->type = ICMP6_RA;
    UIP_ICMP_BUF->icode = 0;

    UIP_ND6_RA_BUF->cur_ttl = uip_ds6_if.cur_hop_limit;

    UIP_ND6_RA_BUF->flags_reserved =
    (UIP_ND6_M_FLAG << 7) | (UIP_ND6_O_FLAG << 6);

    UIP_ND6_RA_BUF->router_lifetime = uip_htons(UIP_ND6_ROUTER_LIFETIME);
    //UIP_ND6_RA_BUF->reachable_time = uip_htonl(uip_ds6_if.reachable_time);
    //UIP_ND6_RA_BUF->retrans_timer = uip_htonl(uip_ds6_if.retrans_timer);
    UIP_ND6_RA_BUF->reachable_time = 0;
    UIP_ND6_RA_BUF->retrans_timer = 0;

    uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_RA_LEN;
    nd6_opt_offset = UIP_ND6_RA_LEN;

#if !UIP_CONF_ROUTER
    /* Prefix list */
    for(prefix = uip_ds6_prefix_list;
            prefix < uip_ds6_prefix_list + UIP_DS6_PREFIX_NB; prefix++)
    {
        if((prefix->isused) && (prefix->advertise))
        {
            UIP_ND6_OPT_PREFIX_BUF->type = UIP_ND6_OPT_PREFIX_INFO;
            UIP_ND6_OPT_PREFIX_BUF->len = UIP_ND6_OPT_PREFIX_INFO_LEN / 8;
            UIP_ND6_OPT_PREFIX_BUF->preflen = prefix->length;
            UIP_ND6_OPT_PREFIX_BUF->flagsreserved1 = prefix->l_a_reserved;
            UIP_ND6_OPT_PREFIX_BUF->validlt = uip_htonl(prefix->vlifetime);
            UIP_ND6_OPT_PREFIX_BUF->preferredlt = uip_htonl(prefix->plifetime);
            UIP_ND6_OPT_PREFIX_BUF->reserved2 = 0;
            uip_ipaddr_copy(&(UIP_ND6_OPT_PREFIX_BUF->prefix), &(prefix->ipaddr));
            nd6_opt_offset += UIP_ND6_OPT_PREFIX_INFO_LEN;
            uip_len += UIP_ND6_OPT_PREFIX_INFO_LEN;
        }
    }
#endif /* !UIP_CONF_ROUTER */

    /* Source link-layer option */
    create_llao((uint8_t *)UIP_ND6_OPT_HDR_BUF, UIP_ND6_OPT_SLLAO);

    uip_len += UIP_ND6_OPT_LLAO_LEN;
    nd6_opt_offset += UIP_ND6_OPT_LLAO_LEN;

    /* MTU */
    UIP_ND6_OPT_MTU_BUF->type = UIP_ND6_OPT_MTU;
    UIP_ND6_OPT_MTU_BUF->len = UIP_ND6_OPT_MTU_LEN >> 3;
    UIP_ND6_OPT_MTU_BUF->reserved = 0;
    //UIP_ND6_OPT_MTU_BUF->mtu = uip_htonl(uip_ds6_if.link_mtu);
    UIP_ND6_OPT_MTU_BUF->mtu = uip_htonl(1500);

    uip_len += UIP_ND6_OPT_MTU_LEN;
    nd6_opt_offset += UIP_ND6_OPT_MTU_LEN;
    UIP_IP_BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
    UIP_IP_BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);

    /*ICMP checksum */
    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

    UIP_STAT(++uip_stat.nd6.sent);
    LOG_INFO("Sending RA to");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    LOG_INFO("from");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    LOG_INFO("\n");
    return;
}
#endif /* UIP_ND6_SEND_RA */
#endif /* UIP_CONF_ROUTER */

#if !UIP_CONF_ROUTER
/*---------------------------------------------------------------------------*/
void
uip_nd6_rs_output(void)
{
    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 0;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;
    uip_create_linklocal_allrouters_mcast(&UIP_IP_BUF->destipaddr);
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    UIP_ICMP_BUF->type = ICMP6_RS;
    UIP_ICMP_BUF->icode = 0;
    UIP_IP_BUF->len[0] = 0; /* length will not be more than 255 */

    if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr))
    {
        UIP_IP_BUF->len[1] = UIP_ICMPH_LEN + UIP_ND6_RS_LEN;
        uip_len = uip_l3_icmp_hdr_len + UIP_ND6_RS_LEN;
    }
    else
    {
        uip_len = uip_l3_icmp_hdr_len + UIP_ND6_RS_LEN + UIP_ND6_OPT_LLAO_LEN;
        UIP_IP_BUF->len[1] =
        UIP_ICMPH_LEN + UIP_ND6_RS_LEN + UIP_ND6_OPT_LLAO_LEN;

        create_llao(&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_RS_LEN],
                UIP_ND6_OPT_SLLAO);
    }

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

    UIP_STAT(++uip_stat.nd6.sent);
    LOG_INFO("Sendin RS to");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    LOG_INFO("from");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    LOG_INFO("\n");
    return;
}

/*---------------------------------------------------------------------------*/
void
uip_nd6_ra_input(void)
{
    LOG_INFO("Received RA from");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    LOG_INFO("to");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    LOG_INFO("\n");
    UIP_STAT(++uip_stat.nd6.recv);

#if UIP_CONF_IPV6_CHECKS
    if((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) ||
            (!uip_is_addr_link_local(&UIP_IP_BUF->srcipaddr)) ||
            (UIP_ICMP_BUF->icode != 0))
    {
        LOG_INFO("RA received is bad");
        goto discard;
    }
#endif /*UIP_CONF_IPV6_CHECKS */

    if(UIP_ND6_RA_BUF->cur_ttl != 0)
    {
        uip_ds6_if.cur_hop_limit = UIP_ND6_RA_BUF->cur_ttl;
        PRINTF("uip_ds6_if.cur_hop_limit ");  //, uip_ds6_if.cur_hop_limit);
        PRINTF(uip_ds6_if.cur_hop_limit);
    }

    if(UIP_ND6_RA_BUF->reachable_time != 0)
    {
        if(uip_ds6_if.base_reachable_time !=
                uip_ntohl(UIP_ND6_RA_BUF->reachable_time))
        {
            uip_ds6_if.base_reachable_time = uip_ntohl(UIP_ND6_RA_BUF->reachable_time);
            uip_ds6_if.reachable_time = uip_ds6_compute_reachable_time();
        }
    }
    if(UIP_ND6_RA_BUF->retrans_timer != 0)
    {
        uip_ds6_if.retrans_timer = uip_ntohl(UIP_ND6_RA_BUF->retrans_timer);
    }

    /* Options processing */
    nd6_opt_offset = UIP_ND6_RA_LEN;
    while(uip_l3_icmp_hdr_len + nd6_opt_offset < uip_len)
    {
        if(UIP_ND6_OPT_HDR_BUF->len == 0)
        {
            LOG_INFO("RA received is bad");
            goto discard;
        }
        switch (UIP_ND6_OPT_HDR_BUF->type)
        {
            case UIP_ND6_OPT_SLLAO:
            LOG_INFO("Processing SLLAO option in RA\n");
            nd6_opt_llao = (uint8_t *) UIP_ND6_OPT_HDR_BUF;
            nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->srcipaddr);
            if(nbr == NULL)
            {
                nbr = uip_ds6_nbr_add(&UIP_IP_BUF->srcipaddr,
                        (uip_lladdr_t *)&nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET],
                        1, NBR_STALE);
            }
            else
            {
                if(nbr->state == NBR_INCOMPLETE)
                {
                    nbr->state = NBR_STALE;
                }
                uip_lladdr_t *lladdr = uip_ds6_nbr_get_ll(nbr);
                if(pltfrm_byte_memcmp(&nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0,
                                lladdr, 0, BYTE_UNITS(UIP_LLADDR_LEN)) != 0)
                {
                    pltfrm_byte_memcpy(lladdr, 0, &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], 0,
                            BYTE_UNITS(UIP_LLADDR_LEN));
                    nbr->state = NBR_STALE;
                }
                nbr->isrouter = 1;
            }
            break;
            case UIP_ND6_OPT_MTU:
            LOG_INFO("Processing MTU option in RA\n");
            uip_ds6_if.link_mtu =
            uip_ntohl(((uip_nd6_opt_mtu *) UIP_ND6_OPT_HDR_BUF)->mtu);
            break;
            case UIP_ND6_OPT_PREFIX_INFO:
            LOG_INFO("Processing PREFIX option in RA\n");
            nd6_opt_prefix_info = (uip_nd6_opt_prefix_info *) UIP_ND6_OPT_HDR_BUF;
            if((uip_ntohl(nd6_opt_prefix_info->validlt) >=
                            uip_ntohl(nd6_opt_prefix_info->preferredlt))
                    && (!uip_is_addr_link_local(&nd6_opt_prefix_info->prefix)))
            {
                /* on-link flag related processing */
                if(nd6_opt_prefix_info->flagsreserved1 & UIP_ND6_RA_FLAG_ONLINK)
                {
                    prefix =
                    uip_ds6_prefix_lookup(&nd6_opt_prefix_info->prefix,
                            nd6_opt_prefix_info->preflen);
                    if(prefix == NULL)
                    {
                        if(nd6_opt_prefix_info->validlt != 0)
                        {
                            if(nd6_opt_prefix_info->validlt != UIP_ND6_INFINITE_LIFETIME)
                            {
                                prefix = uip_ds6_prefix_add(&nd6_opt_prefix_info->prefix,
                                        nd6_opt_prefix_info->preflen,
                                        uip_ntohl(nd6_opt_prefix_info->
                                                validlt));
                            }
                            else
                            {
                                prefix = uip_ds6_prefix_add(&nd6_opt_prefix_info->prefix,
                                        nd6_opt_prefix_info->preflen, 0);
                            }
                        }
                    }
                    else
                    {
                        switch (nd6_opt_prefix_info->validlt)
                        {
                            case 0:
                            uip_ds6_prefix_rm(prefix);
                            break;
                            case UIP_ND6_INFINITE_LIFETIME:
                            prefix->isinfinite = 1;
                            break;
                            default:
                            LOG_INFO("Updating timer of prefix");
                            PRINT6ADDR(&prefix->ipaddr);
                            LOG_INFO("new value %lu\n", uip_ntohl(nd6_opt_prefix_info->validlt));
                            stimer_set(&prefix->vlifetime,
                                    uip_ntohl(nd6_opt_prefix_info->validlt));
                            prefix->isinfinite = 0;
                            break;
                        }
                    }
                }
                /* End of on-link flag related processing */
                /* autonomous flag related processing */
                if((nd6_opt_prefix_info->flagsreserved1 & UIP_ND6_RA_FLAG_AUTONOMOUS)
                        && (nd6_opt_prefix_info->validlt != 0)
                        && (nd6_opt_prefix_info->preflen == UIP_DEFAULT_PREFIX_LEN))
                {

                    uip_ipaddr_copy(&ipaddr, &nd6_opt_prefix_info->prefix);
                    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
                    addr = uip_ds6_addr_lookup(&ipaddr);
                    if((addr != NULL) && (addr->type == ADDR_AUTOCONF))
                    {
                        if(nd6_opt_prefix_info->validlt != UIP_ND6_INFINITE_LIFETIME)
                        {
                            /* The processing below is defined in RFC4862 section 5.5.3 e */
                            if((uip_ntohl(nd6_opt_prefix_info->validlt) > 2 * 60 * 60) ||
                                    (uip_ntohl(nd6_opt_prefix_info->validlt) >
                                            stimer_remaining(&addr->vlifetime)))
                            {
                                LOG_INFO("Updating timer of address");
                                PRINT6ADDR(&addr->ipaddr);
                                LOG_INFO("new value %lu\n",
                                        uip_ntohl(nd6_opt_prefix_info->validlt));
                                stimer_set(&addr->vlifetime,
                                        uip_ntohl(nd6_opt_prefix_info->validlt));
                            }
                            else
                            {
                                stimer_set(&addr->vlifetime, 2 * 60 * 60);
                                LOG_INFO("Updating timer of address ");
                                PRINT6ADDR(&addr->ipaddr);
                                LOG_INFO("new value %lu\n", (unsigned long)(2 * 60 * 60));
                            }
                            addr->isinfinite = 0;
                        }
                        else
                        {
                            addr->isinfinite = 1;
                        }
                    }
                    else
                    {
                        if(uip_ntohl(nd6_opt_prefix_info->validlt) ==
                                UIP_ND6_INFINITE_LIFETIME)
                        {
                            uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
                        }
                        else
                        {
                            uip_ds6_addr_add(&ipaddr, uip_ntohl(nd6_opt_prefix_info->validlt),
                                    ADDR_AUTOCONF);
                        }
                    }
                }
                /* End of autonomous flag related processing */
            }
            break;
            default:
            LOG_INFO("ND option not supported in RA");
            break;
        }
        nd6_opt_offset += (UIP_ND6_OPT_HDR_BUF->len << 3);
    }

    defrt = uip_ds6_defrt_lookup(&UIP_IP_BUF->srcipaddr);
    if(UIP_ND6_RA_BUF->router_lifetime != 0)
    {
        if(nbr != NULL)
        {
            nbr->isrouter = 1;
        }
        if(defrt == NULL)
        {
            uip_ds6_defrt_add(&UIP_IP_BUF->srcipaddr,
                    (unsigned
                            long)(uip_ntohs(UIP_ND6_RA_BUF->router_lifetime)));
        }
        else
        {
            stimer_set(&(defrt->lifetime),
                    (unsigned long)(uip_ntohs(UIP_ND6_RA_BUF->router_lifetime)));
        }
    }
    else
    {
        if(defrt != NULL)
        {
            uip_ds6_defrt_rm(defrt);
        }
    }

#if UIP_CONF_IPV6_QUEUE_PKT
    /* If the nbr just became reachable (e.g. it was in NBR_INCOMPLETE state
     * and we got a SLLAO), check if we had buffered a pkt for it */
    /*  if((nbr != NULL) && (nbr->queue_buf_len != 0)) {
     uip_len = nbr->queue_buf_len;
     memcpy(UIP_IP_BUF, nbr->queue_buf, uip_len);
     nbr->queue_buf_len = 0;
     return;
     }*/
    if(nbr != NULL && uip_packetqueue_buflen(&nbr->packethandle) != 0)
    {
        uip_len = uip_packetqueue_buflen(&nbr->packethandle);
        memcpy(UIP_IP_BUF, uip_packetqueue_buf(&nbr->packethandle), uip_len);
        uip_packetqueue_free(&nbr->packethandle);
        return;
    }

#endif /*UIP_CONF_IPV6_QUEUE_PKT */

    discard:
    uip_len = 0;
    return;
}
#endif /* !UIP_CONF_ROUTER */

/** @} */
#endif /* UIP_CONF_IPV6 */
