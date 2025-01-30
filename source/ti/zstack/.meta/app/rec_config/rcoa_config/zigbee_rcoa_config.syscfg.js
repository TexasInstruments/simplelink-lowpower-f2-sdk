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
 *  ======== zigbee_rcoa_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Attributes from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get mo long descriptions */
const docs = system.getScript("/ti/zstack/app/rec_config/rcoa_config/zigbee_rcoa_config_docs.js");

/* Get BDB Settings script */
const bdbScript = system.getScript("/ti/zstack/app/bdb_config/zigbee_bdb_config");

/* Configurables for the Zigbee Application Optional Attributes module */
const rcoaModule = {
    displayName: "Optional Attributes",
    config: [],
};

let rcoaServerDropDowns = [];
let rcoaClientDropDowns = [];
let rcoaServerAlwaysHidden = [];
let rcoaClientAlwaysHidden = [];
for (let i in clusters) {
  let cluster = clusters[i];

  let tempServerAttributeOptions = [];
  if (cluster.server && cluster.server.attribute) {
    for (let j in cluster.server.attribute) {
      let attribute = cluster.server.attribute[j];
      if (attribute._required == "false") {
        tempServerAttributeOptions.push({name: attribute._id, displayName: attribute._name});
      }
    }
  }

  if (tempServerAttributeOptions.length == 0) {
    rcoaServerAlwaysHidden.push(cluster._id + "rcoaServer");
    tempServerAttributeOptions.push({name: "dummy", displayName: "dummy"});
  }
  rcoaModule.config.push({
    name: cluster._id + "rcoaServer",
    displayName: cluster._name + " Server Attributes",
    description: "Optional Zigbee Server Attributes for the " + cluster._name + " cluster.",
    longDescription: docs.optAttributesLongDescription,
    default: [],
    options: tempServerAttributeOptions,
    hidden: true,
    minSelections: 0,
    onChange: bdbScript.bdbOnChange
  })
  rcoaServerDropDowns.push(cluster._id + "rcoaServer");

  let tempClientAttributeOptions = [];
  if (cluster.client && cluster.client.attribute) {
    for (let j in cluster.client.attribute) {
      let attribute = cluster.client.attribute[j];
      if (attribute._required == "false") {
        tempClientAttributeOptions.push({name: attribute._id, displayName: attribute._name});
      }
    }
  }

  if (tempClientAttributeOptions.length == 0) {
    rcoaClientAlwaysHidden.push(cluster._id + "rcoaClient");
    tempClientAttributeOptions.push({name: "dummy", displayName: "dummy"});
  }
  rcoaModule.config.push({
    name: cluster._id + "rcoaClient",
    displayName: cluster._name + " Client Attributes",
    description: "Optional Zigbee Client Attributes for the " + cluster._name + " cluster.",
    longDescription: docs.optAttributesLongDescription,
    default: [],
    options: tempClientAttributeOptions,
    hidden: true,
    minSelections: 0,
    onChange: bdbScript.bdbOnChange
  })
  rcoaClientDropDowns.push(cluster._id + "rcoaClient");
}

/* Function to handle changes in the recommended server clusters */
function rcoaOnServerClusterChange(inst, ui)
{
  for (let i in rcoaServerDropDowns) { ui[rcoaServerDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "recServer"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "rcoaServer"].hidden = false; }

  for (let i in rcoaServerAlwaysHidden) { ui[rcoaServerAlwaysHidden[i]].hidden = true; }
}

/* Function to handle changes in the recommended client clusters */
function rcoaOnClientClusterChange(inst, ui)
{
  for (let i in rcoaClientDropDowns) { ui[rcoaClientDropDowns[i]].hidden = true; }

  let clusterIDs = inst[inst.zigbeeDeviceType + "recClient"];
  for (let i in clusterIDs) { ui[clusterIDs[i] + "rcoaClient"].hidden = false; }

  for (let i in rcoaClientAlwaysHidden) { ui[rcoaClientAlwaysHidden[i]].hidden = true; }
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  rcoaModule: rcoaModule,
  rcoaOnServerClusterChange: rcoaOnServerClusterChange,
  rcoaOnClientClusterChange: rcoaOnClientClusterChange
};
