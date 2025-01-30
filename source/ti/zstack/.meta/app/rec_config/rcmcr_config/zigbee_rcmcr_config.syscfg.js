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
 *  ======== zigbee_rcmcr_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Commands from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get rcmcr long descriptions */
const docs = system.getScript("/ti/zstack/app/rec_config/rcmcr_config/zigbee_rcmcr_config_docs.js");

/* Configurables for the Zigbee Application Mandatory Commands Received module */
const rcmcrModule = {
    displayName: "Mandatory Commands Received",
    config: [],
};

let rcmcrServerDropDowns = [];
let rcmcrClientDropDowns = [];
let rcmcrServerAlwaysHidden = [];
let rcmcrClientAlwaysHidden = [];
for (let i in clusters) {
  let cluster = clusters[i];

  let tempServerCommandDefault = [];
  let tempServerCommandOptions = [];
  if (cluster.server && cluster.server.command) {
    for (let j=0; j<cluster.server.command.length; j++) {
      let command = cluster.server.command[j];
      if (command._required == "true") {
        tempServerCommandDefault.push(command._id);
        tempServerCommandOptions.push({name: command._id, displayName: command._name});
      }
    }
  }

  if (tempServerCommandOptions.length == 0) {
    rcmcrServerAlwaysHidden.push(cluster._id + "rcmcrServer");
    tempServerCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  rcmcrModule.config.push({
    name: cluster._id + "rcmcrServer",
    displayName: cluster._name + " Server Commands Received",
    description: "Mandatory Zigbee Server Commands Received for the " + cluster._name + " cluster.",
    longDescription: docs.manCommandsLongDescription,
    default: tempServerCommandDefault,
    options: tempServerCommandOptions,
    hidden: true,
    minSelections: 0
  })
  rcmcrServerDropDowns.push(cluster._id + "rcmcrServer");

  let tempClientCommandDefault = [];
  let tempClientCommandOptions = [];
  if (cluster.client && cluster.client.command) {
    for (let j=0; j<cluster.client.command.length; j++) {
      let command = cluster.client.command[j];
      if (command._required == "true") {
        tempClientCommandDefault.push(command._id);
        tempClientCommandOptions.push({name: command._id, displayName: command._name});
      }
    }
  }

  if (tempClientCommandOptions.length == 0) {
    rcmcrClientAlwaysHidden.push(cluster._id + "rcmcrClient");
    tempClientCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  rcmcrModule.config.push({
    name: cluster._id + "rcmcrClient",
    displayName: cluster._name + " Client Commands Received",
    description: "Mandatory Zigbee Client Commands Received for the " + cluster._name + " cluster.",
    longDescription: docs.manCommandsLongDescription,
    default: tempClientCommandDefault,
    options: tempClientCommandOptions,
    hidden: true,
    minSelections: 0
  })
  rcmcrClientDropDowns.push(cluster._id + "rcmcrClient");
}

/* Function to handle changes in the recommended server clusters */
function rcmcrOnServerClusterChange(inst, ui)
{
  for (let i in rcmcrServerDropDowns) { ui[rcmcrServerDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "recServer"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "rcmcrServer"].hidden = false; }

  for (let i in rcmcrServerAlwaysHidden) { ui[rcmcrServerAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the recommended client clusters */
function rcmcrOnClientClusterChange(inst, ui)
{
  for (let i in rcmcrClientDropDowns) { ui[rcmcrClientDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "recClient"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "rcmcrClient"].hidden = false; }

  for (let i in rcmcrClientAlwaysHidden) { ui[rcmcrClientAlwaysHidden[i]].hidden = true; }
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  rcmcrModule: rcmcrModule,
  rcmcrOnServerClusterChange: rcmcrOnServerClusterChange,
  rcmcrOnClientClusterChange: rcmcrOnClientClusterChange
};
