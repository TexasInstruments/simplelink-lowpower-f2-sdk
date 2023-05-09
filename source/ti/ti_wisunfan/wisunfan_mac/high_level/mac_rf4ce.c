/******************************************************************************

 @file  mac_rf4ce.c

 @brief This module implements RF4CE specific MAC functionality.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2005-2023, Texas Instruments Incorporated

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_154stack_features.h"
#endif

/* hal */

/* osal */
#include "osal.h"

/* mac */
#include "mac_api.h"
#include "mac_low_level.h"
#include "mac_spec.h"
#include "mac_main.h"
#include "mac_timer.h"
#include "mac_mgmt.h"
#include "mac_pib.h"
#include "mac_data.h"
#include "mac_scan.h"
#include "mac_device.h"
#include "mac_coord.h"
#include "mac_assert.h"
#include "mac_pwr.h"

/* mac_rf4ce */
#include "mac_rf4ce.h"


#ifndef RCN_FEATURE_TARGET
#define RCN_FEATURE_TARGET TRUE
#endif


static void macNullAction(macEvent_t *pEvent);
extern void MAC_SetRandomSeedCB(macRNGFcn_t pCBFcn);
static void macSeedCB(uint8 *seed);
uint8 macStrongRandomSeed[MAC_RANDOM_SEED_LEN];

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* Mgmt action set 2 */
const macAction_t macMgmtRf4ceAction2[] =
{
  macApiPwrOnReq,
  macNullAction,
  macNullAction,
  macNoAction, // Beacon message event - we don't want any warnings on receiving a beacon message
  macNullAction
};

/* Coordinator action set 1 */
static const macAction_t macRf4ceCoordAction1[] =
{
  macApiStartReq,
  macStartContinue
};

/* Coordinator action set 2 */
static const macAction_t macRf4ceCoordAction2[] =
{
  macNullAction,
  macNullAction,
  macRxBeaconReq,
  macNullAction,
  macStartComplete,
  macNullAction
};

/* Device action set 1 */
static const macAction_t macRf4ceDeviceAction1[] =
{
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macStartFrameResponseTimer,
  macStartResponseTimer
};

/* Device action set 2 */
static const macAction_t macRf4ceDeviceAction2[] =
{
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction,
  macNullAction
};


/**************************************************************************************************
 * @fn          MAC_InitRf4ce
 *
 * @brief       This function initializes MAC for RF4CE.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MAC_InitRf4ce(void)
{
  macActionSet[MAC_MGMT_ACTION_1] = macMgmtAction1;

  macMain.featureMask = 0;

  /* Must be called before macLowLevelInit(). */
  MAC_SetRandomSeedCB(macSeedCB);  
  macLowLevelInit();
}


/**************************************************************************************************
 * @fn          MAC_InitRf4ceDevice
 *
 * @brief       This function intializes the RF4CE MAC as a device.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MAC_InitRf4ceDevice(void)
{
  macActionSet[MAC_MGMT_ACTION_2]   = macMgmtRf4ceAction2;

  macActionSet[MAC_DEVICE_ACTION_1] = macRf4ceDeviceAction1;
  macActionSet[MAC_DEVICE_ACTION_2] = macRf4ceDeviceAction2;

  macReset[MAC_FEAT_DEVICE] = &macDeviceReset;

  macMain.featureMask |= MAC_FEAT_DEVICE_MASK;

  if (RCN_FEATURE_TARGET)
  {
    macActionSet[MAC_SCAN_ACTION] = macScanAction;
    MAC_InitRf4ceCoord();
  }
}


/**************************************************************************************************
 * @fn          MAC_InitRf4ceCoord
 *
 * @brief       This function initializes the RF4CE MAC as a coordinator.  It loads coordinator
 *              action functions into the state machine action table.  It also configures the
 *              state machine to use state tables required for operation as a device.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MAC_InitRf4ceCoord(void)
{
  macActionSet[MAC_COORD_ACTION_1] = macRf4ceCoordAction1;
  macActionSet[MAC_COORD_ACTION_2] = macRf4ceCoordAction2;

  macStateTbl[MAC_STARTING_ST]     = macStartingSt;

  macReset[MAC_FEAT_COORD] = &macCoordReset;

  macMain.featureMask |= MAC_FEAT_COORD_MASK;
}


/**************************************************************************************************
 * @fn          macNullAction
 *
 * @brief       This function asserts when an unexpected event occurs.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void macNullAction(macEvent_t *pEvent)
{
  (void) pEvent; // unused argument
  
  MAC_ASSERT_FORCED();
}


/**************************************************************************************************
 * @fn          macSeedCB
 *
 * @brief       This function copies the random seed to macStrongRandomSeed
 *
 * input parameters
 *
 * @param       seed - Random seed generated by the macLowLevelInit().
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void macSeedCB(uint8 *seed)
{
  (void)MAP_osal_memcpy(macStrongRandomSeed, seed, MAC_RANDOM_SEED_LEN);
}

