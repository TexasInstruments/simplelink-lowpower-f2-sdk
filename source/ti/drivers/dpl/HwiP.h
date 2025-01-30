/*
 * Copyright (c) 2015-2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** ============================================================================
 *  @file       HwiP.h
 *
 *  @brief      Hardware Interrupt module for the RTOS Porting Interface
 *
 *  The #HwiP_disable()/#HwiP_restore() APIs can be called recursively. The
 *  order of the #HwiP_restore() calls, must be in reversed order. For example:
 *  @code
 *  uintptr_t key1, key2;
 *  key1 = HwiP_disable();
 *  key2 = HwiP_disable();
 *  HwiP_restore(key2);
 *  HwiP_restore(key1);
 *  @endcode
 *
 *  @anchor ti_drivers_HwiP_Cross_Platform_Considerations
 *  ## Cross-platform Considerations ##
 *  The following sections highlight some important items to consider when
 *  using the HwiP DPL. Due to the nature and intention of different operating
 *  systems, the HwiP DPL implementation may differ depending on the underlying
 *  platform and operating system.
 *
 *  ### Arm Cortex-M Interrupt Priorities ###
 *  On Arm Cortex-M platforms, the highest logical interrupt priority has the
 *  lowest numerical value. That is, 0 is the highest interrupt priority
 *  (interrupt level 0 has the highest urgency). The number of interrupt
 *  priorities is given by the number of interrupt priority bits implemented,
 *  which is vendor-specific.
 *
 *  For example:
 *
 *  | Device Family | Architecture | Priority Bits | Priority Levels |
 *  |---------------|--------------|---------------|-----------------|
 *  | CC23X0        | Armv6-m      | 2             | 4 (0-3)         |
 *  | CC13X2_CC26X2 | Armv7-m      | 3             | 8 (0-7)         |
 *  | CC13X4_CC26X4 | Armv8-m      | 3             | 8 (0-7)         |
 *  | CC27XX        | Armv8-m      | 4             | 16 (0-15)       |
 *
 *  On Arm Cortex-M platforms, interrupt priority values are stored in the most
 *  significant bits of the 8-bit interrupt priority registers. For example,
 *  with 3 interrupt priority bits implemented, interrupt priority level 2
 *  (0b010) is represented in the interrupt priority registers as
 *  0b010 << (8-3) = 0x40.
 *  This is automatically handled by SysConfig, but may cause some confusion,
 *  for example, when inspecting the raw value of the #HwiP_Params.priority
 *  field or the Cortex-M interrupt priority registers.
 *
 *  See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html for more details.
 *
 *  Arm Cortex-M devices may support 2 methods for masking interrupts
 *  - \c PRIMASK disables all interrupt levels indiscriminately (with the
 *    exception of non-maskable interrupts).
 *  - \c BASEPRI disables interrupts with priority equal to or lower than a
 *    certain level. Software can set \c BASEPRI to a priority level between 1
 *    (second highest priority) and the maximum supported priority level
 *    (lowest priority). This means that interrupt priority level 0 cannot be
 *    masked using \c BASEPRI.
 *
 *  The table gives an overview of what interrupt masking controls the different
 *  Arm platforms support:
 *
 *  | Architecture | PRIMASK | BASEPRI |
 *  |--------------|:-------:|:-------:|
 *  | Armv6-m      |  Yes    |  No     |
 *  | Armv7-m      |  Yes    |  Yes    |
 *  | Armv8-m      |  Yes    |  Yes    |
 *
 *  ### FreeRTOS ###
 *  On Cortex-M platforms that support \c BASEPRI, the FreeRTOS kernel does not
 *  completely disable interrupts even inside critical sections. This is
 *  achieved by these FreeRTOS configurations:
 *
 *  - \c configMAX_SYSCALL_INTERRUPT_PRIORITY defines the highest logical
      priority at which FreeRTOS kernel API calls can be made.
 *  - \c configKERNEL_INTERRUPT_PRIORITY sets the priority of the FreeRTOS
      kernel itself, usually the lowest logical priority possible.
 *
 *  This permits a fully nested model where higher-priority interrupts can
 *  preempt lower-priority ones. However, interrupts that require FreeRTOS API
 *  calls must adhere to the \c configMAX_SYSCALL_INTERRUPT_PRIORITY limit to
 *  avoid corrupting kernel data.
 *
 *  See https://www.freertos.org/a00110.html#kernel_priority for more details.
 *
 *  - #HwiP_disable(): sets \c BASEPRI to \c
 *    configMAX_SYSCALL_INTERRUPT_PRIORITY. Returns original \c BASEPRI.
 *  - #HwiP_restore() sets \c BASEPRI to 0 (no interrupts are masked).
 *
 *  ### TI-RTOS ###
 *  TI-RTOS behaves similar to FreeRTOS, meaning that on Cortex-M platforms that
 *  support \c BASEPRI, the TI-RTOS kernel does not completely disable
 *  interrupts even inside critical sections.
 *
 *  - #HwiP_disable() sets \c BASEPRI to \c Hwi_disablePriority. Returns
 *    original \c BASEPRI.
 *  - #HwiP_restore() sets \c BASEPRI to the provided argument value.
 *
 *  See TI-RTOS Hwi documentation for more details.
 *
 *  ### NoRTOS ###
 *  - #HwiP_disable() disables all interrupt levels indiscriminately.
 *  - #HwiP_restore() globally enables all interrupts.
 *
 *  ============================================================================
 */

