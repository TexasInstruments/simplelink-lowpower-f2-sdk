/*
 * Copyright (c) 2016-2018, Texas Instruments Incorporated
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

/* Kernel Header files */
#include <semaphore.h>
#include <stdio.h>

/* TI-RTOS Header files */
#include <ti/display/Display.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/gpio.h)
#include DeviceFamily_constructPath(inc/hw_ccfg.h)

/* Application specific Header files */
#include "menu.h"
#include "config.h"

#include <ti_drivers_config.h>
#include <ti_radio_config.h>

/***** Defines *****/

/* Task and event configuration */
#define MENU_TASK_STACKSIZE     2048

/* Expected values of the CCFG when CCFG_FORCE_VDDR_HH=0x1 */
#define CCFG_FORCED_VDDR_ALT_DCDC_VMIN  (0xC)
#define CCFG_FORCED_VDDR_VDDR_EXT_LOAD  (0x0)
#define CCFG_FORCED_VDDR_VDDS_BOD_LEVEL (0x1)

/***** Variable declarations *****/

/* TX Semaphore */
static sem_t buttonSemaphore;

/* Events used in the application */
typedef enum
{
    MenuEvent_None = 0,
    MenuEvent_Navigate = 1,
    MenuEvent_Select = 2,
    MenuEvent_AnyButtonPushed = MenuEvent_Navigate + MenuEvent_Select,
} MenuEvent;

/* Menu row indices */
typedef enum
{
    TitleRow = 0,
    TestModeRow,
    ModulationRow,
    FrequencyRow,
    PacketCountRow,
    IntervalRow,
    PayloadLengthRow,
#if (defined SUPPORT_HIGH_PA)
    HighPaRow,
#endif
    StartRow,
    NrOfMainMenuRows,
} MenuIndex;

/* String constants for different boards */
static const char* const button0Text = "BTN-1";
static const char* const button1Text = "BTN-2";

/* Convenience macros for printing on a vt100 terminal via UART */
#define vt100_print0(handle, row, col, text) \
    Display_printf(handle, 0, 0, "\x1b[%d;%df" text, row+1, col+1)

#define vt100_print1(handle, row, col, formatString, arg1) \
    Display_printf(handle, 0, 0, "\x1b[%d;%df" formatString, row+1, col+1, arg1)

#define vt100_print2(handle, row, col, formatString, arg1, arg2) \
    Display_printf(handle, 0, 0, "\x1b[%d;%df" formatString, row+1, col+1, arg1, arg2)

#define vt100_clear(handle) \
    Display_printf(handle, 0, 0, "\x1b[2J\x1b[H")

#define vt100_setCursorVisible(handle, visible) \
    Display_printf(handle, 0, 0, "\x1b[?25%c", ((visible) == true) ? 'h' : 'l')

