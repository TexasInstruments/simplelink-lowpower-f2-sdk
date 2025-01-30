/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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

/*
 *  ======== zstack_common.js ========
 */

"use strict";

let global_clusters = system.getScript("/ti/zstack/stack/zcl/ZDL.js").ZDL.globals;

const TI154Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

/* Min poll period (ms) */
const POLL_PERIOD_MIN = 100;

/* Max poll period (ms) */
const POLL_PERIOD_MAX = 0xFFFFFFFF;

/* PAN ID length (bytes) */
const PAN_ID_LEN = 2;

/* Extended PAN ID length (bytes) */
const EPID_LEN = 8;

/* Network Key length (bytes) */
const NWK_KEY_LEN = 16;

/* GPD Type length (bytes) */
const GPD_TYPE_LEN = 1;

/* GPD ID length (bytes) */
const GPD_ID_LEN = 4;

/* GPDF Security Key length (bytes) */
const GPDF_SEC_KEY_LEN = 16;

/* Helper function for range validation (making sure a configurable is
 * between a minimum and maximum value */
function validateRange(inst, validation, configVal, configName, configDisplay,
    min, max)
{
    if(min != null && configVal < min)
    {
        validation.logError(
            configDisplay + " must be greater than or equal to " + min,
            inst, configName
        );
    }
    else if(max != null && configVal > max)
    {
        validation.logError(
            configDisplay + " must be less than or equal to " + max,
            inst, configName
        );
    }
}

/* Function that converts the channel array from the RF channel configurables
 * to a bitmask in hex string format */
function chanArrToBitmask(chanArr)
{
    let i;
    let bitmask = 0;
    for(i = 0; i < chanArr.length; i++)
    {
        const channel = chanArr[i];

        // Shift the channel bit depending on what channel is selected
        const bit = 0x1 << (channel);

        // Add channel bit to the bitmask
        bitmask |= bit;
    }
    return("0x" + bitmask.toString(16).padStart(8, "0"));
}

// Dictionary mapping a device name to default LaunchPad; used to discover the
// appropriate RF settings when a device is being used without a LaunchPad
const deviceToBoard = {
  CC1352R: "CC1352R1_LAUNCHXL",
  CC1352P: "CC1352P1_LAUNCHXL",
  CC2652R7: "LP_CC2652R7",
  CC2652R1FSIP: "LP_CC2652RSIP",
  CC2652P1FSIP: "LP_CC2652PSIP",
  CC2651R3SIPA: "LP_CC2651R3SIPA",
  CC2652R1: "CC26X2R1_LAUNCHXL",
  CC2652RB: "LP_CC2652RB",
  CC1311R3: "LP_CC1311R3",
  CC1311P3: "LP_CC1311P3",
  CC2651R3: "LP_CC2651R3",
  CC2651P3: "LP_CC2651P3",
  CC1314R10: "LP_EM_CC1314R10",
  CC1354R10: "LP_CC1354R10_RGZ",
  CC2674R10: "LP_CC2674R10_FPGA",
  CC2653P10: "LP_CC2653P10",
  CC1354P10: "LP_EM_CC1354P10_1"
};

const commonWarning = "You need to also modify the linker command file according to these guidelines: https://dev.ti.com/tirex/explore/content/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/docs/zigbee/html/cc13xx_cc26xx/software-on-cc13x4-26x4.html#migrate-your-cc13x2-or-cc26x2-project";

