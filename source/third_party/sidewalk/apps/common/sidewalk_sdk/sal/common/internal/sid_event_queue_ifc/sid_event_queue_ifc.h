/*
 * Copyright 2019-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_EVENT_QUEUE_IFC_H
#define SID_EVENT_QUEUE_IFC_H

#include <sid_pal_timer_ifc.h>
#include <sid_error.h>

#include <lk/list.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Event type
 */
struct sid_event;

/**
 * Event handler callback type
 */
typedef sid_error_t (*sid_event_handler_t)(struct sid_event *event, void *data);

/**
 *  Event object
 */
struct sid_event {
    sid_event_handler_t handler;
    void *arg;
    list_node_t node;
};

/**
 * Deferred event object
 */
struct sid_deferred_event {
    struct sid_event pending_event;
    struct sid_timespec when;
};

/**
 * Event queue type
 */
struct sid_event_queue;

/**
 * Event queue enqueue callback type
 */
typedef void (*sid_event_queue_on_enqueue_cb_t)(const struct sid_event_queue *const event_queue, void *context);

/**
 *  Event queue object
 */
struct sid_event_queue {
    list_node_t queue;
    list_node_t pending_events;
    list_node_t freelist;
    void *freelist_base;    // Base of dynamically-allocated freelist
    sid_event_queue_on_enqueue_cb_t on_enqueue;
    void *on_enqueue_context;
    struct sid_event on_timer_event;
    sid_pal_timer_t timer;
};

/**
 * Initialize a queue object
 *
 * @param[in]   event_queue             Queue context object to initialize
 * @param[in]   on_enqueue_cb           Callback to call when event is enqueued.
 * @param[in]   on_event_cb_context     Context for the callback.
 *
 * @return  SID_ERROR_NONE in case of success
 */
sid_error_t sid_event_queue_init_with_cb(struct sid_event_queue *const event_queue,
                                         const sid_event_queue_on_enqueue_cb_t on_enqueue_cb,
                                         void *on_enqueue_cb_context);

/**
 * Initialize an queue context object
 *
 * @param[in]   event_queue            Queue context object to initialize
 *
 * @return  SID_ERROR_NONE in case of success
 */
sid_error_t sid_event_queue_init(struct sid_event_queue *const event_queue);

/**
 * Test if an event queue context object has been initialized
 *
 * @param[in]   event_queue            Queue context object
 *
 * @return  True if initialized
 */
bool sid_event_queue_is_initialized(struct sid_event_queue *const event_queue);

/**
 * Test if an event queue is empty
 *
 * @param[in]   event_queue            Queue context object
 *
 * @return  True if empty
 */
bool sid_event_queue_is_empty(struct sid_event_queue *const event_queue);

/**
 * De-initialize an event queue object.
 *
 * If the event queue used a dynamically-allocated freelist then the freelist
 * will also be freed up.
 *
 * @param[in]   event_queue           Queue context object to de-initialize
 *
 * Function fully de-initializes the @p queue object.
 * If queue has events will be removed from that and then de-initialized queue.
 */
void sid_event_queue_deinit(struct sid_event_queue *const event_queue);

/**
 * Initialize an pre-allocated sid_event freelist.
 *
 * Use of a freelist is optional. It allows a pool of sid_events (possibly including
 * extra event payload) to be pre-allocated after the event queue has been created.
 * Producers and consumers of the event queue can then use sid_event_queue_freelist_get()
 * and sid_event_queue_freelist_put() respectively to remove the need for dynamic
 * allocation of the event objects at runtime. This may be useful to constrain memory
 * requirements for the event queue (e.g. to limit the number of simultaneous events).
 *
 * The @p event_queue must have already been initialised using sid_event_queue_init()
 * and must not have previously had a freelist allocated.
 *
 * The memory used for the freelist can be provided via the @p base parameter (e.g.
 * statically allocated at compile time), or if @p base is NULL it will be dynamically
 * allocated at runtime. The memory block must be contiguous and large enough to hold
 * @p num_evts * @p evt_size bytes.
 *
 * @param[in]   event_queue Queue context object to add freelist to
 * @param[in]   num_evts    Number of events to allocate
 * @param[in]   evt_size    Size of a single event, must be at least sizeof(sid_event)
 * @param[in]   base        (optional) Pointer to preallocated memory for event pool
 *
 * @return  SID_ERROR_NONE in case of success
 */
