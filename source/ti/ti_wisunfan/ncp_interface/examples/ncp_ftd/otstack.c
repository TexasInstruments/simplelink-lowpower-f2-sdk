/******************************************************************************

 @file otstack.c

 @brief OpenThread stack processing and instantiation and handling of
        application CoAP server.

 Group: CMCU, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2020, Texas Instruments Incorporated
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
#include <openthread/config.h>
#ifdef OPENTHREAD_PROJECT_CORE_CONFIG_FILE
//#include OPENTHREAD_PROJECT_CORE_CONFIG_FILE
#include "openthread-core-cc1352-config-ide.h"
#endif

/* Standard Library Header files */
#include <assert.h>
#include <stddef.h>
#include <string.h>

/* POSIX Header files */
#include <mqueue.h>
#include <pthread.h>
#include <sched.h>

/* OpenThread public API Header files */
#include <openthread/coap.h>
#include <openthread/dataset.h>
#include <openthread/diag.h>
#include <openthread/joiner.h>
#include <openthread/platform/settings.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>

/* OpenThread Internal/Example Header files */
#include "otsupport/otinstance.h"
#include "otsupport/otrtosapi.h"
#include "platform/system.h"

/* Example/Board Header files */
#include "ti_drivers_config.h"
#include "otstack.h"
#include "task_config.h"
#include "utils/code_utils.h"

#include <ti/drivers/GPIO.h>
#include <openthread/ncp.h>

#if 0
/* include files for TI MAC */
#include "macTask.h"

/* include files for mbed nanostack */
#include "mesh_system.h"
#include "borderrouter_tasklet.h"
#include "ws_management_api.h"
#include "mesh_interface_types.h"
#endif

#include "eventOS_event.h"
#include "net_interface.h"

/* Configuration Header files */
//#include "tiop_config.h"

/* OpenThread Stack thread */
extern void *OtStack_task(void *arg0);

#if 0
extern mesh_error_t nanostack_wisunInterface_configure(void);
#endif

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/**
 * @brief Size of the message queue for `OtStack_procQueue`
 *
 * Size determined by:
 *  7   main processing loop commands
 *  6   radio process requests
 *  2   UART process requests
 * +1   buffer
 * -----------------------------------
 *  16  queue slots
 */
#define OTSTACK_PROC_QUEUE_MAX_MSG      (16)

enum OtStack_procQueueCmd
{
    //OtStack_procQueueCmd_alarm,
    //OtStack_procQueueCmd_radio,
    OtStack_procQueueCmd_tasklets,
    OtStack_procQueueCmd_uart,
    //OtStack_procQueueCmd_random,
    //OtStack_procQueueCmd_alarmu,
    //OtStack_procQueueCmd_spi,
};

struct OtStack_procQueueMsg {
    enum OtStack_procQueueCmd cmd;
    uintptr_t arg;
};

/******************************************************************************
 Local variables
 *****************************************************************************/

static otInstance *OtStack_instance = NULL;

/* POSIX message queue for passing state to the processing loop */
const  char  OtStack_procQueueName[] = "/tiop_process";
static mqd_t OtStack_procQueueDesc;

/* OpenThread Stack thread call stack */
static char OtStack_stack[TASK_CONFIG_OT_TASK_STACK_SIZE];

volatile OtStack_procWisunCmd g_wisunCmd = OtStack_procWisunCmd_none;

#if 0
extern uint8_t timacTaskId;
extern macUserCfg_t macUser0Cfg[];
#endif

/******************************************************************************
 External Functions
 *****************************************************************************/

/**
 * Documented in task_config.h.
 */
void OtStack_taskCreate(void)
{
    pthread_t           thread;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;
    int                 retc;

    /* create api gate */
    OtRtosApi_init();

    retc = pthread_attr_init(&pAttrs);
    assert(retc == 0);

    retc = pthread_attr_setdetachstate(&pAttrs, PTHREAD_CREATE_DETACHED);
    assert(retc == 0);

    priParam.sched_priority = TASK_CONFIG_OT_TASK_PRIORITY;
    retc = pthread_attr_setschedparam(&pAttrs, &priParam);
    assert(retc == 0);

    retc = pthread_attr_setstack(&pAttrs, (void *)OtStack_stack,
                                 TASK_CONFIG_OT_TASK_STACK_SIZE);
    assert(retc == 0);

    retc = pthread_create(&thread, &pAttrs, OtStack_task, NULL);
    assert(retc == 0);

    retc = pthread_attr_destroy(&pAttrs);
    assert(retc == 0);

    (void) retc;
}

/**
 * Callback from OpenThread stack to indicate tasklets are pending processing.
 */
void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
    struct OtStack_procQueueMsg msg;
    int                         ret;
    msg.cmd = OtStack_procQueueCmd_tasklets;
    ret = mq_send(OtStack_procQueueDesc, (const char *)&msg, sizeof(msg), 0);
    assert(0 == ret);

    (void) ret;
}



/**
 * Documented in otsupport/otinstance.h.
 */
otInstance *OtInstance_get(void)
{
    otInstance *ret;

    if (NULL != OtStack_instance)
    {
        ret = OtStack_instance;
    }
    else
    {
        /* lock and unlock the API to make sure that the stack is initialized
         *  before the caller tries to use this pointer.
         */
        OtRtosApi_lock();
        ret = OtStack_instance;
        OtRtosApi_unlock();
    }

    return ret;
}

/**
 * Main processing thread for OpenThread Stack.
 */
void *OtStack_task(void *arg0)
{
    struct mq_attr attr;
    mqd_t          procQueueLoopDesc;

    attr.mq_curmsgs = 0;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = OTSTACK_PROC_QUEUE_MAX_MSG;
    attr.mq_msgsize = sizeof(struct OtStack_procQueueMsg);

    /* Open The processing queue in non-blocking write mode for the notify
     * callback functions
     */
    OtStack_procQueueDesc = mq_open(OtStack_procQueueName,
                                    (O_WRONLY | O_NONBLOCK | O_CREAT),
                                    0, &attr);

    /* Open the processing queue in blocking read mode for the process loop */
    procQueueLoopDesc = mq_open(OtStack_procQueueName, O_RDONLY, 0, NULL);

    OtStack_instance = otInstanceInitSingle();
    assert(OtStack_instance);

    otNcpInit(OtStack_instance);

    /* allow the application to lock the API */
    OtRtosApi_unlock();

    while (1)
    {
        struct OtStack_procQueueMsg msg;
        ssize_t ret;

        ret = mq_receive(procQueueLoopDesc, (char *)&msg, sizeof(msg), NULL);
        /* priorities are ignored */
        if (ret < 0 || ret != sizeof(msg))
        {
            /* there was an error on receive or we did not receive a full message */
            continue;
        }

        OtRtosApi_lock();
        switch (msg.cmd)
        {
            case OtStack_procQueueCmd_tasklets:
            {
                otTaskletsProcess(OtInstance_get());
                break;
            }

            case OtStack_procQueueCmd_uart:
            {
                platformUartProcess(msg.arg);
                break;
            }

            default:
            {
                break;
            }
        }
        OtRtosApi_unlock();

        //GPIO_toggle(CONFIG_GPIO_RLED);
    }
}

