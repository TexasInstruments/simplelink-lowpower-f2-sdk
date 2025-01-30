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
 *  ======== zigbee_rec_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Attributes from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get devices from ZDL.js */
let devices = system.getScript("/ti/zstack/stack/zcl/ZDL.js").ZDL.devices;

/* Get Man Attributes script */
const rcmaScript = system.getScript("/ti/zstack/app/rec_config/rcma_config/zigbee_rcma_config");

/* Get Opt Attributes script */
const rcoaScript = system.getScript("/ti/zstack/app/rec_config/rcoa_config/zigbee_rcoa_config");

/* Get Man Commands Generated script */
const rcmcgScript = system.getScript("/ti/zstack/app/rec_config/rcmcg_config/zigbee_rcmcg_config");

/* Get Opt Commands Generated script */
const rcocgScript = system.getScript("/ti/zstack/app/rec_config/rcocg_config/zigbee_rcocg_config");

/* Get Man Commands Received script */
const rcmcrScript = system.getScript("/ti/zstack/app/rec_config/rcmcr_config/zigbee_rcmcr_config");

/* Get Opt Commands Received script */
const rcocrScript = system.getScript("/ti/zstack/app/rec_config/rcocr_config/zigbee_rcocr_config");

/* Get BDB Settings script */
const bdbScript = system.getScript("/ti/zstack/app/bdb_config/zigbee_bdb_config");

/* Get Rec long descriptions */
const docs = system.getScript("/ti/zstack/app/rec_config/zigbee_rec_config_docs.js");

Common.addGlobalClusters(devices, clusters);

/* Configurables for the Zigbee Application Recommended Optional Clusters module */
const recModule = {
    displayName: "Recommended Optional Clusters",
    config: []
};

/* Build clusterDict */
let clusterDict = {};
for (let i in clusters) { clusterDict[clusters[i]._id] = clusters[i]; }

/* Build Device Cluster Drop Downs */
let recDropDowns = [];
let recAlwaysHidden = [];
for (let i=0; i<devices.length; i++) {
  let device = devices[i];
  let tempServerClusterOptions = [];
  if (device.server && device.server.cluster) {
    for (let j in device.server.cluster) {
      let cluster = device.server.cluster[j];
      if (clusterDict[cluster._id] && cluster._mandatory == "false") {
        tempServerClusterOptions.push({name: cluster._id, displayName: clusterDict[cluster._id]._name});
      }
    }
  }

  if (tempServerClusterOptions.length == 0) {
    recAlwaysHidden.push(device._id + "recServer");
    tempServerClusterOptions.push({name: "dummy", displayName: "dummy"});
  }
  recModule.config.push({
    name: device._id + "recServer",
    displayName: "Server Clusters",
    description: "Recommended Optional Zigbee Server Clusters for the " + device._name + " device.",
    longDescription: docs.recServerClustersLongDescription,
    default: [],
    options: tempServerClusterOptions,
    onChange: recOnServerClustersChange,
    hidden: true,
    minSelections: 0
  })
  recDropDowns.push(device._id + "recServer");

  let tempClientClusterOptions = [];
  if (device.client && device.client.cluster) {
    for (let j in device.client.cluster) {
      let cluster = device.client.cluster[j];
      if (clusterDict[cluster._id] && cluster._mandatory == "false") {
        tempClientClusterOptions.push({name: cluster._id, displayName: clusterDict[cluster._id]._name});
      }
    }
  }

  if (tempClientClusterOptions.length == 0) {
    recAlwaysHidden.push(device._id + "recClient");
    tempClientClusterOptions.push({name: "dummy", displayName: "dummy"});
  }
  recModule.config.push({
    name: device._id + "recClient",
    displayName: "Client Clusters",
    description: "Recommended Optional Zigbee Client Clusters for the " + device._name + " device.",
    longDescription: docs.recClientClustersLongDescription,
    default: [],
    options: tempClientClusterOptions,
    onChange: recOnClientClustersChange,
    hidden: true,
    minSelections: 0
  })
  recDropDowns.push(device._id + "recClient");
}

/* Add Attribute Groups */
recModule.config.push(rcmaScript.rcmaModule);
recModule.config.push(rcoaScript.rcoaModule);

/* Add Command Generated Groups */
recModule.config.push(rcmcgScript.rcmcgModule);
recModule.config.push(rcocgScript.rcocgModule);

/* Add Command Received Groups */
recModule.config.push(rcmcrScript.rcmcrModule);
recModule.config.push(rcocrScript.rcocrModule);

/* Function to handle changes in device configurable */
function recOnDeviceTypeChange(inst, ui)
{
  recOnServerClustersChange(inst, ui);
  recOnClientClustersChange(inst, ui);

  for (let i in recDropDowns) { ui[recDropDowns[i]].hidden = true; }

  ui[inst.zigbeeDeviceType + "recServer"].hidden = false;
  ui[inst.zigbeeDeviceType + "recClient"].hidden = false;

  for (let i in recAlwaysHidden) { ui[recAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the recommended server clusters */
function recOnServerClustersChange(inst, ui)
{
  rcmaScript.rcmaOnServerClusterChange(inst, ui);
  rcoaScript.rcoaOnServerClusterChange(inst, ui);
  rcmcgScript.rcmcgOnServerClusterChange(inst, ui);
  rcocgScript.rcocgOnServerClusterChange(inst, ui);
  rcmcrScript.rcmcrOnServerClusterChange(inst, ui);
  rcocrScript.rcocrOnServerClusterChange(inst, ui);
  bdbScript.bdbOnChange(inst, ui);
}

/* Function to handle changes in the recommended client clusters */
function recOnClientClustersChange(inst, ui)
{
  rcmaScript.rcmaOnClientClusterChange(inst, ui);
  rcoaScript.rcoaOnClientClusterChange(inst, ui);
  rcmcgScript.rcmcgOnClientClusterChange(inst, ui);
  rcocgScript.rcocgOnClientClusterChange(inst, ui);
  rcmcrScript.rcmcrOnClientClusterChange(inst, ui);
  rcocrScript.rcocrOnClientClusterChange(inst, ui);
  bdbScript.bdbOnChange(inst, ui);
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  recModule: recModule,
  recOnDeviceTypeChange: recOnDeviceTypeChange
};
