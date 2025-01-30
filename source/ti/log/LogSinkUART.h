/*
 * Copyright (c) 2023-2024 Texas Instruments Incorporated - http://www.ti.com
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

/*!*****************************************************************************
 *  @file       LogSinkUART.h
 *  @brief      <b>PRELIMINARY</b> LogSinkUART interface
 *
 *  @warning These APIs are <b>PRELIMINARY</b>
 *
 *  The LogSinkUART module is a sink that can be used in conjunction with the
 *  Log.h API in source/ti/log/ and the logging tools available in
 *  tools/log/tiutils. The API defined in this file is made available to the
 *  Logging framework and used as a transport layer for Log.h. For more
 *  information about the Log API, see the @ref log "Log documentation".
 *
 *  To use the UART sink, ensure that the correct library for your
 *  device is linked in and include this header file as follows:
 *  @code
 *  #include <ti/log/LogSinkUART.h>
 *  @endcode
 *
 *  This module implements two functions that are required by the Log API:
 *   - printf(const Log_Module *handle, uint32_t header, uint32_t headerPtr,
 *     uint32_t numArgs, ...);
 *   - buf(const Log_Module *handle, uint32_t header, uint32_t headerPtr,
 *     uint8_t *data, size_t size);
 *
 *  Whenever a log statement that uses LogSinkUART as its sink is called, the
 *  log module delegates to one of the functions above.
 *
 *  @anchor ti_log_LogSinkUART_Overview
 *  # Overview
 *  LogSinkUART is a sink/transport layer that asynchronously outputs encoded
 *  log statements over UART. It uses the UART2 driver to stream data out onto a
 *  user-selectable pin, which can be received and processed by a host-side
 *  tool. For more information about the host-side tool, see tools/log/tiutils.
 *
 *  At the log-site the sink separates the generation of the log record from the
 *  transportation of the record off the device. Deferring the transportation to
 *  a later point of the program execution is done to minimize the runtime
 *  intrusion that would be caused by synchronously outputting the log
 *  statements through the relatively slow UART. The first part generates and
 *  stores the log statements into an intermediate storage synchronously. The
 *  second part uses the idle task of the OS, executed when no other tasks or
 *  interrupts are running, to move the data stored in the intermediate storage
 *  off the device.
 *
 *  By transmitting messages asynchronously the readout at the host-side is
 *  also asynchronous. As a consequence, the receiving of log statements at the
 *  host-side can be deferred from their execution in the program.
 *
 *  This sink requires no special hardware to capture and decode the logs beyond
 *  a basic UART-to-USB bridge.
 *
 *  The data flow at a high level is:
 *      1. Log statement captured at the log site with timestamp
 *      2. Log data marshalled into a log packet
 *      3. Log packet moved to ring buffer working as intermediate storage
 *      4. Intermediate storage flushed via the UART2 driver when nothing else
 *         is happening through a function installed in the Idle-loop/task
 *      5. Data sent out on the UART line
 *      6. Data received by listening COM port on host
 *      7. Data decoded by host and fed through remainder of host logging
 *         infrastructure
 *      8. Logs visualised in Wireshark or dumped to console / log file
 *
 *  @note Throughout this documentation and API the term "packet" is used
 *  instead of the term "record". Both terms are equivalent and can be used
 *  interchangeably.
 *
 *  <hr>
 *  @anchor ti_log_LogSinkUART_Considerations
 *  # Considerations
 *
 *  When using this sink consider the following:
 *      - The number of sinks is limited by the number of UART peripherals in
 *        the device.
 *      - When a UART instance is consumed for a sink it can not be used for
 *        other operations.
 *      - When a single UART LogSink instance is used, an optimised printf is
 *        used that reduces the footprint of each #Log_printf statement.
 *      - Each UART LogSink will create its own Ring Buffer.
 *      - If any part of the transmission is lost/not received by the
 *        logging-tool, the rest of the data might not be interpreted correctly.
 *      - 9.54 hours of logging timestamps are available before overflowing in a
 *        system where 1 bit = 8 us. The tick period for each device can be read
 *        from the multiplier variable in TimestamP<device>.c under
 *        kernel/freertos/dpl. If the timestamp overflows it will go back to 0.
 *      - The LogSinkUART sinks are flushed in the order they are instantiated. 
 *      - If performance issues are observed either increase the size of the
 *        UART2 ring buffer to increase throughput or increase the size of the
 *        intermediate ring buffer size for more log statements. Also note that
 *        the device needs to have idle-time to automatically flush data. If the
 *        software is always doing something then nothing will ever be output.
 *      - SRAM requirements scale with the number of log records to store in
 *        between flushing the buffer.
 *      - A #Log_printf call has an execution time of 20 us to 27.8 us depending
 *        on the number of arguments.
 *      - A Log_buf call has a minimum execution time of 32.8 us and an
 *        approximate increase of 0.183 us per byte in the buffer.
 *
 *  <hr>
 *  @anchor ti_log_LogSinkUART_DesignArchitecture
 *  # Design Architecture
 *
 *  The LogSinkUART implementation is based on the following architecture.
 *
 *  ## Packet Transmission Format
 *  All log packets begin with a 32-bit metadata pointer followed by a 32-bit
 *  timestamp. The next fields depend on the type of log statement:
 *      - Log_printf: Variable number of 32-bit arguments that range from 0 to
 *        8.
 *      - Log_buf: 32-bit field with the size of the buffer being sent followed
 *        by the buffer data.
 *
 *  @startuml
 *  skinparam useBetaStyle true
 *
 *  <style>
 *  timingDiagram {
 *      LineColor #065959
 *  }
 *  </style>
 *
 *  concise "Log_printf packet" as LP
 *  concise "Log_buf packet" as LB
 *  hide time-axis
 *
 *  scale 1 as 50 pixels
 *
 *  @LP
 *  0 is Metadata_Pointer #FFCC99: 0:31
 *  +4 is Timestamp #CC99FF: 32:63
 *  +4 is VA_Arg_0 #99CCFF: 64:95
 *  +4 is {-} #99CCFF: ...
 *  +1 is VA_Arg_n #99CCFF: 32-bits
 *  +4 is {-}
 *
 *  @LB
 *  0 is Metadata_Pointer #FFCC99: 0:31
 *  +4 is Timestamp #CC99FF: 32:63
 *  +4 is Buffer_Size #97D077: 64:95
 *  +4 is Buffer_Data #99CCFF
 *  +5 is {-}
 *  @enduml
 *
 *  If a packet would overflow the ring buffer, a 32-bit overflow packet is
 *  placed instead. It is the original metadata pointer modified to be
 *  identified as an overflow packet. The host-side tool decodes it and displays
 *  an overflow message, indicating that at least that message would have
 *  overflowed. When this is observed, it is recommended to either resize the
 *  ring buffer or disable some log statements.
 *  @note If the intermediate ring buffer is full, no new overflow or log
 *  packets will be stored.
 *
 *  Each log statement used will occupy the following amount of SRAM:
 *
 *  Log statement type | Log statement size (bytes)  |
 *  ------------------ | --------------------------- |
 *  Log_printf         | 8 + 4 * number_of_arguments |
 *  Log_buf            | 12 + buffer_size            |
 *  Overflow           | 4                           |
 *
 *  ## Packet Framing
 *  The host-side must receive and properly handle a continuous stream of packets.
 *  It is able to decode and synchronize packets. If the first 32 bits is not a
 *  valid metadata-pointer address, it will left-shift byte-by-byte until it detects
 *  a valid one. Once a metadata-pointer address is verified, the host-side tool
 *  knows that it is followed by a timestamp. The number of arguments for each
 *  frame is extracted from the .out file. This determines the length of the
 *  current packet and when the metadata-pointer address from the next packet is
 *  expected.
 *
 *  ## Flushing the data
 *  A hook function installed in the Idle-loop/task is run when no other tasks
 *  or interrupts are running. It flushes as many log packets as possible from
 *  the intermediate storage via the UART2 driver set in nonblocking mode. In
 *  this mode, UART2_write() will copy as much data into the transmit buffer as
 *  space allows and return immediately. The maximum space allowed, and
 *  therefore the amount of data sent out every time that the hook function is
 *  called, is determined by the size of the TX Ring Buffer.
 *
 *  The Idle-loop/task will always be run before the power management loop.
 *
 *  Since each OS has a different implementation of the Idle-loop/task, the
 *  installation of the hook function will also be different for each OS. The
 *  automatic installation is currently supported for FreeRTOS and TI-RTOS 7
 *  when using SysConfig.
 *
 *  <hr>
 *  @anchor ti_log_LogSinkUART_Usage
 *  # Usage
 *
 *  To use the UART LogSink the application calls the following APIs:
 *    - LogSinkUART_init(): Initialize a UART sink. This function takes as
 *      argument an index that describes the sink that has to be initialized.
 *    - LogSinkUART_flush(): Function to flush all the LogSinkUART sinks in the
 *      order they are added. For each sink it will read data from the ring
 *      buffer and put as much as possible on the UART interface.
 *    - LogSinkUART_finalize(): Finalize a UART sink. This function takes as
 *      argument an index that describes the sink that has to be finalized.
 *
 *  Details on usage are provided in the following subsections.
 *
 *  @anchor ti_log_LogSinkUART_Examples
 *  ## Examples #
 *  * @ref ti_log_LogSinkUART_initialize "Initializing a UART LogSink"
 *  * @ref ti_log_LogSinkUART_flush "Flushing UART sinks"
 *  * @ref ti_log_LogSinkUART_finalize "Finalizing a UART LogSink"
 *  * @ref ti_log_LogSinkUART_protect "Protect log statements"
 *
 *  @anchor ti_log_LogSinkUART_initialize
 *  ### Initializing a UART LogSink
 *  If LogSinkUART is enabled through SysConfig, then LogSinkUART_init() will be
 *  automatically called from Board_init(). If SysConfig is not used, the user
 *  must initialize the log sink. LogSinkUART_init() can also be called after a
 *  sink has been finalized with LogSinkUART_finalize(). To initialize a sink,
 *  first include the ti_log_config.h library containing the expansion of the
 *  sink name. Afterwards, call the initialize function passing as an argument
 *  the name of the sink to be initialized.
 *
 *  @code
 *  #include "ti_log_config.h"
 *
 *  LogSinkUART_init(sink_name);
 *  @endcode
 *
 *  @anchor ti_log_LogSinkUART_flush
 *  ### Flushing UART sinks
 *  LogSinkUART_flush() will be called automatically in the Idle-loop/task for
 *  FreeRTOS and TI-RTOS 7. Despite this, the user can still manually call the
 *  function to send out as much data as possible from all the existing ring
 *  buffers when desired. The flush function can be called from either a task or
 *  interrupt context.
 *
 *  @anchor ti_log_LogSinkUART_finalize
 *  ### Finalizing a UART LogSink
 *  LogSinkUART_finalize() will cancel all ongoing UART writes and call
 *  UART2_close(). All log packets remaining in the intermediate storage will be
 *  lost when calling LogSinkUART_init() because the ring buffer will be reset.
 *  All log statements after a finalize call will also be lost due to the reset
 *  of the ring buffer when initializing the sink. To finalize a sink first
 *  include the  ti_log_config.h library containing the expansion of the sink
 *  name. Afterwards, call the finalize function passing as an  argument the
 *  name of the sink to be finalized.
 *
 *  @code
 *  #include "ti_log_config.h"
 *
 *  LogSinkUART_finalize(sink_name);
 *  @endcode
 *
 *  @anchor ti_log_LogSinkUART_protect
 *  ### Protect log statements
 *  To ensure that logs are correctly ordered when put back together on the
 *  host, log statements can be called from a context where HWI are disabled.
 *  This also ensures that the recorded timestamp is faithful to when the log
 *  statement was executed.
 *
 *  Logs can end up ordered out as a consequence of a log call being preempted
 *  by a higher priority task with other log statements. The following example
 *  shows how a log statement can be protected to ensure that the execution
 *  sequence is maintained in the ordering on the host-side.
 *
 *  @code
 *  #include <ti/drivers/dpl/HwiP.h>
 *  uint32_t key;
 *
 *  key = HwiP_disable();
 *  Log_printf(MyModule, Log_DEBUG, "The answer is %d", 42);
 *  HwiP_restore(key);
 *  @endcode
 *
 *  <hr>
 *  @anchor ti_log_LogSinkUART_Configuration
 *  # Configuration
 *
 *  In order to use the LogSinkUART APIs, the application is required to provide
 *  sink-specific configuration in the ti_log_config.c file. The LogSinkUART
 *  interface defines a configuration data structure:
 *
 *  @code
 *  typedef struct  {
 *      void                   *object;
 *      void          const    *hwAttrs;
 *  } LogSinkUART_Config;
 *  @endcode
 *
 *  The application must declare an array of #LogSinkUART_Config elements, named
 *  @p LogSinkUART_config[].  Each element of @p LogSinkUART_config[] must be
 *  populated with pointers to a sink specific object, and hardware attributes.
 *  The hardware attributes define properties such as the UART peripheral's
 *  attributes, and a pointer to the intermediate ring buffer and its size.
 *  These are automatically assigned through SysConfig. Each element in
 *  @p LogSinkUART_config[] corresponds to a UART sink instance, and none of the
 *  elements should have NULL pointers.
 *
 *
 *  The configuration for the UART LogSink is based on the driver's
 *  configuration. Refer to the @ref driver_configuration
 *  "Driver's Configuration" section for driver configuration information.
 *
 *  To automatically initialize a UART sink when initializing the board,
 *  LogSinkUART_init() is called inside Board_init() in ti_drivers_config.c. To
 *  have access to the function and get the expansion of the sink name, include
 *  the following libraries:
 *
 *  @code
 *  #include "ti_log_config.h"
 *  #include <ti/log/LogSinkUART.h>
 *  @endcode
 *
 *  ============================================================================
 */

