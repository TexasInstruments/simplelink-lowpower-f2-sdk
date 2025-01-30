/*
 * Copyright (c) 2016-2018 ARM Limited
 * Copyright (c) 2023 Nuvoton Technology Corp.
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file platform_retarget.h
 * \brief This file defines all the peripheral base addresses for MPS2/AN519 platform.
 */

#ifndef __M2354_RETARGET_H__
#define __M2354_RETARGET_H__

#include "NuMicro.h"

/* ======= Defines peripherals memory map addresses ======= */
/* Non-secure memory map addresses */
#define CMSDK_TIMER0_BASE_NS   (TMR01_BASE + NS_OFFSET)
#define CMSDK_TIMER1_BASE_NS   (TMR01_BASE + 100UL + NS_OFFSET)
#define UART0_BASE_NS          (UART0_BASE + NS_OFFSET)

/* Secure memory map addresses */
#define CMSDK_TIMER0_BASE_S    (TMR01_BASE)
#define CMSDK_TIMER1_BASE_S    (TMR01_BASE + 100UL)
#define UART0_BASE_S           UART0_BASE

#endif  /* __M2354_RETARGET_H__ */
