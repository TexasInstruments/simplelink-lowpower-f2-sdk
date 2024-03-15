/*
 * Copyright (c) 2021-2023, Texas Instruments Incorporated - http://www.ti.com
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
 *
 */
/** ============================================================================
 *  @file       EventP.h
 *
 *  @brief      Event Group support
 *
 * Events are a collection of bits with an application-defined meaning,
 * typically used for messaging or synchronisation. A task may check the state
 * of a set of bits or pend on an EventP object to react to changes when they
 * are posted from another context.
 *
 * Only one Task may pend on a single EventP object at any given time.
 *
 * Events are synchronous in nature, meaning that a receiving task will block or
 * pend while waiting for the events to occur. When the desired events are
 * received, the pending task continues its execution, as it would after a call
 * to Semaphore_pend(), for example.
 *
 * EventP_pend is used to wait for events. The eventMask determine which
 * event(s) must occur before returning from EventP_pend. The timeout parameter
 * allows the task to wait until a timeout, wait indefinitely, or not wait at
 * all. If waitForAll is true, the pend call will not return until all of the
 * bits in eventMask are set. If it is false, any of the bits in eventMask will
 * be returned. A return value of zero indicates that a timeout has occurred. A
 * non-zero return value is the set of events that were active at the time the
 * task was unblocked. Event bits that caused a return (either the whole
 * eventMask or any individual bit, depending on waitForAll) will be cleared
 * when EventP_pend returns.
 *
 *  ============================================================================
 */

#ifndef ti_dpl_EventP__include
#define ti_dpl_EventP__include

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief    Number of bytes greater than or equal to the size of any RTOS Event object.
 *
 *  BIOS 6.x: 20
 *  BIOS 7.x: 20
 *  FreeRTOS: 52
 */
#define EventP_STRUCT_SIZE (52)

/*!
 *  @brief    EventP structure.
 *
 *  Opaque structure that should be large enough to hold any of the
 *  RTOS specific EventP objects.
 */
typedef union EventP_Struct
{
    uint32_t dummy; /*!< Align object */
    uint8_t data[EventP_STRUCT_SIZE];
} EventP_Struct;

/*!
 *  @brief    Wait forever define
 */
#define EventP_WAIT_FOREVER ~(0)

/*!
 *  @brief    No wait define
 */
#define EventP_NO_WAIT (0)

/*!
 *  @brief Opaque client reference to an instance of a EventP
 *
 *  A EventP_Handle returned from create or construct represents that instance.
 */
typedef EventP_Struct *EventP_Handle;

/*!
 * @brief Create an EventP, allocating memory on the heap.
 *
 * EventP_create creates a new event object. EventP_create returns the
 * handle of the new task object or NULL if the event could not be created.
 *
 * When created, no bits of an event are set. For FreeRTOS,
 * configSUPPORT_DYNAMIC_ALLOCATION also has to be set to 1 in FreeRTOSConfig.h.
 * See 'Configuration with FreeRTOS' in the Core SDK User's Guide for how to do
 * this.
 *
 * This API cannot be called from interrupt contexts.
 *
 * @retval EventP handle (NULL on failure)
 */
extern EventP_Handle EventP_create(void);

/*!
 *  @brief  Function to delete an EventP.
 *
 *  @param  handle  A EventP_Handle returned from EventP_create
 */
extern void EventP_delete(EventP_Handle handle);

/*!
 * @brief Construct an EventP, using statically allocated memory.
 *
 * EventP_construct creates a new event object. EventP_construct returns the
 * handle of the new task object or NULL if the event could not be created.
 *
 * When created, no bits of an event are set. For FreeRTOS,
 * configSUPPORT_STATIC_ALLOCATION also has to be set to 1 in FreeRTOSConfig.h.
 * See 'Configuration with FreeRTOS' in the Core SDK User's Guide for how to do
 * this.
 *
 * This API cannot be called from interrupt contexts.
 *
 * @retval EventP handle (NULL on failure)
 */
extern EventP_Handle EventP_construct(EventP_Struct *obj);

/*!
 *  @brief  Function to destruct an EventP
 *
 *  @param  obj     Pointer to a EventP_Struct object that was passed to
 *                  EventP_construct().
 *
 *  @return
 */
extern void EventP_destruct(EventP_Struct *obj);

/*!
 * @brief Wait for the events listed in eventMask.
 *
 * EventP_pend is used to wait for events. The eventMask determine which event(s)
 * must occur before returning from EventP_pend. The timeout parameter allows the
 * task to wait until a timeout, wait indefinitely, or not wait at all. If
 * waitForAll is true, the pend call will not return until all of the bits in
 * eventMask are set. If it is false, any of the bits in eventMask will be
 * returned. A return value of zero indicates that a timeout has occurred. A
 * non-zero return value is the set of events in the eventMask that were active
 * at the time the task was unblocked.
 *
 * Event bits that caused a return (either the whole eventMask or any individual
 * bit, depending on waitForAll) will be cleared when EventP_pend returns.
 *
 * A timeout value of EventP_WAIT_FOREVER causes the task to wait indefinitely
 * for matching events to be posted. A timeout value of EventP_NO_WAIT causes
 * EventP_pend to return immediately.
 *
 * This API cannot be called from interrupt contexts.
 *
 * @param event         Event handle
 * @param eventMask     Match against the events in this bitmask. For FreeRTOS,
 *                      only the 24 least significant bits in the event mask may
 *                      be set, meaning the maximum allowed value for FreeRTOS
 *                      is 0x00FFFFFF.
 * @param waitForAll    If true, only return when all matching bits are set
 * @param timeout       Return after this many ClockP ticks, even if there is no match
 *
 * @retval A bitmask containing all consumed events, or zero on timeout.
 */
extern uint32_t EventP_pend(EventP_Handle event, uint32_t eventMask, bool waitForAll, uint32_t timeout);

/*!
 * @brief Post events to an event object.
 *
 * EventP_post() is used to signal events. If a task is waiting for the event
 * and the event conditions are met, EventP_post() unblocks the task. If no
 * tasks are waiting, EventP_post() simply registers the event with the event
 * object and returns.
 *
 * @param event         Event handle
 * @param eventMask     Mask of eventIds to post (this must be non-zero). For
 *                      FreeRTOS, only the 24 least significant bits in the
 *                      event mask may be set, meaning the maximum allowed value
 *                      for FreeRTOS is 0x00FFFFFF.
 */
extern void EventP_post(EventP_Handle event, uint32_t eventMask);

/*!
 * @brief Clear events from an event object.
 *
 * Clears the bits in eventMask from the EventP.
 *
 * @param event         Event handle
 * @param eventMask     Mask of eventIds to clear (this must be non-zero). For
 *                      FreeRTOS, only the 24 least significant bits in the
 *                      event mask may be set, meaning the maximum allowed value
 *                      for FreeRTOS is 0x00FFFFFF.
 */
extern void EventP_clear(EventP_Handle event, uint32_t eventMask);

/*!
 * @brief Get the current events from an event object.
 *
 * Returns the currently active events in an EventP without clearing them.
 *
 * @param event Event handle
 *
 * @retval Currently active events
 */
extern uint32_t EventP_get(EventP_Handle event);

#ifdef __cplusplus
}
#endif

#endif /* ti_dpl_EventP__include */
