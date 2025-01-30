/*
 * Copyright (c) 2019-2024 Texas Instruments Incorporated - http://www.ti.com
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
 *  ## Definitions ##
 *
 *  The following terms are used throughout the log documentation.
 *
 *  - `Log module`: A parameter passed to Log APIs to indicate which software
 *      module the log statement originated from. Modules also control the
 *      routing of logs to sinks.
 *  - `Log level`: The severity or importance of a given log statement.
 *  - `Log sink`: Also simply called a logger. This is a transport-specific
 *      logger implementation on the target side. <br> The Logging framework is
 *      flexible such that multiple sinks may exist in a single firmware image.
 *  - `Call site`: A specific invocation of a Log API in a given file or program.
 *  - `Record`: The binary representation of a log when it is stored or
 *      transported by a given sink. The log record format varies slightly with
 *      each sink depending on their implementation and needs. However, they all
 *      convey the same information.
 *  - Link Time Optimization (LTO): A feature of some toolchains that can
 *      significantly reduce the code overhead of the log statements through a
 *      process called dead code elimination. In order to maximize the benefits
 *      of this, all static libraries and application files should have LTO
 *      enabled.
 *
 *  ## Summary ##
 *
 *  The following sections describe the usage of the TI logging system
 *  implementation. This document will focus on the target (i.e. code that runs)
 *  on the embedded device. For associated PC tooling, please see the
 *  [README](../../../tools/log/tiutils/Readme.html) in the tools/log/tiutils/
 *  folder.
 *
 *  Design Philosophy:
 *
 *  * Logs target code should be as efficient as possible.
 *    * This means that Log APIs should minimize FLASH, RAM, and execution
 *      overhead.
 *  * Complexity should be pushed to host side tooling where possible.
 *    *  Even if this means that PC setup/tooling requirements are more complex.
 *  * Multiple log sink implementations shall be able to exist in a system.
 *    *  Where applicable, multiple instances should be supported (e.g. multiple
 *       circular buffers to collect logs)
 *  * It shall be possible to remove logging entirely using the preprocessor
 *  * Configuration of logging should be deferred to application compile and
 *    link time.
 *    * That means that the end application builder should make decisions about
 *       the logging settings. This means that TI provided libraries are not
 *       opinionated about what log levels should be enabled or how modules
 *       should be routed to sinks.
 *  * TI's logging system will leverage SysConfig out of the box, but it should
 *    be possible to configure and use logging easily without the needing
 *    SysConfig.
 *
 *  ## Stated Limitations ##
 *
 *  * It is not possible to control which log sink is used for each call site.
 *    Routing of logs is controlled at a module level.
 *  * A maximum of 8 arguments is supported for variadic APIs.
 *  * Users may not add log statements to functions that are direct dependencies
 *    of delegate log functions. Not respecting this will lead to recursive
 *    emissions of the log statements added to the dependencies. The
 *    dependencies of the configured delegate function depends on which log sink
 *    is selected for a given log module. For example, if an application using
 *    the UART log sink has a Log_printf() at the application level and another
 *    in HwiP_disable(), the first call will be delegated to
 *    LogSinkUART_printf(), which then calls HwiP_disable(). This, in turn,
 *    triggers another Log_printf() that loops back to LogSinkUART_printf(),
 *    creating an endless cycle. Note that SysConfig will prevent users from
 *    adding such log statements.
 *
 *  ## Anatomy of Log Statement ##
 *
 *  At the core of the logging implementation is heavy use of the C
 *  preprocessor. When reading an application, the Log APIs may look like
 *  function calls, but the preprocessor expands them heavily.
 *
 *  There are several ways in which the preprocessor is used.
 *
 *  ### Global ###
 *  1. To enable/disable logs globally. If `ti_log_Log_ENABLE` is not defined,
 *     all statements are removed by the preprocessor. This does not rely on LTO
 *     or any other optimization. It removes any traces of logs from the
 *     program.
 *
 *     This define is pushed to `ti_utils_build_compiler.opt` whenever any Log
 *     module is enabled in SysConfig.
 *
 *  ### Module ###
 *  1. To enable/disable logs by module. If
 *     `ti_log_Log_ENABLE_<MyLogModuleName>=1` is not defined, all statements
 *     using that Log module are removed by the preprocessor. This does not rely
 *     on LTO or any other optimization. Removing the define removes all traces
 *     of the log from the compiled code. Just defining the symbol name
 *     `ti_log_Log_ENABLE_<MyLogModuleName>` without setting it to 1 will not
 *     include Log statements during compilation.
 *
 *     These defines are automatically pushed to `ti_utils_build_compiler.opt`
 *     for all modules configured in SysConfig.
 *
 *     Some TI libraries that have logging enabled also contain multiple log
 *     modules. Enabling only a subset of Log modules via the preprocessor will
 *     not cause the Log statements associated with the remaining Log modules to
 *     be removed since this is a compile-time event. The Log statements
 *     associated with individual modules can be removed from logging-enabled TI
 *     libraries by recompiling those libraries without the module-level flags
 *     in question.
 *
 *  2. If a Log Module's #Log_Module.dynamicLevelsPtr is set to NULL, dynamic
 *     module log levels will be disabled for the module and only the constant
 *     defined 'levels' bitmap will be used for the Log Module. If a Log Module's
 *     #Log_Module.dynamicLevelsPtr is not NULL, it will be set to a separate,
 *     volatile, `levels` bitmap for the Log Module. This separate bitmap will
 *     replace the previous, constant defined 'levels' bitmap for all module log
 *     level comparisons. In doing so, the log module's log levels will be
 *     dynamic and can be changed at runtime by the logging macro
 *     #Log_MODULE_SET_LEVELS(). Changing a module's log levels as a runtime
 *     operation enables total control over which log levels are enabled.
 *
 *     As mentioned, if dynamic module log levels are enabled, comparisons done
 *     by the Log APIs on a module's `levels` bitmap will be at runtime instead
 *     of compile-time. This will both increase code size and increase logging
 *     execution time. Before enabling dynamic module log levels, these tradeoffs
 *     should be considered.
 *
 *  ### Per Log Statement ###
 *  1. (Level filtering): Insert the if statement that checks if the log level
 *     of the statement has been enabled in its module configuration. If the log
 *     level is not enabled, the process ends here.
 *
 *  2. (String declaration): Automate placement of constant strings, format
 *     strings, and pointers to these strings in the the nonloadable metadata
 *     section of the out file. This saves FLASH on the target. Each string
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
 *     Log API controls which sink its log statements will be routed to. The
 *     preprocessor does name expansion to look up the enabled log levels and
 *     selected sink function pointers from the module's configuration
 *     structure. NOTE: The used sink may require initialization. Please verify
 *     with the specific sink documentation on how to initialize the sink.
 *
 *  5. (Sink API Invocation): With the names resolved and levels checked, the
 *     logger is now ready to execute the sink function. This is done via
 *     function pointer.
 *
 *  An simplified pseudo-C implementation of what `Log_printf(LogModule_App1,
 *  Log_DEBUG, "Hello World!");` would expand to is shown below. This will not
 *  compile and is not extensive, just for illustration.
 *
 *  @code
 *  // Global log enable check, wrapped around each log site
 *  #if defined(ti_log_Log_ENABLE)
 *      #if ti_log_Log_ENABLE_LogModule_App1 == 1
 *          // Check if the level of this specific log statement has been enabled by the module
 *          if (LogMod_LogModule_App1.levels & level) {
 *                // Note that ^^ is the record separator.
 *                // Pack meta information into format string. This is stored off target.
 *                const string logMeta =
 *                  "LOG_OPCODE_FORMATED_TEXT^^"../../log.c"^^80^^Log_DEBUG^^LogMod_LogModule_App1^^"Hello World!"^^0";
 *                // Route log to the selected sink implementation.
 *                // This is done via function pointer.
 *                // The 0 indicates no arguments. If runtime arguments were
 *                // provided, they would follow.
 *                LogMod_LogModule_App1.printf(pointerToModuleConfig, 0);
 *          }
 *      #endif
 *  #endif
 *  @endcode
 *
 *  From here, the logger has transferred control over to the sink
 *  implementation, which varies based on the transport (e.g. circular buffer in
 *  memory or UART).
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
 *  controlled via the Log Modules panel in SysConfig, but can be changed in
 *  plain C code - despite this being generally discouraged. To do it, use the
 *  macro @ref Log_MODULE_DEFINE and pass the sink specific `Log_MODULE_INIT_`
 *  to the `init` parameter within the @ref Log_MODULE_DEFINE macro.
 *  `Log_MODULE_INIT_` uses delegate functions for printf and buf that are
 *  implementation dependent for each sink based on the available hardware and
 *  whether the function is implemented for optimization. An example for the
 *  LogBuf sink is below, it will do the following
 *
 *  1. Create a module called `LogModule_App1`.
 *  2. Initialize the module for use with the buffer based LogSink. Use buffer
 *     instance called `CONFIG_ti_log_LogSinkBuf_0`.
 *  3. Enable only the `Log_ERROR` level. Other logs will not be stored.
 *  4. Add the delegate function `LogSinkBuf_printfSingleton` for printf and `LogSinkBuf_bufDepInjection` for buf.
 *  6. Disable dynamic log levels with `NULL`.
 *
 *  @code
 *  #include <ti/log/Log.h>
 *  #include <ti/log/LogSinkBuf.h>
 *  Log_MODULE_DEFINE(LogModule_App1,
 *                    Log_MODULE_INIT_SINK_BUF(CONFIG_ti_log_LogSinkBuf_0,
 *                                             Log_ERROR, LogSinkBuf_printfSingleton, LogSinkBuf_bufDepInjection, NULL));
 *  @endcode
 *
 *  TI created libraries will never use @ref Log_MODULE_DEFINE. This leaves the
 *  choice of routing logs to their sinks to the end application writer. This is
 *  recommended when creating any static libraries to defer the final logging
 *  decisions to link time.
 *
 *  Each new module will instantiate a Log_Module structure with a `levels`
 *  bitmap and pointers to the selected sink implementation and sink
 *  configuration. See the @ref Log_Module structure for more information.
 *
 *  If dynamic module log levels are enabled, each new Log_Module structure will
 *  include a pointer to a separate, volatile, `levels` bitmap. This new bitmap
 *  will be used for runtime comparisons of a module's log level. Two logging
 *  macros can be used to both set and get a module's log level.
 *  Log_MODULE_SET_LEVELS can be used to set the log level of a specific log
 *  module and Log_MODULE_GET_LEVELS can be used to retrieve the log level of a
 *  specific log module.
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
 *  Additionally, depending whether dynamic module log levels are enabled or not,
 *  checking the level at each log statement will be evaluated at runtime or
 *  compile-time. Enabling dynamic module log levels or disabling
 *  Link-Time-Optimisation (LTO) will cause each log statement to perform a
 *  runtime check and will also increase the code size.
 *
 *  @code
 *  if ((level) & module.levels) {
 *      // Call Log API
 *  }
 *  @endcode
 *
 *  Optimization level `-flto` for both the TICLANG toolchain and GCC will
 *  typically be able to optimize the above statement.
 *
 *  ## Log Metadata ##
 *
 *  Each time a Log API is invoked, a metadata string is placed in the .out
 *  file. This string contains information about the API type, file, line
 *  module, level, and other information associated with the log call site. Each
 *  call site emits a string to a specific memory section called `.log_data`. In
 *  addition to this, a pointer to the string in .log_data is stored in another
 *  section called `.log_ptr`. Because the .log_ptr section is always in the
 *  same location, and each entry is the same size, an indexing-scheme can be
 *  used to refer to each log-string. Entry 0 in .log_ptr would point to the
 *  first string, entry 1 would point to the second string, etc. This Is
 *  necessary on some devices where transmitting an entire 32-bit address as a
 *  reference to the string is not possible, and instead an 8-bit index can be
 *  transmitted across the Log sink implementation instead. In order to use
 *  logging, this section should be added to the linker command file. By
 *  default, this section points to a nonloadable region of memory. Meaning that
 *  the metadata will not be loaded on the target device. Instead, the various
 *  logging visualization tools such as wireshark and TI ROV will read the
 *  metadata from this section and properly decode the log statements. The
 *  benefit of this approach is that very little memory is consumed on target.
 *  Additionally, the log transport only needs to store or send pointers to this
 *  meta section when a log API is called.
 *
 *  This approach minimizes the amount of memory consumed on device and bytes
 *  sent over the transport. This section can be loaded on target if desired or
 *  if you are creating a custom logger. The design does not preclude this.
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
 *  Sinks may vary in their implementation based on the nature of the storage or
 *  transport that they support, but they all have the following in common:
 *
 *  * Are named ti_log_LogSink<SinkName>. Where `<SinkName>` is the name of the
 *    sink.
 *  * Must implement the Log_printf and Log_buf APIs from this file.
 *  * Must provide _USE, _INIT, and _DEFINE macros.
 *
 *  In addition, some sinks require initialization. This will be listed in the
 *  documentation for the sink implementation. Sinks are closely tied to their
 *  associated host side tooling. Since the log statements are not parsed at all
 *  by the target code, this must be delegated to a program running on a PC.
 *  While the binary format of log records may vary across sink implementations,
 *  it is suggested that each log record contain:
 *
 *  1. Timestamp
 *  2. Pointer to metadata string. This will be looked up by the PC side tooling
 *     in the out file.
 *  3. Runtime arguments
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
 *  // If using SysConfig, it will be done automatically, or it can be done
 *  // manually:
 *  // Use helper macro from <ti/log/LogSinkBuf.h> to make a sink instance
 *  // (buffer + config) with 100 entries.
 *  Log_SINK_BUF_DEFINE(MyBufferSink, LogSinkBuf_Type_CIRCULAR, 100);
 *
 *  // Use helper macro from <ti/log/Log.h> to make a module pointing at the new
 *  // sink instance.
 *  // This example will enable all log levels and disable dynamic log levels
 *  Log_MODULE_DEFINE(MyModule, Log_MODULE_INIT_SINK_BUF(MyBufferSink, Log_ALL, LogSinkBuf_printfSingleton, LogSinkBuf_bufDepInjection, NULL))
 *
 *  // Some log sinks may require special initialization to configure hardware.
 *  // Refer to the documentation of the sink you wish to use. For example,
 *  // LogSinkITM must be initialised like this before it can be used:
 *  // LogSinkITM_init();
 *
 *  // Invoke one of the log APIs you want to use for either formatted strings
 *  // or large buffers
 *  Log_printf(MyModule, Log_DEBUG, "The answer is %d", 42);
 *  uint8_t buffer[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 *  Log_buf(MyModule,
 *          Log_VERBOSE,
 *          "The buffer values are:",
 *          buffer,
 *          sizeof(buffer));
 *  @endcode
 *
 *  @anchor ti_log_LOG_Examples
 *  ### Examples ###
 *  * @ref ti_utils_LOG_Example_printf "Log printf"
 *  * @ref ti_utils_LOG_Example_buf "Logging buffers"
 *
 *  @anchor ti_utils_LOG_Example_printf **Log Printf**:
 *
 *  The following example demonstrates use of the #Log_printf API in code. Log
 *  will embed the format string in the call site and will take arguments using
 *  varadic arguments.
 *
 *  @code
 *  Log_printf(MyModule, Log_DEBUG, "Hello World!");
 *  @endcode
 *
 *  The arguments are type-cast to a uintptr_t, which is an unsigned integer
 *  type. This limits the supported format specifiers to the following:
 *      - Unsigned decimal integer: \%u
 *      - Unsigned hexadecimal integer: \%x
 *      - Unsigned hexadecimal integer (capital letters): \%X
 *      - Character: \%c
 *      - Signed decimal integer for positive values: \%i, \%d
 *      - Signed octal for positive values: \%o
 *
 *  @anchor ti_utils_LOG_Example_buf **Log Buf**:
 *
 *  The following example demonstrates use of the Log buf API in code.
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
 *  Log_buf(ti_log_LogMain,
 *          Log_DEBUG,
 *          "The contents of bufferToLog are: ",
 *          bufferToLog,
 *          sizeof(bufferToLog));
 *  @endcode
 *
 *  @anchor ti_utils_LOG_Example_guide **Log API usage**:
 *
 *  For a uniform experience with the logging tool, users are recommended to
 *  follow certain guidelines regarding the Log API. Typical use-cases for each
 *  API call is described below
 *
 *  #### Log_printf ####
 *
 *  #Log_printf should be the default mechanism for generating a log statement
 *  within an application. Along with the log levels, #Log_printf should be used
 *  to communicate debug information as a formatted string, which accepts
 *  variadic arguments. In this case, a pointer to the string and the arguments
 *  themselves are transported by the Log sink.
 *
 *  @code
 *  Log_printf(MyLibraryLogModule,
 *             Log_ERROR,
 *             "Library function received illegal argument: %d",
 *             arg);
 *  @endcode
 *
 *  #### Log_buf ####
 *
 *  When the debug-information to be emitted is a large amount of dynamic data,
 *  and is not suitable as an argument to printf, then #Log_buf should be used.
 *  #Log_buf can transport the contents of large dynamic buffers, and as a
 *  consequence has a larger overhead and should be used sparsely.
 */

