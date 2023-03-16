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
 *  ======== SemaphoreP_tfm.c ========
 */

#include <stdio.h>
#include <stdlib.h>

#include "SemaphoreP_tfm.h"
#include <ti/drivers/dpl/HwiP.h>

/*
 *  ======== SemaphoreP_constructBinary ========
 */
SemaphoreP_Handle SemaphoreP_constructBinary(SemaphoreP_Struct *handle,
                                             unsigned int count)
{
    if (count == 0)
    {
        handle->isAvailable = false;
    }
    else
    {
        handle->isAvailable = true;
    }

    return ((SemaphoreP_Handle)handle);
}

/*
 *  ======== SemaphoreP_pend ========
 */
SemaphoreP_Status SemaphoreP_pend(SemaphoreP_Handle handle, uint32_t timeout)
{
    ((void)timeout); /* Unused variable */
    SemaphoreP_Struct *pSemaphore = (SemaphoreP_Struct *)handle;
    SemaphoreP_Status status = SemaphoreP_TIMEOUT;

    if (pSemaphore->isAvailable)
    {
        pSemaphore->isAvailable = false;
        status = SemaphoreP_OK;
    }

    return (status);
}

/*
 *  ======== SemaphoreP_post ========
 */
void SemaphoreP_post(SemaphoreP_Handle handle)
{
    SemaphoreP_Struct *pSemaphore = (SemaphoreP_Struct *)handle;

    pSemaphore->isAvailable = true;
}

/*
 *  ======== SemaphoreP_destruct ========
 */
void SemaphoreP_destruct(SemaphoreP_Handle handle)
{
    /* Do nothing */
}
