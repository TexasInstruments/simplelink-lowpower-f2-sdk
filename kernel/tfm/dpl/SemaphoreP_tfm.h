/*
 * Copyright (c) 2022, Texas Instruments Incorporated
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
 *  @file       SemaphoreP_tfm.h
 *
 *  @brief      Semaphore module for the Secure Driver Porting Interface
 *
 *  This module only implements binary semaphores without timeout.
 *
 *  Binary semaphores can have only two states: available (count = 1) and
 *  unavailable (count = 0). They can be used to share a single resource
 *  between tasks. They can also be used for a basic signalling mechanism, where
 *  the semaphore can be posted multiple times. Binary semaphores do not keep
 *  track of the count; they simply track whether the semaphore has been posted
 *  or not.
 *
 *  ============================================================================
 */

#ifndef ti_dpl_SemaphoreP_tfm__include
#define ti_dpl_SemaphoreP_tfm__include

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief    SemaphoreP structure.
 *
 */
typedef union SemaphoreP_Struct {
    uint32_t dummy;  /*!< Align object */
    bool isAvailable;
} SemaphoreP_Struct;

/*!
 *  @brief    No wait define
 */
#define SemaphoreP_NO_WAIT       (0)

/*!
 *  @brief    Status codes for SemaphoreP APIs (for backwards compatibility)
 */
typedef enum {
    /*! API completed successfully */
    SemaphoreP_OK = 0,
    /*! API failed because of a timeout */
    SemaphoreP_TIMEOUT = -1
} SemaphoreP_Status;

/*!
 *  @brief    Opaque client reference to an instance of a SemaphoreP
 *
 *  A SemaphoreP_Handle returned from the ::SemaphoreP_create represents that
 *  instance and  is used in the other instance based functions (e.g.
 *  ::SemaphoreP_post or ::SemaphoreP_pend, etc.).
 */
typedef  void *SemaphoreP_Handle;

/*
 *  @brief  Function to construct a binary a semaphore.
 *
 *  @param  sem     Pointer to SemaphoreP_Struct
 *
 *  @param  count   Semaphore is available (count = 1) and unavailable (count = 0)
 *
 *  @return handle for the semaphore
 */
extern SemaphoreP_Handle SemaphoreP_constructBinary(SemaphoreP_Struct *sem,
                                                    unsigned int count);

/*!
 *  @brief  Function to pend (wait) on a semaphore
 *
 *  @param  handle  A SemaphoreP_Handle returned from ::SemaphoreP_constructBinary
 *
 *  @param  timeout Timeout must always be SemaphoreP_NO_WAIT.
 *
 *  @return Status of the functions
 *    - SemaphoreP_OK: Obtained the semaphore
 *    - SemaphoreP_TIMEOUT: Timed out. Semaphore was not obtained.
 */
extern SemaphoreP_Status SemaphoreP_pend(SemaphoreP_Handle handle,
                                         uint32_t timeout);

/*!
 *  @brief  Function to post (signal) a semaphore
 *
 *  @param  handle  A SemaphoreP_Handle returned from ::SemaphoreP_constructBinary
 */
extern void SemaphoreP_post(SemaphoreP_Handle handle);

/*!
 *  @brief  Function to destroy a semaphore
 *
 *  @param  handle  A SemaphoreP_Handle returned from ::SemaphoreP_constructBinary
 */
extern void SemaphoreP_destruct(SemaphoreP_Handle handle);


#ifdef __cplusplus
}
#endif

#endif /* ti_dpl_SemaphoreP_s__include */
