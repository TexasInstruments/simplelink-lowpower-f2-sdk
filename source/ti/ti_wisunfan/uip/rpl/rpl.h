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
 *	Public API declarations for ContikiRPL.
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

#ifndef RPL_H
#define RPL_H

#include "uip-conf.h"

#include "rpl/rpl_ipv6_api.h"
#include "rpl/rpl_api.h"

#include "lib/list.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "sys/ctimer.h"

#define ICMP6_RPL                       155  /**< RPL */

/*---------------------------------------------------------------------------*/
/* The amount of parents that this node has in a particular DAG. */
#define RPL_PARENT_COUNT(dag)   list_length((dag)->parents)
/*---------------------------------------------------------------------------*/
typedef uint16_t rpl_rank_t;
typedef uint16_t rpl_ocp_t;
/*---------------------------------------------------------------------------*/
/* DAG Metric Container Object Types, to be confirmed by IANA. */
#define RPL_DAG_MC_NONE			0 /* Local identifier for empty MC */
#define RPL_DAG_MC_NSA                  1 /* Node State and Attributes */
#define RPL_DAG_MC_ENERGY               2 /* Node Energy */
#define RPL_DAG_MC_HOPCOUNT             3 /* Hop Count */
#define RPL_DAG_MC_THROUGHPUT           4 /* Throughput */
#define RPL_DAG_MC_LATENCY              5 /* Latency */
#define RPL_DAG_MC_LQL                  6 /* Link Quality Level */
#define RPL_DAG_MC_ETX                  7 /* Expected Transmission Count */
#define RPL_DAG_MC_LC                   8 /* Link Color */

/* DAG Metric Container flags. */
#define RPL_DAG_MC_FLAG_P               0x8
#define RPL_DAG_MC_FLAG_C               0x4
#define RPL_DAG_MC_FLAG_O               0x2
#define RPL_DAG_MC_FLAG_R               0x1

/* DAG Metric Container aggregation mode. */
#define RPL_DAG_MC_AGGR_ADDITIVE        0
#define RPL_DAG_MC_AGGR_MAXIMUM         1
#define RPL_DAG_MC_AGGR_MINIMUM         2
#define RPL_DAG_MC_AGGR_MULTIPLICATIVE  3

/* The bit index within the flags field of
 the rpl_metric_object_energy structure. */
#define RPL_DAG_MC_ENERGY_INCLUDED	3
#define RPL_DAG_MC_ENERGY_TYPE		1
#define RPL_DAG_MC_ENERGY_ESTIMATION	0

#define RPL_DAG_MC_ENERGY_TYPE_MAINS		0
#define RPL_DAG_MC_ENERGY_TYPE_BATTERY		1
#define RPL_DAG_MC_ENERGY_TYPE_SCAVENGING	2

#define UIP_ROUTING_HDR_RPL_TYPE	3

#define UIP_ROUTING_HDR_RPL_INITIAL_LEN		CHAR_BIT_UNITS(8) //8 bytes are the length of the header without any address added

#define UIP_EXT_HDR_OPT_RPL   0x63

#if UIP_BYTE_ORDER == UIP_LITTLE_ENDIAN

//RPL Routing header for Non-Storing mode
typedef struct uip_routing_hdr_rpl
{
    u16_t next :8;
    u16_t len :8;
    u16_t routing_type :8;
    u16_t seg_left :8;
    u16_t cmprI_cmprE :8;
    u16_t pad_rsrv :8;
    u16_t rsrv0 :8;
    u16_t rsrv1 :8;
} uip_routing_hdr_rpl;

/* RPL option */
typedef struct uip_ext_hdr_opt_rpl
{
    u16_t opt_type :8;
    u16_t opt_len :8;
    u16_t flags :8;
    u16_t instance :8;
    u16_t senderrank;
} uip_ext_hdr_opt_rpl;

#else //UIP_BYTE_ORDER == UIP_BIG_ENDIAN

//RPL Routing header for Non-Storing mode
typedef struct uip_routing_hdr_rpl
{
    u16_t len:8;
    u16_t next:8;

    u16_t seg_left:8;
    u16_t routing_type:8;

    u16_t pad_rsrv:8;
    u16_t cmprI_cmprE:8;

    u16_t rsrv1:8;
    u16_t rsrv0:8;
}uip_routing_hdr_rpl;

/* RPL option */
typedef struct uip_ext_hdr_opt_rpl
{
    u16_t opt_len:8;
    u16_t opt_type:8;

    u16_t instance:8;
    u16_t flags:8;

    u16_t senderrank;
}uip_ext_hdr_opt_rpl;

#endif

struct rpl_metric_object_energy
{
    uint16_t flags :8;
    uint16_t energy_est :8;
};

