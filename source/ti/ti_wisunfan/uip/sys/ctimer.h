/**
 * \addtogroup sys
 * @{
 */

/**
 * \defgroup ctimer Callback timer
 * @{
 *
 * The ctimer module provides a timer mechanism that calls a specified
 * C function when a ctimer expires.
 *
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
 * $Id: ctimer.h,v 1.2 2010/06/14 07:35:53 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the callback timer
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __CTIMER_H__
#define __CTIMER_H__

#include "uip-conf.h"

#include "sys/etimer.h"

#define CALLBACK_TIMER 0xcc

struct ctimer
{
#if !WITH_PLTFRM_TIMERS
    struct ctimer *next;
    struct etimer etimer;
#else
    pltfrm_timer_t t;
    clock_time_t interval;
#endif  
    void (*f)(void *);
    void *ptr;
};

/**
 * \brief      Reset a callback timer with the same interval as was
 *             previously set.
 * \param c    A pointer to the callback timer.
 *
 *             This function resets the callback timer with the same
 *             interval that was given to the callback timer with the
 *             ctimer_set() function. The start point of the interval
 *             is the exact time that the callback timer last
 *             expired. Therefore, this function will cause the timer
 *             to be stable over time, unlike the ctimer_restart()
 *             function.
 *
 * \sa ctimer_restart()
 */
void ctimer_reset(struct ctimer *c);

/**
 * \brief      Restart a callback timer from the current point in time
 * \param c    A pointer to the callback timer.
 *
 *             This function restarts the callback timer with the same
 *             interval that was given to the ctimer_set()
 *             function. The callback timer will start at the current
 *             time.
 *
 *             \note A periodic timer will drift if this function is
 *             used to reset it. For periodic timers, use the
 *             ctimer_reset() function instead.
 *
 * \sa ctimer_reset()
 */
void ctimer_restart(struct ctimer *c);

/**
 * \brief      Set a callback timer.
 * \param c    A pointer to the callback timer.
 * \param t    The interval before the timer expires.
 * \param f    A function to be called when the timer expires.
 * \param ptr  An opaque pointer that will be supplied as an argument to the callback function.
 *
 *             This function is used to set a callback timer for a time
 *             sometime in the future. When the callback timer expires,
 *             the callback function f will be called with ptr as argument.
 *
 */
void ctimer_set(struct ctimer *c, clock_time_t t, void (*f)(void *), void *ptr);

/**
 * \brief      Stop a pending callback timer.
 * \param c    A pointer to the pending callback timer.
 *
 *             This function stops a callback timer that has previously
 *             been set with ctimer_set(), ctimer_reset(), or ctimer_restart().
 *             After this function has been called, the callback timer will be
 *             expired and will not call the callback function.
 *
 */
void ctimer_stop(struct ctimer *c);

/**
 * \brief      Check if a callback timer has expired.
 * \param c    A pointer to the callback timer
 * \return     Non-zero if the timer has expired, zero otherwise.
 *
 *             This function tests if a callback timer has expired and
 *             returns true or false depending on its status.
 */
int ctimer_expired(struct ctimer *c);

/**
 * \brief      Initialize the callback timer library.
 *
 *             This function initializes the callback timer library and
 *             should be called from the system boot up code.
 */
//The process post function may be used to change contexts for the execution of the callback function, once the timer expired. However, if this post function calls directly the ctimer_process_handle function, then there will be no context switching
void ctimer_init(process_post_func_t func);

//Initialize an instance of a ctimer
void ctimer_timer_init(struct ctimer *c);

//This function needs to be called from the ctimer process context (if context switching is needed) or directly from the ctimer process_post_func_t for no context switching.
void ctimer_process_handler(process_event_t ev, process_data_t data);

#endif /* __CTIMER_H__ */
/** @} */
/** @} */
