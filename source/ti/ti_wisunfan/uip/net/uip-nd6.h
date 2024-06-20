/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         Neighbor discovery (RFC 4861)
 * \author Julien Abeille <jabeille@cisco.com>
 * \author Mathilde Durvy <mdurvy@cisco.com>
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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

#ifndef __UIP_ND6_H__
#define __UIP_ND6_H__

#include "net/uip.h"
#include "sys/stimer.h"
/**
 *  \name General
 * @{
 */
/** \brief HOP LIMIT to be used when sending ND messages (255) */
#define UIP_ND6_HOP_LIMIT               255
/** \brief INFINITE lifetime */
#define UIP_ND6_INFINITE_LIFETIME       0xFFFFFFFF
/** @} */

/** \name RFC 4861 Host constant */
/** @{ */
#define UIP_ND6_MAX_RTR_SOLICITATION_DELAY 1
#define UIP_ND6_RTR_SOLICITATION_INTERVAL  4
#define UIP_ND6_MAX_RTR_SOLICITATIONS	   3
/** @} */

/** \name RFC 4861 Router constants */
/** @{ */
#ifndef UIP_CONF_ND6_SEND_RA
#define UIP_ND6_SEND_RA                     1   /* enable/disable RA sending */
#else
#define UIP_ND6_SEND_RA UIP_CONF_ND6_SEND_RA
#endif
#ifndef UIP_CONF_ND6_SEND_NA
#define UIP_ND6_SEND_NA                     1   /* enable/disable NA sending */
#else
#define UIP_ND6_SEND_NA UIP_CONF_ND6_SEND_NA
#endif
#define UIP_ND6_MAX_RA_INTERVAL             600
#define UIP_ND6_MIN_RA_INTERVAL             (UIP_ND6_MAX_RA_INTERVAL / 3)
#define UIP_ND6_M_FLAG                      0
#define UIP_ND6_O_FLAG                      0
#define UIP_ND6_ROUTER_LIFETIME             3 * UIP_ND6_MAX_RA_INTERVAL

#define UIP_ND6_MAX_INITIAL_RA_INTERVAL     16  /*seconds*/
#define UIP_ND6_MAX_INITIAL_RAS             3   /*transmissions*/
#define UIP_ND6_MIN_DELAY_BETWEEN_RAS       3   /*seconds*/
//#define UIP_ND6_MAX_RA_DELAY_TIME           0.5 /*seconds*/
#define UIP_ND6_MAX_RA_DELAY_TIME_MS        500 /*milli seconds*/
/** @} */

#ifndef UIP_CONF_ND6_DEF_MAXDADNS
/** \brief Do not try DAD when using EUI-64 as allowed by draft-ietf-6lowpan-nd-15 section 8.2 */
#if UIP_CONF_LL_802154
#define UIP_ND6_DEF_MAXDADNS 0
#else /* UIP_CONF_LL_802154 */
#define UIP_ND6_DEF_MAXDADNS UIP_ND6_SEND_NA
#endif /* UIP_CONF_LL_802154 */
#else /* UIP_CONF_ND6_DEF_MAXDADNS */
#define UIP_ND6_DEF_MAXDADNS UIP_CONF_ND6_DEF_MAXDADNS
#endif /* UIP_CONF_ND6_DEF_MAXDADNS */

/** \name RFC 4861 Node constant */
#define UIP_ND6_MAX_MULTICAST_SOLICIT  3

#ifdef UIP_CONF_ND6_MAX_UNICAST_SOLICIT
#define UIP_ND6_MAX_UNICAST_SOLICIT    UIP_CONF_ND6_MAX_UNICAST_SOLICIT
#else /* UIP_CONF_ND6_MAX_UNICAST_SOLICIT */
#define UIP_ND6_MAX_UNICAST_SOLICIT    3
#endif /* UIP_CONF_ND6_MAX_UNICAST_SOLICIT */

#ifdef UIP_CONF_ND6_REACHABLE_TIME
#define UIP_ND6_REACHABLE_TIME         UIP_CONF_ND6_REACHABLE_TIME
#else
#define UIP_ND6_REACHABLE_TIME         30000
#endif
#ifdef UIP_CONF_ND6_RETRANS_TIMER
#define UIP_ND6_RETRANS_TIMER	       UIP_CONF_ND6_RETRANS_TIMER
#else
#define UIP_ND6_RETRANS_TIMER	       10000
#endif
#define UIP_ND6_DELAY_FIRST_PROBE_TIME 5
#define UIP_ND6_MIN_RANDOM_FACTOR(x)   (x / 2)
#define UIP_ND6_MAX_RANDOM_FACTOR(x)   ((x) + (x) / 2)
/** @} */