#ifndef ti_dpl_HwiP__include
#define ti_dpl_HwiP__include

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief    Number of bytes greater than or equal to the size of any RTOS
 *            HwiP object.
 *
 *  NoRTOS:   12
 *  FreeRTOS: 12
 *  BIOS 6.x: 28
 *  BIOS 7.x: 20
 */
#define HwiP_STRUCT_SIZE (28)

/*!
 *  @brief    HwiP structure.
 *
 *  Opaque structure that should be large enough to hold any of the RTOS
 *  specific HwiP objects.
 */
typedef union HwiP_Struct
{
    uint32_t dummy; /*!< Align object */
    uint8_t data[HwiP_STRUCT_SIZE];
} HwiP_Struct;

/*!
 *  @brief    Opaque client reference to an instance of a HwiP
 *
 *  A HwiP_Handle returned from the #HwiP_create() represents that instance.
 */
typedef void *HwiP_Handle;

/*!
 *  @brief    Status codes for HwiP APIs
 */
typedef enum
{
    HwiP_OK      = 0,
    HwiP_FAILURE = -1
} HwiP_Status;

/*!
 *  @brief  Prototype for the entry function for a hardware interrupt
 */
typedef void (*HwiP_Fxn)(uintptr_t arg);

/*!
 *  @brief    Basic HwiP Parameters
 *
 *  Structure that contains the parameters passed into #HwiP_create()
 *  when creating a HwiP instance. The #HwiP_Params_init() function should
 *  be used to initialize the fields to default values before the application sets
 *  the fields manually. The HwiP default parameters are noted in
 *  #HwiP_Params_init().
 *
 *  Parameter enableInt specifies if the interrupt should be enabled
 *  upon creation of the HwiP object.  The default is true.
 */
typedef struct
{
    uintptr_t arg;     /*!< Argument passed into the Hwi function. */
    uint32_t priority; /*!< Device specific priority encoded using the device
                        *   platform's internal priority representation.
                        *   See @ref ti_drivers_HwiP_Cross_Platform_Considerations
                        */
    bool enableInt;    /*!< Enable interrupt on creation. */
} HwiP_Params;

/*!
 *  @brief    Interrupt number posted by SwiP
 *
 *  The SwiP module needs its scheduler to run at key points in SwiP
 *  processing.  This is accomplished via an interrupt that is configured
 *  at the lowest possible interrupt priority level and is plugged with
 *  the SwiP scheduler.  This interrupt must be the *only* interrupt at
 *  that lowest priority.  SwiP will post this interrupt whenever its
 *  scheduler needs to run.
 *
 *  The default value for your device should suffice, but if a different
 *  interrupt is needed to be used for SwiP scheduling then HwiP_swiPIntNum
 *  can be assigned with this interrupt (early on, before HwiPs are created
 *  and before any SwiP gets posted).
 */
extern int HwiP_swiPIntNum;

/*!
 *  @brief  Function to construct a hardware interrupt object.
 *
 *  @param  hwiP   Pointer to HwiP_Struct object.
 *  @param  interruptNum Interrupt Vector Id
 *  @param  hwiFxn entry function of the hardware interrupt
 *
 *  @param  params    Pointer to the instance configuration parameters. NULL
 *                    denotes to use the default parameters. The HwiP default
 *                    parameters are noted in #HwiP_Params_init().
 *
 *  @return A HwiP_Handle on success or a NULL on an error
 */
extern HwiP_Handle HwiP_construct(HwiP_Struct *hwiP, int interruptNum, HwiP_Fxn hwiFxn, HwiP_Params *params);

/*!
 *  @brief  Function to destruct a hardware interrupt object
 *
 *  @param  hwiP  Pointer to a HwiP_Struct object that was passed to
 *                #HwiP_construct().
 *
 *  @return
 */
