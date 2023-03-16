/*
 * Copyright (c) 2022, Texas Instruments Incorporated
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
 *  ======== TaskP_tirtos.c ========
 */

#include <ti/drivers/dpl/TaskP.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/knl/Task.h>

/*
 *  ======== Array for conversion of TI-RTOS task state to DPL task state ========
 */
const TaskP_State taskState[] = {TaskP_State_RUNNING,   /*!< Task_Mode_RUNNING */
                                 TaskP_State_READY,     /*!< Task_Mode_READY */
                                 TaskP_State_BLOCKED,   /*!< Task_Mode_BLOCKED */
                                 TaskP_State_DELETED,   /*!< Task_Mode_TERMINATED */
                                 TaskP_State_INACTIVE}; /*!< Task_Mode_INACTIVE */

/*
 *  ======== Default TaskP_Params values ========
 */
static const TaskP_Params TaskP_defaultParams = {
    .name      = "NAME",
    .arg       = NULL,
    .priority  = 1,
    .stackSize = TaskP_DEFAULT_STACK_SIZE,
    .stack     = NULL,
};

/*
 *  ======== TaskP_Params_init ========
 */
void TaskP_Params_init(TaskP_Params *params)
{
    /* structure copy */
    *params = TaskP_defaultParams;
}

/*
 *  ======== TaskP_create ========
 */
TaskP_Handle TaskP_create(TaskP_Function fxn, const TaskP_Params *params)
{
    Task_Handle task = NULL;

    if (params == NULL)
    {
        /* Set default parameter values */
        params = &TaskP_defaultParams;
    }

    /* Configure task params */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.name      = params->name;
    taskParams.arg0      = (uintptr_t)params->arg;
    taskParams.arg1      = 0;
    taskParams.priority  = params->priority;
    taskParams.stack     = params->stack;
    taskParams.stackSize = params->stackSize;

    /* Create the task using dynamic memory allocation */
    task = Task_create((Task_FuncPtr)fxn, &taskParams, NULL);

    return ((TaskP_Handle)task);
}

/*
 *  ======== TaskP_delete ========
 */
void TaskP_delete(TaskP_Handle task)
{
    if (task != NULL)
    {
        /* Delete the task that was created by use of dynamic memory allocation */
        Task_delete((Task_Handle *)&task);
    }
}

/*
 *  ======== TaskP_construct ========
 */
TaskP_Handle TaskP_construct(TaskP_Struct *obj, TaskP_Function fxn, const TaskP_Params *params)
{
    /* Configure task params */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.name      = params->name;
    taskParams.arg0      = (uintptr_t)params->arg;
    taskParams.priority  = params->priority;
    taskParams.stack     = params->stack;
    taskParams.stackSize = params->stackSize;

    /* Construct the task using static memory */
    return ((TaskP_Handle)Task_construct((Task_Struct *)obj, (Task_FuncPtr)fxn, &taskParams, NULL));
}

/*
 *  ======== TaskP_destruct ========
 */
void TaskP_destruct(TaskP_Struct *obj)
{
    /* Destruct the task that was constructed using static memory */
    if (obj != NULL)
    {
        Task_destruct((Task_Struct *)obj);
    }
}

/*
 *  ======== TaskP_getState ========
 */
TaskP_State TaskP_getState(TaskP_Handle task)
{
    Task_Stat statBuf;

    Task_stat((Task_Handle)task, &statBuf);

    /* Convert from TI-RTOS task state to DPL task state */
    return (taskState[statBuf.mode]);
}

/*
 *  ======== TaskP_getCurrentTask ========
 */
TaskP_Handle TaskP_getCurrentTask(void)
{
    return ((TaskP_Handle)Task_self());
}

/*
 *  ======== TaskP_disableScheduler ========
 */
uintptr_t TaskP_disableScheduler(void)
{
    return ((uintptr_t)Task_disable());
}

/*
 *  ======== TaskP_restoreScheduler ========
 */
void TaskP_restoreScheduler(uintptr_t key)
{
    Task_restore((unsigned int)key);
}

/*
 *  ======== TaskP_yield ========
 */
void TaskP_yield(void)
{
    Task_yield();
}

/*
 *  ======== TaskP_getTaskObjectSize ========
 */
uint32_t TaskP_getTaskObjectSize(void)
{
    return (sizeof(Task_Struct));
}
