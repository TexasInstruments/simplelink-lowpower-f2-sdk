/*
 * Copyright (c) 2016-2018 ARM Limited
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#include "cmsis.h"
#include "system_cc26x4.h"

#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  extern uint32_t __Vectors;
#endif

#define  SYSTEM_CLOCK     (48000000)

/* required by TFM NS app */
uint32_t SystemCoreClock = SYSTEM_CLOCK;

/* System initialization function */
void SystemInit (void)
{
#if __DOMAIN_NS != 1U
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
    SCB->VTOR = (uint32_t) &__Vectors;
#endif
#ifdef UNALIGNED_SUPPORT_DISABLE
    SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif
#endif /* __DOMAIN_NS != 1U */

    SystemCoreClock = SYSTEM_CLOCK;
}
