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
 *  ======== zigbee_mcocr_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Commands from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get mcocr long descriptions */
const docs = system.getScript("/ti/zstack/app/man_config/mcocr_config/zigbee_mcocr_config_docs.js");

/* Configurables for the Zigbee Application Optional Commands Received module */
const mcocrModule = {
    displayName: "Optional Commands Received",
    config: [],
};

let mcocrServerDropDowns = [];
let mcocrClientDropDowns = [];
let mcocrServerAlwaysHidden = [];
let mcocrClientAlwaysHidden = [];
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
    mcocrServerAlwaysHidden.push(cluster._id + "mcocrServer");
    tempServerCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  mcocrModule.config.push({
    name: cluster._id + "mcocrServer",
    displayName: cluster._name + " Server Commands Receieved",
    description: "Optional Zigbee Server Commands for the " + cluster._name + " cluster.",
    longDescription: docs.optCommandsRecLongDescription,
    default: [],
    options: tempServerCommandOptions,
    hidden: true,
    minSelections: 0
  })
  mcocrServerDropDowns.push(cluster._id + "mcocrServer");

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
    mcocrClientAlwaysHidden.push(cluster._id + "mcocrClient");
    tempClientCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  mcocrModule.config.push({
    name: cluster._id + "mcocrClient",
    displayName: cluster._name + " Client Commands Received",
    description: "Optional Zigbee Client Commands for the " + cluster._name + " cluster.",
    longDescription: docs.optCommandsRecLongDescription,
    default: [],
    options: tempClientCommandOptions,
    hidden: true,
    minSelections: 0
  })
  mcocrClientDropDowns.push(cluster._id + "mcocrClient");
}

/* Function to handle changes in the mandatory server clusters */
function mcocrOnServerClusterChange(inst, ui)
{
  for (let i in mcocrServerDropDowns) { ui[mcocrServerDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "manServer"];
  for (let i=0; i<clusterIDs.length; i++) { ui[clusterIDs[i] + "mcocrServer"].hidden = false; }

  for (let i in mcocrServerAlwaysHidden) { ui[mcocrServerAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the mandatory client clusters */
function mcocrOnClientClusterChange(inst, ui)
{
  for (let i in mcocrClientDropDowns) { ui[mcocrClientDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "manClient"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "mcocrClient"].hidden = false; }

  for (let i in mcocrClientAlwaysHidden) { ui[mcocrClientAlwaysHidden[i]].hidden = true; }
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  mcocrModule: mcocrModule,
  mcocrOnServerClusterChange: mcocrOnServerClusterChange,
  mcocrOnClientClusterChange: mcocrOnClientClusterChange
};
