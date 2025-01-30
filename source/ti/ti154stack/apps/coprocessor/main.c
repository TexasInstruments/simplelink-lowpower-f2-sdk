/******************************************************************************

 @file  main.c

 @brief Main entry of the MAC-CoProcessor application

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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

/******************************************************************************
 Includes
 *****************************************************************************/

/* RTOS header files */
#ifndef FREERTOS_SUPPORT
#include <ti/sysbios/BIOS.h>
#else
#include <FreeRTOS.h>
#include <task.h>
#endif

#include <pthread.h>
#include <ti/drivers/GPIO.h>

#ifdef OSAL_PORT2TIRTOS
#include "macTask.h"
#include"rom_jt_154.h"
#else
#include "api_mac.h"
#include "icall.h"
#endif

#include "ti_drivers_config.h"
#include "board_led.h"

#include <inc/hw_ccfg.h>
#include <inc/hw_ccfg_simple_struct.h>

/* Required for the idle task function */
#include <ti/drivers/power/PowerCC26XX.h>

/* Required to enable instruction fetch cache */
#include <vims.h>

#ifdef NV_RESTORE
#include "macconfig.h"
#include "nvocmp.h"
#endif

#include <string.h>

#include "api_mac.h"
#include "mt_sys.h"
#include "mcp.h"

#ifndef USE_DEFAULT_USER_CFG
#include "macs.h"
/* MAC user defined configuration */
macUserCfg_t macUser0Cfg[] = MAC_USER_CFG;
#endif /* USE_DEFAULT_USER_CFG */

/******************************************************************************
 Constants
 *****************************************************************************/
/*! Extended Address offset in FCFG (LSB..MSB) */
#define EXTADDR_OFFSET 0x2F0

/*! Memory location of cutomer-configured IEEE address */
#define CCFG_IEEE ((uint8_t *)&(__ccfg.CCFG_IEEE_MAC_0))

/*! Memory location of unique factory-programmed IEEE address */
#define PRIM_IEEE ((uint8_t *)(FCFG1_BASE + EXTADDR_OFFSET))

/*! Size of stack for MNP application */
#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4) || defined(DeviceFamily_CC26X3)
#define APP_TASK_STACK_SIZE  1400
#else
#define APP_TASK_STACK_SIZE  800
#endif

#define APP_TASK_PRIORITY   1

/******************************************************************************
 External Variables
 *****************************************************************************/
extern ApiMac_sAddrExt_t ApiMac_extAddr;

/******************************************************************************
 Global Variables
 *****************************************************************************/
pthread_t appThread;

#ifdef OSAL_PORT2TIRTOS
static uint8_t _macTaskId;
#endif

#ifdef NV_RESTORE
mac_Config_t Main_user1Cfg = {0};
#endif

/******************************************************************************
 Local Variables
 *****************************************************************************/
/*! Used to check for a valid extended address */
static const uint8_t dummyExtAddr[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/

extern void Board_init(void);

#ifdef FREERTOS_SUPPORT
extern void startRfCbThread(void);
#endif

/*!
 * @brief       Reads the IEEE extended MAC address from the CCFG
 * @param       addr - Extended address pointer
 */
static inline void CCFGRead_IEEE_MAC(ApiMac_sAddrExt_t addr)
{
    uint32_t macAddr = (( HWREG(
            CCFG_BASE + CCFG_O_IEEE_MAC_0 ) &
            CCFG_IEEE_MAC_0_ADDR_M ) >>
            CCFG_IEEE_MAC_0_ADDR_S );
    memcpy(addr, (uint8_t *)&macAddr, (APIMAC_SADDR_EXT_LEN / 2));

    macAddr = (( HWREG(
            CCFG_BASE + CCFG_O_IEEE_MAC_1 ) &
            CCFG_IEEE_MAC_1_ADDR_M ) >>
            CCFG_IEEE_MAC_1_ADDR_S );
    memcpy(addr + (APIMAC_SADDR_EXT_LEN / 2), (uint8_t *)&macAddr,
           (APIMAC_SADDR_EXT_LEN / 2));
}

static void *taskFxn(void *arg0);

/*! USER assert handler */
static void userAssertHandler(uint8_t reason);


void assertHandler(void)
{
}

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 * @brief  "main()" function - starting point
 */
int main()
{
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

#ifndef USE_DEFAULT_USER_CFG
    /* Register our MAC assert handler */
    macUser0Cfg[0].pAssertFP = assertHandler;
#endif

    /*
     Initialization for board related stuff, such as LEDs
     following TI-RTOS convention
     */
    Board_init();

#if defined(POWER_MEAS)
    /* Disable external flash for power measurements */
    Board_shutDownExtFlash();
#endif

#ifdef OSAL_PORT2TIRTOS
    _macTaskId = macTaskInit(macUser0Cfg);
#endif

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = APP_TASK_PRIORITY;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, APP_TASK_STACK_SIZE);
    if (retc != 0) {
        /* failed to set attributes */
        while (1) {}
    }

    retc = pthread_create(&appThread, &attrs, taskFxn, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1) {}
    }

