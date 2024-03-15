/*******************************************************************************
 Filename:       application.h
 Revised:
 Revision:

 Description:    This file contains the task that handles CLI serial
                 communication via the ns_cmdline API


 Copyright 2012-2020 Texas Instruments Incorporated. All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License").  You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio
 frequency transceiver, which is integrated into your product.  Other than for
 the foregoing purpose, you may not use, reproduce, copy, prepare derivative
 works of, modify, distribute, perform, display or sell this Software and/or
 its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED Ã¯Â¿Â½AS ISÃ¯Â¿Â½ WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.
 *******************************************************************************/

#include "mbed-mesh-api/mesh_interface_types.h"
#include "saddr.h"
#include "api_mac.h"
#include "6LoWPAN/ws/ws_bootstrap.h"

/* Network name; max 32 octets + terminating 0 */
#define MAX_NETWORK_NAME_SIZE 33
#define CHANNEL_BITMAP_SIZE 17
#define HWADDR_SIZE 8
#define SIZE_OF_NEIGH_LIST 32 // max value currently set in nanostack code

#ifdef COAP_SERVICE_ENABLE
#define COAP_PORT 5683
extern int8_t service_id;
#endif

#define EAPOL_EUI_LIST_SIZE 50

typedef struct configurable_props_s {
    int8_t ccaDefaultdBm;
    int8_t phyTxPower;
    uint8_t uc_channel_function;
    uint8_t uc_channel_list[CHANNEL_BITMAP_SIZE];
    uint8_t uc_fixed_channel;
    uint8_t uc_dwell_interval;
    uint8_t bc_channel_function;
    uint8_t bc_channel_list[CHANNEL_BITMAP_SIZE];
    uint8_t bc_fixed_channel;
    uint32_t bc_interval;
    uint8_t bc_dwell_interval;
    uint8_t async_channel_list[CHANNEL_BITMAP_SIZE];
    uint16_t pan_id;
    char network_name[MAX_NETWORK_NAME_SIZE];
    mesh_device_type_t wisun_device_type;
    uint32_t ch0_center_frequency; //in KHz
    uint16_t config_channel_spacing;
    uint8_t config_phy_id;
    uint8_t config_reg_domain;
    uint8_t operating_class;
    uint8_t operating_mode;
    uint8_t hwaddr[HWADDR_SIZE];

}configurable_props_t;

typedef struct nbr_node_metrics_s {
    sAddrExt_t mac_eui;
    uint8_t rssi_in;
    uint8_t rssi_out;
    /* add whatever else is needed */
}nbr_node_metrics_t;

#ifdef WISUN_TEST_METRICS
typedef struct Heap_debug
{
    uint32_t heap_sector_size;
    uint32_t heap_sector_allocated_bytes;
    uint32_t heap_sector_allocated_bytes_max;
} Heap_debug_s;

typedef struct test_metrics
{
    uint16_t revision;
    uint16_t length;
    JOIN_TIME_s join_time;
    MAC_debug_s mac_debug;
    Heap_debug_s heap_debug;
    MAC_Perf_Data mac_perf_data;
    uint32_t udpPktCnt;
    uint16_t current_rank;
} test_metrics_s;
#endif
