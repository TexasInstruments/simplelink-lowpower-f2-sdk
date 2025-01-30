/*
 * Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
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

/**
 * \file platform_retarget.h
 * \brief This file defines all the peripheral base addresses for CC27XX platform.
 */

#ifndef __CC27XX_RETARGET_H__
#define __CC27XX_RETARGET_H__

#include "cmsis.h"
#include "flash_layout.h"
/* TI CC27xx SDK include(s) */
#include "ti/devices/cc27xx/inc/hw_memmap.h"

/* ======= Defines peripherals memory map addresses ======= */
/* Non-secure memory map addresses */
#define UART0_BASE_NS          UART0_BASE
#define UART0_BASE_S           UART0_BASE

#endif  /* __CC27XX_RETARGET_H__ */