#ifndef ti_log_Log__include
#define ti_log_Log__include

/*! @ingroup ti_log_LOG */
/*@{*/

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

#if defined(DOXYGEN) || ti_log_Log_ENABLE
/*
 *  =============================
 *  ======== Log Enabled ========
 *  =============================
 */

/*!
 * @brief Log version
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
 *   `Log_MODULE_DEFINE(MyDriver,
 *                      Log_MODULE_INIT_SINK_ITM(Log_DEBUG | Log_ERROR,
 *                                               LogSinkBuf_printfSingleton,
 *                                               LogSinkBuf_bufSingleton,
 *                                               Null))`
 *
 *  Perhaps you used the LogSinkBuf.h helper macro which needs a unique name
 *  per instance and made a separate buffer for critical errors:
 *
 *   `Log_MODULE_DEFINE(MyCritical,
 *                      Log_MODULE_INIT_SINK_BUF(criticalBuf,
 *                                               Log_ERROR,
 *                                               LogSinkBuf_printfDepInjection,
 *                                               LogSinkBuf_bufDepInjection,
 *                                               NULL)`
 *
 *  You would use this in your application via
 *      `Log_printf(MyCritical, Log_ERROR, "Oops")`
 */
#define Log_MODULE_DEFINE(name, init) const Log_Module LogMod_ ## name = init