/* Holds the configuration for the current test case */
static ApplicationConfig config =
{
#if ((defined CONFIG_CC26X2R1_LAUNCHXL) || (defined CONFIG_CC2652R1FRGZ) || \
     (defined CONFIG_LP_CC2652RSIP)     || (defined CONFIG_CC2652R1FSIP) || \
     (defined CONFIG_LP_CC2652PSIP)     || (defined CONFIG_CC2652P1FSIP) || \
     (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
     (defined CONFIG_LP_CC2651P3)       || (defined CONFIG_CC2651P3RGZ)  || \
     (defined CONFIG_LP_CC2651R3)       || (defined CONFIG_CC2651R3RGZ)  || \
     (defined CONFIG_LP_CC2651R3SIPA)   || (defined CONFIG_CC2651R3SIPA) || \
     (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
     (defined CONFIG_LP_CC2674R10_FPGA) || (defined CONFIG_CC2674R10RGZ) || \
     (defined CONFIG_LP_CC2653P10)      || (defined CONFIG_CC2653P10RGZ))
    RfSetup_2_4_Fsk_250,
#else
    RfSetup_Fsk,
#endif
    IntervalMode_Yes,
    TestMode_Rx,
    10,
    30,
    NULL,
    0,
#if (defined SUPPORT_HIGH_PA)
    HighPa_Dis
#endif
};

static Display_Handle lcdDisplay;
static Display_Handle uartDisplay;
bool ccfgForceVddrHH;
static MenuEvent eventFlag = MenuEvent_None;

/* Read ALT_DCDC_VMIN from CCFG's MODE_CONF_1 register */
static inline uint32_t CCFGRead_ALT_DCDC_VMIN(void) {
    return (( HWREG( CCFG_BASE + CCFG_O_MODE_CONF_1 ) &
            CCFG_MODE_CONF_1_ALT_DCDC_VMIN_M ) >>
            CCFG_MODE_CONF_1_ALT_DCDC_VMIN_S );
}

/* Read VDDR_EXT_LOAD from CCFG's MODE_CONF register */
static inline uint32_t CCFGRead_VDDR_EXT_LOAD(void) {
    return (( HWREG( CCFG_BASE + CCFG_O_MODE_CONF ) &
         CCFG_MODE_CONF_VDDR_EXT_LOAD_M ) >>
         CCFG_MODE_CONF_VDDR_EXT_LOAD_S );
}

/* Read VDDS_BOD_LEVEL from CCFG's MODE_CONF register */
static inline uint32_t CCFGRead_VDDS_BOD_LEVEL(void) {
    return (( HWREG( CCFG_BASE + CCFG_O_MODE_CONF ) &
            CCFG_MODE_CONF_VDDS_BOD_LEVEL_M ) >>
            CCFG_MODE_CONF_VDDS_BOD_LEVEL_S );
}

/* Read CCFG and determine if  CCFG_FORCE_VDDR_HH=0x1*/
static inline bool CCFGRead_FORCE_VDDR_HH(void)
{
    uint32_t altDcDcVMin  = CCFGRead_ALT_DCDC_VMIN();
    uint32_t vddrExtLoad  = CCFGRead_VDDR_EXT_LOAD();
    uint32_t vddsBodLevel = CCFGRead_VDDS_BOD_LEVEL();

    return((altDcDcVMin == CCFG_FORCED_VDDR_ALT_DCDC_VMIN)\
            && (vddrExtLoad == CCFG_FORCED_VDDR_VDDR_EXT_LOAD)\
            && (vddsBodLevel == CCFG_FORCED_VDDR_VDDS_BOD_LEVEL));
}

bool menu_isButtonPressed()
{
    return ((eventFlag & MenuEvent_AnyButtonPushed) != 0);
}

/*
Menu task function.

This task contains the main application logic. It prints the menu on both,
LCD and UART and starts the RX and TX test cases.
The setup code is generated from the .cfg file.
*/
void menu_init()
{
    int retc;

    config.frequencyTable = config_frequencyTable_Lut[config.rfSetup];

    /* Init displays */
    Display_Params params;
    Display_Params_init(&params);
    params.lineClearMode = DISPLAY_CLEAR_NONE;

#if !(defined(DeviceFamily_CC26X4) || defined(DeviceFamily_CC13X4))
    lcdDisplay = Display_open(Display_Type_LCD, &params);

#if !((defined CONFIG_LP_CC2652PSIP)  || (defined CONFIG_CC2652P1FSIP) || \
      (defined DeviceFamily_CC26X4)   || (defined DeviceFamily_CC13X4))
    if(lcdDisplay == NULL)
    {
        while(1);
    }
#endif

    Display_clear(lcdDisplay);
#endif
    uartDisplay = Display_open(Display_Type_UART, &params);
    if(uartDisplay == NULL)
    {
        while(1);
    }

    ccfgForceVddrHH = CCFGRead_FORCE_VDDR_HH();


    retc = sem_init(&buttonSemaphore, 0, 0);
    if (retc != 0) {
        while (1);
    }
}

