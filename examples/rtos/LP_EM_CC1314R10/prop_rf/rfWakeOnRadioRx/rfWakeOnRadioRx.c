/*
 * Copyright (c) 2016-2019, Texas Instruments Incorporated
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

/***** Includes *****/
/* Standard C Libraries */
#include <stdlib.h>

/* TI-RTOS Header files */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>

/* Board Header files */
#include "ti_drivers_config.h"

/* Application Header files */
#include "RFQueue.h"
#include <ti_radio_config.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)
#include DeviceFamily_constructPath(driverlib/ioc.h)

/***** Defines *****/
/* Wake-on-Radio wakeups per second */
#define WOR_WAKEUPS_PER_SECOND  2

/* Wake-on-Radio mode. Can be:
 * - RSSI only
 * - PQT, preamble detection
 * - Both, first RSSI and then PQT if RSSI  */
#define WOR_MODE CarrierSenseMode_RSSIandPQT

/* Threshold for RSSI based Carrier Sense in dBm */
#define WOR_RSSI_THRESHOLD      ((int8_t)(-111))

/* Data Rate in use */
#define WOR_RF_PHY_DATARATE_50KBPS  0 // 2-GFSK 50Kbps
#define WOR_RF_PHY_DATARATE_100KBPS 1 // 2-GFSK 100Kbps
#define WOR_RF_PHY_DATARATE_200KBPS 2 // 2-GFSK 200Kbps
#define WOR_RF_PHY_DATARATE_300KBPS 3 // 2-GFSK 300Kbps
#define WOR_RF_PHY_DATARATE_400KBPS 4 // 2-GFSK 400Kbps
#define WOR_RF_PHY_DATARATE_500KBPS 5 // 2-GFSK 500Kbps

#define WOR_RF_PHY_DATARATE WOR_RF_PHY_DATARATE_50KBPS

/* Macro used to set actual wakeup interval */
#define WOR_WAKE_UP_MARGIN_S 0.005f
#define WOR_WAKE_UP_INTERVAL_RAT_TICKS(x) \
    ((uint32_t)(4000000*(1.0f/(x) - (WOR_WAKE_UP_MARGIN_S))))

/* TI-RTOS Task configuration */
#define RX_TASK_STACK_SIZE 1024
#define RX_TASK_PRIORITY   2

/* TX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             31 /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     1  /* Length byte included in the stored packet */

/***** Type declarations *****/
/* General wake-on-radio RX statistics */
struct WorStatistics {
  uint32_t doneIdle;
  uint32_t doneIdleTimeout;
  uint32_t doneRxTimeout;
  uint32_t doneOk;
};

/* Modes of carrier sense possible */
enum CarrierSenseMode {
    CarrierSenseMode_RSSI,
    CarrierSenseMode_PQT,
    CarrierSenseMode_RSSIandPQT,
};


/***** Variable declarations *****/

/* RF driver object and handle */
static RF_Object rfObject;
static RF_Handle rfHandle;

/* General wake-on-radio sniff status statistics and statistics from the RF Core about received packets */
static volatile struct WorStatistics worStatistics;
static rfc_propRxOutput_t rxStatistics;

/* Buffer which contains all Data Entries for receiving data.
 * Pragmas are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core) */
#if defined(__TI_COMPILER_VERSION__)
    #pragma DATA_ALIGN (rxDataEntryBuffer, 4);
        static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma data_alignment = 4
        static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];
#elif defined(__GNUC__)
        static uint8_t rxDataEntryBuffer [RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
            MAX_LENGTH, NUM_APPENDED_BYTES)] __attribute__ ((aligned (4)));
#else
    #error This compiler is not supported.
#endif

/* RX Data Queue and Data Entry pointer to read out received packets */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;

/* Received packet's length and pointer to the payload */
static uint8_t packetLength;
static uint8_t* packetDataPointer;

static volatile uint8_t dummy;

/* Sniff command for doing combined Carrier Sense and RX*/
static rfc_CMD_PROP_RX_SNIFF_t RF_cmdPropRxSniff;

