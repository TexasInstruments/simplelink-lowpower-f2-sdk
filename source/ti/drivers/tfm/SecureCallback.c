/*
 * Copyright (c) 2022 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== SecureCallback.c ========
 */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/utils/List.h>

#include "SecureCallback.h"

/* Driverlib includes*/
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/interrupt.h)

static HwiP_Struct hwipStruct;
static HwiP_Handle hwipHandle;

/* This list is only defined on the user application, and will be optimized out
 * of the secure image
 */
static List_List allCallbackObjects;

/* This object will be provided by sysconfig both the secure and application
 * images, and must have the same interrupt number configured in both cases
 */
extern const SecureCallback_Config SecureCallback_config;

/*
 *  ======== SecureCallback_hwi ========
 */
static void SecureCallback_hwi(uintptr_t arg)
{
    SecureCallback_Object *element = (SecureCallback_Object *)List_head(&allCallbackObjects);
    while (element != NULL)
    {
        if (element->pending == true && element->fxn != NULL)
        {
            element->pending = false;
            element->fxn(element->arg);
        }
        element = (SecureCallback_Object *)List_next((List_Elem *)element);
    }
}

/*
 *  ======== SecureCallback_init ========
 */
void SecureCallback_init()
{
    HwiP_Params hwiParams;
    hwiParams.priority  = SecureCallback_config.intPriority;
    hwiParams.enableInt = true;

    List_clearList(&allCallbackObjects);
    hwipHandle = HwiP_construct(&hwipStruct, SecureCallback_config.intNumber, SecureCallback_hwi, &hwiParams);

    /* No recovery mechanism for failure to create HwiP */
    if (hwipHandle == NULL)
    {
        while (1) {}
    }
}

/*
 *  ======== SecureCallback_construct ========
 */
SecureCallback_Handle SecureCallback_construct(SecureCallback_Object *object, SecureCallback_FuncPtr fxn, uintptr_t arg)
{
    List_put(&allCallbackObjects, (List_Elem *)object);
    object->pending = false;
    object->arg     = arg;
    object->fxn     = fxn;
    return object;
}

/*
 *  ======== SecureCallback_destruct ========
 */
void SecureCallback_destruct(SecureCallback_Object *object)
{
    List_remove(&allCallbackObjects, (List_Elem *)object);
}

/*
 *  ======== SecureCallback_post ========
 */
void SecureCallback_post(SecureCallback_Handle handle)
{
    handle->pending = true;
    IntPendSet(SecureCallback_config.intNumber);
}
