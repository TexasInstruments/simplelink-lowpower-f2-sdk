/*
 *
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef KERNEL_H_INCLUDE
#define KERNEL_H_INCLUDE

#include <errno.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>

#include <toolchain.h>
#include <sys_clock.h>
#include <zephyr/types.h>
#include <sys/atomic.h>
#include <sys/util.h>
#include <sys/slist.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <arch/cpu.h>

#include <ti/sysbios/runtime/IHeap.h>
#include <ti/sysbios/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/gates/GateMutex.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>

#include <ti/kernel/SCOM.h>

#include <drivers/flash.h>

#define K_FOREVER Z_TIMEOUT_TICKS(BIOS_WAIT_FOREVER)

#define K_NO_WAIT Z_TIMEOUT_NO_WAIT

#define K_THREAD_STACK_DEFINE K_KERNEL_STACK_DEFINE

#define K_THREAD_STACK_SIZEOF K_KERNEL_STACK_SIZEOF

#define MSECS_TO_USECS 1000
#define k_panic()               \
    {                           \
        volatile uint8_t x = 0; \
        while (1)               \
        {                       \
            x++;                \
        }                       \
    }
/** @brief System-wide macro to denote "forever" in milliseconds
 *
 *  Usage of this macro is limited to APIs that want to expose a timeout value
 *  that can optionally be unlimited, or "forever".
 *  This macro can not be fed into kernel functions or macros directly. Use
 *  @ref SYS_TIMEOUT_MS instead.
 */
#define SYS_FOREVER_MS (-1)

/** @brief System-wide macro to convert milliseconds to kernel timeouts
 */
#define SYS_TIMEOUT_MS(ms) ((ms) == SYS_FOREVER_MS ? K_FOREVER : K_MSEC(ms))

static inline uint32_t k_uptime_get_32(void)
{
    return (((uint64_t)Clock_getTicks() * Clock_tickPeriod) / MSECS_TO_USECS);
}

static inline int64_t k_uptime_get(void)
{
    return k_uptime_get_32();
}

#define z_tick_get k_uptime_get

/**
 * @brief Get elapsed time.
 *
 * This routine computes the elapsed time between the current system uptime
 * and an earlier reference time, in milliseconds.
 *
 * @param reftime Pointer to a reference time, which is updated to the current
 *                uptime upon return.
 *
 * @return Elapsed time.
 */
static inline int64_t k_uptime_delta(int64_t *reftime)
{
    int64_t uptime, delta;
    const int64_t MAX_VAL = ((uint64_t)UINT32_MAX * Clock_tickPeriod) / MSECS_TO_USECS;

    uptime = k_uptime_get();
    delta = uptime < *reftime ? (MAX_VAL) - *reftime + uptime : uptime - *reftime;
    *reftime = uptime;

    return delta;
}

/** @brief Convert ticks to milliseconds
 *
 * Converts time values in ticks to milliseconds.
 * Computes result in 32 bit precision.
 * Rounds up to the next highest output unit.
 *
 * @return The converted time value
 */
static inline uint32_t k_ticks_to_ms_ceil32(uint32_t t)
{
    return t / 4000; // 4000 - RAT_TICKS_IN_1MS
}

/** @brief Convert milliseconds to ticks
 *
 * Converts time values in milliseconds to ticks.
 * Computes result in 32 bit precision.
 * Rounds up to the next highest output unit.
 *
 * @return The converted time value
 */
static inline uint32_t k_ms_to_ticks_ceil32(uint32_t t)
{
    return t * 4000; // 4000 - RAT_TICKS_IN_1MS
}

/* Returns the uptime expiration (relative to an unlocked "now"!) of a
 * timeout object.  When used correctly, this should be called once,
 * synchronously with the user passing a new timeout value.  It should
 * not be used iteratively to adjust a timeout.
 */
static inline uint64_t z_timeout_end_calc(k_timeout_t timeout)
{
    k_ticks_t dt;

    if (K_TIMEOUT_EQ(timeout, K_FOREVER))
    {
        return UINT64_MAX;
    }
    else if (K_TIMEOUT_EQ(timeout, K_NO_WAIT))
    {
        return z_tick_get();
    }

    dt = k_ms_to_ticks_ceil32(timeout.ticks);

    return z_tick_get() + MAX(1, dt);
}

typedef struct
{
    uint16_t _slab_block_size;
    uint16_t _slab_num_blocks;
    uint16_t _num_free_blocks;
} k_mem_slab;

