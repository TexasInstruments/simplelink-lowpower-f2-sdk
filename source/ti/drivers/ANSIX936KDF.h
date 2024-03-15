/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
/*!****************************************************************************
 *  @file       ANSIX936KDF.h
 *
 *  @brief      ANSIX936KDF driver header
 *
 *  @anchor ti_drivers_ANSIX936KDF_Overview
 *  # Overview #
 *
 *  The ANSI X9.63 Key Derivation Function (KDF) driver utilizes the SHA-256 hash
 *  function to derive a key from a shared secret value and optional shared
 *  info. See ANSI X9.63-2011 or SEC-1 v2.0 standard for more information.
 *
 *  @anchor ti_drivers_ANSIX936KDF_Usage
 *  # Usage #
 *
 *  Before starting a ANSIX936KDF operation, the application must do the following:
 *      - Call #ANSIX936KDF_init() to initialize the driver.
 *      - Call #ANSIX936KDF_Params_init() to initialize the ANSIX936KDF_Params to default values.
 *      - Modify the #ANSIX936KDF_Params as desired.
 *      - Call #ANSIX936KDF_open() to open an instance of the driver.
 *
 *  @anchor ti_drivers_ANSIX936KDF_Synopsis
 *  # Synopsis
 *
 *  @anchor ti_drivers_ANSIX936KDF_Synopsis_Code
 *  @code
 *
 *  // Import ANSIX936KDF Driver definitions
 *  #include <ti/drivers/ANSIX936KDF.h>
 *
 *  // Import driver configuration
 *  #include "ti_drivers_config.h"
 *
 *  // Initialize driver
 *  ANSIX936KDF_init();
 *
 *  // Open driver instance
 *  handle = ANSIX936KDF_open(CONFIG_ANSIX936KDF_0, NULL);
 *
 *  // Perform key derivation
 *  result = ANSIX936KDF_deriveKey(handle, input, sizeof(input), sharedInfo,
 *                                 sizeof(sharedInfo), output, sizeof(output));
 *
 *  // Close driver instance
 *  ANSIX936KDF_close(handle);
 *  @endcode
 *
 *  @anchor ti_drivers_ANSIX936KDF_Example
 *  # Example #
 *
 *  The #ANSIX936KDF_deriveKey() function performs a key derivation operation in
 *  a single call.
 *
 *  After a ANSIX936KDF operation completes, the application may either start
 *  another operation or close the driver by calling #ANSIX936KDF_close().
 *
 *  @code
 *  ANSIX936KDF_Params params;
 *  ANSIX936KDF_Handle handle;
 *  int_fast16_t result;
 *  uint8_t sharedInfo[] = {0x75, 0xee, 0xf8, 0x1a, 0xa3, 0x04, 0x1e, 0x33,
 *                          0xb8, 0x09, 0x71, 0x20, 0x3d, 0x2c, 0x0c, 0x52};
 *  uint8_t input[] = {0x22, 0x51, 0x8b, 0x10, 0xe7, 0x0f, 0x2a, 0x3f, 0x24, 0x38, 0x10, 0xae,
 *                     0x32, 0x54, 0x13, 0x9e, 0xfb, 0xee, 0x04, 0xaa, 0x57, 0xc7, 0xaf, 0x7d};
 *  uint8_t output[32] = {0};
 *
 *  ANSIX936KDF_init();
 *
 *  ANSIX936KDF_Params_init(&params);
 *  params.returnBehavior = ANSIX936KDF_RETURN_BEHAVIOR_POLLING;
 *
 *  handle = ANSIX936KDF_open(CONFIG_ANSIX936KDF_0, &params);
 *  assert(handle != NULL);
 *
 *  result = ANSIX936KDF_deriveKey(handle, input, sizeof(input), sharedInfo,
 *                                 sizeof(sharedInfo), output, sizeof(output));
 *  assert(result == ANSIX936KDF_STATUS_SUCCESS);
 *
 *  ANSIX936KDF_close(handle);
 *  @endcode
 */

#ifndef ti_drivers_ANSIX936KDF__include
#define ti_drivers_ANSIX936KDF__include

#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/SHA2.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  Common ANSIX936KDF status code reservation offset.
 *  ANSIX936KDF driver implementations should offset status codes with
 *  ANSIX936KDF_STATUS_RESERVED growing negatively.
 *
 *  Example implementation specific status codes:
 *  @code
 *  #define ANSIX936KDFXYZ_STATUS_ERROR0    ANSIX936KDF_STATUS_RESERVED - 0
 *  #define ANSIX936KDFXYZ_STATUS_ERROR1    ANSIX936KDF_STATUS_RESERVED - 1
 *  #define ANSIX936KDFXYZ_STATUS_ERROR2    ANSIX936KDF_STATUS_RESERVED - 2
 *  @endcode
 */
#define ANSIX936KDF_STATUS_RESERVED ((int_fast16_t)-32)

