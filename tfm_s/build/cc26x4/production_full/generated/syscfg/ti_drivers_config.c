/*
 *  ======== ti_drivers_config.c ========
 *  Configured TI-Drivers module definitions
 *
 *  DO NOT EDIT - This file is generated for the CC2674R10RGZ
 *  by the SysConfig tool.
 */

#include <stddef.h>
#include <stdint.h>

#ifndef DeviceFamily_CC26X4
#define DeviceFamily_CC26X4
#endif

#include <ti/devices/DeviceFamily.h>

#include "ti_drivers_config.h"

/*
 *  =============================== AESCBC ===============================
 */

#include <ti/drivers/AESCBC.h>
#include <ti/drivers/aescbc/AESCBCCC26XX.h>

#define CONFIG_AESCBC_COUNT 1


AESCBCCC26XX_Object aescbcCC26XXObjects[CONFIG_AESCBC_COUNT];

/*
 *  ======== aescbcCC26XXHWAttrs ========
 */
const AESCBCCC26XX_HWAttrs aescbcCC26XXHWAttrs[CONFIG_AESCBC_COUNT] = {
    {
        .intPriority = (~0),
    },
};

const AESCBC_Config AESCBC_config[CONFIG_AESCBC_COUNT] = {
    {   /* CONFIG_AESCBC_0 */
        .object  = &aescbcCC26XXObjects[CONFIG_AESCBC_0],
        .hwAttrs = &aescbcCC26XXHWAttrs[CONFIG_AESCBC_0]
    },
};

const uint_least8_t CONFIG_AESCBC_0_CONST = CONFIG_AESCBC_0;
const uint_least8_t AESCBC_count = CONFIG_AESCBC_COUNT;

/*
 *  =============================== AESCCM ===============================
 */

#include <ti/drivers/AESCCM.h>
#include <ti/drivers/aesccm/AESCCMCC26X4.h>

#define CONFIG_AESCCM_COUNT 1


AESCCMCC26X4_Object aesccmCC26X4Objects[CONFIG_AESCCM_COUNT];

/*
 *  ======== aesccmCC26X4HWAttrs ========
 */
const AESCCMCC26X4_HWAttrs aesccmCC26X4HWAttrs[CONFIG_AESCCM_COUNT] = {
    {
        .intPriority = (~0),
    },
};

const AESCCM_Config AESCCM_config[CONFIG_AESCCM_COUNT] = {
    {   /* CONFIG_AESCCM_0 */
        .object  = &aesccmCC26X4Objects[CONFIG_AESCCM_0],
        .hwAttrs = &aesccmCC26X4HWAttrs[CONFIG_AESCCM_0]
    },
};

const uint_least8_t CONFIG_AESCCM_0_CONST = CONFIG_AESCCM_0;
const uint_least8_t AESCCM_count = CONFIG_AESCCM_COUNT;

/*
 *  =============================== AESCMAC ===============================
 */

#include <ti/drivers/AESCMAC.h>
#include <ti/drivers/aescmac/AESCMACCC26XX.h>

#define CONFIG_AESCMAC_COUNT 2


AESCMACCC26XX_Object aescmacCC26XXObjects[CONFIG_AESCMAC_COUNT];

/*
 *  ======== aescmacCC26XXHWAttrs ========
 */
const AESCMACCC26XX_HWAttrs aescmacCC26XXHWAttrs[CONFIG_AESCMAC_COUNT] = {
    {
        .intPriority = (~0),
    },
    {
        .intPriority = (~0),
    },
};

const AESCMAC_Config AESCMAC_config[CONFIG_AESCMAC_COUNT] = {
    {   /* CONFIG_AESCMAC_0 */
        .object  = &aescmacCC26XXObjects[CONFIG_AESCMAC_0],
        .hwAttrs = &aescmacCC26XXHWAttrs[CONFIG_AESCMAC_0]
    },
    {   /* CONFIG_AESCMAC_1 */
        .object  = &aescmacCC26XXObjects[CONFIG_AESCMAC_1],
        .hwAttrs = &aescmacCC26XXHWAttrs[CONFIG_AESCMAC_1]
    },
};

const uint_least8_t CONFIG_AESCMAC_0_CONST = CONFIG_AESCMAC_0;
const uint_least8_t CONFIG_AESCMAC_1_CONST = CONFIG_AESCMAC_1;
const uint_least8_t AESCMAC_count = CONFIG_AESCMAC_COUNT;

/*
 *  =============================== AESCTR ===============================
 */

