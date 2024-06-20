/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * \file
 *	Public configuration and API declarations for ContikiRPL.
 * \author
 *	Joakim Eriksson <joakime@sics.se> & Nicolas Tsiftes <nvt@sics.se>
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

#ifndef RPL_CONF_H
#define RPL_CONF_H

#include "config-rpl-private.h"

#ifndef UIP_RPL_ROOT
#error "Error. Private configuration files must define the UIP_RPL_ROOT symbol as 0 or 1"
#endif

//Define if the node is a NON_STORING_MODE_ROOT
#if ((UIP_RPL_ROOT) && (RPL_CONF_MOP == RPL_MOP_NON_STORING))
#define NON_STORING_MODE_ROOT	1
#else
#define NON_STORING_MODE_ROOT	0
#endif

#ifndef RPL_SUPPRESS_DIS_CONF
#define RPL_SUPPRESS_DIS_CONF   0
#endif

#ifndef RPL_SUPPRESS_DAO_ACK_CONF
#define RPL_SUPPRESS_DAO_ACK_CONF   0
#endif

/* Set to 1 to enable RPL statistics */
#ifndef RPL_CONF_STATS
#define RPL_CONF_STATS 0
#endif /* RPL_CONF_STATS */

/* 
 * Select routing metric supported at runtime. This must be a valid
 * DAG Metric Container Object Type (see below). Currently, we only 
 * support RPL_DAG_MC_ETX and RPL_DAG_MC_ENERGY.
 * When MRHOF (RFC6719) is used with ETX, no metric container must
 * be used; instead the rank carries ETX directly.
 */
#ifdef RPL_CONF_DAG_MC
#define RPL_DAG_MC RPL_CONF_DAG_MC
#else
#define RPL_DAG_MC RPL_DAG_MC_NONE
#endif /* RPL_CONF_DAG_MC */

/*
 * The objective function used by RPL is configurable through the 
 * RPL_CONF_OF parameter. This should be defined to be the name of an 
 * rpl_of object linked into the system image, e.g., rpl_of0.
 */
#ifdef RPL_CONF_OF
#define RPL_OF RPL_CONF_OF
#else
/* ETX is the default objective function. */
#define RPL_OF rpl_mrhof
#endif /* RPL_CONF_OF */

/* This value decides which DAG instance we should participate in by default. */
#ifdef RPL_CONF_DEFAULT_INSTANCE
#define RPL_DEFAULT_INSTANCE RPL_CONF_DEFAULT_INSTANCE
#else
#define RPL_DEFAULT_INSTANCE	       0x1e
#endif /* RPL_CONF_DEFAULT_INSTANCE */

/*
 * This value decides if this node must stay as a leaf or not
 * as allowed by draft-ietf-roll-rpl-19#section-8.5
 */
#ifdef RPL_CONF_LEAF_ONLY
#define RPL_LEAF_ONLY RPL_CONF_LEAF_ONLY
#else
#define RPL_LEAF_ONLY 0
#endif

/*
 * Maximum of concurent RPL instances.
 */
#ifdef RPL_CONF_MAX_INSTANCES
#define RPL_MAX_INSTANCES     RPL_CONF_MAX_INSTANCES
#else
#define RPL_MAX_INSTANCES     1
#endif /* RPL_CONF_MAX_INSTANCES */

/*
 * Maximum number of DAGs within an instance.
 */
#ifdef RPL_CONF_MAX_DAG_PER_INSTANCE
#define RPL_MAX_DAG_PER_INSTANCE     RPL_CONF_MAX_DAG_PER_INSTANCE
#else
#define RPL_MAX_DAG_PER_INSTANCE     1
#endif /* RPL_CONF_MAX_DAG_PER_INSTANCE */

//Configure the max number of RPL parents for all dags and instances
#ifndef RPL_CONF_MAX_PARENTS
#ifdef  RPL_CONF_MAX_PARENTS_PER_DAG            
#define RPL_CONF_MAX_PARENTS    (RPL_CONF_MAX_PARENTS_PER_DAG*RPL_MAX_DAG_PER_INSTANCE*RPL_MAX_INSTANCES)    //If we configured the max number of parents per dag, use that
#else
#ifdef NBR_TABLE_CONF_MAX_NEIGHBORS
#define RPL_CONF_MAX_PARENTS    NBR_TABLE_CONF_MAX_NEIGHBORS    //Contiki 2.7 would use the max number of possible parents as the max number of neighbors
#else
#define RPL_CONF_MAX_PARENTS    5
#endif 
#endif
#endif

