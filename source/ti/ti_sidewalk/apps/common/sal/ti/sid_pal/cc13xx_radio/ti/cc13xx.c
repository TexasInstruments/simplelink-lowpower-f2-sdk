/******************************************************************************

 @file  cc13xx.c

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <ti/drivers/rf/RF.h>
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)
#include "ti/RFQueue.h"
#include "fsk_ti_radio_config.h"

#include <sid_pal_radio_ifc.h>
#include <sid_pal_delay_ifc.h>
#include <sid_pal_critical_region_ifc.h>
#include <sid_clock_ifc.h>
#include <sid_pal_log_ifc.h>

#include "ti/cc13xx.h"
#include "cc13xx_radio.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define MIN_RX_PREAMBLE_SIZE_IN_BITS 32

static uint8_t cc13xx_radio_state;
static RF_Params rfParams;
static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_CmdHandle cw_cmdHandle;
static RF_CmdHandle rx_cmdHandle;
static RF_CmdHandle tx_cmdHandle;

/* Buffer which contains all Data Entries for receiving data.
 * Pragmas are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core) */
static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)]
                                                                 __attribute__((aligned(4)));
/* Receive dataQueue for RF Core to fill in data */
static dataQueue_t dataQueue;

static uint8_t tx_packet[MAX_LENGTH];

static void cc13xx_set_radio_state(uint8_t state)
{
    cc13xx_radio_state |= state;
    return;
}

static uint8_t cc13xx_get_radio_state(void)
{
    return cc13xx_radio_state;
}

static void cc13xx_reset_radio_state(uint8_t state)
{
    RF_CmdHandle ch = RF_SCHEDULE_CMD_ERROR;

    if ((state == CC13XX_RADIO_SLEEP) || (state == CC13XX_RADIO_UNKNOWN)) {
        cc13xx_radio_state = CC13XX_RADIO_UNKNOWN;
        if (rfHandle) {
            ch = RF_CMDHANDLE_FLUSH_ALL;
            RF_cancelCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 0);
        }
    }

    if (state == CC13XX_RADIO_STANDBY) {
        if (cc13xx_radio_state & CC13XX_RADIO_CW) {
            cc13xx_radio_state &= ~CC13XX_RADIO_CW;
            ch = cw_cmdHandle;
            RF_cancelCmd(rfHandle, cw_cmdHandle, 0);
        }

        if (cc13xx_radio_state & CC13XX_RADIO_RX) {
            cc13xx_radio_state &= ~CC13XX_RADIO_RX;
            ch = rx_cmdHandle;
            RF_cancelCmd(rfHandle, rx_cmdHandle, 0);
        }
    }

    return;
}

static void cc13xx_clear_radio_state(uint8_t state)
{
    cc13xx_radio_state &= ~state;
    return;
}

static inline uint32_t cc13xx_get_gfsk_crc_len_in_bytes(cc13xx_gfsk_crc_types_t crc_type)
{
    uint8_t len;

    switch (crc_type) {
    case CC13XX_GFSK_CRC_OFF:
        len = FRAME_CRC_BYTES_ZERO;
        break;
    case CC13XX_GFSK_CRC_1_BYTE:
    case CC13XX_GFSK_CRC_1_BYTE_INV:
        len = FRAME_CRC_BYTES_ONE;
        break;
    case CC13XX_GFSK_CRC_2_BYTES:
    case CC13XX_GFSK_CRC_2_BYTES_INV:
    default:
        len = FRAME_CRC_BYTES_TWO;
        break;
    }

    return len;
}

static void cc13xx_event_notify(sid_pal_radio_events_t radio_event)
{
    halo_drv_cc13xx_ctx_t *drv_ctx = (halo_drv_cc13xx_ctx_t *)cc13xx_get_drv_ctx();

    if (drv_ctx && drv_ctx->report_radio_event) {
        drv_ctx->report_radio_event(radio_event);
    }

    return;
}

