/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"

__attribute__((naked)) void boot_clear_bl2_ram_area(void)
{
    __ASM volatile(
        ".syntax unified                             \n"
        "mov     r0, #0                              \n"
        "ldr     r1, =Image$$ER_DATA$$Base           \n"
        "ldr     r2, =Image$$ARM_LIB_HEAP$$ZI$$Limit \n"
        "subs    r2, r2, r1                          \n"
        "Loop:                                       \n"
        "subs    r2, #4                              \n"
        "itt     ge                                  \n"
        "strge   r0, [r1, r2]                        \n"
        "bge     Loop                                \n"
        "bx      lr                                  \n"
         : : : "r0" , "r1" , "r2" , "memory"
    );
}