#ifndef RPL_CONF_DAO_SPECIFY_DAG
#if RPL_MAX_DAG_PER_INSTANCE > 1
#define RPL_DAO_SPECIFY_DAG 1
#else
#define RPL_DAO_SPECIFY_DAG 0
#endif /* RPL_MAX_DAG_PER_INSTANCE > 1 */
#else
#define RPL_DAO_SPECIFY_DAG RPL_CONF_DAO_SPECIFY_DAG
#endif /* RPL_CONF_DAO_SPECIFY_DAG */

/*
 * The DIO interval (n) represents 2^n ms.
 *
 * According to the specification, the default value is 3 which
 * means 8 milliseconds. That is far too low when using duty cycling
 * with wake-up intervals that are typically hundreds of milliseconds.
 * ContikiRPL thus sets the default to 2^12 ms = 4.096 s.
 */
#ifdef RPL_CONF_DIO_INTERVAL_MIN
#define RPL_DIO_INTERVAL_MIN        RPL_CONF_DIO_INTERVAL_MIN
#else
#define RPL_DIO_INTERVAL_MIN        12
#endif

/*
 * Maximum amount of timer doublings.
 *
 * The maximum interval will by default be 2^(12+8) ms = 1048.576 s.
 * RFC 6550 suggests a default value of 20, which of course would be
 * unsuitable when we start with a minimum interval of 2^12.
 */
#ifdef RPL_CONF_DIO_INTERVAL_DOUBLINGS
#define RPL_DIO_INTERVAL_DOUBLINGS  RPL_CONF_DIO_INTERVAL_DOUBLINGS
#else
#define RPL_DIO_INTERVAL_DOUBLINGS  8
#endif

/*
 * DIO redundancy. To learn more about this, see RFC 6206.
 *
 * RFC 6550 suggests a default value of 10. It is unclear what the basis
 * of this suggestion is. Network operators might attain more efficient
 * operation by tuning this parameter for specific deployments.
 */
#ifdef RPL_CONF_DIO_REDUNDANCY
#define RPL_DIO_REDUNDANCY          RPL_CONF_DIO_REDUNDANCY
#else
#define RPL_DIO_REDUNDANCY          10
#endif

/*
 * Initial metric attributed to a link when the ETX is unknown
 */
#ifndef RPL_CONF_INIT_LINK_METRIC
#define RPL_INIT_LINK_METRIC        5
#else
#define RPL_INIT_LINK_METRIC        RPL_CONF_INIT_LINK_METRIC
#endif

#ifndef RPL_ROUTE_INFINITE_LIFETIME
#define RPL_ROUTE_INFINITE_LIFETIME     0
#endif

#if RPL_ROUTE_INFINITE_LIFETIME

#define RPL_DEFAULT_LIFETIME_UNIT       0xffff
#define RPL_DEFAULT_LIFETIME            0xff

#else
/*
 * Default route lifetime unit. This is the granularity of time
 * used in RPL lifetime values, in seconds.
 */
#ifndef RPL_CONF_DEFAULT_LIFETIME_UNIT
#define RPL_DEFAULT_LIFETIME_UNIT       0xffff
#else
#define RPL_DEFAULT_LIFETIME_UNIT       RPL_CONF_DEFAULT_LIFETIME_UNIT
#endif   

/*
 * Default route lifetime as a multiple of the lifetime unit.
 */
#ifndef RPL_CONF_DEFAULT_LIFETIME
#define RPL_DEFAULT_LIFETIME            0xff
#else
#define RPL_DEFAULT_LIFETIME            RPL_CONF_DEFAULT_LIFETIME
#endif

#endif //RPL_ROUTE_INFINITE_LIFETIME

