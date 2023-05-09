/*
 *  ======== ti_radio_config.h ========
 *  Configured RadioConfig module definitions
 *  
 *  Radio Config module version : 1.2
 *  SmartRF Studio data version : 2.14.0
 */
#ifndef _TI_RADIO_CONFIG_H_
#define _TI_RADIO_CONFIG_H_

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_ieee_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include <ti/drivers/rf/RF.h>

// *********************************************************************************
//   RF Frontend configuration
// *********************************************************************************
// Which launchpad the RF design is based on
#define LAUNCHXL_CC1352R1

// RF frontend configuration
#define FRONTEND_SUB1G_DIFF_RF
#define FRONTEND_SUB1G_EXT_BIAS
#define FRONTEND_24G_DIFF_RF
#define FRONTEND_24G_EXT_BIAS

// Supported frequency bands
#define SUPPORT_FREQBAND_868
#define SUPPORT_FREQBAND_2400

//*********************************************************************************
//  RF Setting:   IEEE 802.15.4 O-QPSK DSSS PHY (250 kbps) at 2450 MHz band
//
//  PHY:          ieee154     
//  Setting file: setting_ieee_802_15_4.json
//*********************************************************************************

// TX Power table size definition
#define TX_POWER_TABLE_SIZE_ieee154 16

// TX Power Table Object
extern RF_TxPowerTable_Entry txPowerTable_ieee154[];

// TI-RTOS RF Mode Object
extern RF_Mode RF_prop_ieee154;

// RF Core API commands
extern const rfc_CMD_RADIO_SETUP_t RF_cmdRadioSetup_ieee154;
extern const rfc_CMD_FS_t RF_cmdFs_ieee154;
extern const rfc_CMD_IEEE_TX_t RF_cmdIeeeTx_ieee154;
extern const rfc_CMD_IEEE_RX_t RF_cmdIeeeRx_ieee154;
extern const rfc_CMD_IEEE_CSMA_t RF_cmdIeeeCsma_ieee154;
extern const rfc_CMD_IEEE_RX_ACK_t RF_cmdIeeeRxAck_ieee154;

// RF Core API Overrides
extern uint32_t pOverrides_ieee154[];


//*********************************************************************************
//  RF Setting:   50 kbps, 2-GFSK, 25 kHz deviation, IEEE 802.15.4g MR-FSK PHY mode
//
//  PHY:          2gfsk50kbps154g     
//  Setting file: setting_tc106_154g.json
//*********************************************************************************

// TX Power table size definition
#define TX_POWER_TABLE_SIZE_2gfsk50kbps154g 20

// TX Power Table Object
extern RF_TxPowerTable_Entry txPowerTable_2gfsk50kbps154g[];

// TI-RTOS RF Mode Object
extern RF_Mode RF_prop_2gfsk50kbps154g;

// RF Core API commands
extern const rfc_CMD_PROP_RADIO_DIV_SETUP_t RF_cmdPropRadioDivSetup_2gfsk50kbps154g;
extern const rfc_CMD_FS_t RF_cmdFs_2gfsk50kbps154g;
extern const rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv_2gfsk50kbps154g;
extern const rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv_2gfsk50kbps154g;
extern const rfc_CMD_PROP_CS_t RF_cmdPropCs_2gfsk50kbps154g;

// RF Core API Overrides
extern uint32_t pOverrides_2gfsk50kbps154g[];

//*********************************************************************************
//  RF Setting:   200 kbps, 2-GFSK, 50 kHz deviation, IEEE 802.15.4g FSK PHY mode, 915 MHz
//
//  PHY:          2gfsk200kbps154g     
//  Setting file: setting_tc146_154g.json
//*********************************************************************************

// TX Power table size definition
#define TX_POWER_TABLE_SIZE_2gfsk200kbps154g 20

// TX Power Table Object
extern RF_TxPowerTable_Entry txPowerTable_2gfsk200kbps154g[];

// TI-RTOS RF Mode Object
extern RF_Mode RF_prop_2gfsk200kbps154g;

// RF Core API commands
extern const rfc_CMD_PROP_RADIO_DIV_SETUP_t RF_cmdPropRadioDivSetup_2gfsk200kbps154g;
extern const rfc_CMD_FS_t RF_cmdFs_2gfsk200kbps154g;
extern const rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv_2gfsk200kbps154g;
extern const rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv_2gfsk200kbps154g;
extern const rfc_CMD_PROP_CS_t RF_cmdPropCs_2gfsk200kbps154g;

// RF Core API Overrides
extern uint32_t pOverrides_2gfsk200kbps154g[];

//*********************************************************************************
//  RF Setting:   5 kbps, SimpleLink Long Range (20 kchip/s, 2-GFSK, conv. FEC r=1/2 K=7, DSSS SF=2, Tx dev.: 5 kHz, Rx BW: 34 kHz)
//
//  PHY:          slr5kbps2gfsk     
//  Setting file: setting_tc480.json
//*********************************************************************************

// TX Power table size definition
#define TX_POWER_TABLE_SIZE_slr5kbps2gfsk 20

// TX Power Table Object
extern RF_TxPowerTable_Entry txPowerTable_slr5kbps2gfsk[];

// TI-RTOS RF Mode Object
extern RF_Mode RF_prop_slr5kbps2gfsk;

// RF Core API commands
extern const rfc_CMD_PROP_RADIO_DIV_SETUP_t RF_cmdPropRadioDivSetup_slr5kbps2gfsk;
extern const rfc_CMD_FS_t RF_cmdFs_slr5kbps2gfsk;
extern const rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv_slr5kbps2gfsk;
extern const rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv_slr5kbps2gfsk;
extern const rfc_CMD_PROP_CS_t RF_cmdPropCs_slr5kbps2gfsk;

// RF Core API Overrides
extern uint32_t pOverrides_slr5kbps2gfsk[];

#endif // _TI_RADIO_CONFIG_H_
