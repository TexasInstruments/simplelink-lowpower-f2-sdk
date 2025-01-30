/*
 *  ======== ti_drivers_config.h ========
 *  Configured TI-Drivers module declarations
 *
 *  The macros defines herein are intended for use by applications which
 *  directly include this header. These macros should NOT be hard coded or
 *  copied into library source code.
 *
 *  Symbols declared as const are intended for use with libraries.
 *  Library source code must extern the correct symbol--which is resolved
 *  when the application is linked.
 *
 *  DO NOT EDIT - This file is generated for the CC2674R10RGZ
 *  by the SysConfig tool.
 */
#ifndef ti_drivers_config_h
#define ti_drivers_config_h

#define CONFIG_SYSCONFIG_PREVIEW

#define CONFIG_CC2674R10RGZ
#ifndef DeviceFamily_CC26X4
#define DeviceFamily_CC26X4
#endif

#include <ti/devices/DeviceFamily.h>

#include <stdint.h>

/* support C++ sources */
#ifdef __cplusplus
extern "C" {
#endif


/*
 *  ======== CCFG ========
 */


/*
 *  ======== AESCBC ========
 */

extern const uint_least8_t                  CONFIG_AESCBC_0_CONST;
#define CONFIG_AESCBC_0                     0
#define CONFIG_TI_DRIVERS_AESCBC_COUNT      1


/*
 *  ======== AESCCM ========
 */

extern const uint_least8_t                  CONFIG_AESCCM_0_CONST;
#define CONFIG_AESCCM_0                     0
#define CONFIG_TI_DRIVERS_AESCCM_COUNT      1


/*
 *  ======== AESCMAC ========
 */

extern const uint_least8_t                  CONFIG_AESCMAC_0_CONST;
#define CONFIG_AESCMAC_0                    0
extern const uint_least8_t                  CONFIG_AESCMAC_1_CONST;
#define CONFIG_AESCMAC_1                    1
#define CONFIG_TI_DRIVERS_AESCMAC_COUNT     2


/*
 *  ======== AESCTR ========
 */

extern const uint_least8_t                  CONFIG_AESCTR_0_CONST;
#define CONFIG_AESCTR_0                     0
#define CONFIG_TI_DRIVERS_AESCTR_COUNT      1


/*
 *  ======== AESCTRDRBG ========
 */

extern const uint_least8_t                      CONFIG_AESCTRDRBG_0_CONST;
#define CONFIG_AESCTRDRBG_0                     0
#define CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT      1


/*
 *  ======== AESECB ========
 */

extern const uint_least8_t                  CONFIG_AESECB_0_CONST;
#define CONFIG_AESECB_0                     0
#define CONFIG_TI_DRIVERS_AESECB_COUNT      1


/*
 *  ======== AESGCM ========
 */

extern const uint_least8_t                  CONFIG_AESGCM_0_CONST;
#define CONFIG_AESGCM_0                     0
#define CONFIG_TI_DRIVERS_AESGCM_COUNT      1


/*
 *  ======== ECDH ========
 */

extern const uint_least8_t              CONFIG_ECDH_0_CONST;
#define CONFIG_ECDH_0                   0
#define CONFIG_TI_DRIVERS_ECDH_COUNT    1


/*
 *  ======== ECDSA ========
 */

extern const uint_least8_t                  CONFIG_ECDSA_0_CONST;
#define CONFIG_ECDSA_0                      0
extern const uint_least8_t                  CONFIG_ECDSA_1_CONST;
#define CONFIG_ECDSA_1                      1
#define CONFIG_TI_DRIVERS_ECDSA_COUNT       2


/*
 *  ======== ECJPAKE ========
 */

extern const uint_least8_t                  CONFIG_ECJPAKE_0_CONST;
#define CONFIG_ECJPAKE_0                    0
#define CONFIG_TI_DRIVERS_ECJPAKE_COUNT     1


/*
 *  ======== EDDSA ========
 */

extern const uint_least8_t                  CONFIG_EDDSA_0_CONST;
#define CONFIG_EDDSA_0                      0
#define CONFIG_TI_DRIVERS_EDDSA_COUNT       1


/*
 *  ======== GPIO ========
 */
/* The range of pins available on this device */
extern const uint_least8_t GPIO_pinLowerBound;
extern const uint_least8_t GPIO_pinUpperBound;

/* LEDs are active high */
#define CONFIG_GPIO_LED_ON  (1)
#define CONFIG_GPIO_LED_OFF (0)

#define CONFIG_LED_ON  (CONFIG_GPIO_LED_ON)
#define CONFIG_LED_OFF (CONFIG_GPIO_LED_OFF)


/*
 *  ======== SHA2 ========
 */

extern const uint_least8_t              CONFIG_SHA2_0_CONST;
#define CONFIG_SHA2_0                   0
extern const uint_least8_t              CONFIG_SHA2_1_CONST;
#define CONFIG_SHA2_1                   1
#define CONFIG_TI_DRIVERS_SHA2_COUNT    2


/*
 *  ======== TRNG ========
 */

extern const uint_least8_t              CONFIG_TRNG_0_CONST;
#define CONFIG_TRNG_0                   0
extern const uint_least8_t              CONFIG_TRNG_1_CONST;
#define CONFIG_TRNG_1                   1
#define CONFIG_TI_DRIVERS_TRNG_COUNT    2


/*
 *  ======== Board_init ========
 *  Perform all required TI-Drivers initialization
 *
 *  This function should be called once at a point before any use of
 *  TI-Drivers.
 */
extern void Board_init(void);

/*
 *  ======== Board_initGeneral ========
 *  (deprecated)
 *
 *  Board_initGeneral() is defined purely for backward compatibility.
 *
 *  All new code should use Board_init() to do any required TI-Drivers
 *  initialization _and_ use <Driver>_init() for only where specific drivers
 *  are explicitly referenced by the application.  <Driver>_init() functions
 *  are idempotent.
 */
#define Board_initGeneral Board_init

#ifdef __cplusplus
}
#endif

#endif /* include guard */