void cc13xx_radio_process(void)
{
    halo_drv_cc13xx_ctx_t *drv_ctx = (halo_drv_cc13xx_ctx_t *)cc13xx_get_drv_ctx();
    RF_EventMask e;

    drv_ctx->disable_cs_on_rx = false;

    // RX events
    sid_pal_enter_critical_region();
    e = drv_ctx->last_rx_event;
    drv_ctx->last_rx_event = 0;
    sid_pal_exit_critical_region();

    if ((e & RF_EventRxEntryDone) || (e & RF_EventRxBufFull)) {
        int32_t err = radio_fsk_process_rx_done(drv_ctx);

        RFQueue_nextEntry();

        if (err == RADIO_ERROR_NONE) {
            cc13xx_event_notify(SID_PAL_RADIO_EVENT_RX_DONE);
        } else {
            cc13xx_event_notify(SID_PAL_RADIO_EVENT_RX_ERROR);
        }
    } else if ((e & RF_EventRxIgnored) || (e & RF_EventRxEmpty) ||
               (e & RF_EventRxAborted) || (e & RF_EventRxCollisionDetected) ||
               (e & RF_EventInternalError) || (e & RF_EventRxNOk)) {
        cc13xx_event_notify(SID_PAL_RADIO_EVENT_RX_ERROR);
    } else if (e & RF_EventLastCmdDone) {
        if (RF_cmdPropRxAdv.status == PROP_DONE_RXTIMEOUT) {
            cc13xx_event_notify(SID_PAL_RADIO_EVENT_RX_TIMEOUT);
        }
        if ((RF_cmdPropCs.status == PROP_DONE_IDLETIMEOUT) || (RF_cmdPropCs.status == PROP_DONE_IDLE)) {
            cc13xx_event_notify(SID_PAL_RADIO_EVENT_RX_TIMEOUT);
        }
    }

    if (e & RF_EventLastCmdDone) {
        cc13xx_clear_radio_state(CC13XX_RADIO_RX);
    }

    // TX Events
    sid_pal_enter_critical_region();
    e = drv_ctx->last_tx_event;
    drv_ctx->last_tx_event = 0;
    sid_pal_exit_critical_region();

    if (e & RF_EventLastCmdDone) {
        if (RF_cmdPropTxAdv.status == PROP_DONE_OK) {
            cc13xx_event_notify(SID_PAL_RADIO_EVENT_TX_DONE);
        } else {
            cc13xx_event_notify(SID_PAL_RADIO_EVENT_CS_DONE);
        }
        cc13xx_clear_radio_state(CC13XX_RADIO_TX);
    }

    return;
}

static void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    halo_drv_cc13xx_ctx_t *drv_ctx = (halo_drv_cc13xx_ctx_t *)cc13xx_get_drv_ctx();
    sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &drv_ctx->radio_rx_packet->rcv_tm, NULL);

    // save event for processing
    drv_ctx->last_tx_event = e;

    if (drv_ctx->hw_irq_handler) {
        drv_ctx->hw_irq_handler();
    }

    return;
}

static void rx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    halo_drv_cc13xx_ctx_t *drv_ctx = (halo_drv_cc13xx_ctx_t *)cc13xx_get_drv_ctx();
    sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &drv_ctx->radio_rx_packet->rcv_tm, NULL);

    // save event for processing
    drv_ctx->last_rx_event = e;

    if (drv_ctx->hw_irq_handler) {
        drv_ctx->hw_irq_handler();
    }
}

static void standby_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventLastCmdDone || e & RF_EventCmdPreempted || e & RF_EventCmdCancelled) {
        cc13xx_clear_radio_state(CC13XX_RADIO_SLEEP);
        cc13xx_set_radio_state(CC13XX_RADIO_STANDBY);
    }
    return;
}

static void cc13xx_set_carrier_sense(uint32_t timeout, uint16_t mode, uint8_t next_op)
{
    RF_cmdPropCs.status = 0;
    RF_cmdPropCs.pNextOp = 0;
    RF_cmdPropCs.startTrigger.triggerType = TRIG_NOW;
    RF_cmdPropCs.startTrigger.pastTrig = 1;
    RF_cmdPropCs.condition.rule = COND_NEVER;
    RF_cmdPropCs.condition.nSkip = 0;
    if (timeout > 0) {
        RF_cmdPropCs.csEndTrigger.triggerType = TRIG_REL_START;
        RF_cmdPropCs.csEndTime = RF_convertUsToRatTicks(timeout);
    } else {
        RF_cmdPropCs.csEndTrigger.triggerType = TRIG_NEVER;
        RF_cmdPropCs.csEndTime = 0;
    }

    if (mode == CC13XX_CS_PREAMBLE_MODE) {
        RF_cmdPropCs.csConf.bEnaRssi = 0x0;
        RF_cmdPropCs.csConf.bEnaCorr = 0x1;
    } else {
        RF_cmdPropCs.csConf.bEnaRssi = 0x1;
        RF_cmdPropCs.csConf.bEnaCorr = 0x0;
    }

    RF_cmdPropCs.csConf.busyOp = 0x1;
    RF_cmdPropCs.csConf.idleOp = 0x0;
    if (next_op == CC13XX_CS_NEXT_RX) {
        RF_cmdPropCs.corrConfig.numCorrInv = 3;
        RF_cmdPropCs.corrConfig.numCorrBusy = 1;
    } else {
        RF_cmdPropCs.corrConfig.numCorrInv = 1;
        RF_cmdPropCs.corrConfig.numCorrBusy = 1;
    }
}

