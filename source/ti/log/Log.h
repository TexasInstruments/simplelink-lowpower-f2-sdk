/*
 * Copyright (c) 2019-2023 Texas Instruments Incorporated - http://www.ti.com
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

/**
 *  @file  ti/log/Log.h
 *
 *  @addtogroup ti_log_LOG Log Interface
 *
 *  @brief      The Log module provides APIs to instrument source code
 *
 *  To access the LOG APIs, the application should include its header file as
 *  follows:
 *  @code
 *  #include <ti/log/Log.h>
 *  @endcode
 *
 *  ## Beta Disclaimer ##
 *  The logging ecosystem are to be considered beta quality. They are not
 *  recommended for use in production code by TI. APIs and behaviour will change
 *  in future releases. Please report issues or feedback to [__E2E__][e2e].
 *
 *  [e2e]: https://e2e.ti.com/
 *
 *  ## Definitions ##
 *
 *  The following terms are used throughout the log documentation.
 *
 *  | Term                            | Definition                                                                                                                                                                                                                                                                                                                     |
 *  |---------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 *  | `LogModule`                     | A parameter passed to Log APIs to indicate which software module the log statement originated from. Modules also control the routing of logs to sinks.                                                                                                                                                                         |
 *  | `LogLevel`                      | The severity or importance of a given log statement.                                                                                                                                                                                                    |
 *  | `Sink`                          | Also simply called a logger. This is a transport specific logger implementation. <br> The Logging framework is flexible such that multiple sinks may exist in a single firmware image.                                                                                                                                         |
 *  | `CallSite`                      | A specific invocation of a Log API in a given file or program.                                                                                                                                                                                                                                                                 |
 *  | `Record`                        | The binary representation of a log when it is stored or transported by a given sink. The log record format varys slightly with each sink depending on their implementation and needs. However, they all convey the same information.                                                                                           |
 *  | Link Time Optimization (LTO)    | A feature of some toolchains that can significantly reduce the code overhead of the log statements through a process called dead code elimination. In order to maximize the benefits of this, all static libraries and application files should have LTO enabled.                                                              |
 *
 *  ## Summary ##
 *
 *  The following sections describe the usage of the TI logging system
 *  implementation. This document will focus on the target (i.e. code that runs)
 *  on the embedded device. For associated PC tooling, please see the
 *  [README](../../../tools/log/tiutils/Readme.html) in the tools/log/tiutils/ folder.
 *
 *  Desgin Philiosophy:
 *
 *  * Logs target code should be as efficient as possible.
 *    * This means that Log APIs should minimize FLASH, RAM, and execution overhead.
 *  * Complexity should be pushed to host side tooling where possible.
 *    *  Even if this means that PC setup/tooling requirements are more complex.
 *  * Multiple log sink implemenetations shall be able to exist in a system.
 *    *  Where applicable, multiple instances should be supported (e.g. multiple circular buffers to collect logs)
 *  * It shall be possible to remove logging entirely using the preprocessor
 *  * Configuration of logging should be deferred to application compile and
 *    link time.
 *    * That means that the end application builder should make
 *       decisions about the logging settings. This means that TI provided libraries
 *       are not opinionated about what log levels should be enabled or how
 *       modules should be routed to sinks.
 *  * TI's logging system will leverage SysConfig out of the box, but it should
 *    be possible to configure and use logging easily without the needing
 *    SysConfig.
 *
 *  ## Stated Limitations ##
 *
 *  * It is not possible to control which log sink is used for each call site.
 *    Routing of logs is controlled at a module level.
 *  * A maximum of 8 arguments is supported for variadic APIs.
 *
 *  ## Anatomy of Log Statement ##
 *
 *  At the core of the logging implementation is heavy use of the C
 *  preprocessor. When reading an application, the Log APIs may look like
 *  function calls, but the preprocessor expands them heavily.
 *
 *  There are several ways in which the preprocessor is used.
 *
 *  ### Globally ###
 *  1. To enable/disable logs globally. If `ti_log_Log_ENABLE` is not defined,
 *     all statements are removed by the preprocessor. This does not rely on LTO
 *     or any other optimization. It removes any traces of logs from the program.
 *
 * ### Per Log Statement ###
 *  1. (Level filtering): Insert the if statement that checks if the log level
 *     of the statement has been enabled in its module configuration. If the log
 *     level is not enabled, the process ends here.
 *
 *  2. (String declaration): Automate placement of constant strings, format
 *     strings, and pointers to these strings in the the nonloadable metadata
 *     section of the out file. This saves FLASH on the target. Each sring
 *     contains a large amount of data, including the following:
 *
 *     * File and line number of the log statement
 *     * The log level and module of the log statement
 *     * The format string
 *     * The number of arguments
 *
 *  3. (Argument counting): Log APIs are variadic in nature, up to 8 arguments
 *     are supported. However, at preprocess time, the number of arguments must
 *     be known.
 *
 *  4. (Name spacing): Routing from module to sink. The module parameter of the
 *     Log API controls which sink its log statements will be routed to.
 *     The preprocessor does name expansion to look up the enabled log levels
 *     and selected sink function pointers from the module's configuration
 *     structure. NOTE: The used sink may require initialization. Please
 *     verify with the specific sink documentation on how to initialize the sink.
 *
 *  5. (Sink API Invocation): With the names resolved and levels checked,
 *     the logger is now ready to execute the sink function. This is done via
 *     function pointer.
 *
 *  An simplified pseudo-C implementation of what
 *  `Log_printf(LogModule_App1, Log_DEBUG, "Hello World!");` would expand to
 *  is shown below. This will not compile and is not extensive, just for
 *  illustration.
 *
 *  @code
 *  // Global log enable check, wrapped around each log site
 *  #if defined(ti_log_Log_ENABLE)
 *      // Check if the level of this specific log statement has been enabled by the module
 *      if (LogMod_LogModule_App1.levels & level) {
 *            // Note that ^^ is the record separator. Pack meta information into format string. This is stored off target.
 *            const string logMeta = "LOG_OPCODE_FORMATED_TEXT^^"../../log.c"^^80^^Log_DEBUG^^LogMod_LogModule_App1^^"Hello World!"^^0";
 *            // Route log to the selected sink implementation. This is done via function pointer.
 *            // The 0 indicates no arguments. If runtime arguments were provided, they would follow.
 *            LogMod_LogModule_App1.printf(pointerToModuleConfig, 0);
 *      }
 *  #endif
 *  @endcode
 *
 *  From here, the logger has transferred control over to the sink
 *  implementation, which varys based on the tansport
 *  (e.g. circular buffer in memory or UART).
 *
 *  ## Modules ##
 *
 *  When adding log statements to the target software, it is recommended to
 *  create a logging module for each software component in the image. Modules
 *  enable the reader to understand where the log record originated from. Some
 *  log visualizers may allow the reader to filter or sort log statements by
 *  module. It is also recommended to namespace modules.
 *
 *  For example, a good module name for the `UART` driver that exists in
 *  `source/ti/drivers`, could be `ti_drivers_UART`.
 *
 *  Modules also control the routing of log records to a sink. Routing is
 *  controlled via the LogModule panel in SysConfig, but can be changed in plain C
 *  code using the macro @ref Log_MODULE_DEFINE and passing the sink specific
 *  `Log_MODULE_INIT_` to the `init` parameter within the @ref Log_MODULE_DEFINE
 *  macro. An example for the LogBuf sink is below, it will do the following
 *
 *  1. Create a module called `LogModule_App1`.
 *  1. Initialze the module for use with the buffer based LogSink.
 *     Use buffer instance called `CONFIG_ti_log_LogSinkBuf_0`.
 *  1. Enable only the `Log_ERROR` level. Other logs will not be stored.
 *
 *  @code
 *  #include <ti/log/Log.h>
 *  #include <ti/log/LogSinkBuf.h>
 *  Log_MODULE_DEFINE(LogModule_App1, Log_MODULE_INIT_SINK_BUF(CONFIG_ti_log_LogSinkBuf_0, Log_ERROR));
 *  @endcode
 *
 *  TI created libraries will never use @ref Log_MODULE_DEFINE.
 *  This leaves the choice of routing logs to their sinks
 *  to the end application writer. This is recommended when creating any
 *  static libraries to defer the final logging decisions to link time.
 *
 *  Each new module will instantiate a Log_Module structure with a
 *  `levels` bitmap and pointers to the selected sink implementation and sink
 *  configuration. See the @ref Log_Module structure for more information.
 *
 *  ## Levels ##
 *  Log levels are a way to indicate the severity or importance of the contents
 *  of a particular log call site. Each call site takes an argument that allows
 *  the user to specify the level. As with modules, log visualization tools
 *  allow the user to sort or filter on a given level. This can help the reader
 *  to find important or relevant log statements in visualization.
 *
 *  Log levels are also used to control the emission of logs. Each call site
 *  will check that the level is enabled before calling the underlying log API.
 *
 *  Depending on optimization, the check at each log statement for whether the
 *  given level is enabled or not may end up being optimized away, and the
 *  entire log statement may be optimized away if the log level is not enabled.
 *
 *  @code
 *  if ((level) & module.levels) {// Call Log API
 *  }
 *  @endcode
 *
 *  Optimization level `-flto` for both the TICLANG toolchain and GCC will
 *  typically be able to optimize the above statement.
 *
 *  @remark
 *
 *  ## Log Metadata ##
 *
 *  Each time a Log API is invoked, a metadata string is placed in the .out
 *  file. This string contains information about the API type, file, line
 *  module, level, and other information associated with the log call site. Each
 *  call site emits a string to a specific memory section called `.log_data`.
 *  In addition to this, a pointer to the string in .log_data is stored in
 *  another section called `.log_ptr`. Because the .log_ptr section is always
 *  in the same location, and each entry is the same size, an indexing-scheme
 *  can be used to refer to each log-string. Entry 0 in .log_ptr would point
 *  to the first string, entry 1 would point to the second string, etc. This
 *  Is necessary on some devices where transmitting an entire 32-bit address
 *  as a reference to the string is not possible, and instead an 8-bit index
 *  can be transmitted across the Log sink implementation instead.
 *  In order to use logging, this section should be added to the linker command
 *  file. By default, this section points to a nonloadable region of memory.
 *  Meaning that the metadata will not be loaded on the target device. Instead,
 *  the various logging visualization tools such as wireshark and TI ROV2 will
 *  read the metadata from this section and properly decode the log statements.
 *  The benefit of this approach is that very little memory is consumed on
 *  target. Additionally, the log transport only needs to store or send pointers
 *  to this meta section when a log API is called.
 *
 *  This approach minimizes the amount of memory consumed on device
 *  and bytes sent over the transport. This section can be loaded on target if
 *  desired or if you are creating a custom logger. The design does not preclude
 *  this.
 *
 *  In order to use the logging framework, the log section must be added to the
 *  linker command file. Here is a sample for the TI linker. Other examples can
 *  be found in the TI provided linker files for each toolchain.
 *
 *  @code
 *  MEMORY
 *  {
 *      // List other memory regions here
 *      LOG_DATA (R) : origin = 0x90000000, length = 0x40000
 *      LOG_PTR  (R) : origin = 0x94000008, length = 0x40000
 *  }
 *  SECTIONS
 *  {
 *       .log_data       :   > LOG_DATA, type = COPY
 *       .log_ptr        : { *(.log_ptr*) } > LOG_PTR align 4, type = COPY
 *  }
 *  @endcode
 *
 *  ## Sinks ##
 *
 *  Sinks are responsible for storing or transporting the log record. In general
 *  there are two categories of sinks:
 *
 *  1. Those that perform storage of logs.
 *  2. Those that stream logs over a transport medium, and thus do not perform
 *     storage.
 *
 *  Sinks may vary in their implementation based on the nature of the storage
 *  or transport that they support, but they all have the following in common:
 *
 *  * Are named ti_log_LogSink<SinkName>. Where `<SinkName>` is the name of the sink.
 *  * Must implement the Log_printf and Log_buf APIs from this file.
 *  * Must provide _USE, _INIT, and _DEFINE macros.
 *
 *  In addition, some sinks require initialization. This will be listed in the
 *  documentation for the sink implementation.
 *  Sinks are closely tied to their associated host side tooling. Since the
 *  log statements are not parsed at all by the target code, this must be
 *  delegated to a program running on a PC. While the binary format of log
 *  records may vary across sink implementations, it is suggested that each
 *  log record contain:
 *
 *  1. Timestamp
 *  1. Pointer to metadata string. This will be looked up by the PC side tooling
 *     in the out file.
 *  1. Runtime arguments
 *
 *  This is the minimum amount of information needed to decode a log statement.
 *
 *  # Usage #
 *  This section provides a basic @ref ti_log_LOG_Synopsis "usage summary" and a
 *  set of @ref ti_log_LOG_Examples "examples" in the form of commented code
 *  fragments. Detailed descriptions of the LOG APIs are provided in subsequent
 *  sections.
 *
 *  @anchor ti_log_LOG_Synopsis
 *  ### Synopsis ###
 *
 *  @code
 *  // Import the Log header
 *  #include <ti/log/Log.h>
 *
 *  // Define your log module and log sink
 *  // If using SysConfig, it will be done automatically, or it can be done manually:
 *  // Use helper macro from <ti/log/LogSinkBuf.h> to make a sink instance (buffer + config) with 100 entries.
 *  Log_SINK_BUF_DEFINE(MyBufferSink, LogSinkBuf_Type_CIRCULAR, 100);
 *
 *  // Use helper macro from <ti/log/Log.h> to make a module pointing at the new sink instace.
 *  // This example will enable all log levels
 *  Log_MODULE_DEFINE(MyModule, Log_MODULE_INIT_SINK_BUF(MyBufferSink, Log_ALL))
 *
 *  // Some log sinks may require special initialization to configure hardware. Refer to the documentation of
 *  // the sink you wish to use. For example, LogSinkITM must be initialised like this before it can be used:
 *  // LogSinkITM_init();
 *
 *  // Invoke one of the log APIs you want to use for either premade events or formatted strings
 *  Log_printf(MyModule, Log_DEBUG, "The answer is %d", 42);
 *  uint8_t buffer[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 *  Log_buf(MyModule, Log_VERBOSE, buffer, sizeof(buffer));
 *  @endcode
 *
 *
 *
 *  @anchor ti_log_LOG_Examples
 *  ### Examples ###
 *  * @ref ti_utils_LOG_Example_event "Log Event"
 *  * @ref ti_utils_LOG_Example_printf "Log printf"
 *  * @ref ti_utils_LOG_Example_buf "Logging buffers"
 *
 *  @anchor ti_utils_LOG_Example_event **Log Event**:
 *
 *  The following example demonstrates how to create a log event object and use
 *  it in the code. There are two steps to using a log event: 1. instantiation
 *  and 2. call site(s). Instantiation creates the event and the necessary
 *  metadata, and call site is where the event is actually recorded by the
 *  logger framework.
 *
 *  @code
 *  // Create a log event data type called LogEvent_count
 *  // The log module is MyModule
 *  // The format string is "count=%d" -- this should describe what the event does
 *
 *  Log_EVENT_DEFINE(LogEvent_count, "count=%d");
 *  @endcode
 *
 *  Later on, in the application, the count event is consumed. Note the log
 *  module must match between event creation and call site. In the code below, a
 *  LogEvent record is created for serialization or stage by the Log sink.
 *
 *  @code
 *  Log_EVENT_USE(LogEvent_count); // If not defined in same file
 *  // ...
 *  Log_event(MyModule, Log_DEBUG, LogEvent_count, count++);
 *  @endcode
 *
 *  @anchor ti_utils_LOG_Example_printf **Log Printf**:
 *
 *  The following example demonstrates use of the Log printf API. in code. Log
 *  will embed the format string in the call site and will take arguments using
 *  varadic arguments
 *
 *  @code
 *  Log_printf(MyModule, Log_DEBUG, "Hello World!");
 *  @endcode
 *
 *  @anchor ti_utils_LOG_Example_buf **Log Buf**:
 *
 *  The following example demonstrates use of the Log buf API. in code.
 *
 *  Buf will embed the format string in the call site and will take the buffer
 *  as a pointer and length. Buffers are treated as arrays of bytes. The buffer
 *  API should only be used when it is necessary to log data that is only
 *  available at runtime. It will actually send or store the entire contents of
 *  the buffer, so this API should be used sparingly as it is costly in terms of
 *  runtime and memory overhead.
 *
 *  @code
 *  uint8_t bufferToLog[] = {0, 1, 2, 3, 4, 5};
 *  Log_buf(ti_log_LogMain, Log_DEBUG, "The contents of bufferToLog are: ", bufferToLog, sizeof(bufferToLog));
 *  @endcode
 *
 *  @anchor ti_utils_LOG_Example_guide **Log API usage**:
 *
 *  For a uniform experience with the logging tool, users are recommended to follow certain guidelines regarding
 *  the Log API. Typical use-cases for each API call is desribed below
 *
 *  #### Log_printf ####
 *
 *  Log_printf should be the default mechanism for emitting a log statement within an application.
 *  Along with the Log-levels, Log_printf should be used to communicate debug information as a formatted string,
 *  which accepts variadic arguments. In this case, a pointer to the string and the arguments themselves are
 *  transported by the Log sink.
 *
 *  @code
 *  Log_printf(MyLibraryLogModule, Log_ERROR, "Library function received illegal argument: %d", arg);
 *  @endcode
 *
 *  #### Log_event ####
 *
 *  Log_event is meant to represent more generic debug-information, and typically something that can occur from
 *  anywhere in the application, as opposed to being localised in a single library. Events can also be defined once
 *  and referenced from anywhere in the application, so the same event can be used by multiple libraries.
 *  A generic example would be an event such as "Entering critical section"
 *
 *  @code
 *  Log_EVENT_DEFINE(LogEvent_enterCritical, "Entering critical section");
 *
 *  Log_EVENT_USE(LogEvent_enterCritical); // If not defined in same file
 *  // ...
 *  Log_event(MyModule, Log_DEBUG, LogEvent_enterCritical);
 *  @endcode
 *
 *  #### Log_buf ####
 *
 *  When the debug-information to be emitted is a large amount of dynamic data, and is not suitable as an argument
 *  to printf, then Log_buf should be used. Log_buf can transport the contents of large dynamic buffers, and as a
 *  consequence has a larger overhead and should be used sparsely.
 */

