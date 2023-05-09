/*
 * Copyright (c) 2020, Texas Instruments Incorporated
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
 *  ======== SCOM.h ========
 */

#ifndef ti_kernel_SCOM__include
#define ti_kernel_SCOM__include

#include <stdbool.h>
#include <stdint.h>

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct SCOM_Elem
    {
        struct SCOM_Elem *next;
    } SCOM_Elem;

    typedef struct SCOM_Params
    {
        bool dummy;
    } SCOM_Params;

    typedef struct SCOM_Struct
    {
        SCOM_Elem *head;
        SCOM_Elem *tail;
        Semaphore_Struct sem;
    } SCOM_Struct;

    typedef SCOM_Struct *SCOM_Handle;

/*!
 *  @brief  Function to convert a SCOM_Struct pointer to a SCOM_Handle
 */
#define SCOM_handle(structPtr) ((SCOM_Handle)structPtr)

    /*!
     *  @brief  This routine causes first thread pending on SCOM, if any, to
     *          return from SCOM_get() call with NULL value (as if timeout expired).
     */
    extern void SCOM_cancel(SCOM_Handle scom);

    /*!
     *  @brief  Function to create a SCOM_Elem
     */
    extern SCOM_Handle SCOM_create(SCOM_Params *params);

    /*!
     *  @brief  Function to initialize a SCOM_Elem
     */
    extern void SCOM_construct(SCOM_Struct *scom, SCOM_Params *params);

    /*!
     *  @brief  Function to initialize a SCOM_Elem
     */
    extern void SCOM_delete(SCOM_Handle scom);

    /*!
     *  @brief  Function to initialize a SCOM_Elem
     */
    extern void SCOM_destruct(SCOM_Struct *scom);

    /*!
     *  @brief  Function to test whether a linked list is empty
     *
     *  @param  scom A pointer to a linked list
     *
     *  @return true if empty, false if not empty
     */
    static inline bool SCOM_empty(SCOM_Handle scom)
    {
        return (scom->head == NULL);
    }

    /*!
     *  @brief  Function to atomically put an elem onto the end of a linked list in
     *          an non-atomic manner.
     *
     *  @param  scom A pointer to the linked list
     *
     *  @param  elem Element to place onto the end of the linked list
     */
    extern void SCOM_enqueue(SCOM_Handle scom, SCOM_Elem *elem);

    /*!
     *  @brief  Function to atomically get the first elem in a linked list
     *
     *  @param  scom A pointer to a linked list
     *          timeout Timeout in milliseconds
     *
     *  @return Pointer the first elem in the linked list or NULL if empty
     */
    extern SCOM_Elem *SCOM_get(SCOM_Handle scom, k_timeout_t timeout);

    /*!
     *  @brief  Function to atomically push an elem onto the beginning of a linked list
     *
     *  @param  scom A pointer to the linked list
     *
     *  @param  elem Element to place onto the beginning of the linked list
     */
    extern void SCOM_push(SCOM_Handle scom, SCOM_Elem *elem);

    /*!
     *  @brief  Function to atomically put an elem onto the end of a linked list
     *
     *  @param  scom A pointer to the linked list
     *
     *  @param  elem Element to place onto the end of the linked list
     */
    extern void SCOM_put(SCOM_Handle scom, SCOM_Elem *elem);

    /*!
     *  @brief  Function to remove an elem from the linked list
     *
     *  @param  scom A pointer to the linked list
     *
     *  @param  elem Element to remove
     *
     *  @return true if data item was removed
     */
    extern bool SCOM_remove(SCOM_Handle scom, SCOM_Elem *elem);

#ifdef __cplusplus
}
#endif

#endif /* ti_kernel_SCOM__include */