/***** Prototypes *****/
static void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
static void initializeSniffCmdFromRxCmd(rfc_CMD_PROP_RX_SNIFF_t* rxSniffCmd, rfc_CMD_PROP_RX_t* rxCmd);
static void configureSniffCmd(rfc_CMD_PROP_RX_SNIFF_t* rxSniffCmd, enum CarrierSenseMode mode, uint32_t datarate, uint8_t wakeupPerSecond);
static uint32_t calculateSymbolRate(uint8_t prescaler, uint32_t rateWord);

/* RX task function. Executed in Task context when the scheduler starts. */
void *mainThread(void *arg0)
{
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    GPIO_setConfig(CONFIG_GPIO_RLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);

    /* Route out LNA active pin to LED1 */
    GPIO_setMux(CONFIG_GPIO_RLED, IOC_PORT_RFC_GPO0);

    /* Create queue and data entries */
    if (RFQueue_defineQueue(&dataQueue,
                            rxDataEntryBuffer,
                            sizeof(rxDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        /* Failed to allocate space for all data entries */
        while(1);
    }

    /* Copy all RX options from the SmartRF Studio exported RX command to the RX Sniff command */
    initializeSniffCmdFromRxCmd(&RF_cmdPropRxSniff, &RF_cmdPropRx);

    /* Configure RX part of RX_SNIFF command */
    RF_cmdPropRxSniff.pQueue    = &dataQueue;
    RF_cmdPropRxSniff.pOutput   = (uint8_t*)&rxStatistics;
    RF_cmdPropRxSniff.maxPktLen = MAX_LENGTH;

    /* Discard ignored packets and CRC errors from Rx queue */
    RF_cmdPropRxSniff.rxConf.bAutoFlushIgnored = 1;
    RF_cmdPropRxSniff.rxConf.bAutoFlushCrcErr  = 1;

    /* Calculate datarate from prescaler and rate word */
#if defined(DeviceFamily_CC26X0R2)
    uint32_t datarate = calculateSymbolRate(RF_cmdPropRadioSetup.symbolRate.preScale,
                                            RF_cmdPropRadioSetup.symbolRate.rateWord);
#else
    uint32_t datarate = calculateSymbolRate(RF_cmdPropRadioDivSetup.symbolRate.preScale,
                                            RF_cmdPropRadioDivSetup.symbolRate.rateWord);
#endif// DeviceFamily_CC26X0R2

    /* Configure Sniff-mode part of the RX_SNIFF command */
    configureSniffCmd(&RF_cmdPropRxSniff, WOR_MODE, datarate, WOR_WAKEUPS_PER_SECOND);

    /* Request access to the radio */
#if defined(DeviceFamily_CC26X0R2)
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
#else
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
#endif// DeviceFamily_CC26X0R2

    /* Set frequency */
    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, &callback, 0);

    /* Save the current radio time */
    RF_cmdPropRxSniff.startTime = RF_getCurrentTime();

    /* Enter main loop */
    while(1)
    {
        /* Set next wakeup time in the future */
        RF_cmdPropRxSniff.startTime += WOR_WAKE_UP_INTERVAL_RAT_TICKS(WOR_WAKEUPS_PER_SECOND);

        /* Schedule RX */
        RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropRxSniff, RF_PriorityNormal, &callback, RF_EventRxEntryDone);

        /* Log RX_SNIFF status */
        switch(RF_cmdPropRxSniff.status) {
            case PROP_DONE_IDLE:
                /* Idle based on RSSI */
                worStatistics.doneIdle++;
                break;
            case PROP_DONE_IDLETIMEOUT:
                /* Idle based on PQT */
                worStatistics.doneIdleTimeout++;
                break;
            case PROP_DONE_RXTIMEOUT:
                /* Got valid preamble on the air, but did not find sync word */
                worStatistics.doneRxTimeout++;
                break;
            case PROP_DONE_OK:
                /* Received packet */
                worStatistics.doneOk++;
                break;
            default:
                /* Unhandled status */
                break;
        };
    }
}

