/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
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

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

#include "bootutil/bootutil.h"
#include "bootutil/image.h"
#include "bootutil/security_cnt.h"

#include "mcuboot_config/mcuboot_logging.h"

#ifndef EXCLUDE_GPIOS
#include "ti/common/cc26xx/debug/led_debug.h"
#endif // EXCLUDE_GPIOS

#ifdef MCUBOOT_DATA_SHARING
#include "ti/common/mcuboot/boot_seed/boot_seed.h"
#endif
#define BLINK_INTERVAL     500000  /* Set blink interval to 500000us or 500ms */

static __attribute__((naked)) void start_app(uint32_t *vector_table) {

    /* The following code resets the SP to the value specified in the
     * provided vector table, and then the Reset Handler is invoked.
     *
     * Per ARM Cortex specification:
     *
     *           ARM Cortex VTOR
     *
     *
     *   Offset             Vector
     *
     * 0x00000000  ++++++++++++++++++++++++++
     *             |    Initial SP value    |
     * 0x00000004  ++++++++++++++++++++++++++
     *             |         Reset          |
     * 0x00000008  ++++++++++++++++++++++++++
     *             |          NMI           |
     *             ++++++++++++++++++++++++++
     *             |           .            |
     *             |           .            |
     *             |           .            |
     *
     * */

    __asm volatile ("ldr r1, [r0, #0]\n"
                    "msr msp, r1\n"
                    "ldr r1, [r0, #4]\n"
                    "mov pc, r1\n"
                    );
}

static void do_boot(struct boot_rsp *rsp) {
    MCUBOOT_LOG_INF("Starting Main Application");
    MCUBOOT_LOG_INF("  Image Start Offset: 0x%x", (int)rsp->br_image_off);

    // We run from internal flash. Base address of this medium is 0x0
    uint32_t vector_table = 0x0 + rsp->br_image_off + rsp->br_hdr->ih_hdr_size;

    MCUBOOT_LOG_INF("  Vector Table Start Address: 0x%x",
    (int)vector_table);

#ifdef MCUBOOT_DATA_SHARING
    Boot_Seed();
#endif

    start_app((uint32_t *)vector_table);
}

static void mcubootFail(void) {
    powerUpGpio();
    while(1)
    {
#ifndef EXCLUDE_GPIOS
        blinkLed(RED_LED, 10, 50);
        blinkLed(RED_LED, 2, 1000);
#endif
    }
}
/*
 *  ======== mainThread ========
 */
int main(void)
{
    fih_int bootStatus;
    struct boot_rsp bootRsp;

#ifndef EXCLUDE_GPIOS
    powerUpGpio();
    lightRedLed();
#endif //EXCLUDE_GPIOS

    trace_init();
    MCUBOOT_LOG_INF("mcuboot_app");

#ifdef MCUBOOT_HW_ROLLBACK_PROT

    if (FIH_SUCCESS != boot_nv_security_counter_init()) {
        MCUBOOT_LOG_INF("bootRsp: failed to initialize secure counter page");
        mcubootFail();
    }

#endif
    bootStatus = boot_go(&bootRsp);

#ifdef MCUBOOT_HW_ROLLBACK_PROT
    lockBimSector();
#endif

    if ((0 == bootStatus) && (IMAGE_MAGIC == bootRsp.br_hdr->ih_magic))
    {
        blinkLed(GREEN_LED, 3, 500);
        MCUBOOT_LOG_INF("bootRsp: slot = %x, offset = %x, ver=%d.%d.%d.%d",
                            bootStatus,
                            bootRsp.br_image_off,
                            bootRsp.br_hdr->ih_ver.iv_major,
                            bootRsp.br_hdr->ih_ver.iv_minor,
                            bootRsp.br_hdr->ih_ver.iv_revision,
                            bootRsp.br_hdr->ih_ver.iv_build_num);

        do_boot(&bootRsp);
    }
    else
    {
        MCUBOOT_LOG_INF("bootRsp: no image found");
        mcubootFail();
    }
}