static void cc13xx_set_rx(uint32_t timeout)
{
    RF_cmdPropRxAdv.startTrigger.triggerType = TRIG_NOW;
    RF_cmdPropRxAdv.startTrigger.pastTrig = 1;
    RF_cmdPropRxAdv.startTime = 0;
    RF_cmdPropRxAdv.pNextOp = 0;
    RF_cmdPropRxAdv.pQueue = &dataQueue;
    RF_cmdPropRxAdv.rxConf.bAutoFlushIgnored = 1;
    RF_cmdPropRxAdv.rxConf.bAutoFlushCrcErr = 1;
    RF_cmdPropRxAdv.maxPktLen = MAX_LENGTH;
    RF_cmdPropRxAdv.pktConf.bRepeatOk = 0;
    RF_cmdPropRxAdv.pktConf.bRepeatNok = 0;
    if (timeout > 0) {
        RF_cmdPropRxAdv.endTrigger.triggerType = TRIG_REL_START;
        RF_cmdPropRxAdv.endTime = RF_convertUsToRatTicks(timeout);
    } else {
        RF_cmdPropRxAdv.endTrigger.triggerType = TRIG_NEVER;
        RF_cmdPropRxAdv.endTime = 0;
    }
}

uint16_t cc13xx_get_rx_entry(uint8_t *phr, uint8_t *buffer, int8_t *rssi)
{
    uint8_t phr_low = 0;
    uint8_t phr_high = 0;
    uint8_t phr_fcs_type = 0;
    uint8_t phr_whitening = 0;
    uint16_t phr_len_field = 0;
    uint16_t payload_len = 0;
    uint8_t crc_len = 0;
    uint8_t rx_append_len = 0;
    uint8_t rx_append_status[NUM_APPENDED_BYTES] = {0};
    uint8_t* entry_data_pointer = 0;
    rfc_dataEntryGeneral_t* current_data_entry = {0};

    current_data_entry = RFQueue_getDataEntry();

    phr_low = *(uint8_t*)(&current_data_entry->data);
    phr_high = *(uint8_t*)(&current_data_entry->data+1);
    phr_fcs_type = (phr_high & PHR_FIELD_FCS_TYPE) >> PHR_FIELD_FCS_TYPE_SHIFT;
    phr_whitening = (phr_high & PHR_FIELD_WHITEN) >> PHR_FIELD_WHITEN_SHIFT;
    phr_len_field = (uint16_t)((phr_high & PHR_PAYLOAD_CRC_LEN_HIG) <<
                               PHR_FIELD_PAYCRC_LEN_HIG_SHIFT);
    phr_len_field |= (uint16_t)(phr_low & PHR_PAYLOAD_CRC_LEN_LOW);

    if (phr_fcs_type == 0) {
        crc_len = PHR_FCS_LEN_4_BYTES;
        rx_append_len = REAR_APPENDED_10_BYTES;
    } else {
        crc_len = PHR_FCS_LEN_2_BYTES;
        rx_append_len = REAR_APPENDED_8_BYTES;
    }
    payload_len = phr_len_field - crc_len;
    if (payload_len > CC13XX_FSK_MAX_PAYLOAD_LEN) {
        payload_len = CC13XX_FSK_MAX_PAYLOAD_LEN;
    }

    phr[PHR_TYPE_OFFSET] = phr_high;
    phr[PHR_LENGTH_OFFSET] = phr_low;

    entry_data_pointer = (uint8_t*)(&current_data_entry->data + PHR_LEN_IN_BYTES);
    memcpy(buffer, entry_data_pointer, payload_len);

    entry_data_pointer = (uint8_t*)(&current_data_entry->data + PHR_LEN_IN_BYTES + payload_len);
    memcpy(rx_append_status, entry_data_pointer, rx_append_len);

    *rssi = rx_append_status[rx_append_len-REAR_APPENDED_RSSI_OFFSET];

    return payload_len;
}