const commonMigrations = {
  // Common Linkers
  cc13x2_cc26x2: {
    CC1352R1F3RGZ: {},
    CC1352R1_LAUNCHXL: {},
    CC1352P_2_LAUNCHXL: {},
    CC1352P1F3RGZ: {},
    CC1352P_4_LAUNCHXL: {},
    CC1352P1F3RGZ: {},
    CC2652R1FRGZ: {},
    CC26X2R1_LAUNCHXL: {},
    CC2652P1FSIP: {},
    LP_CC2652PSIP: {},
    LP_CC2652RB: {},
    CC2652RB1FRGZ: {},
    CC2652R1FSIP: {},
    LP_CC2652RSIP: {},
  },
  cc13x1_cc26x1: {
    CC2651P3RGZ: {},
    LP_CC2651P3: {},
    CC2651R3RGZ: {},
    LP_CC2651R3: {}
  },
  cc13x4_cc26x4: {
    CC1352P7RGZ: {},
    LP_CC1352P7_4: {},
    CC2652R7RGZ: {},
    LP_CC2652R7: {}
  },
  cc13x2x7_cc26x2x7: {
    LP_EM_CC1354P10_1: {},
    CC2674R10RGZ: {},
    LP_CC2674R10_RGZ: {},
    LP_EM_CC1354P10_6: {},
    CC2674P10RGZ: {},
    LP_CC2674P10_RGZ: {},
    LP_CC1354P10_1_RGZ: {warn: commonWarning},
    LP_CC1354R10_RGZ: {warn: commonWarning}
  }
};

const supportedMigrations = {

  // Boards

  // cc13x2_cc26x2
  CC1352R1: commonMigrations["cc13x2_cc26x2"],
  CC1352P_2_LAUNCHXL: commonMigrations["cc13x2_cc26x2"],
  CC1352P_4_LAUNCHXL: commonMigrations["cc13x2_cc26x2"],
  CC1352P1F3RGZ: commonMigrations["cc13x2_cc26x2"],
  /* Represents 26X2R1 board and device */
  "CC26.2R1": commonMigrations["cc13x2_cc26x2"],
  /* Represents PSIP board and device */
  "CC2652P.*SIP": commonMigrations["cc13x2_cc26x2"],
  CC2652R7: commonMigrations["cc13x2_cc26x2"],
  LP_CC2652RSIP: commonMigrations["cc13x2_cc26x2"],
  CC2652RB: commonMigrations["cc13x2_cc26x2"],
  "CC2652R.*SIP": commonMigrations["cc13x2_cc26x2"],

  // cc13x1_cc26x1
  CC2651P3: commonMigrations["cc13x1_cc26x1"],
  CC2651R3: commonMigrations["cc13x1_cc26x1"],

  // cc13x4_cc26x4
  CC2652R7: commonMigrations["cc13x4_cc26x4"],
  LP_CC1352P7_4: commonMigrations["cc13x4_cc26x4"],

  // cc13x2x7_cc26x2x7
  LP_EM_CC1354P10_1: commonMigrations["cc13x2x7_cc26x2x7"],
  LP_EM_CC1354P10_6: commonMigrations["cc13x2x7_cc26x2x7"]
};

/*
* ======== isMigrationValid ========
* Determines whether a migration from one board/device to another board/device
* is supported by the EasyLink module.
*
* @returns One of the following Objects:
*    - {} <--- Empty object if migration is valid
*    - {warn: "Warning markdown text"} <--- Object with warn property
*                                           if migration is valid but
*                                           might require user action
*    - {disable: "Disable markdown text"} <--- Object with disable property
*                                              if migration is not valid
*/
function isMigrationValid(currentTarget, migrationTarget)
{
    let migRegex = null;

    const defaultDisableText = "Consider starting from an example in "
    + " <SDK_INSTALL_DIR>/examples/ that is closer to the desired migration "
    + "target";

    let migSupported = {disable: defaultDisableText};

    for(migRegex in supportedMigrations)
    {
        if(currentTarget.match(new RegExp(migRegex))
            && supportedMigrations[migRegex][migrationTarget])
        {
            migSupported = supportedMigrations[migRegex][migrationTarget];

            // If function exists then migration support is conditional
            if(_.isFunction(migSupported))
            {
                migSupported = migSupported(migrationTarget);
            }
            break;
        }
    }

    return(migSupported);
}

