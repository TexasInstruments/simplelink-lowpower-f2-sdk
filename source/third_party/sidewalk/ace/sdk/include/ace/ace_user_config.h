/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file ace_user_config.h
 * @brief Configuration of Ace components during initialization.
 *
 * The aceUserConfig_config_t structure provides a way for the product to pass
 * in their config and override the defaults in ACE library.
 * @addtogroup ACE_INIT_DEFINE
 * @{
 */
#ifndef ACE_USER_CONFIG_H
#define ACE_USER_CONFIG_H

#include <stdio.h>
#include <ace/osal_threads.h>

#define ACE_USER_CONFIG_VER 1

/**
 * @brief This structure contains pointers to all configurable parameters by the
 * product.
 */
typedef struct aceUserConfig_config_s {
    unsigned int version;
    const aceThread_prio_config* thread_prio;
} aceUserConfig_config_t;

/**
 * @brief Parse the custom config file.
 *
 * This function is to be use by the Linux build systems only.
 * It reads the config line by line from a config file denoted by the file
 * pointer fp and constructs the ace_config_t out of it.
 * The config file will be in TOML format.
 *
 * @private
 *
 * @return ACE_STATUS_OK on success, else error code ACE_STATUS_NULL_POINTER.
 */
ace_status_t aceUserConfig_parser(FILE* fp, aceUserConfig_config_t* ace_config);

/** @} */
#endif /* ACE_USER_CONFIG_H */
