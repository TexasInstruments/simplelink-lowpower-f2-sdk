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
 *  ======== zigbee_mcmcr_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Attributes from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get mcmcr long descriptions */
const docs = system.getScript("/ti/zstack/app/man_config/mcmcr_config/zigbee_mcmcr_config_docs.js");

/* Configurables for the Zigbee Application Mandatory Commands Received module */
const mcmcrModule = {
    displayName: "Mandatory Commands Received",
    config: [],
};

let mcmcrServerDropDowns = [];
let mcmcrClientDropDowns = [];
let mcmcrServerAlwaysHidden = [];
let mcmcrClientAlwaysHidden = [];
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
    mcmcrServerAlwaysHidden.push(cluster._id + "mcmcrServer");
    tempServerCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  mcmcrModule.config.push({
    name: cluster._id + "mcmcrServer",
    displayName: cluster._name + " Server Commands",
    description: "Mandatory Zigbee Server Commands for the " + cluster._name + " cluster.",
    longDescription: docs.manCommandsLongDescription,
    default: tempServerCommandDefault,
    options: tempServerCommandOptions,
    hidden: true,
    minSelections: 0
  })
  mcmcrServerDropDowns.push(cluster._id + "mcmcrServer");

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
    mcmcrClientAlwaysHidden.push(cluster._id + "mcmcrClient");
    tempClientCommandOptions.push({name: "dummy", displayName: "dummy"});
  }
  mcmcrModule.config.push({
    name: cluster._id + "mcmcrClient",
    displayName: cluster._name + " Client Commands",
    description: "Mandatory Zigbee Client Commands for the " + cluster._name + " cluster.",
    longDescription: docs.manCommandsLongDescription,
    default: tempClientCommandDefault,
    options: tempClientCommandOptions,
    hidden: true,
    minSelections: 0
  })
  mcmcrClientDropDowns.push(cluster._id + "mcmcrClient");
}

/* Function to handle changes in the mandatory server clusters */
function mcmcrOnServerClusterChange(inst, ui)
{
  for (let i in mcmcrServerDropDowns) { ui[mcmcrServerDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "manServer"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "mcmcrServer"].hidden = false; }

  for (let i in mcmcrServerAlwaysHidden) { ui[mcmcrServerAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the mandatory client clusters */
function mcmcrOnClientClusterChange(inst, ui)
{
  for (let i in mcmcrClientDropDowns) { ui[mcmcrClientDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "manClient"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "mcmcrClient"].hidden = false; }

  for (let i in mcmcrClientAlwaysHidden) { ui[mcmcrClientAlwaysHidden[i]].hidden = true; }
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  mcmcrModule: mcmcrModule,
  mcmcrOnServerClusterChange: mcmcrOnServerClusterChange,
  mcmcrOnClientClusterChange: mcmcrOnClientClusterChange
};