#include <ti/drivers/AESCTR.h>
#include <ti/drivers/aesctr/AESCTRCC26XX.h>

#define CONFIG_AESCTR_COUNT 1


AESCTRCC26XX_Object aesctrCC26XXObjects[CONFIG_AESCTR_COUNT];

/*
 *  ======== aesctrCC26XXHWAttrs ========
 */
const AESCTRCC26XX_HWAttrs aesctrCC26XXHWAttrs[CONFIG_AESCTR_COUNT] = {
    {
        .intPriority = (~0),
    },
};

const AESCTR_Config AESCTR_config[CONFIG_AESCTR_COUNT] = {
    {   /* CONFIG_AESCTR_0 */
        .object  = &aesctrCC26XXObjects[CONFIG_AESCTR_0],
        .hwAttrs = &aesctrCC26XXHWAttrs[CONFIG_AESCTR_0]
    },
};

const uint_least8_t CONFIG_AESCTR_0_CONST = CONFIG_AESCTR_0;
const uint_least8_t AESCTR_count = CONFIG_AESCTR_COUNT;

/*
 *  =============================== AESCTRDRBG ===============================
 */

#include <ti/drivers/AESCTRDRBG.h>
#include <ti/drivers/aesctrdrbg/AESCTRDRBGXX.h>

#define CONFIG_AESCTRDRBG_COUNT 1

/*
 *  ======== aesctrdrbgXXObjects ========
 */
AESCTRDRBGXX_Object aesctrdrbgXXObjects[CONFIG_AESCTRDRBG_COUNT];

/*
 *  ======== aesctrdrbgXXHWAttrs ========
 */
const AESCTRDRBGXX_HWAttrs aesctrdrbgXXHWAttrs[CONFIG_AESCTRDRBG_COUNT] = {
    /* CONFIG_AESCTRDRBG_0 */
    {
        .aesctrHWAttrs.intPriority = (~0)
    },
};

/*
 *  ======== AESCTRDRBG_config ========
 */
const AESCTRDRBG_Config AESCTRDRBG_config[CONFIG_AESCTRDRBG_COUNT] = {
    /* CONFIG_AESCTRDRBG_0 */
    {
        .object = &aesctrdrbgXXObjects[CONFIG_AESCTRDRBG_0],
        .hwAttrs = &aesctrdrbgXXHWAttrs[CONFIG_AESCTRDRBG_0]
    },
};

const uint_least8_t CONFIG_AESCTRDRBG_0_CONST = CONFIG_AESCTRDRBG_0;
const uint_least8_t AESCTRDRBG_count = CONFIG_AESCTRDRBG_COUNT;

/*
 *  =============================== AESECB ===============================
 */

#include <ti/drivers/AESECB.h>
#include <ti/drivers/aesecb/AESECBCC26XX.h>

#define CONFIG_AESECB_COUNT 1


AESECBCC26XX_Object aesecbCC26XXObjects[CONFIG_AESECB_COUNT];

/*
 *  ======== aesecbCC26XXHWAttrs ========
 */
const AESECBCC26XX_HWAttrs aesecbCC26XXHWAttrs[CONFIG_AESECB_COUNT] = {
    {
        .intPriority = (~0),
    },
};

const AESECB_Config AESECB_config[CONFIG_AESECB_COUNT] = {
    {   /* CONFIG_AESECB_0 */
        .object  = &aesecbCC26XXObjects[CONFIG_AESECB_0],
        .hwAttrs = &aesecbCC26XXHWAttrs[CONFIG_AESECB_0]
    },
};


const uint_least8_t CONFIG_AESECB_0_CONST = CONFIG_AESECB_0;
const uint_least8_t AESECB_count = CONFIG_AESECB_COUNT;

/*
 *  =============================== AESGCM ===============================
 */

#include <ti/drivers/AESGCM.h>
#include <ti/drivers/aesgcm/AESGCMCC26X4.h>

#define CONFIG_AESGCM_COUNT 1


AESGCMCC26X4_Object aesgcmCC26X4Objects[CONFIG_AESGCM_COUNT];

/*
 *  ======== aesgcmCC26X4HWAttrs ========
 */
const AESGCMCC26X4_HWAttrs aesgcmCC26X4HWAttrs[CONFIG_AESGCM_COUNT] = {
    {
        .intPriority = (~0),
    },
};

const AESGCM_Config AESGCM_config[CONFIG_AESGCM_COUNT] = {
    {   /* CONFIG_AESGCM_0 */
        .object  = &aesgcmCC26X4Objects[CONFIG_AESGCM_0],
        .hwAttrs = &aesgcmCC26X4HWAttrs[CONFIG_AESGCM_0]
    },
};

