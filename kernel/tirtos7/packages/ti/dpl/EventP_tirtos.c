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
 *  ======== EventP_tirtos.c ========
 */

#include <ti/drivers/dpl/EventP.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <ti/sysbios/knl/Event.h>

/*
 *  ======== EventP_create ========
 */
EventP_Handle EventP_create(void)
{
    Event_Handle event = NULL;

    event = Event_create(NULL, NULL);

    return ((EventP_Handle)event);
}

/*
 *  ======== EventP_delete ========
 */
void EventP_delete(EventP_Handle handle)
{
    if (handle != NULL)
    {
        /* Delete the event that was created by use of dynamic memory allocation */
        Event_delete((Event_Handle *)&handle);
    }
}

/*
 *  ======== EventP_construct ========
 */
EventP_Handle EventP_construct(EventP_Struct *obj)
{
    /* Construct the event using static memory */
    return ((EventP_Handle)Event_construct((Event_Struct *)obj, NULL));
}

/*
 *  ======== EventP_destruct ========
 */
void EventP_destruct(EventP_Struct *obj)
{
    /* Destruct the event that was constructed using static memory */
    if (obj != NULL)
    {
        Event_destruct((Event_Struct *)obj);
    }
}

/*
 *  ======== EventP_pend ========
 */
uint32_t EventP_pend(EventP_Handle event, uint32_t eventMask, bool waitForAll, uint32_t timeout)
{
    unsigned int andMask = waitForAll ? eventMask : 0;
    unsigned int orMask  = waitForAll ? 0 : eventMask;
    unsigned int eventTimeout;

    if (timeout == EventP_WAIT_FOREVER)
    {
        eventTimeout = BIOS_WAIT_FOREVER;
    }
    else if (timeout == EventP_NO_WAIT)
    {
        eventTimeout = BIOS_NO_WAIT;
    }
    else
    {
        eventTimeout = timeout;
    }

    return Event_pend((Event_Handle)event, andMask, orMask, eventTimeout);
}

/*
 *  ======== EventP_post ========
 */
void EventP_post(EventP_Handle event, uint32_t eventMask)
{
    Event_post((Event_Handle)event, eventMask);
}

/*
 *  ======== EventP_clear ========
 */
void EventP_clear(EventP_Handle event, uint32_t eventMask)
{
    /* Pend on event with no wait to clear currently active events */
    Event_pend((Event_Handle)event, 0, eventMask, BIOS_NO_WAIT);
}

/*
 *  ======== EventP_get ========
 */
uint32_t EventP_get(EventP_Handle event)
{
    return Event_getPostedEvents((Event_Handle)event);
}