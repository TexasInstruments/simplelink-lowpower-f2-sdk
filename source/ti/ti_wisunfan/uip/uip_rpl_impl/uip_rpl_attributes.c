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
 * include this software, other than combinations with devices manufactured by or for TI (TI Devices).
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
 * THIS SOFTWARE IS PROVIDED BY TI AND TIS LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TIS LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ====================== uip_rpl_attributes.c =============================================
 */

#include "uip-conf.h"
#ifndef RPL_DISABLE
#include "rpl/rpl.h"
#endif
#include "net/tcpip.h"
#include "net/uip-ds6.h"
#include "net/uip-nd6.h"

#include "uip_rpl_process.h"

// in GW, we need to  copy the tun_addr interface to this array
//
const uint8_t uip_rpl_attr_prefix[UIP_RPL_ATTR_PREFIX_LEN] = {
        UIP_RPL_ATTR_PREFIX };
//uint8_t uip_rpl_attr_prefix[UIP_RPL_ATTR_PREFIX_LEN] = {UIP_RPL_ATTR_PREFIX};

uip_rpl_attr_t uip_rpl_attributes;

void uip_rpl_attr_init()
{
#ifndef RPL_DISABLE
    uip_rpl_attributes.dao_latency = (UIP_RPL_ATTR_DAO_LATENCY * CLOCK_SECOND);
    uip_rpl_attributes.dio_interval_min = UIP_RPL_ATTR_DIO_INTERVAL_MIN;
    uip_rpl_attributes.dio_interval_doublings =
            UIP_RPL_ATTR_DIO_INTERVAL_DOUBLINGS;
#endif
    memcpy(uip_rpl_attributes.prefix, uip_rpl_attr_prefix,
           UIP_RPL_ATTR_PREFIX_LEN);
}
#ifndef RPL_DISABLE
uip_rpl_attr_status uip_rpl_attr_get_dio_next_delay(uint32_t *delay)
{
    rpl_instance_t *instance = rpl_get_default_instance();

    if (!tcpip_isInitialized())
        return UIP_RPL_ATTR_ERROR_STACK_NOT_STARTED;

    if (instance == NULL)
        return UIP_RPL_ATTR_ERROR;

    //Need to convert from system ticks to milliseconds
    (*delay) =
            ((instance->dio_next_delay / CLOCK_SECOND) * 1000)
                    + (((instance->dio_next_delay % CLOCK_SECOND) * 1000)
                            / CLOCK_SECOND);

    return UIP_RPL_ATTR_OK;
}

uip_rpl_attr_status uip_rpl_attr_get_dio_interval_min(uint8_t *int_min)
{
    rpl_instance_t *instance = rpl_get_default_instance();
    if (instance != NULL)
    {
        uip_rpl_attributes.dio_interval_min = instance->dio_intmin;
    }
    (*int_min) = uip_rpl_attributes.dio_interval_min;
    return UIP_RPL_ATTR_OK;
}

uip_rpl_attr_status uip_rpl_attr_set_dio_interval_min(uint8_t int_min)
{
#if UIP_RPL_ROOT
    if (tcpip_isInitialized())
    return UIP_RPL_ATTR_ERROR_STACK_STARTED;

    uip_rpl_attributes.dio_interval_min = int_min;

    return UIP_RPL_ATTR_OK;
#else
    return UIP_RPL_ATTR_ERROR; //Regular nodes take this value from DIO. We cannot change it
#endif
}

uip_rpl_attr_status uip_rpl_attr_get_dio_interval_doublings(
        uint8_t *int_doublings)
{
    rpl_instance_t *instance = rpl_get_default_instance();
    if (instance != NULL)
    {
        uip_rpl_attributes.dio_interval_doublings = instance->dio_intdoubl;
    }
    (*int_doublings) = uip_rpl_attributes.dio_interval_doublings;
    return UIP_RPL_ATTR_OK;
}