/**
 *  @brief Defines Log module as weak
 *
 *  If there are multiple modules containing Log statements per library,
 *  special care must be taken not to create link-time failures.
 *  Whether Log statements from a library are present in the final binary is
 *  determined by the library configuration the application links against
 *  (instrumented vs uninstrumented).
 *  Each Log statement has a link-time dependency on its Log module. Enabling
 *  only a subset of Log modules contained within the library will cause any
 *  Log statements from other Log modules of that library to fail at link-time.
 *  This is avoided by declaring a weak instance of each Log module in C code
 *  that is compiled into the library. That way, the SysConfig-generated Log
 *  module definitions will override the weak library ones but they are there
 *  if SysConfig does not define that particular module.
 *
 *  @param[in]  name    Name of the log module. Gets prefixed with `LogMod_`.
 *  @param[in]  init    Initialization value of the Log_Module struct.
 */
#if defined(DOXYGEN) || defined(__IAR_SYSTEMS_ICC__)
#define Log_MODULE_DEFINE_WEAK(name, init) const __weak Log_Module LogMod_ ## name = init
#elif defined(__TI_COMPILER_VERSION__) || (defined(__clang__) && defined(__ti_version__)) || defined(__GNUC__)
#define Log_MODULE_DEFINE_WEAK(name, init) const Log_Module LogMod_ ## name __attribute__((weak)) = init
#else
#error "Incompatible compiler: Logging is currently supported by the following \
compilers: TI ARM Compiler, TI CLANG Compiler, GCC, IAR. Please migrate to a \
supported compiler."
#endif

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

