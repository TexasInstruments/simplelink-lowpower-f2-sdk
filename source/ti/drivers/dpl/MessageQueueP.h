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
/** ============================================================================
 *  @file       MessageQueueP.h
 *
 *  @brief      MessageQueue module for the RTOS Porting Interface
 *
 *  MessageQueueP objects are RTOS message queues backed by OS-specific queue or
 *  mailbox objects.
 *
 *  Message queues can be used for intertask communication. They support sending
 *  messages between tasks, and between interrupts and tasks.
 *  Message queues can either be allocated statically with
 *  #MessageQueueP_construct() or dynamically with #MessageQueueP_create().
 *
 *  ============================================================================
 */

#ifndef ti_dpl_MessageQueueP__include
#define ti_dpl_MessageQueueP__include

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <ti/devices/DeviceFamily.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief    Number of bytes greater than or equal to the size of any RTOS Queue/Mailbox data structure.
 *
 *  TI-RTOS7: 104
 *  FreeRTOS: 80
 */
#define MessageQueueP_STRUCT_SIZE (104)

/*!
 *  @brief    Required number of bytes of a statically allocated message queue buffer.
 *
 * This macro is defined to support the user in configuring the size of a message queue
 * buffer. A pointer to this user defined buffer is one of the arguments of the
 * #MessageQueueP_construct() function.
 * The macro gives the minimal number of bytes required for the message queue.
 * Please note the following for devices supporting TI-RTOS7:
 * - The macro takes into account an eight byte message header which is only required
 *   by TI-RTOS and not by FreeRTOS. For user applications only targeting FreeRTOS,
 *   SRAM usage can be limited by setting the buffer size to (msgCount * msgSize)
 *   instead of using this macro.
 */
#if ((DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X0_CC26X0) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X2_CC26X2) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X1_CC26X1) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4))
    #define MessageQueueP_BUFFER_SIZE(msgSize, msgCount) ((msgCount) * ((msgSize) + 8))
#else
    #define MessageQueueP_BUFFER_SIZE(msgSize, msgCount) ((msgCount) * (msgSize))
#endif

/*!
 *  @brief    MessageQueueP structure.
 *
 *  Opaque structure that should be large enough to hold any of the RTOS specific MessageQueueP objects.
 */
typedef union MessageQueueP_Struct
{
    uint32_t dummy; /*!< Align object */
    uint8_t data[MessageQueueP_STRUCT_SIZE];
} MessageQueueP_Struct;

/*!
 *  @brief    Wait forever define
 */
#define MessageQueueP_WAIT_FOREVER ~(0)

/*!
 *  @brief    No wait define
 */
#define MessageQueueP_NO_WAIT (0)

/*!
 *  @brief    Status codes for MessageQueueP APIs
 */
typedef enum
{
    /*! API completed successfully */
    MessageQueueP_OK      = 0,
    /*! API failed because of a timeout */
    MessageQueueP_TIMEOUT = -1
} MessageQueueP_Status;

/*!
 *  @brief    Opaque client reference to an instance of a MessageQueueP
 *
 * A MessageQueueP_Handle returned from #MessageQueueP_create() or
 * #MessageQueueP_construct() represents that instance. It is then is used
 * in the other instance based functions (e.g. #MessageQueueP_pend(),
 * #MessageQueueP_post(), etc.).
 */
typedef void *MessageQueueP_Handle;

/*!
 * @brief Create a MessageQueueP, allocating memory on the heap.
 *
 * #MessageQueueP_create creates a new message queue object. #MessageQueueP_create
 * returns the handle of the new message queue object or NULL if the message queue
 * could not be created.
 *
 * The message queue object will be allocated on the heap - make sure you have a
 * sufficiently large heap.
 *
 * \note This API cannot be called from interrupt contexts.
 *
 * For FreeRTOS, configSUPPORT_DYNAMIC_ALLOCATION has to be set to 1 in FreeRTOSConfig.h.
 * See 'Configuration with FreeRTOS' in the Core SDK User's Guide for how to do this.
 *
 * @param msgSize        The size, in bytes, required to hold each item in the message queue
 * @param msgCount       The maximum number of items the message queue can hold at any one time
 *
 * @retval MessageQueueP handle (NULL on failure)
 */
extern MessageQueueP_Handle MessageQueueP_create(size_t msgSize, size_t msgCount);

/*!
 * @brief Construct a MessageQueueP from statically allocated memory.
 *
 * #MessageQueueP_construct creates a new message queue object. #MessageQueueP_construct
 * returns the handle of the new message queue object or NULL if the message queue
 * could not be created.
 *
 * To use #MessageQueueP_construct \a msgBuf must point to a valid preallocated memory
 * array that is at least large enough to hold the maximum number of items that can be
 * in the message queue at any one time.
 *  - When used with FreeRTOS the array size must be at least ( \a msgCount * \a msgSize) bytes.
 *  - When used with TI-RTOS the array size must be at least ( \a msgCount * ( \a msgSize + 8)) bytes.
 *  - Since the buffer must be a aligned properly, it may be necessary to 'round up' the total size
 *    of the buffer to the next multiple of the alignment for odd sized messages.
 *
 * \note This API cannot be called from interrupt contexts.
 *
 * For FreeRTOS, configSUPPORT_STATIC_ALLOCATION has to be set to 1 in FreeRTOSConfig.h.
 * See 'Configuration with FreeRTOS' in the Core SDK User's Guide for how to do this.
 *
 * @param queueStruct    Must point to a variable that holds the message queue's data structure
 * @param msgSize        The size, in bytes, required to hold each item in the message queue
 * @param msgCount       The maximum number of items the message queue can hold at any one time
 * @param msgBuf         Pointer to variable that holds the message queue's data buffer
 *
 * @retval MessageQueueP handle (NULL on failure)
 */