/* The default value for the DAO timer. */
#ifdef RPL_CONF_DAO_LATENCY
#define RPL_DAO_LATENCY                 RPL_CONF_DAO_LATENCY
#else /* RPL_CONF_DAO_LATENCY */
#define RPL_DAO_LATENCY                 (CLOCK_SECOND * 4)    
#endif /* RPL_DAO_LATENCY */

/* The default value for the DAO ACK timer delay. */
#ifdef RPL_CONF_DAO_ACK_INITIAL_LATENCY
#define RPL_DAO_ACK_INITIAL_LATENCY                 RPL_CONF_DAO_ACK_INITIAL_LATENCY
#else /* RPL_CONF_DAO_LATENCY */
#define RPL_DAO_ACK_INITIAL_LATENCY                 (CLOCK_SECOND * 6)    
#endif /* RPL_DAO_LATENCY */

/* The default value for the DAO retries (if no DAO Ack). */
#ifdef RPL_CONF_DAO_ACK_MAX_TRIES
#define RPL_DAO_ACK_MAX_TRIES                 RPL_CONF_DAO_ACK_MAX_TRIES
#else /* RPL_DAO_ACK_MAX_TRIES */
#define RPL_DAO_ACK_MAX_TRIES                 3    
#endif /* RPL_DAO_ACK_MAX_TRIES */

/* Respond with DAO ACK or not */
#if RPL_SUPPRESS_DAO_ACK_CONF  
#undef RPL_CONF_DAO_ACK
#define RPL_CONF_DAO_ACK        0   
#else

#ifndef RPL_CONF_DAO_ACK
#define RPL_CONF_DAO_ACK        0
#endif

#endif

#ifndef RPL_CONF_DODAG_PREFERENCE
#define RPL_DODAG_PREFERENCE    0
#else
#define RPL_DODAG_PREFERENCE    RPL_CONF_DODAG_PREFERENCE
#endif

#ifdef  RPL_CONF_MOP
#define RPL_MOP_DEFAULT                 RPL_CONF_MOP
#else
#define RPL_MOP_DEFAULT                 RPL_MOP_STORING_NO_MULTICAST
#endif

/* DIS related */
#ifdef	RPL_DIS_SEND_CONF
#define RPL_DIS_SEND			RPL_DIS_SEND_CONF
#else
#define RPL_DIS_SEND			1
#endif

#ifdef  RPL_DIS_INTERVAL_CONF
#define RPL_DIS_INTERVAL                RPL_DIS_INTERVAL_CONF
#else
#define RPL_DIS_INTERVAL                60
#endif

#ifdef RPL_DIS_FIXED_START_DELAY_CONF //Fixed starting delay for DIS, no randomness
#define RPL_DIS_FIXED_START_DELAY       RPL_DIS_FIXED_START_DELAY_CONF
#endif

/*----------------------------------------------------------------------------*/
#ifndef RPL_DISABLE_HBHO_CONF
#define USE_RPL_HBHO_HEADER			1 //Use RPL Option header by default
#else
#define USE_RPL_HBHO_HEADER			!RPL_DISABLE_HBHO_CONF
#endif

/*----------------------------------------------------------------------------*/

#ifndef RPL_ICMP6_SEND_FUNC
#define RPL_ICMP6_SEND_FUNC          uip_icmp6_send //Just call the uIPv6 ICMPv6 send function. No context switch
#endif

/*----------------------------------------------------------------------------*/
//Configure these to save code space!
#if RPL_MAX_INSTANCES == 1
#define RPL_SINGLE_INSTANCE     1
#else
#define RPL_SINGLE_INSTANCE     0
#endif

#if RPL_MAX_DAG_PER_INSTANCE == 1
#define RPL_SINGLE_DAG  1
#else
#define RPL_SINGLE_DAG  0
#endif

/*----------------------------------------------------------------------------*/
//RPL++ configuration
#ifndef RPL_PLUSPLUS_CONF
#define RPL_PLUSPLUS_CONF       1
#endif

#ifndef RPL_PLUSPLUS_MODIFY_DAO_INTERVAL_CONF
#define RPL_PLUSPLUS_MODIFY_DAO_INTERVAL_CONF   1
#endif

