/*
 * Copyright (c) 2022 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== SecureCallback.h ========
 */

#ifndef ti_drivers_tfm_SecureCallback__include
#define ti_drivers_tfm_SecureCallback__include

#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/utils/List.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*SecureCallback_FuncPtr)(uintptr_t arg);

typedef struct SecureCallback_Object
{
    List_Elem elem;
    uintptr_t arg;
    bool pending;
    SecureCallback_FuncPtr fxn;
} SecureCallback_Object;

typedef SecureCallback_Object *SecureCallback_Handle;

typedef struct SecureCallback_Config
{
    uint32_t intNumber;
    uint8_t intPriority;
} SecureCallback_Config;

/*!
 *  @brief  Initialize SecureCallback driver
 *
 *  @note   This API is only available to non-secure code. Must be called once
 *          prior to invoking any other SecureCallback APIs.
 */
void SecureCallback_init(void);

/*!
 *  @brief  Construct a callback object and adds it to the servicing list.
 *
 *  @note   This API is only available to non-secure code.
 *
 *  @param[in]  object  A pointer to a #SecureCallback_Object structure.
 *  @param[in]  fxn     A pointer to this object's callback function.
 *  @param[in]  arg     A uint argument passed to the callback function.
 */
SecureCallback_Handle SecureCallback_construct(SecureCallback_Object *object,
                                               SecureCallback_FuncPtr fxn,
                                               uintptr_t arg);

/*!
 *  @brief  Destruct a callback object, removing it from the servicing list.
 *
 *  @note   This API is only available to non-secure code.
 *
 *  @param[in]  object  A pointer to a #SecureCallback_Object structure.
 */
void SecureCallback_destruct(SecureCallback_Object *object);

/*!
 *  @brief  Triggers the dispatcher targeting the specified callback.
 *
 *  @note   This API is available to both non-secure and secure code.
 *
 *  @param[in]  handle  A #SecureCallback_Handle pointer.
 *
 *  @pre The callback object must have been created by #SecureCallback_construct().
 */
void SecureCallback_post(SecureCallback_Handle handle);

#if defined(__cplusplus)
}
#endif

#endif /* ti_drivers_tfm_SecureCallback__include */
