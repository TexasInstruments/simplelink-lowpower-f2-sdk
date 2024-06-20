/**
 * \addtogroup uip6
 * @{
 */
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
 */
/**
 * \file
 *         RPL timer management.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
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

#include "uip-conf.h"
#include "rpl/rpl-private.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "rpl/rpl-plusplus.h"

#if UIP_CONF_IPV6

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

/*---------------------------------------------------------------------------*/

static struct ctimer periodic_timer;
static void handle_dio_timer(void *ptr);

void handle_rpl_periodic(void *ptr);
static void new_dio_interval(rpl_instance_t *instance);

#if !RPL_SUPPRESS_DIS_CONF
static uint16_t next_dis;
#endif

/* dio_send_ok is true if the node is ready to send DIOs */
static uint16_t dio_send_ok;

/*---------------------------------------------------------------------------*/
void handle_rpl_periodic(void *ptr)
{
#if !RPL_ROUTE_INFINITE_LIFETIME
    rpl_purge_routes();
#endif
    rpl_recalculate_ranks();

#if RPL_PLUSPLUS_PERIODIC_PARENT_CHECK
    rpl_periodic_parent_check(default_instance);
#endif

#if !RPL_SUPPRESS_DIS_CONF
    /* handle DIS */
    if (RPL_DIS_SEND)
    {
        next_dis++;
        if(rpl_get_any_dag() == NULL && next_dis >= RPL_DIS_INTERVAL)
        {
            next_dis = 0;
            dis_output(NULL);
        }
    }
#endif

    ctimer_reset(&periodic_timer);
}
/************************************************************************/
static void new_dio_interval(rpl_instance_t *instance)
{
    uint32_t time;
    clock_time_t ticks;

    /* TODO: too small timer intervals for many cases */
    time = 1UL << instance->dio_intcurrent;

    /* Convert from milliseconds to CLOCK_TICKS. */
    ticks = (clock_time_t) ((clock_time_t) (time / 1000) * CLOCK_SECOND);
    instance->dio_next_delay = ticks;

    /* random number between I/2 and I */
    ticks = (ticks >> 1) + (random_rand() % (ticks >> 1));

    /*
     * The intervals must be equally long among the nodes for Trickle to
     * operate efficiently. Therefore we need to calculate the delay between
     * the randomized time and the start time of the next interval.
     */
    instance->dio_next_delay -= ticks;
    instance->dio_send = 1;

    /* reset the redundancy counter */
    instance->dio_counter = 0;

    /* schedule the timer */
    PRINTF("RPL: Scheduling DIO timer %lu ticks in future (Interval)\n", ticks);

    ctimer_set(&instance->dio_timer, ticks, &handle_dio_timer, instance);
}
/************************************************************************/
static void handle_dio_timer(void *ptr)
{
    rpl_instance_t *instance;

    instance = (rpl_instance_t *) ptr;

    LOG_INFO0("RPL: DIO Timer triggered\n");
    if (!dio_send_ok)
    {
        if (uip_ds6_get_link_local(ADDR_PREFERRED) != NULL)
        {
            dio_send_ok = 1;
        }
        else
        {
            LOG_INFO0("RPL: Postponing DIO transmission since link local address is not ok\n");
            ctimer_set(&instance->dio_timer, CLOCK_SECOND, &handle_dio_timer,
                       instance);
            return;
        }
    }

    if (instance->dio_send)
    {
        /* send DIO if counter is less than desired redundancy */
        if (instance->dio_counter < instance->dio_redundancy)
        {
#if RPL_CONF_STATS
            instance->dio_totsend++;
#endif /* RPL_CONF_STATS */
            dio_output(instance, NULL);
        }
        else
        {
            LOG_INFO2("RPL: Suppressing DIO transmission (%d >= %d)\n",
                    instance->dio_counter, instance->dio_redundancy);
        }
        instance->dio_send = 0;
        LOG_INFO1("RPL: Scheduling DIO timer %lu ticks in future (sent)\n",
                instance->dio_next_delay);
        ctimer_set(&instance->dio_timer, instance->dio_next_delay,
                   handle_dio_timer, instance);
    }
    else
    {
        /* check if we need to double interval */
        if (instance->dio_intcurrent
                < instance->dio_intmin + instance->dio_intdoubl)
        {
            instance->dio_intcurrent++;
            PRINTF("RPL: DIO Timer interval doubled %d\n", instance->dio_intcurrent);
        }
        new_dio_interval(instance);
    }
}
/*---------------------------------------------------------------------------*/
void rpl_reset_periodic_timer(void)
{
#if !RPL_SUPPRESS_DIS_CONF
    next_dis = RPL_DIS_FIXED_START_DELAY;
#endif
    /*
     next_dis = RPL_DIS_INTERVAL / 2 +
     ((uint32_t)(RPL_DIS_INTERVAL/2) * (uint32_t)random_rand()) / RANDOM_RAND_MAX ;
     */

    ctimer_set(&periodic_timer, CLOCK_SECOND, handle_rpl_periodic, NULL);
}

