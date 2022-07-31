/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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

/*!***************************************************************************
 *  @file       AESCommon.h
 *
 *  @brief      AES common module header for all devices
 *****************************************************************************/

#ifndef ti_drivers_AESCommon_include
#define ti_drivers_AESCommon_include

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief   Successful status code.
 *
 * Functions return #AES_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define AES_STATUS_SUCCESS ((int_fast16_t)0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return #AES_STATUS_ERROR if the function was not executed
 * successfully and no more pertinent error code could be returned.
 */
#define AES_STATUS_ERROR ((int_fast16_t)-1)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * AES driver implementations may have hardware or software limitations on how
 * many clients can simultaneously perform operations. This status code is
 * returned if the mutual exclusion mechanism signals that an operation cannot
 * currently be performed.
 */
#define AES_STATUS_RESOURCE_UNAVAILABLE ((int_fast16_t)-2)

/*!
 * @brief   The ongoing operation was canceled.
 */
#define AES_STATUS_CANCELED ((int_fast16_t)-3)

/*!
 * @brief   The MAC verification failed.
 *
 * Functions return #AES_STATUS_MAC_INVALID if the MAC computed
 * for the provided (key, message) pair did not match the MAC provided.
 */
#define AES_STATUS_MAC_INVALID ((int_fast16_t)-4)

/*!
 *  @brief   The operation tried to load a key from the keystore using
 *           an invalid key ID.
 *
 * This code is returned if the provided CryptoKey reference
 * is returned as invalid by the key store module.
 */
#define AES_STATUS_KEYSTORE_INVALID_ID ((int_fast16_t)-5)

/*!
 * @brief   The key store module returned a generic error. See key store
 * documentation for additional details.
 */
#define AES_STATUS_KEYSTORE_GENERIC_ERROR ((int_fast16_t)-6)

/*!
 * @brief   The operation requested is not supported.
 */
#define AES_STATUS_FEATURE_NOT_SUPPORTED ((int_fast16_t)-7)

/*!
 * @brief    The operation does not support non-word-aligned input and/or output.
 */
#define AES_STATUS_UNALIGNED_IO_NOT_SUPPORTED ((int_fast16_t)-8)

/*!
 * @brief   A driver shall use this error code and grow negatively until
 *          (AES_STATUS_RESERVED + 1) if more driver specific error codes
 *          are needed beyond the common codes listed above.
 *
 * @note    Not to be confused with #AES_STATUS_RESERVED which is for defining
 *          device specific codes if needed for a given driver, while
 *          #AES_STATUS_DRIVER_SPECIFIC_ERROR is for a driver but common across
 *          all devices for which that driver is implemented.
 *
 * Example implementation specific status codes:
 * @code
 * #define AESXYZ_STATUS_ERROR0    AES_STATUS_DRIVER_SPECIFIC_ERROR - 0
 * #define AESXYZ_STATUS_ERROR1    AES_STATUS_DRIVER_SPECIFIC_ERROR - 1
 * #define AESXYZ_STATUS_ERROR2    AES_STATUS_DRIVER_SPECIFIC_ERROR - 2
 * @endcode
 */
#define AES_STATUS_DRIVER_SPECIFIC_ERROR ((int_fast16_t)-16)

/*!
 * Common AES status code reservation offset.
 * AES driver implementations should offset status codes with
 * #AES_STATUS_RESERVED growing negatively.
 *
 * Example implementation specific status codes:
 * @code
 * #define AESXYZCCXXXX_STATUS_ERROR0    AES_STATUS_RESERVED - 0
 * #define AESXYZCCXXXX_STATUS_ERROR1    AES_STATUS_RESERVED - 1
 * #define AESXYZCCXXXX_STATUS_ERROR2    AES_STATUS_RESERVED - 2
 * @endcode
 */
#define AES_STATUS_RESERVED ((int_fast16_t)-32)

/*!
 *  @brief AES Global configuration
 *
 *  The #AESCommon_Config structure contains a set of pointers used to
 *  characterize the AES driver implementation.
 */
typedef struct
{
    /*! Pointer to a driver specific data object */
    void *object;

    /*! Pointer to a driver specific hardware attributes structure */
    void const *hwAttrs;
} AESCommon_Config;

/*!
 * @brief   The return behavior of AES functions
 *
 * Not all AES operations exhibit the specified return behavior. Functions
 * that do not require significant computation and cannot offload that
 * computation to a background thread behave like regular functions.
 * Which functions exhibit the specified return behavior is not
 * implementation dependent. Specifically, a software-backed implementation
 * run on the same CPU as the application will emulate the return behavior
 * while not actually offloading the computation to the background thread.
 *
 * AES functions exhibiting the specified return behavior have restrictions
 * on the context from which they may be called.
 *
 * |                             | Task  | Hwi   | Swi   |
 * |-----------------------------|-------|-------|-------|
 * |AES_RETURN_BEHAVIOR_CALLBACK | X     | X     | X     |
 * |AES_RETURN_BEHAVIOR_BLOCKING | X     |       |       |
 * |AES_RETURN_BEHAVIOR_POLLING  | X     | X     | X     |
 *
 */
typedef enum
{
    AES_RETURN_BEHAVIOR_CALLBACK = 1, /*!< The function call will return immediately while the
                                       * operation goes on in the background. The registered
                                       * callback function is called after the operation completes.
                                       * The context the callback function is called (task, HWI, SWI)
                                       * is implementation-dependent.
                                       */
    AES_RETURN_BEHAVIOR_BLOCKING = 2, /*!< The function call will block while the operation goes
                                       * on in the background. Operation results are available
                                       * after the function returns.
                                       */
    AES_RETURN_BEHAVIOR_POLLING  = 4, /*!< The function call will continuously poll a flag while
                                       * operation goes on in the background. Operation results
                                       * are available after the function returns.
                                       */
} AES_ReturnBehavior;

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_AESCommon__include */
