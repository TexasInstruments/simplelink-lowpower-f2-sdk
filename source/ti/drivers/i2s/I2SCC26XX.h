/*
 * Copyright (c) 2019-2023, Texas Instruments Incorporated
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
/*! ============================================================================
 *  @file       I2SCC26XX.h
 *
 *  @brief      I2S driver implementation for a CC26XX I2S controller
 *
 * # Limitations #
 *
 * ## Flash Memory range ##
 *
 * The I2S driver is unable to access flash memory in the address range
 * 0x0000 - 0x2000 on devices based on the Cortex M33+ core (CC26X3/CC26X4) due
 * to security constraints.
 *
 * ## Supported #I2S_MemoryLength values ##
 *  Only the following memory lengths are supported:
 *   - #I2S_MEMORY_LENGTH_16BITS
 *   - #I2S_MEMORY_LENGTH_24BITS
 *
 * ## #I2S_Params Attributes Limitations ##
 *
 * Some attributes in the #I2S_Params structure have a limited set of supported
 * values. These limitations are described below:
 *  - #I2S_Params.samplingFrequency
 *    - The SCK frequency resulting from the selected sampling frequency should
 *      be between 47 kHz and 4 MHz.
 *  - #I2S_Params.isDMAUnused
 *    - Must be false. All transmissions are always performed by the I2S module's
 *      own DMA.
 *  - #I2S_Params.isMSBFirst
 *    - Must be true. All samples are always transmitted MSB first.
 *  - #I2S_Params.memorySlotLength
 *    - Must be one of the suported #I2S_MemoryLength values listed in the above
 *      section.
 *  - #I2S_Params.fixedBufferLength
 *    - Additional restriction: Must be an even multiple of the number of bytes
 *      in a frame.
 *
 * <hr>
 * # Sample Buffers #
 *
 * This section describes the structure and requirements for the sample buffers
 * used in the #I2S_Transaction objects.
 *
 * Sample words are read from or written to the sample buffers in little-endian
 * byte order, meaning that the least significant byte (LSByte) is stored at the
 * lower byte address, and the most significant byte (MSByte) is stored at the
 * higher byte address.
 *
 * The sample buffers are divided into frames which are further subdivided into
 * channels, and if a channel is used by both SD0 and SD1 (where the direction
 * of the two pins are the same), then that channel is further subdivided into a
 * sample word for first SD0 and then SD1.
 *
 * ## Buffer Size Requirements ##
 *
 * The size of the buffers used in #I2S_Transaction objects must be an even
 * multiple of the number of bytes per frame. I.e. the number of bytes in the
 * buffers must be of the form: 2*n*k, where k is the size of a frame in bytes
 * and n is an integer satisfying n>=2. 2*n is the number of frames in the
 * buffer.
 *
 * ## General Sample Buffer Structure ##
 *
 * Below code describes the general structure of a sample buffer if SD0 and SD1
 * are configured to the same direction.
 *
 * @code
 * struct
 * {
 * #if SD0_USE_CHANNEL_0 || SD1_USE_CHANNEL_0
 *     struct
 *     {
 *   #if SD0_USE_CHANNEL_0
 *         uint8_t sd0SampleWord[BYTES_PER_WORD];
 *   #endif
 *   #if SD1_USE_CHANNEL_0
 *         uint8_t sd1SampleWord[BYTES_PER_WORD];
 *   #endif
 *     } channel0;
 * #endif
 * #if SD0_USE_CHANNEL_1 || SD1_USE_CHANNEL_1
 *     struct
 *     {
 *   #if SD0_USE_CHANNEL_1
 *         uint8_t sd0SampleWord[BYTES_PER_WORD];
 *   #endif
 *   #if SD1_USE_CHANNEL_1
 *         uint8_t sd1SampleWord[BYTES_PER_WORD];
 *   #endif
 *     } channel1;
 * #endif
 * // ...
 * #if SD0_USE_CHANNEL_8 || SD1_USE_CHANNEL_8
 *     struct
 *     {
 *   #if SD0_USE_CHANNEL_8
 *         uint8_t sd0SampleWord[BYTES_PER_WORD];
 *   #endif
 *   #if SD1_USE_CHANNEL_8
 *         uint8_t sd1SampleWord[BYTES_PER_WORD];
 *   #endif
 *     } channel8;
 * #endif
 * } sampleBufferFrames[FRAMES_PER_BUFFER];
 * @endcode
 *
 * Notes:
 *  - \c SD0_USE_CHANNEL_n should be true if SD0 uses channel n, otherwise false.
 *  - \c SD1_USE_CHANNEL_n should be true if SD1 uses channel n, otherwise false.
 *  - \c BYTES_PER_WORD is based on the configured memory length:
 *    - #I2S_MEMORY_LENGTH_16BITS: 2
 *    - #I2S_MEMORY_LENGTH_24BITS: 3
 *  - \c FRAMES_PER_BUFFER must be divisible by 2
 *  - \c sampleBufferFrames needs to be cast to an \c uint8_t pointer to be used
 *    with the I2S driver.
 *
 * If SD0 and SD1 are not configured to the same direction (or only one is used)
 * then the structure can be simplified as below:
 * @code
 * struct
 * {
 * #if USE_CHANNEL_0
 *     uint8_t channel0SampleWord[BYTES_PER_WORD];
 * #endif
 * #if USE_CHANNEL_1
 *     uint8_t channel1SampleWord[BYTES_PER_WORD];
 * #endif
 * // ...
 * #if USE_CHANNEL_8
 *     uint8_t channel8SampleWord[BYTES_PER_WORD];
 * #endif
 * } sampleBufferFrames[FRAMES_PER_BUFFER];
 * @endcode
 *
 * Notes:
 *  - \c USE_CHANNEL_n should be true if channel n is used, otherwise false.
 *
 *
 * ### Sample Buffer Structure Example ###
 *
 * If for example SD0 and SD1 are configured to the same direction and if
 * channel 0 and 1 are used for SD0 and channel 0 is used for SD1, then the
 * sample buffer would be structured as in the code below.
 *
 * @code
 * struct
 * {
 *     struct
 *     {
 *         uint8_t sd0SampleWord[BYTES_PER_WORD];
 *         uint8_t sd1SampleWord[BYTES_PER_WORD];
 *     } channel0;
 *     struct
 *     {
 *         uint8_t sd0SampleWord[BYTES_PER_WORD];
 *     } channel1;
 * } sampleBufferFrames[FRAMES_PER_BUFFER];
 *
 *  // Access LSB of sample 10 of channel 0 on SD1
 *  uint8_t tmp = sampleBufferFrames[10].channel0.sd1SampleWord[0];
 *
 * @endcode
 *
 *  ============================================================================
 */
