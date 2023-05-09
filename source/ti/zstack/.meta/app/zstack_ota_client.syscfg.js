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
 *  ======== zstack_app.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get devices from ZDL.js */
let devices = system.getScript("/ti/zstack/stack/zcl/ZDL.js").ZDL.devices;

/* Get clusters from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get Man Settings script */
const manScript = system.getScript("/ti/zstack/app/man_config/zigbee_man_config"); //Mandatory

/* Get Rec Settings script */
const recScript = system.getScript("/ti/zstack/app/rec_config/zigbee_rec_config"); //Recommended

/* Get Add Settings script */
const addScript = system.getScript("/ti/zstack/app/add_config/zigbee_add_config"); //Additional

/* Get BDB Settings script */
const bdbScript = system.getScript("/ti/zstack/app/bdb_config/zigbee_bdb_config"); //bdb reporting

/* Get Adv Settings script */
const advScript = system.getScript("/ti/zstack/app/adv_config/zigbee_adv_config"); //Advanced Settings

/* Get App long descriptions */
const docs = system.getScript("/ti/zstack/app/zstack_app_docs.js");

/* Object used to verify Mandatory Elements */
let optionLengths = {};

/* Object used to verify all OTA Elements are Added */
let ota_requiredAttrs = ["0x0001", "0x0002", "0x0003", "0x0004", "0x0005", "0x0007", "0x0009"];

/* Parse Device Name */
let boardID = "0x" + system.deviceData.deviceId.match(/\d{4}/);

Common.addGlobalClusters(devices, clusters);

/* Parse ZDL.js */
let noteDevices = new Set();
let zigbeeDeviceOptions = [];
for (let i in devices) {
  let device = devices[i];
  if (device.server && device.server.cluster) {
      optionLengths[device._id + "manServer"] = 0;
      for (let j in device.server.cluster) {
          let cluster = device.server.cluster[j];
          if (cluster._mandatory == "true") {
            optionLengths[device._id + "manServer"] += 1;
          }
      }
  }
  if (device.client && device.client.cluster) {
      for (let j in device.client.cluster) {
          optionLengths[device._id + "manClient"] = 0;
          let cluster = device.client.cluster[j];
          if (cluster._mandatory == "true") {
            optionLengths[device._id + "manClient"] += 1;
          }
      }
  }
  if (device._note) {
      noteDevices.add(device._id);
  }
  zigbeeDeviceOptions.push({name: device._id, displayName: device._name});
}

