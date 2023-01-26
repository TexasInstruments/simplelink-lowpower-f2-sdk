/*
 * Copyright (c) 2015-2022, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdint.h>
#include "boot_seed.h"

#include "bootutil/boot_record.h"
#include "mcuboot_config/mcuboot_logging.h"

/* This only for cc13x4 and cc26x3, when Secure option is being enabled */
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/trng.h)
#include DeviceFamily_constructPath(driverlib/prcm.h)

#define BOOT_SEED_SIZE 32

typedef struct {
    uint8_t data[BOOT_SEED_SIZE];
} Boot_Seed_T;

#define BOOT_SEED_ITERATIONS (BOOT_SEED_SIZE >> 3)

void Boot_Seed (void)
{
    int i;
    Boot_Seed_T boot_seed;
    uint32_t *word = (uint32_t*)&boot_seed.data, lo, hi;

    PRCMPowerDomainOn(PRCM_PERIPH_TRNG);
    while (PRCMPowerDomainsAllOn(PRCM_PERIPH_TRNG) != PRCM_DOMAIN_POWER_ON);

    PRCMPeripheralRunEnable(PRCM_PERIPH_TRNG);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    TRNGConfigure(0, 24000, 0);
    TRNGEnable();

    /* HW TRNG generates 8 bytes random numbers, each made of a 32-bit hi and low words.
     * Boot seed requires 256 bit numbers = 32 bytes, then 4 iterations are needed.
     * A total of 5 rounds are made, but the first one is discarded as TRNG returns
     * 0's the first time.
     *
     * TODO: Find TRNG root cause of this first round as 0's. Very likely output
     * is not ready yet.
     */
    for (i=0; i <= BOOT_SEED_ITERATIONS; i++) {
        lo = TRNGNumberGet(TRNG_LOW_WORD);
        hi = TRNGNumberGet(TRNG_HI_WORD);
        if (i > 0) {
            *word++ = lo;
            *word++ = hi;
        }
        MCUBOOT_LOG_INF("%s() %08x %08x %08x %u\n", __func__, TRNGStatusGet(), hi, lo);
    }

    MCUBOOT_LOG_INF("Boot seed: ");
    for (i=0; i < BOOT_SEED_SIZE; i++) {
        MCUBOOT_LOG_INF("%02x", boot_seed.data[i]);
    }
    MCUBOOT_LOG_INF("\n");

    boot_add_data_to_shared_area(0, 0, BOOT_SEED_SIZE,
                                 boot_seed.data);

    TRNGDisable();

    PRCMPeripheralRunDisable(PRCM_PERIPH_TRNG);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    PRCMPowerDomainOff(PRCM_PERIPH_TRNG);
    while (PRCMPowerDomainsAllOn(PRCM_PERIPH_TRNG) != PRCM_DOMAIN_POWER_OFF);
}