#ifndef ti_log_LogSinkUART__include
#define ti_log_LogSinkUART__include

#include <stdint.h>
#include <stddef.h>
#include <ti/log/Log.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/utils/RingBuf.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief LogSinkUART version
 */
#define Log_TI_LOG_SINK_UART_VERSION 0.1.0

/*!
 *  @brief      LogSinkUART Hardware attributes
 */
typedef struct
{
    unsigned char *bufPtr; /*!< Pointer to intermediate ring buffer */
    size_t bufSize;        /*!< Size of bufPtr */
    uint32_t baudRate;     /*!< UART2 baudRate */
    UART2_Parity parity;   /*!< UART2 parity */
    uint32_t uartIndex;    /*!< UART2 instance index */
} LogSinkUART_HWAttrs;

/*!
 *  @brief      LogSinkUART Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct
{
    UART2_Handle uartHandle; /*!< UART2 handle */
    RingBuf_Object ringObj;  /*!< Intermediate ring buffer */
} LogSinkUART_Object;

/*!
 *  @brief  LogSinkUART Global configuration
 *
 *  The LogSinkUART_Config structure contains a set of pointers used to
 *  characterize a LogSinkUART implementation.
 *
 */
typedef struct
{
    /*! Pointer to a driver specific data object */
    void *object;
    /*! Pointer to a driver specific hardware attributes structure */
    void const *hwAttrs;
} LogSinkUART_Config;