#ifndef ti_drivers_i2s_I2SCC26XX__include
#define ti_drivers_i2s_I2SCC26XX__include

#include <ti/drivers/I2S.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/Power.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief  I2S Hardware attributes
 *
 *  intPriority is the I2S peripheral's interrupt priority, as defined by the
 *  RTOS kernel. This value is passed unmodified to HwiP_construct().
 *
 *  pinSD1 and pinSD0 define the SD0 and SD1 data pin mapping, respectively.
 *  pinSCK, pinCCLK and pinWS define the SCK, CCLK and WS clock pin mapping, respectively.
 *  All these pins are typically defined with a macro in a header file, which maps to an IOID.
 *
 *  A sample structure is shown below:
 *  @code
 *  const I2SCC26XX_HWAttrs i2sHWAttrs[CC26X2R1_LAUNCHXL_I2SCOUNT] = {
 *      {
 *         .pinSD1      =  CONFIG_I2S_ADI,
 *         .pinSD0      =  CONFIG_I2S_ADO,
 *         .pinSCK      =  CONFIG_I2S_BCLK,
 *         .pinCCLK     =  CONFIG_I2S_CCLK,
 *         .pinWS       =  CONFIG_I2S_WCLK,
 *         .intPriority =  ~0,
 *      },
 *  };
 *  @endcode
 */
typedef struct
{
    uint_least8_t pinSD1;  /*!< Pin used for SD1 signal. */
    uint_least8_t pinSD0;  /*!< Pin used for SD0 signal. */
    uint_least8_t pinSCK;  /*!< Pin used for SCK signal. */
    uint_least8_t pinCCLK; /*!< Pin used for CCLK signal. Not used in most of the applications. */
    uint_least8_t pinWS;   /*!< Pin used for WS signal. */
    uint8_t intPriority;   /*!< I2S Peripheral's interrupt priority. */
} I2SCC26XX_HWAttrs;

/*!
 *  @cond NODOC
 *  I2S data-interface
 *
 *  This enum defines how the physical I2S interface (SD0/SD1) behaves.
 *  Do not modify.
 */
typedef struct
{
    uint8_t numberOfChannelsUsed;         /*!< Number of channels used on SDx. */
    I2S_ChannelConfig channelsUsed;       /*!< List of the used channels. */
    I2S_DataInterfaceUse interfaceConfig; /*!< IN / OUT / UNUSED */
} I2SCC26XX_DataInterface;
/*! @endcond */

/*!
 *  @cond NODOC
 *  I2S interface
 *
 *  This enum defines one of the interfaces (READ or WRITE) of the I2S module.
 *  Do not modify.
 */
