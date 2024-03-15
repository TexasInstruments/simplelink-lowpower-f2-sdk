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
 *  ====================== rpl_plusplus.c =============================================
 *  This file is created to contain the functions that were added for RPL++ improvements 
 */

#include "rpl/rpl-plusplus.h"

#if RPL_PLUSPLUS

#define DAO_LATENCY_ADDITIVE		1
#define DAO_LATENCY_MULTIPLICATIVE	2

#define DAO_LATENCY_MODE			DAO_LATENCY_ADDITIVE

#define DAO_LATENCY_EXTREME_BOUND	(RPL_DAO_LATENCY * 9)

#define DAO_LATENCY_MULTIPLICATIVE_K	2

uint32_t pow(uint16_t base, uint16_t exponent)
{
    uint16_t i;
    uint32_t result = 1;
    for (i=0;i<exponent;++i)
    {
        result = result * base;
    }
    return result;
}

void rpl_modify_dao_interval(rpl_instance_t *instance)
{
#if DAO_LATENCY_MODE == DAO_LATENCY_ADDITIVE
    instance->dao_latency += RPL_DAO_LATENCY;
    if (instance->dao_latency > DAO_LATENCY_EXTREME_BOUND)
    instance->dao_latency = DAO_LATENCY_EXTREME_BOUND;
#elif DAO_LATENCY_MODE == DAO_LATENCY_MULTIPLICATIVE
    instance->dao_reemissions++;
    instance->dao_latency = pow(DAO_LATENCY_MULTIPLICATIVE_K,instance->dao_reemissions) * RPL_DAO_LATENCY;
    if (instance->dao_latency > DAO_LATENCY_EXTREME_BOUND)
    instance->dao_latency = DAO_LATENCY_EXTREME_BOUND;
#endif
}

void rpl_no_ack(uip_ipaddr_t* ipaddr)
{
    uip_ds6_nbr_t* nbr = uip_ds6_nbr_lookup(ipaddr);
    if (nbr != NULL)
    {
        //Set the reachable timer to 0 for the stack to test the node
        if (nbr->state != NBR_PROBE)
        { //If we are not already testing it, force test
            /*
             stimer_set(&nbr->reachable, 0);
             nbr->state = NBR_DELAY;
             */
            nbr->state = NBR_DELAY;
            stimer_set(&nbr->reachable, 0);
            nbr->nscount = 0;
        }
    }
}

void rpl_periodic_parent_check(rpl_instance_t *instance)
{
    rpl_parent_t *parent;
    uip_ds6_nbr_t* nbr;
    rpl_dag_t *dag;

    dag = instance->current_dag;
    if(dag != NULL)
    {
        parent = dag->preferred_parent;
        if(parent != NULL)
        {
            nbr = parent->nbr;
            if (nbr != NULL)
            {
                if (nbr->state == NBR_STALE)
                {
                    /*
                     stimer_set(&nbr->reachable, 0);
                     nbr->state = NBR_DELAY;
                     */
                    nbr->state = NBR_DELAY;
                    stimer_set(&nbr->reachable, 0);
                    nbr->nscount = 0;
                }
            }
        }
    }
}

#if (RPL_CONF_MOP != RPL_MOP_NON_STORING)
//ROOT does not need to send any dao to anyone.
//In the case of non root Storing mode, these functions should be called and corresponding DAOs should be sent

void rpl_dao_update_routes(rpl_parent_t *parent, rpl_instance_t *instance)
{
    uip_ds6_route_t *locrt = uip_ds6_route_head();
    while (locrt)
    {
        if (!locrt->state.nopath_received)
        {
            //while we have downward routes, send that info upwards with a DAO
            dao_output_target(parent, &locrt->ipaddr, instance->default_lifetime);
        }
        locrt = uip_ds6_route_next(locrt);
    }
}

void rpl_nexthop_dead(uip_ds6_nbr_t *nbr)
{
    //if he is not a parent, he must be a downward route, which is lost.. we must erase all routes that use him as next step and tell our parents with a No-Path DAO
    uip_ds6_route_t *locrt;
    rpl_dag_t *dag = rpl_get_any_dag();
    while (locrt = uip_ds6_route_lookup_by_nexthop(&nbr->ipaddr))
    {
        if (dag->preferred_parent != NULL)
        { //if we have a preferred parent, send him DAOs No Path to tell that the routes are lost
            //Send this if we have a preferred parent. If we do not have a preferred parent, probably we are the root
            dao_output_target(dag->preferred_parent, &locrt->ipaddr, RPL_ZERO_LIFETIME);
        }
        uip_ds6_route_rm(locrt);
    }
}

#endif

#endif //RPL_PLUSPLUS
