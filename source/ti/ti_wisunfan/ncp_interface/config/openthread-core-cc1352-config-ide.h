/*
 *  Copyright (c) 2017, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OPENTHREAD_CORE_CC1352_CONFIG_H_
#define OPENTHREAD_CORE_CC1352_CONFIG_H_

/**
 * @def OPENTHREAD_CONFIG_PLATFORM_INFO
 *
 * The platform-specific string to insert into the OpenThread version string.
 *
 */
#define OPENTHREAD_CONFIG_PLATFORM_INFO                    "CC1352"

/**
 * @def SETTINGS_CONFIG_BASE_ADDRESS
 *
 * The base address of the pages to be used for non-volatile-settings storage.
 */
#define SETTINGS_CONFIG_BASE_ADDRESS                       (0x52000)

/**
 * @def SETTINGS_CONFIG_PAGE_SIZE
 *
 * The size in bytes of a page for the cc13x2 platform.
 *
 * @note *MUST BE* 8K.
 */
#define SETTINGS_CONFIG_PAGE_SIZE                          (0x2000)

/**
 * @def SETTINGS_CONFIG_PAGE_NUM
 *
 * The number of flash pages to use for non-volatile settings storage.
 */
#define SETTINGS_CONFIG_PAGE_NUM                           (2)

/**
 * @def OPENTHREAD_CONFIG_LOG_OUTPUT
 *
 * For more details see: ${openthread}/src/core/openthread-core-default-config.h
 *
 * Set the default log output if not set.
 */
#if !defined(OPENTHREAD_CONFIG_LOG_OUTPUT)
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_NONE
#endif

#include <openthread/config.h>

#ifdef OPENTHREAD_PROJECT_CORE_CONFIG_FILE
//#include OPENTHREAD_PROJECT_CORE_CONFIG_FILE
#include "openthread-core-cc1352-config-ide.h"
#endif

#endif /* OPENTHREAD_CORE_CC1352_CONFIG_H_ */
