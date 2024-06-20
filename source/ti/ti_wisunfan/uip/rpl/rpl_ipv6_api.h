/*
 * Copyright (c) 2014 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free,
 * non-exclusive license under copyrights and patents it now or hereafter
 * owns or controls to make, have made, use, import, offer to sell and sell ("Utilize")
 * this software subject to the terms herein.  With respect to the foregoing patent
 *license, such license is granted  solely to the extent that any such patent is necessary
 * to Utilize the software alone.  The patent license shall not apply to any combinations which
 * include this software, other than combinations with devices manufactured by or for TI (â€œTI Devicesâ€�).
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license (including the
 * above copyright notice and the disclaimer and (if applicable) source code license limitations below)
 * in the documentation and/or other materials provided with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided that the following
 * conditions are met:
 *
 *       * No reverse engineering, decompilation, or disassembly of this software is permitted with respect to any
 *     software provided in binary form.
 *       * any redistribution and use are licensed by TI for use only with TI Devices.
 *       * Nothing shall obligate TI to provide you with source code for the software licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the source code are permitted
 * provided that the following conditions are met:
 *
 *   * any redistribution and use of the source code, including any resulting derivative works, are licensed by
 *     TI for use only with TI Devices.
 *   * any redistribution and use of any object code compiled from the source code and any resulting derivative
 *     works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers may be used to endorse or
 * promote products derived from this software without specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TIâ€™S LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TIâ€™S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ====================== rpl_ipv6_api.h =============================================
 *  RPL API header file for IP layer. 
 */

#ifndef RPL_IPV6_API_H
#define RPL_IPV6_API_H

#include "uip-conf.h"
#include "net/uip.h"
#include "net/uip-ds6.h"

//Global variables
#define rpl_ip_buf             uip_buf
#define rpl_ip_len             uip_len
#define rpl_ip_ext_len         uip_ext_len
#define rpl_ip_src_lladdr      uip_src_lladdr
#define RPL_BUFSIZE            UIP_BUFSIZE

//IPv6 ds6 notification event constants
#define RPL_DS6_NBR_ADD                 UIP_DS6_NBR_ADD
#define RPL_DS6_NBR_RM                  UIP_DS6_NBR_RM
#define RPL_DS6_ROUTE_DEFRT_ADD         UIP_DS6_ROUTE_DEFRT_ADD
#define RPL_DS6_ROUTE_DEFRT_RM          UIP_DS6_ROUTE_DEFRT_RM
#define RPL_DS6_ROUTE_ADD               UIP_DS6_ROUTE_ADD
#define RPL_DS6_ROUTE_RM                UIP_DS6_ROUTE_RM

//Hop By Hop Option header actions
#define RPL_HBHO_HDR_UPDATE_EMPTY        UIP_HBHO_HDR_UPDATE_EMPTY
#define RPL_HBHO_HDR_UPDATE_NEXT_HOP     UIP_HBHO_HDR_UPDATE_NEXT_HOP
#define RPL_HBHO_HDR_VERIFY              UIP_HBHO_HDR_VERIFY
#define RPL_HBHO_HDR_INSERT              UIP_HBHO_HDR_INSERT
#define RPL_HBHO_HDR_INVERT              UIP_HBHO_HDR_INVERT

//Source Routing header actions
#define RPL_SOURCE_ROUTING_HDR_INSERT    UIP_SOURCE_ROUTING_HDR_INSERT
#define RPL_SOURCE_ROUTING_HDR_PROCESS   UIP_SOURCE_ROUTING_HDR_PROCESS

//RPL Routing mode
#define RPL_SOURCE_ROUTING_MODE			 UIP_DS6_SOURCE_ROUTING
#define RPL_STANDARD_ROUTING_MODE		 UIP_DS6_STANDARD_ROUTING
#define RPL_NO_ROUTING_MODE				 UIP_DS6_NO_ROUTING
#define RPL_ROUTING_MODE				 UIP_DS6_ROUTING_MODE

//ICMPv6 Transmission
void uip_icmp6_send(uip_ipaddr_t *dest, int type, int code, int payload_len);
void uip_icmp6_error_output_destination(u16_t type, u16_t code, u32_t param,
                                        uip_ipaddr_t* destination);

//ROUTES 
#if (RPL_ROUTING_MODE == RPL_SOURCE_ROUTING_MODE)

uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_add(uip_ipaddr_t *ipaddr, u16_t length,
        uip_ipaddr_t *previousHop);