/*!
 *  @brief  Successful status code.
 *
 *  Functions return ANSIX936KDF_STATUS_SUCCESS if the function was executed
 *  successfully.
 */
#define ANSIX936KDF_STATUS_SUCCESS ((int_fast16_t)0)

/*!
 *  @brief  Generic error status code.
 *
 *  Functions return ANSIX936KDF_STATUS_ERROR if the function was not executed
 *  successfully and no more specific error is applicable.
 */
#define ANSIX936KDF_STATUS_ERROR ((int_fast16_t)-1)

/*!
 *  @brief  An error status code returned if the hardware or software resource is
 *          currently unavailable.
 *
 *  ANSIX936KDF driver implementations may have hardware or software limitations
 *  on how many clients can simultaneously perform operations. This status code
 *  is returned if the mutual exclusion mechanism signals that an operation
 *  cannot currently be performed.
 */
#define ANSIX936KDF_STATUS_RESOURCE_UNAVAILABLE ((int_fast16_t)-2)

/*!
 *  @brief  The way in which ANSIX936KDF function calls return after performing
 *          an operation.
 *
 *  Not all ANSIX936KDF operations exhibit the specified return behavior.
 *  Functions that do not require significant computation and cannot offload that
 *  computation to a background thread behave like regular functions. Which
 *  functions exhibit the specified return behavior is not implementation
 *  dependent. Specifically, a software-backed implementation run on the same CPU
 *  as the application will emulate the return behavior while not actually
 *  offloading the computation to the background thread.
 *
 *  ANSIX936KDF functions exhibiting the specified return behavior have
 *  restrictions on the context from which they may be called.
 *
 *  |                                       | Task  | Hwi   | Swi   |
 *  |---------------------------------------|-------|-------|-------|
 *  |ANSIX936KDF_RETURN_BEHAVIOR_BLOCKING   | X     |       |       |
 *  |ANSIX936KDF_RETURN_BEHAVIOR_POLLING    | X     | X     | X     |
 *
 */
typedef enum
{
    /*! The function call will block while the ANSIX936KDF operation goes
     *  on in the background. ANSIX936KDF operation results are available
     *  after the function returns.
     */
    ANSIX936KDF_RETURN_BEHAVIOR_BLOCKING = SHA2_RETURN_BEHAVIOR_BLOCKING,

    /*! The function call will continuously poll a flag while the ANSIX936KDF
     *  operation goes on in the background. ANSIX936KDF operation results
     *  are available after the function returns.
     */
    ANSIX936KDF_RETURN_BEHAVIOR_POLLING = SHA2_RETURN_BEHAVIOR_POLLING,
} ANSIX936KDF_ReturnBehavior;

/*!
 *  @brief  ANSIX936KDF Global configuration
 *
 *  The %ANSIX936KDF_Config structure contains a set of pointers used to characterize
 *  the ANSIX936KDF driver implementation.
 *
 *  This structure needs to be defined before calling #ANSIX936KDF_init() and it must
 *  not be changed thereafter.
 *
 *  @sa     ANSIX936KDF_init()
 */
typedef struct
{
    /*! Pointer to a driver specific data object */
    void *object;

    /*! Pointer to a driver specific hardware attributes structure */
    void const *hwAttrs;
} ANSIX936KDF_Config;

/*!
 *  @brief  A handle that is returned from an ANSIX936KDF_open() call.
 */
typedef ANSIX936KDF_Config *ANSIX936KDF_Handle;

/*!
 *  @brief  ANSIX936KDF Parameters
 *
 *  ANSIX936KDF Parameters are used to with the ANSIX936KDF_open() call. Default values for
 *  these parameters are set using ANSIX936KDF_Params_init().
 *
 *  @sa     ANSIX936KDF_Params_init()
 */
typedef struct
{
    ANSIX936KDF_ReturnBehavior returnBehavior; /*!< Blocking or polling return behavior */
    uint32_t timeout;                          /*!< Timeout before the driver returns an error in
                                                *   ::ANSIX936KDF_RETURN_BEHAVIOR_BLOCKING
                                                */
} ANSIX936KDF_Params;

/*!
 *  @brief Global ANSIX936KDF configuration struct.
 *
 *  Specifies context objects and hardware attributes for every
 *  driver instance.
 *
 *  This variable is supposed to be defined in the board file.
 */
extern const ANSIX936KDF_Config ANSIX936KDF_config[];

/*!
 *  @brief Global ANSIX936KDF configuration count.
 *
 *  Specifies the number of available ANSIX936KDF driver instances.
 *
 *  This variable is supposed to be defined in the board file.
 */
extern const uint_least8_t ANSIX936KDF_count;

/*!
 *  @brief  Default ANSIX936KDF_Params structure
 *
 *  @sa     #ANSIX936KDF_Params_init()
 */
extern const ANSIX936KDF_Params ANSIX936KDF_defaultParams;

