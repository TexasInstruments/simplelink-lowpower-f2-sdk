/*
 * Copyright (c) 2021-2024, Texas Instruments Incorporated
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
 *  @file       RNG.h
 *
 *  @brief      RNG driver header
 *
 *  @anchor ti_drivers_RNG_Overview
 *  # Overview #
 *  The Random Number Generator (RNG) module generates random data of variable
 *  lengths from a pool of entropy. The pool of entropy is maintained by the
 *  driver using implementation-specific sources of entropy.
 *  The output is suitable for applications requiring cryptographically
 *  random data such as keying material for private or symmetric keys.
 *
 *  The RNG driver for CC27XX devices is strictly a CRNG/HSM implementation only.
 *
 *  CRNG (Cryptographically Secure Pseudo Random Number) a DRBG method alternative to
 *  True Random Number (TRNG)
 *
 *  HSM (Hardware Security Module) is a HW IP used for RNG operations
 *
 *  For CC27XX devices only, the RNG driver accepts two types of cryptoKey encoding
 *      - CryptoKey_BLANK_PLAINTEXT
 *      - CryptoKey_BLANK_PLAINTEXT_HSM
 *
 *  @anchor ti_drivers_RNG_Usage
 *  # Usage #
 *
 *  ## Initialization ##
 *  Unlike most drivers, there is a global instance of RNG driver data
 *  that is always available once #RNG_init() is called. This data will contain
 *  the entropy pool and any needed state information required to refill the
 *  pool. #RNG_init() should be called once before using other RNG
 *  driver APIs.
 *
 *  @note Some implementations restrict when RNG_init() may be called.
 *        Check the implementation's documentation for more information.
 *
 *  For CC23X0, RNG must be initialized by application in a task context with interrupts enabled
 *  using the following steps prior to the use of the Radio because CC23X0 uses the ADC samples
 *  from radio as noise that is conditioned using CBC MAC to generate the seed for RNG driver
 *
 *  ### Step 1: Required header file ###
 *
 *  @code
 *
 *   #include <ti/drivers/rng/RNGLPF3RF.h> // required for external syscfg variable RNGLPF3RF_noiseInputWordLen
 *
 *  @endcode
 *
 *  ### Step 2: External APIs ###
 *
 *  @code
 *
 *  // Use the function provided by RCL to read noise input //
 *  extern int_fast16_t RCL_AdcNoise_get_samples_blocking(uint32_t *buffer, uint32_t numWords);
 *
 *  @endcode
 *
 *  ### Step 3: Read noise input from RCL using RCL_AdcNoise_get_samples_blocking() ###
 *
 *  @code
 *
 *  int_fast16_t rclStatus, result;

 *  // User's global array for noise input based on size provided in syscfg //
 *  uint32_t localNoiseInput[]; //Minimum array size 80 words
 *
 *   // Clear noise input //
 *  memset(localNoiseInput, 0, sizeof(localNoiseInput));
 *
 *  // Fill noise input from RCL //
 *  //RNGLPF3RF_noiseInputWordLen is external variable from RNGLPF3RF.h
 *   rclStatus = RCL_AdcNoise_get_samples_blocking(localNoiseInput, RNGLPF3RF_noiseInputWordLen);
 *
 *  if (rclStatus != 0)
 *  {
 *      //Handle error;
 *  }
 *
 *  // Initialize the RNG driver noise input pointer with global noise input array from user //
 *  result = RNGLPF3RF_conditionNoiseToGenerateSeed(localNoiseInput);
 *  if ( rclStatus != 0)
 *  {
 *      //Handle error;
 *  }
 *
 *  @endcode
 *
 *
 *  ## Before starting a RNG operation ##
 *
 *  Before starting a RNG operation, the application must do the following:
 *      - Call RNG_init() to initialize the driver's global instance data.
 *      - Call RNG_Params_init() to initialize the RNG_Params to default values.
 *      - Modify the RNG_Params as desired.
 *      - Call RNG_open() to open an instance of the driver.
 *
 *  @note Some implementations restrict when RNG_init() may be called.
 *        Check the implementation's documentation for more information.
 *
 *  ## Entropy Pool Management ##
 *
 *  At any time after calling RNG_init(), the application may call
 *  RNG_fillPoolIfLessThan() to add entropy to the pool which will then make
 *  future requests for entropy execute faster. Note that the driver never
 *  automatically refills the pool. However, if the pool is empty, the RNG
 *  driver will still generate entropy upon request (for example when
 *  RNG_getRandomBits() is called).
 *
 *  The application is responsible for deciding when it is appropriate to
 *  spend the time and energy to refill the pool. One suggested location
 *  to do so is the idle thread.
 *
 *  ## RNG operations ##
 *
 *  Use RNG_getRandomBits() to obtain random bits from the entropy pool and
 *  copy them to a buffer/array. The caller must allocate memory sufficient
 *  to hold at least the number of bits of random data requested.
 *
 *  ## After the RNG operation completes ##
 *
 *  After the RNG operation completes, the application should either start
 *  another operation or close the driver by calling RNG_close(). Note that the
 *  singleton instance of the driver, along with its associated pool of entropy
 *  will still exist and will be used by any future RNG_open() calls. Note that
 *  closing the driver instance may not be strictly required, but is good
 *  practice.
 *
 *  ## Security ##
 *
 *  ### Data Protection ###
 *
 *  The entropy pool and any required state to generate more entropy is
 *  maintained in memory, in the driver's global instance data. The entirety of
 *  this data is stored in two global variables called RNG_instanceData and
 *  RNG_instancePool. It is up to the system to provide adequate
 *  protection (primarily confidentiality and integrity) of these in-memory
 *  assets.
 *
 *  ### Timing Side Channels ###
 *
 *  Functions which provide for generation of a value within a range use
 *  an algorithm which is timing-constant when the following parameters
 *  are held constant: lowerLimit, upperLimit, bitLength,
 *  and endianess. Thus, while the driver may create multiple candidates for the
 *  value to find one within the range, timing will not leak the final
 *  value's relation to the limits. However, timing may leak the bitLength,
 *  the endianess, and the use of #CryptoUtils_limitZero, #CryptoUtils_limitOne,
 *  or NULL for the limit values.
 *
 *  @anchor ti_drivers_RNG_Synopsis
 *  ## Synopsis
 *  @anchor ti_drivers_RNG_Synopsis_Code
 *  ### Generate random bytes to a user provided buffer #
 *
 *  @code
 *
 *  #include <ti/drivers/RNG.h>
 *  #include "ti_drivers_config.h"
 *
 *  // Setup RNG
 *  RNG_Init();
 *  RNG_fillPoolIfLessThan(RNG_POOL_BYTE_SIZE);
 *
 *  // Use RNG
 *  #define RANDOM_BYTES_SIZE 16u
 *  RNG_Handle handle;
 *  int_fast16_t result;
 *
 *  uint8_t randomBytesArray[RANDOM_BYTES_SIZE] = {0};
 *
 *  handle = RNG_open(0, NULL);
 *
 *  if (!handle) {
 *      // Handle error
 *      while(1);
 *  }
 *
 *  result = RNG_getRandomBits(handle, randomBytesArray, RANDOM_BYTES_SIZE * 8);
 *
 *  if (result != RNG_STATUS_SUCCESS) {
 *      // Handle error
 *      while(1);
 *  }
 *
 *  RNG_close(handle);
 *
 *  // Refill RNG Pool when convenient
 *  RNG_fillPoolIfLessThan(RNG_POOL_BYTE_SIZE);
 *  @endcode
 *
 *  @anchor ti_drivers_RNG_Examples
 *  ## Examples
 *
 *  The following examples do not show the process of initializing the RNG
 *  module and refilling the pool.
 *  See @ref ti_drivers_RNG_Synopsis RNG Driver Synopsis for an example
 *  showing those parts of RNG operation. *
 *
 *  ### Generate a number within a range ###
 *
 *  @code
 *
 *  #include <ti/drivers/RNG.h>
 *
 *  #define RANDOM_BIT_SIZE  15u
 *  #define RANDOM_BYTE_SIZE ((RANDOM_BIT_SIZE + 7u)/8u)
 *
 *  RNG_Handle handle;
 *  int_fast16_t result;
 *
 *  uint8_t randomBytesArray[RANDOM_BYTES_SIZE] = {0};
 *  uint8_t upperLimit[RANDOM_BYTES_SIZE] = {0xA9, 0x61}; // 25,001, LE format
 *
 *  handle = RNG_open(0, NULL);
 *
 *  if (!handle) {
 *      // Handle error
 *      while(1);
 *  }
 *
 *  // Generate a number from 1 to 25,000 (inclusive)
 *  // Note that lowerLimit parameter is inclusive and upperLimit is
 *  // exclusive. Thus, upperLimit is set to 25,001.
 *  result = RNG_getLERandomNumberInRange(RNG_Handle handle, RNG_limitOne,
 *                                        upperLimit, randomBytesArray,
 *                                        RANDOM_BIT_SIZE);
 *
 *
 *  if (result != RNG_STATUS_SUCCESS) {
 *      // Handle error
 *      while(1);
 *  }
 *
 *  RNG_close(handle);
 *
 *  @endcode
 *
 *
 *  ### Generate an ECC private key ###
 *
 *  @code
 *
 *  #include <ti/drivers/RNG.h>
 *  #include <ti/drivers/cryptoutils/ecc/ECCParams.h>
 *
 *  // Values are chosen to generate a NIST 256 bit key.
 *  CryptoKey privateKey;
 *  uint8_t privateKeyingMaterial[NISTP256_PARAM_SIZE_BYTES];
 *  RNG_Handle handle;
 *  int_fast16_t result;
 *
 *  handle = RNG_open(0, NULL);
 *
 *  if (!handle) {
 *      // Handle error
 *      while(1);
 *  }
 *
 *  CryptoKeyPlaintext_initBlankKey(&privateKey, privateKeyingMaterial,
 *                                  ECCParams_NISTP256.length);
 *
 *  // Generate NIST 256 bit key in BE format.
 *  result = RNG_generateBEKeyInRange(RNG_Handle handle, RNG_limitOne,
 *                                    ECCParams_NISTP256.order, privateKey,
 *                                    256);
 *
 *
 *  if (result != RNG_STATUS_SUCCESS) {
 *      // Handle error
 *      while(1);
 *  }
 *
 *  RNG_close(handle);
 *
 *  @endcode
 *
 */