#define K_MEM_SLAB_DEFINE(name, slab_block_size, slab_num_blocks, slab_align) \
    static k_mem_slab name =                                                  \
        {                                                                     \
            ._slab_block_size = slab_block_size,                              \
            ._slab_num_blocks = slab_num_blocks,                              \
            ._num_free_blocks = slab_num_blocks,                              \
    }

static inline void k_mem_slab_free(k_mem_slab *slab, void **mem)
{
    free(*mem);
    slab->_num_free_blocks++;
}

static inline int k_mem_slab_alloc(k_mem_slab *slab, void **mem, k_timeout_t timeout)
{
    volatile void *_mem;
    // Make sure there are blocks to alloc
    if (slab->_num_free_blocks == 0)
    {
        return -ENOMEM;
    }

    // Make sure slab is not null
    if (!slab)
    {
        return -EINVAL;
    }

    // Allocate memory block
    _mem = malloc(slab->_slab_block_size);

    *mem = (void *)_mem;

    // Decrement the block count
    slab->_num_free_blocks--;

    return 0;
}

static inline uint32_t k_mem_slab_num_free_get(k_mem_slab *slab)
{
    return slab->_num_free_blocks;
}

struct k_thread
{
    Task_Struct taskStruct;
    int prio;
    int coop;
    void *arg1;
    void *arg2;
    void *arg3;
};

/* Thread stack space */
#define k_thread_stack_t char

typedef void (*k_thread_entry_t)(void *, void *, void *);

void k_thread_create(struct k_thread *new_thread, k_thread_stack_t *stack,
                     size_t stack_size, k_thread_entry_t entry,
                     void *p1, void *p2, void *p3, int prio, uint32_t options,
                     k_timeout_t delay);

int _thread_setPri(int priority);

/**
 * @brief Start an inactive thread
 *
 * If a thread was created with K_FOREVER in the delay parameter, it will
 * not be added to the scheduling queue until this function is called
 * on it.
 *
 * @param thread thread to start
 */
void k_thread_start(struct k_thread *thread);

static inline void k_thread_name_set(struct k_thread *t, const char *name)
{
    (void)t;
    (void)name;
}

typedef void (*k_thread_user_cb_t)(const struct k_thread *thread,
                                   void *user_data);

static inline void k_thread_foreach(k_thread_user_cb_t fxn, void *arg)
{
    (void)fxn;
    (void)arg;
}

static inline void k_yield(void)
{
    Task_yield();
}

/* @breif: Converts Zephyr's sleep method to Osal's sleep method
 * @param time: sleep duration (in ms)
 */
static inline void k_sleep(k_timeout_t time)
{
    // Task_sleep takes in the amount of ticks to sleep
    Task_sleep(time.ticks * MSECS_TO_USECS / Clock_tickPeriod);
}

struct _timeout
{
    // sys_dnode_t node;
    int32_t dticks;
    //_timeout_func_t fn;
};

struct k_work;
#if 0
struct k_work_poll;

/* private, used by k_poll and k_work_poll */
typedef int (*_poller_cb_t)(struct k_poll_event *event, uint32_t state);
struct _poller {
    volatile bool is_polling;
    struct k_thread *thread;
    _poller_cb_t cb;
};
#endif

/* Work-around for not supporting SYS_INIT yet */
int k_sys_work_q_init(void *dev);

/**
 * @addtogroup thread_apis
 * @{
 */

/**
 * @typedef k_work_handler_t
 * @brief Work item handler function type.
 *
 * A work item's handler function is executed by a workqueue's thread
 * when the work item is processed by the workqueue.
 *
 * @param work Address of the work item.
 *
 * @return N/A
 * @req K-WORK-001
 */
typedef void (*k_work_handler_t)(struct k_work *work);

/**
 * @cond INTERNAL_HIDDEN
 */

struct k_work_q
{
    SCOM_Struct queue;
    Task_Struct thread; // todo should we use Task_Struct or k_thread here?
};

enum
{
    K_WORK_STATE_PENDING, /* Work item pending state */
};

#define irq_lock Hwi_disable
#define irq_unlock Hwi_restore

#undef CONFIG_NUM_COOP_PRIORITIES
#undef CONFIG_NUM_PREEMPT_PRIORITIES
#define CONFIG_NUM_COOP_PRIORITIES (Task_numPriorities / 2)
#define CONFIG_NUM_PREEMPT_PRIORITIES (Task_numPriorities / 2 - 1)

