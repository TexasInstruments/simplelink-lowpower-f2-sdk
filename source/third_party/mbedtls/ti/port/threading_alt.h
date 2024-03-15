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

#ifndef THREADING_ALT_H
#define THREADING_ALT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct
{
    pthread_mutex_t mutex;
    char is_valid;
} mbedtls_threading_mutex_t;

/*
 *  @brief Thread mutex initialization callback
 *
 *  This is required for threading operation. Add this mutex init callback to
 *  mbedtls using 'mbedtls_threading_set_alt()' in the  application.
 */
void threading_mutex_init_pthread(mbedtls_threading_mutex_t *mutex);

/*
 *  @brief Thread mutex free callback
 *
 *  This is required for threading operation. Add this mutex free callback to
 *  mbedtls using 'mbedtls_threading_set_alt()' in the  application.
 */
void threading_mutex_free_pthread(mbedtls_threading_mutex_t *mutex);

/*
 *  @brief Thread mutex lock callback
 *
 *  This is required for threading operation. Add this mutex lock callback to
 *  mbedtls using 'mbedtls_threading_set_alt()' in the  application.
 */
int threading_mutex_lock_pthread(mbedtls_threading_mutex_t *mutex);

/*
 *  @brief Thread mutex unlock callback
 *
 *  This is required for threading operation. Add this mutex unlock callback to
 *  mbedtls using 'mbedtls_threading_set_alt()' in the  application.
 */
int threading_mutex_unlock_pthread(mbedtls_threading_mutex_t *mutex);

#ifdef __cplusplus
}
#endif
#endif
