/*
 *  ======== ti_radio_config.h ========
 *  Configured RadioConfig module definitions
 *
 *  DO NOT EDIT - This file is generated for the CC1352P7RGZ
 *  by the SysConfig tool.
 *
 *  Radio Config module version : 1.9
 *  SmartRF Studio data version : 2.21.0
 */
#ifndef _FSK_TI_RADIO_CONFIG_H_
#define _FSK_TI_RADIO_CONFIG_H_

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include <ti/drivers/rf/RF.h>


//*********************************************************************************
//  RF Setting:   50 kbps, 2-GFSK, 25 kHz deviation
//
//  PHY:          2gfsk50kbps
//  Setting file: setting_tc706.json
//*********************************************************************************
// RF design based on: LP_CC1352P7-1 (CC1352PEM-XD7793-XD24)
#define LP_CC1352P7_1

// TX Power table size definition
#define TX_POWER_TABLE_SIZE 26

// TX Power Table Object
extern RF_TxPowerTable_Entry txPowerTable[];

// TI-RTOS RF Mode Object
extern RF_Mode RF_prop;

// RF Core API commands
extern rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioDivSetup;
extern rfc_CMD_FS_t RF_cmdFs;
extern rfc_CMD_TX_TEST_t RF_cmdTxTest;
extern rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv;
extern rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv;
extern rfc_CMD_PROP_CS_t RF_cmdPropCs;
extern rfc_CMD_NOP_t RF_cmdNop;

// RF Core API Overrides
extern uint32_t pOverrides[];
extern uint32_t pOverridesTxStd[];
extern uint32_t pOverridesTx20[];
extern uint32_t pOverrides_BT1[];
extern uint32_t pOverrides_BT05[];

// BLE configuration
// RF frontend configuration
#define FRONTEND_24G_DIFF_RF
#define FRONTEND_24G_EXT_BIAS

// Supported frequency bands
#define SUPPORT_FREQBAND_2400

// TX power table size definitions
#define TXPOWERTABLE_2400_PA5_SIZE 16 // 2400 MHz, 5 dBm

// TX power tables
extern RF_TxPowerTable_Entry txPowerTable_2400_pa5[]; // 2400 MHz, 5 dBm



//*********************************************************************************
//  RF Setting:   BLE, 1 Mbps, LE 1M (Preview settings)
//
//  PHY:          bt5le1m
//  Setting file: setting_bt5_le_1m.json
//*********************************************************************************

#define BLE_STACK_OVERRIDES_OFFSET 17

// PA table usage
#define RF_BLE_TX_POWER_TABLE_SIZE TXPOWERTABLE_2400_PA5_SIZE

#define RF_BLE_txPowerTable txPowerTable_2400_pa5

// TI-RTOS RF Mode object
extern RF_Mode RF_modeBle;

// RF Core API overrides
extern uint32_t pOverrides_bleCommon[];
extern uint32_t pOverrides_ble1Mbps[];
extern uint32_t pOverrides_ble2Mbps[];
extern uint32_t pOverrides_bleCoded[];

#endif // _FSK_TI_RADIO_CONFIG_H_