#define K_KERNEL_STACK_DEFINE(v, s) \
    __aligned(8) char v[s];

#define K_KERNEL_STACK_SIZEOF(sym) sizeof(sym)

#define K_PRIO_COOP(x) (-(CONFIG_NUM_COOP_PRIORITIES - (x)))
#define K_PRIO_PREEMPT(x) (x)

struct k_work
{
    void *_reserved; /* Used by k_queue implementation. */
    k_work_handler_t handler;
    atomic_t flags[1];
};

struct k_delayed_work
{
    struct k_work work;
    Clock_Struct timeout;
    struct k_work_q *work_q;
};

#if 0
struct k_work_poll {
    struct k_work work;
    struct _poller poller;
    struct k_poll_event *events;
    int num_events;
    k_work_handler_t real_handler;
    struct _timeout timeout;
    int poll_result;
};
#endif

extern struct k_work_q k_sys_work_q;

/**
 * INTERNAL_HIDDEN @endcond
 */

#define Z_WORK_INITIALIZER(work_handler) \
    {                                    \
        ._reserved = NULL,               \
        .handler = work_handler,         \
        .flags = { 0 }                   \
    }

#define K_WORK_INITIALIZER __DEPRECATED_MACRO Z_WORK_INITIALIZER

/**
 * @brief Initialize a statically-defined work item.
 *
 * This macro can be used to initialize a statically-defined workqueue work
 * item, prior to its first use. For example,
 *
 * @code static K_WORK_DEFINE(<work>, <work_handler>); @endcode
 *
 * @param work Symbol name for work item object
 * @param work_handler Function to invoke each time work item is processed.
 * @req K-WORK-002
 */
#define K_WORK_DEFINE(work, work_handler) \
    struct k_work work = Z_WORK_INITIALIZER(work_handler)

/**
 * @brief Initialize a work item.
 *
 * This routine initializes a workqueue work item, prior to its first use.
 *
 * @param work Address of work item.
 * @param handler Function to invoke each time work item is processed.
 *
 * @return N/A
 * @req K-WORK-001
 */
static inline void k_work_init(struct k_work *work, k_work_handler_t handler)
{
    work->handler = handler;
    // work->flags = { 0 };
}

/**
 * @brief Submit a work item.
 *
 * This routine submits work item @a work to be processed by workqueue
 * @a work_q. If the work item is already pending in the workqueue's queue
 * as a result of an earlier submission, this routine has no effect on the
 * work item. If the work item has already been processed, or is currently
 * being processed, its work is considered complete and the work item can be
 * resubmitted.
 *
 * @warning
 * A submitted work item must not be modified until it has been processed
 * by the workqueue.
 *
 * @note Can be called by ISRs.
 *
 * @param work_q Address of workqueue.
 * @param work Address of work item.
 *
 * @return N/A
 * @req K-WORK-001
 */
static inline void k_work_submit_to_queue(struct k_work_q *work_q,
                                          struct k_work *work)
{
    if (!atomic_test_and_set_bit(work->flags, K_WORK_STATE_PENDING))
    {
        SCOM_put((SCOM_Handle)&work_q->queue, (SCOM_Elem *)work);
    }
}

/**
 * @brief Submit a work item to a user mode workqueue
 *
 * Submits a work item to a workqueue that runs in user mode. A temporary
 * memory allocation is made from the caller's resource pool which is freed
 * once the worker thread consumes the k_work item. The workqueue
 * thread must have memory access to the k_work item being submitted. The caller
 * must have permission granted on the work_q parameter's queue object.
 *
 * Otherwise this works the same as k_work_submit_to_queue().
 *
 * @note Can be called by ISRs.
 *
 * @param work_q Address of workqueue.
 * @param work Address of work item.
 *
 * @retval -EBUSY if the work item was already in some workqueue
 * @retval -ENOMEM if no memory for thread resource pool allocation
 * @retval 0 Success
 * @req K-WORK-001
 */
#define k_work_submit_to_user_queue k_work_submit_to_queue

/**
 * @brief Check if a work item is pending.
 *
 * This routine indicates if work item @a work is pending in a workqueue's
 * queue.
 *
 * @note Can be called by ISRs.
 *
 * @param work Address of work item.
 *
 * @return true if work item is pending, or false if it is not pending.
 * @req K-WORK-001
 */
static inline bool k_work_pending(struct k_work *work)
{
    return atomic_test_bit(work->flags, K_WORK_STATE_PENDING);
}

