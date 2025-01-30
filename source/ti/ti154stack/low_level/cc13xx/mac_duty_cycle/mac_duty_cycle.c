/******************************************************************************

 @file  mac_duty_cycle.c

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

#ifdef MAC_DUTY_CYCLE_CHECKING

/* Includes */
#include <stdbool.h>
#include <ti/drivers/dpl/ClockP.h>

#include "mac.h"
#include "mac_duty_cycle.h"
#include "mac_pib.h"
#include "mac_api.h"
#include "util_timer.h"
#include "mac_hl_patch.h"

#include "rom_jt_154.h"

/* Local Defines */

/* msec */
#define DUTY_CYCLE_BUFFER_LENGTH    (DUTY_CYCLE_MEAS_PERIOD/(DUTY_CYCLE_BUCKETS - 1))

#define DUTYCYCLE_USECPERBYTE_250K     32
#define DUTYCYCLE_USECPERBYTE_5K       1600
#define DUTYCYCLE_USECPERBYTE_50K      160
#define DUTYCYCLE_USECPERBYTE_150K     53
#define DUTYCYCLE_USECPERBYTE_200K     40
#define DUTYCYCLE_USECPERBYTE_100K     80

#define DUTYCYCLE_USECPERBYTE_CUSTOM   ((200/symbol_rate_custom) * 40)

/* 54 sec in msec */
#ifndef DUTY_CYCLE_THRESH_LIMITED
#define DUTY_CYCLE_THRESH_LIMITED 54000
#endif //DUTY_CYCLE_THRESH_LIMITED

/* 75 sec in msec */
#ifndef DUTY_CYCLE_THRESH_CRITICAL
#define DUTY_CYCLE_THRESH_CRITICAL 75000
#endif //DUTY_CYCLE_THRESH_CRITICAL

#ifndef DUTY_CYCLE_THRESH_REGULATED
/* 90 sec in msec */
#define DUTY_CYCLE_THRESH_REGULATED 90000
#endif //DUTY_CYCLE_THRESH_REGULATED


/* Local Variables */
static uint32_t num_packets;
static uint32_t dbgViolations;
static uint32_t addedLen;
static uint32_t phyMultiplier;
static uint32_t bucketLen;
static uint32_t numBuckets;

/* Fake PIB Variables */
uint32_t dcLimited;
uint32_t dcCritical;
uint32_t dcRegulated;
uint32_t dcTimeUsed;
uint32_t dcPtr;
DC_NODE_ENTRY_s dcBucket[DUTY_CYCLE_BUCKETS];
uint8_t dcStatus;
bool dcEnabled = false;

static ClockP_Handle dutyCycleClkHandle;


/* Local Function Definitions */
static void macDutyCycleTimerCB(uintptr_t a0);

static bool macDutyCycleNotify(uint32_t sum);

static void macDutyCycleBumpBuffer(void);


/* Local Functions */
/**************************************************************************************************
 * @fn          macDutyCycleNotify
 *
 * @brief       Check the mode for which the device is in and notify the app of this mode and manage
 *              timers
 *
 * @param       sum - amount of transmissions in usec
 *
 * @return      true if amount of transmissions is allowed, false if not
 **************************************************************************************************
 */
static bool macDutyCycleNotify(uint32_t sum)
{
    uint32_t beaconTime;
    uint32_t dcRegulatedNoBeacon;
    uint32_t dcCriticalNoBeacon;
    uint32_t dcLimitedNoBeacon;

    if (pMacPib->beaconOrder != MAC_BO_NON_BEACON && macPanCoordinator)
    {
        // Subtract beacon time from actual times
        beaconTime = (macSpecUsecsPerSymbol*macTimerRolloverValue[pMacPib->beaconOrder])/1000;
        dcRegulatedNoBeacon = (beaconTime > dcRegulated) ? 0 : (dcRegulated - beaconTime);
        dcCriticalNoBeacon = (beaconTime > dcCritical) ? 0 : (dcCritical - beaconTime);
        dcLimitedNoBeacon = (beaconTime > dcLimited) ? 0 : (dcLimited - beaconTime);
    }
    else
    {
        dcRegulatedNoBeacon = dcRegulated;
        dcCriticalNoBeacon = dcCritical;
        dcLimitedNoBeacon = dcLimited;
    }

    if (sum >= (1000*dcRegulatedNoBeacon))
    {
        /* Suspended Mode: TX not allowed and report suspended */
        // notify of REGULATED MODE
        if (!(dcStatus == dcModeRegulated))
        {
            dcStatus = dcModeRegulated;
            MAC_DutyCycleNotifyCB(dcStatus);
        }
        return false;
    }
    else if (sum >= (1000*dcCriticalNoBeacon))
    {
        /* Critical Mode: TX allowed and report critical */
        // notify of CRITICAL MODE
        if (!(dcStatus == dcModeCritical))
        {
            dcStatus = dcModeCritical;
            MAC_DutyCycleNotifyCB(dcStatus);
        }
        return true;
    }

    else if (sum >= (1000*dcLimitedNoBeacon))
    {
        /* Limited Mode: TX allowed and report limited */
        // notify of LIMITED MODE
        if (!(dcStatus == dcModeLimited))
        {
            dcStatus = dcModeLimited;
            MAC_DutyCycleNotifyCB(dcStatus);
        }
        return true;
    }
    else
    {
        /* Normal Mode: TX allowed and report normal */
        // notify of NORMAL MODE
        if (!(dcStatus == dcModeNormal))
        {
            dcStatus = dcModeNormal;
            MAC_DutyCycleNotifyCB(dcStatus);
        }

        return true;
    }
}

