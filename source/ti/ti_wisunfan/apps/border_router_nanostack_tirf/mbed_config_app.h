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

#ifndef __MBED_CONFIG_APP_DATA__
#define __MBED_CONFIG_APP_DATA__

/* Configuration parameters */

/* configuration set through application */
#define MBED_CONF_MBED_MESH_API_WISUN_NETWORK_SIZE                            1
#define MBED_CONF_APP_DEBUG_TRACE                                             true

#define MBED_CONF_APP_MULTICAST_ADDR                                          ff05::7

#define MBED_CONF_APP_CERTIFICATE_HEADER                                      "wisun_certificates.h"
#define MBED_CONF_APP_OWN_CERTIFICATE                                         WISUN_SERVER_CERTIFICATE
#define MBED_CONF_APP_OWN_CERTIFICATE_KEY                                     WISUN_SERVER_KEY
#define MBED_CONF_APP_ROOT_CERTIFICATE                                        WISUN_ROOT_CERTIFICATE

#define MBED_CONF_APP_BACKHAUL_DEFAULT_ROUTE                                  "::/0"
#define MBED_CONF_APP_BACKHAUL_DRIVER                                         EMAC
#define MBED_CONF_APP_BACKHAUL_NEXT_HOP                                       "fe80::1"
#define MBED_CONF_APP_BACKHAUL_PREFIX                                         "fd00:db8:ff1::"
#define MBED_CONF_APP_BACKHAUL_DYNAMIC_BOOTSTRAP                              1
#define MBED_CONF_MBED_MESH_API_WISUN_DEVICE_TYPE                             MESH_DEVICE_TYPE_WISUN_BORDER_ROUTER
#define MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_THREAD_STACK_SIZE                  4144

#endif //__MBED_CONFIG_DATA__