/*
* ======== migrate ========
* Perform stack specific changes to the SysConfig env POST migration
*
* @param currTarget - Board/device being migrated FROM
* @param migrationTarget - Board/device being migrated TO
* @param env - SysConfig environment providing access to all configurables
* @param projectName - Optional name of the project being migrated
*
* @returns boolean - true when migration is supported and succesful, false when
*                    migration is not supported and/or unsuccesful
*/
function migrate(currTarget, migrationTarget, env, projectName=null)
{
  const migrationInfo= isMigrationValid(currTarget, migrationTarget);
  let migrationValid = true;
  if(migrationInfo.disable)
  {
      migrationValid = false;
  }

  if(migrationValid)
  {
    // Currently no zstack specific sysconfig changes required POST migration.
  }

  return(migrationValid);
}

/*
* ======== getMigrationMarkdown ========
* Returns text in markdown format that customers can use to aid in migrating a
* project between device/boards. It is recommended to provide no more
* than 3 bullet points with up to 120 characters per line.
*
* @param currTarget - Board/device being migrated FROM
*
* @returns string - Markdown formatted string
*/
function getMigrationMarkdown(currTarget)
{
  // TODO: May need to add migration guidelines when other boards are supported
  const migrationText = ``

  return(migrationText);
}

/*!
 *  ======== getDeviceOrLaunchPadName ========
 *  Get the name of the board (or device)
 *
 *  @param convertToBoard - Boolean. When true, return the associated LaunchPad
 *                          name if a device is being used without a LaunchPad
 *
 *  @returns String - Name of the board with prefix /ti/boards and
 *                    suffix .syscfg.json stripped off.  If no board
 *                    was specified, the device name is returned.
 */
function getDeviceOrLaunchPadName(convertToBoard)
{
    let name = system.deviceData.deviceId;

    if(system.deviceData.board != null)
    {
        name = system.deviceData.board.source;

        /* Strip off everything up to and including the last '/' */
        name = name.replace(/.*\//, "");

        /* Strip off everything after and including the first '.' */
        name = name.replace(/\..*/, "");
    }

    // Check if this is a standalone device without a LaunchPad
    if(convertToBoard && !name.includes("LAUNCHXL"))
    {
        // Find the LaunchPad name in deviceToBoard dictionary
        let key = null;
        for(key in deviceToBoard)
        {
            if(name.includes(key))
            {
                name = deviceToBoard[key];
                break;
            }
        }
    }

    return(name);
}


/*
 * ======== getBoardPhySettings ========
 * Determines which rf_defaults script to use based on device or inst.rfDesign
 *
 * @param inst - Instance of this module
 *
 * @returns Obj - rf_defaults script from which to get phy settings in
 *                radioconfig format. If device is not supported, returns null
 */
function getBoardPhySettings(inst)
{
    let phySettings;

    if(inst !== null && system.deviceData.deviceId === "CC1352P1F3RGZ")
    {
        const rfDesign = inst.rfDesign;

        // Get the RF Design configurable
        if(rfDesign === "LAUNCHXL-CC1352P-4")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "CC1352P_4_LAUNCHXL_rf_defaults.js");
        }
        else if(rfDesign === "LAUNCHXL-CC1352P1")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "CC1352P1_LAUNCHXL_rf_defaults.js");
        }
        else if(rfDesign === "LAUNCHXL-CC1352P-2")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "CC1352P_2_LAUNCHXL_rf_defaults.js");
        }
    }
    else if(inst !== null && system.deviceData.deviceId === "CC1352P7RGZ")
    {
        const rfDesign = inst.rfDesign;

        // Get the RF Design configurable
        if(rfDesign === "LP_CC1352P7-4")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "LP_CC1352P7_4_rf_defaults.js");
        }
    }
    else if(inst !== null && system.deviceData.deviceId === "CC2652P1FSIP")
    {
        const rfDesign = inst.rfDesign;

        // Get the RF Design configurable
        if(rfDesign === "LP_CC2652PSIP")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "LP_CC2652PSIP_rf_defaults.js");
        }
    }
    else
    {
        // Initialize with launchpad mapped from device
        phySettings = system.getScript("/ti/ti154stack/rf_config/"
            + TI154Common.getLaunchPadFromDevice() + "_rf_defaults.js");
    }

    return(phySettings);
}