#ifndef ti_log_Log__include
#define ti_log_Log__include

/*! @ingroup ti_log_LOG */
/*@{*/

/*
 *  ======== Log.h ========
 *  @brief Contains Log library APIs
 */
#include <stdint.h>
#include <stddef.h>

#if defined (__cplusplus)
extern "C" {
#endif

/*
 *  ======== ti_log_Log_ENABLE ========
 *  Enable instrumentation using link-time optimization implementation
 *
 *  Define this symbol to add instrumentation at compile time.
 *  It must be defined before including this header file.
 */
#if ti_log_Log_ENABLE
/*
 *  =============================
 *  ======== Log Enabled ========
 *  =============================
 */

#define Log_TI_LOG_VERSION 0.1.0
/**
 *  @brief Defines a log module
 *
 *  Log modules are like namespaces for log statements, but also controls the
 *  enabled log levels and decides where the log statement is redirected.
 *
 *  @param[in]  name       Name of the log module. Gets prefixed with `LogMod_`.
 *  @param[in]  init       Initialization macro from the wanted sink
 *
 *  This is a helper to define `Log_Module LogMod_yourName` and initialize it
 *  with the configuration and functions of the wanted log sink.
 *
 *  For example, you have already used the sink definition macros found in
 *  LogSinkITM.h, and now you want to define a new module that uses this:
 *
 *   `Log_MODULE_DEFINE(MyDriver, Log_MODULE_INIT_SINK_ITM(Log_DEBUG | Log_ERROR))`
 *
 *  Perhaps you used the LogSinkBuf.h helper macro which needs a unique name
 *  per instance and made a separate buffer for critical errors:
 *
 *   `Log_MODULE_DEFINE(MyCritical, Log_MODULE_INIT_SINK_BUF(criticalBuf, Log_ERROR)`
 *
 *  You would use this in your application via
 *      `Log(MyCritical, Log_ERROR, "Oops")`
 */
#define Log_MODULE_DEFINE(name, init) const Log_Module LogMod_ ## name = init

/**
 *  @brief Declares a reference to a log module
 *
 *  Declares that a log module is defined in another file so that it can be
 *  used in the file with this macro in it.
 *
 *  @note This is done automatically for `Log` and `Log_buf` statements.
 *
 *  @param[in]  name Name of the log module. Gets prefixed with `LogMod_`.
 */
#define Log_MODULE_USE(name) extern const Log_Module LogMod_ ## name

/**
 *  @brief Resolves to the symbol name of the log module
 *
 *  Provided for forward compatibility purposes should you have a need to
 *  reference the log module symbol directly.
 */
#define LOG_MODULE_SYM(name) LogMod_ ## name

/** @cond NODOC */

/* This macro protects against sideffects of the C preprocessor expansion
 * of log statements. Each log API should be guarded by it.
 * An article explaining this behavior can be found here:
 * https://gcc.gnu.org/onlinedocs/cpp/Swallowing-the-Semicolon.html
 */
#define _Log_GUARD_MACRO(x) do{ x }while(0)

/*
 *
 *  ======== Log Private Macros ========
 *
 *  The following macros are intended to be private to the log module and
 *  are not intended for use by the user. Private macros will start with _Log.
 *
 *  In the case of multi level macros (macros that invoke other macros), a
 *  letter is appended at the end of the definition. With each level of nesting,
 *  the appended letter is incremented.
 *
 *  For example: _Log_test --> _Log_test_A --> _Log_test_B
 */
/* Extracts the first/remaining argument from __VA_ARGS__ */
#define _Log_CAR_ARG(N, ...) N
#define _Log_CDR_ARG(N, ...) __VA_ARGS__


/*
 *  ======== Meta string tokenization macros ========
 */
/*  Helper macro to concatenate two symbols */
#define _Log_CONCAT2_A(x,y) x ## _ ## y
#define _Log_CONCAT2(x,y) _Log_CONCAT2_A(x,y)
#define _Log_CONCAT3(x,y,z) _Log_CONCAT2(x,_Log_CONCAT2(y,z))

/*  Helper macro to concatenate two symbols */
#define _Log__TOKEN2STRING_A(x) #x
#define _Log_TOKEN2STRING(x) _Log__TOKEN2STRING_A(x)

/* Helper macro to convert a log_ptr address to a pseudo-index, that needs to be divided by 4.
 * If needed, the division is handled by the sink
 */
#define _Log_LOG_PTR_TO_INDEX(ptr) (((uint32_t)(&ptr) - 0x08) & 0x7FFFC)

/* Macro to place meta string in a memory section separated by record separator */
#define _Log_APPEND_META_TO_FORMAT(opcode,                                     \
                                    file,                                      \
                                    line,                                      \
                                    level,                                     \
                                    module,                                    \
                                    format,                                    \
                                    nargs)                                     \
                                    _Log_TOKEN2STRING(opcode)     "\x1e"       \
                                    _Log_TOKEN2STRING(file)       "\x1e"       \
                                    _Log_TOKEN2STRING(line)       "\x1e"       \
                                    _Log_TOKEN2STRING(level)      "\x1e"       \
                                    _Log_TOKEN2STRING(module)     "\x1e"       \
                                    _Log_TOKEN2STRING(format)     "\x1e"       \
                                    _Log_TOKEN2STRING(nargs)

