/******************************************************************************

 @file dmm_scheduler.h

 @brief DMM Scheduler

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2019-2025, Texas Instruments Incorporated

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
/*!****************************************************************************
 *  @file  dmm_scheduler.h
 *
 *  @brief      DMM Scheduler
 *
 *  DMM enables devices to run multiple wireless protocol stacks concurrently.
 *  The DMMSch module enables concurrent operation of multiple stacks with minimized conflicts
 *  so that it does not cause significant performance degradation.
 *  The DMMSch uses Application Level, Stack Level information, and Global Priority Table (GPT) when scheduling a command.
 *  Stack Level information (provided by stack) is embedded in each RF command and it includes:
 *  - Start Type, Start Time, AllowDelay, Priority, Activity, etc.
 *  Application Level information (provided by User via the Policy Table) includes:
 *  - Application State Name, Weight, AppliedActivity, Pause, etc.
 *  Global Priority Table (GPT)
 *  - GPT defines relative priorities of the two stacks
 *  The DMMSch uses Start Type, Start Time, and AllowDelay (Stack Level Information) when scheduling a RF command
 *  and tries to avoid a conflict by delaying the command if allowed.
 *  When a command conflicts with another (during scheduling or execution),
 *  The DMMSch selects one command over the other based on the final priority of two commands as below:
 *  Final Priority = GPT (Stack level priority) +  weight (Application Level)
 *
 *  # DMMSch API #
 *
 *  To use the DMMSch module to schedule a stack's RF commands, the application
 *  calls the following APIs:
 *    - DMMSch_init(): Initialize the DMMSch module/task.
 *    - DMMSch_Params_init():  Initialize a DMMSch_Params structure
 *      with default values.  Then change the parameters from non-default
 *      values as needed.
 *    - DMMSch_open():  Open an instance of the DMMSch module,
 *      passing the initialized parameters.
 *    - Stack A application - DMMSch_registerClient(): Passes Task_Handle and StackRole so
 *                            DMMSch can map the Task_Handle to the stack role
 *    - Stack A application - RF_open() -> DMMSch_rfOpen(): DMMSch overwrites the RF_Mode and
 *                            rf patches for multi-mode operation, maps RF Handle to the stack ID,
 *                            assigns phySwitchingTime for DMM operation.
 *                            From this point, Task_Handle, StackRole, and Stack ID are all related.
 *    - Stack B application - DMMSch_registerClient(): Passes Task_Handle and StackRole so
 *                            DMMSch can map the Task_Handle to the stack role
 *    - Stack B application - RF_open() -> DMMSch_rfOpen(): DMMSch overwrites the RF_Mode and
 *                            rf patches for multi-mode operation, maps RF Handle to the stack ID,
 *                            assigns phySwitchingTime for DMM operation.
 *                            From this point, Task_Handle, StackRole, and Stack ID are all related.
 *    - Stack A application - RF_scheduleCmd() -> DMMSch_rfScheduleCmd(): DMMSch adjusted timing based on policy
 *    - Stack B application - RF_scheduleCmd() -> DMMSch_rfScheduleCmd(): DMMSch adjusted timing based on policy
 *
 *
 ********************************************************************************/
#ifndef DMMSch_H_
#define DMMSch_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#include <ti/drivers/rf/RF.h>

#ifndef FREERTOS
#include <ti/sysbios/knl/Task.h>
#else
#include <FreeRTOS.h>
#include <task.h>
#endif
#include "dmm_policy.h"

/*!
 * @brief DMM Debug Logging
 * @anchor DMM_dbgLog
 *
 * Debugging for internal use only.
 * @{
 */
#define xDMM_DEBUG_LOGGING