/** \name ND6 option types */
/** @{ */
#define UIP_ND6_OPT_SLLAO               1
#define UIP_ND6_OPT_TLLAO               2
#define UIP_ND6_OPT_PREFIX_INFO         3
#define UIP_ND6_OPT_REDIRECTED_HDR      4
#define UIP_ND6_OPT_MTU                 5
/** @} */

/** \name ND6 option types */
/** @{ */
//Offsets stay as byte units
#define UIP_ND6_OPT_TYPE_OFFSET         0
#define UIP_ND6_OPT_LEN_OFFSET          1
//EXCEPT for this one
#define UIP_ND6_OPT_DATA_OFFSET         CHAR_BIT_UNITS(2)

/** \name ND6 message length (excluding options) */
/** @{ */
#define UIP_ND6_NA_LEN                  CHAR_BIT_UNITS(20)
#define UIP_ND6_NS_LEN                  CHAR_BIT_UNITS(20)
#define UIP_ND6_RA_LEN                  CHAR_BIT_UNITS(12)
#define UIP_ND6_RS_LEN                  CHAR_BIT_UNITS(4)
/** @} */

/** \name ND6 option length in bytes */
/** @{ */
#define UIP_ND6_OPT_HDR_LEN            2
#define UIP_ND6_OPT_PREFIX_INFO_LEN    32
#define UIP_ND6_OPT_MTU_LEN            8

/* Length of TLLAO and SLLAO options, it is L2 dependant */
#if UIP_CONF_LL_802154
/* If the interface is 802.15.4. For now we use only long addresses */
#define UIP_ND6_OPT_SHORT_LLAO_LEN     8
#define UIP_ND6_OPT_LONG_LLAO_LEN      16
/** \brief length of a ND6 LLAO option for 802.15.4 */
#define UIP_ND6_OPT_LLAO_LEN 	CHAR_BIT_UNITS(UIP_ND6_OPT_LONG_LLAO_LEN)
#else /*UIP_CONF_LL_802154*/
#if UIP_CONF_LL_80211
/* If the interface is 802.11 */
/** \brief length of a ND6 LLAO option for 802.11 */
#define UIP_ND6_OPT_LLAO_LEN           8
#else /*UIP_CONF_LL_80211*/
/** \brief length of a ND6 LLAO option for default L2 type (e.g. Ethernet) */
#define UIP_ND6_OPT_LLAO_LEN           8
#endif /*UIP_CONF_LL_80211*/
#endif /*UIP_CONF_LL_802154*/
/** @} */

/** \name Neighbor Advertisement flags masks */
/** @{ */
#define UIP_ND6_NA_FLAG_ROUTER          0x80
#define UIP_ND6_NA_FLAG_SOLICITED       0x40
#define UIP_ND6_NA_FLAG_OVERRIDE        0x20
#define UIP_ND6_RA_FLAG_ONLINK          0x80
#define UIP_ND6_RA_FLAG_AUTONOMOUS      0x40
/** @} */

/**
 * \name ND message structures
 * @{
 */

/**
 * \brief A neighbor solicitation constant part
 *
 * Possible option is: SLLAO
 */
typedef struct uip_nd6_ns
{
    //u32_t reserved;
    u16_t reserved1;
    u16_t reserved2;
    uip_ipaddr_t tgtipaddr;
} uip_nd6_ns;

#if UIP_BYTE_ORDER == UIP_LITTLE_ENDIAN
/**
 * \brief A neighbor advertisement constant part.
 *
 * Possible option is: TLLAO
 */
typedef struct uip_nd6_na
{
    u16_t flagsreserved :8;
    u16_t reserved0 :8;
    u16_t reserved1 :8;
    u16_t reserved2 :8;
    uip_ipaddr_t tgtipaddr;
} uip_nd6_na;

/**
 * \brief A router advertisement constant part
 *
 * Possible options are: SLLAO, MTU, Prefix Information
 */
typedef struct uip_nd6_ra
{
    u16_t cur_ttl :8;
    u16_t flags_reserved :8;
    u16_t router_lifetime;
    u32_t reachable_time;
    u32_t retrans_timer;
} uip_nd6_ra;

/**
 * \name ND Option structures
 * @{
 */

/** \brief ND option header */
typedef struct uip_nd6_opt_hdr
{
    u16_t type :8;
    u16_t len :8;
} uip_nd6_opt_hdr;

/** \brief ND option prefix information */
typedef struct uip_nd6_opt_prefix_info
{
    u16_t type :8;
    u16_t len :8;
    u16_t preflen :8;
    u16_t flagsreserved1 :8;
    u32_t validlt;
    u32_t preferredlt;
    u32_t reserved2;
    uip_ipaddr_t prefix;
} uip_nd6_opt_prefix_info;