void menu_runTask()
{
    uint8_t cursorRow = TestModeRow;
    uint8_t packetIndex = 0;
    uint8_t payloadIndex = 0;
    int retc;
    MenuEvent event;

    vt100_clear(uartDisplay);
    vt100_setCursorVisible(uartDisplay, false);

    /* Splash screen */
    Display_printf(lcdDisplay, 0, 0, "PER TEST");
    Display_printf(lcdDisplay, 1, 0, PER_VERSION);
    Display_printf(lcdDisplay, 3, 0, "Select:   %s", button0Text);
    Display_printf(lcdDisplay, 4, 0, "Navigate: %s", button1Text);
    Display_printf(lcdDisplay, 6, 0, "Push a button");
    Display_printf(lcdDisplay, 7, 0, "to proceed...");


    vt100_print0(uartDisplay, 0, 0, "PER TEST");
    vt100_print0(uartDisplay, 1, 0, PER_VERSION);
    vt100_print1(uartDisplay, 3, 0, "Select:   %s", button0Text);
    vt100_print1(uartDisplay, 4, 0, "Navigate: %s", button1Text);
    vt100_print0(uartDisplay, 6, 0, "Push a button");
    vt100_print0(uartDisplay, 7, 0, "to proceed...");

    /* Wait for a button press */
    retc = sem_wait(&buttonSemaphore);
    if (retc == -1) {
        while (1);
    }
    
    Display_clear(lcdDisplay);

    vt100_clear(uartDisplay);

    while(true)
    {
        /* Main Menu */
        Display_printf(lcdDisplay, 0, 0, "Main Menu");
        Display_printf(lcdDisplay, TestModeRow, 0,      " Test: %s", config_testmodeLabels[config.testMode]);
        Display_printf(lcdDisplay, ModulationRow, 0,    " Mode: %s", config_rfSetupLabels[config.rfSetup]);
        Display_printf(lcdDisplay, FrequencyRow, 0,     " Freq: %s", config.frequencyTable[config.frequency].label);
        Display_printf(lcdDisplay, PacketCountRow, 0,   " Pkts: %-5d", config.packetCount);
        if(config.testMode == TestMode_Rx)
        {
            Display_printf(lcdDisplay, IntervalRow, 0,      " Interval: -- ");
            Display_printf(lcdDisplay, PayloadLengthRow, 0, " Length: -- ");
#if (defined SUPPORT_HIGH_PA)
            Display_printf(lcdDisplay, HighPaRow, 0,        " HighPA: --      ");
#endif
        }
        else
        {
            Display_printf(lcdDisplay, IntervalRow, 0,      " Interval: %s", config_intervalLabels[config.intervalMode]);
            Display_printf(lcdDisplay, PayloadLengthRow, 0, " Length: %-3d", config.payloadLength);
#if (defined SUPPORT_HIGH_PA)
            Display_printf(lcdDisplay, HighPaRow, 0,        " HighPA: %s", config_highPaLabels[config.highPa]);
#endif
        }
        Display_printf(lcdDisplay, StartRow, 0, " Start...");
        vt100_print0(uartDisplay, 0, 0, "Main Menu");
        vt100_print1(uartDisplay, TestModeRow, 0,      " Test: %s", config_testmodeLabels[config.testMode]);
        vt100_print1(uartDisplay, ModulationRow, 0,    " Mode: %s", config_rfSetupLabels[config.rfSetup]);
        vt100_print1(uartDisplay, FrequencyRow, 0,     " Freq: %s", config.frequencyTable[config.frequency].label);
        vt100_print1(uartDisplay, PacketCountRow, 0,   " Pkts: %-5d", config.packetCount);
        if(config.testMode == TestMode_Rx)
        {
            vt100_print0(uartDisplay, IntervalRow, 0,      " Interval: -- ");
            vt100_print0(uartDisplay, PayloadLengthRow, 0, " Length: -- ");
#if (defined SUPPORT_HIGH_PA)
            vt100_print0(uartDisplay, HighPaRow, 0,        " HighPA: --      ");
#endif
        }
        else
        {
            vt100_print1(uartDisplay, IntervalRow, 0,      " Interval: %s", config_intervalLabels[config.intervalMode]);
            vt100_print1(uartDisplay, PayloadLengthRow, 0, " Length: %-3d", config.payloadLength);
#if (defined SUPPORT_HIGH_PA)
            vt100_print1(uartDisplay, HighPaRow, 0,        " HighPA: %s", config_highPaLabels[config.highPa]);
#endif
        }

        vt100_print0(uartDisplay, StartRow, 0, " Start...");

        /* Print the selector */
        Display_printf(lcdDisplay, cursorRow, 0, ">");
        vt100_print0(uartDisplay, cursorRow, 0, ">" "\x1b[1A"); // Overlay selector and cursor

        /* Navigation is done event based. Events are set from button interrupts and sem is released */
        retc = sem_wait(&buttonSemaphore);
        if (retc == -1) {
            while (1);
        }

        event = eventFlag;
        eventFlag = MenuEvent_None;
        if (event & MenuEvent_Navigate)
        {
            cursorRow++;
            if (cursorRow >= NrOfMainMenuRows)
            {
                cursorRow = TestModeRow;
            }
        }
        if (event & MenuEvent_Select)
        {
            switch(cursorRow)
            {
                case TestModeRow:
                    config.testMode = (TestMode)((config.testMode + 1) % NrOfTestModes);
                    break;

                case ModulationRow:
                    config.rfSetup = (RfSetup)((config.rfSetup + 1) % NrOfRfSetups);
                    config.frequencyTable = config_frequencyTable_Lut[config.rfSetup];
                    config.frequency = 0;
#if (defined SUPPORT_HIGH_PA)
                    config.highPa = HighPa_Dis;
#endif
#if (defined SUPPORT_BLE)
                    if ((config.rfSetup == RfSetup_Ble) || (config.rfSetup == RfSetup_Ble5))
                    {
                        // Fixed payload length of 30
                        payloadIndex = 0;
                        config.payloadLength = config_payloadLengthTable[payloadIndex];
                    }
#endif
                    break;

                case FrequencyRow:
                    // Custom settings only. Use the freq from ti_radio_config.c
                    if (config.rfSetup != RfSetup_Custom)
                    {
                        config.frequency = (config.frequency + 1);
                        if(config.frequencyTable[config.frequency].frequency == 0xFFFF)
                        {
                            config.frequency = 0;
                        }
                    }

                    break;

            case PacketCountRow:
                packetIndex = (packetIndex + 1) % config_NrOfPacketCounts;
                config.packetCount = config_packetCountTable[packetIndex];
                break;

            case IntervalRow:
                config.intervalMode = (IntervalMode)((config.intervalMode + 1) % NrOfIntervalModes);
                if(config.intervalMode == IntervalMode_Yes)
                {
                    /* Fixed payload length of 30 */
                    payloadIndex = 0;
                    config.payloadLength = config_payloadLengthTable[payloadIndex];
                }
                break;

            case PayloadLengthRow:
#if !defined(SUPPORT_BLE)
                if(config.intervalMode == IntervalMode_No)
#else
                if((config.intervalMode == IntervalMode_No)\
                    && (config.rfSetup != RfSetup_Ble)\
                    && (config.rfSetup != RfSetup_Ble5))
#endif

                {
                    payloadIndex = (payloadIndex + 1) % config_NrOfPayloadLengths;
                }
                config.payloadLength = config_payloadLengthTable[payloadIndex];
                break;

#if (defined SUPPORT_HIGH_PA)
            case HighPaRow:
#if ((defined LAUNCHXL_CC1352P1)  || (defined LAUNCHXL_CC1352P_4) || \
     (defined LP_CC1352P7_1)      || (defined LP_CC1352P7_4))
                if((config.rfSetup == RfSetup_Ble) || (config.rfSetup == RfSetup_Ble5))
                {
                    // CC1352P1 and CC1352P-4 do not support high PA BLE Tx
                    config.highPa = HighPa_Dis;
                }
                else
                {
                    config.highPa = (HighPa)((config.highPa + 1) % NrOfHighPa);
                }
                break;
#elif ((defined LAUNCHXL_CC1352P_2) || (defined CONFIG_LP_CC2652PSIP))
                if(!(config.rfSetup == RfSetup_Ble) && !(config.rfSetup == RfSetup_Ble5))
                {
                    // CC1352P-2 does not support high PA Sub-1 Tx
                    config.highPa = HighPa_Dis;
                }
                else
                {
                    config.highPa = (HighPa)((config.highPa + 1) % NrOfHighPa);
                }
                break;
#else
                config.highPa = (HighPa)((config.highPa + 1) % NrOfHighPa);
                break;
#endif
#endif

            case StartRow:

                if (config.testMode == TestMode_Rx)
                {
                    /* Prepare RX display */
                    Display_clear(lcdDisplay);
                    Display_printf(lcdDisplay, 0, 0, "Receiving...");
                    Display_printf(lcdDisplay, 1, 0, "%s %s",
                            config_rfSetupLabels[config.rfSetup],
                            config.frequencyTable[config.frequency].label);
                    Display_printf(lcdDisplay, 2, 0, "Pkts ok   :%-5d", 0);
                    Display_printf(lcdDisplay, 3, 0, "RSSI [dBm]:n/a");
                    Display_printf(lcdDisplay, 4, 0, "TP[bps]:n/a");
                    Display_printf(lcdDisplay, 5, 0, "PER  [%%]  :n/a");
                    Display_printf(lcdDisplay, 7, 0, "Push a button");
                    Display_printf(lcdDisplay, 8, 0, "to abort.");

                    vt100_clear(uartDisplay);
                    vt100_print0(uartDisplay, 0, 0, "Receiving...");
                    vt100_print2(uartDisplay, 1, 0, "%s %s",
                            config_rfSetupLabels[config.rfSetup],
                            config.frequencyTable[config.frequency].label);
                    vt100_print1(uartDisplay, 2, 0, "Pkts ok   : %-5d", 0);
                    vt100_print0(uartDisplay, 3, 0, "RSSI [dBm]: n/a");
                    vt100_print0(uartDisplay, 4, 0, "TP[bps]: n/a");
                    vt100_print0(uartDisplay, 5, 0, "PER  [%%]  : n/a");
                    vt100_print0(uartDisplay, 7, 0, "Push a button");
                    vt100_print0(uartDisplay, 8, 0, "to abort.");

                    /* Run the test. */
                    TestResult result = rx_runRxTest(&config);
                    if (result == TestResult_Finished)
                    {
                        Display_printf(lcdDisplay, 7, 0, "...finished. ");
                        vt100_print0(uartDisplay, 7, 0, "...finished. ");

                        Display_printf(lcdDisplay, 8, 0, "Push a button...");
                        vt100_print0(uartDisplay, 8, 0, "Push a button...");

                        /* wait for button press */
                        retc = sem_wait(&buttonSemaphore);
                        if (retc == -1) {
                            while (1);
                        }
                    }
                }
                else
                {
                    /* Prepare TX display */
                    Display_clear(lcdDisplay);
                    Display_printf(lcdDisplay, 0, 0, "Sending...");
                    Display_printf(lcdDisplay, 1, 0, "%s %s",
                            config_rfSetupLabels[config.rfSetup],
                            config.frequencyTable[config.frequency].label);
                    Display_printf(lcdDisplay, 3, 0, "Pkts sent: %-5d", 0);


                    vt100_clear(uartDisplay);
                    vt100_print0(uartDisplay, 0, 0, "Sending...");
                    vt100_print2(uartDisplay, 1, 0, "%s %s",
                            config_rfSetupLabels[config.rfSetup],
                            config.frequencyTable[config.frequency].label);
                    vt100_print1(uartDisplay, 3, 0, "Pkts sent: %-5d", 0);

                    /* Run the test. */
                    TestResult result = tx_runTxTest(&config);
                    if (result == TestResult_Aborted)
                    {
                        Display_printf(lcdDisplay, 8, 0, "...aborted.");
                        vt100_print0(uartDisplay, 8, 0, "...aborted.");
                    }
                    else if (result == TestResult_Finished)
                    {
                        Display_printf(lcdDisplay, 8, 0, "...finished.");
                        vt100_print0(uartDisplay, 8, 0, "...finished.");
                    }
                    Display_printf(lcdDisplay, 9, 0, "Push a button...");
                    vt100_print0(uartDisplay, 9, 0, "Push a button...");

                    /* wait for button press */
                    retc = sem_wait(&buttonSemaphore);
                    if (retc == -1) {
                        while (1);
                    }
                }
                Display_clear(lcdDisplay);
                vt100_clear(uartDisplay);
                break;
            }
        }
    }
}