#ifndef ti_drivers_RNG__include
#define ti_drivers_RNG__include

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Common RNG status code reservation offset.
 * RNG driver implementations should offset status codes with
 * RNG_STATUS_RESERVED growing negatively.
 *
 * Example implementation specific status codes:
 * @code
 * #define RNGXYZ_STATUS_ERROR0    ((int_fast16_t) (RNG_STATUS_RESERVED - 0))
 * #define RNGXYZ_STATUS_ERROR1    ((int_fast16_t) (RNG_STATUS_RESERVED - 1))
 * #define RNGXYZ_STATUS_ERROR2    ((int_fast16_t) (RNG_STATUS_RESERVED - 2))
 * @endcode
 */
#define RNG_STATUS_RESERVED (-32)

/*!
 * @brief   Successful status code.
 *
 * Functions return RNG_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define RNG_STATUS_SUCCESS ((int_fast16_t)0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return RNG_STATUS_ERROR if the function was not executed
 * successfully.
 */
#define RNG_STATUS_ERROR ((int_fast16_t)-1)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * RNG driver implementations may have hardware or software limitations on how
 * many clients can simultaneously perform operations. This status code is
 * returned if the mutual exclusion mechanism signals that an operation cannot
 * currently be performed.
 */
#define RNG_STATUS_RESOURCE_UNAVAILABLE ((int_fast16_t)-2)

