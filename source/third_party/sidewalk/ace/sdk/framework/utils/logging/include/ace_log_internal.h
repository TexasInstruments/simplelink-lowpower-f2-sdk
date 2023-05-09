/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.This file
 * is a Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#ifndef _LIBS_ACE_LOG_INTERNAL_H
#define _LIBS_ACE_LOG_INTERNAL_H

#include "ace/ace.h"
#include <ace/ace_log.h>

ace_status_t aceLog_init(void);

ace_status_t aceLog_cliRegisterFilter(const char* filtertag,
                                         aceLog_level_t priority);
ace_status_t aceLog_cliUnregisterFilter(const char* filtertag);

/*
 * @brief Initialize logging filter.
 */
void aceLog_initFilter();

/*
 * @brief Check if logging filter is found.
 *
 * @param[in] priority    Log level for the filter.
 * @param[in] tag         Log tag string.
 */
ace_status_t aceLog_checkFilter(aceLog_level_t priority, const char* tag);


// ===========================Deprecated APIs=================================

ace_status_t ace_log_cli_register_filter(const char* filtertag,
                                         ace_loglevel priority);
ace_status_t ace_log_cli_unregister_filter(const char* filtertag);

/*
 * @deprecated Use aceLog_initFilter()
 */
void ace_log_init_filter();

/*
 * @deprecated Use aceLog_checkFilter()
 */
ace_status_t ace_log_check_filter(ace_loglevel priority, const char* tag);

#endif
