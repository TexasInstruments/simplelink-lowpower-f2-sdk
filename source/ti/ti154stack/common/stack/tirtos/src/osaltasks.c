/******************************************************************************

 @file  osaltasks.c

 @brief Settings and other functions that the user could set and change.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2013-2025, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <icall.h>
#include <icall_cc26xx_defs.h>
#include "zcomdef.h"

#include "osal.h"
#include "osal_tasks.h"
#include "osal_snv.h"

#include "ti_drivers_config.h"

#include "mac_user_config.h"
#include "crypto_mac_api.h"
#include <chipinfo.h>

#include "macstack.h"
#include "mac_low_level.h"

#include "hal_mcu.h"
#include "hw_ints.h"
#include "hal_trng_wrapper.h"

#ifdef FEATURE_MAC_SECURITY
#include "hal_aes.h"
#endif

#if defined DEBUG_SW_TRACE || defined DBG_ENABLE
/* Header files required for tracer enabling */
#include <ioc.h>
#include <hw_ioc.h>
#include <hw_memmap.h>
#endif // defined DEBUG_SW_TRACE || defined DBG_ENABLE

#ifdef TIMAC_ROM_IMAGE_BUILD
extern void TIMAC_ROM_Init( void );
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */

// The order in this table must be identical to the task initialization calls below in osalInitTask.
const pTaskEventHandlerFn tasksArr[] = {
  macStackEventLoop
};

const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );
uint16 *macTasksEvents;

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/


/*********************************************************************
 * @fn      osalInitTasks
 *
 * @brief   This function invokes the initialization function for each task.
 *
 * @param   void
 *
 * @return  none
 */
void osalInitTasks( void )
{
  uint8 taskID = 0;

  macTasksEvents = (uint16 *)osal_mem_alloc( sizeof( uint16 ) * tasksCnt);
  if (macTasksEvents == NULL)
  {
    /* Abort if memory alloc fails */
    ICall_abort();
  }
  osal_memset( macTasksEvents, 0, (sizeof( uint16 ) * tasksCnt));

  /* Initialize OSAL tasks */
  macStackTaskInit(taskID);
}

/**
 * Main entry function for the stack image
 */
int stack_main( void *arg )
{
  halIntState_t state;
#ifdef FEATURE_MAC_SECURITY
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2)
  CryptoCC26XX_Params CryptoCC26XXParams;
#else
  AESCCM_Params AESCCMParams;
#endif
#endif /* FEATURE_MAC_SECURITY */

#ifdef TIMAC_ROM_IMAGE_BUILD
  TIMAC_ROM_Init();
#endif

#if defined( DEBUG_SW_TRACE )
  #if defined(HAL_UART) && (HAL_UART==TRUE)
    // Enable tracer output on DIO24/ATEST1.
    // Set max drive strength and reduced slew rate.
    // PortID = 46 = RFcore tracer.
    HWREG(IOC_BASE + IOC_O_IOCFG24) = (3 << IOC_IOCFG24_IOSTR_S) | (1 << IOC_IOCFG24_SLEW_RED_S) | 46;

  #else // no UART so allow trace on Tx pin - Debug Package only

    // Enable tracer output on DIO23/ATEST0 or DIO24/ATEST1:
    // For 7x7 EM on SmartRF06EB:
    // Port ID for RF Tracer is 46
    // [see ioc.h: #define IOC_PORT_RFC_TRC  0x0000002E // RF Core Tracer]

    // DIO 23 maps to IOID 8
    // [see Board.h: #define Board_SPI0_MISO IOID_8     /* RF1.20 */]
    HWREG(IOC_BASE + IOC_O_IOCFG23) = (3 << IOC_IOCFG23_IOSTR_S) | 46;

    // OR

    // DIO 24 maps to IOID 24
    // [see Board.h: #define Board_SPI1_MISO IOID_24   /* RF2.10 */]
    //HWREG(IOC_BASE + IOC_O_IOCFG24) = (3 << IOC_IOCFG24_IOSTR_S) | 46;

  #endif // HAL_UART
