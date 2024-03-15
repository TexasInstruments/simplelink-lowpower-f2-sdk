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

#define MBED_CONF_MBED_MESH_APP_WISUN_NETWORK_SIZE                            1
#define MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_THREAD_STACK_SIZE                  3800
#define MBED_CONF_MBED_MESH_API_CERTIFICATE_HEADER                            "wisun_certificates.h"
#define MBED_CONF_MBED_MESH_API_OWN_CERTIFICATE                               WISUN_CLIENT_CERTIFICATE
#define MBED_CONF_MBED_MESH_API_OWN_CERTIFICATE_KEY                           WISUN_CLIENT_KEY
#define MBED_CONF_MBED_MESH_API_ROOT_CERTIFICATE                              WISUN_ROOT_CERTIFICATE

#endif //__MBED_CONFIG_DATA__
