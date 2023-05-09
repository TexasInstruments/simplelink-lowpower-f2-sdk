/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
/**
 * @file osal_mq.h
 * @brief ACE OSAL intra-process message queue
 * @addtogroup ACE_OSAL_MQ
 *
 * The aceMq_*() APIs implement an intra-process message queue i.e. it
 * operates within a process context. It is a multi writer, single reader
 * message queue. The APIs allow for a FIFO queue.
 *
 * Notes - Intentionally missing following items:
 * Queue names - If names for some queues are needed, - they can be
 *               implemented by the user
 * priorities - if multiple priorities are needed - use multiple queues and
 *              a semaphore indicating message availability
 * aceMq_getFreeCount - this is just aceMq_getSize - aceMq_getMsgCount
 *
 * @{
 */

#ifndef OSAL_ACE_MQ_H
#define OSAL_ACE_MQ_H

#include <ace/ace_status.h>
#include <ace/os_specific.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @cond DEPRECATED
 * @deprecated Please use the new symbols.
 * @{
 */
#define aceMQ_struct aceMq_struct
#define aceMQ_t aceMq_t

#define aceMQ_create aceMq_create
#define aceMQ_delete aceMq_delete
#define aceMQ_put aceMq_put
#define aceMQ_get aceMq_get
#define aceMQ_getTimeout aceMq_getTimeout
#define aceMQ_getSize aceMq_getSize
#define aceMQ_getMsgSize aceMq_getMsgSize
#define aceMQ_getMsgCount aceMq_getMsgCount
/**
 * @}
 * @endcond
 */

/**
 * @brief Type representing the ace queue
 */
typedef struct aceMq_struct aceMq_t;

/**
 * @brief macro for calculating MQ buffer size
 */
#define ACE_QUEUE_BUFF_SIZE(num_items, item_size) ((num_items) * (item_size))

/* ----------- function prototypes ----------- */
/**
 * @brief Create a ace message queue
 *
 * @param [in] mem  Caller supplied memory for data storage
 * @param [in] mem_size  Size of the memory for data storage in byte
 *                       mem_size = msg_size * max_message_count
 * @param [in] msg_size  Message size (bytes)
 * @param [out] queue Pointer to created ace MQ
 * @returns ACE_STATUS_OK or an error status from ace_status_t
 * @remark Thread-safe. The implementation memsets the passed in buffer.
 */
ace_status_t aceMq_create(void* mem, uint32_t mem_size, uint32_t msg_size,
                          aceMq_t* queue);

/**
 * @brief Delete the message queue identified by the handle
 *
 * @param [in] queue Pointer of ace message queue
 * @returns ACE_STATUS_OK or an error status from ace_status_t
 *
 * @remark thread-safe
 */
ace_status_t aceMq_delete(aceMq_t* queue);

/**
 * @brief Enqueue a message onto the queue
 *
 * @param [in] queue Pointer of ace message queue
 * @param [in] msg_ptr Pointer to message to enqueue
 * @param [in] timeout_ms If full, max time in milliseconds to block waiting for
               free space on queue.
 * @returns ACE_STATUS_OK if message was successfully written from the queue
 *          before the timeout_ms expired or an error status from ace_status_t.
 * @retval  ACE_STATUS_TIMEOUT when non-zero timeout_ms expired and message
 *          could not be written to the queue because it was full.
 * @retval  ACE_STATUS_TRY_AGAIN when timeout_ms is 0, if message could not be
 *          written to the queue because it was full.
 * @remark thread-safe
 */
ace_status_t aceMq_put(aceMq_t* queue, const void* msg_ptr,
                       uint32_t timeout_ms);

/**
 * @brief Dequeue a message from the queue
 * If the message queue was not empty, then the calling thread was placed into
 * the blocked state to wait for message to become available on the queue.
 *
 * @param [in] queue Pointer of ace message queue
 * @param [out] msg_out Pointer where msg is dequeued
 * @returns ACE_STATUS_OK (if message was successfully read from the queue)\n
 *          or an error status from ace_status_t
 *
 * @remark thread-safe, blocks indefinitely until a message
 *         is recieved.
 */
ace_status_t aceMq_get(aceMq_t* queue, void* msg_out);

/**
 * @brief Dequeue a message from the queue with timeout
 *
 * @param [in] queue Pointer of ace message queue
 * @param [in] timeout_ms If empty, max time in milliseconds to wait for msg on
               queue.
 * @param [out] msg_out Pointer where msg is dequeued
 * @returns ACE_STATUS_OK if message was successfully read from the queue
 *          before the timeout_ms expired or an error status from ace_status_t.
 * @retval  ACE_STATUS_TIMEOUT when non-zero timeout_ms expired and no message
 *          could be read from the queue.
 * @retval  ACE_STATUS_TRY_AGAIN when timeout_ms is 0, and no message could be
 *          read from the queue.
 * @remark thread-safe, user needs to handle if the API times out
 */
ace_status_t aceMq_getTimeout(aceMq_t* queue, uint32_t timeout_ms,
                              void* msg_out);

/**
 * @brief Get the maximum number of messages on the queue
 *
 * @param [in] queue Pointer of ace message queue
 * @param [out] qSize Pointer where capacity of the queue is returned
 * @returns ACE_STATUS_OK or an error status from ace_status_t
 *
 * @remark thread-safe
 */
ace_status_t aceMq_getSize(const aceMq_t* queue, uint32_t* qSize);

/**
 * @brief Get the message size (bytes) that can be enqueued on the queue
 *
 * @param [in] queue Pointer of ace message queue
 * @param [out] msgSize Pointer where message size (bytes) is returned
 * @returns ACE_STATUS_OK or an error status from ace_status_t
 *
 * @remark thread-safe
 */
ace_status_t aceMq_getMsgSize(const aceMq_t* queue, uint32_t* msgSize);

/**
 * @brief Get the number of queued messages
 *
 * @param [in] queue Pointer of ace message queue
 * @param [out] numQueued Pointer where number of queued messages is returned
 * @returns ACE_STATUS_OK or an error status from ace_status_t
 *
 * @remark thread-safe
 */
ace_status_t aceMq_getMsgCount(aceMq_t* queue, uint32_t* numQueued);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* OSAL_ACE_MQ_H */
