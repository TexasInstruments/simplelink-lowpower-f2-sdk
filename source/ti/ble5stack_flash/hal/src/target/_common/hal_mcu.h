/******************************************************************************

 @file  hal_mcu.h

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef HAL_MCU_H
#define HAL_MCU_H



/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdint.h>
#include "hal_defs.h"
#include "hal_types.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#ifndef CC33xx
#include DeviceFamily_constructPath(inc/hw_nvic.h)
#include DeviceFamily_constructPath(inc/hw_gpio.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(driverlib/gpio.h)
#include DeviceFamily_constructPath(driverlib/systick.h)
#include DeviceFamily_constructPath(driverlib/uart.h)
#ifndef CC23X0
#include DeviceFamily_constructPath(driverlib/flash.h)
#include DeviceFamily_constructPath(driverlib/ioc.h)
#endif //CC23X0
#else
#include "osi.h"
#endif //CC33xx


/* ------------------------------------------------------------------------------------------------
 *                                     Compiler Abstraction
 * ------------------------------------------------------------------------------------------------
 */

/* ---------------------- IAR Compiler ---------------------- */
#if defined (__IAR_SYSTEMS_ICC__)
#define HAL_COMPILER_IAR
#define HAL_MCU_LITTLE_ENDIAN()   __LITTLE_ENDIAN__

/* ---------------------- Keil Compiler ---------------------- */
#elif defined (__KEIL__)
#define HAL_COMPILER_KEIL
#define HAL_MCU_LITTLE_ENDIAN()   0


/* ------------------ Unrecognized Compiler ------------------ */
#elif defined (ccs) || defined __TI_COMPILER_VERSION__ || defined (__GNUC__)
#define HAL_MCU_LITTLE_ENDIAN()   1
//do nothing for now
#else
#error "ERROR: Unknown compiler."
#endif


/* ------------------------------------------------------------------------------------------------
 *                                       Interrupt Macros
 * ------------------------------------------------------------------------------------------------
 */

#ifdef USE_ICALL
#include <icall.h>

typedef ICall_CSState halIntState_t;

#ifdef CC33xx
/* Enter critical section */
#define HAL_ENTER_CRITICAL_SECTION(x) x=osi_DisablePreemption();

/* Exit critical section */
#define HAL_EXIT_CRITICAL_SECTION(x) osi_RestorePreemption(x);
#else
/* Enable interrupts */
#define HAL_ENABLE_INTERRUPTS()     ICall_enableMInt()

/* Disable interrupts */
#define HAL_DISABLE_INTERRUPTS()    ICall_disableMInt()

/* Enter critical section */
#define HAL_ENTER_CRITICAL_SECTION(x) st(x = ICall_enterCriticalSection();)

/* Exit critical section */
#define HAL_EXIT_CRITICAL_SECTION(x) ICall_leaveCriticalSection(x)
#endif //CC33xx

/* Enable RF interrupt */
#define HAL_ENABLE_RF_INTERRUPT()    \
{                                    \
  ICall_enableInt(INT_RFCORERTX);    \
}

/* Enable RF error interrupt */
#define HAL_ENABLE_RF_ERROR_INTERRUPT() \
{                                       \
  ICall_enableInt(INT_RFCOREERR);       \
}

/* Note that check of whether interrupts are enabled or not is not supported
 * by any random operating system.
 * Hence, the call to HAL_INTERRUPTS_ARE_ENABLED() itself must not be made
 * from the beginning.
 */
#define HAL_INTERRUPTS_ARE_ENABLED() FALSE

#elif defined OSAL_PORT2TIRTOS

#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Task.h>

typedef int halIntState_t;

/* Enable interrupts */
#define HAL_ENABLE_INTERRUPTS()                 \
  do { Hwi_enable(); Task_enable(); } while (0)

/* Disable interrupts */
#define HAL_DISABLE_INTERRUPTS()                \
  do { Task_disable(); Hwi_disable(); } while (0)

/* Enter critical section */
#define HAL_ENTER_CRITICAL_SECTION(x)                   \
  do { extern void zipEnterCriticalSection(void);       \
    (void) x; zipEnterCriticalSection(); } while (0)