//Remove a given route in the source routing table
void uip_ds6_route_sourceRouting_rm(uip_ds6_route_sourceRouting_t *route);
//Obtaine a route for the target in "addr". This would only be the route target/previousHop but not the path to it
uip_ds6_route_sourceRouting_t *
uip_ds6_sourceRouting_route_lookup(uip_ipaddr_t *addr);
//Get a pointer to the beginning of the list of route items in the source routing table. NULL if there are no route items
uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_head(void);
//Get a pointer to the next route item in the source routing table. NULL if there are no more items
uip_ds6_route_sourceRouting_t *
uip_ds6_route_sourceRouting_next(uip_ds6_route_sourceRouting_t *r);

#elif (RPL_ROUTING_MODE == RPL_STANDARD_ROUTING_MODE)

uip_ds6_route_t *uip_ds6_route_lookup(uip_ipaddr_t *destipaddr);
uip_ds6_route_t *uip_ds6_route_lookup_by_nexthop(uip_ipaddr_t *nexthop);
uip_ds6_route_t *uip_ds6_route_add(uip_ipaddr_t *ipaddr, uint16_t length,
                                   uip_ipaddr_t *next_hop);
void uip_ds6_route_rm(uip_ds6_route_t *route);
uip_ipaddr_t *uip_ds6_route_nexthop(uip_ds6_route_t *);
uip_ds6_route_t *uip_ds6_route_head(void);
uip_ds6_route_t *uip_ds6_route_next(uip_ds6_route_t *);

#endif

uip_ds6_defrt_t *uip_ds6_defrt_add(uip_ipaddr_t *ipaddr,
                                   unsigned long interval);
void uip_ds6_defrt_rm(uip_ds6_defrt_t *defrt);

//NEIGHBORS 
uip_ds6_nbr_t *uip_ds6_nbr_add(const uip_ipaddr_t *ipaddr,
                               const uip_lladdr_t *lladdr, u16_t isrouter,
                               u16_t state);
void uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr);
uip_lladdr_t *uip_ds6_nbr_get_ll(uip_ds6_nbr_t *nbr);
uip_ds6_nbr_t *uip_ds6_nbr_lookup(uip_ipaddr_t *ipaddr);
uip_ds6_nbr_t *uip_ds6_nbr_ll_lookup(const uip_lladdr_t *lladdr);
void uip_ds6_nbr_lock(uip_ds6_nbr_t *nbr);
void uip_ds6_nbr_unlock(uip_ds6_nbr_t *nbr);

//ADDRESSES
uip_ds6_addr_t *uip_ds6_addr_add(uip_ipaddr_t *ipaddr, unsigned long vlifetime,
                                 uint16_t type);
void uip_ds6_addr_rm(uip_ds6_addr_t *addr);
uip_ds6_addr_t *uip_ds6_addr_lookup(uip_ipaddr_t *ipaddr);
uip_ds6_addr_t *uip_ds6_get_link_local(int16_t state);
uip_ds6_addr_t *uip_ds6_get_global(int16_t state);
void uip_ds6_set_addr_iid(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr);
uip_ds6_maddr_t *uip_ds6_maddr_add(const uip_ipaddr_t *ipaddr);

#ifndef uip_ds6_is_my_addr
int uip_ds6_is_my_addr(uip_ipaddr_t *ipaddr);
#endif

//UTILITIES
#ifndef uip_ip6addr
void uip_ip6addr(uip_ipaddr_t *addr, uint16_t addr0, uint16_t addr1,
        uint16_t addr2, uint16_t addr3, uint16_t addr4,
        uint16_t addr5, uint16_t addr6, uint16_t addr7);
#endif

#ifndef uip_ipaddr_copy
void uip_ipaddr_copy(uip_ipaddr_t *dest, uip_ipaddr_t *src);
#endif

#ifndef uip_ipaddr_cmp
int uip_ipaddr_cmp(uip_ipaddr_t *addr1, uip_ipaddr_t *addr2);
#endif

#ifndef uip_ipaddr_prefixcmp
int uip_ipaddr_prefixcmp(uip_ipaddr_t *addr1, uip_ipaddr_t *addr2, int prefix_len);
#endif

#ifndef uip_is_addr_link_local
int uip_is_addr_link_local(uip_ipaddr_t *addr);
#endif

#ifndef uip_is_addr_mcast
int uip_is_addr_mcast(uip_ipaddr_t *addr);
#endif

#endif /* RPL_IPV6_API_H */