/*!
 *  @brief  Initializes the ANSIX936KDF driver module.
 *
 *  @pre    The #ANSIX936KDF_config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other ANSIX936KDF driver APIs. This function call does not modify any
 *          peripheral registers.
 */
void ANSIX936KDF_init(void);

/*!
 *  @brief  Initializes @c params with default values.
 *
 *  @param  params      A pointer to #ANSIX936KDF_Params structure for
 *                      initialization
 *
 *  Default values:  <br>
 *      returnBehavior    = ANSIX936KDF_RETURN_BEHAVIOR_BLOCKING  <br>
 *      timeout           = SemaphoreP_WAIT_FOREVER               <br>
 */
void ANSIX936KDF_Params_init(ANSIX936KDF_Params *params);

/*!
 *  @brief  Initializes a ANSIX936KDF driver instance and returns a handle.
 *
 *  @pre    ANSIX936KDF controller has been initialized using #ANSIX936KDF_init()
 *
 *  @param  index         Logical peripheral number for the ANSIX936KDF indexed into
 *                        the #ANSIX936KDF_config table
 *
 *  @param  params        Pointer to a parameter block, if NULL it will use
 *                        default values.
 *
 *  @return A #ANSIX936KDF_Handle on success or a NULL on an error or if it has been
 *          opened already.
 *
 *  @sa     #ANSIX936KDF_init()
 *  @sa     #ANSIX936KDF_close()
 */
ANSIX936KDF_Handle ANSIX936KDF_open(uint_least8_t index, const ANSIX936KDF_Params *params);

/*!
 *  @brief  Closes a ANSIX936KDF peripheral specified by @c handle.
 *
 *  @pre    #ANSIX936KDF_open() has to be called first.
 *
 *  @param  handle A #ANSIX936KDF_Handle returned from ANSIX936KDF_open()
 *
 *  @sa     #ANSIX936KDF_open()
 */
void ANSIX936KDF_close(ANSIX936KDF_Handle handle);

/*!
 *  @brief  Derives a key with the SHA-256 hash function.
 *
 *  Uses the SHA-256 hash function to derives a key from a shared secret value
 *  and optional shared information.
 *
 *  @pre    #ANSIX936KDF_open() has to be called first.
 *
 *  @param  handle        A #ANSIX936KDF_Handle returned from #ANSIX936KDF_open()
 *
 *  @param  input         A pointer to the input (i.e. shared secret value)
 *
 *  @param  inputLen      The length of @c input in bytes.
 *
 *  @param  sharedInfo    A pointer to the shared info. May be NULL if there is no
 *                        shared info.
 *
 *  @param  sharedInfoLen The length of the @c sharedInfo in bytes.
 *                        Set to zero if @c sharedInfo is NULL.
 *
 *  @param  output        A pointer to the location to write the output (i.e. derived key).
 *
 *  @param  outputLen     Output length in bytes.
 *
 *  @retval #ANSIX936KDF_STATUS_SUCCESS               The operation succeeded.
 *  @retval #ANSIX936KDF_STATUS_ERROR                 The operation failed.
 *  @retval #ANSIX936KDF_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource
 *                                                    was not available. Try again
 *                                                    later.
 *
 *  @sa     #ANSIX936KDF_open()
 */
int_fast16_t ANSIX936KDF_deriveKey(ANSIX936KDF_Handle handle,
                                   const void *input,
                                   size_t inputLen,
                                   const void *sharedInfo,
                                   size_t sharedInfoLen,
                                   void *output,
                                   size_t outputLen);

/*!
 *  @brief  Constructs a new ANSIX936KDF object
 *
 *  Unlike #ANSIX936KDF_open(), #ANSIX936KDF_construct() does not require the hwAttrs and
 *  object to be allocated in a #ANSIX936KDF_Config array that is indexed into.
 *  Instead, the #ANSIX936KDF_Config, hwAttrs, and object can be allocated at any
 *  location. This allows for relatively simple run-time allocation of temporary
 *  driver instances on the stack or the heap.
 *  The drawback is that this makes it more difficult to write device-agnostic
 *  code. If you use an ifdef with DeviceFamily, you can choose the correct
 *  object and hwAttrs to allocate. That compilation unit will be tied to the
 *  device it was compiled for at this point. To change devices, recompilation
 *  of the application with a different DeviceFamily setting is necessary.
 *
 *  @param  config #ANSIX936KDF_Config describing the location of the object and hwAttrs.
 *
 *  @param  params #ANSIX936KDF_Params to configure the driver instance.
 *
 *  @return Returns a #ANSIX936KDF_Handle on success or NULL on failure.
 *
 *  @pre    The object struct @c config points to must be zeroed out prior to
 *          calling this function. Otherwise, unexpected behavior may ensue.
 */
ANSIX936KDF_Handle ANSIX936KDF_construct(ANSIX936KDF_Config *config, const ANSIX936KDF_Params *params);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_ANSIX936KDF__include */
