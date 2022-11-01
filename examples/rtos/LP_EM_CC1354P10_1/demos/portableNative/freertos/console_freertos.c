/*
 * Copyright (c) 2016-2022, Texas Instruments Incorporated
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
 *  ======== console.c ========
 */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART2.h>
#ifdef CC32XX
    #include <ti/drivers/Power.h>
    #include <ti/drivers/power/PowerCC32XX.h>
#endif

/* Driver configuration */
#include "ti_drivers_config.h"

/* Console display strings */
const char consoleDisplay[]   = "\fConsole (h for help)\r\n";
const char helpPrompt[]       = "Valid Commands\r\n"
                                "--------------\r\n"
                                "h: help\r\n"
                                "q: quit and shutdown UART\r\n"
                                "c: clear the screen\r\n"
                                "t: display current temperature\r\n";
const char byeDisplay[]       = "Bye! Hit button1 to start UART again\r\n";
const char tempStartDisplay[] = "Current temp = ";
const char tempMidDisplay[]   = "C (";
const char tempEndDisplay[]   = "F)\r\n";
const char cleanDisplay[]     = "\f";
const char userPrompt[]       = "> ";
const char readErrDisplay[]   = "Problem read UART.\r\n";

/* Used to determine whether to have the thread block */
volatile bool uart2Enabled = true;
SemaphoreHandle_t xConsoleSemaphore;

/* Temperature written by the temperature thread and read by console thread */
extern volatile float temperatureC;
extern volatile float temperatureF;

/* Used itoa instead of sprintf to help minimize the size of the stack */
static void itoa(int n, char s[]);

/*
 *  ======== gpioButtonFxn ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_1.
 *  There is no debounce logic here since we are just looking for
 *  a button push. The uart2Enabled variable protects use against any
 *  additional interrupts cased by the bouncing of the button.
 */
void gpioButtonFxn(uint_least8_t index)
{
    BaseType_t xHigherPriorityTaskWoken;

    /* If disabled, enable and post the semaphore */
    if (uart2Enabled == false)
    {
        uart2Enabled = true;
        xSemaphoreGiveFromISR(xConsoleSemaphore, &xHigherPriorityTaskWoken);
    }
}

/*
 *  ======== simpleConsole ========
 *  Handle the user input. Currently this console does not handle
 *  user back-spaces or other "hard" characters.
 */
void simpleConsole(UART2_Handle uart2Handle)
{
    char cmd;
    int status;
    char tempStr[8];
    int localTemperatureC;
    int localTemperatureF;

    UART2_write(uart2Handle, consoleDisplay, sizeof(consoleDisplay) - 1, NULL);

    /* Loop until read fails or user quits */
    while (1)
    {
        UART2_write(uart2Handle, userPrompt, sizeof(userPrompt) - 1, NULL);
        status = UART2_read(uart2Handle, &cmd, sizeof(cmd), NULL);
        if (status != UART2_STATUS_SUCCESS)
        {
            UART2_write(uart2Handle, readErrDisplay, sizeof(readErrDisplay) - 1, NULL);
            cmd = 'q';
        }

        switch (cmd)
        {
            case 't':
                UART2_write(uart2Handle, tempStartDisplay, sizeof(tempStartDisplay) - 1, NULL);
                /*
                 *  Make sure we are accessing the global float temperature variables
                 *  in a thread-safe manner.
                 */
                taskENTER_CRITICAL();
                localTemperatureC = (int)temperatureC;
                localTemperatureF = (int)temperatureF;
                taskEXIT_CRITICAL();

                itoa((int)localTemperatureC, tempStr);
                UART2_write(uart2Handle, tempStr, strlen(tempStr), NULL);
                UART2_write(uart2Handle, tempMidDisplay, sizeof(tempMidDisplay) - 1, NULL);
                itoa((int)localTemperatureF, tempStr);
                UART2_write(uart2Handle, tempStr, strlen(tempStr), NULL);
                UART2_write(uart2Handle, tempEndDisplay, sizeof(tempEndDisplay) - 1, NULL);
                break;
            case 'c':
                UART2_write(uart2Handle, cleanDisplay, sizeof(cleanDisplay) - 1, NULL);
                break;
            case 'q':
                UART2_write(uart2Handle, byeDisplay, sizeof(byeDisplay) - 1, NULL);
                return;
            case 'h':
            default:
                UART2_write(uart2Handle, helpPrompt, sizeof(helpPrompt) - 1, NULL);
                break;
        }
    }
}

/*
 *  ======== consoleThread ========
 */
void consoleThread(void *arg0)
{
    UART2_Params uart2Params;
    UART2_Handle uart2Handle;
    int retc;

#ifdef CC32XX
    /*
     *  The CC32XX examples by default do not have power management enabled.
     *  This allows a better debug experience. With the power management
     *  enabled, if the device goes into a low power mode the emulation
     *  session is lost.
     *  Let's enable it and also configure the button to wake us up.
     */
    PowerCC32XX_Wakeup wakeup;

    PowerCC32XX_getWakeup(&wakeup);
    wakeup.wakeupGPIOFxnLPDS = gpioButtonFxn;
    PowerCC32XX_configureWakeup(&wakeup);
    Power_enablePolicy();

#else

    /* Configure the button pin */
    GPIO_setConfig(CONFIG_GPIO_BUTTON_1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* install Button callback and enable it */
    GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn);
    GPIO_enableInt(CONFIG_GPIO_BUTTON_1);

#endif

    xConsoleSemaphore = xSemaphoreCreateBinary();
    if (xConsoleSemaphore == NULL)
    {
        while (1) {}
    }

    /*
     *  Initialize the UART2 parameters outside the loop. Let's keep
     *  most of the defaults (e.g. baudrate = 115200) and only change the
     *  following.
     */
    UART2_Params_init(&uart2Params);
    uart2Params.readReturnMode = UART2_ReadReturnMode_FULL;

    /* Loop forever to start the console */
    while (1)
    {
        if (uart2Enabled == false)
        {
            retc = xSemaphoreTake(xConsoleSemaphore, portMAX_DELAY);
            if (retc == pdFALSE)
            {
                while (1) {}
            }
        }

        /* Create a UART for the console */
        uart2Handle = UART2_open(CONFIG_UART2_0, &uart2Params);
        if (uart2Handle == NULL)
        {
            while (1) {}
        }

        simpleConsole(uart2Handle);

        /*
         * Since we returned from the console, we need to close the UART.
         * The Power Manager will go into a lower power mode when the UART
         * is closed.
         */

        UART2_close(uart2Handle);
        uart2Enabled = false;
    }
}

/*
 * The following function is from good old K & R.
 */
static void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
    {
        c    = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/*
 * The following function is from good old K & R.
 */
static void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0) /* record sign */
        n = -n;         /* make n positive */
    i = 0;
    do
    {                          /* generate digits in reverse order */
        s[i++] = n % 10 + '0'; /* get next digit */
    } while ((n /= 10) > 0);   /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}
