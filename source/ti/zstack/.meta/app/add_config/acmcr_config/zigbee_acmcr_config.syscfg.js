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
 *  ======== zigbee_acmcr_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Commands from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get acmcr long descriptions */
const docs = system.getScript("/ti/zstack/app/add_config/acmcr_config/zigbee_acmcr_config_docs.js");

/* Configurables for the Zigbee Application Mandatory Commands Receieved module */
const acmcrModule = {
    displayName: "Mandatory Commands Received",
    config: [],
};

let acmcrServerDropDowns = [];
let acmcrClientDropDowns = [];
let acmcrServerAlwaysHidden = [];
let acmcrClientAlwaysHidden = [];
for (let i in clusters) {
  let cluster = clusters[i];

  let tempServerCommandDefault = [];
  let tempServerCommandOptions = [];
  if (cluster.server && cluster.server.command) {
    for (let j in cluster.server.command) {
      let command = cluster.server.command[j];
      if (command._required == "true") {
        tempServerCommandDefault.push(command._id);
        tempServerCommandOptions.push({name: command._id, displayName: command._name});
      }
    }
  }

  if (tempServerCommandOptions.length == 0) {
    acmcrServerAlwaysHidden.push(cluster._id + "acmcrServer");
    tempServerCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  acmcrModule.config.push({
    name: cluster._id + "acmcrServer",
    displayName: cluster._name + " Server Commands Received",
    description: "Mandatory Zigbee Server Commands Received for the " + cluster._name + " cluster.",
    longDescription: docs.manCommandsLongDescription,
    default: tempServerCommandDefault,
    options: tempServerCommandOptions,
    hidden: true,
    minSelections: 0
  })
  acmcrServerDropDowns.push(cluster._id + "acmcrServer");

  let tempClientCommandDefault = [];
  let tempClientCommandOptions = [];
  if (cluster.client && cluster.client.command) {
    for (let j in cluster.client.command) {
      let command = cluster.client.command[j];
      if (command._required == "true") {
        tempClientCommandDefault.push(command._id);
        tempClientCommandOptions.push({name: command._id, displayName: command._name});
      }
    }
  }

  if (tempClientCommandOptions.length == 0) {
    acmcrClientAlwaysHidden.push(cluster._id + "acmcrClient");
    tempClientCommandOptions.push({name: "dummy", displayName: "dummy"});
}
  acmcrModule.config.push({
    name: cluster._id + "acmcrClient",
    displayName: cluster._name + " Client Commands Received",
    description: "Mandatory Zigbee Client Commands Received for the " + cluster._name + " cluster.",
    longDescription: docs.manCommandsLongDescription,
    default: tempClientCommandDefault,
    options: tempClientCommandOptions,
    hidden: true,
    minSelections: 0
  })
  acmcrClientDropDowns.push(cluster._id + "acmcrClient");
}

/* Function to handle changes in the additional server clusters */
function acmcrOnServerClusterChange(inst, ui)
{
  for (let i in acmcrServerDropDowns) { ui[acmcrServerDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "addServer"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "acmcrServer"].hidden = false; }

  for (let i in acmcrServerAlwaysHidden) { ui[acmcrServerAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the additional client clusters */
function acmcrOnClientClusterChange(inst, ui)
{
  for (let i in acmcrClientDropDowns) { ui[acmcrClientDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "addClient"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "acmcrClient"].hidden = false; }

  for (let i in acmcrClientAlwaysHidden) { ui[acmcrClientAlwaysHidden[i]].hidden = true; }
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  acmcrModule: acmcrModule,
  acmcrOnServerClusterChange: acmcrOnServerClusterChange,
  acmcrOnClientClusterChange: acmcrOnClientClusterChange
};
