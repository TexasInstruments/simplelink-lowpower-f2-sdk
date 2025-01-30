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
 *  ======== zigbee_mcma_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Attributes from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get mcma long descriptions */
const docs = system.getScript("/ti/zstack/app/man_config/mcma_config/zigbee_mcma_config_docs.js");

/* Get BDB Settings script */
const bdbScript = system.getScript("/ti/zstack/app/bdb_config/zigbee_bdb_config");

/* Configurables for the Zigbee Application Mandatory Attributes module */
const mcmaModule = {
    displayName: "Mandatory Attributes",
    config: [],
};

let mcmaServerDropDowns = [];
let mcmaClientDropDowns = [];
let mcmaServerAlwaysHidden = [];
let mcmaClientAlwaysHidden = [];
for (let i in clusters) {
  let cluster = clusters[i];

  let tempServerAttributeDefault = [];
  let tempServerAttributeOptions = [];
  if (cluster.server && cluster.server.attribute) {
    for (let j in cluster.server.attribute) {
      let attribute = cluster.server.attribute[j];
      if (attribute._required == "true") {
        tempServerAttributeDefault.push(attribute._id);
        tempServerAttributeOptions.push({name: attribute._id, displayName: attribute._name});
      }
    }
  }

  if (tempServerAttributeOptions.length == 0) {
    mcmaServerAlwaysHidden.push(cluster._id + "mcmaServer");
    tempServerAttributeOptions.push({name: "dummy", displayName: "dummy"});
  }
  mcmaModule.config.push({
    name: cluster._id + "mcmaServer",
    displayName: cluster._name + " Server Attributes",
    description: "Mandatory Zigbee Server Attributes for the " + cluster._name + " cluster.",
    longDescription: docs.manAttributesLongDescription,
    default: tempServerAttributeDefault,
    options: tempServerAttributeOptions,
    hidden: true,
    minSelections: 0,
    onChange: bdbScript.bdbOnChange
  })
  mcmaServerDropDowns.push(cluster._id + "mcmaServer");

  let tempClientAttributeDefault = [];
  let tempClientAttributeOptions = [];
  if (cluster.client && cluster.client.attribute) {
    for (let j in cluster.client.attribute) {
      let attribute = cluster.client.attribute[j];
      if (attribute._required == "true") {
        tempClientAttributeDefault.push(attribute._id);
        tempClientAttributeOptions.push({name: attribute._id, displayName: attribute._name});
      }
    }
  }

  if (tempClientAttributeOptions.length == 0) {
    mcmaClientAlwaysHidden.push(cluster._id + "mcmaClient");
    tempClientAttributeOptions.push({name: "dummy", displayName: "dummy"});
  }
  mcmaModule.config.push({
    name: cluster._id + "mcmaClient",
    displayName: cluster._name + " Client Attributes",
    description: "Mandatory Zigbee Client Attributes for the " + cluster._name + " cluster.",
    longDescription: docs.manAttributesLongDescription,
    default: tempClientAttributeDefault,
    options: tempClientAttributeOptions,
    hidden: true,
    minSelections: 0,
    onChange: bdbScript.bdbOnChange
  })
  mcmaClientDropDowns.push(cluster._id + "mcmaClient");
}

/* Function to handle changes in the mandatory server clusters */
function mcmaOnServerClusterChange(inst, ui)
{
  for (let i in mcmaServerDropDowns) { ui[mcmaServerDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "manServer"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "mcmaServer"].hidden = false; }

  for (let i in mcmaServerAlwaysHidden) { ui[mcmaServerAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the mandatory client clusters */
function mcmaOnClientClusterChange(inst, ui)
{
  for (let i in mcmaClientDropDowns) { ui[mcmaClientDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "manClient"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "mcmaClient"].hidden = false; }

  for (let i in mcmaClientAlwaysHidden) { ui[mcmaClientAlwaysHidden[i]].hidden = true; }
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  mcmaModule: mcmaModule,
  mcmaOnServerClusterChange: mcmaOnServerClusterChange,
  mcmaOnClientClusterChange: mcmaOnClientClusterChange
};
