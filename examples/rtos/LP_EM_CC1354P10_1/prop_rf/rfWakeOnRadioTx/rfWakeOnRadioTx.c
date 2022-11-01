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
#include <stdlib.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>

/* TI-RTOS Header files */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>
#include <ti/display/Display.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/cpu.h)

/* Board Header files */
#include "ti_drivers_config.h"

/* RF settings */
#include <ti_radio_config.h>

/***** Defines *****/
/* Wake-on-Radio configuration */
#define WOR_WAKEUPS_PER_SECOND 2

/* TX number of random payload bytes */
#define PAYLOAD_LENGTH 30

/* WOR Example configuration defines */
#define WOR_PREAMBLE_TIME_RAT_TICKS(x) \
    ((uint32_t)(4000000*(1.0f/(x))))

/* TX task stack size and priority */
#define TX_TASK_STACK_SIZE 1024
#define TX_TASK_PRIORITY   2


/***** Prototypes *****/
static void initializeTxAdvCmdFromTxCmd(rfc_CMD_PROP_TX_ADV_t* RF_cmdPropTxAdv, rfc_CMD_PROP_TX_t* RF_cmdPropTx);

/* TX packet payload (length +1 to fit length byte) and sequence number */
static uint8_t packet[PAYLOAD_LENGTH +1];
static uint16_t seqNumber;

/* RF driver objects and handles */
static RF_Object rfObject;
static RF_Handle rfHandle;

/* TX Semaphore */
static sem_t txSemaphore;

/* Advanced TX command for sending long preamble */
static rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv;


/***** Function definitions *****/
/* GPIO interrupt Callback function for CONFIG_GPIO_BTN1. */
void buttonCallbackFunction(uint_least8_t index) {

    /* Simple debounce logic, only toggle if the button is still pushed (low) */
    CPUdelay((uint32_t)((48000000/3)*0.050f));
    if (!GPIO_read(index)) {
        /* Post TX semaphore to TX task */
        sem_post(&txSemaphore);
    }
}

/* TX task function. Executed in Task context when the scheduler starts. */
void *mainThread(void *arg0)
{
    int retc;

    Display_init();

    GPIO_setConfig(CONFIG_GPIO_GLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BTN1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Install Button callback */
    GPIO_setCallback(CONFIG_GPIO_BTN1, buttonCallbackFunction);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BTN1);

    retc = sem_init(&txSemaphore, 0, 0);
    if (retc != 0) {
        while (1);
    }

    /* Initialize the display and try to open both UART and LCD types of display. */
    Display_Params params;
    Display_Params_init(&params);
    params.lineClearMode = DISPLAY_CLEAR_BOTH;
    Display_Handle uartDisplayHandle = Display_open(Display_Type_UART, &params);
    Display_Handle lcdDisplayHandle = Display_open(Display_Type_LCD, &params);

    /* Print initial display content on both UART and any LCD present */
    Display_printf(uartDisplayHandle, 0, 0, "Wake-on-Radio TX");
    Display_printf(uartDisplayHandle, 0, 0, "Pkts sent: %u", seqNumber);
    Display_printf(lcdDisplayHandle, 0, 0, "Wake-on-Radio TX");
    Display_printf(lcdDisplayHandle, 1, 0, "Pkts sent: %u", seqNumber);

    /* Initialize the radio */
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    /* Initialize TX_ADV command from TX command */
    initializeTxAdvCmdFromTxCmd(&RF_cmdPropTxAdv, &RF_cmdPropTx);

    /* Set application specific fields */
    RF_cmdPropTxAdv.pktLen = PAYLOAD_LENGTH +1; /* +1 for length byte */
    RF_cmdPropTxAdv.pPkt = packet;
    RF_cmdPropTxAdv.preTrigger.triggerType = TRIG_REL_START;
    RF_cmdPropTxAdv.preTime = WOR_PREAMBLE_TIME_RAT_TICKS(WOR_WAKEUPS_PER_SECOND);

    /* Request access to the radio */
#if defined(DeviceFamily_CC26X0R2)
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
#else
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
#endif// DeviceFamily_CC26X0R2

    /* Set the frequency */
    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);

    /* Enter main TX loop */
    while(1)
    {
        int retc;

        /* Wait for a button press */
        retc = sem_wait(&txSemaphore);
        if (retc == -1) {
            while (1);
        }

        /* Create packet with incrementing sequence number and random payload */
        packet[0] = PAYLOAD_LENGTH;
        packet[1] = (uint8_t)(seqNumber >> 8);
        packet[2] = (uint8_t)(seqNumber++);
        uint8_t i;
        for (i = 3; i < PAYLOAD_LENGTH +1; i++)
        {
            packet[i] = rand();
        }

        /* Send packet */
        RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTxAdv, RF_PriorityNormal, NULL, 0);

        /* Update display */
        Display_printf(uartDisplayHandle, 0, 0, "Pkts sent: %u", seqNumber);
        Display_printf(lcdDisplayHandle, 1, 0, "Pkts sent: %u", seqNumber);
        /* Toggle LED */
        GPIO_toggle(CONFIG_GPIO_GLED);
    }
}

/* Copy the basic RX configuration from CMD_PROP_RX to CMD_PROP_RX_SNIFF command. */
static void initializeTxAdvCmdFromTxCmd(rfc_CMD_PROP_TX_ADV_t* RF_cmdPropTxAdv, rfc_CMD_PROP_TX_t* RF_cmdPropTx)
{
    #define RADIO_OP_HEADER_SIZE 14

    /* Copy general radio operation header from TX commmand to TX_ADV */
    memcpy(RF_cmdPropTxAdv, RF_cmdPropTx, RADIO_OP_HEADER_SIZE);

    /* Set command to CMD_PROP_TX_ADV */
    RF_cmdPropTxAdv->commandNo = CMD_PROP_TX_ADV;

    /* Copy over relevant parameters */
    RF_cmdPropTxAdv->pktConf.bFsOff = RF_cmdPropTx->pktConf.bFsOff;
    RF_cmdPropTxAdv->pktConf.bUseCrc = RF_cmdPropTx->pktConf.bUseCrc;
    RF_cmdPropTxAdv->syncWord = RF_cmdPropTx->syncWord;
}