/*!
 * @brief   Operation failed due to invalid inputs.
 *
 * Functions return RNG_STATUS_INVALID_INPUTS if input validation fails.
 */
#define RNG_STATUS_INVALID_INPUTS ((int_fast16_t)-3)

/*!
 * @brief  The ongoing operation was canceled.
 */
#define RNG_STATUS_CANCELED ((int_fast16_t)-4)

/*!
 * @brief  The pool of entropy has been exhausted and additional entropy cannot
 *         be generated. A reset of the system may be required to generate more
 *         entropy.
 */
#define RNG_ENTROPY_EXHAUSTED ((int_fast16_t)-5)

/*!
 * @brief Some implementations have restrictions on how often or when
 *        RNG_init may be called. See the documentation for the implementation
 *        for more information.
 */
#define RNG_STATUS_INIT_NOT_ALLOWED ((int_fast16_t)-6)

/*!
 * @brief Some implementations require a noise input during initialization
 *        which is conditioned to seed the RNG driver. RNG_init() will return this
 *        error if the noise input pointer is not initialized with valid information.
 *        See documentation for the implementation for more information.
 */
#define RNG_STATUS_NOISE_INPUT_INVALID ((int_fast16_t)-7)

/*!
 * @brief RNG driver not initialized.
 *
 * @note Some implementations restrict when RNG_init() may be called.
 *       Refer to #RNG_init() for more information.
 */
#define RNG_STATUS_NOT_INITIALIZED ((int_fast16_t)-8)

/*!
 * @brief  Maximum number of bits that may be requested in a single call
 */
#define RNG_MAX_BIT_LENGTH ((size_t)1u << 20u) /* 1 MiB */

/*!
 *  @brief RNG Global configuration
 *
 *  The RNG_Config structure contains a set of pointers used to characterize
 *  the RNG driver implementation.
 *
 *  This structure needs to be defined before calling RNG_init() and it must
 *  not be changed thereafter.
 *
 *  @sa     RNG_init()
 */
typedef struct
{
    /*! Pointer to a driver specific data object */
    void *object;

    /*! Pointer to a driver specific hardware attributes structure */
    void const *hwAttrs;
} RNG_Config;

/*!
 *  @brief  A handle that is returned from a RNG_open() call.
 */
typedef const RNG_Config *RNG_Handle;