/**************************************************************************************************
 * @fn          macDutyCycleBumpBuffer
 *
 * @brief       Bumps the bucket buffer so that the circular buffer is looking at the
 *              correct location
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void macDutyCycleBumpBuffer(void)
{
    dcPtr = (dcPtr + 1) % DUTY_CYCLE_BUCKETS;
    dcTimeUsed -= (dcBucket[dcPtr].timeUsed);
    dcBucket[dcPtr].timeUsed = 0;
}

/**************************************************************************************************
 * @fn          macDutyCycleUpdatePhy
 *
 * @brief       Update the phyMultipler parameter based on the current PHY ID. Note that
 *              this is done every time macDutyCycleByteToUSec is called to avoid
 *              changing ROM code by modifying PIB-related functions. Otherwise, this
 *              would be added in the macSetDefaultsByPhyID function.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void macDutyCycleUpdatePhy(void)
{
#ifdef COMBO_MAC
    uint8 rfFreq = pMacPib->rfFreq;
    if (rfFreq == MAC_RF_FREQ_SUBG)
    {
        uint8_t phyRate = MAP_macCheckPhyRate(pMacPib->curPhyID);
        switch (phyRate)
        {
        case PHY_MODE_SLR_5K:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_5K;
            break;
        case PHY_MODE_FSK_50K:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_50K;
            break;
        case PHY_MODE_FSK_150K:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_150K;
            break;
        case PHY_MODE_FSK_200K:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_200K;
            break;
        case PHY_MODE_CUSTOM:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_CUSTOM;
            break;
        default:
            break;
        }
    }
    else
    {
        phyMultiplier = DUTYCYCLE_USECPERBYTE_250K;
    }
#else
#ifdef FREQ_2_4G
    phyMultiplier = DUTYCYCLE_USECPERBYTE_250K;
#else
    uint8_t phyRate = MAP_macCheckPhyRate(pMacPib->curPhyID);
    switch (phyRate)
    {
    case PHY_MODE_SLR_5K:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_5K;
        break;
    case PHY_MODE_FSK_50K:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_50K;
        break;
    case PHY_MODE_FSK_150K:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_150K;
        break;
    case PHY_MODE_FSK_200K:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_200K;
        break;
    case PHY_MODE_CUSTOM:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_CUSTOM;
        break;
    default:
        break;
    }
#endif
#endif
}

/* Functions */
/**************************************************************************************************
 * @fn          macDutyCycleInit
 *
 * @brief       Initialize low-level MAC Duty Cycling
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macDutyCycleInit(void)
{
    num_packets = 0;

    dcPtr = 0;
    dcTimeUsed = 0;
    dcStatus = dcModeNormal;

    //debug variables
    dbgViolations = 0;

    bucketLen = DUTY_CYCLE_BUFFER_LENGTH;
    numBuckets = DUTY_CYCLE_BUCKETS;

    uint8_t i;
    for (i = 0; i < numBuckets; i++)
    {
        dcBucket[i].timeUsed = 0;
    }

    dcLimited = DUTY_CYCLE_THRESH_LIMITED;
    dcCritical = DUTY_CYCLE_THRESH_CRITICAL;
    dcRegulated = DUTY_CYCLE_THRESH_REGULATED;

#ifdef COMBO_MAC
    uint8 rfFreq = pMacPib->rfFreq;
    if (rfFreq == MAC_RF_FREQ_SUBG)
    {
        addedLen = MAC_PHY_SHR_LEN_2FSK + MAC_FCS2_FIELD_LEN;
        bool fcsType;
        MAP_MAC_MlmeGetReq(MAC_FCS_TYPE, &fcsType);
        if (!fcsType)
        {
            addedLen += MAC_FCS2_FIELD_LEN;
        }
        uint8_t phyRate = MAP_macCheckPhyRate(pMacPib->curPhyID);
        switch (phyRate)
        {
        case PHY_MODE_SLR_5K:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_5K;
            break;
        case PHY_MODE_FSK_50K:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_50K;
            break;
        case PHY_MODE_FSK_150K:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_150K;
            break;
        case PHY_MODE_FSK_200K:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_200K;
            break;
        case PHY_MODE_CUSTOM:
            phyMultiplier = DUTYCYCLE_USECPERBYTE_CUSTOM;
            break;
        default:
            break;
        }
    }
    else
    {
        addedLen = MAC_PHY_SHR_LEN + MAC_PHY_PHR_IEEE_LEN;
        phyMultiplier = DUTYCYCLE_USECPERBYTE_250K;
    }
#else
#ifdef FREQ_2_4G
    addedLen = MAC_PHY_SHR_LEN + MAC_PHY_PHR_IEEE_LEN;
    phyMultiplier = DUTYCYCLE_USECPERBYTE_250K;
#else
    addedLen = MAC_PHY_SHR_LEN_2FSK + MAC_FCS2_FIELD_LEN;
    phyMultiplier = 0;
    bool fcsType;
    MAP_MAC_MlmeGetReq(MAC_FCS_TYPE, &fcsType);
    if (!fcsType)
    {
        addedLen += MAC_FCS2_FIELD_LEN;
    }
    uint8_t phyRate = MAP_macCheckPhyRate(pMacPib->curPhyID);
    switch (phyRate)
    {
    case PHY_MODE_SLR_5K:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_5K;
        break;
    case PHY_MODE_FSK_50K:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_50K;
        break;
    case PHY_MODE_FSK_150K:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_150K;
        break;
    case PHY_MODE_FSK_200K:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_200K;
        break;
    case PHY_MODE_CUSTOM:
        phyMultiplier = DUTYCYCLE_USECPERBYTE_CUSTOM;
        break;
    default:
        break;
    }
#endif
#endif

    if (dutyCycleClkHandle == NULL)
    {
        ClockP_Params params;
        ClockP_Params_init(&params);
        params.startFlag = FALSE;
        params.period = 0;
        dutyCycleClkHandle = ClockP_create((ClockP_Fxn) macDutyCycleTimerCB,
                                           1, &params);
        if (!ClockP_isActive(dutyCycleClkHandle))
        {
            uint32_t ticks = (1000 * bucketLen);
            ticks /= ClockP_getSystemTickPeriod();
            ClockP_setTimeout(dutyCycleClkHandle, ticks);
            ClockP_start(dutyCycleClkHandle);
        }
    }
}

/**************************************************************************************************
 * @fn          macDutyCycleUpdateTransmission
 *
 * @brief       Add packet information into duty cycle buffer and update
 *              number of packets.
 *
 * @param       curLen - current length of packet in usec
 *
 * @return      none
 **************************************************************************************************
 */
