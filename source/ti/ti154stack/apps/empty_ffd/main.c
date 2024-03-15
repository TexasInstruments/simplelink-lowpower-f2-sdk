/*
 * Copyright (c) 2016-2019, Texas Instruments Incorporated
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
 *  ======== main.c ========
 */
#include <stdint.h>

/* RTOS header files */
#ifndef FREERTOS_SUPPORT
#include <ti/sysbios/BIOS.h>
#else
#include <FreeRTOS.h>
#include <task.h>
#endif

#include <pthread.h>

/* Example/Board Header files */
#include <ti/drivers/Board.h>

/* MAC header files */
#include "rom_jt_154.h"
#include "macTask.h"
#include "mac_user_config.h"

/* Primary IEEE Address CCFG offset */
#define EXTADDR_OFFSET 0x2F0

/* MAC user defined configuration */
macUserCfg_t macUser0Cfg[] = MAC_USER_CFG;

/* MAC IEEE Address */
extern ApiMac_sAddrExt_t ApiMac_extAddr;

extern void *mainThread(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE    1024

pthread_t appThread;
static uint8_t appTaskStack[THREADSTACKSIZE];

#ifdef FREERTOS_SUPPORT
extern void startRxCbThread(void);
extern void startTxCbThread(void);
#endif

/*
 *  Assert used by MAC
 */
void assertHandler(void)
{
    while(1);
}

/*!
 * @brief       TIRTOS HWI Handler.  The name of this function is set to
 *              M3Hwi.excHandlerFunc in app.cfg, you can disable this by
 *              setting it to null.
 *
 * @param       excStack - uint32_t variable
 * @param       lr - uint32_t variable
 */
void Main_excHandler(uint32_t *excStack, uint32_t lr)
{
    /* User defined function */
    assertHandler();
}

/*
 *  ======== main ========
 */
int main(void)
{
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    /* Call driver init functions */
    Board_init();

    /* provide mac assert handler */
    macUser0Cfg[0].pAssertFP = assertHandler;

    /* Get the Primary IEEE Address */
    memcpy(ApiMac_extAddr, (uint8_t *)(FCFG1_BASE + EXTADDR_OFFSET),
           (APIMAC_SADDR_EXT_LEN));

    uint8_t _macTaskId = macTaskInit(macUser0Cfg);

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    attrs.stack = appTaskStack;
    attrs.stacksize = THREADSTACKSIZE;
    if (retc != 0) {
        /* failed to set attributes */
        while (1) {}
    }

    retc = pthread_create(&appThread, &attrs, mainThread, &_macTaskId);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1) {}
    }

#ifdef FREERTOS_SUPPORT
    startRxCbThread();
    startTxCbThread();
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
#else
    BIOS_start(); /* enable interrupts and start SYS/BIOS */
#endif

    return (0);
}