#ifdef DMM_DEBUG_LOGGING
#include <xdc/runtime/Log.h>
#define DMM_dbgLog0(str)                     Log_info0(str)
#define DMM_dbgLog1(str, a0)                 Log_info1(str, a0)
#define DMM_dbgLog2(str, a0, a1)             Log_info2(str, a0, a1)
#define DMM_dbgLog3(str, a0, a1, a2)         Log_info3(str, a0, a1, a2)
#define DMM_dbgLog4(str, a0, a1, a2, a3)     Log_info4(str, a0, a1, a2, a3)
#define DMM_dbgLog5(str, a0, a1, a2, a3, a4) Log_info5(str, a0, a1, a2, a3, a4)
#else
#define DMM_dbgLog0(str)
#define DMM_dbgLog1(str, a0)
#define DMM_dbgLog2(str, a0, a1)
#define DMM_dbgLog3(str, a0, a1, a2)
#define DMM_dbgLog4(str, a0, a1, a2, a3)
#define DMM_dbgLog5(str, a0, a1, a2, a3, a4)
#endif
/** @} */

/** @brief RF parameter struct
 *  DMM Scheduler parameters are used with the DMMSch_open() and DMMSch_Params_init() call.
 */
typedef struct {
    DMMPolicy_StackRole stackRoles[DMMPOLICY_NUM_STACKS];       ///< Define the stacks that are supported by the scheduler
    DMMPolicy_StackCmdIndexTable *indexTable;                   ///< Define the index table for the scheduler
} DMMSch_Params;

/** @brief DMM Conflict struct
 *  DMM Conflict Status
 */
typedef enum
{
    DMM_NoConflict  = 0,        ///< No conflict in commands.
    DMM_ConflictWithPrev,       ///< There is a conflict with the previous command.
    DMM_ConflictWithNext,       ///< There is a conflict with the next command.
    DMM_ConfictWithBoth,        ///< There is a conflict with both previous and next commands.
} DMM_ConflictStatus;

/*!
 * @brief Define to flush all commands
 *
 * Default option to flush all commands for a specific client for a conflict
 */
#define CONFLICT_FLUSH_ALL       1

/**
 *  @brief Defines the preemption callback.
 *
 *  The function is invoked when a preemption occurs in DMM Scheduler.
 *
 * @param stackRolePreempted    Stack role for command was preempted.
 */
typedef void (*DMMSch_PreemptionCb)(DMMPolicy_StackRole stackRolePreempted);

/** @brief  Function to initialize the DMMSch_Params struct to its defaults
 *
 *  @param  params      Pointer to DMMSch_Params structure for initialization
 *
 *  @note   Any initialized struct fields with invalid values must be further
 *          initialized by the caller before calling DMMSch_open().
 *
 *  Defaults values are:
 *    - invalid DMMSch_Params::stackRoles
 *    - NULL DMMSch_Params::indexTable
 */
extern void DMMSch_Params_init(DMMSch_Params *params);

/** @brief  Function that initializes the DMMSch module
 *
 */
extern void DMMSch_init(void);

/** @brief  allows policy manager to register a callback on command preemption
 *
 *  @param  dmmSchPreemptionCb     callback to register
 */
extern void DMMSch_registerPreemptionCb(DMMSch_PreemptionCb dmmSchPreemptionCb);

/** @brief  Open the DMMSch module
 *
 *  @param  params      Pointer to initialized DMMSch_Params structure
 */
extern void DMMSch_open(DMMSch_Params *params);

/** @brief  Register an DMM Scheduler client
 *
 *  @param  pTaskHndl RTOS Thread handle that the stack is running in, used to map the
 *                    RF Client handle to a stack role
 *
 *  @param  StackRole   stack role associated with Task handle
 */
#ifndef FREERTOS
void DMMSch_registerClient(Task_Handle* pTaskHndl, DMMPolicy_StackRole StackRole);
#else
extern void DMMSch_registerClient(TaskHandle_t pTaskHndl, DMMPolicy_StackRole StackRole);
#endif

/** @brief  De-Register an DMM Scheduler client, removing all cached data regarding a client from the DMM scheduler.
 *          Note: This API will also attempt to close the DMM client's RF handle via RF_Close() if not already closed.
 *
 *  @param  pTaskHndl RTOS Thread handle that the stack is running in, used to map the
 *                    RF Client handle to a stack role
 *
 */
#ifndef FREERTOS
void DMMSch_deregisterClient(Task_Handle *pTaskHndl);
#else
extern void DMMSch_deregisterClient(TaskHandle_t pTaskHndl);
#endif

