/*
 * Common and shared functions used by multiple modules in the Mbed TLS
 * library.
 *
 *  Copyright (C) 2018, Arm Limited, All Rights Reserved
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
 *  This file is part of Mbed TLS (https://tls.mbed.org)
 */

/*
 * Ensure gmtime_r is available even with -std=c99; must be defined before
 * config.h, which pulls in glibc's features.h. Harmless on other platforms.
 */
#if !defined(MBEDTLS_PLATFORM_ZEROIZE_ALT)
#include "string.h"

static void * (* const volatile memset_func)( void *, int, size_t ) = memset;

void mbedtls_platform_zeroize( void *buf, size_t len )
{
    //BIOS_ASSERT( len == 0 || buf != NULL );

    if( len > 0 )
        memset( buf, 0, len );
}
#endif /* MBEDTLS_PLATFORM_ZEROIZE_ALT */
