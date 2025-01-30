/*
 * Copyright (c) 2015-2024, Texas Instruments Incorporated
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

/*
 *  ======== blinky.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/dpl/HwiP.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/flash.h)
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#else
#include DeviceFamily_constructPath(driverlib/hapi.h)
#endif

/* Driver configuration */
#include "ti_drivers_config.h"

#include "bootutil/image.h"

#include "trace.h"

#define BLINK_INTERVAL     500000  /* Set blink interval to 500000us or 500ms */

#define TRACE_GROUP "APP "

extern int MCUBOOT_HDR_BASE;
struct image_header *mcubootHdr = (struct image_header *)&MCUBOOT_HDR_BASE;

#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
/*
 *  ======== disableFlashCache ========
 */

static uint8_t disableFlashCache(void)
{
    uint8_t mode = VIMSModeGet(VIMS_BASE);

    VIMSLineBufDisable(VIMS_BASE);

    if (mode != VIMS_MODE_DISABLED) {
        VIMSModeSet(VIMS_BASE, VIMS_MODE_DISABLED);
        while (VIMSModeGet(VIMS_BASE) != VIMS_MODE_DISABLED);
    }

    return (mode);
}

/*
 *  ======== restoreFlashCache ========
 */
static void restoreFlashCache(uint8_t mode)
{

    if (mode != VIMS_MODE_DISABLED) {
        VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
    }

    VIMSLineBufEnable(VIMS_BASE);
}
#endif

/*
 *  ======== gpioButtonFxn0 ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_0.
 *
 *  Note: GPIO interrupts are cleared prior to invoking callbacks.
 */
void gpioButtonFxn0(uint_least8_t index)
{
    struct image_version versionZero = {0};
    uint32_t flashStat;
    unsigned int key;
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    uint8_t mode;
#endif

    TRACE_LOG_INF("blinky_app invalidating image");

    /* Set mcuboot header ver to 0 to invalidate the image */
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    mode = disableFlashCache();
#endif
    key = HwiP_disable();
    flashStat = FlashProgram((uint8_t *) &versionZero, (uint32_t) &(mcubootHdr->ih_ver), sizeof(mcubootHdr->ih_ver));
    HwiP_restore(key);
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    restoreFlashCache(mode);
#endif

    if(flashStat != FAPI_STATUS_SUCCESS)
    {
        TRACE_LOG_ERR("blinky_app failed to invalidate image %x", flashStat);
    }

    /* Reset and run mcuboot */
#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
    SysCtrlSystemReset();
#else
    HapiResetDevice();
#endif
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* configure trace */
    trace_init();

    TRACE_LOG_INF("blinky_app hdr=%x", mcubootHdr);

    TRACE_LOG_INF("blinky_app ver=%d.%d.%d.%d",
                      mcubootHdr->ih_ver.iv_major,
                      mcubootHdr->ih_ver.iv_minor,
                      mcubootHdr->ih_ver.iv_revision,
                      mcubootHdr->ih_ver.iv_build_num);

    /* Init LED's state */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);

    /* Install Button callback and enable interrupt */
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn0);
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);

    while(1)
    {
        usleep(BLINK_INTERVAL);
        GPIO_write(CONFIG_GPIO_LED_0, !GPIO_read(CONFIG_GPIO_LED_0));
        GPIO_write(CONFIG_GPIO_LED_1, !GPIO_read(CONFIG_GPIO_LED_1));
    }
}
