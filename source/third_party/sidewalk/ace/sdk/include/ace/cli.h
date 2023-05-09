/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
/**
 * @file cli.h
 */
#ifndef ACE_CLI_LIB_H
#define ACE_CLI_LIB_H

#include <ace/ace_status.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup  ACE_CLI_DEFINE
 * @{
 */
#define ACE_CLI_MAX_ARGV 30

/**
 * @brief  Defines the CLI function called after a command is processed.
 * @note   Returns ACE_STATUS_OK if parameters were good
 *          or ACE_STATUS_CLI_HELP_COMMAND to have cli module
 *          print out help for the command arguments are argc, argv
 */
typedef ace_status_t(aceCli_function_t)(int32_t, const char**);

/**
 * @brief Defines a modules command list.
 * @note  The list must be persistent for the lifetime of the module.
 */
typedef struct aceCli_moduleCmd {
    const char* commandMsg;
    const char* help;  // prints out on '?' or  '?' followed by command
    uint8_t flags;     // will say if terminal command or has subcommands
    // Only used if ACE_CLI_EXEC_SPAWN bit is set
    uint8_t stack_size_kb;        // Multiple of 1K.
    union {                       // type determined by flag(LEAF)
        aceCli_function_t* func;  // 0
        const struct aceCli_moduleCmd* subCommands; // 1
    } command;
} aceCli_moduleCmd_t;

/**
 * @brief Enum representing the bit position of a flag
 * @note  Use defines below, these should not be used directly.
 */
enum aceCli_flags_t {
    ACE_CLI_LEAF = 0,
    ACE_CLI_RESTRICTED = 1,
    ACE_CLI_EXEC_SPAWN = 2,
};

/**
 * @name ACE CLI Defines
 * @{
 */
/** @brief Set as an endpoint with an associated function. */
#define ACE_CLI_SET_LEAF (0x01 << ACE_CLI_LEAF)
/** @brief Spawn the leaf CLI function in a new task and wait for completion. */
#define ACE_CLI_SET_EXEC_SPAWN (0x01 << ACE_CLI_EXEC_SPAWN)
/** @brief Set as a sub command with further nesting. */
#define ACE_CLI_SET_FUNC 0
/** @brief Set as not restricted.Can be Or'd with other defines */
#define ACE_CLI_SET_NOT_RESTRICTED (0x01 << ACE_CLI_RESTRICTED)
/** @} */

/** @brief Must be last elem in any list of ace_cli_moduleCmd_t, denotes end of
 *         array */
#define ACE_CLI_NULL_MODULE \
    { .commandMsg = NULL, .help = NULL, .flags = 0, .command.func = NULL }

/** @brief Callback function called to check if device is locked */
typedef bool(aceCli_isLockedFunc_t)(void);

extern aceCli_isLockedFunc_t* g_aceCli_isLockedFunc;

/** @brief Callback Function that overrides printf for CLI output */
typedef int(aceCli_printOutputFunc_t)(const char* fmt, ...);

extern aceCli_printOutputFunc_t* g_aceCli_printOutputFunc;
/** @} */  // End of ACE_CLI_DEFINE

/**
 * @addtogroup ACE_CLI_API
 * @{
 */
/**
 * @brief   CLI entry function to process a command
 * @details Options: \n
 *          ?: Prints out help of all module_cmd_t in CLI_MODULE_CMDS \n
 *          <command> ? : Prints out help of all sub_cmds for command in
 *                        module_cmt_t->sub_cmds \n
 *          <command> invalidSubCommand : Prints out help of <command> \n
 *          <command> <subcommand Leaf> ? : Prints out help of sub_command \n
 *          <command> <subcommand Leaf> <anything>: Responsibility of func to
 *                                                  handle \n
 *
 * @param[in] msg: null deliminated string, note this message will be modified
 *
 * @note Parse msg by dilineating by line (each \\r\\n is one message to parse,
 * if no \\r\\n, then just one line) then separating by space.
 *
 * @return ACE_STATUS_OK if arguments were good, else CLI status from
 *         ace_status_t
 */
ace_status_t aceCli_processCmd(char* msg);

/**
 * @brief   Sets the command list for aceCli to search through
 *
 * @param[in] module_cmd_list: Expected valid module command list. The list must
 *                       be persistent for the lifetime of the module.
 * @return ACE_STATUS_OK on success, else ace_status_t
 */
ace_status_t aceCli_setCmdList(const aceCli_moduleCmd_t* module_cmd_list);

/**
 * @brief   Sets the islocked function.
 * @details If set, calls is_locked_func() and only calls
 *          restricted (ACE_CLI_RESTRICTED = 1) commands if this is false. If
 *          not set, all cli functions will always be called. This allows for
 *          a sudo-like experience of commands labeled restricted, and it is
 *          up to the implementer to determine what is_locked looks like.
 *
 * @param[in] is_locked_func Function to call to check if device is locked.
 *            Can be NULL.
 *
 * @return None
 */
void aceCli_setIsLocked(aceCli_isLockedFunc_t* is_locked_func);

/**
 * @brief   Overrides printf for another output function.
 * @details Set callback function for printing to console, by default
 *          printf() is used.
 *
 * @param[in] print_output_func The output to call instead of printf.
 *            Cannot be null.
 *
 * @note Must support format specifier %s and newline(CR,LF) characters.
 *
 * @return  None
 */
void aceCli_setPrintOutputFunc(aceCli_printOutputFunc_t* print_output_func);

#ifdef __cplusplus
}
#endif

/** @} */  // End of ACE_CLI_API
#endif     /* ACE_CLI_LIB_H */