/* This macro protects against side effects of the C preprocessor expansion
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
#error "Incompatible compiler: Logging is currently supported by the following \
compilers: TI ARM Compiler, TI CLANG Compiler, GCC, IAR. Please migrate to a \
supported compiler."
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

/*
 *  ======== Module-level preprocessor include macros ========
 */

/* Helper macro to extract the second argument of a variable number of input
 * args
 */
#define _Log_SECOND_ARG(x, y, ...) y

/* Temporary token name.
 * Name must end in "1" for preprocessor substitution below to work.
 */
#define _Log_TOKEN_1 0,

/* Helper macro to check whether a symbol is defined with a non-zero value.
 * If x is a preprocessor define, the conversion below shows the macro output:
 * x = 0 -> 0
 * x = 1 -> 1
 * x (no value) -> 0
 * (undefined) -> 0
 */
#define _Log_DEFINED(x)     _Log_DEFINED_A(x)

/* If x is 1, _Log_TOKEN_##y turns into _Log_TOKEN_1 and is replaced with "0,"
 * If x is anything else, _Log_TOKEN_##y turns into _Log_TOKEN_y.
 */
#define _Log_DEFINED_A(y)   _Log_DEFINED_B(_Log_TOKEN_##y)

/* If z is "0,", _Log_SECOND_ARG is called with the triplet "0, 1, 0" and
 * selects the second item in it, 1.
 * If z is anything else, _Log_SECOND_ARG is called with the tuple "z 1, 0" and
 * selects the second item in it, 0.
 */
