/*
 * Copyright (c) 2015-2022, Texas Instruments Incorporated
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
 *  ======== rfATCommands.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/SPI.h>
#include <ti/drivers/GPIO.h>

/* Board Header files */
#include "ti_drivers_config.h"

/* AT Header files */
#include "at/platform/inc/AtTerm.h"
#include "at/AtProcess.h"
#include "at/AtParams.h"

#include "radio/radio.h"

/***** Defines *****/
#define atTaskStackSize       2048
#define atTaskPriority        2

/***** Variable declarations *****/
static Task_Params atTaskParams;
Task_Struct atTask;
static uint8_t atTaskStack[atTaskStackSize];

static void atFxn(UArg arg0, UArg arg1)
{

    AtTerm_init();

    AtTerm_clearTerm();
    AtTerm_sendString("RF Diagnostics Example \r");

    Radio_Init();

    GPIO_setConfig(CONFIG_GPIO_GLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_RLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
    GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);

    while (1)
    {
        AtProcess_processingLoop();
    }
}

/*
 *  ======== main ========
 */
void * mainThread(void * arg0)
{
    (void)arg0;

    Task_Params_init(&atTaskParams);
    atTaskParams.stackSize = atTaskStackSize;
    atTaskParams.priority = atTaskPriority;
    atTaskParams.stack = &atTaskStack;
    atTaskParams.arg0 = (UInt)1000000;

    Task_construct(&atTask, atFxn, &atTaskParams, NULL);

    while (1){};
}
