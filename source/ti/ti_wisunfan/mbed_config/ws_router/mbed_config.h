/*
 * mbed SDK
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MBED_CONFIG_DATA__
#define __MBED_CONFIG_DATA__


#define MBED_CONF_NANOSTACK_CONFIGURATION                                     ti_ws_router
#define MBED_CONF_MBED_MESH_API_HEAP_SIZE                                     42000
#define MBED_CONF_MBED_MESH_API_HEAP_STAT_INFO                                NULL
#define MBED_CONF_MBED_MESH_API_USE_MALLOC_FOR_HEAP                           0

// #define MBED_CONF_NANOSTACK_HAL_CRITICAL_SECTION_USABLE_FROM_INTERRUPT        0
#define MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_DISPATCH_FROM_APPLICATION          0
// #define MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_THREAD_STACK_SIZE                  6144

#define MBED_CONF_MBED_MESH_API_WISUN_DEVICE_TYPE                             MESH_DEVICE_TYPE_WISUN_ROUTER

#define MBED_CONF_MBED_MESH_API_MAC_NEIGH_TABLE_SIZE                          32

#define MBED_CONF_NSAPI_DEFAULT_STACK                                         LWIP
#define MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE                       ETHERNET

#define MEM_ALLOC                                                             malloc
#define MEM_FREE                                                              free
// #define MBED_CONF_MBED_MESH_APP_WISUN_NETWORK_SIZE                            1
// Macros
#define MBEDTLS_CIPHER_MODE_CTR                                                                                                                                                // defined by library:SecureStore
#define MBEDTLS_CMAC_C                                                                                                                                                         // defined by library:SecureStore
#define MBEDTLS_PSA_HAS_ITS_IO                                                                                                                                                 // defined by library:mbed-crypto
#define MBEDTLS_USER_CONFIG_FILE                                              "mbedtls_wisun_config.h"                                                                         // defined by application
#define DEVICE_TRNG                                                           1
#define NSAPI_PPP_AVAILABLE                                                   (MBED_CONF_PPP_ENABLED || MBED_CONF_LWIP_PPP_ENABLED)                                            // defined by library:ppp
//#define NS_USE_EXTERNAL_MBED_TLS                                                                                                                                               // defined by library:nanostack
#define UNITY_INCLUDE_CONFIG_H                                                                                                                                                 // defined by library:utest
#define _RTE_                                                                                                                                                                  // defined by library:rtos

#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define EQUEUE_PLATFORM_POSIX

#endif //__MBED_CONFIG_DATA__