const uint_least8_t CONFIG_AESGCM_0_CONST = CONFIG_AESGCM_0;
const uint_least8_t AESGCM_count = CONFIG_AESGCM_COUNT;

/*
 *  =============================== ECDH ===============================
 */

#include <ti/drivers/ECDH.h>
#include <ti/drivers/ecdh/ECDHCC26X2.h>

#define CONFIG_ECDH_COUNT 1


ECDHCC26X2_Object ecdhCC26X2Objects[CONFIG_ECDH_COUNT];

/*
 *  ======== ecdhCC26X2HWAttrs ========
 */
const ECDHCC26X2_HWAttrs ecdhCC26X2HWAttrs[CONFIG_ECDH_COUNT] = {
    {
        .intPriority = (~0),
    },
};

const ECDH_Config ECDH_config[CONFIG_ECDH_COUNT] = {
    {   /* CONFIG_ECDH_0 */
        .object         = &ecdhCC26X2Objects[CONFIG_ECDH_0],
        .hwAttrs        = &ecdhCC26X2HWAttrs[CONFIG_ECDH_0]
    },
};

const uint_least8_t CONFIG_ECDH_0_CONST = CONFIG_ECDH_0;
const uint_least8_t ECDH_count = CONFIG_ECDH_COUNT;

/*
 *  =============================== ECDSA ===============================
 */

#include <ti/drivers/ECDSA.h>
#include <ti/drivers/ecdsa/ECDSACC26X2.h>

#define CONFIG_ECDSA_COUNT 2


ECDSACC26X2_Object ecdsaCC26X2Objects[CONFIG_ECDSA_COUNT];

/*
 *  ======== ecdsaCC26X2HWAttrs ========
 */
const ECDSACC26X2_HWAttrs ecdsaCC26X2HWAttrs[CONFIG_ECDSA_COUNT] = {
    {
        .intPriority = (~0),
        .trngIntPriority = (~0),
    },
    {
        .intPriority = (~0),
        .trngIntPriority = (~0),
    },
};

const ECDSA_Config ECDSA_config[CONFIG_ECDSA_COUNT] = {
    {   /* CONFIG_ECDSA_0 */
        .object         = &ecdsaCC26X2Objects[CONFIG_ECDSA_0],
        .hwAttrs        = &ecdsaCC26X2HWAttrs[CONFIG_ECDSA_0]
    },
    {   /* CONFIG_ECDSA_1 */
        .object         = &ecdsaCC26X2Objects[CONFIG_ECDSA_1],
        .hwAttrs        = &ecdsaCC26X2HWAttrs[CONFIG_ECDSA_1]
    },
};

const uint_least8_t CONFIG_ECDSA_0_CONST = CONFIG_ECDSA_0;
const uint_least8_t CONFIG_ECDSA_1_CONST = CONFIG_ECDSA_1;
const uint_least8_t ECDSA_count = CONFIG_ECDSA_COUNT;

/*
 *  =============================== ECJPAKE ===============================
 */

#include <ti/drivers/ECJPAKE.h>
#include <ti/drivers/ecjpake/ECJPAKECC26X2.h>

#define CONFIG_ECJPAKE_COUNT 1


ECJPAKECC26X2_Object ecjpakeCC26X2Objects[CONFIG_ECJPAKE_COUNT];

/*
 *  ======== ecjpakeCC26X2HWAttrs ========
 */
const ECJPAKECC26X2_HWAttrs ecjpakeCC26X2HWAttrs[CONFIG_ECJPAKE_COUNT] = {
    {
        .intPriority = (~0),
    },
};

const ECJPAKE_Config ECJPAKE_config[CONFIG_ECJPAKE_COUNT] = {
    {   /* CONFIG_ECJPAKE_0 */
        .object         = &ecjpakeCC26X2Objects[CONFIG_ECJPAKE_0],
        .hwAttrs        = &ecjpakeCC26X2HWAttrs[CONFIG_ECJPAKE_0]
    },
};

const uint_least8_t CONFIG_ECJPAKE_0_CONST = CONFIG_ECJPAKE_0;
const uint_least8_t ECJPAKE_count = CONFIG_ECJPAKE_COUNT;

/*
 *  =============================== EDDSA ===============================
 */

#include <ti/drivers/EDDSA.h>
#include <ti/drivers/eddsa/EDDSACC26X2.h>