#ifndef RPL_PLUSPLUS_PROBE_IF_NO_ACK_CONF
#define RPL_PLUSPLUS_PROBE_IF_NO_ACK_CONF       1
#endif

#ifndef RPL_PLUSPLUS_PERIODIC_PARENT_CHECK_CONF
#define RPL_PLUSPLUS_PERIODIC_PARENT_CHECK_CONF 1
#endif

#ifndef RPL_PLUSPLUS_DAO_UPDATE_ROUTES_CONF
#define RPL_PLUSPLUS_DAO_UPDATE_ROUTES_CONF     1
#endif

#ifndef RPL_PLUSPLUS_NEXT_HOP_DEAD_CONF
#define RPL_PLUSPLUS_NEXT_HOP_DEAD_CONF         1
#endif

/*----------------------------------------------------------------------------*/

#ifndef RPL_PLUSPLUS_DTSN_STRATEGY_CONF
#define RPL_PLUSPLUS_DTSN_STRATEGY_CONF         1
#endif

/*----------------------------------------------------------------------------*/
//uIPv6 Configuration for RPL to work!   
#if (RPL_CONF_MOP == RPL_MOP_NON_STORING)

#if NON_STORING_MODE_ROOT
#define UIP_DS6_ROUTING_MODE_CONF            UIP_DS6_SOURCE_ROUTING
#else
#define UIP_DS6_ROUTING_MODE_CONF            UIP_DS6_NO_ROUTING
#endif

#else //(RPL_CONF_MOP != RPL_MOP_NON_STORING)

#define UIP_DS6_ROUTING_MODE_CONF            UIP_DS6_STANDARD_ROUTING

#endif

//Define the uIPv6 function implementation for RPL Hop By Hop header extension
#if USE_RPL_HBHO_HEADER
#define UIP_HBHO_HDR_EXTENSION_FUNC             rpl_ipv6_hbho_header_extension
#endif

//Define the uIPv6 function implementation for RPL Source Routing header extension
#if (RPL_CONF_MOP == RPL_MOP_NON_STORING)
#define UIP_SRC_ROUTING_HDR_EXTENSION_FUNC      rpl_ipv6_routing_header_extension
#endif

//define some additional RPL related route state and
//neighbor callback for RPL - if not a DS6_ROUTE_STATE is already set 
#define UIP_DS6_ROUTE_STATE_TYPE rpl_route_entry_t
// Needed for the extended route entry state when using ContikiRPL 
typedef struct rpl_route_entry
{
#if !RPL_ROUTE_INFINITE_LIFETIME
    uint32_t lifetime;
#endif
    void *dag;
    uint16_t learned_from :8;
    uint16_t nopath_received :8;
} rpl_route_entry_t;

/*----------------------------------------------------------------------------*/
//SET uIPv6 Callbacks defined by RPL
#ifndef UIP_ICMP6_INPUT_INDICATION_FUNC
#define UIP_ICMP6_INPUT_INDICATION_FUNC         rpl_ipv6_icmp6_input
#endif

#ifndef UIP_TX_CONFIRMATION_FUNC
#define UIP_TX_CONFIRMATION_FUNC                rpl_ipv6_tx_confirmation
#endif

#ifndef UIP_DS6_NOTIFICATION_FUNC
#define UIP_DS6_NOTIFICATION_FUNC         		rpl_ipv6_ds6_notification
#endif