/*
 * To open TI radio driver instance and also define RX queue, but not enable RX yet.
 */
cc13xx_status_t cc13xx_platform_init(void)
{
    cc13xx_status_t status = CC13XX_STATUS_ERROR;

    cc13xx_reset_radio_state(CC13XX_RADIO_UNKNOWN);

    if(RFQueue_defineQueue(&dataQueue, rxDataEntryBuffer, sizeof(rxDataEntryBuffer),
                           NUM_DATA_ENTRIES, MAX_LENGTH + NUM_APPENDED_BYTES)) {
        goto ret;
    }

    RF_Params_init(&rfParams);
    
    if (rfHandle != NULL)
    {
        RF_close(rfHandle);
        rfHandle = NULL;
    }
    
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
    RF_ScheduleCmdParams rfSchedParams;
    RF_ScheduleCmdParams_init(&rfSchedParams);

    RF_runScheduleCmd(rfHandle, (RF_Op*)&RF_cmdFs, &rfSchedParams, NULL, 0);
    status = CC13XX_STATUS_OK;

ret:
    return status;
}

cc13xx_status_t cc13xx_set_sleep(void)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;
    uint8_t mode = cc13xx_get_radio_state();

    if (rfHandle && (mode != CC13XX_RADIO_SLEEP) && (mode != CC13XX_RADIO_UNKNOWN)) {
        cc13xx_reset_radio_state(CC13XX_RADIO_SLEEP);
        RF_yield(rfHandle);
        cc13xx_set_radio_state(CC13XX_RADIO_SLEEP);
    }
    return status;
}

cc13xx_status_t cc13xx_set_standby(void)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;
    uint8_t mode = cc13xx_get_radio_state();

    if (rfHandle && ((mode == CC13XX_RADIO_SLEEP) || (mode == CC13XX_RADIO_UNKNOWN))) {
        RF_ScheduleCmdParams rfSchedParams;
        RF_ScheduleCmdParams_init(&rfSchedParams);

        RF_CmdHandle cmdHandle = RF_scheduleCmd(rfHandle, (RF_Op*) &RF_cmdNop, &rfSchedParams,
                       standby_callback, RF_EventLastCmdDone);
        if (cmdHandle < 0) {
            status = CC13XX_STATUS_ERROR;
            goto ret;
        }
    }
ret:
    return status;
}

cc13xx_status_t cc13xx_set_rf_control(void)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;

    return status;
}

