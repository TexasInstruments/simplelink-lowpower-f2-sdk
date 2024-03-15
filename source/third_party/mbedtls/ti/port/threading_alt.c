/*
 *  Threading abstraction layer
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#include <mbedtls/threading.h>
#include "threading_alt.h"

void threading_mutex_init_pthread(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL || mutex->is_valid)
        return;

    mutex->is_valid = pthread_mutex_init(&mutex->mutex, NULL) == 0;
}

void threading_mutex_free_pthread(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL || !mutex->is_valid)
        return;

    (void)pthread_mutex_destroy(&mutex->mutex);
    mutex->is_valid = 0;
}

int threading_mutex_lock_pthread(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL || !mutex->is_valid)
        return (MBEDTLS_ERR_THREADING_BAD_INPUT_DATA);

    if (pthread_mutex_lock(&mutex->mutex) != 0)
        return (MBEDTLS_ERR_THREADING_MUTEX_ERROR);

    return (0);
}

int threading_mutex_unlock_pthread(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL || !mutex->is_valid)
        return (MBEDTLS_ERR_THREADING_BAD_INPUT_DATA);

    if (pthread_mutex_unlock(&mutex->mutex) != 0)
        return (MBEDTLS_ERR_THREADING_MUTEX_ERROR);

    return (0);
}
