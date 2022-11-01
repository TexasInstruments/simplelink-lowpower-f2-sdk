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

#ifndef PHYSETTINGS_H
#define PHYSETTINGS_H

/* Include Files */

/* SysConfig Generated Header Files */
#include <ti_radio_config.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_ble_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_ieee_cmd.h)

extern rfc_CMD_PROP_TX_t RF_cmdPropTxBle;
extern rfc_CMD_PROP_RX_t RF_cmdPropRxBle;

typedef enum
{
    PhySettings_PHY_0,
    PhySettings_PHY_1,
    PhySettings_PHY_2,
    PhySettings_PHY_3,
    PhySettings_PHY_4,
    PhySettings_PHY_5,
    PhySettings_PHY_6,
    PhySettings_PHY_7,
    PhySettings_PHY_8,
    PhySettings_PHY_9,
    // PhySettings_PHY_10,   // Must be included if more than 10 PHYs shall be supported
    // PhySettings_PHY_11,
    // .
    // .
    Num_Phy_Settings,
} PhySettings_phyIndex;

typedef enum
{
    PhySettings_API_PROP,
    PhySettings_API_BLE,
    PhySettings_API_IEEE,
    Num_Api_Settings,
} PhySettings_apiType;

typedef enum
{
    PhySettings_TEST_RF_PERF,
    PhySettings_TEST_STUDIO_COMPL,
    Num_Test_Settings,
} PhySettings_testType;


typedef enum
{
    PhySettings_CMD_STANDARD,
    PhySettings_CMD_ADVANCED,
    Num_Cmd_Settings,
} PhySettings_cmdType;

typedef enum
{
    PhySettings_SETUP_PROP,
    PhySettings_SETUP_PROP_PA,
    PhySettings_SETUP_BLE,
    PhySettings_SETUP_BLE_PA,
    PhySettings_SETUP_IEEE,
    PhySettings_SETUP_IEEE_PA,
    Num_setupType,
} PhySettings_setupType;

typedef struct
{
    PhySettings_phyIndex PhySettings_phyIndex;
    char PhySettings_phyName[70];
    PhySettings_apiType PhySettings_apiType;
    PhySettings_testType  PhySettings_testType;
    RF_Mode *RF_pMode;
    rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t *RF_pCmdPropRadioDivSetupPa;
    rfc_CMD_PROP_RADIO_DIV_SETUP_t *RF_pCmdPropRadioDivSetup;
    rfc_CMD_BLE5_RADIO_SETUP_PA_t *RF_pCmdBle5RadioSetupPa;
    rfc_CMD_BLE5_RADIO_SETUP_t *RF_pCmdBle5RadioSetup;
    rfc_CMD_RADIO_SETUP_PA_t *RF_pCmdRadioSetupPa;
    rfc_CMD_RADIO_SETUP_t *RF_pCmdRadioSetup;
    PhySettings_setupType PhySettings_setupType;
    rfc_CMD_FS_t *RF_pCmdFs;
    rfc_CMD_PROP_TX_t *RF_pCmdPropTx;
    rfc_CMD_PROP_TX_ADV_t *RF_pCmdPropTxAdv;
    rfc_CMD_PROP_RX_t *RF_pCmdPropRx;
    rfc_CMD_PROP_RX_ADV_t *RF_pCmdPropRxAdv;
    rfc_CMD_BLE5_ADV_AUX_t  *RF_pCmdBle5AdvAux;
    rfc_CMD_BLE5_GENERIC_RX_t *RF_pCmdBle5GenericRx;
    rfc_CMD_IEEE_TX_t *RF_pCmdIeeeTx;
    rfc_CMD_IEEE_RX_t *RF_pCmdIeeeRx;
    PhySettings_cmdType PhySettings_cmdType;
    RF_TxPowerTable_Entry *RF_pTxPowerTable;
    uint8_t RF_txPowerTableSize;
} PhySettings_RfSetting;

extern const uint8_t PhySettings_numSupportedPhys;
extern PhySettings_RfSetting PhySettings_supportedPhys[];

#endif // PHYSETTINGS_H