/*!
 *  @brief      Array with the configuration of each sink.
 */
extern const LogSinkUART_Config LogSinkUART_config[];

/*!
 *  @brief      LogSinkUART Sink parameters
 *
 *  The LogSinkUART_Instance structure contains a set of parameters used to
 *  characterize a LogSinkUART sink.
 */
typedef struct
{
    /*! Index of the LogSinkUART sink instance used to index
     *  @p LogSinkUART_config[] */
    uint_least8_t index;
} LogSinkUART_Instance;

/*!
 *  @brief      A handle for the LogSinkUART_Instance structure.
 */
typedef LogSinkUART_Instance *LogSinkUART_Handle;

/*!
 *  @brief Flush all the LogSinkUART sinks.
 *
 *  Function to flush each of the LogSinkUART sinks in the order they are added.
 *  For each sink it will read its ring buffer and put as much data as possible
 *  on the UART interface. The flush can occur in task or interrupt context.
 */
extern void LogSinkUART_flush(void);

/*!
 *  @brief  Initialize a given LogSinkUART sink.
 *
 *  Function to initialize a given LogSinkUART sink specified by the
 *  particular index value. It constructs a ring buffer, sets up the
 *  UART2 attributes and opens the given UART peripheral
 *
 *  @param[in]  index     Logical sink number for the LogSinkUART indexed into
 *                        the LogSinkUART_config table
 *
 *  The UART peripheral is set to send only mode, with the following fixed
 *  parameters:
 *  @code
 *      writeMode = UART2_Mode_NONBLOCKING;
 *      dataLength = UART2_DataLen_8;
 *      stopBits = UART2_StopBits_1;
 *  @endcode
 *
 *  The baudRate and parityType can be configured from SysConfig. The size of
 *  the ring buffer can also be set from the SysConfig.
 */
