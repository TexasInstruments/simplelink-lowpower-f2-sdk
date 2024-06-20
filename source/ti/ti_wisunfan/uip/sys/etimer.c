/**
 * \addtogroup etimer
 * @{
 */

/**
 * \file
 * Event timer library implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 */

/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: etimer.c,v 1.3 2007/10/07 19:59:27 joxe Exp $
 */

#include "uip-conf.h"

#if (!WITH_PLTFRM_TIMERS)

#include "sys/etimer.h"
#include "lib/list.h"
#ifdef DEBUG  
#undef DEBUG
#endif

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

LIST(etimer_list);

static int etimer_isInitialized = 0;

void etimer_init()
{
    if (etimer_isInitialized != 0)
    {
        list_init(etimer_list);
        etimer_isInitialized = 1;
    }
}

void etimer_timer_init(struct etimer *et)
{
    memset(et, 0, sizeof(*et));
}

/*---------------------------------------------------------------------------*/
void etimer_poll()
{
    struct etimer *t;
    PRINTF("ETIMER POLL");
    for(t = list_head(etimer_list); t != NULL; t = t->next)
    {
        if(timer_expired(&t->timer))
        {
            list_remove(etimer_list, t);
            if (t->p != PROCESS_NONE)
            {
                PRINTF("ETIMER RUN");
                if (t->p->process_post != NULL)
                {
                    t->p->process_post(EVENT_TIMER, t);
                }
            }
            t = list_head(etimer_list); //start again
        }
    }
}

/*---------------------------------------------------------------------------*/
static void
add_timer(struct etimer *timer)
{
    list_remove(etimer_list, timer);
    list_add(etimer_list, timer);

}
/*---------------------------------------------------------------------------*/
void
etimer_set(struct etimer *et, process_obj_t *p, clock_time_t interval)
{
    PRINTF("ETIMER SET");
    etimer_stop(et);
    timer_set(&et->timer, interval);
    if (p != NULL)
    {
        et->p = p;
    }
    else
    {
        et->p = PROCESS_NONE;
    }
    add_timer(et);
}
/*---------------------------------------------------------------------------*/
void
etimer_reset(struct etimer *et)
{
    timer_reset(&et->timer);
    add_timer(et);
}
/*---------------------------------------------------------------------------*/
void
etimer_restart(struct etimer *et)
{
    timer_restart(&et->timer);
    add_timer(et);
}
/*---------------------------------------------------------------------------*/
int
etimer_expired(struct etimer *et)
{
    return timer_expired(&et->timer);
}
/*---------------------------------------------------------------------------*/
clock_time_t
etimer_expiration_time(struct etimer *et)
{
    return et->timer.start + et->timer.interval;
}

/*---------------------------------------------------------------------------*/
void
etimer_stop(struct etimer *et)
{
    list_remove(etimer_list, et);
    /* Set the timer as expired */
    et->p = PROCESS_NONE;
}
/*---------------------------------------------------------------------------*/

#else //WITH_PLTFRM_TIMERS

#include "sys/etimer.h"

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

void etimer_init()
{
}

void etimer_timer_init(struct etimer *et)
{
    pltfrm_timer_init(&(et->t));
    et->p = PROCESS_NONE;
    et->interval = 0;
}

static void etimer_expired_function(void* args)
{
    struct etimer *et = (struct etimer*) args;
    et->p->process_post(EVENT_TIMER, et);
}

void etimer_set(struct etimer *et, process_obj_t *p, clock_time_t interval)
{
    et->interval = interval;
    et->p = p;
    if (!pltfrm_timer_isCreated(&(et->t)))
    {
        pltfrm_timer_create(&(et->t), &etimer_expired_function, (void*) et,
                            interval);
    }
    pltfrm_timer_restart(&(et->t), interval);
}

void etimer_reset(struct etimer *et)
{
    pltfrm_timer_restart(&(et->t), et->interval);
}

int etimer_expired(struct etimer *et)
{
    if (pltfrm_timer_isCreated(&(et->t)))
        return !pltfrm_timer_isActive(&(et->t));
    return 1;
}

void etimer_restart(struct etimer *et)
{
    pltfrm_timer_restart(&(et->t), et->interval);
}

void etimer_stop(struct etimer *et)
{
    if (et == NULL || et->t.handle == NULL)
        return;
    pltfrm_timer_stop(&(et->t));
    pltfrm_timer_delete(&(et->t));
}

#endif //WITH_PLTFRM_TIMERS

/** @} */
