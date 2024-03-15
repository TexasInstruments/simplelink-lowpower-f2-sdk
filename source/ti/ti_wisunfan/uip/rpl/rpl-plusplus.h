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
 *  ====================== rpl_plusplus.h =============================================
 *  Header file for RPL++ improvements.
 */

#ifndef RPL_PLUSPLUS_H_
#define RPL_PLUSPLUS_H_

#include "rpl/rpl-private.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"

#ifndef RPL_PLUSPLUS_CONF
#define RPL_PLUSPLUS    1
#else
#define RPL_PLUSPLUS    RPL_PLUSPLUS_CONF
#endif

#if !RPL_PLUSPLUS

#define RPL_PLUSPLUS_MODIFY_DAO_INTERVAL        0 //Dynamic DAO interval (for very large networks)
#define RPL_PLUSPLUS_PROBE_IF_NO_ACK		0 //Probe neighbor if no ACK
#define RPL_PLUSPLUS_PERIODIC_PARENT_CHECK      0 //Parent "keep alive"
#define RPL_PLUSPLUS_DAO_UPDATE_ROUTES          0 //Fast recovery
#define RPL_PLUSPLUS_NEXT_HOP_DEAD              0 //Update route info

#else

#ifndef RPL_PLUSPLUS_MODIFY_DAO_INTERVAL_CONF
#define RPL_PLUSPLUS_MODIFY_DAO_INTERVAL        0 //Dynamic DAO interval (for very large networks)
#else
#define RPL_PLUSPLUS_MODIFY_DAO_INTERVAL        RPL_PLUSPLUS_MODIFY_DAO_INTERVAL_CONF
#endif

#ifndef RPL_PLUSPLUS_PROBE_IF_NO_ACK_CONF
#define RPL_PLUSPLUS_PROBE_IF_NO_ACK		0 //Probe neighbor if no ACK
#else
#define RPL_PLUSPLUS_PROBE_IF_NO_ACK    RPL_PLUSPLUS_PROBE_IF_NO_ACK_CONF
#endif

#ifndef RPL_PLUSPLUS_PERIODIC_PARENT_CHECK_CONF
#define RPL_PLUSPLUS_PERIODIC_PARENT_CHECK      0 //Parent "keep alive"
#else
#define RPL_PLUSPLUS_PERIODIC_PARENT_CHECK      RPL_PLUSPLUS_PERIODIC_PARENT_CHECK_CONF
#endif

#ifndef RPL_PLUSPLUS_DAO_UPDATE_ROUTES_CONF
#define RPL_PLUSPLUS_DAO_UPDATE_ROUTES          1 //Fast recovery
#else
#define RPL_PLUSPLUS_DAO_UPDATE_ROUTES  RPL_PLUSPLUS_DAO_UPDATE_ROUTES_CONF
#endif

#ifndef RPL_PLUSPLUS_NEXT_HOP_DEAD_CONF
#define RPL_PLUSPLUS_NEXT_HOP_DEAD              1 //Update route info
#else
#define RPL_PLUSPLUS_NEXT_HOP_DEAD      RPL_PLUSPLUS_NEXT_HOP_DEAD_CONF
#endif

#ifndef RPL_PLUSPLUS_DTSN_STRATEGY_CONF
#define RPL_PLUSPLUS_DTSN_STRATEGY              0 //DTSN incrementation strategy
#else
#define RPL_PLUSPLUS_DTSN_STRATEGY      RPL_PLUSPLUS_DTSN_STRATEGY_CONF
#endif

#endif //RPL_PLUSPLUS

void rpl_modify_dao_interval(rpl_instance_t *instance);
void rpl_no_ack(uip_ipaddr_t* ipaddr);
void rpl_periodic_parent_check(rpl_instance_t *instance);
#if (RPL_MOP_DEFAULT != RPL_NON_STORING)
void rpl_dao_update_routes(rpl_parent_t *parent, rpl_instance_t *instance);
void rpl_nexthop_dead(uip_ds6_nbr_t *nbr);
#endif

#endif //RPL_PLUSPLUS_H_