/*Stop RPL periodic timer*/
void rpl_stop_periodic_timer(void)
{
    ctimer_stop(&periodic_timer);
}
/*---------------------------------------------------------------------------*/
/* Resets the DIO timer in the instance to its minimal interval. */
void rpl_reset_dio_timer(rpl_instance_t *instance)
{
#if !RPL_LEAF_ONLY
    /* Do not reset if we are already on the minimum interval,
     unless forced to do so. */
    if (instance->dio_intcurrent > instance->dio_intmin)
    {
        instance->dio_counter = 0;
        instance->dio_intcurrent = instance->dio_intmin;
        new_dio_interval(instance);
    } UIP_STAT_RPL_DIO_RESET_NUM_INC;
#if RPL_CONF_STATS
    rpl_stats.resets++;
#endif /* RPL_CONF_STATS */
#endif /* RPL_LEAF_ONLY */
}
/************************************************************************/

static void handle_dao_timer(void *ptr);

#if !RPL_ROUTE_INFINITE_LIFETIME
static void set_dao_lifetime_timer(rpl_instance_t *instance)
{

    /* Set up another DAO within half the expiration time, if such a
     time has been configured */
    if (instance->lifetime_unit != 0xffff && instance->default_lifetime != 0xff)
    {
        clock_time_t expiration_time;
        expiration_time = (clock_time_t) instance->default_lifetime
                * (clock_time_t) instance->lifetime_unit *
                CLOCK_SECOND / 4; //2;
        PRINTF("RPL: Scheduling DAO lifetime timer %u ticks in the future\n",
                (unsigned)expiration_time);
        ctimer_set(&instance->dao_lifetime_timer, expiration_time,
                   handle_dao_timer, instance);
    }
}
#endif
/*---------------------------------------------------------------------------*/
static void handle_dao_timer(void *ptr)
{
    rpl_instance_t *instance;

    instance = (rpl_instance_t *) ptr;

    if (!dio_send_ok && uip_ds6_get_link_local(ADDR_PREFERRED) == NULL)
    {
        LOG_INFO("RPL: Postpone DAO transmission\n");
        ctimer_set(&instance->dao_timer, CLOCK_SECOND, handle_dao_timer,
                   instance);
        return;
    }

    /* Send the DAO to the DAO parent set -- the preferred parent in our case. */
    if (instance->current_dag->preferred_parent != NULL)
    {
        PRINTF("RPL: handle_dao_timer - sending DAO\n");
        /* Set the route lifetime to the default value. */
        dao_output(instance->current_dag->preferred_parent,
                   instance->default_lifetime);
    }
    else
    {
        PRINTF("RPL: No suitable DAO parent\n");
    }
    ctimer_stop(&instance->dao_timer);

#if !RPL_ROUTE_INFINITE_LIFETIME
    //if(etimer_expired(&instance->dao_lifetime_timer.etimer)) {
    if (ctimer_expired(&instance->dao_lifetime_timer))
    {
        set_dao_lifetime_timer(instance);
    }
#endif
}
/*---------------------------------------------------------------------------*/
static void schedule_dao(rpl_instance_t *instance, clock_time_t latency)
{
    clock_time_t expiration_time;

    //if(!etimer_expired(&instance->dao_timer.etimer)) {
    if (!ctimer_expired(&instance->dao_timer))
    {
        PRINTF("RPL: DAO timer already scheduled\n");
    }
    else
    {
        if (latency != 0)
        {
            expiration_time = latency / 2 + (random_rand() % (latency));
        }
        else
        {
            expiration_time = 0;
        } PRINTF("RPL: Scheduling DAO timer %u ticks in the future\n",
                (unsigned)expiration_time);
        ctimer_set(&instance->dao_timer, expiration_time, handle_dao_timer,
                   instance);

#if !RPL_ROUTE_INFINITE_LIFETIME
        set_dao_lifetime_timer(instance);
#endif

        UIP_STAT_RPL_DAO_SCHEDULED_NUM_INC;
    }
}
/*---------------------------------------------------------------------------*/
void rpl_schedule_dao(rpl_instance_t *instance)
{
    schedule_dao(instance, instance->dao_latency); //RPL_DAO_LATENCY);
}
/*---------------------------------------------------------------------------*/
void rpl_schedule_dao_immediately(rpl_instance_t *instance)
{
    schedule_dao(instance, 0);
}
/*---------------------------------------------------------------------------*/
void rpl_cancel_dao(rpl_instance_t *instance)
{
    ctimer_stop(&instance->dao_timer);
#if !RPL_ROUTE_INFINITE_LIFETIME
    ctimer_stop(&instance->dao_lifetime_timer);
#endif
#if RPL_CONF_DAO_ACK
    ctimer_stop(&instance->dao_ack_timer);
#endif
}
/*---------------------------------------------------------------------------*/
void rpl_periodic_timer_init()
{
    ctimer_timer_init(&periodic_timer);
}
/*---------------------------------------------------------------------------*/
#if RPL_CONF_DAO_ACK