/**
 * @brief Start a workqueue.
 *
 * This routine starts workqueue @a work_q. The workqueue spawns its work
 * processing thread, which runs forever.
 *
 * @param work_q Address of workqueue.
 * @param stack Pointer to work queue thread's stack space, as defined by
 *        K_THREAD_STACK_DEFINE()
 * @param stack_size Size of the work queue thread's stack (in bytes), which
 *        should either be the same constant passed to
 *        K_THREAD_STACK_DEFINE() or the value of K_THREAD_STACK_SIZEOF().
 * @param prio Priority of the work queue's thread.
 *
 * @return N/A
 * @req K-WORK-001
 */
extern void k_work_q_start(struct k_work_q *work_q,
                           k_thread_stack_t *stack,
                           size_t stack_size, int prio);

/**
 * @brief Start a workqueue in user mode
 *
 * This works identically to k_work_q_start() except it is callable from user
 * mode, and the worker thread created will run in user mode.
 * The caller must have permissions granted on both the work_q parameter's
 * thread and queue objects, and the same restrictions on priority apply as
 * k_thread_create().
 *
 * @param work_q Address of workqueue.
 * @param stack Pointer to work queue thread's stack space, as defined by
 *        K_THREAD_STACK_DEFINE()
 * @param stack_size Size of the work queue thread's stack (in bytes), which
 *        should either be the same constant passed to
 *        K_THREAD_STACK_DEFINE() or the value of K_THREAD_STACK_SIZEOF().
 * @param prio Priority of the work queue's thread.
 *
 * @return N/A
 * @req K-WORK-001
 */
extern void k_work_q_user_start(struct k_work_q *work_q,
                                k_thread_stack_t *stack,
                                size_t stack_size, int prio);

/**
 * @brief Initialize a delayed work item.
 *
 * This routine initializes a workqueue delayed work item, prior to
 * its first use.
 *
 * @param work Address of delayed work item.
 * @param handler Function to invoke each time work item is processed.
 *
 * @return N/A
 * @req K-DWORK-001
 */
extern void k_delayed_work_init(struct k_delayed_work *work,
                                k_work_handler_t handler);

/**
 * @brief Submit a delayed work item.
 *
 * This routine schedules work item @a work to be processed by workqueue
 * @a work_q after a delay of @a delay milliseconds. The routine initiates
 * an asynchronous countdown for the work item and then returns to the caller.
 * Only when the countdown completes is the work item actually submitted to
 * the workqueue and becomes pending.
 *
 * Submitting a previously submitted delayed work item that is still
 * counting down cancels the existing submission and restarts the
 * countdown using the new delay.  Note that this behavior is
 * inherently subject to race conditions with the pre-existing
 * timeouts and work queue, so care must be taken to synchronize such
 * resubmissions externally.
 *
 * @warning
 * A delayed work item must not be modified until it has been processed
 * by the workqueue.
 *
 * @note Can be called by ISRs.
 *
 * @param work_q Address of workqueue.
 * @param work Address of delayed work item.
 * @param delay Non-negative delay before submitting the work item (in
 *        milliseconds).
 *
 * @retval 0 Work item countdown started.
 * @retval -EINVAL Work item is being processed or has completed its work.
 * @retval -EADDRINUSE Work item is pending on a different workqueue.
 * @req K-DWORK-001
 */
extern int k_delayed_work_submit_to_queue(struct k_work_q *work_q,
                                          struct k_delayed_work *work,
                                          k_timeout_t delay);

/**
 * @brief Cancel a delayed work item.
 *
 * This routine cancels the submission of delayed work item @a work.
 * A delayed work item can only be canceled while its countdown is still
 * underway.
 *
 * @note Can be called by ISRs.
 *
 * @note The result of calling this on a k_delayed_work item that has
 * not been submitted (i.e. before the return of the
 * k_delayed_work_submit_to_queue() call) is undefined.
 *
 * @param work Address of delayed work item.
 *
 * @retval 0 Work item countdown canceled.
 * @retval -EINVAL Work item is being processed or has completed its work.
 * @req K-DWORK-001
 */
extern int k_delayed_work_cancel(struct k_delayed_work *work);

