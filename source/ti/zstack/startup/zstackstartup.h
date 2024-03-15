/******************************************************************************
 @file

 @brief

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2004-2024, Texas Instruments Incorporated
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

#ifndef STACK_TASK_H
#define STACK_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zstackconfig.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      osalInitTasks
 *
 * @brief   This function creates the TIRTOS task for the Stack, registers the
 *          stack layers service task with OsalPort messaging and calls each of
 *          the service task init functions.
 *
 * @param   pUserCfg The stack onfiguration structure
 *
 * @return  void
 */
extern void stackTask_init( zstack_Config_t*  pUserCfg );

/*********************************************************************
 * @fn      stackTask_init
 *
 * @brief   This function creates the TIRTOS task for the Stack, registers the
 *          stack layers service functions with OsalPort messaging and calls each of
 *          the service task init functions.
 *
 * @param   none
 *
 * @return  task service ID for the main stack service task
 */
extern uint8_t stackTask_getStackServiceId(void);

/**************************************************************************************************
 * @fn          stackTaskGetTaskHndl
 *
 * @brief       This function returns the TIRTOS Task handle of the Stack Task.
 *
 * input parameters
 *
 * @param       pUserCfg - MAC user config
 *
 * output parameters
 *
 *
 * @return      Stack Task ID.
 **************************************************************************************************
 */
extern Task_Handle* stackTaskGetTaskHndl(void);

#ifdef IEEE_COEX_ENABLED
/**************************************************************************************************
 * @fn          stackTask_EnableCoex
 *
 * @brief       This enables or disables coex in Zstack.
 *
 * @param       enabled - true if enabling Coex, false if disabling
 *
 *
 * @return      none
 **************************************************************************************************
 */
extern void stackTask_EnableCoex(bool enabled);
#endif /* IEEE_COEX_ENABLED */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* STACK_TASK_H */
