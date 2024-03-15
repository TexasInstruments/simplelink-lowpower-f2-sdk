/* mbed Microcontroller Library
 * Copyright (c) 2006-2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_ASSERT_H
#define MBED_ASSERT_H

// print assert to trace
#define MBED_ASSERT(expr)                                \
do {                                                     \
    if (!(expr)) {                                       \
        while(1);                                        \
    }                                                    \
} while (0)

// print assert to trace
#define MBED_STRUCT_STATIC_ASSERT(expr, msg) while(1);

#endif

/**@}*/

/**@}*/