#define _Log_DEFINED_B(z)   _Log_SECOND_ARG(z 1, 0)

/* Empty Log buf macro to use when a log module is not enabled in the
 * preprocessor during compilation
 */
#define _Log_buf_C_0(module, level, format, data, size)

/* Log_buf macro to use when a log module is enabled in the preprocessor during
 * compilation.
 */
#define _Log_buf_C_1(module, level, format, data, size)                                                 \
    _Log_GUARD_MACRO(                                                                                   \
        Log_MODULE_USE(module);                                                                         \
            if ((level) & LogMod_ ## module.levels) {                                                   \
                _Log_PLACE_FORMAT_IN_SECTOR(_Log_CONCAT2(LogSymbol, __LINE__),                          \
                                            LOG_OPCODE_BUFFER,                                          \
                                            level,                                                      \
                                            LogMod_ ## module,                                          \
                                            format,                                                     \
                                            0);                                                         \
                LogMod_ ## module.buf(&LogMod_ ## module,                                               \
                        (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),                                   \
                        (uint32_t)&_Log_CONCAT3(Ptr, LogSymbol, __LINE__),                              \
                        data,                                                                           \
                        size);                                                                          \
            }                                                                                           \
    )

/* First level indirection macro for Log_buf that delegates between an empty
 * implementation and the actual log emission based on whether a module is
 * enabled in the preprocessor during compilation.
 *
 * The _Log_DEFINED() macro generates a token output of [0, 1] that is then
 * concatenated with "_Log_buf_C" to form the correct delegate macro name.
 *
 * The expected module define name is ti_log_Log_ENABLE_ | <module> and must be
 * set to 1. E.g. "-Dti_log_Log_ENABLE_MyLogModule=1". Just defining the symbol in
 * the preprocessor will not emit any logs.
 */