void macDutyCycleUpdateTransmission(uint32_t curLen)
{
    if (num_packets == UINT32_MAX)
    {
        num_packets = 0;
    }
    else
    {
        num_packets++;
    }

    dcTimeUsed += curLen;
    dcBucket[dcPtr].timeUsed += curLen;

    macDutyCycleNotify(dcTimeUsed);
}

/**************************************************************************************************
 * @fn          macDutyCycleCheck
 *
 * @brief       Check to see if there are any duty cycling conflicts
 *
 * @param       curLen - current length of packet in usec
 *
 * @return      True if packet is acceptable to be sent, false if packet will violate duty cycling
 **************************************************************************************************
 */
bool macDutyCycleCheck(uint32_t curLen)
{
    if (num_packets == 0)
    {
        return true;
    }

    if (macDutyCycleNotify(curLen + dcTimeUsed))
    {
        return true;
    }
    else
    {
        if (dbgViolations < UINT32_MAX)
        {
            dbgViolations ++;
        }
        return false;
    }

}

/**************************************************************************************************
 * @fn          macDutyCycleTimerCB
 *
 * @brief       Callback for Duty Cycle Timer
 *
 * @param       a0 - empty arg used in timer
 *
 * @return      none
 **************************************************************************************************
 */
static void macDutyCycleTimerCB(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    macDutyCycleBumpBuffer();
    macDutyCycleNotify(dcTimeUsed);

    uint32_t ticks = (1000 * bucketLen);
    ticks /= ClockP_getSystemTickPeriod();
    // start timer(BUFFER_LENGTH in msec)
    if (!ClockP_isActive(dutyCycleClkHandle))
    {
        ClockP_setTimeout(dutyCycleClkHandle, ticks);
        ClockP_start(dutyCycleClkHandle);
    }
}

/**************************************************************************************************
 * @fn          macDutyCycleByteToUSec
 *
 * @brief       Convert Bytes of packet to USec depending on phy and datarate
 *
 * @param       len - length of packet in bytes
 *
 * @return      True if packet is acceptable to be sent, false if packet will violate duty cycling
 **************************************************************************************************
 */
uint32_t macDutyCycleByteToUSec(uint32_t curLen)
{
    macDutyCycleUpdatePhy();
    curLen += addedLen;
    return curLen * phyMultiplier;
}

void MAC_DutyCycleNotifyCB(uint8_t mode)
{
    (void) mode;
    return;
}
#endif