cc13xx_status_t cc13xx_radio_start_carrier_sense(uint32_t timeout, sid_pal_radio_cad_param_exit_mode_t exit_mode)
{
    RF_EventMask bmEvent = 0;
    cc13xx_status_t status = CC13XX_STATUS_OK;
    RF_Callback pCb = 0;

    cc13xx_reset_radio_state(CC13XX_RADIO_STANDBY);

    switch(exit_mode) {
        case SID_PAL_RADIO_CAD_EXIT_MODE_CS_ONLY:
            cc13xx_set_carrier_sense(timeout, CC13XX_CS_PREAMBLE_MODE, CC13XX_CS_NEXT_RX);
            break;
        case SID_PAL_RADIO_CAD_EXIT_MODE_CS_RX:
            cc13xx_set_carrier_sense(timeout, CC13XX_CS_PREAMBLE_MODE, CC13XX_CS_NEXT_RX);
            cc13xx_set_rx(0);
            RF_cmdPropRxAdv.status = 0;
            RF_cmdPropCs.pNextOp = (rfc_radioOp_t *)&RF_cmdPropRxAdv;
            RF_cmdPropCs.condition.rule = COND_STOP_ON_FALSE;
            bmEvent = RF_EventRxEntryDone | RF_EventRxIgnored | RF_EventRxEmpty | RF_EventRxBufFull |
                      RF_EventRxAborted | RF_EventRxAborted | RF_EventInternalError | RF_EventRxNOk;
            pCb = &rx_callback;
            break;
        case SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT:
            cc13xx_set_carrier_sense(timeout, CC13XX_CS_PREAMBLE_MODE, CC13XX_CS_NEXT_TX);
            RF_cmdPropTxAdv.status = 0;
            RF_cmdPropCs.pNextOp = (rfc_radioOp_t *)&RF_cmdPropTxAdv;
            RF_cmdPropCs.condition.rule = COND_STOP_ON_TRUE;
            pCb = &tx_callback;
            break;
        case SID_PAL_RADIO_CAD_EXIT_MODE_ED_ONLY:
            cc13xx_set_carrier_sense(timeout, CC13XX_CS_RSSI_MODE, CC13XX_CS_NEXT_RX);
            break;
        case SID_PAL_RADIO_CAD_EXIT_MODE_ED_RX:
            cc13xx_set_carrier_sense(timeout, CC13XX_CS_RSSI_MODE, CC13XX_CS_NEXT_RX);
            cc13xx_set_rx(0);
            RF_cmdPropRxAdv.status = 0;
            RF_cmdPropCs.pNextOp = (rfc_radioOp_t *)&RF_cmdPropRxAdv;
            RF_cmdPropCs.condition.rule = COND_STOP_ON_FALSE;
            bmEvent = RF_EventRxEntryDone | RF_EventRxIgnored | RF_EventRxEmpty | RF_EventRxBufFull |
                      RF_EventRxAborted | RF_EventRxAborted | RF_EventInternalError | RF_EventRxNOk;
            pCb = &rx_callback;
            break;
        case SID_PAL_RADIO_CAD_EXIT_MODE_ED_LBT:
            cc13xx_set_carrier_sense(timeout, CC13XX_CS_RSSI_MODE, CC13XX_CS_NEXT_TX);
            RF_cmdPropTxAdv.status = 0;
            RF_cmdPropCs.pNextOp = (rfc_radioOp_t *)&RF_cmdPropTxAdv;
            RF_cmdPropCs.condition.rule = COND_STOP_ON_TRUE;
            pCb = &tx_callback;
            break;
        case SID_PAL_RADIO_CAD_EXIT_MODE_NONE:
        default:
            return status;
    }

    RF_ScheduleCmdParams rfSchedParams;
    RF_ScheduleCmdParams_init(&rfSchedParams);

    rx_cmdHandle = RF_scheduleCmd(rfHandle, (RF_Op*)&RF_cmdPropCs, &rfSchedParams, pCb, bmEvent);

    cc13xx_set_radio_state(CC13XX_RADIO_CAD);

    return status;
}

/*
 * This function is used to setup of RX handler and setup of rx_callback for incoming packets.
 */
cc13xx_status_t cc13xx_radio_start_rx(uint32_t timeout, bool continuous)
{
    RF_Op * radio_op;
    cc13xx_status_t status = CC13XX_STATUS_OK;
    halo_drv_cc13xx_ctx_t *drv_ctx = (halo_drv_cc13xx_ctx_t *)cc13xx_get_drv_ctx();

    cc13xx_reset_radio_state(CC13XX_RADIO_STANDBY);

#if CC13XX_RX_FOR_SHORT_WINDOW
    if (drv_ctx->disable_cs_on_rx) {
       cc13xx_set_rx(timeout);
       radio_op = (RF_Op*)&RF_cmdPropRxAdv;
    } else {
       cc13xx_set_carrier_sense(timeout, CC13XX_CS_PREAMBLE_MODE, CC13XX_CS_NEXT_RX);
       cc13xx_set_rx(CC13XX_SHR_LONG_PREAMBLE_LEN);
       RF_cmdPropRxAdv.status = 0;
       RF_cmdPropCs.pNextOp = (rfc_radioOp_t *)&RF_cmdPropRxAdv;
       RF_cmdPropCs.condition.rule = COND_STOP_ON_FALSE;
       radio_op = (RF_Op*)&RF_cmdPropCs;
    }
#else
    cc13xx_set_rx(timeout);
    radio_op = (RF_Op*)&RF_cmdPropRxAdv;
#endif
    if (continuous) {
        RF_cmdPropRxAdv.pktConf.bRepeatOk = 1;
        RF_cmdPropRxAdv.pktConf.bRepeatNok = 1;
    }

    RF_ScheduleCmdParams rfSchedParams;
    RF_ScheduleCmdParams_init(&rfSchedParams);

    RF_EventMask bmEvent = RF_EventRxEntryDone | RF_EventRxIgnored | RF_EventRxEmpty | RF_EventRxBufFull |
                           RF_EventRxAborted | RF_EventRxAborted | RF_EventInternalError | RF_EventRxNOk;

    rx_cmdHandle = RF_scheduleCmd(rfHandle, radio_op, &rfSchedParams, &rx_callback, bmEvent);

    if (rx_cmdHandle < 0) {
        status = CC13XX_STATUS_ERROR;
        goto ret;
    }

    cc13xx_set_radio_state(CC13XX_RADIO_RX);

ret:
    return status;
}