/*!
 * @brief   The way in which RNG function calls return after generating
 *          the requested entropy.
 *
 * Not all RNG implementations support all of the return behavior options.
 *
 * Not all RNG operations exhibit the specified return behavior. Functions that
 * do not require significant computation and cannot offload that computation to
 * a background thread behave like regular functions. Which functions exhibit
 * the specified return behavior is not implementation dependent. Specifically,
 * a software-backed implementation run on the same CPU as the application will
 * emulate the return behavior while not actually offloading the computation to
 * the background thread.
 *
 * RNG functions exhibiting the specified return behavior have restrictions on
 * the context from which they may be called.
 *
 * |                              | Task  | Hwi   | Swi   |
 * |------------------------------|-------|-------|-------|
 * | RNG_RETURN_BEHAVIOR_CALLBACK | X     |       |       |
 * | RNG_RETURN_BEHAVIOR_BLOCKING | X     |       |       |
 * | RNG_RETURN_BEHAVIOR_POLLING  | X     | X     | X     |
 *
 */
typedef enum
{
    RNG_RETURN_BEHAVIOR_CALLBACK = 1, /*!< The function call will return immediately while the
                                       *   RNG operation goes on in the background. The
                                       *   registered callback function is called after the
                                       *   operation completes. The context the callback
                                       *   function is called (task, HWI, SWI) is
                                       *   implementation-dependent.
                                       */
    RNG_RETURN_BEHAVIOR_BLOCKING = 2, /*!< The function call will block while RNG operation
                                       *   goes on in the background. RNG operation results
                                       *    are available after the function returns.
                                       */
    RNG_RETURN_BEHAVIOR_POLLING  = 4, /*!< The function call will continuously poll a flag
                                       *   while RNG operation goes on in the background. RNG
                                       *   operation results are available after the function
                                       *   returns.
                                       */
} RNG_ReturnBehavior;

/*!
 *  @brief  The definition of a callback function used by the RNG driver
 *          when RNG_generateKey(), RNG_generateLEKeyInRange(), or
 *          RNG_generateBEKeyInRange() is called with ::RNG_RETURN_BEHAVIOR_CALLBACK
 *
 *  @warning Attempting to make calls to the RNG driver from the callback
 *           may result in deadlock.
 *
 *  @param  handle  Handle of the client that started the RNG operation.
 *
 *  @param  returnValue Return status code describing the outcome of the operation.
 *
 *  @param  key     The CryptoKey that describes the location the generated
 *                      entropy will be copied to.
 */
typedef void (*RNG_CryptoKeyCallbackFxn)(RNG_Handle handle, int_fast16_t returnValue, CryptoKey *key);

/*!
 *  @brief  The definition of a callback function used by the RNG driver
 *          when RNG_getRandomBits(), RNG_getLERandomNumberInRange(),
 *          or RNG_getBERandomNumberInRange is called with
 *          ::RNG_RETURN_BEHAVIOR_CALLBACK
 *
 *  @warning Attempting to make calls to the RNG driver from the callback
 *           may result in deadlock.
 *
 *  @param  handle  Handle of the client that started the RNG operation.
 *
 *  @param  returnValue Return status code describing the outcome of the operation.
 *
 *  @param  randomBits Pointer to an array that stores the random bits
 *                      output by this function.
 *
 *  @param  randomBitsLength The length of the random data generated, in bits.
 */
typedef void (*RNG_RandomBitsCallbackFxn)(RNG_Handle handle,
                                          int_fast16_t returnValue,
                                          uint8_t *randomBits,
                                          size_t randomBitsLength);

/*!
 *  @brief  RNG Parameters
 *
 *  RNG Parameters are used with the RNG_open() call. Default values for
 *  these parameters are set using RNG_Params_init().
 *
 *  @attention When using the driver in #RNG_RETURN_BEHAVIOR_CALLBACK,
 *             set the appropriate callback function field to point to a
 *             valid callback function and set the other one to NULL.
 *
 *  @sa     RNG_Params_init()
 */
typedef struct
{
    RNG_ReturnBehavior returnBehavior;               /*!< Blocking, callback, or polling return behavior */
    RNG_CryptoKeyCallbackFxn cryptoKeyCallbackFxn;   /*!< Callback function to use with RNG_generateKey()
                                                      *   and RNG_generateKeyInRange().
                                                      *   Set randomBitsCallbackFxn to NULL if using this.
                                                      */
    RNG_RandomBitsCallbackFxn randomBitsCallbackFxn; /*!< Callback function to use with RNG_getRandomBits(),
                                                      *   RNG_getLERandomNumberInRange(), and
                                                      *   RNG_getBERandomNumberInRange().
                                                      *   Set cryptoKeyCallbackFxn to NULL if using this.
                                                      */
    uint32_t timeout;                                /*!< Timeout (in ClockP ticks) before the driver
                                                      *   returns an error in ::RNG_RETURN_BEHAVIOR_BLOCKING
                                                      */
} RNG_Params;

/*!
 *  @brief Default RNG_Params structure
 *
 *  @sa     RNG_Params_init()
 */
extern const RNG_Params RNG_defaultParams;

/*!
 *  @brief The byte size of the pool
 */
extern const size_t RNG_poolByteSize;