/* Place a string in trace format section named ".log_data" locally
 * This section must exist in the linker file
 */
#if defined(__IAR_SYSTEMS_ICC__)
#define _Log_PLACE_FORMAT_IN_SECTOR(name, opcode, level, module, format, nargs)\
            __root static const char name[] @ ".log_data" =                    \
            _Log_APPEND_META_TO_FORMAT(opcode,                                 \
                                       __FILE__,                               \
                                       __LINE__,                               \
                                       level,                                  \
                                       module,                                 \
                                       format,                                 \
                                       nargs);                                 \
           __root static const char * const _Log_CONCAT2(Ptr, name) @ _Log_TOKEN2STRING(_Log_CONCAT2(.log_ptr, module)) = name;
#elif defined(__TI_COMPILER_VERSION__) || (defined(__clang__) && defined(__ti_version__)) || defined(__GNUC__)
#define _Log_PLACE_FORMAT_IN_SECTOR(name, opcode, level, module, format, nargs)\
            static const char name[]                                           \
            __attribute__((used,section(".log_data"))) =                       \
            _Log_APPEND_META_TO_FORMAT(opcode,                                 \
                                        __FILE__,                              \
                                        __LINE__,                              \
                                        level,                                 \
                                        module,                                \
                                        format,                                \
                                        nargs);                                \
            static const char * const _Log_CONCAT2(Ptr, name)                  \
            __attribute__((used,section(_Log_TOKEN2STRING(_Log_CONCAT3(.log_ptr, __LINE__, module))))) = name;