/** @brief              Intercepts calls from a stack to RF_postCmd (re-mapped to DMMSch_rfOpen),
 *                      The DMMSch module uses this to tie
 *
 * @param pObj          Pointer to RF Object
 * @param pRfMode       Pointer to RF Mode
 * @param pOpSetup      Pointer to Radio Setup
 * @param params        Pointer to RF params
 *
 * @return              Handle to DMMSch RF instance
 */
extern RF_Handle DMMSch_rfOpen(RF_Object *pObj, RF_Mode *pRfMode, RF_RadioSetup *pOpSetup, RF_Params *params);

/** @brief              Intercepts calls from a stack to RF_close
 *                      The DMMSch module uses this to safely close an RF instance through DMM.
 *                      Note that this API does NOT de-register the stack with the DMM scheduler.
 *                      This **Must** be used in tandem with DMMSch_deregisterClient to remove all client data from the DMM scheduler.
 * @param h             Pointer to RF Handle
 */
extern void DMMSch_rfClose(RF_Handle h);

/**
 *  @brief  Handles calls from a stack to RF_postCmd (re-mapped to DMMSch_postCmd),
 *  adjusts timing as necessary and schedules then accordingly with RF_scheduleCmd.
 *
 *  @sa RF_pendCmd(), RF_runCmd(), RF_scheduleCmd(), RF_RF_cancelCmd(), RF_flushCmd(), RF_getCmdOp()
 *
 *  @param h         Driver handle previously returned by RF_open()
 *  @param pOp       Pointer to the RF operation command.
 *  @param ePri      Priority of this RF command (used for arbitration in multi-client systems)
 *  @param pCb       Callback function called during command execution and upon completion.
 *                   If RF_postCmd() fails, no callback is made.
 *  @param bmEvent   Bitmask of events that will trigger the callback or that can be pended on.
 *  @return          A handle to the RF command. Return value of RF_ALLOC_ERROR indicates error.
 */
extern RF_CmdHandle DMMSch_rfPostCmd(RF_Handle h, RF_Op* pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent);

/**
 *  @brief  Handles calls from a stack to RF_scheduleCmd (re-mapped to DMMSch_scheduleCmd),
 *          adjusts timing as necessary and schedules then accordingly with RF_scheduleCmd.
 *
 *  @param h         Handle previously returned by RF_open()
 *  @param pOp       Pointer to the RF_Op. Must normally be in persistent and writeable memory
 *  @param pSchParams Pointer to the schedule command parameter structure
 *  @param pCb       Callback function called upon command completion (and some other events).
 *                   If RF_scheduleCmd() fails no callback is made
 *  @param bmEvent   Bitmask of events that will trigger the callback.
 *  @return          A handle to the RF command. Return value of RF_ALLOC_ERROR indicates error.
 */
extern RF_CmdHandle DMMSch_rfScheduleCmd(RF_Handle h, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams, RF_Callback pCb, RF_EventMask bmEvent);

/** @brief  Handles calls from a stack to RF_runCmd (re-mapped to DMMSch_runCmd),
 *          adjusts timing as necessary and schedules then accordingly with RF_scheduleCmd.
 *
 *  @param h         Driver handle previously returned by RF_open()
 *  @param pOp       Pointer to the RF operation command.
 *  @param ePri      Priority of this RF command (used for arbitration in multi-client systems)
 *  @param pCb       Callback function called during command execution and upon completion.
 *                   If RF_runCmd() fails, no callback is made.
 *  @param bmEvent   Bitmask of events that will trigger the callback or that can be pended on.
 *  @return          The relevant termination event.
 */
extern RF_EventMask DMMSch_rfRunCmd(RF_Handle h, RF_Op* pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent);

/**
 *  @brief  Handles calls from a stack to RF_runScheduleCmd (re-mapped to DMMSch_runScheduleCmd),
 *          adjusts timing as necessary and schedules then accordingly with RF_scheduleCmd.
 *
 *  @param h         Handle previously returned by RF_open()
 *  @param pOp       Pointer to the RF_Op. Must normally be in persistent and writeable memory
 *  @param pSchParams Pointer to the schedule command parameter structure
 *  @param pCb       Callback function called upon command completion (and some other events).
 *                   If RF_runScheduleCmd() fails, no callback is made.
 *  @param bmEvent   Bitmask of events that will trigger the callback.
 *  @return          The relevant command completed event.
 */