/* Calculates datarate from prescaler and rate word */
static uint32_t calculateSymbolRate(uint8_t prescaler, uint32_t rateWord)
{
    /* Calculate datarate according to TRM Section 23.7.5.2:
     * f_baudrate = (R * f_ref)/(p * 2^20)
     *   - R = rateWord
     *   - f_ref = 24Mhz
     *   - p = prescaler */
    uint64_t numerator = rateWord*24000000ULL;
    uint64_t denominator = prescaler*1048576ULL;
    uint32_t result = (uint32_t)(numerator/denominator);
    return result;
}

/* Copies all RX options from the SmartRF Studio exported RX command to the RX Sniff command */
static void initializeSniffCmdFromRxCmd(rfc_CMD_PROP_RX_SNIFF_t* rxSniffCmd, rfc_CMD_PROP_RX_t* rxCmd)
{

    /* Copy RX configuration from RX command */
    memcpy(rxSniffCmd, rxCmd, sizeof(rfc_CMD_PROP_RX_t));

    /* Change to RX_SNIFF command from RX command */
    rxSniffCmd->commandNo = CMD_PROP_RX_SNIFF;
}

/* Configures Sniff-mode part of the RX_SNIFF command based on mode, datarate and wakeup interval */
static void configureSniffCmd(rfc_CMD_PROP_RX_SNIFF_t* rxSniffCmd, enum CarrierSenseMode mode, uint32_t datarate, uint8_t wakeupPerSecond)
{
    /* Enable or disable RSSI */
    if ((mode == CarrierSenseMode_RSSI) || (mode == CarrierSenseMode_RSSIandPQT)) {
        rxSniffCmd->csConf.bEnaRssi        = 1;
    } else {
        rxSniffCmd->csConf.bEnaRssi        = 0;
    }

    /* Enable or disable PQT */
    if ((mode == CarrierSenseMode_PQT) || (mode == CarrierSenseMode_RSSIandPQT)) {
        rxSniffCmd->csConf.bEnaCorr        = 1;
        rxSniffCmd->csEndTrigger.triggerType  = TRIG_REL_START;
    } else {
        rxSniffCmd->csConf.bEnaCorr        = 0;
        rxSniffCmd->csEndTrigger.triggerType  = TRIG_NEVER;
    }

    /* General Carrier Sense configuration */
    rxSniffCmd->csConf.operation       = 1; /* Report Idle if RSSI reports Idle to quickly exit if not above
                                                 RSSI threshold */
    rxSniffCmd->csConf.busyOp          = 0; /* End carrier sense on channel Busy (the receiver will continue when
                                                 carrier sense ends, but it will then not end if channel goes Idle) */
    rxSniffCmd->csConf.idleOp          = 1; /* End on channel Idle */
    rxSniffCmd->csConf.timeoutRes      = 1; /* If the channel is invalid, it will return PROP_DONE_IDLE_TIMEOUT */

    /* RSSI configuration */
    rxSniffCmd->numRssiIdle            = 1; /* One idle RSSI samples signals that the channel is idle */
    rxSniffCmd->numRssiBusy            = 1; /* One busy RSSI samples signals that the channel is busy */
    rxSniffCmd->rssiThr    = (int8_t)WOR_RSSI_THRESHOLD; /* Set the RSSI threshold in dBm */

    /* PQT configuration */
    rxSniffCmd->corrConfig.numCorrBusy = 1;   /* One busy PQT samples signals that the channel is busy */
    rxSniffCmd->corrConfig.numCorrInv  = 1;   /* One busy PQT samples signals that the channel is busy */

    /* Calculate basic timing parameters */
    uint32_t symbolLengthUs  = 1000000UL/datarate;
    uint32_t preambleSymbols = (1000000UL/wakeupPerSecond)/symbolLengthUs;
#if defined(DeviceFamily_CC26X0R2)
    uint8_t syncWordSymbols  = RF_cmdPropRadioSetup.formatConf.nSwBits;
#else
    uint8_t syncWordSymbols  = RF_cmdPropRadioDivSetup.formatConf.nSwBits;
#endif// DeviceFamily_CC26X0R2

    /* Calculate sniff mode parameters */
    #define US_TO_RAT_TICKS 4
    #define CORR_PERIOD_SYM_MARGIN 16
    #define RX_END_TIME_SYM_MARGIN 8
    #define CS_END_TIME_MIN_TIME_SYM 30
#if ((WOR_RF_PHY_DATARATE == WOR_RF_PHY_DATARATE_50KBPS)  || \
     (WOR_RF_PHY_DATARATE == WOR_RF_PHY_DATARATE_100KBPS) || \
     (WOR_RF_PHY_DATARATE == WOR_RF_PHY_DATARATE_200KBPS))
    #define CS_END_TIME_MIN_TIME_STATIC_US 150
