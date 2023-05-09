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
 * @file ace.h
 * @brief Initialization of ACE framework components.
 *
 * This file provides access to ACE OSAL headers and APIs needed for
 * initializing ACE framework components.
 * @addtogroup ACE_INIT_API
 * @{
 */
#ifndef ACE_DOT_H
#define ACE_DOT_H

#include <ace/ace_config.h>
#include <ace/osal_common.h>
#include <ace/os_specific.h>
#include <ace/osal_mp.h>
#include <ace/osal_mq.h>
#include <ace/osal_mutex.h>
#include <ace/osal_rand.h>
#include <ace/osal_semaphore.h>
#include <ace/osal_shmem.h>
#include <ace/osal_threads.h>
#include <ace/osal_time.h>
#include <ace/osal_ip_mutex.h>
#include <ace/osal_alloc.h>
#include <ace/ace_user_config.h>
/**
 * @brief   Initialize ACE framework components, no user config provided.
 *
 * @details Initializes ACE framework components if the product wants to
 *          use just the default configuration set in the lib.
 *
 * @return  ACE_STATUS_OK on success, else error code.
 */
int ace_init(void);

/**
 * @brief   Initialize ACE framework components, user config provided.
 *
 * @details Initializes ACE framework components if the product does not
 *          want to use the default configuration set in the lib.
 *          @ref aceUserConfig_config_t struct defined in @ref
 *          ace_user_config.h.
 *
 * @param[in] ace_config Populated @ref aceUserConfig_config_t struct provided
 *                       by product.
 *
 * @return ACE_STATUS_OK on success, else error code.
 */
int ace_initWithConfig(aceUserConfig_config_t* ace_config);

/** @} */
#endif /* ACE_DOT_H */