extern void LogSinkUART_init(uint_least8_t index);

/*!
 *  @brief  Finalize a given LogSinkUART sink.
 *
 *  Function to finalize a given LogSinkUART sink specified by the
 *  particular index value. It cancels any ongoing write operation over the
 *  given UART peripheral and closes it.
 *
 *  @param[in]  index     Logical sink number for the LogSinkUART indexed into
 *                        the LogSinkUART_config table
 */
extern void LogSinkUART_finalize(uint_least8_t index);

/*!
 *  @cond NODOC
 *  @brief  Marshal and store a #Log_printf statement into a ring buffer.
 *
 *  Function to marshal a #Log_printf statement into a packet
 *  and store it into a ring buffer. If the packet would overflow
 *  the ring buffer it stores an overflow packet instead.
 *
 *  This implementation is optimised for use with LTO when there is only a
 *  single UART LogSink instance to reduce the footprint of #Log_printf
 *  statements.
 *
 *  It should not be used when multiple UART LogSink instances are present
 *  within the system.
 *
 *  @note Applications must not call this function directly. This is a helper
 *  function to implement #Log_printf
 *
 *  @param[in]  handle     LogSinkUART sink handle
 *
 *  @param[in]  header     Metadata pointer
 *
 *  @param[in]  headerPtr  Pointer to metadata pointer
 *
 *  @param[in]  numArgs    Number of arguments
 *
 *  @param[in]  ...        Variable amount of arguments
 */
