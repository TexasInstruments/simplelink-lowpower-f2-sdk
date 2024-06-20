/**
 * \addtogroup ctimer
 * @{
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
 * $Id: ctimer.c,v 1.1 2010//14 07:34:36 adamdunkels Exp $
 */

/**
 * \file
 *         Callback timer implementation
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "uip-conf.h"

#include "sys/ctimer.h"
#include "lib/list.h"

#if !WITH_PLTFRM_TIMERS

LIST(ctimer_list);

static char initialized;
#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

process_obj_t ctimer_process_obj;

void ctimer_process_handler(process_event_t ev, process_data_t data)
{
    struct ctimer *c;
    PRINTF("CTIMER PROCESS");
    for(c = list_head(ctimer_list); c != NULL; c = c->next)
    {
        if(&c->etimer == data)
        {
            PRINTF("IS ETIMER!");
            list_remove(ctimer_list, c);
            if(c->f != NULL)
            {
                PRINTF("RUNNING FUNCTION");
                c->f(c->ptr);
            }
            break;
        }
    }
}
/*---------------------------------------------------------------------------*/

void ctimer_process_init(process_post_func_t func)
{
    struct ctimer *c;

    process_obj_init(&ctimer_process_obj, func);

    for(c = list_head(ctimer_list); c != NULL; c = c->next)
    {
        PRINTF("CTIMER SETTING ETIMER");
        etimer_timer_init(&c->etimer);
        etimer_set(&c->etimer, &ctimer_process_obj, c->etimer.timer.interval);
    }
    initialized = 1;
}

void
ctimer_init(process_post_func_t func)
{
    PRINTF("CTIMER INIT");
    initialized = 0;
    list_init(ctimer_list);
    ctimer_process_init(func);
}

void ctimer_timer_init(struct ctimer *c)
{
    c->f = NULL;
    c->next = NULL;
    c->ptr = NULL;
    etimer_timer_init(&c->etimer);
}

/*---------------------------------------------------------------------------*/
void
ctimer_set(struct ctimer *c, clock_time_t t,
        void (*f)(void *), void *ptr)
{
    c->f = f;
    c->ptr = ptr;
    if(initialized)
    {
        etimer_set(&c->etimer, &ctimer_process_obj, t);
        PRINTF("CTIMER NOT INITIALIZED SET");
        PRINTF_DEC(t);
    }
    else
    {
        c->etimer.timer.interval = t;
        PRINTF("CTIMER INITIALIZED SET");
        PRINTF_DEC(t);
    }

    list_remove(ctimer_list, c);
    list_add(ctimer_list, c);
}
/*---------------------------------------------------------------------------*/
void
ctimer_reset(struct ctimer *c)
{
    if(initialized)
    {
        etimer_reset(&c->etimer);
    }

    list_remove(ctimer_list, c);
    list_add(ctimer_list, c);
}
/*---------------------------------------------------------------------------*/
void
ctimer_restart(struct ctimer *c)
{
    if(initialized)
    {
        etimer_restart(&c->etimer);
    }

    list_remove(ctimer_list, c);
    list_add(ctimer_list, c);
}
/*---------------------------------------------------------------------------*/
void
ctimer_stop(struct ctimer *c)
{
    if(initialized)
    {
        etimer_stop(&c->etimer);
    }
    else
    {
        c->etimer.next = NULL;
        c->etimer.p = PROCESS_NONE;
    }
    list_remove(ctimer_list, c);
}
/*---------------------------------------------------------------------------*/
int
ctimer_expired(struct ctimer *c)
{
    struct ctimer *t;
    if(initialized)
    {
        return etimer_expired(&c->etimer);
    }
    for(t = list_head(ctimer_list); t != NULL; t = t->next)
    {
        if(t == c)
        {
            return 0;
        }
    }
    return 1;
}
/*---------------------------------------------------------------------------*/

#else //WITH_PLTFRM_TIEMRS
#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

process_obj_t ctimer_process_obj;

void ctimer_process_handler(process_event_t ev, process_data_t data)
{
    struct ctimer *c = (struct ctimer *) data;
    PRINTF("CTIMER PROCESS");
    if (c->f != NULL)
    {
        PRINTF("RUNNING FUNCTION");
        c->f(c->ptr);
    }
}
/*---------------------------------------------------------------------------*/

void ctimer_process_init(process_post_func_t func)
{
    process_obj_init(&ctimer_process_obj, func);
}

void ctimer_init(process_post_func_t func)
{
    PRINTF("CTIMER INIT");
    ctimer_process_init(func);
}

void ctimer_timer_init(struct ctimer *c)
{
    c->f = NULL;
    c->ptr = NULL;
    pltfrm_timer_init(&(c->t));
}

/*---------------------------------------------------------------------------*/
static void ctimer_expired_function(void* args)
{
    ctimer_process_obj.process_post(CALLBACK_TIMER, args);
}

void ctimer_set(struct ctimer *c, clock_time_t t, void (*f)(void *), void *ptr)
{
    c->interval = t;
    c->f = f;
    c->ptr = ptr;
    if (!pltfrm_timer_isCreated(&(c->t)))
    {
        pltfrm_timer_create(&(c->t), &ctimer_expired_function, (void*) c, t);
    }
    pltfrm_timer_restart(&(c->t), t);
}
void ctimer_reset(struct ctimer *c)
{
    pltfrm_timer_restart(&(c->t), c->interval);
}

int ctimer_expired(struct ctimer *c)
{
    if (pltfrm_timer_isCreated(&(c->t)))
        return !pltfrm_timer_isActive(&(c->t));
    return 1;
}

void ctimer_restart(struct ctimer *c)
{
    pltfrm_timer_restart(&(c->t), c->interval);
}

void ctimer_stop(struct ctimer *c)
{
    if (c == NULL)
        return;

    if ((c->t.handle) != NULL)
    {
        pltfrm_timer_stop(&(c->t));
        pltfrm_timer_delete(&(c->t));
    }
}
#endif

/** @} */