#endif // DEBUG_SW_TRACE

  /* User App Reconfiguration of TIMAC */
  macSetUserConfig( arg );

  /* Register an entity to use timer service to implement OSAL timer */
  if (ICall_registerApp(&osal_entity, &osal_semaphore) !=
      ICALL_ERRNO_SUCCESS)
  {
    /* abort */
    ICall_abort();
  }

  /* Check device revision */
  if (((HwRevision_t (*)(void)) ((macUserCfg_t*) arg)->getHwRevision)() > HWREV_2_0)
  {
    /* PG2.0 and PG2.1 devices are supported by this version of software.
     * This stub is reserved for future HW version check if it becomes necessary.
     */
  }

  HAL_ENTER_CRITICAL_SECTION(state);

  // Have to turn on the TRNG power before HalTRNG_InitTRNG
  // but must not repeat it every time the device wakes up
  // hence the call cannot be added to HalTRNG_InitTRNG();

  ICall_pwrRequire(ICALL_PWR_D_PERIPH_TRNG);

  HalTRNG_InitTRNG();

#ifdef FEATURE_MAC_SECURITY
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2)
  extern CryptoCC26XX_Handle Crypto_handle;

  CryptoCC26XX_init();
  CryptoCC26XX_Params_init(&CryptoCC26XXParams);
  Crypto_handle = CryptoCC26XX_open(Board_CRYPTO0, false, &CryptoCC26XXParams);
  if (!Crypto_handle)
  {
    /* abort */
    ICall_abort();
  }

  HalAesInit();
#else
  extern AESCCM_Handle AESCCM_handle;

  AESCCM_init();
  AESCCM_Params_init(&AESCCMParams);
  AESCCMParams.returnBehavior = AESCCM_RETURN_BEHAVIOR_POLLING;
  AESCCM_handle = AESCCM_open(CONFIG_AESCCM_0, &AESCCMParams);
  if (!AESCCM_handle)
  {
    /* abort */
    ICall_abort();
  }
#endif
#endif /* FEATURE_MAC_SECURITY */

#ifdef ICALL_LITE
{
  extern void icall_slimMsgParser(void * msg);
  //osal_set_icall_hook((osal_icallMsg_hook_t*)icall_slimMsgParser);
  
  // use the common component from RTM branch
  osal_set_icall_hook((osal_icallMsg_hook_t)icall_slimMsgParser);
}
#endif /* ICALL_SLIM */
  /* Initialize MAC */
  MAC_Init();

#ifdef COPROCESSOR
  /* Initialize the MAC function tables and features */
  MAC_InitDevice();
#ifdef FEATURE_FULL_FUNCTION_DEVICE
  MAC_InitCoord();
#endif /* FEATURE_FULL_FUNCTION_DEVICE */
#else
  /* Initialize the MAC function tables and features */

#ifdef FEATURE_FULL_FUNCTION_DEVICE
  MAC_InitCoord();
#else /* FEATURE_FULL_FUNCTION_DEVICE */
  MAC_InitDevice();
#endif
#endif

#ifdef FEATURE_BEACON_MODE
  MAC_InitBeaconDevice();
#ifdef FEATURE_FULL_FUNCTION_DEVICE
  MAC_InitBeaconCoord();
#endif /* FEATURE_FULL_FUNCTION_DEVICE */
#endif /* FEATURE_BEACON_MODE */

#ifdef FEATURE_FREQ_HOP_MODE
  MAC_InitFH();
#endif /* FEATURE_FREQ_HOP_MODE */
  // Initialize the operating system
  osal_init_system();

   /* Initialize MAC buffer */
  macLowLevelBufferInit();

  /* Enable interrupts */
  HAL_EXIT_CRITICAL_SECTION(state);

  // Must be done last
  macLowLevelInit();

  osal_start_system(); // No Return from here

  return 0;  // Shouldn't get here.
}

/*********************************************************************
*********************************************************************/
