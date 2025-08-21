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
 *  ======== main_tirtos.c ========
 */
#include <stdint.h>

/* RTOS header files */
#ifndef FREERTOS_SUPPORT
#include <ti/sysbios/BIOS.h>
#else
#include <FreeRTOS.h>
#include <task.h>
#endif

/* POSIX Header files */
#include <pthread.h>

/* Driver configuration */
#include "ti_drivers_config.h"
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/AESECB.h>
#include <ti/drivers/AESCBC.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_cpu_scs.h)

/******************************************************************************
 Extern functions
 *****************************************************************************/
extern void ws_br_handler_init();
extern void ncp_tasklet_start();

#ifdef FREERTOS_SUPPORT
extern void startRfCbThread(void);
#endif

/******************************************************************************
 Global Variables
 *****************************************************************************/
#ifdef FEATURE_TIMAC_SUPPORT

#include "macTask.h"
#include "rom_jt_154.h"
/* Assert Reasons */
#define MAIN_ASSERT_MAC          3
#define MAIN_ASSERT_HWI_TIRTOS   4

#ifndef USE_DEFAULT_USER_CFG
#include "mac_user_config.h"
/* MAC user defined configuration */
macUserCfg_t macUser0Cfg[] = MAC_USER_CFG;
#endif /* USE_DEFAULT_USER_CFG */

uint8_t timacTaskId;
#endif /* FEATURE_TIMAC_SUPPORT */

bool ncp_enabled = true;

#ifndef WISUN_NCP_ENABLE
extern void *mainThread(void *arg0);

/* Stack size in bytes */
#define WISUNTHREADSTACKSIZE    1024

#endif

/* 
    To prevent the LTO to throw away these variables, adding the used attribute for the variables
*/
__attribute__((used)) uint32_t exc_lr;
__attribute__((used)) uint32_t exc_sp;

//__attribute__((naked)) void MacAssert_get_SP_LR(void)
 void MacAssert_get_SP_LR(void)
{
    __asm volatile
    (
            // Get current state, add more as needed

            "ldr r8, =exc_sp                    \n"
            "str sp,[r8]                        \n"
            "ldr r8, =exc_lr                    \n"
            "str lr,[r8]                        \n"
    );
}

#ifdef FEATURE_TIMAC_SUPPORT
void Main_assertHandler(uint8_t assertReason)
{
#if defined(RESET_ASSERT)
    //Ssf_assertInd(assertReason);

    /* Pull the plug and start over */
    SysCtrlSystemReset();
#else
    HwiP_disable();

    while(1)
    {
        /* Put you code here to do something if in assert */
    }
#endif

}

/*!
 * @brief       TIRTOS HWI Handler.  The name of this function is set to
 *              M3Hwi.excHandlerFunc in app.cfg, you can disable this by
 *              setting it to null.
 *
 * @param       excStack - TIROS variable
 * @param       lr - TIROS variable
 */
void Main_excHandler(uint32_t *excStack, uint32_t lr)
{
    /* User defined function */
    Main_assertHandler(MAIN_ASSERT_HWI_TIRTOS);
}


/*!
 * @brief       HAL assert handler required by OSAL memory module.
 */
void assertHandler(void)
{
    // save LP and SP for later debugging
    MacAssert_get_SP_LR();    
    /* User defined function */
    Main_assertHandler(MAIN_ASSERT_MAC);
}

/*!
 * @brief       HAL assert handler required by OSAL memory module.
 */
void halAssertHandler(void)
{
    // save LP and SP for later debugging
    MacAssert_get_SP_LR();        
    /* User defined function */
    Main_assertHandler(0);
}

/*!
 * @brief       MAC HAL assert handler.
 */
void macHalAssertHandler(void)
{
    // save LP and SP for later debugging
    MacAssert_get_SP_LR();        
    /* User defined function */
    Main_assertHandler(MAIN_ASSERT_MAC);
}
#endif /* FEATURE_TIMAC_SUPPORT */

/*
 *  ======== main ========
 */

int main(void)
{
#if !defined(WISUN_NCP_ENABLE) || !defined(EXCLUDE_TRACE) || defined(COAP_SERVICE_ENABLE)
    /* Disable power idle mode to prevent known issue with ITM
       output garbage. */
    Power_setConstraint(PowerCC26XX_IDLE_PD_DISALLOW);
    Power_setConstraint(PowerCC26XX_SB_DISALLOW);
#endif

    // Disable write-buffering. Note that this negatively affect performance.
    // HWREG(CPU_SCS_BASE + CPU_SCS_O_ACTLR) = CPU_SCS_ACTLR_DISDEFWBUF;

    /* Call driver init functions */
    Board_init();
    GPIO_init();
    Button_init();
    LED_init();

#if defined(MBEDTLS_AES_ALT)
    // AESECB_init();
    AESCBC_init();
#endif

#ifdef FEATURE_TIMAC_SUPPORT
#ifndef USE_DEFAULT_USER_CFG
    macUser0Cfg[0].pAssertFP = assertHandler;
#endif //USE_DEFAULT_USER_CFG
    timacTaskId = macTaskInit(macUser0Cfg);
#endif //FEATURE_TIMAC_SUPPORT

    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);
    ncp_enabled = false;

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, WISUNTHREADSTACKSIZE);
    if (retc != 0) {
        /* failed to set attributes */
        while (1) {}
    }

    retc = pthread_create(&thread, &attrs, mainThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1) {}
    }

    // kick-start scheduler
#ifdef FREERTOS_SUPPORT
    /* Start the FreeRTOS scheduler */
    startRfCbThread();
    vTaskStartScheduler();
#else
    BIOS_start(); /* enable interrupts and start SYS/BIOS */
#endif
    return (0);
}