/** \brief ND option MTU */
typedef struct uip_nd6_opt_mtu
{
    u16_t type :8;
    u16_t len :8;
    u16_t reserved;
    u32_t mtu;
} uip_nd6_opt_mtu;

/** \struct Redirected header option */
typedef struct uip_nd6_opt_redirected_hdr
{
    u16_t type :8;
    u16_t len :8;
    u16_t reserved0 :8;
    u16_t reserved1 :8;
    u16_t reserved2 :8;
    u16_t reserved3 :8;
    u16_t reserved4 :8;
    u16_t reserved5 :8;
} uip_nd6_opt_redirected_hdr;
/** @} */

#else //UIP_BYTE_ORDER == UIP_BIG_ENDIAN
/**
 * \brief A neighbor advertisement constant part.
 *
 * Possible option is: TLLAO
 */
typedef struct uip_nd6_na
{
    u16_t reserved0:8;
    u16_t flagsreserved:8;

    u16_t reserved2:8;
    u16_t reserved1:8;

    uip_ipaddr_t tgtipaddr;
}uip_nd6_na;

/** \brief ND option header */
typedef struct uip_nd6_opt_hdr
{
    u16_t len:8;
    u16_t type:8;

}uip_nd6_opt_hdr;

#if 0
#error no support

/**
 * \brief A router solicitation  constant part
 *
 * Possible option is: SLLAO
 */
typedef struct uip_nd6_rs
{
    u32_t reserved;
}uip_nd6_rs;

/**
 * \brief A redirect message constant part
 *
 * Possible options are: TLLAO, redirected header
 */
typedef struct uip_nd6_redirect
{
    u32_t reserved;
    uip_ipaddr_t tgtipaddress;
    uip_ipaddr_t destipaddress;
}uip_nd6_redirect;
/** @} */
/** \brief ND option header */
typedef struct uip_nd6_opt_hdr
{
    u16_t len:8;
    u16_t type:8;

}uip_nd6_opt_hdr;
/**
 * \brief A router advertisement constant part
 *
 * Possible options are: SLLAO, MTU, Prefix Information
 */
typedef struct uip_nd6_ra
{
    u16_t flags_reserved:8;
    u16_t cur_ttl:8;

    u16_t router_lifetime;
    u32_t reachable_time;
    u32_t retrans_timer;
}uip_nd6_ra;

/**
 * \name ND Option structures
 * @{
 */

/** \brief ND option prefix information */
typedef struct uip_nd6_opt_prefix_info
{
    u16_t len:8;
    u16_t type:8;

    u16_t flagsreserved1:8;
    u16_t preflen:8;

    u32_t validlt;
    u32_t preferredlt;
    u32_t reserved2;
    uip_ipaddr_t prefix;
}uip_nd6_opt_prefix_info;

/** \brief ND option MTU */
typedef struct uip_nd6_opt_mtu
{
    u16_t len:8;
    u16_t type:8;

    u16_t reserved;
    u32_t mtu;
}uip_nd6_opt_mtu;

/** \struct Redirected header option */
typedef struct uip_nd6_opt_redirected_hdr
{
    u16_t len:8;
    u16_t type:8;

    u16_t reserved1:8;
    u16_t reserved0:8;

    u16_t reserved3:8;
    u16_t reserved2:8;

    u16_t reserved5:8;
    u16_t reserved4:8;

}uip_nd6_opt_redirected_hdr;
/** @} */
#endif

#endif //NOT SUPPORTED

/**
 * \name ND Messages Processing and Generation
 * @{
 */
/**
 * \brief Process a neighbor solicitation
 *
 * The NS can be received in 3 cases (procedures):
 * - sender is performing DAD (ip src = unspecified, no SLLAO option)
 * - sender is performing NUD (ip dst = unicast)
 * - sender is performing address resolution (ip dest = solicited node mcast
 * address)
 *
 * We do:
 * - if the tgt belongs to me, reply, otherwise ignore
 * - if i was performing DAD for the same address, two cases:
 * -- I already sent a NS, hence I win
 * -- I did not send a NS yet, hence I lose
 *
 * If we need to send a NA in response (i.e. the NS was done for NUD, or
 * address resolution, or DAD and there is a conflict), we do it in this
 * function: set src, dst, tgt address in the three cases, then for all cases
 * set the rest, including  SLLAO
 *
 */
void
uip_nd6_ns_input(void);