sid_error_t sid_event_queue_freelist_init(struct sid_event_queue *const event_queue,
                                          int num_evts, size_t evt_size, void *base);

/**
 * Get a preallocated sid_event from the event queue freelist.
 *
 * @param[in]   event_queue Queue context object
 * @param[out]  event       Pointer to storage for sid_event pointer
 *
 * @return  SID_ERROR_NONE in case of success, or SID_ERROR_OOM if no events are available
 */
sid_error_t sid_event_queue_freelist_get(struct sid_event_queue *const event_queue, struct sid_event **event);

/**
 * Return a preallocated sid_event back onto the event queue freelist.
 *
 * @param[in]   event_queue Queue context object
 * @param[in]   event       Pointer to sid_event to put back onto freelist
 *
 * @return  SID_ERROR_NONE in case of success
 */
sid_error_t sid_event_queue_freelist_put(struct sid_event_queue *const event_queue, struct sid_event *event);

/**
 * Initialize an event object
 *
 * @param[in]   event               Event object to initialize
 * @param[in]   event_cb            Pointer to the callback function the called when event processed
 * @param[in]   event_arg           Argument to be provided to the @p event_cb during call
 *
 * @return  SID_ERROR_NONE in case of success
 */
sid_error_t
sid_event_queue_event_init(struct sid_event *const event, const sid_event_handler_t event_cb, void *event_arg);

/**
 * De-initialize an event object
 *
 * @param[in]   event               Event object to de-initialize
 *
 * Function fully de-initializes the @p item object.
 */
void sid_event_queue_event_deinit(struct sid_event *const event);

/**
 * Check an event object check usage event in the queue
 *
 * @param[in]   event               Event object to check
 *
 * @retval  SID_ERROR_NONE in case of @p event object is don`t used
 * @retval  SID_ERROR_BUSY in case of @p event object is used
 */
sid_error_t sid_event_queue_event_check_used(const struct sid_event *const event);

/**
 * Get an event object from the head of the queue
 *
 * @param[in]   event_queue         Queue context object
 * @param[out]  event               Pointer to an Event object taken from the queue.
 *
 * @return  SID_ERROR_NONE in case of success, @c sid_error_t code otherwise
 *
 * @note In a successful case of @p event can be NULL  when the @p queue has no events
 */
sid_error_t sid_event_queue_get_event(const struct sid_event_queue *const event_queue, struct sid_event **event);

/**
 * Get all events from the queue and process them
 *
 * @param[in] event_queue         Queue context object where to get events from
 *
 * @return  SID_ERROR_NONE in case of success, @c sid_error_t code otherwise
 */
sid_error_t sid_event_queue_process_events(const struct sid_event_queue *const event_queue);

/**
 * Add  an event object to the tail of the queue (FIFO)
 *
 * @param[in] event_queue         Queue context object
 * @param[in] event               Event object that is put to the queue.
 *
 * @return  SID_ERROR_NONE in case of success, @c sid_error_t code otherwise
 * @retval  SID_ERROR_BUSY in case of @p event already used in queue
 */
sid_error_t sid_event_queue_add_tail(const struct sid_event_queue *const event_queue,
                                     const struct sid_event *const event);

/**
 * Add  an event object to the head of the queue (LIFO)
 *
 * @param[in] event_queue         Queue context object
 * @param[in] event               Event object that is put to the queue.
 *
 * @return  SID_ERROR_NONE in case of success, @c sid_error_t code otherwise
 * @retval  SID_ERROR_BUSY in case of @p event already used in queue
 *
 * @note Please avoid using this feature. Better to use sid_event_queue_add_tail if it possible.
 */
