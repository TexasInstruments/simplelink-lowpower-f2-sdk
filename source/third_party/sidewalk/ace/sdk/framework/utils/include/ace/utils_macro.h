/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 *
 * PORTIONS OF THIS FILE ARE BASED ON UTIL.H AVAILABLE AT: https://git.io/JvWFL
 * THE ORIGINAL FILE HEADER IS RETAINED BELOW.
 */
/*
 * Copyright (c) 2011-2014, Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file       utils_macro.h
 * @brief      Utility macros for general usage.
 */
#ifndef UTILS_MACRO_H
#define UTILS_MACRO_H

#include <stddef.h>
#include <stdbool.h>

#include "utils_macro_map.h"

/**
 * @addtogroup ACE_FRAMEWORK_UTILS
 * @{
 */

/**
 * @brief      Returns the number of elements in `array`.
 * @note       Where possible, will result in a compiler error if `array` is
 *             not an array (e.g. a pointer).
 *
 * @param      array  The array to calculate the number of elements of.
 *
 * @return     The number of elements in `array`.
 */
#define ACE_ARRAY_SIZE(array) \
    ((size_t)_IS_ARRAY(array) + (sizeof(array) / sizeof((array)[0])))

/**
 * @brief      Given a pointer to a member contained within a struct, obtain a
 *             pointer to the member's "container" -- i.e. the struct itself.
 *
 * @param      member_ptr   A pointer to a member within a struct.
 * @param      struct_type  The type of the struct.
 * @param      member_name  The name of the member as defined within
 *                          `struct_type`.
 *
 * @return     A pointer to a container (struct) of type `struct_type`.
 */
#define ACE_CONTAINER_OF(member_ptr, struct_type, member_name) \
    ((struct_type*)(((char*)(member_ptr)) - offsetof(struct_type, member_name)))

/**
 * @brief      Round input unsigned long up to the next multiple of `align`.
 * @note       `align` must be a power of two.
 */
#define ACE_CEIL2(x, align)                                        \
    (x == 0 ? align                                                \
            : (((unsigned long)(x) + ((unsigned long)(align)-1)) & \
               ~((unsigned long)(align)-1)))

/**
 * @brief      Round input unsigned long down to the closest multiple of
 *             `align`.
 * @note       `align` must be a power of two.
 */
#define ACE_FLOOR2(x, align) \
    ((unsigned long)(x) & ~((unsigned long)(align)-1))

/**
 * @brief      Returns the larger value of the two provided expressions.
 * @note       The provided expressions should not have side effects, as they
 *             may be evaluated multiple times.
 */
#define ACE_MAX(a, b) (((a) > (b)) ? (a) : (b))

/**
 * @brief      Returns the smaller value of the two provided expressions.
 * @note       The provided expressions should not have side effects, as they
 *             may be evaluated multiple times.
 */
#define ACE_MIN(a, b) (((a) < (b)) ? (a) : (b))

/**
 * @brief      Checks if the provided macro is "enabled" (has a value of 1) in
 *             compiler-visible expressions, and so it can be used in normal
 *             code.
 * @details    This is intended to replace `#ifdef` style code of the form:
 *             @code{.c}
 *             #if defined(MY_CONFIG_FEATURE) && (MY_CONFIG_FEATURE == 1)
 *             // Do something
 *             #endif
 *             @endcode
 *
 *             ... with code that is cleaner, and can be understood by the
 *             compiler (and so it can be used anywhere normal code can be):
 *             @code{.c}
 *             if (ACE_IS_FLAG_ENABLED(MY_CONFIG_FEATURE))
 *                 // Do something
 *             @endcode
 *
 * @param      config_macro  The macro to evaluate at compile-time (as opposed
 *                           to at preprocessing-time).
 *
 * @return     `true` if the macro is "enabled" (has a value of 1), `false`
 *             otherwise.
 */
#define ACE_IS_FLAG_ENABLED(config_macro) _IS_ENABLED1(config_macro)

/**
 * @brief      Conditionally inserts code depending on the evaluation of
 *             `_flag`.
 * @details    Usage example:
 *             @code{.c}
 *             struct my_struct_s my_struct = {
 *                 .data = my_ptr,
 *                 ACE_IF_FLAG_ENABLED(CONFIG_MYFEATURE, (.wipe_data = true),
 *                                     (.wipe_data = false))
 *             };
 *             @endcode
 *
 * @param      _flag       The flag to be evaluated .
 * @param      _if_1_code  The code to insert if `_flag` is true.
 * @param      _else_code  The code to insert if `_flag` is false.
 */
#define ACE_IF_FLAG_ENABLED(_flag, _if_1_code, _else_code) \
    _COND_CODE_1(_flag, _if_1_code, _else_code)

/**
 * @brief      Conditionally inserts code depending on the evaluation of
 *             `_flag`.
 *
 * @details    See @ref ACE_IF_FLAG_ENABLED for details.
 */
#define ACE_IF_FLAG_DISABLED(_flag, _if_0_code, _else_code) \
    _COND_CODE_0(_flag, _if_0_code, _else_code)

/**
 * @brief      Gets the first arg from a variable list of args.
 */
#define ACE_GET_ARG1(arg1, ...) arg1

/**
 * @brief      Gets the second arg from a variable list of args.
 */
#define ACE_GET_ARG2(arg1, arg2, ...) arg2

/**
 * @brief      Strips the first arg from a variable list of args.
 */
#define ACE_GET_ARGS_LESS_1(val, ...) __VA_ARGS__

/** @} */

/*
 * Helper macros, used to implement the public ones above.
 */

#define _ZERO_OR_COMPILE_ERROR(cond) ((int)sizeof(char[1 - 2 * !(cond)]) - 1)

// These rely on non-standard features. Default to "unsupported", and we'll
// redefine them later if we can support them.
#define _IS_ARRAY(array)

// clang-format off
#ifdef __has_builtin
    #if __has_builtin(__builtin_types_compatible_p)
        #undef _IS_ARRAY
        #define _IS_ARRAY(array)                                  \
            _ZERO_OR_COMPILE_ERROR(!__builtin_types_compatible_p( \
                __typeof__(array), __typeof__(&(array[0]))))
    #else
    #endif
#endif
// clang-format on

#define _IS_ENABLED1(config_macro) _IS_ENABLED2(_XXXX##config_macro)
#define _XXXX1 _YYYY,
#define _IS_ENABLED2(one_or_two_args) _IS_ENABLED3(one_or_two_args true, false)
#define _IS_ENABLED3(ignore_this, val, ...) val

#define __DEBRACKET(...) __VA_ARGS__
#define __GET_ARG2_DEBRACKET(ignore_this, val, ...) __DEBRACKET val

#define _COND_CODE(one_or_two_args, _if_code, _else_code) \
    __GET_ARG2_DEBRACKET(one_or_two_args _if_code, _else_code)

#define _COND_CODE_1(_flag, _if_1_code, _else_code) \
    _COND_CODE(_XXXX##_flag, _if_1_code, _else_code)

#define _COND_CODE_0(_flag, _if_0_code, _else_code) \
    _COND_CODE(_ZZZZ##_flag, _if_0_code, _else_code)

#define _ZZZZ0 _YYYY,

#endif  // UTILS_MACRO_H