/*!
 *  @brief  This function initializes the RNG module.
 *
 *  @pre    The RNG_config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other RNG driver APIs. This function call does not modify any
 *          peripheral registers.
 *          For CC23X0, RNG must be initialized by application in a task context with interrupts enabled
 *          using the following steps prior to the use of the Radio because CC23X0 uses the ADC samples
 *          from radio as noise that is conditioned using CBC MAC
 *          to generate the seed for RNG driver
 *          1. Read radio noise using RCL_AdcNoise_get_samples_blocking(). This RCL function must
 *             be called from a task context with interrupts enabled and therefore cannot be called
 *             by startup code. This must be executed prior to the use of the radio.
 *          2. Condition the noise to seed the RNG using RNGLPF3RF_conditionNoiseToGenerateSeed().
 *          3. Initialize the RNG from the application with RNG_init()
 *
 *  @retval #RNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #RNG_STATUS_ERROR                 The operation failed.
 *  @retval #RNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was
 *                                            not available. Try again later.
 */
int_fast16_t RNG_init(void);

/*!
 *  @brief  Fills the pool with entropy if the number of bytes with entropy in
 *          the pool is less than the value specified.
 *
 *  @note   This function does not take in a handle and the implementation
 *          may run in either blocking or polling mode.
 *
 *  @pre    RNG_init() has to be called first.
 *
 *  @param  bytes Pool will be filled if current level is less than this number.
 *                Use RNG_POOL_BYTE_SIZE (from ti_drivers_config.h) to always fill.
 *
 *  @retval #RNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #RNG_STATUS_ERROR                 The operation failed.
 *  @retval #RNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was
 *                                            not available. Try again later.
 *  @retval #RNG_ENTROPY_EXHAUSTED            Pool could not be refilled, device
 *                                            may need reset.
 *  @retval #RNG_STATUS_NOT_INITIALIZED       RNG not initialized.
 */
int_fast16_t RNG_fillPoolIfLessThan(size_t bytes);

/*!
 *  @brief  Function to initialize the RNG_Params struct to its defaults
 *
 *  @param  params      An pointer to RNG_Params structure for
 *                      initialization
 *
 *  Default values are:    <br>
 *      returnBehavior              = RNG_RETURN_BEHAVIOR_BLOCKING  <br>
 *      cryptoKeyCallbackFxn        = NULL                          <br>
 *      randomBitsCallbackFxn       = NULL                          <br>
 *      timeout                     = SemaphoreP_WAIT_FOREVER       <br>
 *      custom                      = NULL                          <br>
 */
void RNG_Params_init(RNG_Params *params);

/*!
 *  @brief  This function opens a given RNG peripheral.
 *
 *  @pre    RNG controller has been initialized using RNG_init()
 *
 *  @param  index         Logical peripheral number for the RNG indexed into
 *                        the RNG_config table
 *
 *  @param  params        Pointer to an parameter block, if NULL it will use
 *                        default values.
 *
 *  @return A RNG_Handle on success or a NULL on an error or if it has been
 *          opened already.
 *
 *  @sa     RNG_init()
 *  @sa     RNG_close()
 */
RNG_Handle RNG_open(uint_least8_t index, const RNG_Params *params);

/*!
 *  @brief  Function to close a RNG peripheral specified by the RNG handle
 *
 *  @pre    RNG_open() has to be called first.
 *
 *  @param  handle A RNG handle returned from RNG_open()
 *
 *  @sa     RNG_open()
 */
void RNG_close(RNG_Handle handle);

/*!
 *  @brief  Generate random bits and output to the given array.
 *
 *  Generates random a random number of bits with length of \c randomBitsLength.
 *  The output length in bytes will be the minimum number of bytes needed
 *  to contain \c randomBitsLength. The output will be placed at the address
 *  pointed to by \c randomBits. The user shall be responsible for allocating
 *  sufficient memory starting at the address pointed at by \c randomBits to
 *  hold the number of bytes output.
 *
 *  @attention When called with ::RNG_RETURN_BEHAVIOR_CALLBACK, provide a
 *             callback function of type #RNG_RandomBitsCallbackFxn.
 *
 *  @note See #RNG_generateKey() to output random bytes to a \c CryptoKey instead.
 *
 *  @pre    RNG_open() has to be called first.
 *
 *  @param  handle A RNG handle returned from RNG_open().
 *
 *  @param  randomBits Pointer to an array that stores the random bits
 *                     output by this function.
 *
 *  @param  randomBitsLength The length of the random data required, in bits.
 *                           A maximum of 1MiB is allowed.
 *
 *  @sa RNG_getLERandomNumberInRange
 *  @sa RNG_getBERandomNumberInRange
 *
 *  @retval #RNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #RNG_STATUS_ERROR                 The operation failed.
 *  @retval #RNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was
 *                                            not available. Try again later.
 *  @retval #RNG_STATUS_INVALID_INPUTS        Inputs provided are not valid.
 *  @retval #RNG_ENTROPY_EXHAUSTED            Requested number of bytes could
 *                                            not be obtained. Device may need reset.
 *  @retval #RNG_STATUS_NOT_INITIALIZED       RNG not initialized.
 */
