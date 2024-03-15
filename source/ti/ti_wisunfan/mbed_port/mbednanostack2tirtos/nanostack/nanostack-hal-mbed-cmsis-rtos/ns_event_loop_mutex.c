/*
 * Copyright (c) 2018, Arm Limited and affiliates.
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

#include "ns_event_loop_mutex.h"
#include "stdint.h"

#include <pthread.h>
#include <semaphore.h>

pthread_t event_mutex_owner_id;
sem_t ns_event_mutex_handle;

static uint32_t owner_count = 0;

void eventOS_scheduler_mutex_wait(void)
{
    sem_wait(&ns_event_mutex_handle);

    if (0 == owner_count) {
        /* store mutex owner task handle */
        event_mutex_owner_id = pthread_self();
    }
    owner_count++;
}

void eventOS_scheduler_mutex_release(void)
{
    owner_count--;
    if (0 == owner_count) {
        event_mutex_owner_id = NULL;
    }

    sem_post(&ns_event_mutex_handle);
}

uint8_t eventOS_scheduler_mutex_is_owner(void)
{
    return pthread_self() == event_mutex_owner_id ? 1 : 0;
}

void ns_event_loop_mutex_init(void)
{
    int retc;

    retc = sem_init(&ns_event_mutex_handle, 0, 1);
    if (retc != 0) {
        while (1);
    }
}