#elif ((WOR_RF_PHY_DATARATE == WOR_RF_PHY_DATARATE_300KBPS) || \
       (WOR_RF_PHY_DATARATE == WOR_RF_PHY_DATARATE_400KBPS))
    #define CS_END_TIME_MIN_TIME_STATIC_US 200
#elif (WOR_RF_PHY_DATARATE == WOR_RF_PHY_DATARATE_500KBPS)
    #define CS_END_TIME_MIN_TIME_STATIC_US 250
#else
#error "WOR_RF_PHY_DATARATE is undefined or has an invalid option"
#endif

    /* Represents the time in which we need to receive corrConfig.numCorr* correlation peaks to detect preamble.
     * When continously checking the preamble quality, this period has to be wide enough to also contain the sync
     * word, with a margin. If it is not, then there is a chance the SNIFF command will abort while receiving the
     * sync word, as it no longer detects a preamble. */
    uint32_t correlationPeriodUs = (syncWordSymbols + CORR_PERIOD_SYM_MARGIN)*symbolLengthUs;

    /* Represents the time where we will force a check if preamble is present (only done once).
     * The main idea is that his should be shorter than "correlationPeriodUs" so that if we get RSSI valid, but
     * there is not a valid preamble on the air, we will leave RX as quickly as possible. */
    uint32_t csEndTimeUs = (CS_END_TIME_MIN_TIME_SYM*symbolLengthUs + CS_END_TIME_MIN_TIME_STATIC_US);

    /* Represents the maximum time from the startTrigger to when we expect a sync word to be received. */
    uint32_t rxEndTimeUs = (preambleSymbols + syncWordSymbols + RX_END_TIME_SYM_MARGIN)*symbolLengthUs;

    /* Set sniff mode timing configuration in sniff command in RAT ticks */
    rxSniffCmd->corrPeriod = (uint16_t)(correlationPeriodUs * US_TO_RAT_TICKS);
    rxSniffCmd->csEndTime  = (uint32_t)(csEndTimeUs * US_TO_RAT_TICKS);
    rxSniffCmd->endTime    = (uint32_t)(rxEndTimeUs * US_TO_RAT_TICKS);

    /* Set correct trigger types */
    rxSniffCmd->endTrigger.triggerType   = TRIG_REL_START;
    rxSniffCmd->startTrigger.triggerType = TRIG_ABSTIME;
    rxSniffCmd->startTrigger.pastTrig    = 1;
}

/* Called for every received packet and command done */
void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    /* If we've received a new packet and it's available to read out */
    if (e & RF_EventRxEntryDone)
    {
        do
        {
            /* Toggle LED on RX */
            GPIO_toggle(CONFIG_GPIO_GLED);

            /* Get current unhandled data entry */
            currentDataEntry = RFQueue_getDataEntry();

            /* Handle the packet data, located at &currentDataEntry->data:
             * - Length is the first byte with the current configuration
             * - Data starts from the second byte */
            packetLength      = *(uint8_t*)(&currentDataEntry->data);
            packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

            /* This code block is added to avoid a compiler warning.
            * Normally, an application will reference these variables for
            * useful data. */
            dummy = packetLength + packetDataPointer[0];

        } while(RFQueue_nextEntry() == DATA_ENTRY_FINISHED);
    }
}
