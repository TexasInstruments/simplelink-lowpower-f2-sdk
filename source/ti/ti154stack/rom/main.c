/******************************************************************************

 @file main.c

 @brief main entry of the example application

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

#include <xdc/std.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <ioc.h>

#include "sys_ctrl.h"

#include "board.h"
#include <inc/hw_ccfg.h>
#include <inc/hw_ccfg_simple_struct.h>

/* Header files required for the temporary idle task function */
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

/* Header files required to enable instruction fetch cache */
#ifdef NV_RESTORE
#include "macconfig.h"
#include "nvocmp.h"
#endif

#include <string.h>

#include "api_mac.h"
#include "uart_printf.h"

#ifndef USE_DEFAULT_USER_CFG

#include "mac_user_config.h"

/* MAC user defined configuration */
//macUserCfg_t user0Cfg[] = MAC_USER_CFG;

#endif /* USE_DEFAULT_USER_CFG */

#include "rom_jt_154.h"
/******************************************************************************
 Constants
 *****************************************************************************/

/******************************************************************************
 External Variables
 *****************************************************************************/

/******************************************************************************
 Global Variables
 *****************************************************************************/

/*
 When assert happens, this field will be filled with the reason:
       MAIN_ASSERT_HWI_TIRTOS,
       MAIN_ASSERT_ICALL,
       MAIN_ASSERT_MAC
 */
uint8 Main_assertReason = 0;

#ifdef NV_RESTORE
mac_Config_t Main_user1Cfg =
    { 0 };
#endif

#if defined(BOARD_DISPLAY_USE_UART)
UART_Params uartParams;
#endif

void halAssertHandler(void)
{
    /* User defined function */
    //Main_assertHandler(MAIN_ASSERT_ICALL);
}
/******************************************************************************
 Local Variables
 *****************************************************************************/
volatile int32_t ret=0;
void FH_ROM_Init(void);

/* define TIMAC_ROM version */
#pragma DATA_SECTION(TIMAC_ROM_FW_VERSION_NUMBER,"timac_rom_fw_version")
const uint32 TIMAC_ROM_FW_VERSION_NUMBER=0x02000006;

/*!
 * @brief       "main()" function - starting point
 */
Void main()
{
    /* dummy main */
    TIMAC_ROM_Init();

    ret = 1;

}