int_fast16_t RNG_getRandomBits(RNG_Handle handle, void *randomBits, size_t randomBitsLength);

/*!
 *  @brief  Generate random number, stored in little-endian (LE) format, where
 *          the number is within the specified range.
 *
 *  Generates random a random number within the range [lowerLimit, upperLimit)
 *  of bit size \c randomNumberBitLength. The output length in bytes will be the
 *  minimum number of bytes needed to contain \c randomNumberBitLength. The
 *  output will be placed at the address pointed to by \c randomNumber. The user
 *  shall be responsible for allocating sufficient memory starting at the
 *  address pointed at by \c randomNumber to hold the number of bytes output.
 *
 *  Note that the special values of #CryptoUtils_limitZero and
 *  #CryptoUtils_limitOne are available to pass in for the \c lowerLimit.
 *  (These values can also be used for the \c upperLimit but their use for the
 *  upperLimit has no practical use.)
 *
 *  If \c lowerLimit is NULL then the lower limit is taken as 0.
 *  If \c upperLimit is NULL then the upper limit is taken as
 *  2<sup>(\c bitLength + 1)</sup>.
 *
 *  @attention When called with ::RNG_RETURN_BEHAVIOR_CALLBACK, provide a callback
 *              function of type #RNG_RandomBitsCallbackFxn.
 *
 *  @note See #RNG_generateLEKeyInRange() to output a ranged number to a \c CryptoKey instead.
 *
 *  @pre    RNG_open() has to be called first.
 *
 *  @param  handle A RNG handle returned from RNG_open().
 *
 *  @param  lowerLimit Pointer to an array that stores the lower limit (inclusive)
 *                     in LE format for the generated number.
 *
 *  @param  upperLimit Pointer to an array that stores the upper limit (exclusive)
 *                     in LE format for the generated number.
 *
 *  @param  randomNumber Pointer to an array that stores the random number
 *                       output by this function.
 *
 *  @param  randomNumberBitLength The length, in bits, of both the limit values
 *                                and the random number to be generated.
 *
 *  @sa     CryptoUtils_limitZero
 *  @sa     CryptoUtils_limitOne
 *
 *  @retval #RNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #RNG_STATUS_ERROR                 The operation failed.
 *  @retval #RNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was
 *                                            not available. Try again later.
 *  @retval #RNG_STATUS_INVALID_INPUTS        Inputs provided are not valid.
 *  @retval #RNG_ENTROPY_EXHAUSTED            Requested number of bytes could
 *                                            not be obtained. Device may need reset.
 *  @retval #RNG_STATUS_NOT_INITIALIZED       RNG not initialized.
 */
int_fast16_t RNG_getLERandomNumberInRange(RNG_Handle handle,
                                          const void *lowerLimit,
                                          const void *upperLimit,
                                          void *randomNumber,
                                          size_t randomNumberBitLength);

/*!
 *  @brief  Generate random number, stored in big-endian (BE) format, where
 *          the number is within the specified range.
 *
 *  Generates random a random number within the range [lowerLimit, upperLimit)
 *  of bit size \c randomNumberBitLength. The output length in bytes will be the
 *  minimum number of bytes needed to contain \c randomNumberBitLength. The
 *  output will be placed at the address pointed to by \c randomNumber. The user
 *  shall be responsible for allocating sufficient memory starting at the address
 *  pointed at by \c randomNumber to hold the number of bytes output.
 *
 *  Note that the special values of #CryptoUtils_limitZero and
 *  #CryptoUtils_limitOne are available to pass in for the \c lowerLimit.
 *  (These values can also be used for the \c upperLimit but their use for the
 *  upperLimit has no practical use.)
 *
 *  If \c lowerLimit is NULL then the lower limit is taken as 0.
 *  If \c upperLimit is NULL then the upper limit is taken as
 *  2<sup>(\c bitLength + 1)</sup>.
 *
 *  @attention When called with ::RNG_RETURN_BEHAVIOR_CALLBACK, provide a callback
 *              function of type #RNG_RandomBitsCallbackFxn.
 *
 *  @note See #RNG_generateBEKeyInRange() to output a ranged number to a \c CryptoKey instead.
 *
 *  @pre    RNG_open() has to be called first.
 *
 *  @param  handle A RNG handle returned from RNG_open().
 *
 *  @param  lowerLimit Pointer to an array that stores the lower limit (inclusive)
 *                     in BE format for the generated number.
 *
 *  @param  upperLimit Pointer to an array that stores the upper limit (exclusive)
 *                     in BE format for the generated number.
 *
 *  @param  randomNumber Pointer to an array that stores the random number
 *                       output by this function.
 *
 *  @param  randomNumberBitLength The length, in bits, of both the limit value
 *                                and the random number to be generated.
 *
 *  @sa     CryptoUtils_limitZero
 *  @sa     CryptoUtils_limitOne
 *
 *  @retval #RNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #RNG_STATUS_ERROR                 The operation failed.
 *  @retval #RNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was
 *                                            not available. Try again later.
 *  @retval #RNG_STATUS_INVALID_INPUTS        Inputs provided are not valid.
 *  @retval #RNG_ENTROPY_EXHAUSTED            Requested number of bytes could
 *                                            not be obtained. Device may need reset.
 *  @retval #RNG_STATUS_NOT_INITIALIZED       RNG not initialized.
 */
