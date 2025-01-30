/*
 * Copyright (c) 2023-2024, Texas Instruments Incorporated
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
 *  ======== MessageQueueP_freertos.c ========
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>

#include <ti/drivers/dpl/MessageQueueP.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/HwiP.h>

static TickType_t MessageQueueP_getTicks(uint32_t timeout);

/* Number of micro seconds between each FreeRTOS OS tick */
#define FREERTOS_TICKPERIOD_US (1000000 / configTICK_RATE_HZ)

/*
 *  ======== MessageQueueP_create ========
 */
MessageQueueP_Handle MessageQueueP_create(size_t msgSize, size_t msgCount)
{
    QueueHandle_t handle = NULL;

    /* Create the message queue using dynamic memory allocation */
    handle = xQueueCreate((UBaseType_t)msgCount, (UBaseType_t)msgSize);

    return (MessageQueueP_Handle)handle;
}

/*
 *  ======== MessageQueueP_delete ========
 */
void MessageQueueP_delete(MessageQueueP_Handle handle)
{
    if (handle != NULL)
    {
        /* Delete the message queue that was created by use of dynamic memory allocation.
         * The message queue will only be deleted if it was dynamically allocated. If it
         * was statically allocated, the function will not attempt to free it.
         */
        vQueueDelete((QueueHandle_t)handle);
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
    QueueHandle_t handle = NULL;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    /* Construct the message queue using static memory */
    handle = xQueueCreateStatic((UBaseType_t)msgCount,
                                (UBaseType_t)msgSize,
                                (uint8_t *)msgBuf,
                                (StaticQueue_t *)msgStruct);
#endif

    return (MessageQueueP_Handle)handle;
}

/*
 *  ======== MessageQueueP_destruct ========
 */
void MessageQueueP_destruct(MessageQueueP_Handle handle)
{
    MessageQueueP_delete(handle);
}

/*
 *  ======== MessageQueueP_pend ========
 */
MessageQueueP_Status MessageQueueP_pend(MessageQueueP_Handle handle, void *message, uint32_t timeout)
{
    BaseType_t status = pdFALSE;
    TickType_t ticks;

    if (handle != NULL)
    {
        /* In ISR? */
        if (HwiP_inISR())
        {
            status = xQueueReceiveFromISR((QueueHandle_t)handle, message, NULL);
        }
        else
        {
            ticks  = MessageQueueP_getTicks(timeout);
            status = xQueueReceive((QueueHandle_t)handle, message, ticks);
        }
    }

    return status == pdPASS ? MessageQueueP_OK : MessageQueueP_TIMEOUT;
}

/*
 *  ======== MessageQueueP_peek ========
 */
MessageQueueP_Status MessageQueueP_peek(MessageQueueP_Handle handle, void *message, uint32_t timeout)
{
    BaseType_t status = pdFALSE;
    TickType_t ticks;

    if (handle != NULL)
    {
        /* In ISR? */
        if (HwiP_inISR())
        {
            status = xQueuePeekFromISR((QueueHandle_t)handle, message);
        }
        else
        {
            ticks  = MessageQueueP_getTicks(timeout);
            status = xQueuePeek((QueueHandle_t)handle, message, ticks);
        }
    }

    configASSERT(status == pdPASS);

    return status == pdPASS ? MessageQueueP_OK : MessageQueueP_TIMEOUT;
}

/*
 *  ======== MessageQueueP_post ========
 */
MessageQueueP_Status MessageQueueP_post(MessageQueueP_Handle handle, const void *message, uint32_t timeout)
{
    BaseType_t status = pdFALSE;
    TickType_t ticks;

    if (handle != NULL)
    {
        /* In ISR? */
        if (HwiP_inISR())
        {
            status = xQueueSendFromISR((QueueHandle_t)handle, message, NULL);
        }
        else
        {
            ticks  = MessageQueueP_getTicks(timeout);
            status = xQueueSend((QueueHandle_t)handle, message, ticks);
        }
    }

    configASSERT(status == pdPASS);

    return status == pdPASS ? MessageQueueP_OK : MessageQueueP_TIMEOUT;
}

/*
 *  ======== MessageQueueP_postFront ========
 */
MessageQueueP_Status MessageQueueP_postFront(MessageQueueP_Handle handle, const void *message, uint32_t timeout)
{
    BaseType_t status = pdFALSE;
    TickType_t ticks;

    if (handle != NULL)
    {
        /* In ISR? */
        if (HwiP_inISR())
        {
            status = xQueueSendToFrontFromISR((QueueHandle_t)handle, message, NULL);
        }
        else
        {
            ticks  = MessageQueueP_getTicks(timeout);
            status = xQueueSendToFront((QueueHandle_t)handle, message, ticks);
        }
    }

    configASSERT(status == pdPASS);

    return status == pdPASS ? MessageQueueP_OK : MessageQueueP_TIMEOUT;
}

/*
 *  ======== MessageQueueP_getPendingCount ========
 */
size_t MessageQueueP_getPendingCount(MessageQueueP_Handle handle)
{
    BaseType_t msgCount = 0;

    if (handle != NULL)
    {
        /* In ISR? */
        if (HwiP_inISR())
        {
            msgCount = uxQueueMessagesWaitingFromISR((QueueHandle_t)handle);
        }
        else
        {
            msgCount = uxQueueMessagesWaiting((QueueHandle_t)handle);
        }
    }

    return (size_t)msgCount;
}

/*
 *  ======== MessageQueueP_getFreeCount ========
 */
size_t MessageQueueP_getFreeCount(MessageQueueP_Handle handle)
{
    BaseType_t msgCount = 0;

    if (handle != NULL)
    {
        msgCount = uxQueueSpacesAvailable((QueueHandle_t)handle);
    }

    return (size_t)msgCount;
}

/*
 *  ======== MessageQueueP_getMessageQueueObjectSize ========
 */
uint32_t MessageQueueP_getMessageQueueObjectSize(void)
{
    return sizeof(StaticQueue_t);
}

/*
 *  ======== MessageQueueP_getTicks ========
 */
static TickType_t MessageQueueP_getTicks(uint32_t timeout)
{
    TickType_t ticksFR;
    uint32_t tickPeriod;
    uint64_t timeUS;

#if (INCLUDE_vTaskSuspend == 1)
    if (timeout == MessageQueueP_WAIT_FOREVER)
    {
        ticksFR = portMAX_DELAY;
    }
    else
#endif
    {
        /* If necessary, convert ClockP ticks to FreeRTOS ticks */
        tickPeriod = ClockP_getSystemTickPeriod();
        if (tickPeriod != FREERTOS_TICKPERIOD_US)
        {
            timeUS  = timeout * (uint64_t)tickPeriod;
            ticksFR = (TickType_t)(timeUS / FREERTOS_TICKPERIOD_US);
        }
        else
        {
            ticksFR = timeout;
        }
    }

    return ticksFR;
}