#define CONFIG_EDDSA_COUNT 1


EDDSACC26X2_Object eddsaCC26X2Objects[CONFIG_EDDSA_COUNT];

/*
 *  ======== eddsaCC26X2HWAttrs ========
 */
const EDDSACC26X2_HWAttrs eddsaCC26X2HWAttrs[CONFIG_EDDSA_COUNT] = {
    {
        .intPriority = (~0),
        .sha2IntPriority = (~0),
    },
};

const EDDSA_Config EDDSA_config[CONFIG_EDDSA_COUNT] = {
    {   /* CONFIG_EDDSA_0 */
        .object         = &eddsaCC26X2Objects[CONFIG_EDDSA_0],
        .hwAttrs        = &eddsaCC26X2HWAttrs[CONFIG_EDDSA_0]
    },
};

const uint_least8_t CONFIG_EDDSA_0_CONST = CONFIG_EDDSA_0;
const uint_least8_t EDDSA_count = CONFIG_EDDSA_COUNT;

/*
 *  =============================== GPIO ===============================
 */

#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOCC26XX.h>

/* The range of pins available on this device */
const uint_least8_t GPIO_pinLowerBound = 0;
const uint_least8_t GPIO_pinUpperBound = 30;

/*
 *  ======== gpioPinConfigs ========
 *  Array of Pin configurations
 */
GPIO_PinConfig gpioPinConfigs[31] = {
    GPIO_CFG_NO_DIR, /* DIO_0 */
    GPIO_CFG_NO_DIR, /* DIO_1 */
    GPIO_CFG_NO_DIR, /* DIO_2 */
    GPIO_CFG_NO_DIR, /* DIO_3 */
    GPIO_CFG_NO_DIR, /* DIO_4 */
    GPIO_CFG_NO_DIR, /* DIO_5 */
    GPIO_CFG_NO_DIR, /* DIO_6 */
    GPIO_CFG_NO_DIR, /* DIO_7 */
    GPIO_CFG_NO_DIR, /* DIO_8 */
    GPIO_CFG_NO_DIR, /* DIO_9 */
    GPIO_CFG_NO_DIR, /* DIO_10 */
    GPIO_CFG_NO_DIR, /* DIO_11 */
    GPIO_CFG_NO_DIR, /* DIO_12 */
    GPIO_CFG_NO_DIR, /* DIO_13 */
    GPIO_CFG_NO_DIR, /* DIO_14 */
    GPIO_CFG_NO_DIR, /* DIO_15 */
    GPIO_CFG_NO_DIR, /* DIO_16 */
    GPIO_CFG_NO_DIR, /* DIO_17 */
    GPIO_CFG_NO_DIR, /* DIO_18 */
    GPIO_CFG_NO_DIR, /* DIO_19 */
    GPIO_CFG_NO_DIR, /* DIO_20 */
    GPIO_CFG_NO_DIR, /* DIO_21 */
    GPIO_CFG_NO_DIR, /* DIO_22 */
    GPIO_CFG_NO_DIR, /* DIO_23 */
    GPIO_CFG_NO_DIR, /* DIO_24 */
    GPIO_CFG_NO_DIR, /* DIO_25 */
    GPIO_CFG_NO_DIR, /* DIO_26 */
    GPIO_CFG_NO_DIR, /* DIO_27 */
    GPIO_CFG_NO_DIR, /* DIO_28 */
    GPIO_CFG_NO_DIR, /* DIO_29 */
    GPIO_CFG_NO_DIR, /* DIO_30 */
};

/*
 *  ======== gpioCallbackFunctions ========
 *  Array of callback function pointers
 *  Change at runtime with GPIO_setCallback()
 */
GPIO_CallbackFxn gpioCallbackFunctions[31];

/*
 *  ======== gpioUserArgs ========
 *  Array of user argument pointers
 *  Change at runtime with GPIO_setUserArg()
 *  Get values with GPIO_getUserArg()
 */
void* gpioUserArgs[31];



/*
 *  ======== GPIO_config ========
 */
const GPIO_Config GPIO_config = {
    .configs = (GPIO_PinConfig *)gpioPinConfigs,
    .callbacks = (GPIO_CallbackFxn *)gpioCallbackFunctions,
    .userArgs = gpioUserArgs,
    .intPriority = (~0)
};

/*
 *  =============================== Power ===============================
 */
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include "ti_drivers_config.h"

extern void PowerCC26XX_standbyPolicy(void);
extern bool PowerCC26XX_calibrate(unsigned int);

