/******************************************************************************

 @file main.c

 @brief main entry of the example application

 Group: WCS LPC
 Target Device: CC2652

 ******************************************************************************

 Copyright (c) 2016-2017, Texas Instruments Incorporated
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
 Release Name: simplelink_zigbee_sdk_plugin_0_95_00_18_s
 Release Date: 2017-11-10 13:43:40
 *****************************************************************************/

/******************************************************************************
 Includes
 *****************************************************************************/

#include <string.h>
#include <xdc/std.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <ioc.h>

#include "sys_ctrl.h"

#include "ti_drivers_config.h"
#include <inc/hw_ccfg.h>
#include <inc/hw_ccfg_simple_struct.h>

/* Header files required for the temporary idle task function */
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <aon_rtc.h>
#include <prcm.h>

#ifndef TIMAC_ROM_IMAGE_BUILD
#include "crypto_mac_api.h"
#endif
#include <chipinfo.h>

/* Header files required to enable instruction fetch cache */
#include <vims.h>
#include <hw_memmap.h>

#include <ti/sysbios/hal/Hwi.h>

#include "cpu.h"

#include "nvocmp.h"

#include "zstackconfig.h"

#ifdef ZSTACK_GPD
#include "macTask.h"
#else
#include "zstackstartup.h"
#include "zstackapi.h"
#endif

#ifndef CUI_DISABLE
#include "cui.h"
#endif


#include "ti_zstack_config.h"

/******************************************************************************
 Constants
 *****************************************************************************/
#define CONFIG_PHY_ID 0

/* Assert Reasons */
#define MAIN_ASSERT_STACK        2
#define MAIN_ASSERT_MAC          3
#define MAIN_ASSERT_HWI_TIRTOS   4

#define RFC_MODE_BLE                 PRCM_RFCMODESEL_CURR_MODE1
#define RFC_MODE_IEEE                PRCM_RFCMODESEL_CURR_MODE2
#define RFC_MODE_ANT                 PRCM_RFCMODESEL_CURR_MODE4
#define RFC_MODE_EVERYTHING_BUT_ANT  PRCM_RFCMODESEL_CURR_MODE5
#define RFC_MODE_EVERYTHING          PRCM_RFCMODESEL_CURR_MODE6

/* Extended Address offset in FCFG (LSB..MSB) */
#define EXTADDR_OFFSET 0x2F0


#define APP_TASK_STACK_SIZE 3000

#define SET_RFC_MODE(mode) HWREG( PRCM_BASE + PRCM_O_RFCMODESEL ) = (mode)

// Exented Address Length
#define EXTADDR_LEN 8

// Macro used to break a uint32_t into individual bytes
//#define BREAK_UINT32(var, ByteNum) \
//    (uint8_t)((uint32_t)(((var) >> ((ByteNum) * 8)) & 0x00FF))

/******************************************************************************
 External Variables
 *****************************************************************************/
#ifdef ZSTACK_GPD
extern ApiMac_sAddrExt_t ApiMac_extAddr;
#endif

/******************************************************************************
 Global Variables
 *****************************************************************************/

// ZNP does not need an application task
#ifndef ZNP_NPI
Task_Struct myTask;
Char myTaskStack[APP_TASK_STACK_SIZE];
#endif

#ifndef TIMAC_ROM_IMAGE_BUILD
/* Crypto driver function table */
uint32_t *macCryptoDrvTblPtr = (uint32_t*)macCryptoDriverTable;
#endif

/*
 When assert happens, this field will be filled with the reason:
       MAIN_ASSERT_HWI_TIRTOS,
       MAIN_ASSERT_STACK,
       MAIN_ASSERT_MAC
 */
uint8_t Main_assertReason = 0;

/******************************************************************************
 * ZStack Configuration Structure
 */
zstack_Config_t zstack_user0Cfg =
{
    {0, 0, 0, 0, 0, 0, 0, 0}, // Extended Address
    {0, 0, 0, 0, 0, 0, 0, 0}, // NV function pointers
    0,                        // Application thread ID
    0,                        // stack image init fail flag
    MAC_USER_CFG
};

/* Stack TIRTOS Task semaphore */
Semaphore_Struct npiInitializationMutex;
Semaphore_Handle npiInitializationMutexHandle;

/******************************************************************************
 Local Variables
 *****************************************************************************/

#ifdef ZSTACK_GPD
uint8_t _macTaskId;
#elif !defined (ZNP_NPI)
static uint8_t stackServiceTaskId;
#endif

/*!
 * @brief       Fill in your own assert function.
 *
 * @param       assertReason - reason: MAIN_ASSERT_HWI_TIRTOS,
 *                                     MAIN_ASSERT_STACK, or
 *                                     MAIN_ASSERT_MAC
 */
void Main_assertHandler(uint8_t assertReason)
{
    Main_assertReason = assertReason;

#if defined(RESET_ASSERT)
     /* Pull the plug and start over */
    SysCtrlSystemReset();
#else
    Hwi_disable();
    char assertLine[21] = {'\0'};
    System_snprintf(assertLine, 21, "[Assert Reason] 0x%02X", assertReason);
    while(1)
    {
        /* Put you code here to do something if in assert */
        #ifndef CUI_DISABLE
        CUI_assert(assertLine, TRUE);
        #endif
    }
#endif
}

/*!
 * @brief       Main task function
 *
 * @param       a0 -
 * @param       a1 -
 */