extern void LogSinkUART_printfSingleton(const Log_Module *handle,
                                        uint32_t header,
                                        uint32_t headerPtr,
                                        uint32_t numArgs,
                                        ...);

extern void LogSinkUART_printfSingleton0(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...);

extern void LogSinkUART_printfSingleton1(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...);

extern void LogSinkUART_printfSingleton2(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...);

extern void LogSinkUART_printfSingleton3(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...);
/*! @endcond NODOC */

/*!
 *  @cond NODOC
 *  @brief  Marshal and store a #Log_printf statement into a ring buffer.
 *
 *  Function to marshal a #Log_printf statement into a packet
 *  and store it into a ring buffer. If the packet would overflow
 *  the ring buffer it stores an overflow packet instead.
 *
 *  This is a dependency injection implementation. It is able to support an
 *  arbitrary number of LogSinkUART_Instance instances by passing in the sink
 *  state though @c handle. This requires additional flash to load @c handle in
 *  each #Log_printf though.
 *
 *  @note Applications must not call this function directly. This is a helper
 *  function to implement #Log_printf
 *
 *  @param[in]  handle     LogSinkUART sink handle
 *
 *  @param[in]  header     Metadata pointer
 *
 *  @param[in]  headerPtr  Pointer to metadata pointer
 *
 *  @param[in]  numArgs    Number of arguments
 *
 *  @param[in]  ...        Variable amount of arguments
 */