sid_error_t sid_event_queue_add_head(const struct sid_event_queue *const event_queue,
                                     const struct sid_event *const event);

/**
 * Clear all events from the queue
 *
 * @param[in]  event_queue           Queue context object where to get events from
 *
 * @return  SID_ERROR_NONE in case of success, @c sid_error_t code otherwise
 */
sid_error_t sid_event_queue_clear_all_events(struct sid_event_queue *const event_queue);

/**
 * Initialize deferred event object
 *
 * @param[in]   event               Deferred Event object to initialize
 * @param[in]   event_cb            Pointer to the callback function the called when event processed
 * @param[in]   event_arg           Argument to be provided to the @p event_cb during call
 *
 * @return  SID_ERROR_NONE in case of success
 */
sid_error_t sid_event_queue_deferred_event_init(struct sid_deferred_event *const event,
                                                const sid_event_handler_t event_cb,
                                                void *event_arg);

/**
 * De-initialize an event queue object
 *
 * @param[in]   event_queue      Queue context object
 * @param[in]   event            Deferred Event object to de-initialize
 *
 * Function fully de-initializes the @p event object.
 */
void sid_event_queue_deferred_event_deinit(const struct sid_event_queue *const event_queue,
                                           struct sid_deferred_event *const event);

/**
 * Add deferred event object to the event queue to process it at event->when
 *
 * @param[in]   event_queue      Queue context object
 * @param[in]   event            Deferred event object that is addedt to the queue.
 *
 * @return  SID_ERROR_NONE in case of success, @c sid_error_t code otherwise
 * @retval  SID_ERROR_BUSY in case of @p event already used in queue
 */
sid_error_t sid_event_queue_add_deferred_event(const struct sid_event_queue *const event_queue,
                                               struct sid_deferred_event *const event);

/**
 * Add deferred event object to the event queue for process after delay_ms time
 *
 * @param[in]   event_queue      Queue context object
 * @param[in]   event            Deferred Event object that is put to the queue.
 * @param[in]   delay_ms         The interval from now after which the event will be process
 *
 * @return  SID_ERROR_NONE in case of success, @c sid_error_t code otherwise
 * @retval  SID_ERROR_BUSY in case of @p event already used in queue
 */
sid_error_t sid_event_queue_add_deferred_event_with_delay(const struct sid_event_queue *const event_queue,
                                                          struct sid_deferred_event *const event,
                                                          const uint32_t delay_ms);

/**
 * Remove deferred event object from the event queue
 *
 * @param[in]   event_queue      Queue context object
 * @param[in]   event            Deferred Event object that is remove from the queue.
 */
void sid_event_queue_remove_deferred_event(const struct sid_event_queue *const event_queue,
                                           struct sid_deferred_event *const event);

/**
 * Update deferred event object in the event queue
 *
 * @param[in]   event_queue      Queue context object
 * @param[in]   event            Deferred Event object that is put to the queue.
 * @param[in]   when             Time after which the event will be process.
 *
 * @return  SID_ERROR_NONE in case of success, @c halo_error_t code otherwise
 */
sid_error_t sid_event_queue_update_deferred_event(const struct sid_event_queue *const event_queue,
                                                  struct sid_deferred_event *const event,
                                                  const struct sid_timespec *when);

/**
 * Update deferred event object in the event queue
 *
 * @param[in]   event_queue      Queue context object
 * @param[in]   event            Deferred Event object that is put to the queue.
 * @param[in]   delay_ms         The interval from now after which the event will be process
 *
 * @return  SID_ERROR_NONE in case of success, @c halo_error_t code otherwise
 */
sid_error_t sid_event_queue_update_deferred_event_with_delay(const struct sid_event_queue *const event_queue,
                                                             struct sid_deferred_event *const event,
                                                             const uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* SID_EVENT_QUEUE_IFC_H */
