/*
 * Copyright (c) 2023 Arm Limited. All rights reserved.
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

#ifndef __HOST_CMSIS_DRIVER_CONFIG_H__
#define __HOST_CMSIS_DRIVER_CONFIG_H__

#include "cmsis.h"
#include "device_definition.h"
#include "RTE_Device.h"

#ifdef RSS_USE_HOST_UART
#define UART0_DEV                    UART0_PL011_DEV_NS
#endif /* RSS_USE_HOST_UART */

#define FLASH0_DEV                   SPI_STRATAFLASHJ3_DEV

#endif  /* __HOST_CMSIS_DRIVER_CONFIG_H__ */