/* Logical representation of a DAG Metric Container. */
struct rpl_metric_container
{
    uint16_t type :8;
    uint16_t flags :8;
    uint16_t aggr :8;
    uint16_t prec :8;
    uint16_t length;
    union metric_object
    {
        struct rpl_metric_object_energy energy;
        uint16_t etx;
    } obj;
};
typedef struct rpl_metric_container rpl_metric_container_t;
/*---------------------------------------------------------------------------*/
struct rpl_instance;
struct rpl_dag;
/*---------------------------------------------------------------------------*/
struct rpl_parent
{
    struct rpl_parent *next;
    struct rpl_dag *dag;
#if RPL_DAG_MC != RPL_DAG_MC_NONE
    rpl_metric_container_t mc;
#endif /* RPL_DAG_MC != RPL_DAG_MC_NONE */
    rpl_rank_t rank;
    uint16_t link_metric;
    uint16_t dtsn :8;
    uint16_t updated :8;
    uint16_t router_address_valid;
    uip_ipaddr_t router_address;
    uip_ds6_nbr_t * nbr; //The RPL parent corresponds to a certain neighbor in the neighbor table
};
//typedef struct rpl_parent rpl_parent_t;
/*---------------------------------------------------------------------------*/
/* RPL DIO prefix suboption */
struct rpl_prefix
{
    uip_ipaddr_t prefix;
    uint32_t lifetime;
    uint16_t length :8;
    uint16_t flags :8;
};
typedef struct rpl_prefix rpl_prefix_t;
/*---------------------------------------------------------------------------*/
/* Directed Acyclic Graph */
struct rpl_dag
{
    uip_ipaddr_t dag_id;
    rpl_rank_t min_rank; /* should be reset per DAG iteration! */
    uint16_t version :8;
    uint16_t grounded :8;
    uint16_t preference :8;
    uint16_t used :4;
    /* live data for the DAG */
    uint16_t joined :4;
    rpl_parent_t *preferred_parent;
    rpl_rank_t rank;
    struct rpl_instance *instance;
    rpl_prefix_t prefix_info;
};
typedef struct rpl_dag rpl_dag_t;
//typedef struct rpl_instance rpl_instance_t;
/*---------------------------------------------------------------------------*/
/*
 * API for RPL objective functions (OF)
 *
 * reset(dag)
 *
 *  Resets the objective function state for a specific DAG. This function is
 *  called when doing a global repair on the DAG.
 *
 * neighbor_link_callback(parent, known, etx)
 *
 *  Receives link-layer neighbor information. The parameter "known" is set
 *  either to 0 or 1. The "etx" parameter specifies the current
 *  ETX(estimated transmissions) for the neighbor.
 *
 * best_parent(parent1, parent2)
 *
 *  Compares two parents and returns the best one, according to the OF.
 *
 * best_dag(dag1, dag2)
 *
 *  Compares two DAGs and returns the best one, according to the OF.
 *
 * calculate_rank(parent, base_rank)
 *
 *  Calculates a rank value using the parent rank and a base rank.
 *  If "parent" is NULL, the objective function selects a default increment
 *  that is adds to the "base_rank". Otherwise, the OF uses information known
 *  about "parent" to select an increment to the "base_rank".
 *
 * update_metric_container(dag)
 *
 *  Updates the metric container for outgoing DIOs in a certain DAG.
 *  If the objective function of the DAG does not use metric containers, 
 *  the function should set the object type to RPL_DAG_MC_NONE.
 */