uip_rpl_attr_status uip_rpl_attr_set_dio_interval_doublings(
        uint8_t int_doublings)
{
#if UIP_RPL_ROOT
    if (tcpip_isInitialized())
    return UIP_RPL_ATTR_ERROR_STACK_STARTED;

    uip_rpl_attributes.dio_interval_doublings = int_doublings;

    return UIP_RPL_ATTR_OK;
#else
    return UIP_RPL_ATTR_ERROR; //Regular nodes take this value from DIO. We cannot change it
#endif
}

uip_rpl_attr_status uip_rpl_attr_get_dao_latency(uint8_t *dao_latency)
{
#if UIP_RPL_ROOT
    return UIP_RPL_ATTR_ERROR; //Root nodes do not send DAO
#else
    //Try to update with the default instance dao_latency value
    rpl_instance_t *instance = rpl_get_default_instance();
    if (instance != NULL)
    {
        uip_rpl_attributes.dao_latency = instance->dao_latency;
    }
    (*dao_latency) = (uip_rpl_attributes.dao_latency / CLOCK_SECOND);
    return UIP_RPL_ATTR_OK;
#endif
}

uip_rpl_attr_status uip_rpl_attr_set_dao_latency(uint8_t dao_latency)
{
#if UIP_RPL_ROOT
    return UIP_RPL_ATTR_ERROR; //Root nodes do not send DAO
#else
    uip_rpl_attributes.dao_latency = (dao_latency * CLOCK_SECOND);
    //We also need to update the latency for the default instance, if we have joined
    rpl_instance_t *instance = rpl_get_default_instance();
    if (instance != NULL)
    {
        instance->dao_latency = uip_rpl_attributes.dao_latency;
    }
    return UIP_RPL_ATTR_OK;
#endif
}

#endif //!RPL_DISABLE

uip_rpl_attr_status uip_rpl_attr_get_ip_global_prefix(void* prefix)
{
    pltfrm_byte_memcpy(prefix, 0, uip_rpl_attributes.prefix, 0,
                       UIP_RPL_ATTR_PREFIX_LEN);
    return UIP_RPL_ATTR_OK;
}

uip_rpl_attr_status uip_rpl_attr_set_ip_global_prefix(void* prefix)
{
    pltfrm_byte_memcpy(uip_rpl_attributes.prefix, 0, prefix, 0,
                       UIP_RPL_ATTR_PREFIX_LEN);
    return UIP_RPL_ATTR_OK;
}

uip_rpl_attr_status uip_rpl_attr_get_ip_routing_table_entry(uint16_t index,
                                                            char* entry)
{
    uint16_t i = 0;
    uint16_t pos = 0;

    if (index >= UIP_DS6_ROUTE_NB)
        return UIP_RPL_ATTR_ERROR_INVALID_INDEX;

#if NON_STORING_MODE_ROOT
    uip_ds6_route_sourceRouting_t * route = uip_ds6_route_sourceRouting_head();

    while(route != NULL)
    {
        if (i==index)
        {
            //Prefix
            memcpy(entry+pos, route->ipaddr.u16, sizeof(route->ipaddr.u16));
            pos += sizeof(route->ipaddr.u16);

            //Prefix length
            memcpy(entry+pos, &route->length, sizeof(route->length));
            pos += sizeof(route->length);

            //Previous Hop
            memcpy(entry+pos, route->previousHop.u16, sizeof(route->previousHop.u16));
            pos += sizeof(route->previousHop.u16);

            //Lifetime
#if !defined(RPL_DISABLE) && !RPL_ROUTE_INFINITE_LIFETIME
            memcpy(entry+pos, &route->state.lifetime, sizeof(route->state.lifetime));
#else
            memset(entry+pos, 0, sizeof(uint32_t));
#endif

            return UIP_RPL_ATTR_OK;
        }
        else
        {
            route = uip_ds6_route_sourceRouting_next(route);
            ++i;
        }
    }
#else
    uip_ds6_route_t * route = uip_ds6_route_head();

    while (route != NULL)
    {
        if (i == index)
        {
            //Prefix
            memcpy(entry + pos, route->ipaddr.u16, sizeof(route->ipaddr.u16));
            pos += sizeof(route->ipaddr.u16);

            //Prefix length
            memcpy(entry + pos, &route->length, sizeof(route->length));
            pos += sizeof(route->length);

            //Next Hop
            uip_ipaddr_t *next_hop = uip_ds6_route_nexthop(route);
            if (next_hop == NULL)
            {
                return UIP_RPL_ATTR_ERROR;
            }
            memcpy(entry + pos, next_hop->u16, sizeof(next_hop->u16));
            pos += sizeof(next_hop->u16);

            //Lifetime
#if !defined(RPL_DISABLE) && !RPL_ROUTE_INFINITE_LIFETIME
            memcpy(entry + pos, &route->state.lifetime,
                   sizeof(route->state.lifetime));
#else
            memset(entry+pos, 0, sizeof(uint32_t));
#endif

            return UIP_RPL_ATTR_OK;
        }
        else
        {
            route = uip_ds6_route_next(route);
            ++i;
        }
    }
#endif
    return UIP_RPL_ATTR_ERROR_INVALID_INDEX;
}