cc13xx_status_t cc13xx_set_tx_payload(const uint8_t *buffer, uint8_t size)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;

    memcpy(tx_packet, buffer, size);
    RF_cmdPropTxAdv.pktLen = size;
    RF_cmdPropTxAdv.pPkt = (uint8_t *)tx_packet;
    RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_NOW;

    return status;
}

cc13xx_status_t cc13xx_radio_start_tx(uint32_t timeout)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;

    cc13xx_reset_radio_state(CC13XX_RADIO_STANDBY);

    RF_ScheduleCmdParams rfSchedParams;
    RF_ScheduleCmdParams_init(&rfSchedParams);

    tx_cmdHandle = RF_scheduleCmd(rfHandle, (RF_Op*)&RF_cmdPropTxAdv, &rfSchedParams, &tx_callback, 0);

    if (tx_cmdHandle < 0) {
        status = CC13XX_STATUS_ERROR;
        goto ret;
    }

    cc13xx_set_radio_state(CC13XX_RADIO_TX);

ret:
    return status;
}

cc13xx_status_t cc13xx_set_tx_power(const int8_t tx_power)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;
    RF_CmdHandle cmdHandle;

    cmdHandle = RF_setTxPower(rfHandle, RF_TxPowerTable_findValue(txPowerTable ,tx_power));
    if (cmdHandle < 0) {
        status = CC13XX_STATUS_ERROR;
        goto ret;
    }

ret:
    return status;
}

/*
 * INPUT    freq is frequency in Hz
 * OUTPUT   frequency and fractFreq fields in TI RF_cmdFs structure instance
 *          where frequency is in MHz and fractFreq is remaining KHz in MHz multiply 65535
 * EXAMPLE  input freq 920345670Hz
 *          translate freq to 920.345670MHz, and then assgin to fields as below
 *          RF_cmdFs.frequency is 920
 *          RF_cmdFs.fractFreq is 0.345670 * 65535
 * REASON   because platform does not support float, so compute it by uint32 and to avoid overflow.
 *          first step is to divide 920345670 to 3 variables with temp buffer l_freq.
 *          variable mhz_freq is 920
 *          variable khz_freq is 345
 *          variable hz_freq is 670
 *          second step is to multiple khz_freq and hz_freq with 65535 and translate to MHz unit.
 */
cc13xx_status_t cc13xx_set_rf_frequency(const uint32_t freq)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;
    RF_CmdHandle cmdHandle;

    if (freq != 0) {
        uint32_t mhz_freq = 0;
        uint32_t khz_freq = 0;
        uint32_t hz_freq = 0;
        uint32_t l_freq = 0;

        /* first step */
        mhz_freq = HZ_TO_MHZ(freq);
        khz_freq = freq - (mhz_freq * HZ_IN_MHZ);
        if (khz_freq != 0) {
            l_freq = khz_freq / HZ_IN_KHZ;
        } else {
            l_freq = 0;
        }
        hz_freq = khz_freq - (l_freq * HZ_IN_KHZ);
        khz_freq = l_freq;

        /* second step */
        khz_freq = khz_freq * UINT16_MAX / HZ_IN_KHZ;
        hz_freq = hz_freq * UINT16_MAX / HZ_IN_MHZ;
        khz_freq += hz_freq + 1;
        RF_cmdFs.frequency = mhz_freq;
        RF_cmdFs.fractFreq = khz_freq;

        RF_ScheduleCmdParams rfSchedParams;
        RF_ScheduleCmdParams_init(&rfSchedParams);

        cmdHandle = RF_scheduleCmd(rfHandle, (RF_Op*)&RF_cmdFs, &rfSchedParams, NULL, 0);

        if (cmdHandle < 0) {
            status = CC13XX_STATUS_ERROR;
            goto ret;
        }
    } else {
        status = CC13XX_STATUS_UNKNOWN_VALUE;
    }

ret:
    return status;
}

