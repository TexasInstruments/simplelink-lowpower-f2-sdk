/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ======== MessageQueueP_tirtos.c ========
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/knl/Mailbox.h>

#include <ti/drivers/dpl/MessageQueueP.h>
#include <ti/drivers/dpl/HwiP.h>

static Mailbox_Params mboxParams;

/*
 *  ======== MessageQueueP_create ========
 */
MessageQueueP_Handle MessageQueueP_create(size_t msgSize, size_t msgCount)
{
    Mailbox_Handle handle = NULL;

    /* Create the message queue using dynamic memory allocation */
    handle = Mailbox_create((SizeT)msgSize, (UInt)msgCount, NULL, NULL);

    return (MessageQueueP_Handle)handle;
}

/*
 *  ======== MessageQueueP_delete ========
 */
void MessageQueueP_delete(MessageQueueP_Handle handle)
{
    if (handle != NULL)
    {
        /* Delete the message queue that was created by use of dynamic memory allocation */
        Mailbox_delete((Mailbox_Handle *)&handle);
    }
}

/*
 *  ======== MessageQueueP_construct ========
 */
MessageQueueP_Handle MessageQueueP_construct(MessageQueueP_Struct *msgStruct,
                                             size_t msgSize,
                                             size_t msgCount,
                                             void *msgBuf)
{
    Mailbox_Handle handle = NULL;

    /* Update mailbox params with pointer to user provided message queue buffer and
     * with required size of the buffer.
     * It is the users responsibility that the size of the provided buffer is equal
     * or larger than the required size.
     */
    Mailbox_Params_init(&mboxParams);
    mboxParams.buf     = msgBuf;
    mboxParams.bufSize = (msgSize + sizeof(Mailbox_MbxElem)) * msgCount;

    /* Construct the message queue using static memory */
    handle = Mailbox_construct((Mailbox_Struct *)msgStruct, (SizeT)msgSize, (UInt)msgCount, &mboxParams, NULL);

    return (MessageQueueP_Handle)handle;
}

/*
 *  ======== MessageQueueP_destruct ========
 */
void MessageQueueP_destruct(MessageQueueP_Handle handle)
{
    Mailbox_Struct *msgStruct = NULL;

    if (handle != NULL)
    {
        msgStruct = Mailbox_struct((Mailbox_Handle)handle);
        Mailbox_destruct(msgStruct);
    }
}

/*
 *  ======== MessageQueueP_pend ========
 */
MessageQueueP_Status MessageQueueP_pend(MessageQueueP_Handle handle, void *message, uint32_t timeout)
{
    Bool status = FALSE;

    if (handle != NULL)
    {
        status = Mailbox_pend((Mailbox_Handle)handle, message, timeout);
    }

    return status == TRUE ? MessageQueueP_OK : MessageQueueP_TIMEOUT;
}

/*
 *  ======== MessageQueueP_peek ========
 */
MessageQueueP_Status MessageQueueP_peek(MessageQueueP_Handle handle, void *message, uint32_t timeout)
{
    Bool status = FALSE;

    if (handle != NULL)
    {
        status = Mailbox_peek((Mailbox_Handle)handle, message, timeout);
    }

    return status == TRUE ? MessageQueueP_OK : MessageQueueP_TIMEOUT;
}

/*
 *  ======== MessageQueueP_post ========
 */
MessageQueueP_Status MessageQueueP_post(MessageQueueP_Handle handle, const void *message, uint32_t timeout)
{
    Bool status = FALSE;

    if (handle != NULL)
    {
        status = Mailbox_post((Mailbox_Handle)handle, (void *)message, timeout);
    }

    return status == TRUE ? MessageQueueP_OK : MessageQueueP_TIMEOUT;
}

/*
 *  ======== MessageQueueP_postFront ========
 */
MessageQueueP_Status MessageQueueP_postFront(MessageQueueP_Handle handle, const void *message, uint32_t timeout)
{
    Bool status = FALSE;

    if (handle != NULL)
    {
        status = Mailbox_putHead((Mailbox_Handle)handle, (void *)message, timeout);
    }

    return status == TRUE ? MessageQueueP_OK : MessageQueueP_TIMEOUT;
}

/*
 *  ======== MessageQueueP_getPendingCount ========
 */
size_t MessageQueueP_getPendingCount(MessageQueueP_Handle handle)
{
    int msgCount = 0;

    if (handle != NULL)
    {
        msgCount = Mailbox_getNumPendingMsgs((Mailbox_Handle)handle);
    }

    return (size_t)msgCount;
}

/*
 *  ======== MessageQueueP_getFreeCount ========
 */
size_t MessageQueueP_getFreeCount(MessageQueueP_Handle handle)
{
    int msgCount = 0;

    if (handle != NULL)
    {
        msgCount = Mailbox_getNumFreeMsgs((Mailbox_Handle)handle);
    }

    return (size_t)msgCount;
}

/*
 *  ======== MessageQueueP_getMessageQueueObjectSize ========
 */
uint32_t MessageQueueP_getMessageQueueObjectSize(void)
{
    return sizeof(Mailbox_Struct);
}