/* Parse ZCL.js */
for (let i in clusters) {
    let cluster = clusters[i];
    if (cluster.server) {
        if (cluster.server.attribute) {
            optionLengths[cluster._id + "maServer"] = 0;
            for (let j in cluster.server.attribute) {
                let attribute = cluster.server.attribute[j];
                if (attribute._required == 'true') {
                    optionLengths[cluster._id + "maServer"] += 1;
                }
            }
        }
        if (cluster.server.command) {
            optionLengths[cluster._id + "mcrServer"] = 0;
            optionLengths[cluster._id + "mcgClient"] = 0;
            for (let j in cluster.server.command) {
                let command = cluster.server.command[j];
                if (command._required == 'true') {
                    optionLengths[cluster._id + "mcrServer"] += 1;
                    optionLengths[cluster._id + "mcgClient"] += 1;
                }
            }
        }
    }
    if (cluster.client) {
        if (cluster.client.attribute) {
            optionLengths[cluster._id + "maClient"] = 0;
            for (let j in cluster.client.attribute) {
                let attribute = cluster.client.attribute[j];
                if (attribute._required == 'true') {
                    optionLengths[cluster._id + "maClient"] += 1;
                }
            }
        }
        if (cluster.client.command) {
            optionLengths[cluster._id + "mcrClient"] = 0;
            optionLengths[cluster._id + "mcgServer"] = 0;
            for (let j in cluster.client.command) {
                let command = cluster.client.command[j];
                if (command._required == 'true') {
                    optionLengths[cluster._id + "mcrClient"] += 1;
                    optionLengths[cluster._id + "mcgServer"] += 1;
                }
            }
        }
    }
}
const networkModule = {
    config: [
        {
            name: "deviceType",
            displayName: "Device Type",
            description: "Hidden configurable for passing in device type",
            default: "",
            hidden: false,
        }
    ],
    validate: null
};
/* Application submodule for zstack module */
let appModule = {
    config: [
        {
            name: "zigbeeApplicationName",
            displayName: "Application Name",
            default: "otaClient",
            hidden: true
        },
        {
            name: "zigbeeDeviceType",
            displayName: "Zigbee Device Type",
            description: "Select the desired Zigbee device from the drop down menu.",
            longDescription: docs.zigbeeDeviceTypeLongDescription,
            default: "0xFFFE",
            options: zigbeeDeviceOptions,
            hidden: true
        },
        {
            name: "otaClientEnabled",
            displayName: "OTA Client Enabled",
            description: "Enable OTA Client Functionality",
            default: false,
            onChange: onOtaClientEnabledChange
        },
        {
            name: "otaCurrentFileVersion",
            displayName: "Current File Version",
            default: "0x00000001",
            hidden: false
        },
        {
            name: "otaManufacturerID",
            displayName: "Manufacturer ID",
            default: "0xBEBE",
            hidden: false
        },
        {
            name: "otaTypeID",
            displayName: "Type ID",
            default: boardID,
            readOnly: true
        },
        manScript.manModule,
        recScript.recModule,
        addScript.addModule,
        bdbScript.bdbModule,
        advScript.advModule
    ],
    validate: validate,
    templates: {
        "/ti/zstack/templates/zstack_config.h.xdt":
                            "/ti/zstack/templates/zstack_config.h.xdt",
        "/ti/zstack/templates/zcl_config.h.xdt":
                            "/ti/zstack/templates/zcl_config.h.xdt",
        "/ti/zstack/templates/zcl_config.c.xdt":
                            "/ti/zstack/templates/zcl_config.c.xdt"
    }
};

function verifyBDB(inst, validation, clusterID, attributeID) {
    if (inst["server" + clusterID + attributeID + "reportableChange"]) {
        /* Reportable Change */
        if(inst["server" + clusterID + attributeID + "reportableChange"] < 0x0)
        {
            validation.logError(
                'Reportable Change Value must not be negative.', inst, "server" + clusterID + attributeID + "reportableChange"
            );
        }
        else if(inst["server" + clusterID + attributeID + "reportableChange"] >= Math.pow(0x10, inst.bdbReportingMaxAnalogAttrSize*2))
        {
            validation.logError(
                'Reportable Change Value must not exceed ' + inst.bdbReportingMaxAnalogAttrSize.toString() + ' bytes.', inst, "server" + clusterID + attributeID + "reportableChange"
            );
        }

        /* Maximum Report Interval */
        if(inst["server" + clusterID + attributeID + "maxReportInterval"] < 0x0000)
        {
            validation.logError(
                'Maximum Report Interval Value must not be negative.', inst, "server" + clusterID + attributeID + "maxReportInterval"
            );
        }
        else if(inst["server" + clusterID + attributeID + "maxReportInterval"] > 0xffff)
        {
            validation.logError(
                'Maximum Report Interval Value must not exceed 0xFFFF.', inst, "server" + clusterID + attributeID + "maxReportInterval"
            );
        }

        /* Minimum Report Interval */
        if(inst["server" + clusterID + attributeID + "minReportInterval"] < 0x0000)
        {
            validation.logError(
                'Minimum Report Interval Value must not be negative.', inst, "server" + clusterID + attributeID + "minReportInterval"
            );
        }
        else if(inst["server" + clusterID + attributeID + "minReportInterval"] > 0xffff)
        {
            validation.logError(
                'Minimum Report Interval Value must not exceed 0xFFFF.', inst, "server" + clusterID + attributeID + "minReportInterval"
            );
        }
    }
}