/*----------------------------------------------------------------------------*/
//RPL Statistics: Chose the ones that are interesting and define the incrementation of a suitable variable
//By default, no statistics are used
#ifndef UIP_STAT_RPL_PREFERRED_PARENT_SET_NUM_INC 
#define UIP_STAT_RPL_PREFERRED_PARENT_SET_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_PREFERRED_PARENT_SWITCH_NUM_INC 
#define UIP_STAT_RPL_PREFERRED_PARENT_SWITCH_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_POTENTIAL_PARENT_ADDED_NUM_INC 
#define UIP_STAT_RPL_POTENTIAL_PARENT_ADDED_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_POTENTIAL_PARENT_NOT_ADDED_NUM_INC 
#define UIP_STAT_RPL_POTENTIAL_PARENT_NOT_ADDED_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_POTENTIAL_PARENT_DELETED_NUM_INC 
#define UIP_STAT_RPL_POTENTIAL_PARENT_DELETED_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_LOCAL_REPAIR_NUM_INC 
#define UIP_STAT_RPL_LOCAL_REPAIR_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_GLOBAL_REPAIR_NUM_INC 
#define UIP_STAT_RPL_GLOBAL_REPAIR_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_DIO_RESET_NUM_INC 
#define UIP_STAT_RPL_DIO_RESET_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_DAO_SCHEDULED_NUM_INC 
#define UIP_STAT_RPL_DAO_SCHEDULED_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_DAO_LOWER_RANK_NUM_INC 
#define UIP_STAT_RPL_DAO_LOWER_RANK_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_DAO_FROM_PARENT_NUM_INC 
#define UIP_STAT_RPL_DAO_FROM_PARENT_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_FORWARDING_ERROR_NUM_INC 
#define UIP_STAT_RPL_FORWARDING_ERROR_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_ADDING_FORWARDING_ERROR_NUM_INC 
#define UIP_STAT_RPL_ADDING_FORWARDING_ERROR_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_LOOP_ERROR_NUM_INC 
#define UIP_STAT_RPL_LOOP_ERROR_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_WARNING_ERROR_NUM_INC 
#define UIP_STAT_RPL_WARNING_ERROR_NUM_INC 
#endif

#ifndef UIP_STAT_RPL_MEM_OVERFLOW_NUM_INC
#define UIP_STAT_RPL_MEM_OVERFLOW_NUM_INC
#endif

#ifndef UIP_STAT_RX_DIO_NUM_INC 
#define UIP_STAT_RX_DIO_NUM_INC 
#endif

#ifndef UIP_STAT_RX_DIS_NUM_INC 
#define UIP_STAT_RX_DIS_NUM_INC 
#endif

#ifndef UIP_STAT_RX_DAO_NUM_INC 
#define UIP_STAT_RX_DAO_NUM_INC 
#endif

#ifndef UIP_STAT_RX_DAO_ACK_NUM_INC 
#define UIP_STAT_RX_DAO_ACK_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_MALFORMED_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_MALFORMED_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_UNSUPPORTED_MC_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_UNSUPPORTED_MC_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_UNSUPPORTED_MOP_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_UNSUPPORTED_MOP_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_INCONSISTENT_VERSION_NUMBER_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_INCONSISTENT_VERSION_NUMBER_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_INCONSISTENT_OLD_VERSION_NUMBER_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_INCONSISTENT_OLD_VERSION_NUMBER_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_DIFFERENT_DAG_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_DIFFERENT_DAG_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_NO_OF_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_NO_OF_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_INFINITE_RANK_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_INFINITE_RANK_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_HIGHEROREQUAL_RANK_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_HIGHEROREQUAL_RANK_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_LOWER_RANK_CHILD_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_LOWER_RANK_CHILD_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DIO_POTENTIAL_PARENT_REJECTED_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DIO_POTENTIAL_PARENT_REJECTED_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DAO_UNKNOWN_INSTANCE_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DAO_UNKNOWN_INSTANCE_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DAO_DIFFERENT_DAG_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DAO_DIFFERENT_DAG_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DAO_NO_PATH_NON_STORING_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DAO_NO_PATH_NON_STORING_NUM_INC 
#endif

#ifndef UIP_STAT_RX_RPL_DROP_DAO_MULTICAST_NON_STORING_NUM_INC 
#define UIP_STAT_RX_RPL_DROP_DAO_MULTICAST_NON_STORING_NUM_INC 
#endif

#ifndef UIP_STAT_TX_DIO_NUM_INC 
#define UIP_STAT_TX_DIO_NUM_INC 
#endif

#ifndef UIP_STAT_TX_DIS_NUM_INC 
#define UIP_STAT_TX_DIS_NUM_INC 
#endif

#ifndef UIP_STAT_TX_DAO_NUM_INC 
#define UIP_STAT_TX_DAO_NUM_INC 
#endif

#ifndef UIP_STAT_TX_DAO_ACK_NUM_INC 
#define UIP_STAT_TX_DAO_ACK_NUM_INC 
#endif

#endif /* RPL_CONF_H */