/*
Callback for button interrupts.

This function is supposed to be called asynchronously from within an interrupt
handler and signals a button press event to the application logic.
*/
void menu_notifyButtonPressed(Button button)
{
    if (button == Button_Navigate)
    {
        eventFlag = MenuEvent_Navigate;
    }
    else
    {
        eventFlag = MenuEvent_Select;
    }

    sem_post(&buttonSemaphore);
}

/*
Updates the screen content during an ongoing receive.

Call this function from any other task to refresh the menu with
updated parameters.
*/
void menu_updateRxScreen(rx_metrics *metrics)
{
    char buffer[6];
    int16_t perIntPart;
    int16_t perFracPart;

    /* Convert float to string buffer */
    if ((metrics->packetsReceived <= config.packetCount) &&
        (metrics->packetsReceived <= metrics->packetsExpected))
    {
        /* Avoid a 0.0/0.0 (NaN) or a x/0.0 (+Inf) condition */
        float per = 0.0f;
        if(metrics->packetsExpected > 0)
        {
            per = ((float)(metrics->packetsMissed)/(float)(metrics->packetsExpected))*100.0f;
        }

        perIntPart = (int16_t)per;
        perFracPart = (int16_t)((per - (float)perIntPart) *100.0);
        snprintf(buffer, 6, "%d.%d", perIntPart, perFracPart);
    }
    else
    {
        sprintf(buffer, "n/a  ");
    }

    Display_printf(lcdDisplay, 2, 11, "%-5d", metrics->packetsReceived);
    Display_printf(lcdDisplay, 3, 11, "%-5i", metrics->rssi);
    Display_printf(lcdDisplay, 4, 8, "%-7d", metrics->throughput);
    Display_printf(lcdDisplay, 5, 11, "%s", &buffer);

    vt100_print1(uartDisplay, 2, 0, "Pkts ok   : %-5d", metrics->packetsReceived);
    vt100_print1(uartDisplay, 3, 0, "RSSI [dBm]: %-5i", metrics->rssi);
    vt100_print1(uartDisplay, 4, 0, "TP[bps]: %-7d", metrics->throughput);
    vt100_print1(uartDisplay, 5, 0, "PER  [%%]  : %s", &buffer);
}