function addGlobalClusters(devices, clusters)
{
  let all = global_clusters[0];
  let initiator = global_clusters[1];
  let target = global_clusters[2];

  /* add global clusters to devices */
  for (let device of devices)
  {
    // check if server and client clusters exist for current device.
    // if not, create each corresponding object
    if( device.server )
    {
      if ( device.server.cluster )
      {
        // we good
      }
      else
      {
        device.server["cluster"] = [];
      }
    }
    else
    {
      device["server"] = {};
      device.server["cluster"] = [];
    }

    if( device.client )
    {
      if ( device.client.cluster )
      {
        // we good
      }
      else
      {
        device.client["cluster"] = [];
      }
    }
    else
    {
      device["client"] = {};
      device.client["cluster"] = [];
    }

    // all devices need these clusters
    device.server.cluster = _.unionWith(device.server.cluster, all.server.cluster, _.isEqual);

    for (let cluster of device.server.cluster)
    {
      let result = _.find(clusters, function(o) { return o._id == cluster._id; });
      // device is target type
      if ( result != undefined && result._primary_transaction == "1" )
      {
        device.server.cluster = _.unionWith(device.server.cluster, target.server.cluster, _.isEqual);
      }
      // device is initiator type
      else if ( result != undefined && result._primary_transaction == "2" )
      {
        device.client.cluster = _.unionWith(device.client.cluster, initiator.client.cluster, _.isEqual);
      }
    }

    for (let cluster of device.client.cluster)
    {
      let result = _.find(clusters, function(o) { return o._id == cluster._id; });
      // device is target type
      if ( result != undefined && result._primary_transaction == "1" )
      {
        device.server.cluster = _.unionWith(device.server.cluster, target.server.cluster, _.isEqual);
      }
      // device is initiator type
      else if ( result != undefined && result._primary_transaction == "2" )
      {
        device.client.cluster = _.unionWith(device.client.cluster, initiator.client.cluster, _.isEqual);
      }
    }
  }
}

// Settings for ti/devices/CCFG module
const zstackCCFGSettings = {
    CC1352R1_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: true
    },
    CC1352P1_LAUNCHXL_CCFG_SETTINGS: {},
    CC1352P_2_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: true
    },
    CC1352P_4_LAUNCHXL_CCFG_SETTINGS: {},
    CC26X2R1_LAUNCHXL_CCFG_SETTINGS: {},
    LP_CC2652RB_CCFG_SETTINGS: {},
    LP_CC2651P3_CCFG_SETTINGS: {},
    LP_CC2674R10_FPGA_CCFG_SETTINGS: {},
    LP_CC2654R10_CCFG_SETTINGS: {},
    LP_CC1354R10_CCFG_SETTINGS: {}
};

const boardName = getDeviceOrLaunchPadName(true);
const ccfgSettings = zstackCCFGSettings[boardName + "_CCFG_SETTINGS"];

exports = {
    POLL_PERIOD_MIN: POLL_PERIOD_MIN,
    POLL_PERIOD_MAX: POLL_PERIOD_MAX,
    PAN_ID_LEN: PAN_ID_LEN,
    EPID_LEN: EPID_LEN,
    NWK_KEY_LEN: NWK_KEY_LEN,
    GPD_TYPE_LEN: GPD_TYPE_LEN,
    GPD_ID_LEN: GPD_ID_LEN,
    GPDF_SEC_KEY_LEN: GPDF_SEC_KEY_LEN,
    validateRange: validateRange,
    chanArrToBitmask: chanArrToBitmask,
    getDeviceOrLaunchPadName: getDeviceOrLaunchPadName,
    ccfgSettings: ccfgSettings,
    getBoardPhySettings: getBoardPhySettings,
    addGlobalClusters: addGlobalClusters,
    isMigrationValid: isMigrationValid,
    migrate: migrate,
    getMigrationMarkdown: getMigrationMarkdown
};
