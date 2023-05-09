/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file hal_fr.h
 *
 * @brief ACE Factory Reset HAL builds on top of platform specific factory reset
 * callbacks.
 * @details Factory Reset HAL APIs will be invoked by the Factory Reset Middleware.
 *          For porting suggestions, guides, or typical implementation details, check
 *          the Factory Reset porting guide.
 * @ref FACTORY_RESET_PORTING_GUIDE
 * @addtogroup ACE_HAL_FR
 * @{
 */

#ifndef ACE_HAL_FR_H_
#define ACE_HAL_FR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <ace/ace_status.h>

/**
 * @brief Factory Reset type
 */
typedef enum {
    /** Any product-defined factory reset other than @ref ACEFRHAL_DEEP */
    ACEFRHAL_SOFT = 0,
    /** Full factory reset */
    ACEFRHAL_DEEP,
    /** Used by Middleware only and does not need to be implemented by HAL */
    ACEFRHAL_INVALID
} aceFrHal_type_t;

/**
 * @brief Initialize Factory Reset Platform HAL
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t otherwise
 */
ace_status_t aceFrHal_open(void);

/**
 * @brief Checks whether it's safe to proceed with a factory reset
 * @param[in] type the type of factory reset being attempted
 * @return true to proceed factory reset operation, else false
 */
bool aceFrHal_check(aceFrHal_type_t type);

/**
 * @brief Performs any preparation needed prior to a factory reset
 * @param[in] type the type of factory reset being attempted
 */
void aceFrHal_preOperation(aceFrHal_type_t type);

/**
 * @brief Performs any final operations needed after a factory reset was performed
 * @param[in] type the type of factory reset being attempted
 */
void aceFrHal_postOperation(aceFrHal_type_t type);

/**
 * @brief De-Initialize Factory Reset Platform HAL
 * @return ACE_STATUS_OK on success, and a negative value from @ref ace_status_t otherwise
 */
ace_status_t aceFrHal_close(void);

#ifdef __cplusplus
}
#endif

#endif /* ACE_HAL_FR_H_ */
/** @} */
