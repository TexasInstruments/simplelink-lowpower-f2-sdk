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
 *  ======== zigbee_acocr_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Commands from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get acocr long descriptions */
const docs = system.getScript("/ti/zstack/app/add_config/acocr_config/zigbee_acocr_config_docs.js");

/* Configurables for the Zigbee Application Optional Commands Received module */
const acocrModule = {
    displayName: "Optional Commands Received",
    config: [],
};

let acocrServerDropDowns = [];
let acocrClientDropDowns = [];
let acocrServerAlwaysHidden = [];
let acocrClientAlwaysHidden = [];
for (let i in clusters) {
  let cluster = clusters[i];

  let tempServerCommandOptions = [];
  if (cluster.server && cluster.server.command) {
    for (let j in cluster.server.command) {
      let command = cluster.server.command[j];
      if (command._required == "false") {
        tempServerCommandOptions.push({name: command._id, displayName: command._name});
      }
    }
  }

  if (tempServerCommandOptions.length == 0) {
    acocrServerAlwaysHidden.push(cluster._id + "acocrServer");
    tempServerCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  acocrModule.config.push({
    name: cluster._id + "acocrServer",
    displayName: cluster._name + " Server Commands Received",
    description: "Optional Zigbee Server Commands Received for the " + cluster._name + " cluster.",
    longDescription: docs.optCommandsLongDescription,
    default: [],
    options: tempServerCommandOptions,
    hidden: true,
    minSelections: 0
  })
  acocrServerDropDowns.push(cluster._id + "acocrServer");

  let tempClientCommandOptions = [];
  if (cluster.client && cluster.client.command) {
    for (let j in cluster.client.command) {
      let command = cluster.client.command[j];
      if (command._required == "false") {
        tempClientCommandOptions.push({name: command._id, displayName: command._name});
      }
    }
  }

  if (tempClientCommandOptions.length == 0) {
    acocrClientAlwaysHidden.push(cluster._id + "acocrClient");
    tempClientCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  acocrModule.config.push({
    name: cluster._id + "acocrClient",
    displayName: cluster._name + " Client Commands Received",
    description: "Optional Zigbee Client Commands Received for the " + cluster._name + " cluster.",
    longDescription: docs.optCommandsLongDescription,
    default: [],
    options: tempClientCommandOptions,
    hidden: true,
    minSelections: 0
  })
  acocrClientDropDowns.push(cluster._id + "acocrClient");
}

/* Function to handle changes in the additional server clusters */
function acocrOnServerClusterChange(inst, ui)
{
  for (let i in acocrServerDropDowns) { ui[acocrServerDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "addServer"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "acocrServer"].hidden = false; }

  for (let i in acocrServerAlwaysHidden) { ui[acocrServerAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the additional client clusters */
function acocrOnClientClusterChange(inst, ui)
{
  for (let i in acocrClientDropDowns) { ui[acocrClientDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "addClient"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "acocrClient"].hidden = false; }

  for (let i in acocrClientAlwaysHidden) { ui[acocrClientAlwaysHidden[i]].hidden = true; }
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  acocrModule: acocrModule,
  acocrOnServerClusterChange: acocrOnServerClusterChange,
  acocrOnClientClusterChange: acocrOnClientClusterChange
};