extern void LogSinkUART_printfDepInjection(const Log_Module *handle,
                                           uint32_t header,
                                           uint32_t headerPtr,
                                           uint32_t numArgs,
                                           ...);

extern void LogSinkUART_printfDepInjection0(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...);

extern void LogSinkUART_printfDepInjection1(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...);

extern void LogSinkUART_printfDepInjection2(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...);

extern void LogSinkUART_printfDepInjection3(const Log_Module *handle, uint32_t header, uint32_t headerPtr, ...);
/*! @endcond NODOC */

/*!
 *  @cond NODOC
 *  @brief  Marshal and store a #Log_buf statement into a ring buffer.
 *
 *  Function to marshal a #Log_buf statement into a packet
 *  and store it into a ring buffer. If the packet would overflow
 *  the ring buffer it stores an overflow packet instead.
 *
 *  This is a dependency injection implementation. It is able to support an
 *  arbitrary number of LogSinkUART_Instance instances by passing in the sink
 *  state though @c handle.
 *
 *  @note Applications must not call this function directly. This is a helper
 *  function to implement #Log_buf
 *
 *  @param[in]  handle     LogSinkUART sink handle
 *
 *  @param[in]  header     Unused metadata pointer
 *
 *  @param[in]  headerPtr  Pointer to metadata pointer
 *
 *  @param[in]  data       Data buffer to log
 *
 *  @param[in]  size       Size in bytes of array to store
 */
extern void LogSinkUART_bufDepInjection(const Log_Module *handle,
                                        uint32_t header,
                                        uint32_t headerPtr,
                                        uint8_t *data,
                                        size_t size);
/*! @endcond NODOC */

/*!
 * @brief Create a LogSinkUART instance called @c name
 */
#define Log_SINK_UART_DEFINE(name) LogSinkUART_Instance LogSinkUART_##name##_Config = {.index = name}

/*!
 * @brief Use a LogSinkUART instance called @c name when not created in the same
    file
 */
#define Log_SINK_UART_USE(name) extern LogSinkUART_Instance LogSinkUART_##name##_Config

/*!
 * @brief Initialize a LogSinkUART instance called @c name with log @c _levels ,
 * printf delegate function @c printfDelegate , buf delegate function @c bufDelegate
 * and dynamic log level @c _dynamicLevelsPtr
 */
#define Log_MODULE_INIT_SINK_UART(name, _levels, printfDelegate, bufDelegate, _dynamicLevelsPtr)                      \
    {                                                                                                                 \
        .sinkConfig = &LogSinkUART_##name##_Config, .printf = printfDelegate, .printf0 = printfDelegate##0,           \
        .printf1 = printfDelegate##1, .printf2 = printfDelegate##2, .printf3 = printfDelegate##3, .buf = bufDelegate, \
        .levels = _levels, .dynamicLevelsPtr = _dynamicLevelsPtr,                                                     \
    }

/*!
 *  @cond NODOC
 *  Define log sink version.
 *
 *  @note Applications must not call this function directly. This is a helper
 *  function.
 */
_Log_DEFINE_LOG_VERSION(LogSinkUART, Log_TI_LOG_SINK_UART_VERSION);
/*! @endcond NODOC */

#if defined(__cplusplus)
}
#endif

#endif /* ti_log_LogSinkUART__include */
