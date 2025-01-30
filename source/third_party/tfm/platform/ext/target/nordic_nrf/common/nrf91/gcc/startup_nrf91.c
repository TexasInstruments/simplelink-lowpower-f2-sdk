/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This file is derivative of CMSIS V5.9.0 startup_ARMCM33.c
 * Git SHA: 2b7495b8535bdcb306dac29b9ded4cfb679d7e5c
 */

#include "cmsis.h"
#include "startup.h"
#include "exception_info.h"

__NO_RETURN __attribute__((naked)) void default_tfm_IRQHandler(void) {
	EXCEPTION_INFO(EXCEPTION_TYPE_PLATFORM);

	__ASM volatile(
        "BL        default_irq_handler     \n"
        "B         .                       \n"
    );
}

DEFAULT_IRQ_HANDLER(NMI_Handler)
DEFAULT_IRQ_HANDLER(SVC_Handler)
DEFAULT_IRQ_HANDLER(DebugMon_Handler)
DEFAULT_IRQ_HANDLER(PendSV_Handler)
DEFAULT_IRQ_HANDLER(SysTick_Handler)

DEFAULT_IRQ_HANDLER(CLOCK_POWER_IRQHandler)
DEFAULT_IRQ_HANDLER(UARTE0_SPIM0_SPIS0_TWIM0_TWIS0_IRQHandler)
DEFAULT_IRQ_HANDLER(UARTE1_SPIM1_SPIS1_TWIM1_TWIS1_IRQHandler)
DEFAULT_IRQ_HANDLER(UARTE2_SPIM2_SPIS2_TWIM2_TWIS2_IRQHandler)
DEFAULT_IRQ_HANDLER(UARTE3_SPIM3_SPIS3_TWIM3_TWIS3_IRQHandler)
DEFAULT_IRQ_HANDLER(GPIOTE0_IRQHandler)
DEFAULT_IRQ_HANDLER(SAADC_IRQHandler)
DEFAULT_IRQ_HANDLER(TIMER0_IRQHandler)
DEFAULT_IRQ_HANDLER(TIMER1_IRQHandler)
DEFAULT_IRQ_HANDLER(TIMER2_IRQHandler)
DEFAULT_IRQ_HANDLER(RTC0_IRQHandler)
DEFAULT_IRQ_HANDLER(RTC1_IRQHandler)
DEFAULT_IRQ_HANDLER(WDT_IRQHandler)
DEFAULT_IRQ_HANDLER(EGU0_IRQHandler)
DEFAULT_IRQ_HANDLER(EGU1_IRQHandler)
DEFAULT_IRQ_HANDLER(EGU2_IRQHandler)
DEFAULT_IRQ_HANDLER(EGU3_IRQHandler)
DEFAULT_IRQ_HANDLER(EGU4_IRQHandler)
DEFAULT_IRQ_HANDLER(EGU5_IRQHandler)
DEFAULT_IRQ_HANDLER(PWM0_IRQHandler)
DEFAULT_IRQ_HANDLER(PWM1_IRQHandler)
DEFAULT_IRQ_HANDLER(PWM2_IRQHandler)
DEFAULT_IRQ_HANDLER(PWM3_IRQHandler)
DEFAULT_IRQ_HANDLER(PDM_IRQHandler)
DEFAULT_IRQ_HANDLER(I2S_IRQHandler)
DEFAULT_IRQ_HANDLER(IPC_IRQHandler)
DEFAULT_IRQ_HANDLER(FPU_IRQHandler)
DEFAULT_IRQ_HANDLER(GPIOTE1_IRQHandler)
DEFAULT_IRQ_HANDLER(KMU_IRQHandler)
DEFAULT_IRQ_HANDLER(CRYPTOCELL_IRQHandler)

#if defined(DOMAIN_NS) || defined(BL2)
DEFAULT_IRQ_HANDLER(SPU_IRQHandler)
DEFAULT_IRQ_HANDLER(HardFault_Handler)
DEFAULT_IRQ_HANDLER(MemManage_Handler)
DEFAULT_IRQ_HANDLER(BusFault_Handler)
DEFAULT_IRQ_HANDLER(UsageFault_Handler)
DEFAULT_IRQ_HANDLER(SecureFault_Handler)
#else
/*
 * Default IRQ handlers will usually be overriden as they are
 * weak. But due to the way TF-M links it's binary (doesn't use
 * whole-archive), weak doesn't always work. So we explicitly ifdef
 * out some IRQ handlers that we know will be overridden anyway to be
 * safe.
 */
#endif

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

const VECTOR_TABLE_Type __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE = {
  (VECTOR_TABLE_Type)(&__INITIAL_SP), /* Initial Stack Pointer */
  Reset_Handler,               /* Reset Handler */
  NMI_Handler,                 /* NMI Handler */
  HardFault_Handler,           /* Hard Fault Handler */
  MemManage_Handler,           /* MPU Fault Handler */
  BusFault_Handler,            /* Bus Fault Handler */
  UsageFault_Handler,          /* Usage Fault Handler */
  SecureFault_Handler,         /* Secure Fault Handler */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  SVC_Handler,                 /* SVCall Handler */
  DebugMon_Handler,            /* Debug Monitor Handler */
  0,                           /* Reserved */
  PendSV_Handler,              /* PendSV Handler */
  SysTick_Handler,             /* SysTick Handler */

  /* External Interrupts */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  SPU_IRQHandler,
  0,                           /* Reserved */
  CLOCK_POWER_IRQHandler,
  0,                           /* Reserved */
  0,                           /* Reserved */
  UARTE0_SPIM0_SPIS0_TWIM0_TWIS0_IRQHandler,
  UARTE1_SPIM1_SPIS1_TWIM1_TWIS1_IRQHandler,
  UARTE2_SPIM2_SPIS2_TWIM2_TWIS2_IRQHandler,
  UARTE3_SPIM3_SPIS3_TWIM3_TWIS3_IRQHandler,
  0,                           /* Reserved */
  GPIOTE0_IRQHandler,
  SAADC_IRQHandler,
  TIMER0_IRQHandler,
  TIMER1_IRQHandler,
  TIMER2_IRQHandler,
  0,                           /* Reserved */
  0,                           /* Reserved */
  RTC0_IRQHandler,
  RTC1_IRQHandler,
  0,                           /* Reserved */
  0,                           /* Reserved */
  WDT_IRQHandler,
  0,                           /* Reserved */
  0,                           /* Reserved */
  EGU0_IRQHandler,
  EGU1_IRQHandler,
  EGU2_IRQHandler,
  EGU3_IRQHandler,
  EGU4_IRQHandler,
  EGU5_IRQHandler,
  PWM0_IRQHandler,
  PWM1_IRQHandler,
  PWM2_IRQHandler,
  PWM3_IRQHandler,
  0,                           /* Reserved */
  PDM_IRQHandler,
  0,                           /* Reserved */
  I2S_IRQHandler,
  0,                           /* Reserved */
  IPC_IRQHandler,
  0,                           /* Reserved */
  FPU_IRQHandler,
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  GPIOTE1_IRQHandler,
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  KMU_IRQHandler,
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  0,                           /* Reserved */
  CRYPTOCELL_IRQHandler,
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif
