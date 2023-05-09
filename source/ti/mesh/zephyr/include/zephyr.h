/*
 *
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef ZEPHYR_H_INCLUDE
#define ZEPHYR_H_INCLUDE

#include <errno.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stddef.h>
#include <limits.h>

#include <toolchain.h>
#include <zephyr/types.h>
#include <sys/atomic.h>
#include <sys/util.h>
#include <sys/slist.h>
#include <sys/printk.h>
#include <arch/cpu.h>
#include <kernel.h>

#ifndef ZEPHYR_KERNEL_BUILD
#include <autoconf.h>
#endif

#include <ti/sysbios/knl/Task.h>
#include <ti/kernel/SCOM.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>

#define K_MSEC(x) Z_TIMEOUT_TICKS(MAX(x, 0))
#define K_SECONDS(s) Z_TIMEOUT_TICKS(s * 1000u)
#define K_MINUTES(m) Z_TIMEOUT_TICKS(m * 60000u)
#define K_HOURS(m) Z_TIMEOUT_TICKS(m * 360000u)

#endif /* ZEPHYR_H_ */
