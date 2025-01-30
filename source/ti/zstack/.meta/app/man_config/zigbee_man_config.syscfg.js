/*
 * Copyright (c) 2019 Texas Instruments Incorporated - http://www.ti.com
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
 *
 */

/*
 *  ======== zigbee_man_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Attributes from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get devices from ZDL.js */
let devices = system.getScript("/ti/zstack/stack/zcl/ZDL.js").ZDL.devices;

/* Get Man Attributes script */
const mcmaScript = system.getScript("/ti/zstack/app/man_config/mcma_config/zigbee_mcma_config");

/* Get Opt Attributes script */
const mcoaScript = system.getScript("/ti/zstack/app/man_config/mcoa_config/zigbee_mcoa_config");

/* Get Man Commands Generated script */
const mcmcgScript = system.getScript("/ti/zstack/app/man_config/mcmcg_config/zigbee_mcmcg_config");

/* Get Opt Commands Generated script */
const mcocgScript = system.getScript("/ti/zstack/app/man_config/mcocg_config/zigbee_mcocg_config");

/* Get Man Commands Recieved script */
const mcmcrScript = system.getScript("/ti/zstack/app/man_config/mcmcr_config/zigbee_mcmcr_config");

/* Get Opt Commands Recieved script */
const mcocrScript = system.getScript("/ti/zstack/app/man_config/mcocr_config/zigbee_mcocr_config");

/* Get BDB Settings script */
const bdbScript = system.getScript("/ti/zstack/app/bdb_config/zigbee_bdb_config");

/* Get Man long descriptions */
const docs = system.getScript("/ti/zstack/app/man_config/zigbee_man_config_docs.js");

Common.addGlobalClusters(devices, clusters);

/* Configurables for the Zigbee Application Mandatory Clusters module */
const manModule = {
    displayName: "Mandatory Clusters",
    config: []
};

/* Build clusterDict */
let clusterDict = {};
for (let i in clusters) { clusterDict[clusters[i]._id] = clusters[i]; }

/* Build Device Cluster Drop Downs */
let manDropDowns = [];
let manAlwaysHidden = [];
for (let i in devices) {
  let device = devices[i];

  let tempServerClusterDefault = [];
  let tempServerClusterOptions = [];
  if (device.server && device.server.cluster) {
    for (let j in device.server.cluster) {
      let cluster = device.server.cluster[j];
      if (clusterDict[cluster._id] && cluster._mandatory == "true") {
        tempServerClusterDefault.push(cluster._id);
        tempServerClusterOptions.push({name: cluster._id, displayName: clusterDict[cluster._id]._name});
      }
    }
  }
  if (tempServerClusterOptions.length == 0) {
    manAlwaysHidden.push(device._id + "manServer");
    tempServerClusterOptions.push({name: "dummy", displayName: "dummy"});
  }
  manModule.config.push({
    name: device._id + "manServer",
    displayName: "Server Clusters",
    description: "Mandatory Zigbee Server Clusters for the " + device._name + " device.",
    longDescription: docs.manServerClustersLongDescription,
    default: tempServerClusterDefault,
    options: tempServerClusterOptions,
    onChange: manOnServerClustersChange,
    hidden: true,
    minSelections: 0
  })
  manDropDowns.push(device._id + "manServer");

  let tempClientClusterDefault = [];
  let tempClientClusterOptions = [];
  if (device.client && device.client.cluster) {
    for (let j in device.client.cluster) {
      let cluster = device.client.cluster[j];
      if (clusterDict[cluster._id] && cluster._mandatory == "true") {
        tempClientClusterDefault.push(cluster._id);
        tempClientClusterOptions.push({name: cluster._id, displayName: clusterDict[cluster._id]._name});
      }
    }
  }
  if (tempClientClusterOptions.length == 0) {
    manAlwaysHidden.push(device._id + "manClient");
    tempClientClusterOptions.push({name: "dummy", displayName: "dummy"});
  }
  manModule.config.push({
    name: device._id + "manClient",
    displayName: "Client Clusters",
    description: "Mandatory Zigbee Client Clusters for the " + device._name + " device.",
    longDescription: docs.manClientClustersLongDescription,
    default: tempClientClusterDefault,
    options: tempClientClusterOptions,
    onChange: manOnClientClustersChange,
    hidden: true,
    minSelections: 0
  })
  manDropDowns.push(device._id + "manClient");
}

/* Add Attribute Groups */
manModule.config.push(mcmaScript.mcmaModule);
manModule.config.push(mcoaScript.mcoaModule);

/* Add Commands Generated Groups */
manModule.config.push(mcmcgScript.mcmcgModule);
manModule.config.push(mcocgScript.mcocgModule);

/* Add Commands Generated Groups */
manModule.config.push(mcmcrScript.mcmcrModule);
manModule.config.push(mcocrScript.mcocrModule);

/* Function to handle changes in device configurable */
function manOnDeviceTypeChange(inst, ui)
{
  manOnServerClustersChange(inst, ui);
  manOnClientClustersChange(inst, ui);

  for (let i in manDropDowns) { ui[manDropDowns[i]].hidden = true; }

  ui[inst.zigbeeDeviceType + "manServer"].hidden = false;
  ui[inst.zigbeeDeviceType + "manClient"].hidden = false;

  for (let i in manAlwaysHidden) { ui[manAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the mandatory server clusters */
function manOnServerClustersChange(inst, ui)
{
  mcmaScript.mcmaOnServerClusterChange(inst, ui);
  mcoaScript.mcoaOnServerClusterChange(inst, ui);
  mcmcgScript.mcmcgOnServerClusterChange(inst, ui);
  mcocgScript.mcocgOnServerClusterChange(inst, ui);
  mcmcrScript.mcmcrOnServerClusterChange(inst, ui);
  mcocrScript.mcocrOnServerClusterChange(inst, ui);
  bdbScript.bdbOnChange(inst, ui);
}

/* Function to handle changes in the mandatory client clusters */
function manOnClientClustersChange(inst, ui)
{
  mcmaScript.mcmaOnClientClusterChange(inst, ui);
  mcoaScript.mcoaOnClientClusterChange(inst, ui);
  mcmcgScript.mcmcgOnClientClusterChange(inst, ui);
  mcocgScript.mcocgOnClientClusterChange(inst, ui);
  mcmcrScript.mcmcrOnClientClusterChange(inst, ui);
  mcocrScript.mcocrOnClientClusterChange(inst, ui);
  bdbScript.bdbOnChange(inst, ui);
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  manModule: manModule,
  manOnDeviceTypeChange: manOnDeviceTypeChange
};