extern void HwiP_destruct(HwiP_Struct *hwiP);

/*!
 *  @brief  Function to clear a single interrupt
 *
 *  @param  interruptNum interrupt number to clear
 */
extern void HwiP_clearInterrupt(int interruptNum);

/*!
 *  @brief  Function to create an interrupt on CortexM devices
 *
 *  @note   This function may not be available on all implementations
 *
 *  @param  interruptNum Interrupt Vector Id
 *
 *  @param  hwiFxn entry function of the hardware interrupt
 *
 *  @param  params    Pointer to the instance configuration parameters. NULL
 *                    denotes to use the default parameters. The HwiP default
 *                    parameters are noted in #HwiP_Params_init().
 *
 *  @return A HwiP_Handle on success or a NULL on an error
 */
extern HwiP_Handle HwiP_create(int interruptNum, HwiP_Fxn hwiFxn, HwiP_Params *params);

/*!
 *  @brief  Function to delete an interrupt on CortexM devices
 *
 *  @note   This function may not be available on all implementations
 *
 *  @param  handle returned from the HwiP_create call
 *
 *  @return
 */
extern void HwiP_delete(HwiP_Handle handle);

/*!
 *  @brief  Function to disable interrupts to enter a critical region
 *
 *  This function can be called multiple times, but must unwound in the reverse
 *  order. For example
 *  @code
 *  uintptr_t key1, key2;
 *  key1 = HwiP_disable();
 *  key2 = HwiP_disable();
 *  HwiP_restore(key2);
 *  HwiP_restore(key1);
 *  @endcode
 *
 *  @return A key that must be passed to #HwiP_restore() to re-enable interrupts.
 */
extern uintptr_t HwiP_disable(void);

/*!
 *  @brief  Function to enable interrupts
 */
extern void HwiP_enable(void);

/*!
 *  @brief  Function to disable a single interrupt
 *
 *  @param  interruptNum interrupt number to disable
 */
extern void HwiP_disableInterrupt(int interruptNum);

/*!
 *  @brief  Function to enable a single interrupt
 *
 *  @param  interruptNum interrupt number to enable
 */
extern void HwiP_enableInterrupt(int interruptNum);

/*!
 *  @brief  Function  to return a status based on whether it is in an interrupt
 *      context.
 *
 *  @return A status: indicating whether the function was called in an
 *      ISR (true) or at thread level (false).
 */
extern bool HwiP_inISR(void);

/*!
 *  @brief  Function to determine whether interrupts are currently enabled.
 *
 *  @return Current state of interrupts.
 *    - true  Interrupts are currently enabled.
 *    - false Interrupts are currently disabled.
 */
extern bool HwiP_interruptsEnabled(void);

/*!
 *  @brief  Initialize params structure to default values.
 *
 *  The default parameters are:
 *   - arg: 0
 *   - priority: ~0
 *   - enableInt: true
 *
 *  @param params  Pointer to the instance configuration parameters.
 */
extern void HwiP_Params_init(HwiP_Params *params);

/*!
 *  @brief  Function to plug an interrupt vector
 *
 *  @param  interruptNum ID of interrupt to plug
 *  @param  fxn ISR that services plugged interrupt
 */
extern void HwiP_plug(int interruptNum, void *fxn);

/*!
 *  @brief  Function to generate an interrupt
 *
 *  @param  interruptNum ID of interrupt to generate
 */
extern void HwiP_post(int interruptNum);

/*!
 *  @brief  Function to restore interrupts to exit a critical region
 *
 *  @param  key return from HwiP_disable
 */
extern void HwiP_restore(uintptr_t key);

/*!
 *  @brief  Function to overwrite HwiP function and arg
 *
 *  @param  hwiP handle returned from the HwiP_create or construct call
 *  @param  fxn  pointer to ISR function
 *  @param  arg  argument to ISR function
 */
extern void HwiP_setFunc(HwiP_Handle hwiP, HwiP_Fxn fxn, uintptr_t arg);

/*!
 *  @brief  Function to set the priority of a hardware interrupt
 *
 *  @param  interruptNum id of the interrupt to change
 *  @param  priority new priority
 */
extern void HwiP_setPriority(int interruptNum, uint32_t priority);

/*!
 *  @brief  Function to call the HW ISR function registered by #HwiP_construct()
 *
 *  @note   This function may not be available on all implementations
 *
 *  @param  interruptNum Interrupt Vector Id
 */
void HwiP_dispatchInterrupt(int interruptNum);

#ifdef __cplusplus
}
#endif

#endif /* ti_dpl_HwiP__include */