int_fast16_t RNG_getBERandomNumberInRange(RNG_Handle handle,
                                          const void *lowerLimit,
                                          const void *upperLimit,
                                          void *randomNumber,
                                          size_t randomNumberBitLength);

/*!
 *  @brief  Generate random bits and output them to the given \c CryptoKey object.
 *
 *  Generates a random bitstream of the size defined in the \c key
 *  CryptoKey in the range 0 <= \c key buffer < 2 ^ (entropy length * 8).
 *  The entropy will be generated and stored according to the storage
 *  requirements defined in the CryptoKey. The length of the entropy
 *  generated will be the same as the key length.
 *
 *  @attention When called with ::RNG_RETURN_BEHAVIOR_CALLBACK, provide a
 *             callback function of type #RNG_CryptoKeyCallbackFxn.
 *
 *  @pre    RNG_open() has to be called first.
 *
 *  @param  handle A RNG handle returned from RNG_open().
 *
 *  @param  key Pointer to a blank CryptoKey, initialized with a length and
 *              appropriate storage for storing a key of the specified length.
 *
 *  @sa RNG_generateLEKeyInRange
 *  @sa RNG_generateBEKeyInRange
 *
 *  @retval #RNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #RNG_STATUS_ERROR                 The operation failed.
 *  @retval #RNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was
 *                                            not available. Try again later.
 *  @retval #RNG_STATUS_INVALID_INPUTS        Inputs provided are not valid.
 *  @retval #RNG_ENTROPY_EXHAUSTED            Requested number of bytes could
 *                                            not be obtained. Device may need reset.
 *  @retval #RNG_STATUS_NOT_INITIALIZED       RNG not initialized.
 */
int_fast16_t RNG_generateKey(RNG_Handle handle, CryptoKey *key);

/*!
 *  @brief  Generate random number, in little-endian (LE) format, where the
 *          number is within the specified range. Store the random number in
 *          the given \c CryptoKey object.
 *
 *  Generates a random number within the range [lowerLimit, upperLimit) of bit
 *  size \c randomNumberBitLength. The output length in bytes will be the minimum
 *  number of bytes needed to contain \c randomNumberBitLength. The output will
 *  be placed as specified by the members of \c key.
 *
 *  Note that the special values of #CryptoUtils_limitZero and
 *  #CryptoUtils_limitOne are available to pass in for the \c lowerLimit.
 *  (These values can also be used for the \c upperLimit but their use for the
 *  upperLimit has no practical use.)
 *
 *  If \c lowerLimit is NULL then the lower limit is taken as 0.
 *  If \c upperLimit is NULL then the upper limit is taken as
 *  2<sup>(\c bitLength + 1)</sup>.
 *
 *  @attention When called with ::RNG_RETURN_BEHAVIOR_CALLBACK, provide a callback
 *              function of type #RNG_CryptoKeyCallbackFxn.
 *
 *  @pre    RNG_open() has to be called first.
 *
 *  @param  handle A RNG handle returned from RNG_open().
 *
 *  @param  lowerLimit Pointer to an array that stores the lower limit (inclusive)
 *                     in LE format for the generated number.
 *
 *  @param  upperLimit Pointer to an array that stores the upper limit (exclusive)
 *                     in LE format for the generated number.
 *
 *  @param  key Pointer to a blank CryptoKey, initialized with a length and
 *              appropriate storage for storing a key of the specified length.
 *
 *  @param  randomNumberBitLength The length, in bits, of both the limit values
 *                                and the random number to be generated.
 *
 *  @sa     CryptoUtils_limitZero
 *  @sa     CryptoUtils_limitOne
 *
 *  @retval #RNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #RNG_STATUS_ERROR                 The operation failed.
 *  @retval #RNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was
 *                                            not available. Try again later.
 *  @retval #RNG_STATUS_INVALID_INPUTS        Inputs provided are not valid.
 *  @retval #RNG_ENTROPY_EXHAUSTED            Requested number of bytes could
 *                                            not be obtained. Device may need reset.
 *  @retval #RNG_STATUS_NOT_INITIALIZED       RNG not initialized.
 */
