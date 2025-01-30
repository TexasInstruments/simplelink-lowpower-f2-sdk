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
 *  ======== zigbee_add_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Attributes from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get devices from ZDL.js */
let devices = system.getScript("/ti/zstack/stack/zcl/ZDL.js").ZDL.devices;

/* Get Man Attributes script */
const acmaScript = system.getScript("/ti/zstack/app/add_config/acma_config/zigbee_acma_config");

/* Get Opt Attributes script */
const acoaScript = system.getScript("/ti/zstack/app/add_config/acoa_config/zigbee_acoa_config");

/* Get Man Commands Generated script */
const acmcgScript = system.getScript("/ti/zstack/app/add_config/acmcg_config/zigbee_acmcg_config");

/* Get Opt Commands Generated script */
const acocgScript = system.getScript("/ti/zstack/app/add_config/acocg_config/zigbee_acocg_config");

/* Get Man Commands Received script */
const acmcrScript = system.getScript("/ti/zstack/app/add_config/acmcr_config/zigbee_acmcr_config");

/* Get Opt Commands Received script */
const acocrScript = system.getScript("/ti/zstack/app/add_config/acocr_config/zigbee_acocr_config");

/* Get BDB Settings script */
const bdbScript = system.getScript("/ti/zstack/app/bdb_config/zigbee_bdb_config");

/* Get Add long descriptions */
const docs = system.getScript("/ti/zstack/app/add_config/zigbee_add_config_docs.js");

Common.addGlobalClusters(devices, clusters);

/* Configurables for the Zigbee Application Additional Clusters module */
const addModule = {
    displayName: "Additional Clusters",
    config: [],
};

/* Build clusterDict and clusterList */
let clusterDict = {};
let clusterList = [];
for (let i in clusters) {
  clusterDict[clusters[i]._id] = clusters[i];
  clusterList.push(clusters[i]._id);
}

/* Build Device Cluster Drop Downs */
let addDropDowns = [];
let addAlwaysHidden = [];
for (let i in devices) {
  let device = devices[i];
  let hidden = true;
  if (device._id == '0xFFFE') { hidden = false; }

  let notTempServerClusterOptions = [];
  if (device.server && device.server.cluster) {
    for (let j in device.server.cluster) {
      let cluster = device.server.cluster[j];
      if (clusterDict[cluster._id]) {
        notTempServerClusterOptions.push(cluster._id);
      }
    }
  }
  let tempServerClusterOptions = [];
  for (let j in clusterList) {
    if (notTempServerClusterOptions.indexOf(clusterList[j]) == -1) {
      let cluster = clusterDict[clusterList[j]];
      tempServerClusterOptions.push({name: cluster._id, displayName: cluster._name})
    }
  }

  if (tempServerClusterOptions.length == 0) {
    addAlwaysHidden.push(device._id + "addServer");
    tempServerClusterOptions.push({name: "dummy", displayName: "dummy"});
  }
  addModule.config.push({
    name: device._id + "addServer",
    displayName: "Server Clusters",
    description: "Additional Zigbee Server Clusters for the " + device._name + " device.",
    longDescription: docs.addServerClustersLongDescription,
    default: [],
    options: tempServerClusterOptions,
    onChange: addOnServerClustersChange,
    hidden: hidden,
    minSelections: 0
  })
  addDropDowns.push(device._id + "addServer");

  let notTempClientClusterOptions = [];
  if (device.client && device.client.cluster) {
    for (let j in device.client.cluster) {
      let cluster = device.client.cluster[j];
      if (clusterDict[cluster._id]) {
        notTempClientClusterOptions.push(cluster._id);
      }
    }
  }
  let tempClientClusterOptions = [];
  for (let j in clusterList) {
    if (notTempClientClusterOptions.indexOf(clusterList[j]) == -1) {
      let cluster = clusterDict[clusterList[j]];
      tempClientClusterOptions.push({name: cluster._id, displayName: cluster._name})
    }
  }

  if (tempClientClusterOptions.length == 0) {
    addAlwaysHidden.push(device._id + "addClient");
    tempClientClusterOptions.push({name: "dummy", displayName: "dummy"});
  }
  addModule.config.push({
    name: device._id + "addClient",
    displayName: "Client Clusters",
    description: "Additional Zigbee Client Clusters for the " + device._name + " device.",
    longDescription: docs.addClientClustersLongDescription,
    default: [],
    options: tempClientClusterOptions,
    onChange: addOnClientClustersChange,
    hidden: hidden,
    minSelections: 0
  })
  addDropDowns.push(device._id + "addClient");
}

/* Add Attribute Groups */
addModule.config.push(acmaScript.acmaModule)
addModule.config.push(acoaScript.acoaModule)

/* Add Command Generated Groups */
addModule.config.push(acmcgScript.acmcgModule)
addModule.config.push(acocgScript.acocgModule)

/* Add Command Received Groups */
addModule.config.push(acmcrScript.acmcrModule)
addModule.config.push(acocrScript.acocrModule)

/* Function to handle changes in device configurable */
function addOnDeviceTypeChange(inst, ui)
{
  addOnServerClustersChange(inst, ui);
  addOnClientClustersChange(inst, ui);

  for (let i in addDropDowns) { ui[addDropDowns[i]].hidden = true; }

  ui[inst.zigbeeDeviceType + "addServer"].hidden = false
  ui[inst.zigbeeDeviceType + "addClient"].hidden = false

  for (let i in addAlwaysHidden) { ui[addAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the additional server clusters */
function addOnServerClustersChange(inst, ui)
{
  acmaScript.acmaOnServerClusterChange(inst, ui);
  acoaScript.acoaOnServerClusterChange(inst, ui);
  acmcgScript.acmcgOnServerClusterChange(inst, ui);
  acocgScript.acocgOnServerClusterChange(inst, ui);
  acmcrScript.acmcrOnServerClusterChange(inst, ui);
  acocrScript.acocrOnServerClusterChange(inst, ui);
  bdbScript.bdbOnChange(inst, ui);
}

/* Function to handle changes in the additional client clusters */
function addOnClientClustersChange(inst, ui)
{
  acmaScript.acmaOnClientClusterChange(inst, ui);
  acoaScript.acoaOnClientClusterChange(inst, ui);
  acmcgScript.acmcgOnClientClusterChange(inst, ui);
  acocgScript.acocgOnClientClusterChange(inst, ui);
  acmcrScript.acmcrOnClientClusterChange(inst, ui);
  acocrScript.acocrOnClientClusterChange(inst, ui);
  bdbScript.bdbOnChange(inst, ui);
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  addModule: addModule,
  addOnDeviceTypeChange: addOnDeviceTypeChange
};
