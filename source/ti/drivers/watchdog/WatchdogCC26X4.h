/*
 * Copyright (c) 2022, Texas Instruments Incorporated
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
/*!****************************************************************************
 *  @file       WatchdogCC26X4.h
 *
 *  @brief      Watchdog driver implementation for CC13X4/CC26X4
 *
 *  # Driver include #
 *  The Watchdog header file should be included in an application as follows:
 *  @code
 *  #include <ti/drivers/Watchdog.h>
 *  #include <ti/drivers/watchdog/WatchdogCC26X4.h>
 *  @endcode
 *
 *  Refer to @ref Watchdog.h for a complete description of APIs.
 *
 *  # Overview #
 *
 *  The general Watchdog API should be used in application code, i.e.
 *  #Watchdog_open() should be used instead of WatchdogCC26X4_open(). The board
 *  file will define the device specific config, and casting in the general API
 *  will ensure that the correct device specific functions are called.
 *
 *  # General Behavior #
 *  This Watchdog driver implementation is designed to operate on a CC13X4/CC26X4
 *  device. Before using the Watchdog on CC13X4/CC26X4, the Watchdog driver is
 *  initialized by calling #Watchdog_init(). The Watchdog HW is configured by
 *  calling #Watchdog_open(). Once opened, the Watchdog will count down from
 *  the reload value specified in #WatchdogCC26X4_HWAttrs. If it times out, a reset
 *  signal will be generated. To prevent a reset, #Watchdog_clear() must be called
 *  to reload the timer.
 *
 *  The Watchdog counts down at the rate of the device clock SCLK_LF. SCLK_LF will
 *  tick at different rates depending on the SCLK_LF source selected in CCFG,
 *  ranging between 31250 Hz and 32768 Hz. The Watchdog driver internally calculates
 *  the correct tick value depending on the target SCLK_LF source selected in CCFG.
 *  RCOSC_LF is an inherently inaccurate clock source and will present variations
 *  around the target 32768 Hz frequency. These inaccuracies have to be taken into
 *  consideration at the application level if RSCOC_LF is selected as the source
 *  of SCLK_LF.
 *
 *  The reload value from which the Watchdog timer counts down may be changed
 *  during runtime using #Watchdog_setReload(). This value should be specified
 *  in Watchdog clock ticks and should not exceed "2^32 - 1". This corresponds to
 *  a timeout period of 131071 seconds, calculated at the highest rate of 32768
 *  kHz. If the reload value is set to zero, the Watchdog reset is immediately
 *  generated.
 *
 *  Watchdog_close() is <b>not</b> supported by this driver implementation. Once
 *  started, the Watchdog timer can only be stopped by a hardware reset.
 *
 *  <b>No</b> CC13X4/CC26X4 specific command has been implemented. Any call to
 *  Watchdog_control() will receive the return code Watchdog_STATUS_UNDEFINEDCMD.
 *
 *  The Watchdog module available on CC13X4/CC26X4 devices does not support reset
 *  masking or interrupt generation. Therefore, the two parameters \ref
 *  Watchdog_Params.resetMode and \ref Watchdog_Params.callbackFxn in the \ref
 *  Watchdog_Params struct are not supported and will be ignored by the Watchdog
 *  driver.
 *
 *  # Power Management #
 *  Once started, the Watchdog will keep running in Active, Idle and Standby mode.
 *
 * # Supported Functions #
 *  | Generic API Function          | API Function                      | Description |
 *  |------------------------------ |----------------------------------
 * |--------------------------------------------------- | | #Watchdog_init()              | WatchdogCC26X4_init() |
 * Initialize Watchdog driver                         | | #Watchdog_open()              | WatchdogCC26X4_open() |
 * Initialize Watchdog HW and set system dependencies | | #Watchdog_clear()             | WatchdogCC26X4_clear() |
 * Reload Watchdog counter                            | | #Watchdog_setReload()         | WatchdogCC26X4_setReload() |
 * Update Watchdog timer reload value in clock ticks  | | #Watchdog_convertMsToTicks()  |
 * WatchdogCC26X4_convertMsToTicks() | Converts milliseconds to clock ticks               |
 *
 *  @note All calls should go through the generic API. Please refer to @ref Watchdog.h for a
 *  complete description of the generic APIs.
 *
 * # Use Cases #
 * ## Basic Watchdog #
 *  In this basic watchdog example, the application is expected to start the Watchdog
 *  timer by calling #Watchdog_open(). If needed, #Watchdog_setReload() may be
 *  called to change the timeout period. If all monitored tasks are doing alright,
 *  #Watchdog_clear() should be called regularly to reload the counter so as to
 *  restart the timeout period and to avoid the Watchdog resetting the device.
 *  If the #Watchdog_clear() is missed and the Watchdog timer is allowed to
 *  timeout, the device will be reset.
 *
 *  The following code example shows how to correctly initialize the driver's
 *  parameters, start the Watchdog timer and modify at runtime the timeout period.
 *  @code
 *
 *  Watchdog_Handle handle;
 *  Watchdog_Params params;
 *  uint32_t tickValue;
 *
 *  Watchdog_init();
 *  Watchdog_Params_init(&params);
 *  handle = Watchdog_open(Watchdog_configIndex, &params);
 *  // set timeout period to 100 ms
 *  tickValue = Watchdog_convertMsToTicks(handle, 100);
 *  Watchdog_setReload(handle, tickValue);
 *
 *  @endcode
 */

#ifndef ti_drivers_watchdog_WatchdogCC26X4__include
#define ti_drivers_watchdog_WatchdogCC26X4__include

#include <stdint.h>
#include <stdbool.h>
#include <ti/drivers/Watchdog.h>
#include <ti/drivers/dpl/HwiP.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @addtogroup Watchdog_STATUS
 *  WatchdogCC26X4_STATUS_* macros are command codes only defined in the
 *  WatchdogCC26X4.h driver implementation and need to:
 *  @code
 *  #include <ti/drivers/watchdog/WatchdogCC26X4.h>
 *  @endcode
 *  @{
 */

/* Add WatchdogCC26X4_STATUS_* macros here */

/** @}*/

/**
 *  @addtogroup Watchdog_CMD
 *  WatchdogCC26X4_CMD_* macros are command codes only defined in the
 *  WatchdogCC26X4.h driver implementation and need to:
 *  @code
 *  #include <ti/drivers/watchdog/WatchdogCC26X4.h>
 *  @endcode
 *  @{
 */

/* Add WatchdogCC26X4_CMD_* macros here */

/** @}*/

/*! @brief  Watchdog function table for CC26X4 */
extern const Watchdog_FxnTable WatchdogCC26X4_fxnTable;

/*!
 *  @brief  Watchdog hardware attributes for CC26X4
 */
typedef struct
{
    unsigned long reloadValue; /*!< Reload value in milliseconds for Watchdog */
} WatchdogCC26X4_HWAttrs;

/*!
 *  @brief      Watchdog Object for CC26X4
 *
 *  Not to be accessed by the user.
 */
typedef struct
{
    bool isOpen;                       /* Flag for open/close status */
    Watchdog_DebugMode debugStallMode; /* Mode to stall Watchdog at breakpoints */
    /* Watchdog SYS/BIOS objects */
    HwiP_Struct hwi; /* Hwi object */
} WatchdogCC26X4_Object;

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_watchdog_WatchdogCC26X4__include */