/*
Updates the screen content during an ongoing transmission.

Call this function from any other task to refresh the menu with
updated parameters.
 */
void menu_updateTxScreen(uint32_t packetsSent)
{
    Display_printf(lcdDisplay, 3, 11, "%-5d", packetsSent);
    vt100_print1(uartDisplay, 3, 11, "%-5d", packetsSent);
}

/*
Updates the screen content during an ongoing transmission. This includes
TX metrics like Transmission Power (dBm), Data Rate (bps) and Packet Interval
(ms)

Call this function from any other task to refresh the menu with
updated parameters.
 */
void menu_updateTxMetricScreen(tx_metrics *metrics)
{
    Display_printf(lcdDisplay, 4, 0, "Pwr[dBm]: %-4d", metrics->transmitPowerDbm);
    Display_printf(lcdDisplay, 5, 0, "DR[bps]: %-7d", metrics->dataRateBps);
    Display_printf(lcdDisplay, 6, 0, "Interval: %-3d ms", metrics->packetIntervalMs);

    vt100_print1(uartDisplay, 4, 0,  "Pwr[dBm]: %-4d", metrics->transmitPowerDbm);
    vt100_print1(uartDisplay, 5, 0,  "DR[bps]: %-7d", metrics->dataRateBps);
    vt100_print1(uartDisplay, 6, 0,  "Interval: %-3d ms", metrics->packetIntervalMs);
}
