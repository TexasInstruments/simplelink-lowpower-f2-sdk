/*
 * Copyright (c) 2016-2018, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed_config_app.h"
#include <pthread.h>
#include <semaphore.h>

#include "eventOS_scheduler.h"

#include "ns_event_loop_mutex.h"
#include "ns_event_loop.h"

#include "ns_trace.h"

pthread_t event_thread_id;
sem_t event_thread_sem_handle;

static void *event_loop_thread(void *a0);

extern void eventOS_dispatch_timac_event(void);

void eventOS_scheduler_signal(void)
{
    //tr_debug("signal %p", (void*)event_thread_id);
    sem_post(&event_thread_sem_handle);
    //tr_debug("signalled %p", (void*)event_thread_id);
}

void eventOS_scheduler_idle(void)
{
    //tr_debug("idle");
    eventOS_scheduler_mutex_release();

    sem_wait(&event_thread_sem_handle);

    eventOS_scheduler_mutex_wait();
    /* This function is moved to macTask.c
     *
     */
    //eventOS_dispatch_timac_event();
}

static void *event_loop_thread(void *a0)
{
    eventOS_scheduler_mutex_wait();
    eventOS_scheduler_run(); //Does not return
}

// This is used to initialize the lock used by event loop even
// if it is not ran in a separate thread.
void ns_event_loop_init(void)
{
    ns_event_loop_mutex_init();
}

void ns_event_loop_thread_create(void)
{
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    retc = sem_init(&event_thread_sem_handle, 0, 0);
    if (retc != 0) {
        while (1);
    }

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = 2;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);

    /*
     * POSIX pthread_create will create the stack if attrs.stack= NULL
     */
    retc |= pthread_attr_setstacksize(&attrs, MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_THREAD_STACK_SIZE);

    if (retc != 0) {
        /* failed to set attributes */
        while (1) {}
    }

    retc = pthread_create(&event_thread_id, &attrs, event_loop_thread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1) {}
    }


}

void ns_event_loop_thread_start(void)
{
}
