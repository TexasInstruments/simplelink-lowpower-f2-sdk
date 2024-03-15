/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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
 *  ======== EventP_freertos.c ========
 */

#include <ti/drivers/dpl/EventP.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/HwiP.h>

#include <FreeRTOS.h>
#include <event_groups.h>

#define FREERTOS_TICKPERIOD_US (1000000 / configTICK_RATE_HZ)

/*
 *  ======== EventP_create ========
 */
EventP_Handle EventP_create(void)
{
    EventGroupHandle_t event = NULL;
#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    event = xEventGroupCreate();
#endif
    return ((EventP_Handle)event);
}

/*
 *  ======== EventP_delete ========
 */
void EventP_delete(EventP_Handle handle)
{
#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    if (handle != NULL)
    {
        /* Delete the event that was created by use of dynamic memory allocation */
        vEventGroupDelete((EventGroupHandle_t)handle);
    }
#endif
}

/*
 *  ======== EventP_construct ========
 */
EventP_Handle EventP_construct(EventP_Struct *obj)
{
    EventGroupHandle_t event = NULL;
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    /* Construct the event using static memory */
    event = xEventGroupCreateStatic((StaticEventGroup_t *)obj);
#endif
    return ((EventP_Handle)event);
}

/*
 *  ======== EventP_destruct ========
 */
void EventP_destruct(EventP_Struct *obj)
{
    /* Destruct the event that was constructed using static memory */
    if (obj != NULL)
    {
#if (configSUPPORT_STATIC_ALLOCATION == 1)
        /* The FreeRTOS documentations states that vEventGroupDelete() can be
         * used to delete an event group previously created by a call to
         * xEventGroupCreate(). It does not mention event groups created by a
         * call to xEventGroupCreateStatic().
         * However, the implementation of vEventGroupDelete() does check if the
         * event group was statically allocated, so vEventGroupDelete() will be
         * used here.
         */
        vEventGroupDelete((EventGroupHandle_t)obj);
#endif
    }
}

/*
 *  ======== EventP_pend ========
 */
uint32_t EventP_pend(EventP_Handle event, uint32_t eventMask, bool waitForAll, uint32_t timeout)
{
    uint32_t eventBits, consumedEventBits;
    TickType_t ticksFR;
    uint32_t tickPeriod;
    uint64_t timeUS;

    /* if necessary, convert ClockP ticks to FreeRTOS ticks */
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

    eventBits = (uint32_t)xEventGroupWaitBits((EventGroupHandle_t)event,
                                              (EventBits_t)eventMask,
                                              pdTRUE,
                                              waitForAll ? pdTRUE : pdFALSE,
                                              ticksFR);

    consumedEventBits = eventBits & eventMask;

    /* Check if xEventGroupWaitBits returned because of timeout */
    if (((consumedEventBits == 0)) || ((consumedEventBits != eventMask) && (waitForAll == true)))
    {
        return 0;
    }
    else
    {
        return consumedEventBits;
    }
}

/*
 *  ======== EventP_post ========
 */
void EventP_post(EventP_Handle event, uint32_t eventMask)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (HwiP_inISR())
    {
        xEventGroupSetBitsFromISR((EventGroupHandle_t)event, (EventBits_t)eventMask, &xHigherPriorityTaskWoken);

        /* No need to yield here explicitly since HwiP_dispatch yields
         * unconditionally just before exiting the ISR.
         */
    }
    else
    {
        xEventGroupSetBits((EventGroupHandle_t)event, (EventBits_t)eventMask);
    }
}

/*
 *  ======== EventP_clear ========
 */
void EventP_clear(EventP_Handle event, uint32_t eventMask)
{
    if (HwiP_inISR())
    {
        xEventGroupClearBitsFromISR((EventGroupHandle_t)event, (EventBits_t)eventMask);
    }
    else
    {
        xEventGroupClearBits((EventGroupHandle_t)event, (EventBits_t)eventMask);
    }
}

/*
 *  ======== EventP_get ========
 */
uint32_t EventP_get(EventP_Handle event)
{
    if (HwiP_inISR())
    {
        return ((uint32_t)xEventGroupGetBitsFromISR((EventGroupHandle_t)event));
    }
    else
    {
        return ((uint32_t)xEventGroupGetBits((EventGroupHandle_t)event));
    }
}