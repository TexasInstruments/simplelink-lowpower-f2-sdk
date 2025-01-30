/******************************************************************************

 @file  mac_duty_cycle.h

 @brief This file contains the functions and data structures for handling duty
        cycling in the MAC.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2025, Texas Instruments Incorporated
 All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License"). You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio
 frequency transceiver, which is integrated into your product. Other than for
 the foregoing purpose, you may not use, reproduce, copy, prepare derivative
 works of, modify, distribute, perform, display or sell this Software and/or
 its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

 ******************************************************************************
 
 
 *****************************************************************************/
#ifndef MAC_DUTY_CYCLE_H
#define MAC_DUTY_CYCLE_H


#ifdef MAC_DUTY_CYCLE_CHECKING


/* These values are used in the pre-built libraries so changing
 * it will lead to unexpected behavior
 */
// Number of buckets used for duty cycle time tracking
#define DUTY_CYCLE_BUCKETS 13
// Amount of time in duty cycle measurement period (in ms)
#define DUTY_CYCLE_MEAS_PERIOD 3600000

#include <stdbool.h>

typedef struct {
    uint32_t timeUsed; //usec
} DC_NODE_ENTRY_s;

// Globals

/* Fake PIB Variables */
extern uint32_t dcLimited;
extern uint32_t dcCritical;
extern uint32_t dcRegulated;
extern uint32_t dcTimeUsed;
extern uint32_t dcPtr;
extern DC_NODE_ENTRY_s dcBucket[DUTY_CYCLE_BUCKETS];
extern uint8_t dcStatus;
extern bool dcEnabled;

void macDutyCycleInit(void);

void macDutyCycleUpdateTransmission(uint32_t curLen);

bool macDutyCycleCheck(uint32_t curLen);

uint32_t macDutyCycleByteToUSec(uint32_t curLen);

#endif //MAC_DUTY_CYCLE_CHECKING

#endif //MAC_DUTY_CYCLE_H