/* Exit critical section */
#define HAL_EXIT_CRITICAL_SECTION(x)                    \
  do { extern void zipExitCriticalSection(void);        \
    (void) x; zipExitCriticalSection(); } while (0)

/* Enable RF interrupt */
#define HAL_ENABLE_RF_INTERRUPT() Hwi_enableInterrupt(INT_RFCORERTX)

/* Enable RF error interrupt */
#define HAL_ENABLE_RF_ERROR_INTERRUPT() Hwi_enableInterrupt(INT_RFCOREERR)

/* Note that check of whether interrupts are enabled or not is not supported
 * by any random operating system.
 * Hence, the call to HAL_INTERRUPTS_ARE_ENABLED() itself must not be made
 * from the beginning.
 */
#define HAL_INTERRUPTS_ARE_ENABLED() FALSE

#else /* OSAL_PORT2TIRTOS */

typedef bool halIntState_t;

/* Enable RF interrupt */
#define HAL_ENABLE_RF_INTERRUPT()    \
{                                    \
  IntEnable(INT_RFCORERTX);          \
}

/* Enable RF error interrupt */
#define HAL_ENABLE_RF_ERROR_INTERRUPT() \
{                                       \
  IntEnable(INT_RFCOREERR);             \
}

#ifndef USE_RCL
/* Enable interrupts */
#define HAL_ENABLE_INTERRUPTS()     IntMasterEnable()

/* Disable interrupts */
#define HAL_DISABLE_INTERRUPTS()    IntMasterDisable()

static bool halIntsAreEnabled(void)
{
  bool status = !IntMasterDisable();
  if (status)
  {
    IntMasterEnable();
  }
  return status;
}

#define HAL_INTERRUPTS_ARE_ENABLED() halIntsAreEnabled()
#endif

#ifdef CC23X0
#define HAL_ENTER_CRITICAL_SECTION(x)  \
  do { (x) = !IntDisableMaster(); } while (0)

#define HAL_EXIT_CRITICAL_SECTION(x) \
  do { if (x) { (void) IntEnableMaster(); } } while (0)
#else
#define HAL_ENTER_CRITICAL_SECTION(x)  \
  do { (x) = !IntMasterEnable(); } while (0)

#define HAL_EXIT_CRITICAL_SECTION(x) \
  do { if (x) { (void) IntMasterEnable(); } } while (0)
#endif

#endif /* USE_ICALL */

#define HAL_NON_ISR_ENTER_CRITICAL_SECTION(x)  HAL_ENTER_CRITICAL_SECTION(x)
#define HAL_NON_ISR_EXIT_CRITICAL_SECTION(x)   HAL_EXIT_CRITICAL_SECTION(x)

/* Hal Critical statement definition */
#define HAL_CRITICAL_STATEMENT(x)       st( halIntState_t s; HAL_ENTER_CRITICAL_SECTION(s); x; HAL_EXIT_CRITICAL_SECTION(s); )

/* Enable Key/button interrupts */
#define HAL_ENABLE_PUSH_BUTTON_PORT_INTERRUPTS()  \
{                                                 \
  ICall_enableInt(INT_GPIOC);                           \
  ICall_enableInt(INT_GPIOA);                           \
}

/* Disable Key/button interrupts */
#define HAL_DISABLE_PUSH_BUTTON_PORT_INTERRUPTS()  \
{                                                  \
  ICall_enableInt(INT_GPIOC);                           \
  ICall_enableInt(INT_GPIOA);                           \
}

/* ------------------------------------------------------------------------------------------------
 *                                        Sleep common code stubs
 * ------------------------------------------------------------------------------------------------
 */
#define CLEAR_SLEEP_MODE()
#define ALLOW_SLEEP_MODE()


/* ------------------------------------------------------------------------------------------------
 *                                        Dummy for this platform
 * ------------------------------------------------------------------------------------------------
 */
#define HAL_AES_ENTER_WORKAROUND()
#define HAL_AES_EXIT_WORKAROUND()
/**************************************************************************************************
 */
#endif