/* Validation function for the application advanced settings submodule */
function validate(inst, validation)
{
    /* Verify Name */
    let name = inst.zigbeeApplicationName;
    let pattern = /[a-z][a-z\s0-9]*/i;
    let result = name.match(pattern);
    if(result != name)
    {
        validation.logError(
            'Application Name must be alphanumeric and start with a letter', inst, 'zigbeeApplicationName'
        );
    }

    /* Verify Endpoint */
    if(inst.endpoint < 0x01 || inst.endpoint > 0xFE)
    {
        validation.logError(
            'Endpoint must be a value between 0x01 and 0xFE.', inst, 'endpoint'
        );
    }
    else if(inst.endpoint > 0xf0)
    {
        validation.logWarning(
            'Endpoints between 0xF1 and 0xFE must be approved by the Zigbee Alliance.', inst, 'endpoint'
        );
    }

    /* Verify Device Version */
    if(inst.deviceVersion < 0x0)
    {
        validation.logError(
            'Device Version must not be negative.', inst, 'deviceVersion'
        );
    }
    else if(inst.deviceVersion > 0xf)
    {
        validation.logError(
            'Device Version must not exceed 0xF.', inst, 'deviceVersion'
        );
    }

    /* Verify Flags */
    if(inst.flags < 0x0000)
    {
        validation.logError(
            'Flags must not be negative.', inst, 'flags'
        );
    }
    else if(inst.deviceVersion > 0xffff)
    {
        validation.logError(
            'Flags must not exceed 0xFFFF.', inst, 'flags'
        );
    }

    /* Check Device Version */
    if(inst["otaCurrentFileVersion"] != 0x00000001)
    {
        validation.logInfo(
            "Remember to also change the post-build steps to reflect changes to the version\n", inst, "otaCurrentFileVersion"
        );
    }

    /* Check Device Version */
    if(inst["otaManufacturerID"] != 0xBEBE)
    {
        validation.logInfo(
            "Remember to also change the predefined symbols to reflect changes to the ID\n", inst, "otaManufacturerID"
        );
    }

    /* Check if OTA is enabled and OTA cluster is enabled*/
    if(inst["otaClientEnabled"])
    {
        validation.logInfo(
            "Please refer to the documentation for additional steps to enable OTA Client functionality\n", inst, "otaClientEnabled"
        );
        /* Check if OTA cluster is enabled */
        if(!inst["0x0007addClient"].includes("0x0019"))
        {
            validation.logError(
                "OTAUpgrade cluster is required for OTA Client functionality\n", inst, "0x0007addClient"
            );
        }
    }

    /* BDB - Mandatory Server Clusters */
    let clusters = inst[inst.zigbeeDeviceType + "manServer"];
    for (let i in clusters) {

        /* Mandatory Attributes */
        let attributes = inst[clusters[i] + "mcmaServer"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }

        /* Optional Attributes */
        attributes = inst[clusters[i] + "mcoaServer"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
    }

    /* BDB - Recommended Optional Server Clusters */
    clusters = inst[inst.zigbeeDeviceType + "recServer"];
    for (let i in clusters) {
        let attributes = inst[clusters[i] + "rcmaServer"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
        attributes = inst[clusters[i] + "rcoaServer"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
    }

    /* BDB - Additional Server Clusters */
    clusters = inst[inst.zigbeeDeviceType + "addServer"];
    for (let i in clusters) {
        let attributes = inst[clusters[i] + "acmaServer"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
        attributes = inst[clusters[i] + "acoaServer"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
    }

    /* BDB - Mandatory Client Clusters */
    clusters = inst[inst.zigbeeDeviceType + "manClient"];
    for (let i in clusters) {

        /* Mandatory Attributes */
        let attributes = inst[clusters[i] + "mcmaClient"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }

        /* Optional Attributes */
        attributes = inst[clusters[i] + "mcoaClient"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
    }

    /* BDB - Recommended Optional Client Clusters */
    clusters = inst[inst.zigbeeDeviceType + "recClient"];
    for (let i in clusters) {
        let attributes = inst[clusters[i] + "rcmaClient"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
        attributes = inst[clusters[i] + "rcoaClient"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
    }

    /* BDB - Additional Client Clusters */
    clusters = inst[inst.zigbeeDeviceType + "addClient"];
    for (let i in clusters) {
        let attributes = inst[clusters[i] + "acmaClient"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);
        }
        attributes = inst[clusters[i] + "acoaClient"];
        for (let j in attributes) {
            verifyBDB(inst, validation, clusters[i], attributes[j]);

        }
        if ( !ota_requiredAttrs.every(k => attributes.includes(k)) )
        {
            validation.logWarning(
                "Changing these options may result in undefined OTA Behavior", inst, "0x0019acoaClient"
            );
        }
    }

    /* Check Mandatory Server Clusters */
    clusters = inst[inst.zigbeeDeviceType + "manServer"];
    if (clusters.length < optionLengths[inst.zigbeeDeviceType + "manServer"]) {
        validation.logWarning(
            "Mandatory Server Clusters must be supported for Zigbee Certification.", inst, inst.zigbeeDeviceType + "manServer"
        );
    }

    /* Check Mandatory Client Clusters */
    clusters = inst[inst.zigbeeDeviceType + "manClient"];
    if (clusters.length < optionLengths[inst.zigbeeDeviceType + "manClient"]) {
        validation.logWarning(
            "Mandatory Client Clusters must be supported for Zigbee Certification.", inst, inst.zigbeeDeviceType + "manClient"
        );
    }

    /* Check 1 Rec. Opt. Cluster Edge Case */
    if (noteDevices.has(inst.zigbeeDeviceType)) {
        clusters = inst[inst.zigbeeDeviceType + "recClient"];
        if (clusters.length == 0) {
            validation.logWarning(
                "At least One Recommended Optional Cluster must be supported for Zigbee Certification.", inst, inst.zigbeeDeviceType + "recClient"
            );
        }
    }

    /* Check Mandatory Server Cluster Elements */
    clusters = inst[inst.zigbeeDeviceType + "manServer"];
    for (let i in clusters) {
        /* Mandatory Attributes */
        let attributes = inst[clusters[i] + "mcmaServer"];
        if (attributes.length < optionLengths[clusters[i] + "maServer"]) {
            validation.logWarning(
                "Mandatory Attributes must be supported for Zigbee Certification.", inst, clusters[i] + "mcmaServer"
            );
        }
        /* Mandatory Commands Received */
        let commandsRec = inst[clusters[i] + "mcmcrServer"];
        if (commandsRec.length < optionLengths[clusters[i] + "mcrServer"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "mcmcrServer"
            );
        }
        /* Mandatory Commands Generated */
        let commandsGen = inst[clusters[i] + "mcmcgServer"];
        if (commandsGen.length < optionLengths[clusters[i] + "mcgServer"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "mcmcgServer"
            );
        }
    }

    /* Check Mandatory Client Cluster Elements */
    clusters = inst[inst.zigbeeDeviceType + "manClient"];
    for (let i in clusters) {
        /* Mandatory Attributes */
        let attributes = inst[clusters[i] + "mcmaClient"];
        if (attributes.length < optionLengths[clusters[i] + "maClient"]) {
            validation.logWarning(
                "Mandatory Attributes must be supported for Zigbee Certification.", inst, clusters[i] + "mcmaClient"
            );
        }
        /* Mandatory Commands Received */
        let commandsRec = inst[clusters[i] + "mcmcrClient"];
        if (commandsRec.length < optionLengths[clusters[i] + "mcrClient"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "mcmcrClient"
            );
        }
        /* Mandatory Commands Generated */
        let commandsGen = inst[clusters[i] + "mcmcgClient"];
        if (commandsGen.length < optionLengths[clusters[i] + "mcgClient"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "mcmcgClient"
            );
        }
    }

    /* Check Recommended Server Cluster Elements */
    clusters = inst[inst.zigbeeDeviceType + "recServer"];
    for (let i in clusters) {
        /* Mandatory Attributes */
        let attributes = inst[clusters[i] + "rcmaServer"];
        if (attributes.length < optionLengths[clusters[i] + "maServer"]) {
            validation.logWarning(
                "Mandatory Attributes must be supported for Zigbee Certification.", inst, clusters[i] + "rcmaServer"
            );
        }
        /* Mandatory Commands Received */
        let commandsRec = inst[clusters[i] + "rcmcrServer"];
        if (commandsRec.length < optionLengths[clusters[i] + "mcrServer"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "rcmcrServer"
            );
        }
        /* Mandatory Commands Generated */
        let commandsGen = inst[clusters[i] + "rcmcgServer"];
        if (commandsGen.length < optionLengths[clusters[i] + "mcgServer"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "rcmcgServer"
            );
        }
    }

    /* Check Recommended Client Cluster Elements */
    clusters = inst[inst.zigbeeDeviceType + "recClient"];
    for (let i in clusters) {
        /* Mandatory Attributes */
        let attributes = inst[clusters[i] + "rcmaClient"];
        if (attributes.length < optionLengths[clusters[i] + "maClient"]) {
            validation.logWarning(
                "Mandatory Attributes must be supported for Zigbee Certification.", inst, clusters[i] + "rcmaClient"
            );
        }
        /* Mandatory Commands Received */
        let commandsRec = inst[clusters[i] + "rcmcrClient"];
        if (commandsRec.length < optionLengths[clusters[i] + "mcrClient"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "rcmcrClient"
            );
        }
        /* Mandatory Commands Generated */
        let commandsGen = inst[clusters[i] + "rcmcgClient"];
        if (commandsGen.length < optionLengths[clusters[i] + "mcgClient"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "rcmcgClient"
            );
        }
    }

    /* Check Additional Server Cluster Elements */
    clusters = inst[inst.zigbeeDeviceType + "addServer"];
    for (let i in clusters) {
        /* Mandatory Attributes */
        let attributes = inst[clusters[i] + "acmaServer"];
        if (attributes.length < optionLengths[clusters[i] + "maServer"]) {
            validation.logWarning(
                "Mandatory Attributes must be supported for Zigbee Certification.", inst, clusters[i] + "acmaServer"
            );
        }
        /* Mandatory Commands Received */
        let commandsRec = inst[clusters[i] + "acmcrServer"];
        if (commandsRec.length < optionLengths[clusters[i] + "mcrServer"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "acmcrServer"
            );
        }
        /* Mandatory Commands Generated */
        let commandsGen = inst[clusters[i] + "acmcgServer"];
        if (commandsGen.length < optionLengths[clusters[i] + "mcgServer"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "acmcgServer"
            );
        }
    }

    /* Check Additional Client Cluster Elements */
    clusters = inst[inst.zigbeeDeviceType + "addClient"];
    for (let i in clusters) {
        /* Mandatory Attributes */
        let attributes = inst[clusters[i] + "acmaClient"];
        if (attributes.length < optionLengths[clusters[i] + "maClient"]) {
            validation.logWarning(
                "Mandatory Attributes must be supported for Zigbee Certification.", inst, clusters[i] + "acmaClient"
            );
        }
        /* Mandatory Commands Received */
        let commandsRec = inst[clusters[i] + "acmcrClient"];
        if (commandsRec.length < optionLengths[clusters[i] + "mcrClient"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "acmcrClient"
            );
        }
        /* Mandatory Commands Generated */
        let commandsGen = inst[clusters[i] + "acmcgClient"];
        if (commandsGen.length < optionLengths[clusters[i] + "mcgClient"]) {
            validation.logWarning(
                "Mandatory Commands Received must be supported for Zigbee Certification.", inst, clusters[i] + "acmcgClient"
            );
        }
    }
}

/* Function to handle changes in onOtaClientEnabledChange configurable */
function onOtaClientEnabledChange(inst, ui)
{
  if (inst.zigbeeDeviceType == "0xFFFE")
  {
      // OTA client device type hard-coded as 'Combined Interface' device
      inst.zigbeeDeviceType = "0x0007";
      // add OTA Client cluster to additional client clusters
      inst["0x0007addClient"] = ["0x0019"];
      // add Basic Server and Basic Client
      inst["0x0007recClient"] = ["0x0000"];
      inst["0x0007manServer"] = ["0x0000", "0x0003"];
      // add required OTA attributes
      inst["0x0019acoaClient"] = ota_requiredAttrs;

      inst.endpoint = 0x0E;
  }
  else
  {
      inst.zigbeeDeviceType = "0xFFFE";
      inst["0x0007addClient"] = [];
      inst["0x0007recClient"] = [];
      inst["0x0007addServer"] = [];
      inst["0x0019acoaClient"] = [];
      inst.endpoint = 0x0E;
  }

  manScript.manOnDeviceTypeChange(inst, ui);
  recScript.recOnDeviceTypeChange(inst, ui);
  addScript.addOnDeviceTypeChange(inst, ui);
  bdbScript.bdbOnChange(inst, ui);
}

exports = appModule;