int_fast16_t RNG_generateLEKeyInRange(RNG_Handle handle,
                                      const void *lowerLimit,
                                      const void *upperLimit,
                                      CryptoKey *key,
                                      size_t randomNumberBitLength);

/*!
 *  @brief  Generate random number, stored in big-endian (BE) format, where the
 *          number is within the specified range. Store the random number in
 *          the given \c CryptoKey object.
 *
 *  Generates a random number within the range [lowerLimit, upperLimit) of bit
 *  size \c randomNumberBitLength. The output length in bytes will be the
 *  minimum number of bytes needed to contain \c randomNumberBitLength. The
 *  output will be placed as specified by the members of \c key.
 *
 *  Note that the special values of #CryptoUtils_limitZero and
 *  #CryptoUtils_limitOne are available to pass in for the \c lowerLimit.
 *  (These values can also be used for the \c upperLimit but their use for the
 *  upperLimit has no practical use.)
 *
 *  If \c lowerLimit is NULL then the lower limit is taken as 0.
 *  If \c upperLimit is NULL then the upper limit is taken as
 *  2<sup>(\c bitLength + 1)</sup>.
 *
 *  @attention When called with ::RNG_RETURN_BEHAVIOR_CALLBACK, provide a callback
 *              function of type #RNG_CryptoKeyCallbackFxn.
 *
 *  @pre    RNG_open() has to be called first.
 *
 *  @param  handle A RNG handle returned from RNG_open().
 *
 *  @param  lowerLimit Pointer to an array that stores the lower limit (inclusive)
 *                     in BE format for the generated number.
 *
 *  @param  upperLimit Pointer to an array that stores the upper limit (exclusive)
 *                     in BE format for the generated number.
 *
 *  @param  key Pointer to a blank CryptoKey, initialized with a length and
 *              appropriate storage for storing a key of the specified length.
 *
 *  @param  randomNumberBitLength The length, in bits, of both the limit values
 *                                and the random number to be generated.
 *
 *  @sa     CryptoUtils_limitZero
 *  @sa     CryptoUtils_limitOne
 *
 *  @retval #RNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #RNG_STATUS_ERROR                 The operation failed.
 *  @retval #RNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was
 *                                            not available. Try again later.
 *  @retval #RNG_STATUS_INVALID_INPUTS        Inputs provided are not valid.
 *  @retval #RNG_ENTROPY_EXHAUSTED            Requested number of bytes could
 *                                            not be obtained. Device may need reset.
 *  @retval #RNG_STATUS_NOT_INITIALIZED       RNG not initialized.
 */
int_fast16_t RNG_generateBEKeyInRange(RNG_Handle handle,
                                      const void *lowerLimit,
                                      const void *upperLimit,
                                      CryptoKey *key,
                                      size_t randomNumberBitLength);

/**
 *  @brief  Constructs a new RNG object
 *
 *  Unlike #RNG_open(), #RNG_construct() does not require the hwAttrs and
 *  object to be allocated in a #RNG_Config array that is indexed into.
 *  Instead, the #RNG_Config, hwAttrs, and object can be allocated at any
 *  location. This allows for relatively simple run-time allocation of temporary
 *  driver instances on the stack or the heap.
 *  The drawback is that this makes it more difficult to write device-agnostic
 *  code. If you use an ifdef with DeviceFamily, you can choose the correct
 *  object and hwAttrs to allocate. That compilation unit will be tied to the
 *  device it was compiled for at this point. To change devices, recompilation
 *  of the application with a different DeviceFamily setting is necessary.
 *
 *  @param config #RNG_Config describing the location of the object and hwAttrs.
 *
 *  @param params #RNG_Params to configure the driver instance.
 *
 *  @return Returns a #RNG_Handle on success or NULL on failure.
 *
 *  @pre    The object struct @c config points to must be zeroed out prior to
 *          calling this function. Otherwise, unexpected behavior may ensue.
 */
RNG_Handle RNG_construct(const RNG_Config *config, const RNG_Params *params);

/*!
 *  @brief Aborts an ongoing RNG operation and clears internal buffers.
 *
 *  Aborts an operation to generate random bytes/entropy. The operation will
 *  terminate as though an error occurred and the status code of the operation
 *  will be #RNG_STATUS_CANCELED in this case.
 *
 *  Any entropy already copied out of the pool will have already been removed
 *  from the pool and will not be reused for later requests.
 *
 *  Canceling an operation may be delayed if the entropy pool is below its
 *  minimum fill mark as the driver will refill the pool before finishing
 *  the cancelled operation.
 *
 *  @param  handle A #RNG_Handle returned from #RNG_open()
 *
 *  @retval #RNG_STATUS_SUCCESS    The operation was canceled or there was no
 *                                 operation in progress to be canceled.
 */
int_fast16_t RNG_cancelOperation(RNG_Handle handle);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_RNG__include */