struct rpl_of
{
    void (*reset)(struct rpl_dag *);
    void (*neighbor_link_callback)(rpl_parent_t *, int, int);
    rpl_parent_t *(*best_parent)(rpl_parent_t *, rpl_parent_t *);
    rpl_dag_t *(*best_dag)(rpl_dag_t *, rpl_dag_t *);
    rpl_rank_t (*calculate_rank)(rpl_parent_t *, rpl_rank_t);
    void (*update_metric_container)(rpl_instance_t *);
    rpl_ocp_t ocp;
};
typedef struct rpl_of rpl_of_t;
/*---------------------------------------------------------------------------*/
/* Instance */
struct rpl_instance
{
    /* DAG configuration */
    rpl_metric_container_t mc;
    rpl_of_t *of;
    rpl_dag_t *current_dag;
#if RPL_SINGLE_DAG
    rpl_dag_t single_dag;
#else
    rpl_dag_t dag_table[RPL_MAX_DAG_PER_INSTANCE];
#endif
    /* The current default router - used for routing "upwards" */
    uip_ds6_defrt_t *def_route;
    uint16_t instance_id :8;
    uint16_t used :8;
    uint16_t dtsn_out :8;
    uint16_t mop :8;
    uint16_t dio_intdoubl :8;
    uint16_t dio_intmin :8;
    uint16_t dio_redundancy :8;
    uint16_t default_lifetime :8;
    uint16_t dio_intcurrent :8;
    uint16_t dio_send :8; /* for keeping track of which mode the timer is in */
    uint16_t dio_counter :8;
    uint16_t dao_reemissions :8;
    rpl_rank_t max_rankinc;
    rpl_rank_t min_hoprankinc;
    uint16_t lifetime_unit; /* lifetime in seconds = l_u * d_l */
#if RPL_CONF_STATS
    uint16_t dio_totint;
    uint16_t dio_totsend;
    uint16_t dio_totrecv;
#endif /* RPL_CONF_STATS */
    clock_time_t dio_next_delay; /* delay for completion of dio interval */
    struct ctimer dio_timer;
    struct ctimer dao_timer;
#if !RPL_ROUTE_INFINITE_LIFETIME
    struct ctimer dao_lifetime_timer;
#endif
    clock_time_t dao_latency;
    uint16_t dao_sequence;
#if RPL_CONF_DAO_ACK
    struct ctimer dao_ack_timer;
    uint16_t dao_ack_tries:8;
    uint16_t dao_ack_is_retry:8;
    rpl_parent_t* last_dao_dest;
    uip_ipaddr_t last_dao_target;
    uint16_t last_dao_lifetime;
#endif
};

#define rpl_process_handler(ev,data)    ctimer_process_handler(ev,data)

/*----------------------------------------------------------------------------*/

#ifdef RPL_ACCEPT_DIO_FROM_FUNC
#define RPL_ACCEPT_DIO_FROM(_lladdr) RPL_ACCEPT_DIO_FROM_FUNC(_lladdr)
int RPL_ACCEPT_DIO_FROM(rimeaddr_t* target_lladdr);
#else
#define RPL_ACCEPT_DIO_FROM(_lladdr)    1 //Always accept
#endif 

#ifdef RPL_EVENT_NOTIFICATION_FUNC
#define RPL_EVENT_NOTIFICATION(_e,_n) RPL_EVENT_NOTIFICATION_FUNC(_e,_n)
void RPL_EVENT_NOTIFICATION(int event, uint16_t instance_id);
#else
#define RPL_EVENT_NOTIFICATION(_e,_n)   
#endif 

#define RPL_ICMP6_SEND(_addr, _code, _length) RPL_ICMP6_SEND_FUNC(_addr, ICMP6_RPL, _code, _length)
void RPL_ICMP6_SEND_FUNC(uip_ipaddr_t *dest, int type, int code,
                         int payload_len);

/*----------------------------------------------------------------------------*/
/* Public RPL functions. */
void rpl_init(
        void (*rpl_process_post_func)(process_event_t event,
                                      process_data_t data));
int rpl_ipv6_icmp6_input(uint16_t type);
void rpl_ipv6_ds6_notification(int event, void *data);
void rpl_ipv6_tx_confirmation(const rimeaddr_t *addr, int status, int numtx);
#if UIP_RPL_ROOT
int rpl_repair_root(uint16_t instance_id);
void rpl_start_instance(uip_ipaddr_t *prefix);
#endif
rpl_instance_t *rpl_get_default_instance();
rpl_instance_t *rpl_get_instance(uint16_t instance_id);
void rpl_dtsn_increment(rpl_instance_t *instance);
void rpl_reset_dio_timer(rpl_instance_t *instance);
rpl_parent_t *rpl_parents_head();
rpl_parent_t *rpl_parents_next(rpl_parent_t * entry);
//Hop By Hop Option extension header functions
int rpl_ipv6_hbho_header_extension(int action, void *data);
//Soruce Routing extension header functions
int rpl_ipv6_routing_header_extension(int action, void *data);
//Free the instances and the memory (if needed)
void rpl_clean();
/*----------------------------------------------------------------------------*/
int rpl_set_prefix(rpl_dag_t *dag, uip_ipaddr_t *prefix, unsigned len);
#if UIP_RPL_ROOT
rpl_dag_t *rpl_set_root(uint16_t instance_id, uip_ipaddr_t * dag_id);
#endif
int rpl_set_default_route(rpl_instance_t *instance, uip_ipaddr_t *from);
rpl_dag_t *rpl_get_any_dag(void);
uip_ipaddr_t *rpl_get_parent_ipaddr(rpl_parent_t *nbr);
rpl_rank_t rpl_get_parent_rank(uip_lladdr_t *addr);
uint16_t rpl_get_parent_link_metric(const uip_lladdr_t *addr);
void rpl_dag_init(void);

/*---------------------------------------------------------------------------*/
#endif /* RPL_H */