const PowerCC26X2_Config PowerCC26X2_config = {
    .enablePolicy             = true,
    .policyInitFxn            = NULL,
    .policyFxn                = PowerCC26XX_standbyPolicy,
    .calibrateFxn             = PowerCC26XX_calibrate,
    .calibrateRCOSC_LF        = true,
    .calibrateRCOSC_HF        = true,
    .enableTCXOFxn            = NULL
};


/*
 *  =============================== SHA2 ===============================
 */

#include <ti/drivers/SHA2.h>
#include <ti/drivers/sha2/SHA2CC26X2.h>

#define CONFIG_SHA2_COUNT 2


SHA2CC26X2_Object sha2CC26X2Objects[CONFIG_SHA2_COUNT];

/*
 *  ======== sha2CC26X2HWAttrs ========
 */
const SHA2CC26X2_HWAttrs sha2CC26X2HWAttrs[CONFIG_SHA2_COUNT] = {
    {
        .intPriority = (~0),
    },
    {
        .intPriority = (~0),
    },
};

const SHA2_Config SHA2_config[CONFIG_SHA2_COUNT] = {
    {   /* CONFIG_SHA2_0 */
        .object         = &sha2CC26X2Objects[CONFIG_SHA2_0],
        .hwAttrs        = &sha2CC26X2HWAttrs[CONFIG_SHA2_0]
    },
    {   /* CONFIG_SHA2_1 */
        .object         = &sha2CC26X2Objects[CONFIG_SHA2_1],
        .hwAttrs        = &sha2CC26X2HWAttrs[CONFIG_SHA2_1]
    },
};

const uint_least8_t CONFIG_SHA2_0_CONST = CONFIG_SHA2_0;
const uint_least8_t CONFIG_SHA2_1_CONST = CONFIG_SHA2_1;
const uint_least8_t SHA2_count = CONFIG_SHA2_COUNT;

/*
 *  =============================== TRNG ===============================
 */

#include <ti/drivers/TRNG.h>
#include <ti/drivers/trng/TRNGCC26XX.h>

#define CONFIG_TRNG_COUNT 2


TRNGCC26XX_Object trngCC26XXObjects[CONFIG_TRNG_COUNT];

/*
 *  ======== trngCC26XXHWAttrs ========
 */
static const TRNGCC26XX_HWAttrs trngCC26XXHWAttrs[CONFIG_TRNG_COUNT] = {
    {
        .intPriority = (~0),
        .swiPriority = 0,
        .samplesPerCycle = 240000
    },
    {
        .intPriority = (~0),
        .swiPriority = 0,
        .samplesPerCycle = 240000
    },
};

const TRNG_Config TRNG_config[CONFIG_TRNG_COUNT] = {
    {   /* CONFIG_TRNG_0 */
        .object         = &trngCC26XXObjects[CONFIG_TRNG_0],
        .hwAttrs        = &trngCC26XXHWAttrs[CONFIG_TRNG_0]
    },
    {   /* CONFIG_TRNG_1 */
        .object         = &trngCC26XXObjects[CONFIG_TRNG_1],
        .hwAttrs        = &trngCC26XXHWAttrs[CONFIG_TRNG_1]
    },
};

const uint_least8_t CONFIG_TRNG_0_CONST = CONFIG_TRNG_0;
const uint_least8_t CONFIG_TRNG_1_CONST = CONFIG_TRNG_1;
const uint_least8_t TRNG_count = CONFIG_TRNG_COUNT;

/*
 *  =============================== SecureCallback ===============================
 */
#include <ti/drivers/tfm/SecureCallback.h>
#include "ti_drivers_config.h"

const SecureCallback_Config SecureCallback_config = {
    .intNumber = 59,
    .intPriority = (~0),
};

volatile SecureCallback_Handle SecureCallback_postedHandle;

#include <stdbool.h>

#include <ti/devices/cc13x4_cc26x4/driverlib/ioc.h>
#include <ti/devices/cc13x4_cc26x4/driverlib/cpu.h>

#include <ti/drivers/GPIO.h>

/* Board GPIO defines */
#define BOARD_EXT_FLASH_SPI_CS      20
#define BOARD_EXT_FLASH_SPI_CLK     10
#define BOARD_EXT_FLASH_SPI_PICO    9
#define BOARD_EXT_FLASH_SPI_POCI    8


/*
 *  ======== Board_sendExtFlashByte ========
 */
