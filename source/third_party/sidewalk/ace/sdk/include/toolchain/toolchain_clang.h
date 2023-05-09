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
 */
#ifndef TOOLCHAIN_H
#error "Don't include toolchain_clang.h directly. Use toolchain/toolchain.h instead."
#endif
#ifndef TOOLCHAIN_CLANG_H
#define TOOLCHAIN_CLANG_H

/*
 * Clang has had both __has_attribute() and __has_builtin() since at least 3.0.0
 * (Feb 2012), so no need to define our own version-dependent capabilities like
 * we do in gcc.h.
 *
 * So for now, there's nothing to do here.
 */

#endif  // TOOLCHAIN_CLANG_H
