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

/*
 *  ======== gpioshutdown.c ========
 */
#include <stdbool.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_prcm.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/pwr_ctrl.h)

#include "ti_drivers_config.h"

/* Semaphore used to gate for shutdown */
SemaphoreP_Struct semStruct;
SemaphoreP_Handle semHandle;

static void buttonCallback(Button_Handle buttonHandle, Button_EventMask buttonEvents)
{
    SemaphoreP_post(semHandle);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    LED_Handle led0Handle = LED_open(CONFIG_LED_0, NULL);
    LED_Handle led1Handle = LED_open(CONFIG_LED_1, NULL);

    PowerCC26X2_ResetReason resetReason = PowerCC26X2_getResetReason();

    /* If we are waking up from shutdown, we do something extra. */
    if (resetReason == PowerCC26X2_RESET_SHUTDOWN_IO)
    {
        /* Application code must always disable the IO latches when coming out of shutdown */
        PowerCC26X2_releaseLatches();

        /* In this example we toggle LED1 */
        LED_startBlinking(led1Handle, 500, 3);
    }

    semHandle = SemaphoreP_constructBinary(&semStruct, 0);

    Button_Params buttonParams;
    Button_Params_init(&buttonParams);
    buttonParams.buttonCallback  = buttonCallback;
    buttonParams.buttonEventMask = Button_EV_PRESSED;

    Button_open(CONFIG_BUTTON_SHUTDOWN, &buttonParams);

    /* Turn on LED0 to indicate active */
    LED_setOn(led0Handle, 100);

    /* Pend on semaphore before going to shutdown */
    SemaphoreP_pend(semHandle, SemaphoreP_WAIT_FOREVER);

    /* Turn off LED0 */
    LED_setOff(led0Handle);

    /* Configure DIO for wake up from shutdown */
    GPIO_setConfig(CONFIG_GPIO_WAKEUP, GPIO_CFG_IN_PU | GPIO_CFG_SHUTDOWN_WAKE_LOW);

    /* Go to shutdown */
    Power_shutdown(0, 0);

    /* Should never get here, since shutdown will reset. */
    while (1) {}
}