/**
 * @brief  Check if a delayed work item is pending.
 *
 * This routine indicates if the work item @a work is pending in a workqueue's
 * queue or waiting for the delay timeout.
 *
 * @note Checking if the delayed work is pending gives no guarantee that the
 *       work will still be pending when this information is used. It is up to
 *       the caller to make sure that this information is used in a safe manner.
 *
 * @note Can be called by ISRs.
 *
 * @param work Address of delayed work item.
 *
 * @return true if work item is waiting for the delay to expire or pending on a
 *         work queue, or false if it is not pending.
 */
extern bool k_delayed_work_pending(struct k_delayed_work *work);

extern uint32_t z_timeout_remaining(Clock_Struct *timeout);

/**
 * @brief Submit a work item to the system workqueue.
 *
 * This routine submits work item @a work to be processed by the system
 * workqueue. If the work item is already pending in the workqueue's queue
 * as a result of an earlier submission, this routine has no effect on the
 * work item. If the work item has already been processed, or is currently
 * being processed, its work is considered complete and the work item can be
 * resubmitted.
 *
 * @warning
 * Work items submitted to the system workqueue should avoid using handlers
 * that block or yield since this may prevent the system workqueue from
 * processing other work items in a timely manner.
 *
 * @note Can be called by ISRs.
 *
 * @param work Address of work item.
 *
 * @return N/A
 * @req K-WORK-001
 */
static inline void k_work_submit(struct k_work *work)
{
    k_work_submit_to_queue(&k_sys_work_q, work);
}

/**
 * @brief Submit a delayed work item to the system workqueue.
 *
 * This routine schedules work item @a work to be processed by the system
 * workqueue after a delay of @a delay milliseconds. The routine initiates
 * an asynchronous countdown for the work item and then returns to the caller.
 * Only when the countdown completes is the work item actually submitted to
 * the workqueue and becomes pending.
 *
 * Submitting a previously submitted delayed work item that is still
 * counting down cancels the existing submission and restarts the countdown
 * using the new delay. If the work item is currently pending on the
 * workqueue's queue because the countdown has completed it is too late to
 * resubmit the item, and resubmission fails without impacting the work item.
 * If the work item has already been processed, or is currently being processed,
 * its work is considered complete and the work item can be resubmitted.
 *
 * @warning
 * Work items submitted to the system workqueue should avoid using handlers
 * that block or yield since this may prevent the system workqueue from
 * processing other work items in a timely manner.
 *
 * @note Can be called by ISRs.
 *
 * @param work Address of delayed work item.
 * @param delay Non-negative delay before submitting the work item (in
 *        milliseconds).
 *
 * @retval 0 Work item countdown started.
 * @retval -EINVAL Work item is being processed or has completed its work.
 * @retval -EADDRINUSE Work item is pending on a different workqueue.
 * @req K-DWORK-001
 */
static inline int k_delayed_work_submit(struct k_delayed_work *work,
                                        k_timeout_t delay)
{
    return k_delayed_work_submit_to_queue(&k_sys_work_q, work, delay);
}

/**
 * @brief Get time remaining before a delayed work gets scheduled.
 *
 * This routine computes the (approximate) time remaining before a
 * delayed work gets executed. If the delayed work is not waiting to be
 * scheduled, it returns zero.
 *
 * @param work     Delayed work item.
 *
 * @return Remaining time (in milliseconds).
 * @req K-DWORK-001
 */
static inline int32_t k_delayed_work_remaining_get(struct k_delayed_work *work)
{
    return z_timeout_remaining(&work->timeout);
}

#if 0
/**
 * @brief Initialize a triggered work item.
 *
 * This routine initializes a workqueue triggered work item, prior to
 * its first use.
 *
 * @param work Address of triggered work item.
 * @param handler Function to invoke each time work item is processed.
 *
 * @return N/A
 */
extern void k_work_poll_init(struct k_work_poll *work,
                 k_work_handler_t handler);

/**
 * @brief Submit a triggered work item.
 *
 * This routine schedules work item @a work to be processed by workqueue
 * @a work_q when one of the given @a events is signaled. The routine
 * initiates internal poller for the work item and then returns to the caller.
 * Only when one of the watched events happen the work item is actually
 * submitted to the workqueue and becomes pending.
 *
 * Submitting a previously submitted triggered work item that is still
 * waiting for the event cancels the existing submission and reschedules it
 * the using the new event list. Note that this behavior is inherently subject
 * to race conditions with the pre-existing triggered work item and work queue,
 * so care must be taken to synchronize such resubmissions externally.
 *
 * @note Can be called by ISRs.
 *
 * @warning
 * Provided array of events as well as a triggered work item must be placed
 * in persistent memory (valid until work handler execution or work
 * cancellation) and cannot be modified after submission.
 *
 * @param work_q Address of workqueue.
 * @param work Address of delayed work item.
 * @param events An array of pointers to events which trigger the work.
 * @param num_events The number of events in the array.
 * @param timeout Non-negative timeout after which the work will be scheduled
 *          for execution even if not triggered.
 *
 *
 * @retval 0 Work item started watching for events.
 * @retval -EINVAL Work item is being processed or has completed its work.
 * @retval -EADDRINUSE Work item is pending on a different workqueue.
 */