typedef struct
{
    /*! Size of the memory step to access the following sample */
    uint16_t memoryStep;

    /*! Number of WS cycles to wait before starting the first transfer.
     *  This value is mostly used when performing constant latency transfers.
     */
    uint16_t delay;

    /*! Pointer to callback */
    I2S_Callback callback;

    /*! Pointer on the function used to update PTR-NEXT */
    I2S_RegUpdate pointerSet;

    /*! Pointer on the function used to stop the interface */
    I2S_StopInterface stopInterface;

    /*! Pointer on the ongoing transfer */
    I2S_Transaction *activeTransfer;
} I2SCC26XX_Interface;
/*! @endcond */

/*!
 *  @brief      The definition of a function used by the I2S driver
 *              to refresh the pointer
 *
 *  @param      I2S_Handle  I2S_Handle
 *
 *  @param      I2SCC26XX_Interface *interface Pointer on the interface to update
 *
 */
typedef void (*I2SCC26XX_PtrUpdate)(I2S_Handle handle, I2SCC26XX_Interface *interface);

/*!
 *  @cond NODOC
 *  I2S Object.  The application must not access any member variables
 *  of this structure!
 */
typedef struct
{

    /*! To avoid multiple openings of the I2S. */
    bool isOpen;

    /*! WS inversion.
     *  - false: The WS signal is not internally inverted.
     *  - true:  The WS signal is internally inverted.
     */
    bool invertWS;

    /*! Select the size of the memory used using DriverLib defines.
     *  The two options are 16 bits and 24 bits. Any value can be selected,
     *  whatever the value of #bitsPerWord.
     *  - I2S_MEM_LENGTH_16: Memory length is 16 bits.
     *  - I2S_MEM_LENGTH_24: Memory length is 24 bits.
     */
    uint8_t memorySlotLength;

    /*! When dataShift is set to 0, data are read/write on the data lines from
     *  the first SCK period of the half WS period to the last SCK edge of the
     *  WS half period. By setting dataShift to a value different from zero, you
     *  can postpone the moment when data are read/write during the WS half
     *  period. For example, by setting dataShift to 1, data are read/write on
     *  the data lines from the second SCK period of the half WS period to the
     *  first SCK edge of the next WS half period. If no padding is activated,
     *  this corresponds to the I2S standard.
     */
    uint8_t dataShift;

    /*! Number of bits per word (must be between 8 and 24 bits). */
    uint8_t bitsPerWord;

    /*! Number of SCK periods between the first WS edge and the MSB of the first
     * audio channel data transferred during the phase.
     */
    uint8_t beforeWordPadding;

    /*! Number of SCK periods between the LSB of the last audio channel data
     *  transferred during the phase and the following WS edge.
     */
    uint8_t afterWordPadding;

    /*! Number of consecutive frames (minus 1) in the samples buffers to be
     *  handled during one DMA transfer. This field must be set to a value x - 1
     *  where x is between 1 and 255, both included. All the data buffers used
     *  must contain N*x frames or N*x*b bytes (with N an integer satisfying
     *  N>0, and b being the number of bytes per frame).
     */
    uint8_t dmaBuffSizeConfig;

    /*! Select edge sampling type. */
    I2S_SamplingEdge samplingEdge;

    /*! Select if the current device is a Target or a Controller. */
    I2S_Role moduleRole;

    /*! Select phase type. */
    I2S_PhaseType phaseType;

    /*! Frequency divider for the CCLK signal. */
    uint16_t CCLKDivider;

    /*! Frequency divider for the SCK signal. */
    uint16_t SCKDivider;

    /*! Frequency divider for the WS signal. */
    uint16_t WSDivider;

    /*! Time (in number of WS cycles) to wait before the first transfer. */
    uint16_t startUpDelay;

    /*! Structure to describe the SD0 interface */
    I2SCC26XX_DataInterface dataInterfaceSD0;

    /*! Structure to describe the SD1 interface */
    I2SCC26XX_DataInterface dataInterfaceSD1;

    /*
     * I2S SYS/BIOS objects
     */
    /*! Hwi object for interrupts */
    HwiP_Struct hwi;

    /*! Pointer on the function used to update IN and OUT PTR-NEXT */
    I2SCC26XX_PtrUpdate ptrUpdateFxn;

    /*! Structure to describe the read (in) interface */
    I2SCC26XX_Interface read;

    /*! Structure to describe the write (out) interface */
    I2SCC26XX_Interface write;

    /*! Pointer to error callback */
    I2S_Callback errorCallback;

    /*
     * I2S pre and post notification functions
     */
    /*! I2S pre-notification function pointer */
    void *i2sPreFxn;

    /*! I2S post-notification function pointer */
    void *i2sPostFxn;

    /*! I2S pre-notification object */
    Power_NotifyObj i2sPreObj;

    /*! I2S post-notification object */
    Power_NotifyObj i2sPostObj;

    /*! I2S power constraint flag, guard to avoid power constraints getting out
     *  of sync
     */
    volatile bool i2sPowerConstraint;

} I2SCC26XX_Object;
/*! @endcond */

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_i2s_I2SCC26XX__include */