cc13xx_status_t cc13xx_get_rssi_inst(int16_t *rssi)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;

    *rssi = RF_getRssi(rfHandle);

    return status;
}

cc13xx_status_t cc13xx_get_random_numbers(uint32_t* numbers, unsigned int n)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;
    int8_t i, seed = -1;
    int16_t rssi = 0;
    uint32_t noise = 0;
    uint32_t r_val = 0;

    if (n < 1) {
        n = 1;
    }

    for (i = 0 ; i < RF_RANDOM_TIMES; i++) {
        sid_pal_delay_us(n);
        rssi = sid_pal_radio_rssi();
        noise += (-1 * rssi);
        if (seed == -1) {
            seed = (noise % 8);
        }
        r_val |= (((noise % 16) & 0xf) << (((seed + i) % 8) * 4));
        r_val |= noise;
    }

    *numbers = r_val;

    return status;
}

cc13xx_status_t cc13xx_set_gfsk_sync_word(const uint8_t* sync_word, const uint8_t sync_word_len)
{
    cc13xx_status_t status = CC13XX_STATUS_ERROR;

    if (sync_word_len <= CC13XX_MAX_SYNCWORD_BYTES) {
        uint32_t sync_wd = 0;
        uint8_t sync_bits;
        uint8_t i;

        for (i = 0; i < sync_word_len; i++) {
            sync_wd = (sync_wd << BIT_IN_BYTE) | sync_word[i];
        }
        RF_cmdPropTxAdv.syncWord = sync_wd;
        RF_cmdPropRxAdv.syncWord0 = sync_wd;

        sync_bits = BIT_IN_BYTE * sync_word_len;
        if (RF_cmdPropRadioDivSetup.formatConf.nSwBits != sync_bits) {
            RF_cmdPropRadioDivSetup.formatConf.nSwBits = sync_bits;
            cc13xx_set_rf_control();
        }

        status = CC13XX_STATUS_OK;
    }

    return status;
}

cc13xx_status_t cc13xx_set_gfsk_mod_params(const cc13xx_mod_params_gfsk_t* params)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;
    halo_drv_cc13xx_ctx_t *drv_ctx = (halo_drv_cc13xx_ctx_t *)cc13xx_get_drv_ctx();
    uint8_t modified = 0;

    switch (params->br_in_bps) {
    case RADIO_FSK_BR_50KBPS:
        if ((RF_cmdPropRadioDivSetup.symbolRate.rateWord != TI_FSK_BR_50KBPS) ||
            (RF_cmdPropRadioDivSetup.modulation.deviation != TI_FSK_BR_50KBPS_DEVI_25KHZ)
            || (RF_cmdPropRadioDivSetup.rxBw != TI_FSK_BR_50KBPS_RXBW_155_4KHZ)) {
            modified = 1;
            RF_cmdPropRadioDivSetup.symbolRate.rateWord = TI_FSK_BR_50KBPS;
            RF_cmdPropRadioDivSetup.modulation.deviation = TI_FSK_BR_50KBPS_DEVI_25KHZ;
            RF_cmdPropRadioDivSetup.rxBw = TI_FSK_BR_50KBPS_RXBW_155_4KHZ;
            RF_cmdPropRadioDivSetup.pRegOverride = pOverrides_BT1;
        }
        break;
    case RADIO_FSK_BR_150KBPS:
        if ((RF_cmdPropRadioDivSetup.symbolRate.rateWord != TI_FSK_BR_150KBPS) ||
            (RF_cmdPropRadioDivSetup.modulation.deviation != TI_FSK_BR_150KBPS_DEVI_57KHZ)
            || (RF_cmdPropRadioDivSetup.rxBw != TI_FSK_BR_150KBPS_RXBW_310_8KHZ)) {
            modified = 1;
            RF_cmdPropRadioDivSetup.symbolRate.rateWord = TI_FSK_BR_150KBPS;
            RF_cmdPropRadioDivSetup.modulation.deviation = TI_FSK_BR_150KBPS_DEVI_57KHZ;
            RF_cmdPropRadioDivSetup.rxBw = TI_FSK_BR_150KBPS_RXBW_310_8KHZ;
            RF_cmdPropRadioDivSetup.pRegOverride = pOverrides_BT05;
        }
        break;
    case RADIO_FSK_BR_250KBPS:
        if ((RF_cmdPropRadioDivSetup.symbolRate.rateWord != TI_FSK_BR_250KBPS) ||
            (RF_cmdPropRadioDivSetup.modulation.deviation != TI_FSK_BR_250KBPS_DEVI_62_5KHZ)
            || (RF_cmdPropRadioDivSetup.rxBw != TI_FSK_BR_250KBPS_RXBW_546_3KHZ)) {
            modified = 1;
            RF_cmdPropRadioDivSetup.symbolRate.rateWord = TI_FSK_BR_250KBPS;
            RF_cmdPropRadioDivSetup.modulation.deviation = TI_FSK_BR_250KBPS_DEVI_62_5KHZ;
            RF_cmdPropRadioDivSetup.rxBw = TI_FSK_BR_250KBPS_RXBW_546_3KHZ;
            RF_cmdPropRadioDivSetup.pRegOverride = pOverrides_BT05;
        }
        break;
    default:
        return CC13XX_STATUS_UNKNOWN_VALUE;
    }

    if (modified) {
        cc13xx_set_rf_control();
        drv_ctx->symbol_period = CC13XX_SYMBOL_PERIOD_US(params->br_in_bps);
    }

    return status;
}

