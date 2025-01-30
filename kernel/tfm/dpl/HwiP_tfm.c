/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated
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
 *  ======== HwiP_tfm.c ========
 */

#include <stddef.h>
#include <stdint.h>
#include <cmsis.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/DebugP.h>

/* driverlib header files */
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
#define IntClearPend IntPendClear
#define IntSetPend IntPendSet
#endif

typedef struct _HwiP_Obj
{
    uint32_t intNum;
    HwiP_Fxn fxn;
    uintptr_t arg;
} HwiP_Obj;

static HwiP_Obj *HwiP_dispatchTable[NUM_INTERRUPTS] = {0};

int HwiP_swiPIntNum = INT_PENDSV;

/* Custom offset to store the 1-bit NS primask along with the S primask */
#define PRIMASK_NS_OFFSET 8UL

/*
 *  ======== HwiP_enable ========
 */
void HwiP_enable(void)
{
     __enable_irq();
}

/*
 *  ======== HwiP_disable ========
 */
uintptr_t HwiP_disable(void)
{
    uintptr_t primask_ns;
    uintptr_t primask_s;

    /* Read the non-secure primask */
    primask_ns = __TZ_get_PRIMASK_NS();

    /* Read the secure primask */
    primask_s = __get_PRIMASK();

    /* Write 1 to non-secure primask to disable non-secure interrupts */
    __TZ_set_PRIMASK_NS(1UL);

    /* Disable secure interrupts */
    __disable_irq();

    /* Return key necessary to restore secure and non-secure interrupts using
     * HwiP_restore()
     */
    return (primask_s | (primask_ns << PRIMASK_NS_OFFSET));
}

/*
 *  ======== HwiP_restore ========
 */
void HwiP_restore(uintptr_t key)
{
    /* Restore secure interrupts */
    if ((key & 1UL) == 0UL)
    {
        __enable_irq();
    }

    /* Restore non-secure interrupts */
    __TZ_set_PRIMASK_NS(key >> PRIMASK_NS_OFFSET);
}

/*
 *  ======== HwiP_clearInterrupt ========
 */
void HwiP_clearInterrupt(int interruptNum)
{
    IntClearPend((uint32_t)interruptNum);
}

/*
 *  ======== HwiP_destruct ========
 */
void HwiP_destruct(HwiP_Struct *handle)
{
    HwiP_Obj *obj = (HwiP_Obj *)handle;

    IntDisable(obj->intNum);
    IntUnregister(obj->intNum);
}

/*
 *  ======== HwiP_disableInterrupt ========
 */
void HwiP_disableInterrupt(int interruptNum)
{
    IntDisable((uint32_t)interruptNum);
}

/*
 *  ======== HwiP_dispatchInterrupt ========
 */
void HwiP_dispatchInterrupt(int interruptNum)
{
    HwiP_Obj *obj = HwiP_dispatchTable[interruptNum];
    if (obj)
    {
        (obj->fxn)(obj->arg);
    }
}

/*
 *  ======== HwiP_enableInterrupt ========
 */
void HwiP_enableInterrupt(int interruptNum)
{
    IntEnable((uint32_t)interruptNum);
}

/*
 *  ======== HwiP_interruptsEnabled ========
 */
bool HwiP_interruptsEnabled(void)
{
    uint32_t priMask;

    priMask = __get_PRIMASK();

    return (priMask == 0);
}

/*
 *  ======== HwiP_construct ========
 */
HwiP_Handle HwiP_construct(HwiP_Struct *handle, int interruptNum, HwiP_Fxn hwiFxn, HwiP_Params *params)
{
    HwiP_Params defaultParams;
    HwiP_Obj *obj = (HwiP_Obj *)handle;

    if (handle != NULL)
    {
        if (params == NULL)
        {
            params = &defaultParams;
            HwiP_Params_init(&defaultParams);
        }

        if ((params->priority & 0xFF) == 0xFF)
        {
            /* SwiP_nortos.c uses INT_PRI_LEVEL7 as its scheduler */
            params->priority = INT_PRI_LEVEL6;
        }

        if (interruptNum != HwiP_swiPIntNum && params->priority == INT_PRI_LEVEL7)
        {
            DebugP_log0("HwiP_construct: can't use reserved INT_PRI_LEVEL7");

            handle = NULL;
        }
        else
        {
            HwiP_dispatchTable[interruptNum] = obj;
            obj->fxn                         = hwiFxn;
            obj->arg                         = params->arg;
            obj->intNum                      = (uint32_t)interruptNum;

            /*
             * Do not register the interrupt here. Within the Trusted Firmware-M
             * (TF-M), HW interrupts are handled by the Secure Partition Manager
             * (SPM) and converted to signals sent to the secure partition. The
             * secure partition must then call HwiP_dispatchInterrupt to call
             * the ISR.
             */
            if (params->enableInt)
            {
                IntEnable((uint32_t)interruptNum);
            }
        }
    }

    return ((HwiP_Handle)handle);
}

/*
 *  ======== HwiP_Params_init ========
 */
void HwiP_Params_init(HwiP_Params *params)
{
    if (params != NULL)
    {
        params->arg       = 0;
        params->priority  = (~0);
        params->enableInt = true;
    }
}

/*
 *  ======== HwiP_plug ========
 */
void HwiP_plug(int interruptNum, void *fxn)
{
    IntRegister((uint32_t)interruptNum, (void (*)(void))fxn);
}

/*
 *  ======== HwiP_setFunc ========
 */
void HwiP_setFunc(HwiP_Handle hwiP, HwiP_Fxn fxn, uintptr_t arg)
{
    HwiP_Obj *obj = (HwiP_Obj *)hwiP;

    uintptr_t key = HwiP_disable();

    obj->fxn = fxn;
    obj->arg = arg;

    HwiP_restore(key);
}

/*
 *  ======== HwiP_post ========
 */
void HwiP_post(int interruptNum)
{
    IntSetPend((uint32_t)interruptNum);
}

/*
 *  ======== HwiP_inISR ========
 */
bool HwiP_inISR(void)
{
    bool stat;

    if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0)
    {
        stat = false;
    }
    else
    {
        stat = true;
    }

    return (stat);
}

/*
 *  ======== HwiP_inSwi ========
 */
bool HwiP_inSwi(void)
{
    uint32_t intNum = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;

    if (intNum == HwiP_swiPIntNum)
    {
        /* Currently in a Swi */
        return (true);
    }

    return (false);
}

/*
 * ======== HwiP_setPriority ========
 */
void HwiP_setPriority(int interruptNum, uint32_t priority)
{
    /* Do nothing. It is possible to call tfm_spm_hal_set_secure_irq_priority
     * here to change the secure IRQ priority but secure IRQs have a smaller
     * range of valid priorities than can be selected via SysConfig. Per the
     * standard TF-M software, we will use the secure partition's YAML file
     * to configure secure IRQ priorities.
     */
}

/*
 *  ======== HwiP_staticObjectSize ========
 */
size_t HwiP_staticObjectSize(void)
{
    return (sizeof(HwiP_Obj));
}