void Board_sendExtFlashByte(uint8_t byte)
{
    uint8_t i;

    /* SPI Flash CS */
    GPIO_write(BOARD_EXT_FLASH_SPI_CS, 0);

    for (i = 0; i < 8; i++) {
        GPIO_write(BOARD_EXT_FLASH_SPI_CLK, 0); /* SPI Flash CLK */

        /* SPI Flash PICO */
        GPIO_write(BOARD_EXT_FLASH_SPI_PICO, (byte >> (7 - i)) & 0x01);
        GPIO_write(BOARD_EXT_FLASH_SPI_CLK, 1);  /* SPI Flash CLK */

        /*
         * Waste a few cycles to keep the CLK high for at
         * least 45% of the period.
         * 3 cycles per loop: 8 loops @ 48 Mhz = 0.5 us.
         */
        CPUdelay(8);
    }

    GPIO_write(BOARD_EXT_FLASH_SPI_CLK, 0);  /* CLK */
    GPIO_write(BOARD_EXT_FLASH_SPI_CS, 1);  /* CS */

    /*
     * Keep CS high at least 40 us
     * 3 cycles per loop: 700 loops @ 48 Mhz ~= 44 us
     */
    CPUdelay(700);
}

/*
 *  ======== Board_wakeUpExtFlash ========
 */
void Board_wakeUpExtFlash(void)
{
    /* SPI Flash CS*/
    GPIO_setConfig(BOARD_EXT_FLASH_SPI_CS, GPIO_CFG_OUTPUT | GPIO_CFG_OUT_HIGH | GPIO_CFG_OUT_STR_MED);

    /*
     *  To wake up we need to toggle the chip select at
     *  least 20 ns and ten wait at least 35 us.
     */

    /* Toggle chip select for ~20ns to wake ext. flash */
    GPIO_write(BOARD_EXT_FLASH_SPI_CS, 0);
    /* 3 cycles per loop: 1 loop @ 48 Mhz ~= 62 ns */
    CPUdelay(1);
    GPIO_write(BOARD_EXT_FLASH_SPI_CS, 1);
    /* 3 cycles per loop: 560 loops @ 48 Mhz ~= 35 us */
    CPUdelay(560);
}

/*
 *  ======== Board_shutDownExtFlash ========
 */
void Board_shutDownExtFlash(void)
{
    /*
     *  To be sure we are putting the flash into sleep and not waking it,
     *  we first have to make a wake up call
     */
    Board_wakeUpExtFlash();

    /* SPI Flash CS*/
    GPIO_setConfig(BOARD_EXT_FLASH_SPI_CS, GPIO_CFG_OUTPUT | GPIO_CFG_OUT_HIGH | GPIO_CFG_OUT_STR_MED);
    /* SPI Flash CLK */
    GPIO_setConfig(BOARD_EXT_FLASH_SPI_CLK, GPIO_CFG_OUTPUT | GPIO_CFG_OUT_LOW | GPIO_CFG_OUT_STR_MED);
    /* SPI Flash PICO */
    GPIO_setConfig(BOARD_EXT_FLASH_SPI_PICO, GPIO_CFG_OUTPUT | GPIO_CFG_OUT_LOW | GPIO_CFG_OUT_STR_MED);
    /* SPI Flash POCI */
    GPIO_setConfig(BOARD_EXT_FLASH_SPI_POCI, GPIO_CFG_IN_PD);

    uint8_t extFlashShutdown = 0xB9;

    Board_sendExtFlashByte(extFlashShutdown);

    GPIO_resetConfig(BOARD_EXT_FLASH_SPI_CS);
    GPIO_resetConfig(BOARD_EXT_FLASH_SPI_CLK);
    GPIO_resetConfig(BOARD_EXT_FLASH_SPI_PICO);
    GPIO_resetConfig(BOARD_EXT_FLASH_SPI_POCI);
}


#include <ti/drivers/Board.h>

/*
 *  ======== Board_initHook ========
 *  Perform any board-specific initialization needed at startup.  This
 *  function is declared weak to allow applications to override it if needed.
 */
void __attribute__((weak)) Board_initHook(void)
{
}

/*
 *  ======== Board_init ========
 *  Perform any initialization needed before using any board APIs
 */
void Board_init(void)
{
    /* ==== /ti/drivers/Power initialization ==== */
    Power_init();

    /* ==== /ti/devices/CCFG initialization ==== */

    /* ==== /ti/drivers/GPIO initialization ==== */
    /* Setup GPIO module and default-initialise pins */
    GPIO_init();

    Board_shutDownExtFlash();

    Board_initHook();
}