#ifndef ZNP_NPI
Void taskFxn(UArg a0, UArg a1)
{
#if defined(USE_CACHE_RAM)
    /* Retain the Cache RAM */
    Power_setConstraint(PowerCC26XX_SB_VIMS_CACHE_RETAIN);
#endif

#ifdef ZSTACK_GPD

#else
    /* get the service taskId of the Stack */
    stackServiceTaskId = stackTask_getStackServiceId();
    /* configure the message API the application will use to communicate with
       the stack */
    Zstackapi_init(stackServiceTaskId);
#endif

    /* Kick off application */
    extern void sampleApp_task(NVINTF_nvFuncts_t *pfnNV);
    sampleApp_task(&zstack_user0Cfg.nvFps);
}

#endif

/*!
 * @brief       TIRTOS HWI Handler.  The name of this function is set to
 *              M3Hwi.excHandlerFunc in app.cfg, you can disable this by
 *              setting it to null.
 *
 * @param       excStack - TIROS variable
 * @param       lr - TIROS variable
 */
xdc_Void Main_excHandler(UInt *excStack, UInt lr)
{
    /* User defined function */
    Main_assertHandler(MAIN_ASSERT_HWI_TIRTOS);
}

/*!
 * @brief       HAL assert handler required by OSAL memory module.
 */
void assertHandler(void)
{
    /* User defined function */
    Main_assertHandler(MAIN_ASSERT_MAC);
}

/*!
 * @brief       Callback function when voltage is lower than NVOCMP_MIN_VDD_FLASH_MV
 *              during an NV write operation
 *
 * @param       voltage - Measured device voltage
 */
#ifdef NVOCMP_MIN_VDD_FLASH_MV
void Main_lowVoltageCb(uint32_t voltage)
{
    /* Implement any safety precautions for application due to low voltage detected */
}
#endif

/*!
 * @brief       "main()" function - starting point
 */
int main()
{
#ifndef USE_DEFAULT_USER_CFG
    zstack_user0Cfg.macConfig.pAssertFP = assertHandler;
#endif

    /* enable iCache prefetching */
    VIMSConfigure(VIMS_BASE, TRUE, TRUE);

#if defined(USE_CACHE_RAM)
    /* Disable cache */
    VIMSModeSet( VIMS_BASE, VIMS_MODE_DISABLED);
#else
    /* Enable cache */
    VIMSModeSet( VIMS_BASE, VIMS_MODE_ENABLED);
#endif

    /*
     Initialization for board related stuff such as LEDs
     following TI-RTOS convention
     */
    Board_initGeneral();

#ifdef THOR_FPGA
    // FPGA can't sleep
    Power_setConstraint(PowerCC26XX_IDLE_PD_DISALLOW);
    Power_setConstraint(PowerCC26XX_SB_DISALLOW);
#endif

// OTA client projects use BIM, so CCFG isn't present in this image
#if !((defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED))
    /*
     * Copy the extended address from the CCFG area
     * Assumption: the memory in CCFG_IEEE_MAC_0 and CCFG_IEEE_MAC_1
     * is contiguous and LSB first.
     */
    /* Used to check for a valid extended address */
    static const uint8_t dummyExtAddr[] =
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    OsalPort_memcpy(zstack_user0Cfg.extendedAddress, (uint8_t *)&(__ccfg.CCFG_IEEE_MAC_0),
           (APIMAC_SADDR_EXT_LEN));
    /* Check to see if the CCFG IEEE is valid */
    if(memcmp(zstack_user0Cfg.extendedAddress, dummyExtAddr, APIMAC_SADDR_EXT_LEN) == 0)
#endif // (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED
    {
        /* No, it isn't valid.  Get the Primary IEEE Address */
        OsalPort_memcpy(zstack_user0Cfg.extendedAddress, (uint8_t *)(FCFG1_BASE + EXTADDR_OFFSET),
                       (APIMAC_SADDR_EXT_LEN));
    }

    /* Setup the NV driver */
    NVOCMP_loadApiPtrs(&zstack_user0Cfg.nvFps);
#ifdef NVOCMP_MIN_VDD_FLASH_MV
    NVOCMP_setLowVoltageCb(&Main_lowVoltageCb);
#endif
    if(zstack_user0Cfg.nvFps.initNV)
    {
        zstack_user0Cfg.nvFps.initNV(NULL);
    }

#ifdef ZSTACK_GPD
    OsalPort_memcpy(ApiMac_extAddr, zstack_user0Cfg.extendedAddress,
           (APIMAC_SADDR_EXT_LEN));
    _macTaskId = macTaskInit(&zstack_user0Cfg.macConfig);
#else
    /* configure stack task */
    stackTask_init(&zstack_user0Cfg);
#endif // ZSTACK_GPD

#ifdef NPI
    extern void NPITask_createTask(void);
    NPITask_createTask();

    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&npiInitializationMutex, 0, &semParam);
    npiInitializationMutexHandle = Semaphore_handle(&npiInitializationMutex);
#endif

// ZNP does not need an application task
#ifndef ZNP_NPI

#ifndef CUI_DISABLE
    CUI_params_t cuiParams;

    CUI_paramsInit(&cuiParams);
    CUI_init(&cuiParams);
#endif
    Task_Params taskParams;

    /* Configure app task. */
    Task_Params_init(&taskParams);
    taskParams.stack = myTaskStack;
    taskParams.stackSize = APP_TASK_STACK_SIZE;
    taskParams.priority = 2;
    Task_construct(&myTask, taskFxn, &taskParams, NULL);

#endif

#ifdef DEBUG_SW_TRACE
    IOCPortConfigureSet(IOID_8, IOC_PORT_RFC_TRC, IOC_STD_OUTPUT
                    | IOC_CURRENT_4MA | IOC_SLEW_ENABLE);
#endif /* DEBUG_SW_TRACE */

    BIOS_start(); /* enable interrupts and start SYS/BIOS */

    return 0; // never executed
}