MessageQueueP_Handle MessageQueueP_construct(MessageQueueP_Struct *queueStruct,
                                             size_t msgSize,
                                             size_t msgCount,
                                             void *msgBuf);

/*!
 * @brief Delete a MessageQueueP.
 *
 * #MessageQueueP_delete finalizes and frees this previously allocated message queue
 * object, setting the referenced handle to NULL.
 * This function should be used when the message queue was created by the
 * #MessageQueueP_create() function.
 *
 * \note This API cannot be called from interrupt contexts.
 *
 * @param handle    A handle to the message queue to be deleted
 */
extern void MessageQueueP_delete(MessageQueueP_Handle handle);

/*!
 * @brief Destruct a MessageQueueP.
 *
 * #MessageQueueP_destruct finalizes the message queue object inside the provided structure.
 * This function should be used when the message queue was constructed by the
 * #MessageQueueP_construct() function.
 *
 * \note This API cannot be called from interrupt contexts.
 *
 * @param handle    A handle to the message queue to be destructed
 */
extern void MessageQueueP_destruct(MessageQueueP_Handle handle);

/*!
 * @brief Receive an item from a message queue.
 *
 * #MessageQueueP_pend receives an item from the provided message queue.
 *
 * @param handle     The handle to the message queue from which the item is to be received
 * @param message    Pointer to the buffer into which the received item will be copied
 * @param timeout    The maximum duration in system clock ticks a task should block waiting
 *                   for an item to be received. When no wait or wait forever options are
 *                   wanted the #MessageQueueP_NO_WAIT and #MessageQueueP_WAIT_FOREVER defines
 *                   can be used.
 *
 * @retval Status of the function
 *             - #MessageQueueP_OK: Item recieved
 *             - #MessageQueueP_TIMEOUT: Timed out. Item was not recieved.
 */
extern MessageQueueP_Status MessageQueueP_pend(MessageQueueP_Handle handle, void *message, uint32_t timeout);

/*!
 * @brief Receive an item from a message queue without removing the item from the queue.
 *
 * #MessageQueueP_peek receives an item from the provided message queue without removing the
 * item from the queue.
 *
 * @param handle     The handle to the message queue from which the item is to be received
 * @param message    Pointer to the buffer into which the received item will be copied
 * @param timeout    The maximum duraton in system clock ticks a task should block waiting
 *                   for an item to be received. When no wait or wait forever options are
 *                   wanted the #MessageQueueP_NO_WAIT and #MessageQueueP_WAIT_FOREVER defines
 *                   can be used.
 *
 * @retval Status of the function
 *             - #MessageQueueP_OK: Item recieved
 *             - #MessageQueueP_TIMEOUT: Timed out. Item was not recieved.
 */
extern MessageQueueP_Status MessageQueueP_peek(MessageQueueP_Handle handle, void *message, uint32_t timeout);

/*!
 * @brief Post an item on a message queue.
 *
 * #MessageQueueP_post posts an item on the provided message queue.
 *
 * @param handle     The handle to the message queue to which the item is to be posted
 * @param message    Pointer to the buffer from which the item to be posted is copied
 * @param timeout    The maximum duraton in system clock ticks a task should block waiting
 *                   for an item to be posted. When no wait or wait forever options are
 *                   wanted the #MessageQueueP_NO_WAIT and #MessageQueueP_WAIT_FOREVER defines
 *                   can be used.
 *
 * @retval Status of the function
 *             - #MessageQueueP_OK: Item posted
 *             - #MessageQueueP_TIMEOUT: Timed out. Item was not posted.
 */
extern MessageQueueP_Status MessageQueueP_post(MessageQueueP_Handle handle, const void *message, uint32_t timeout);

/*!
 * @brief Post an item in the front of a message queue.
 *
 * #MessageQueueP_postFront posts an item in the front of the provided message queue.
 *
 * @param handle     The handle to the message queue to which the item is to be posted
 * @param message    Pointer to the buffer from which the item to be posted is copied
 * @param timeout    The maximum duraton in system clock ticks a task should block waiting
 *                   for an item to be posted. When no wait or wait forever options are
 *                   wanted the #MessageQueueP_NO_WAIT and #MessageQueueP_WAIT_FOREVER defines
 *                   can be used.
 *
 * @retval Status of the function
 *             - #MessageQueueP_OK: Item posted
 *             - #MessageQueueP_TIMEOUT: Timed out. Item was not posted.
 */
extern MessageQueueP_Status MessageQueueP_postFront(MessageQueueP_Handle handle, const void *message, uint32_t timeout);

/*!
 * @brief Get the number of messages stored in a message queue.
 *
 * Returns the number of messages in the specified message queue.
 *
 * @param handle    A MessageQueueP_Handle returned from #MessageQueueP_create()
 *                  or #MessageQueueP_construct()
 *
 * @retval Number of stored messages in the specified message queue
 */
extern size_t MessageQueueP_getPendingCount(MessageQueueP_Handle handle);

/*!
 * @brief Get the number of free spaces in a message queue.
 *
 * Returns the number of free spaces in the specified message queue.
 *
 * @param handle    A MessageQueueP_Handle returned from #MessageQueueP_create()
 *                  or #MessageQueueP_construct()
 *
 *  @retval Number of free spaces in the specified message queue
 */
extern size_t MessageQueueP_getFreeCount(MessageQueueP_Handle handle);

#ifdef __cplusplus
}
#endif

#endif /* ti_dpl_MessageQueueP__include */