extern int k_work_poll_submit_to_queue(struct k_work_q *work_q,
                       struct k_work_poll *work,
                       struct k_poll_event *events,
                       int num_events,
                       int32_t timeout);

/**
 * @brief Submit a triggered work item to the system workqueue.
 *
 * This routine schedules work item @a work to be processed by system
 * workqueue when one of the given @a events is signaled. The routine
 * initiates internal poller for the work item and then returns to the caller.
 * Only when one of the watched events happen the work item is actually
 * submitted to the workqueue and becomes pending.
 *
 * Submitting a previously submitted triggered work item that is still
 * waiting for the event cancels the existing submission and reschedules it
 * the using the new event list. Note that this behavior is inherently subject
 * to race conditions with the pre-existing triggered work item and work queue,
 * so care must be taken to synchronize such resubmissions externally.
 *
 * @note Can be called by ISRs.
 *
 * @warning
 * Provided array of events as well as a triggered work item must not be
 * modified until the item has been processed by the workqueue.
 *
 * @param work Address of delayed work item.
 * @param events An array of pointers to events which trigger the work.
 * @param num_events The number of events in the array.
 * @param timeout Non-negative timeout after which the work will be scheduled
 *          for execution even if not triggered.
 *
 * @retval 0 Work item started watching for events.
 * @retval -EINVAL Work item is being processed or has completed its work.
 * @retval -EADDRINUSE Work item is pending on a different workqueue.
 */
static inline int k_work_poll_submit(struct k_work_poll *work,
                     struct k_poll_event *events,
                     int num_events,
                     int32_t timeout)
{
    return k_work_poll_submit_to_queue(&k_sys_work_q, work,
                        events, num_events, timeout);
}

/**
 * @brief Cancel a triggered work item.
 *
 * This routine cancels the submission of triggered work item @a work.
 * A triggered work item can only be canceled if no event triggered work
 * submission.
 *
 * @note Can be called by ISRs.
 *
 * @param work Address of delayed work item.
 *
 * @retval 0 Work item canceled.
 * @retval -EINVAL Work item is being processed or has completed its work.
 */
extern int k_work_poll_cancel(struct k_work_poll *work);
#endif

#define k_fifo SCOM_Struct
#define k_lifo SCOM_Struct

#define Z_SCOM_INITIALIZER(obj)            \
    {                                      \
        NULL,                              \
            NULL,                          \
            NULL,                          \
            NULL,                          \
            0,                             \
            1,                             \
            Semaphore_Mode_COUNTING,       \
            0,                             \
            (void *)&(obj.sem.pendQ.next), \
            (void *)&(obj.sem.pendQ.next)  \
    }

#define _K_FIFO_INITIALIZER(obj) Z_SCOM_INITIALIZER(obj)
#define _K_LIFO_INITIALIZER(obj) Z_SCOM_INITIALIZER(obj)

#define K_FIFO_DEFINE(name) \
    SCOM_Struct name =      \
        Z_SCOM_INITIALIZER(name)

#define K_LIFO_DEFINE(name)                        \
    Z_STRUCT_SECTION_ITERABLE(SCOM_Struct, name) = \
        Z_SCOM_INITIALIZER(name)

#define k_fifo_init(fifo) \
    SCOM_construct((SCOM_Handle)(fifo), NULL)
#define k_lifo_init(lifo) \
    SCOM_construct((SCOM_Handle)(lifo), NULL)

#define k_fifo_put(fifo, data) \
    SCOM_put((SCOM_Handle)(fifo), (SCOM_Elem *)data)
#define k_lifo_put(lifo, data) \
    SCOM_push((SCOM_Handle)(lifo), (SCOM_Elem *)data)

#define k_fifo_get(fifo, timeout) \
    (void *)SCOM_get((SCOM_Handle)(fifo), timeout)