extern RF_EventMask DMMSch_rfRunScheduleCmd(RF_Handle h, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams, RF_Callback pCb, RF_EventMask bmEvent);

/**
 *  @brief  Abort/stop/cancel single command in command queue.
 *
 *  If command is running, aborts/stops it and posts callback for the
 *  aborted/stopped command. <br>
 *  If command has not yet run, cancels it it and posts callback for the
 *  cancelled command. <br>
 *  If command has already run or been aborted/stopped/cancelled, has no effect.<br>
 *  If RF_cancelCmd is called from a Swi context with same or higher priority
 *  than RF Driver Swi, when the RF core is powered OFF -> the cancel callback will be delayed
 *  until the next power-up cycle.<br>
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param ch           Command handle previously returned by RF_postCmd().
 *  @param mode         1: Stop gracefully, 0: abort abruptly
 *  @return             RF_Stat indicates if command was successfully completed
 */
extern RF_Stat DMMSch_rfCancelCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mode);

/**
 *  @brief  Abort/stop/cancel command and any subsequent commands in command queue.
 *
 *  If command is running, aborts/stops it and then cancels all later commands in queue.<br>
 *  If command has not yet run, cancels it and all later commands in queue.<br>
 *  If command has already run or been aborted/stopped/cancelled, has no effect.<br>
 *  The callbacks for all cancelled commands are issued in chronological order.<br>
 *  If RF_flushCmd is called from a Swi context with same or higher priority
 *  than RF Driver Swi, when the RF core is powered OFF -> the cancel callback will be delayed
 *  until the next power-up cycle.<br>
 *
 *  @note Calling context : Task/SWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param ch           Command handle previously returned by RF_postCmd().
 *  @param mode         1: Stop gracefully, 0: abort abruptly
 *  @return             RF_Stat indicates if command was successfully completed
 */
extern RF_Stat DMMSch_rfFlushCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mode);

/**
 *  @brief Send any Immediate command. <br>
 *
 *  Immediate Command is send to RDBELL, if radio is active and the RF_Handle points
 *  to the current client. <br>
 *  In other appropriate RF_Stat values are returned. <br>
 *
 *  @note Calling context : Task/SWI/HWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param pCmdStruct   Pointer to the immediate command structure
 *  @return             RF_Stat indicates if command was successfully completed
*/
extern RF_Stat DMMSch_rfRunImmediateCmd(RF_Handle h, uint32_t* pCmdStruct);

/**
 *  @brief Send any Direct command. <br>
 *
 *  Direct Command value is send to RDBELL immediately, if radio is active and
 *  the RF_Handle point to the current client. <br>
 *  In other appropriate RF_Stat values are returned. <br>
 *
 *  @note Calling context : Task/SWI/HWI
 *
 *  @param h            Handle previously returned by RF_open()
 *  @param cmd          Direct command value.
 *  @return             RF_Stat indicates if command was successfully completed.
*/
extern RF_Stat DMMSch_rfRunDirectCmd(RF_Handle h, uint32_t cmd);


/** @brief  Turn on Block mode
 *
 *  @param  stackRole   stack role associated with Task handle
 *  @return true: success, false: the stack role cannot be found
 */
extern bool DMMSch_setBlockModeOn(DMMPolicy_StackRole stackRole);

/** @brief  Turn off Block mode
 *
 *  @param  stackRole   stack role associated with Task handle
 *  @return true: success, false: the stack role cannot be found
 */
extern bool DMMSch_setBlockModeOff(DMMPolicy_StackRole stackRole);

/** @brief  Get Block mode status
 *
 *  @param  stackRole   stack role associated with Task handle
 *  @return true: Block Mode is On, False: Block Mode is Off
 */
extern bool DMMSch_getBlockModeStatus(DMMPolicy_StackRole stackRole);

/**
 *  @brief Request access RF API that should not be used in DMM <br>
*/
extern RF_Stat DMMSch_rfRequestAccess(RF_Handle h, RF_AccessParams *pParams);

#ifdef __cplusplus
}
#endif

#endif /* DMMSch_H_ */