#define _Log_buf_B(module, level, format, data, size) \
    _Log_CONCAT2(_Log_buf_C, _Log_DEFINED(ti_log_Log_ENABLE_ ## module))(module, level, format, data, size)

/* Redirects to cast all printf arguments to uintptr_t to avoid surprises if
 * passing wider values and the compiler silently allows it.
 *
 *  These redirects also allow us to select which delegate function pointer to
 *  load from the #Log_Module. There are dedicated wrappers for certain printf
 *  argument counts.
 *
 *  - 0-3 args redirect to dedicated functions to avoid loading the argument
 *    count when LTO is enabled.
 *  - 4-8 args redirect to a generic implementation.
 *
 *  The 4-8 args break point is a compromise between how often the wrapper can
 *  be expected to be used in an average application and the constant overhead
 *  associated with providing it at all. There is also overhead per Log_Module
 *  for each additional arg count wrapper used. This overhead is eliminated when
 *  LTO is enabled though.
 */
#define _Log_printf__arg1(module, level, fmt, a0)                              \
    module.printf1(&module,                                                    \
                   (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),               \
                   (uint32_t)&_Log_CONCAT3(Ptr, LogSymbol, __LINE__),          \
                   (uintptr_t)a0)
#define _Log_printf__arg2(module, level, fmt, a0, a1)                          \
    module.printf2(&module,                                                    \
                   (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),               \
                   (uint32_t)&_Log_CONCAT3(Ptr, LogSymbol, __LINE__),          \
                   (uintptr_t)a0,                                              \
                   (uintptr_t)a1)
#define _Log_printf__arg3(module, level, fmt, a0, a1, a2)                      \
    module.printf3(&module,                                                    \
                   (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),               \
                   (uint32_t)&_Log_CONCAT3(Ptr, LogSymbol, __LINE__),          \
                   (uintptr_t)a0,                                              \
                   (uintptr_t)a1,                                              \
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
                  (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),                \
                  (uint32_t)&_Log_CONCAT3(Ptr, LogSymbol, __LINE__),           \
                  _Log_NUMARGS(__VA_ARGS__),                                   \
                  _Log_CDR_ARG(__VA_ARGS__))

#define _Log_printf__noarg(module, level, ...)                                 \
    module.printf0(&module,                                                    \
                   (uint32_t)&_Log_CONCAT2(LogSymbol, __LINE__),               \
                   (uint32_t)&_Log_CONCAT3(Ptr, LogSymbol, __LINE__))

/* Empty Log_printf macro to use when a log module is not enabled in the
 * preprocessor during compilation
 */
#define _Log_printf_C_0(opcode, module, level, ...)

/* Log_printf macro to use when a log module is enabled in the preprocessor during
 * compilation.
 */
#define _Log_printf_C_1(opcode, module, level, ...)                             \
    _Log_GUARD_MACRO(                                                           \
        Log_MODULE_USE(module);                                                 \
        if (((LogMod_ ## module.dynamicLevelsPtr != NULL) && ((level) & *LogMod_ ## module.dynamicLevelsPtr)) ||    \
            ((level) & LogMod_ ## module.levels)) {                             \
            _Log_PLACE_FORMAT_IN_SECTOR(_Log_CONCAT2(LogSymbol, __LINE__),      \
                                        opcode,                                 \
                                        level,                                  \
                                        LogMod_ ## module,                      \
                                        _Log_CAR_ARG(__VA_ARGS__),              \
                                        _Log_NUMARGS(__VA_ARGS__))              \
            _Log_VARIANT(_Log_printf, LogMod_ ## module, level, __VA_ARGS__);   \
        }                                                                       \
    )

/* First level indirection macro for Log_printf that delegates between an empty
 * implementation and the actual log emission based on whether a module is
 * enabled in the preprocessor during compilation.
 *
 * The _Log_DEFINED() macro generates a token output of [0, 1] that is then
 * concatenated with "_Log_buf_C" to form the correct delegate macro name.
 *
 * The expected module define name is ti_log_Log_ENABLE_ | <module> and must be
 * set to 1. E.g. "-Dti_log_Log_ENABLE_MyLogModule=1". Just defining the symbol
 * in the preprocessor will not emit any logs.
 */
#define _Log_printf_B(opcode, module, level, ...) \
    _Log_CONCAT2(_Log_printf_C, _Log_DEFINED(ti_log_Log_ENABLE_ ## module))(opcode, module, level, __VA_ARGS__)

/** @endcond */

/**
 *  @brief Log a continuous block of memory
 *
 *  Use this macro to send out runtime data from the device. This API should be
 *  used when the data is non constant and can only be derived at runtime. It
 *  is the most intrusive in terms of record overhead and instructions used.
 *
 *  @param[in]  module     Log module that the buffer originated from
 *  @param[in]  level      Log level of type @ref Log_Level
 *  @param[in]  format     Restricted format string
 *  @param[in]  data       Pointer to array of bytes (uint8_t *)
 *  @param[in]  size       Size in bytes of array to send
 *
 */
#define Log_buf(module, level, format, data, size) _Log_buf_B(module, level, format, data, size)

/**
 *  @brief Log an event with a printf-formatted string
 *
 *  Use this macro to enable printf style logging. This API offers the most
 *  flexibility as the construction of the format string is embedded in the call
 *  site of the API. It also supports true variadic arguments.
 *
 *  @param[in]  module     Log module that the buffer originated from
 *  @param[in]  level      Log level of type @ref Log_Level
 *  @param[in]  ...        Variable amount of arguments. Must match your
 *                         event or format-string.
 *
 *  Examples:
 *   - `Log_printf(MyModule, Log_INFO, "Hello World")`
 *   - `Log_printf(MyModule, Log_DEBUG, "Age: %d", 42)`
 *
 *  @note All arguments are treated as 32-bit wide and are promoted or
 *        truncated accordingly.
 */
#define Log_printf(module, level, ...) _Log_printf_B(LOG_OPCODE_FORMATED_TEXT, module, level, __VA_ARGS__)

/**
 *  @cond NODOC
 *  Macro for defining the version of the Log API
 */
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
#error "Incompatible compiler: Logging is currently supported by the following \
compilers: TI ARM Compiler, TI CLANG Compiler, GCC, IAR. Please migrate to a \
supported compiler."
#endif

/* Generate a symbol in the elf file that defines the version of the Log API */
_Log_DEFINE_LOG_VERSION(Log, Log_TI_LOG_VERSION);

/*! @endcond */

/**
 *  @brief Set a log module's log level bitmask
 *
 *  If dynamic module log levels is enabled, use this macro to set a specific
 *  log module's log level bitmask as a runtime operation.
 *
 *  @param[in]  module     Name of the log module. Gets prefixed with `LogMod_`.
 *  @param[in]  levels     Log level bitmask containing zero or more @ref Log_Level
 *
 */
#define Log_MODULE_SET_LEVELS(module, levels)               \
    _Log_GUARD_MACRO(                                       \
        Log_MODULE_USE(module);                             \
        if (LogMod_ ## module.dynamicLevelsPtr != NULL) {   \
            *LogMod_ ## module.dynamicLevelsPtr = levels;   \
        }                                                   \
    )

/**
 *  @brief Get a log module's log level bitmask
 *
 *  If dynamic module log levels is enabled, use this macro to get a specific
 *  log module's log level bitmask as a runtime operation.
 *
 *  @param[in]  module     Name of the log module. Gets prefixed with `LogMod_`.
 *
 */
#define Log_MODULE_GET_LEVELS(module)                       \
    _Log_GUARD_MACRO(                                       \
        Log_MODULE_USE(module);                             \
        if (LogMod_ ## module.dynamicLevelsPtr != NULL) {   \
            *LogMod_ ## module.dynamicLevelsPtr;            \
        }                                                   \
        else {                                              \
            LogMod_ ## module.levels;                       \
        }                                                   \
    )

#else /* defined(DOXYGEN) || ti_log_Log_ENABLE */

/*
 *  =================================================
 *  ======== Log Disabled (default behavior) ========
 *  =================================================
 */

#define Log_MODULE_DEFINE(...)
#define Log_MODULE_DEFINE_WEAK(name, init)
#define Log_MODULE_USE(...)
#define Log_printf(module, level, ...)
#define Log_buf(module, level, ...)
#define _Log_DEFINE_LOG_VERSION(module, version)
#define Log_MODULE_SET_LEVELS(module, levels)
#define Log_MODULE_GET_LEVELS(module)

#endif /* defined(DOXYGEN) || ti_log_Log_ENABLE */



/**
 * @brief Log level bitmask values
 *
 * One of these enum values should be used in each #Log_printf and #Log_buf call
 * and one or more should be used in each #Log_Module definition.
 */
typedef enum Log_Level {
    /*! This should be the default level, reserved to be used by users to insert
     *  into applications for debugging. Exported libraries should avoid using
     *  this level.
     */
    Log_DEBUG = 1 << 0,

    /*! This level is recommended to be used in libraries to emit verbose
     *  information about the operation of the system and system state. This
     *  level is typically used for very frequently emitted logs or for very
     *  detailed context and state information.
     *
     *  Enabling this level in multiple #Log_Module may impact the operation of
     *  the application or saturate available log sink bandwidth and cause
     *  stalls in log record emission.
     */
    Log_VERBOSE = 1 << 2,

    /*! This level is recommended to be used in libraries to emit simple
     *  information about the operation of the system and the system state.
     *
     *  Enabling this level in multiple #Log_Module may impact the operation of
     *  the application or saturate available log sink bandwidth and cause
     *  stalls in log record emission.
     */
    Log_INFO = 1 << 4,

    /*! This level is recommended to be used in libraries to emit warnings. It
     *  should typically indicate something unexpected, but not something that
     *  automatically leads to system failure. Warnings may be generated during
     *  the regular operation of an application. Frequent warnings may indicate
     *  a temporary or systemic issue with the system that may require
     *  intervention such as a reset or alteration to the software or hardware
     *  design.
     *
     *  The typical usecase for this level is a recoverable failure such as a
     *  failed heap allocation or CRC error in a wireless transmission.
     *
     *  Enabling this level in a #Log_Module will not impact regular device
     *  operation or impact available log sink bandwidth.
     */
    Log_WARNING = 1 << 6,

    /*! This level is recommended to be used in libraries to emit errors.
     *  Typically, this should be used when something has failed and the system
     *  is unable to continue correct operation. Errors are not generated during
     *  regular system operation. An error record being generated always
     *  indicates an issue with system that requires intervention. The
     *  intervention depends on the error but could be a system reset or changes
     *  to the software or hardware of the system.
     *
     *  Enabling this level in a #Log_Module will not impact regular device
     *  operation or impact available log sink bandwidth.
     */
    Log_ERROR = 1 << 8,

    /*! This value enables all levels. Should only be used in #Log_Module
     *  definitions.
     */
    Log_ALL = Log_DEBUG + Log_VERBOSE + Log_INFO + Log_WARNING + Log_ERROR,

    /*! This value disables all levels when used by itself. Should only be used
     *  in #Log_Module definitions.
     */
    Log_NONE = 0,
} Log_Level;

typedef const struct Log_Module Log_Module;

typedef void (*Log_printf_fxn)(const Log_Module *handle,
                              uint32_t header,
                              uint32_t headerPtr,
                              uint32_t numArgs,
                              ...);

typedef void (*Log_printfN_fxn)(const Log_Module *handle,
                              uint32_t header,
                              uint32_t headerPtr,
                              ...);

typedef void (*Log_buf_fxn)(const Log_Module *handle,
                           uint32_t header,
                           uint32_t headerPtr,
                           uint8_t *data,
                           size_t size);

/*!
 *  @brief      Log module
 *
 *  The application must not access any member variables of this structure!
 */
struct Log_Module {
    void                 *sinkConfig;       /*!< Pointer to the selected sink implementation and sink configuration */
    const Log_printf_fxn  printf;           /*!< Pointer to printf implementation with 4 to 8 arguments*/
    const Log_printfN_fxn printf0;          /*!< Pointer to printf implementation with 0 arguments */
    const Log_printfN_fxn printf1;          /*!< Pointer to printf implementation with 1 arguments */
    const Log_printfN_fxn printf2;          /*!< Pointer to printf implementation with 2 arguments */
    const Log_printfN_fxn printf3;          /*!< Pointer to printf implementation with 3 arguments */
    const Log_buf_fxn     buf;              /*!< Pointer to buf implementation */
    uint32_t              levels;           /*!< Log levels bitmap */
    uint32_t* const       dynamicLevelsPtr; /*!< Pointer to a new volatile levels bitmap */
};

/*! @} */
#if defined (__cplusplus)
}
#endif

#endif // ti_log_Log__include