uip_rpl_attr_status uip_rpl_attr_get_neighbor_table_entry(uint16_t index,
                                                          char* entry)
{
    uint16_t i = 0;
    uint16_t pos = 0;
    uint8_t state = 0;
    uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);

    if (index >= (NBR_TABLE_MAX_NEIGHBORS * 2)) //Look for it in neighbor table AND secondary table
        return UIP_RPL_ATTR_ERROR_INVALID_INDEX;

    while (nbr != NULL)
    {
        if (i == index)
        {
            //Ip address
            memcpy(entry + pos, nbr->ipaddr.u16, sizeof(nbr->ipaddr.u16));
            pos += sizeof(nbr->ipaddr.u16);

            //Extended MAC address
            uip_lladdr_t* extMacAddr = uip_ds6_nbr_get_ll(nbr);
            if (extMacAddr == NULL)
            {
                return UIP_RPL_ATTR_ERROR;
            }
            memcpy(entry + pos, extMacAddr->addr16, sizeof(extMacAddr->addr16));
            pos += sizeof(extMacAddr->addr16);

            //Reachable time
            uint16_t reachable = stimer_remaining(&nbr->reachable);
            memcpy(entry + pos, &reachable, sizeof(reachable));
            pos += sizeof(reachable);

            //State
            state = nbr->state;
            memcpy(entry + pos, &state, sizeof(state));

            return UIP_RPL_ATTR_OK;
        }
        else
        {
            nbr = nbr_table_next(ds6_neighbors, nbr);
            ++i;
        }
    }
    //If we did not find it yet, look for it in the secondary one
    /*	secondary_ipaddr_nbr_t* sec_nbr = secondary_ipaddr_nbr_head();
     while (sec_nbr != NULL){
     if (i==index){
     //Ip address
     memcpy(entry+pos, sec_nbr->ipaddr.u16, sizeof(nbr->ipaddr.u16));
     pos += sizeof(nbr->ipaddr.u16);

     //Extended MAC address
     uip_lladdr_t* extMacAddr = uip_ds6_nbr_get_ll(sec_nbr->nbr);
     if (extMacAddr == NULL){
     return UIP_RPL_ATTR_ERROR;
     }
     memcpy(entry+pos, extMacAddr->addr16, sizeof(extMacAddr->addr16));
     pos += sizeof(extMacAddr->addr16);

     //Reachable time
     uint16_t reachable = stimer_remaining(&sec_nbr->nbr->reachable);
     memcpy(entry+pos, &reachable, sizeof(reachable));
     pos += sizeof(reachable);

     //State
     state = 1;
     memset(entry+pos, &state, sizeof(state));

     return UIP_RPL_ATTR_OK;
     }else{
     sec_nbr = secondary_ipaddr_nbr_next(sec_nbr);
     ++i;
     }
     }*/
    //Removed by Tao, don't understand what is it doing. Secondary table is duplicate
    return UIP_RPL_ATTR_ERROR_INVALID_INDEX;
}