#define k_lifo_get(lifo, timeout) \
    (void *)SCOM_get((SCOM_Handle)(lifo), timeout)

#define k_fifo_cancel_wait(fifo) \
    SCOM_cancel((SCOM_Handle)(fifo))

/**
 * @brief Atomically add a list of elements to a FIFO.
 *
 * This routine adds a list of data items to @a fifo in one operation.
 * The data items must be in a singly-linked list, with the first word of
 * each data item pointing to the next data item; the list must be
 * NULL-terminated.
 *
 * @note Can be called by ISRs.
 *
 * @param fifo Address of the FIFO queue.
 * @param head Pointer to first node in singly-linked list.
 * @param tail Pointer to last node in singly-linked list.
 *
 * @return N/A
 * @req K-FIFO-001
 */
void k_fifo_put_list(struct k_fifo *fifo, void *head, void *tail);

struct k_mutex
{
    GateMutex_Handle mutexHandle;
    IArg key;
    bool initialized;
};

// Initialize a mutex object on the stack and a pointer to that object to be
// used by api calls
#define K_MUTEX_DEFINE(name) struct k_mutex name = {.initialized = FALSE};
//#define K_MUTEX_DEFINE(name) static struct k_mutex _##name = {.initialized = FALSE};\
//                             struct k_mutex *name = &(_##name);

static inline void k_mutex_init(struct k_mutex *mutex)
{
    Error_Block eb;
    GateMutex_Params params;

    // Init the mutex params
    GateMutex_Params_init(&params);

    // Create the tirtos mutex object
    mutex->mutexHandle = GateMutex_create(&params, &eb);
    mutex->initialized = TRUE;
}

static inline int k_mutex_lock(struct k_mutex *mutex, k_timeout_t timeout)
{
    // Init the mutex if first use
    if (!mutex->initialized)
    {
        k_mutex_init(mutex);
    }
    mutex->key = GateMutex_enter(mutex->mutexHandle);
    return 0;
}

static inline void k_mutex_unlock(struct k_mutex *mutex)
{
    if (mutex->initialized)
    {
        GateMutex_leave(mutex->mutexHandle, mutex->key);
    }
}

struct k_sem
{
    Semaphore_Struct semStruct;
    uint32_t maxCount;
};

void k_sem_init(struct k_sem *sem, unsigned int initial_count,
                unsigned int limit);
void k_sem_give(struct k_sem *sem);
int k_sem_take(struct k_sem *sem, k_timeout_t timeout);

/**
 * @brief Reset a semaphore's count to zero.
 *
 * This routine sets the count of @a sem to zero.
 *
 * @param sem Address of the semaphore.
 *
 * @return N/A
 */
static inline void k_sem_reset(struct k_sem *sem)
{
    Semaphore_reset(Semaphore_handle(&(sem->semStruct)), 0);
}

/**
 * @brief Get a semaphore's count.
 *
 * This routine returns the current count of @a sem.
 *
 * @param sem Address of the semaphore.
 *
 * @return Current semaphore count.
 */
static inline unsigned int k_sem_count_get(struct k_sem *sem)
{
    return (Semaphore_getCount(Semaphore_handle(&(sem->semStruct))));
}

struct k_mem_pool
{
    IHeap_Handle base;
    Queue_Elem wait_q;
};

struct k_mem_block_id
{
    uint32_t pool : 8;
    uint32_t level : 4;
    uint32_t block : 20;
};

struct k_mem_block
{
    void *data;
    struct k_mem_block_id id; // TODO currently not using
};

/**
 * @addtogroup mem_pool_apis
 * @{
 */

/**
 * @brief Statically define and initialize a memory pool.
 *
 * The memory pool's buffer contains @a n_max blocks that are @a max_size bytes
 * long. The memory pool allows blocks to be repeatedly partitioned into
 * quarters, down to blocks of @a min_size bytes long. The buffer is aligned
 * to a @a align -byte boundary.
 *
 * If the pool is to be accessed outside the module where it is defined, it
 * can be declared via
 *
 * @code extern struct k_mem_pool <name>; @endcode
 *
 * @param name Name of the memory pool.
 * @param minsz Size of the smallest blocks in the pool (in bytes).
 * @param maxsz Size of the largest blocks in the pool (in bytes).
 * @param nmax Number of maximum sized blocks in the pool.
 * @param align Alignment of the pool's buffer (power of 2).
 * @req K-MPOOL-001
 */