/**
 * \brief Send a neighbor solicitation, send a Neighbor Advertisement
 * \param src pointer to the src of the NS if known
 * \param dest pointer to ip address to send the NS, for DAD or ADDR Resol,
 * MUST be NULL, for NUD, must be correct unicast dest
 * \param tgt  pointer to ip address to fill the target address field, must
 * not be NULL
 *
 * - RFC 4861, 7.2.2 :
 *   "If the source address of the packet prompting the solicitation is the
 *   same as one of the addresses assigned to the outgoing interface, that
 *   address SHOULD be placed in the IP Source Address of the outgoing
 *   solicitation.  Otherwise, any one of the addresses assigned to the
 *   interface should be used."
 *   This is why we have a src ip address as argument. If NULL, we will do
 *   src address selection, otherwise we use the argument.
 *
 * - we check if it is a NS for Address resolution  or NUD, if yes we include
 *   a SLLAO option, otherwise no.
 */
void
uip_nd6_ns_output(uip_ipaddr_t *src, uip_ipaddr_t *dest, uip_ipaddr_t *tgt);

/**
 * \brief Process a Neighbor Advertisement
 *
 * we might have to send a pkt that had been buffered while address
 * resolution was performed (if we support buffering, see UIP_CONF_QUEUE_PKT)
 *
 * As per RFC 4861, on link layer that have addresses, TLLAO options MUST be
 * included when responding to multicast solicitations, SHOULD be included in
 * response to unicast (here we assume it is for now)
 *
 * NA can be received after sending NS for DAD, Address resolution or NUD. Can
 * be unsolicited as well.
 * It can trigger update of the state of the neighbor in the neighbor cache,
 * router in the router list.
 * If the NS was for DAD, it means DAD failed
 *
 */
void
uip_nd6_na_input(void);

#if UIP_CONF_ROUTER
#if UIP_ND6_SEND_RA
/**
 * \brief Process a Router Solicitation
 *
 */
void uip_nd6_rs_input(void);

/**
 * \brief send a Router Advertisement
 *
 * Only for router, for periodic as well as sollicited RA
 */
void uip_nd6_ra_output(uip_ipaddr_t *dest);
#endif /* UIP_ND6_SEND_RA */
#endif /*UIP_CONF_ROUTER*/

/**
 * \brief Send a Router Solicitation
 *
 * src is chosen through the uip_netif_select_src function. If src is
 * unspecified  (i.e. we do not have a preferred address yet), then we do not
 * put a SLLAO option (MUST NOT in RFC 4861). Otherwise we do.
 *
 * RS message format,
 * possible option is SLLAO, MUST NOT be included if source = unspecified
 * SHOULD be included otherwise
 */
void uip_nd6_rs_output(void);

/**
 *
 * \brief process a Router Advertisement
 *
 * - Possible actions when receiving a RA: add router to router list,
 *   recalculate reachable time, update link hop limit, update retrans timer.
 * - If MTU option: update MTU.
 * - If SLLAO option: update entry in neighbor cache
 * - If prefix option: start autoconf, add prefix to prefix list
 */
void
uip_nd6_ra_input(void);
/** @} */

void
uip_appserver_addr_get(uip_ipaddr_t *ipaddr);
/*--------------------------------------*/
/******* ANNEX - message formats ********/
/*--------------------------------------*/

/*
 * RS format. possible option is SLLAO
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |     Code      |          Checksum             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                            Reserved                           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |   Options ...
 *    +-+-+-+-+-+-+-+-+-+-+-+-
 *
 *
 * RA format. possible options: prefix information, MTU, SLLAO
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |     Code      |          Checksum             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    | Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                         Reachable Time                        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                          Retrans Timer                        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |   Options ...
 *    +-+-+-+-+-+-+-+-+-+-+-+-
 *
 *
 * NS format: options should be SLLAO
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |     Code      |          Checksum             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                           Reserved                            |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +                       Target Address                          +
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |   Options ...
 *    +-+-+-+-+-+-+-+-+-+-+-+-
 *
 *
 * NA message format. possible options is TLLAO
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |     Code      |          Checksum             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |R|S|O|                     Reserved                            |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +                       Target Address                          +
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |   Options ...
 *    +-+-+-+-+-+-+-+-+-+-+-+-
 *
 *
 * Redirect message format. Possible options are TLLAO and Redirected header
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |     Code      |          Checksum             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                           Reserved                            |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +                       Target Address                          +
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +                     Destination Address                       +
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |   Options ...
 *    +-+-+-+-+-+-+-+-+-+-+-+-
 *
 *
 * SLLAO/TLLAO option:
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |    Length     |    Link-Layer Address ...
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * Prefix information option
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |    Length     | Prefix Length |L|A| Reserved1 |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                         Valid Lifetime                        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                       Preferred Lifetime                      |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                           Reserved2                           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +                            Prefix                             +
 *    |                                                               |
 *    +                                                               +
 *    |                                                               |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * MTU option
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |    Length     |           Reserved            |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                              MTU                              |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * Redirected header option
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |    Length     |            Reserved           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                           Reserved                            |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                               |
 *    ~                       IP header + data                        ~
 *    |                                                               |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
#endif /* __UIP_ND6_H__ */

/** @} */