#ifdef FREERTOS_SUPPORT
    startRfCbThread();
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
#else
    BIOS_start(); /* enable interrupts and start SYS/BIOS */
#endif
    return (0);
}

/*!
 * @brief  TIRTOS HWI Handler. The name of this function is set to
 *         M3Hwi.excHandlerFunc in app.cfg, you can disable this by
 *         setting it to NULL.
 *
 * @param  excStack - TIROS variable
 * @param  lr - TIROS variable
 */
void Main_excHandler(uint32_t *excStack, uint32_t lr)
{
    /* Intentionally not used */
    (void)excStack;
    (void)lr;

    /* Go to user-defined handler */
    userAssertHandler(MTSYS_ASSERT_RTOS);
}

/*!
 * @brief  assert handler.
 */
void Main_assertHandler(void)
{
    /* Go to user-defined handler */
    userAssertHandler(MTSYS_ASSERT_HAL);
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief  Function to initialize and run main applicatiion task
 *
 * @param  arg0 - TIROS variable
 */
static void *taskFxn(void *arg0)
{
    /* Intentionally not used */
    (void)arg0;

    /* The following code encapsulated in TI_154STACK_FPGA flag is used for
     * internal FPGA evaluation of the 15.4 Stack and should not be used with
     * TI hardware platforms. */
#ifdef TI_154STACK_FPGA
    /* FPGA build disables POWER constraints */
    Power_setConstraint(PowerCC26XX_IDLE_PD_DISALLOW);
    Power_setConstraint(PowerCC26XX_SB_DISALLOW);

    IOCPortConfigureSet(IOID_4, IOC_PORT_RFC_GPO0, IOC_IOMODE_NORMAL);
    IOCPortConfigureSet(IOID_5, IOC_PORT_RFC_GPI0, IOC_INPUT_ENABLE);
    IOCPortConfigureSet(IOID_15, IOC_PORT_RFC_TRC, IOC_IOMODE_NORMAL);
    // configure RF Core SMI Command Link
//    IOCPortConfigureSet(IOID_22, IOC_IOCFG0_PORT_ID_RFC_SMI_CL_OUT, IOC_STD_OUTPUT);
//    IOCPortConfigureSet(IOID_21, IOC_IOCFG0_PORT_ID_RFC_SMI_CL_IN, IOC_STD_INPUT);
#endif

#ifndef OSAL_PORT2TIRTOS
    /* Initialize ICall module */
    ICall_init();
#endif
    /* Copy the extended address from the CCFG area */
    CCFGRead_IEEE_MAC(ApiMac_extAddr);

    /* Check to see if the CCFG IEEE is valid */
    if(memcmp(ApiMac_extAddr, dummyExtAddr, APIMAC_SADDR_EXT_LEN) == 0)
    {
        /* Nothing at CCFG IEEE location, get the Primary IEEE */
        memcpy(ApiMac_extAddr, PRIM_IEEE, APIMAC_SADDR_EXT_LEN);
    }

#ifdef NV_RESTORE
    /* Setup the NV driver */
    NVOCMP_loadApiPtrs(&Main_user1Cfg.nvFps);

    if (Main_user1Cfg.nvFps.initNV)
    {
        Main_user1Cfg.nvFps.initNV(NULL);
    }
#endif

#ifdef OSAL_PORT2TIRTOS
    /* Start tasks of external images */
    MCP_task(_macTaskId);
#else
    /* Start tasks of external images */
    ICall_createRemoteTasks();

    /* Kick off co-processor application task */
    MCP_task();
#endif

    return NULL;
}

/*!
 * @brief  User-defined assert handler
 *
 * @param  Reason for the assertion
 */
static void userAssertHandler(uint8_t reason)
{
    while(1)
    {
        /* Record assert reason, then reset device */
        MtSys_resetReq(reason);
    }
}