#else
#error Incompatible compiler: Logging is currently supported by the following \
compilers: TI ARM Compiler, TI CLANG Compiler, GCC, IAR. Please migrate to a \
a supported compiler.
#endif

/*
 *  ======== Variadic macro workaround ========
 */
/*  Helper macro to count the number of arguments in __VA_ARGS_ */
#define _Log_NUMARGS(...) _Log_NUMARGS_A(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define _Log_NUMARGS_A(...) _Log_NUMARGS_B(__VA_ARGS__)
#define _Log_NUMARGS_B(_first, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N

/*
 * Helper to select arg/noarg variant macro since empty va_arg fails
 * when arguments are expected. Eg
 *   Log_VARIANT(test, A, 7, "Hello") -> test__noarg(A, 7, "Hello")
 *   Log_VARIANT(test, A, 7, "Hello %d", 42) -> test__arg1(A, 7, "Hello %d", 42)
 */
#define _Log_VARIANT(x, module, level, ...) \
    _Log_CONCAT2(x, _Log_NUMARGS_B(__VA_ARGS__, _arg8, _arg7, _arg6, _arg5, _arg4, _arg3, _arg2, _arg1, _noarg)) ( module, level, __VA_ARGS__ )

#define _Log_buf_B(module , level, format, data, size)                         \
    Log_MODULE_USE(module);                                                    \
        if ((Log_ENABLED & LogMod_ ## module.levels) &&                        \
        ((level) & LogMod_ ## module.levels)) {                                \
        _Log_PLACE_FORMAT_IN_SECTOR(_Log_CONCAT2(LogSymbol, __LINE__),         \
                                    LOG_OPCODE_BUFFER,                         \
                                    level,                                     \
                                    LogMod_ ## module,                         \
                                    format,                                    \
                                    0);                                        \
        LogMod_ ## module.buf(&LogMod_ ## module,                              \
                   (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),                          \
                   _Log_LOG_PTR_TO_INDEX(_Log_CONCAT3(Ptr, LogSymbol, __LINE__)), \
                   data,                                                       \
                   size);                                                      \
    }

/*
 * Redirects to cast all printf arguments to uintptr_t to avoid surprises if
 * passing wider values and the compiler silently allows it.
 */
#define _Log_printf__arg1(module, level, fmt, a0)                              \
    _Log_printf__arg(module, level, fmt, (uintptr_t)a0)
#define _Log_printf__arg2(module, level, fmt, a0, a1)                          \
    _Log_printf__arg(module, level, fmt, (uintptr_t)a0,                        \
                                         (uintptr_t)a1)
#define _Log_printf__arg3(module, level, fmt, a0, a1, a2)                      \
    _Log_printf__arg(module, level, fmt, (uintptr_t)a0,                        \
                                         (uintptr_t)a1,                        \
                                         (uintptr_t)a2)
#define _Log_printf__arg4(module, level, fmt, a0, a1, a2, a3)                  \
    _Log_printf__arg(module, level, fmt, (uintptr_t)a0,                        \
                                         (uintptr_t)a1,                        \
                                         (uintptr_t)a2,                        \
                                         (uintptr_t)a3)
#define _Log_printf__arg5(module, level, fmt, a0, a1, a2, a3, a4)              \
    _Log_printf__arg(module, level, fmt, (uintptr_t)a0,                        \
                                         (uintptr_t)a1,                        \
                                         (uintptr_t)a2,                        \
                                         (uintptr_t)a3,                        \
                                         (uintptr_t)a4)
#define _Log_printf__arg6(module, level, fmt, a0, a1, a2, a3, a4, a5)          \
    _Log_printf__arg(module, level, fmt, (uintptr_t)a0,                        \
                                         (uintptr_t)a1,                        \
                                         (uintptr_t)a2,                        \
                                         (uintptr_t)a3,                        \
                                         (uintptr_t)a4,                        \
                                         (uintptr_t)a5)
#define _Log_printf__arg7(module, level, fmt, a0, a1, a2, a3, a4, a5, a6)      \
    _Log_printf__arg(module, level, fmt, (uintptr_t)a0,                        \
                                         (uintptr_t)a1,                        \
                                         (uintptr_t)a2,                        \
                                         (uintptr_t)a3,                        \
                                         (uintptr_t)a4,                        \
                                         (uintptr_t)a5,                        \
                                         (uintptr_t)a6)
#define _Log_printf__arg8(module, level, fmt, a0, a1, a2, a3, a4, a5, a6, a7)  \
    _Log_printf__arg(module, level, fmt, (uintptr_t)a0,                        \
                                         (uintptr_t)a1,                        \
                                         (uintptr_t)a2,                        \
                                         (uintptr_t)a3,                        \
                                         (uintptr_t)a4,                        \
                                         (uintptr_t)a5,                        \
                                         (uintptr_t)a6,                        \
                                         (uintptr_t)a7)

#define _Log_printf__arg(module, level, ...)                                   \
    module.printf(&module,                                                     \
                       (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),                          \
                       _Log_LOG_PTR_TO_INDEX(_Log_CONCAT3(Ptr, LogSymbol, __LINE__)), \
                       _Log_NUMARGS(__VA_ARGS__),                              \
                       _Log_CDR_ARG(__VA_ARGS__))

#define _Log_printf__noarg(module, level, ...)                                 \
    module.printf(&module,                                                     \
                       (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),                          \
                       _Log_LOG_PTR_TO_INDEX(_Log_CONCAT3(Ptr, LogSymbol, __LINE__)), \
                       _Log_NUMARGS(__VA_ARGS__))

#define _Log_printf_B(opcode, module, level, ...)                              \
    Log_MODULE_USE(module);                                                    \
    if ((Log_ENABLED & LogMod_ ## module.levels) &&                            \
        ((level) & LogMod_ ## module.levels)) {                                \
        _Log_PLACE_FORMAT_IN_SECTOR(_Log_CONCAT2(LogSymbol, __LINE__),         \
                                    opcode,                                    \
                                    level,                                     \
                                    LogMod_ ## module,                         \
                                    _Log_CAR_ARG(__VA_ARGS__),                 \
                                    _Log_NUMARGS(__VA_ARGS__))                 \
        _Log_VARIANT(_Log_printf, LogMod_ ## module, level, __VA_ARGS__);      \
    }

/** @endcond */

/**
 *  @brief Construct a log event object
 *
 *  Use this marco to define a log event object. The object is global, and may
 *  be used in other files by invoking Log_EVENT_USE(name) there.
 *
 *  @param[in]  name       Event variable name, to be passed to Log_event API
 *  @param[in]  fmt        Restricted format string. Note `%s` is not supported.
 *                         Supported format specifiers include: `%c`, `%f`,
 *                         `%d`, `%x`
 */
#if defined(__IAR_SYSTEMS_ICC__)
#define Log_EVENT_DEFINE(name, fmt)                                         \
            __root const char LogSymbol_ ## name[] @ ".log_data" =          \
            _Log_APPEND_META_TO_FORMAT(LOG_EVENT_CONSTRUCT,                 \
                                        __FILE__,                           \
                                        __LINE__,                           \
                                        name,                               \
                                        global,                             \
                                        fmt,                                \
                                        0)

#elif defined(__TI_COMPILER_VERSION__) || (defined(__clang__) && defined(__ti_version__)) || defined(__GNUC__)
#define Log_EVENT_DEFINE(name, fmt)                                         \
            const char LogSymbol_ ## name[]                                 \
            __attribute__((used,section(".log_data"))) =                    \
            _Log_APPEND_META_TO_FORMAT(LOG_EVENT_CONSTRUCT,                 \
                                        __FILE__,                           \
                                        __LINE__,                           \
                                        name,                               \
                                        global,                             \
                                        fmt,                                \
                                        0)
#else
#error Incompatible compiler: Logging is currently supported by the following \
compilers: TI ARM Compiler, TI CLANG Compiler, GCC, IAR. Please migrate to a \
a supported compiler.
#endif

/**
 *  @brief Declare usage of a log event symbol defined elsewhere
 *
 *  Use this marco to declare a log event symbol for use. It's just a fancy
 *  `extern` macro.
 *
 *  @param[in]  name       Event variable name, to be passed to Log_event API
 */
#define Log_EVENT_USE(name) extern const char[] LogSymbol_ ## name;


/**
 *  @brief Log a continuous block of memory
 *
 *  Use this marco to send out runtime data from the device. This API should be
 *  used when the data is non constant and can only be derived at runtime. It
 *  is the most intrusive in terms of record overhead and instructions used.
 *
 *  @param[in]  module     Log module that the buffer originated from
 *  @param[in]  level      log level of type @ref Log_Level
 *  @param[in]  format     Restricted format string.
 *  @param[in]  data       Pointer to array of bytes (uint8_t *)
 *  @param[in]  size       Size in bytes of array to send
 *
 */
#define Log_buf(module, level, format, data, size)                             \
                _Log_GUARD_MACRO(_Log_buf_B(module , level, format, data, size))

/**
 *  @brief Emit a log event
 *
 *  Use this marco to enable printf style logging. This API offers the most
 *  flexibility as the construction of the format string is embedded in the call
 *  site of the API. It also supports true variadic arguments.
 *
 *  @param[in]  module     Log module that the buffer originated from
 *  @param[in]  level      Log level of type @ref Log_Level
 *  @param[in]  event      Event to be logged. Can be either a constructed
 *                         Log_EVENT symbol, or a printf-like format-string
 *  @param[in]  ...        Variable amount of arguments. Must match your
 *                         event or format-string.
 *
 * Examples:
 *   `Log_printf(MyTimingEvent, t.start)`, `Log_printf("Hello World")`, `Log_printf("Age: %d", 42)`
 *
 *  @note All arguments are treated as 32-bit wide and are promoted or
 *        truncated accordingly.
 */
#define Log_printf(module, level, ...) \
            _Log_GUARD_MACRO(_Log_printf_B(LOG_OPCODE_FORMATED_TEXT, module, level, __VA_ARGS__))

#define Log_event(module, level, ...) \
            _Log_GUARD_MACRO(_Log_printf_B(LOG_OPCODE_EVENT, module, level, __VA_ARGS__))

/* Macro for defining the version of the Log API */



#if defined(__IAR_SYSTEMS_ICC__)
#define _Log_DEFINE_LOG_VERSION(module, version)                                                \
    __root static const char _Log_CONCAT2(Log_ti_log_version, __COUNTER__)[] @ ".log_data" =    \
    _Log_APPEND_META_TO_FORMAT(LOG_OPCODE_VERSION,                                              \
                                                module,                                         \
                                                version,                                        \
                                                0,                                              \
                                                0,                                              \
                                                0,                                              \
                                                0)
#elif defined(__TI_COMPILER_VERSION__) || (defined(__clang__) && defined(__ti_version__)) || defined(__GNUC__)
#define _Log_DEFINE_LOG_VERSION(module, version)                                \
            static const char _Log_CONCAT2(Log_ti_log_version, __COUNTER__)[]   \
            __attribute__((used,section(".log_data"))) =                        \
            _Log_APPEND_META_TO_FORMAT(LOG_OPCODE_VERSION,                      \
                                                module,                         \
                                                version,                        \
                                                0,                              \
                                                0,                              \
                                                0,                              \
                                                0)
#else
#error Incompatible compiler: Logging is currently supported by the following \
compilers: TI ARM Compiler, TI CLANG Compiler, GCC, IAR. Please migrate to a \
a supported compiler.
#endif

/* Generate a symbol in the elf file that defines the version of the Log API */
_Log_DEFINE_LOG_VERSION(Log, Log_TI_LOG_VERSION);

#else /* ti_log_Log_ENABLE */

/*
 *  =================================================
 *  ======== Log Disabled (default behavior) ========
 *  =================================================
 */

#define Log_MODULE_DEFINE(...)
#define Log_MODULE_USE(...)
#define Log_EVENT_DEFINE(name, fmt)
#define Log_EVENT_USE(name, fmt)
#define Log_printf(module, level, ...)
#define Log_event(module, level, ...)
#define Log_buf(module, level, ...)
#define _Log_DEFINE_LOG_VERSION(module, version)

#endif /* ti_log_Log_ENABLE */

/*
 *  ======== Log_Level ========
 */
typedef enum Log_Level {
    Log_DEBUG = 1,                          /*! This should be the default level, reserved to be used by users to insert into applications for debugging. Exported libraries should avoid using this level. */
    Log_VERBOSE = 4,                        /*! This level is recommended to be used in libraries to emit verbose information */
    Log_INFO = 16,                          /*! This level is recommended to be used in libraries to emit simple information */
    Log_WARNING = 64,                       /*! This level is recommended to be used in libraries to emit warnings. It is up to the library developer to decide what constitutes a warning, but it should typially indicate something unexpected, but not something that leads to system failure */
    Log_ERROR = 256,                        /*! This level is recommended to be used in libraries to emit errors. Typically, this should be used when something has failed and the system is unable to continue correct operation */
    Log_ALL   = 1 + 4 + 16 + 64 + 256,      /*! This enables all levels */
    Log_ENABLED = 512                       /*! This is used to enable or disable the log module, independently of the log levels */
} Log_Level;

typedef const struct Log_Module Log_Module;

typedef void (*Log_printf_fxn)(const Log_Module *handle,
                              uint32_t header,
                              uint32_t index,
                              uint32_t numArgs,
                              ...);

typedef void (*Log_buf_fxn)(const Log_Module *handle,
                           uint32_t header,
                           uint32_t index,
                           uint8_t *data,
                           size_t size);

struct Log_Module {
    void                 *sinkConfig;
    const Log_printf_fxn  printf;
    const Log_buf_fxn     buf;
    uint32_t              levels;
};


/*! @} */
#if defined (__cplusplus)
}
#endif

#endif // ti_log_Log__include
