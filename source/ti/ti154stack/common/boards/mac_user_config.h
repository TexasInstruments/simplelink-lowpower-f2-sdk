/******************************************************************************

 @file  mac_user_config.h

 @brief This file contains user configurable variables for the MAC
        radio such as type of RF Front End used with the TI device,
        TX power table, etc. Please see below for more detail.

        To change the default values of configurable variables:
          - Include the followings in your application main.c file:
            #ifndef USE_DEFAULT_USER_CFG

            #include "mac_user_config.h"

            // MAC user defined configuration
            macUserCfg_t macUser0Cfg = MAC_USER_CFG;

            #endif // USE_DEFAULT_USER_CFG

        Note: User configurable variables except the elements of
              the power table are only used during the initialization
              of the MAC. Changing the values of these variables
              except power table elements after this will have no
              effect.

        Note: To use the default user configurable variables, define
              the preprocessor symbol USE_DEFAULT_USER_CFG in your
              application project.

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef MAC_USER_CONFIG_H
#define MAC_USER_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */
#include <chipinfo.h>
#include "api_mac.h"

/* REGULAR PROJECTS */
#if !defined( COPROCESSOR ) && !defined( LIBRARY )
#include "ti_154stack_config.h"
#endif

/* there is no patch for CC1352 */
//#include <rf_patches/rf_patch_cpe_prop.h>


#if !defined(USE_DMM) && !defined (MAC_RADIO_USE_CSF)
#include <ti/drivers/rf/RF.h>
#else
#include "rf_mac_api.h"
#endif //USE_DMM
#include DeviceFamily_constructPath(driverlib/rf_ieee_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

//
// Device Package and Evaluation Module (EM) Board
//
// The device may come in more than one types of packages.
// For each package, the user may change how the RF Front End (FE) is
// configured. The possible FE settings are provided as a set of defines.
// (The user can also set the FE bias, the settings of which are also provided
// as defines.) The user can change the value of RF_FE_MODE_AND_BIAS to
// configure the RF FE as desired. However, while setting the FE configuration
// determines how the device is configured at the package, it is the PCB the
// device is mounted on (the EM) that determines how those signals are routed.
// So while the FE is configurable, how signals are used is fixed by the EM.
// As can be seen, the value of RF_FE_MODE_AND_BIAS is organized by the EM
// board as defined by EMs produced by Texas Instruments Inc. How the device
// is mounted, routed, and configured for a user product would of course be
// user defined, and the value of RF_FE_MODE_AND_BIAS would have to be set
// accordingly; the user could even dispense with the conditional board
// compiles entirely. So too with the usage of the Tx Power tables. As can be
// seen in mac_user_config.c, there are two tables, one for packages using a
// differential FE, and one for single-end. This too has been organized by TI
// defined EMs and would have to be set appropriately by the user.


/*******************************************************************************
 * TYPEDEFS
 */
/* Alternate HAL Assert function pointer */
typedef void (*alternateHalAssertFp_t)(void);

/* Range Extender function pointer */
typedef void (*setRangeExtenderFp_t)(uint32_t);
typedef void (*rfSelectFp_t)(uint16_t);

typedef struct
{
    uint32_t               getHwRevision;     /* API to get HW revision */
    uint32_t               *pRfDrvTblPtr;     /* RF Driver API table */
    uint32_t               *pCryptoDrvTblPtr; /* Crypto Driver API table */
    alternateHalAssertFp_t pAssertFP;         /* Assert Function Pointer */
    rfSelectFp_t           pRfSelectFP;       /* RF select Function Pointer */
} macUserCfg_t;

typedef struct
{
    RF_Mode *pRfMode;
    RF_TxPowerTable_Entry *pRfPowerTable;
    const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *pRfSetup;
    const rfc_CMD_FS_t    *pRfFs;
    const rfc_CMD_PROP_TX_ADV_t *pRfTx;
    const rfc_CMD_PROP_RX_ADV_t *pRfRx;
    const rfc_CMD_PROP_CS_t *pRfCs;
    void *data;
} macRfCfg_prop_t;

typedef struct
{
    RF_Mode *pRfMode;
    RF_TxPowerTable_Entry *pRfPowerTable;
    const rfc_CMD_RADIO_SETUP_PA_t *pRfSetup;
    const rfc_CMD_FS_t    *pRfFs;
    const rfc_CMD_IEEE_TX_t *pRfTx;
    const rfc_CMD_IEEE_RX_t *pRfRx;
    const rfc_CMD_IEEE_CSMA_t *pRfCs;
    const rfc_CMD_IEEE_RX_ACK_t *pRxAck;
    void *data;
} macRfCfg_ieee_t;

typedef struct
{
    uint8_t first;          //first channel
    uint8_t numbers;        //no of channels
    int8_t *pTable;         //power limit table
} macRfCfg_maxPower_t;
/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
extern const uint32_t           macRfDriverTable[];
extern const uint32_t           macCryptoDriverTable[];

#define MAC_USER_CFG          { (uint32_t)ChipInfo_GetHwRevision,   \
                              (uint32_t *)macRfDriverTable,         \
                              (uint32_t *)macCryptoDriverTable,     \
                              NULL,                                 \
                              rfSelect }