#define RPL_DAO_ACK_EXPIRATION(_tries,_latency) ((((_latency) / 2) + (random_rand() % (_latency))) * (1 << (_tries-1)))

void handle_dao_ack_timer(void *ptr)
{
    rpl_instance_t *instance;
    instance = (rpl_instance_t *) ptr;

    instance->dao_ack_tries++; //DAO ACK timer expired so try again if it corresponds
    if (instance->dao_ack_tries <= RPL_DAO_ACK_MAX_TRIES)
    {
        instance->dao_ack_is_retry = 1;
        dao_output_target(instance->last_dao_dest, &instance->last_dao_target, instance->last_dao_lifetime);
        instance->dao_ack_is_retry = 0;
    }
    else
    {
        instance->dao_ack_tries = 0;
    }
}

void rpl_stop_dao_ack_timer(rpl_instance_t *instance)
{
    if(!ctimer_expired(&instance->dao_ack_timer))
    {
        //If not expired, stop it
        ctimer_stop(&instance->dao_ack_timer);
    }
}

void rpl_set_dao_ack_timer(rpl_instance_t *instance, rpl_parent_t *parent, uip_ipaddr_t *prefix, uint16_t lifetime)
{
    clock_time_t expiration_time;

    if (!instance->dao_ack_is_retry)
    {
        instance->dao_ack_tries = 1; //If we just need to schedule a completely new DAO
        instance->last_dao_dest = parent;
        uip_ipaddr_copy(&instance->last_dao_target, prefix);
        instance->last_dao_lifetime = lifetime;
    }
    expiration_time = RPL_DAO_ACK_EXPIRATION(instance->dao_ack_tries, RPL_DAO_ACK_INITIAL_LATENCY);
    //If there is a timer running, we stop it and start again
    rpl_stop_dao_ack_timer(instance);
    ctimer_set(&instance->dao_ack_timer, expiration_time,
            handle_dao_ack_timer, instance);
}
#endif //RPL_CONF_DAO_ACK

#endif /* UIP_CONF_IPV6 */