cc13xx_status_t cc13xx_set_gfsk_pkt_params(const cc13xx_pkt_params_gfsk_t* params)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;
    halo_drv_cc13xx_ctx_t *drv_ctx = (halo_drv_cc13xx_ctx_t *)cc13xx_get_drv_ctx();
    uint8_t modified = 0;
    uint8_t pream_bytes = 0;

    if (RF_cmdPropRadioDivSetup.formatConf.nSwBits != params->sync_word_len_in_bits) {
        RF_cmdPropRadioDivSetup.formatConf.nSwBits = params->sync_word_len_in_bits;
        modified = 1;
    }

    uint32_t pre_time_ticks;
    pre_time_ticks = (params->pbl_len_in_bits * drv_ctx->symbol_period) * CC13XX_RAT_TICKS_PER_US;
    RF_cmdPropTxAdv.preTrigger.triggerType = TRIG_REL_START;
    RF_cmdPropTxAdv.preTime = pre_time_ticks;

    /*
     * when the preamble is expected to be shorter than 32 bits
     * on the sender side, the receiver should not use CS + RX
     * for preamble detection and should just turn on RX. This
     * fixes issues with acking, due to the shorter preamble size
     * used compared to regular packets
     */
    if (params->pbl_len_in_bits <= MIN_RX_PREAMBLE_SIZE_IN_BITS) {
        drv_ctx->disable_cs_on_rx = true;
    }

    if (modified) {
        cc13xx_set_rf_control();
    }

    return status;
}

cc13xx_status_t cc13xx_set_tx_cw(void)
{
    cc13xx_status_t status = CC13XX_STATUS_OK;

    cc13xx_reset_radio_state(CC13XX_RADIO_STANDBY);

    RF_cmdTxTest.config.bUseCw = 1;

    RF_ScheduleCmdParams rfSchedParams;
    RF_ScheduleCmdParams_init(&rfSchedParams);

    cw_cmdHandle = RF_scheduleCmd(rfHandle, (RF_Op*)&RF_cmdTxTest, &rfSchedParams, NULL, 0);

    if (cw_cmdHandle < 0) {
        status = CC13XX_STATUS_ERROR;
        goto ret;
    }

    cc13xx_set_radio_state(CC13XX_RADIO_CW);

ret:
    return status;
}

uint32_t cc13xx_get_gfsk_time_on_air_numerator(const cc13xx_pkt_params_gfsk_t* pkt_p)
{
    return pkt_p->pbl_len_in_bits + FRAME_HDR_LEN_IN_BITS + pkt_p->sync_word_len_in_bits +
           ((pkt_p->pld_len_in_bytes + (pkt_p->addr_cmp == CC13XX_GFSK_ADDR_CMP_FILT_OFF ? 0 : 1) +
           cc13xx_get_gfsk_crc_len_in_bytes(pkt_p->crc_type)) << 3);
}

uint32_t cc13xx_get_gfsk_time_on_air_in_ms(const cc13xx_pkt_params_gfsk_t* pkt_p,
                                           const cc13xx_mod_params_gfsk_t* mod_p)
{
    return (MS_IN_SEC * cc13xx_get_gfsk_time_on_air_numerator(pkt_p) + mod_p->br_in_bps - 1)
            / mod_p->br_in_bps;
}