#define RF_CONFIG_250KBPS_IEEE_PHY_0   {                                                                  \
     &RF_prop_ieee154,                                                                          \
     txPowerTable_ieee154,                                                                      \
     (const rfc_CMD_RADIO_SETUP_PA_t *)&RF_cmdRadioSetup_ieee154,                               \
     (const rfc_CMD_FS_t *)&RF_cmdFs_ieee154,                                                   \
     (const rfc_CMD_IEEE_TX_t *)&RF_cmdIeeeTx_ieee154,                                          \
     (const rfc_CMD_IEEE_RX_t *)&RF_cmdIeeeRx_ieee154,                                          \
     (const rfc_CMD_IEEE_CSMA_t *)&RF_cmdIeeeCsma_ieee154,                                      \
     (const rfc_CMD_IEEE_RX_ACK_t *)&RF_cmdIeeeRxAck_ieee154,                                   \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_50KBPS_915MHZ_PHY_1    {                                                         \
     &RF_prop_2gfsk50kbps154g,                                                                  \
     txPowerTable_2gfsk50kbps154g,                                                              \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_2gfsk50kbps154g,       \
     (const rfc_CMD_FS_t *)&RF_cmdFs_2gfsk50kbps154g,                                           \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_2gfsk50kbps154g,                           \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_2gfsk50kbps154g,                           \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_2gfsk50kbps154g,                                  \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_50KBPS_868MHZ_PHY_3    {                                                       \
     &RF_prop_2gfsk50kbps154g,                                                                  \
     txPowerTable_2gfsk50kbps154g,                                                              \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_2gfsk50kbps154g,       \
     (const rfc_CMD_FS_t *)&RF_cmdFs_2gfsk50kbps154g,                                           \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_2gfsk50kbps154g,                           \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_2gfsk50kbps154g,                           \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_2gfsk50kbps154g,                                  \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_50KBPS_433MHZ_PHY_128   {                                                 \
     &RF_prop_2gfsk50kbps154g433mhz,                                                            \
     txPowerTable_2gfsk50kbps154g433mhz,                                                        \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_2gfsk50kbps154g433mhz, \
     (const rfc_CMD_FS_t *)&RF_cmdFs_2gfsk50kbps154g433mhz,                                     \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_2gfsk50kbps154g433mhz,                     \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_2gfsk50kbps154g433mhz,                     \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_2gfsk50kbps154g433mhz,                            \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_5KBPS_915MHZ_PHY_129    {                                               \
     &RF_prop_slr5kbps2gfsk,                                                                    \
     txPowerTable_slr5kbps2gfsk,                                                                \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_slr5kbps2gfsk,         \
     (const rfc_CMD_FS_t *)&RF_cmdFs_slr5kbps2gfsk,                                             \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_slr5kbps2gfsk,                             \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_slr5kbps2gfsk,                             \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_slr5kbps2gfsk,                                    \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_5KBPS_433MHZ_PHY_130     {                                           \
     &RF_prop_slr5kbps2gfsk433mhz,                                                              \
     txPowerTable_slr5kbps2gfsk433mhz,                                                          \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_slr5kbps2gfsk433mhz,   \
     (const rfc_CMD_FS_t *)&RF_cmdFs_slr5kbps2gfsk433mhz,                                       \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_slr5kbps2gfsk433mhz,                       \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_slr5kbps2gfsk433mhz,                       \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_slr5kbps2gfsk433mhz,                              \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_5KBPS_868MHZ_PHY_131    {                                             \
     &RF_prop_slr5kbps2gfsk,                                                                    \
     txPowerTable_slr5kbps2gfsk,                                                                \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_slr5kbps2gfsk,         \
     (const rfc_CMD_FS_t *)&RF_cmdFs_slr5kbps2gfsk,                                             \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_slr5kbps2gfsk,                             \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_slr5kbps2gfsk,                             \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_slr5kbps2gfsk,                                    \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_200KBPS_915MHZ_PHY_132     {                                                  \
     &RF_prop_2gfsk200kbps154g,                                                                 \
     txPowerTable_2gfsk200kbps154g,                                                             \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_2gfsk200kbps154g,      \
     (const rfc_CMD_FS_t *)&RF_cmdFs_2gfsk200kbps154g,                                          \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_2gfsk200kbps154g,                          \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_2gfsk200kbps154g,                          \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_2gfsk200kbps154g,                                 \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_200KBPS_868MHZ_PHY_133     {                                                \
     &RF_prop_2gfsk200kbps154g,                                                                 \
     txPowerTable_2gfsk200kbps154g,                                                             \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_2gfsk200kbps154g,      \
     (const rfc_CMD_FS_t *)&RF_cmdFs_2gfsk200kbps154g,                                          \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_2gfsk200kbps154g,                          \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_2gfsk200kbps154g,                          \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_2gfsk200kbps154g,                                 \
     (void *)NULL                                                                               \
}

#define RF_CONFIG_200KBPS_920MHZ_PHY_136     {                                                \
     &RF_prop_2gfsk200kbpsWisun4a,                                                                 \
     txPowerTable_2gfsk200kbpsWisun4a,                                                             \
     (const rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *)&RF_cmdPropRadioDivSetup_2gfsk200kbpsWisun4a,      \
     (const rfc_CMD_FS_t *)&RF_cmdFs_2gfsk200kbpsWisun4a,                                          \
     (const rfc_CMD_PROP_TX_ADV_t *)&RF_cmdPropTxAdv_2gfsk200kbpsWisun4a,                          \
     (const rfc_CMD_PROP_RX_ADV_t *)&RF_cmdPropRxAdv_2gfsk200kbpsWisun4a,                          \
     (const rfc_CMD_PROP_CS_t *)&RF_cmdPropCs_2gfsk200kbpsWisun4a,                                 \
     (void *)NULL                                                                               \
}

/*********************************************************************
 * FUNCTIONS
 */

extern void rfSelect(uint16_t phyId);
extern void rfSetConfigIeee(macRfCfg_ieee_t *pRfCfg);
extern void rfSetConfigSubG(macRfCfg_prop_t *pRfCfg);

/* mac.c */
void macSetUserConfig( macUserCfg_t *pUserCfg );

#ifdef __cplusplus
}
#endif

#endif /* MAC_USER_CONFIG_H */
