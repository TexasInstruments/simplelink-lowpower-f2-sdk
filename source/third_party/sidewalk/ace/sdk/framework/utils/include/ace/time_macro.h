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
/**
 * @file       time_macro.h
 * @brief      Commonly used macros to convert time units.
 */
#ifndef TIME_MACRO_H
#define TIME_MACRO_H

/**
 * @addtogroup ACE_FRAMEWORK_UTILS
 * @{
 */

/**
 * @brief      Converts seconds to milliseconds.
 */
#define ACE_UTILS_SEC_TO_MILLISEC(secs) ((secs)*1000)

/**
 * @brief      Converts seconds to microseconds.
 */
#define ACE_UTILS_SEC_TO_MICROSEC(secs) ((secs)*1000 * 1000)

/**
 * @brief      Converts minutes to seconds.
 */
#define ACE_UTILS_MIN_TO_SEC(min) ((min)*60)

/**
 * @brief      Converts hours to seconds.
 */
#define ACE_UTILS_HRS_TO_SEC(min) ((min)*60 * 60)

/** @} */

#endif  // TIME_MACRO_H