#if 0
#define K_MEM_POOL_DEFINE(name, minsz, maxsz, nmax, align)                                                      \
    char __aligned(WB_UP(align)) _mpool_buf_##name[WB_UP(maxsz) * nmax + _MPOOL_BITS_SIZE(maxsz, minsz, nmax)]; \
    struct sys_mem_pool_lvl _mpool_lvls_##name[Z_MPOOL_LVLS(maxsz, minsz)];                                     \
    Z_STRUCT_SECTION_ITERABLE(k_mem_pool, name) = {                                                             \
        .base = {                                                                                               \
            .buf = _mpool_buf_##name,                                                                           \
            .max_sz = WB_UP(maxsz),                                                                             \
            .n_max = nmax,                                                                                      \
            .n_levels = Z_MPOOL_LVLS(maxsz, minsz),                                                             \
            .levels = _mpool_lvls_##name,                                                                       \
            .flags = SYS_MEM_POOL_KERNEL}};                                                                     \
    BUILD_ASSERT(WB_UP(maxsz) >= _MPOOL_MINBLK)
#endif
/**
 * @brief Allocate memory from a memory pool.
 *
 * This routine allocates a memory block from a memory pool.
 *
 * @param pool Address of the memory pool.
 * @param block Pointer to block descriptor for the allocated memory.
 * @param size Amount of memory to allocate (in bytes).
 * @param timeout Non-negative waiting period to wait for operation to complete
 *        (in milliseconds). Use K_NO_WAIT to return without waiting,
 *        or K_FOREVER to wait as long as necessary.
 *
 * @retval 0 Memory allocated. The @a data field of the block descriptor
 *         is set to the starting address of the memory block.
 * @retval -ENOMEM Returned without waiting.
 * @retval -EAGAIN Waiting period timed out.
 * @req K-MPOOL-002
 */
extern int k_mem_pool_alloc(struct k_mem_pool *pool, struct k_mem_block *block,
                            size_t size, k_timeout_t timeout);

/**
 * @brief Allocate memory from a memory pool with malloc() semantics
 *
 * Such memory must be released using k_free().
 *
 * @param pool Address of the memory pool.
 * @param size Amount of memory to allocate (in bytes).
 * @return Address of the allocated memory if successful, otherwise NULL
 * @req K-MPOOL-002
 */
extern void *k_mem_pool_malloc(struct k_mem_pool *pool, size_t size);

/**
 * @brief Free memory allocated from a memory pool.
 *
 * This routine releases a previously allocated memory block back to its
 * memory pool.
 *
 * @param block Pointer to block descriptor for the allocated memory.
 *
 * @return N/A
 * @req K-MPOOL-002
 */
extern void k_mem_pool_free(struct k_mem_block *block);

/**
 * @brief Free memory allocated from a memory pool.
 *
 * This routine releases a previously allocated memory block back to its
 * memory pool
 *
 * @param id Memory block identifier.
 *
 * @return N/A
 * @req K-MPOOL-002
 */
extern void k_mem_pool_free_id(struct k_mem_block_id *id);

/**
 * @}
 */

/**
 * @defgroup heap_apis Heap Memory Pool APIs
 * @ingroup kernel_apis
 * @{
 */

/**
 * @brief Allocate memory from heap.
 *
 * This routine provides traditional malloc() semantics. Memory is
 * allocated from the heap memory pool.
 *
 * @param size Amount of memory requested (in bytes).
 *
 * @return Address of the allocated memory if successful; otherwise NULL.
 * @req K-HEAP-001
 */
extern void *k_malloc(size_t size);

/**
 * @brief Free memory allocated from heap.
 *
 * This routine provides traditional free() semantics. The memory being
 * returned must have been allocated from the heap memory pool or
 * k_mem_pool_malloc().
 *
 * If @a ptr is NULL, no operation is performed.
 *
 * @param ptr Pointer to previously allocated memory.
 *
 * @return N/A
 * @req K-HEAP-001
 */
extern void k_free(void *ptr);

/**
 * @brief Allocate memory from heap, array style
 *
 * This routine provides traditional calloc() semantics. Memory is
 * allocated from the heap memory pool and zeroed.
 *
 * @param nmemb Number of elements in the requested array
 * @param size Size of each array element (in bytes).
 *
 * @return Address of the allocated memory if successful; otherwise NULL.
 * @req K-HEAP-001
 */
extern void *k_calloc(size_t nmemb, size_t size);

/** @} */

#endif /* KERNEL_H_INCLUDE */
