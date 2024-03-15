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
 *  ======== zigbee_bdb_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Attributes from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get devices from ZDL.js */
let devices = system.getScript("/ti/zstack/stack/zcl/ZDL.js").ZDL.devices;

/* Get BDB long descriptions */
const docs = system.getScript("/ti/zstack/app/bdb_config/zigbee_bdb_config_docs.js");

Common.addGlobalClusters(devices, clusters);

/* Configurables for the Zigbee Application BDB Reporting module */
let bdbModule = {
    displayName: "BDB Reporting",
    config: [
      {
        name: "bdbReportingMaxAnalogAttrSize",
        displayName: "Max Analog Attribute Size",
        description: "BDB Reporting Max Analog Attribute Size",
        longDescription: docs.bdbReportingMaxAnalogAttrSizeLongDescription,
        default: 4,
        options: [ {name: 2}, {name: 4}, {name: 8} ],
        hidden: true
      }
    ]
};

/* Build Attribute Drop Downs */
let serverReportableArray = [];
let serverMaxArray = [];
let serverMinArray = [];
let clientReportableArray = [];
let clientMaxArray = [];
let clientMinArray = [];
for (let i in clusters) {
  let cluster = clusters[i];
  if (cluster.server && cluster.server.attribute) {
    for (let j in cluster.server.attribute) {
      let attribute = cluster.server.attribute[j];
      if (attribute._reportable == "true") {
        let attConfig = {
          displayName: cluster._name + ": " + attribute._name,
          config: []
        }
        attConfig.config.push({
          name: "server" + cluster._id + attribute._id + "reportableChange",
          displayName: "Reportable Change",
          description: "Reportable Change Delta for the " + cluster._name + " Cluster, " + attribute._name + " Attribute.",
          longDescription: docs.bdbReportableChangeLongDescription,
          default: 0x0,
          displayFormat: "hex",
          hidden: true
        })
        serverReportableArray.push("server" + cluster._id + attribute._id + "reportableChange")

        attConfig.config.push({
          name: "server" + cluster._id + attribute._id + "maxReportInterval",
          displayName: "Maximum Reporting Interval",
          description: "Maxiumum Reportable Change for the " + cluster._name + " Cluster, " + attribute._name + " Attribute.",
          longDescription: docs.bdbMaximumReportingIntervalLongDescription,
          default: 0x0,
          displayFormat: "hex",
          hidden: true
        })
        serverMaxArray.push("server" + cluster._id + attribute._id + "maxReportInterval")

        attConfig.config.push({
          name: "server" + cluster._id + attribute._id + "minReportInterval",
          displayName: "Minimum Reporting Interval",
          description: "Minimum Reportable Change for the " + cluster._name + " Cluster, " + attribute._name + " Attribute.",
          longDescription: docs.bdbMinimumReportingIntervalLongDescription,
          default: 0x0,
          displayFormat: "hex",
          hidden: true
        })
        serverMinArray.push("server" + cluster._id + attribute._id + "minReportInterval")

        bdbModule.config.push(attConfig);
      }
    }
  }
  if (cluster.client && cluster.client.attribute) {
    for (let j in cluster.client.attribute) {
      let attribute = cluster.client.attribute[j];
      if (attribute._reportable == "true") {
        let attConfig = {
          displayName: cluster._name + ": " + attribute._name,
          config: []
        }
        attConfig.config.push({
          name: "client" + cluster._id + attribute._id + "reportableChange",
          displayName: "Reportable Change",
          description: "Reportable Change Delta for the " + cluster._name + " Cluster, " + attribute._name + " Attribute.",
          longDescription: docs.bdbReportableChangeLongDescription,
          default: 0x0,
          displayFormat: "hex",
          hidden: true
        })
        clientReportableArray.push("client" + cluster._id + attribute._id + "reportableChange")

        attConfig.config.push({
          name: "client" + cluster._id + attribute._id + "maxReportInterval",
          displayName: "Maximum Reporting Interval",
          description: "Maxiumum Reportable Change for the " + cluster._name + " Cluster, " + attribute._name + " Attribute.",
          longDescription: docs.bdbMaximumReportingIntervalLongDescription,
          default: 0x0,
          displayFormat: "hex",
          hidden: true
        })
        clientMaxArray.push("client" + cluster._id + attribute._id + "maxReportInterval")

        attConfig.config.push({
          name: "client" + cluster._id + attribute._id + "minReportInterval",
          displayName: "Minimum Reporting Interval",
          description: "Minimum Reportable Change for the " + cluster._name + " Cluster, " + attribute._name + " Attribute.",
          longDescription: docs.bdbMinimumReportingIntervalLongDescription,
          default: 0x0,
          displayFormat: "hex",
          hidden: true
        })
        clientMinArray.push("client" + cluster._id + attribute._id + "minReportInterval")

        bdbModule.config.push(attConfig);
      }
    }
  }
}

function bdbOnChange(inst, ui)
{
  ui["bdbReportingMaxAnalogAttrSize"].hidden = true;

  /* Block to handle changes in the server clusters */
  for (let i in serverReportableArray) { ui[serverReportableArray[i]].hidden = true; }
  for (let i in serverMaxArray) { ui[serverMaxArray[i]].hidden = true; }
  for (let i in serverMinArray) { ui[serverMinArray[i]].hidden = true; }

  let clusters = inst[inst.zigbeeDeviceType + "manServer"];
  for (let i in clusters) {
    let attributes = inst[clusters[i] + "mcmaServer"];
    for (let j in attributes) {
      if (ui["server" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["server" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
    attributes = inst[clusters[i] + "mcoaServer"];
    for (let j in attributes) {
      if (ui["server" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["server" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
  }
  clusters = inst[inst.zigbeeDeviceType + "recServer"];
  for (let i in clusters) {
    let attributes = inst[clusters[i] + "rcmaServer"];
    for (let j in attributes) {
      if (ui["server" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["server" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
    attributes = inst[clusters[i] + "rcoaServer"];
    for (let j in attributes) {
      if (ui["server" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["server" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
  }
  clusters = inst[inst.zigbeeDeviceType + "addServer"];
  for (let i in clusters) {
    let attributes = inst[clusters[i] + "acmaServer"];
    for (let j in attributes) {
      if (ui["server" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["server" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
    attributes = inst[clusters[i] + "acoaServer"];
    for (let j in attributes) {
      if (ui["server" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["server" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["server" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
  }

  /* Block to handle changes in the client clusters */
  for (let i in clientReportableArray) { ui[clientReportableArray[i]].hidden = true; }
  for (let i in clientMaxArray) { ui[clientMaxArray[i]].hidden = true; }
  for (let i in clientMinArray) { ui[clientMinArray[i]].hidden = true; }

  clusters = inst[inst.zigbeeDeviceType + "manClient"];
  for (let i in clusters) {
    let attributes = inst[clusters[i] + "mcmaClient"];
    for (let j in attributes) {
      if (ui["client" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["client" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
    attributes = inst[clusters[i] + "mcoaClient"];
    for (let j in attributes) {
      if (ui["client" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["client" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
  }
  clusters = inst[inst.zigbeeDeviceType + "recClient"];
  for (let i in clusters) {
    let attributes = inst[clusters[i] + "rcmaClient"];
    for (let j in attributes) {
      if (ui["client" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["client" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
    attributes = inst[clusters[i] + "rcoaClient"];
    for (let j in attributes) {
      if (ui["client" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["client" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
  }
  clusters = inst[inst.zigbeeDeviceType + "addClient"];
  for (let i in clusters) {
    let attributes = inst[clusters[i] + "acmaClient"];
    for (let j in attributes) {
      if (ui["client" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["client" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
    attributes = inst[clusters[i] + "acoaClient"];
    for (let j in attributes) {
      if (ui["client" + clusters[i] + attributes[j] + "reportableChange"]) {
        ui["client" + clusters[i] + attributes[j] + "reportableChange"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "maxReportInterval"].hidden = false;
        ui["client" + clusters[i] + attributes[j] + "minReportInterval"].hidden = false;
        ui["bdbReportingMaxAnalogAttrSize"].hidden = false;
      }
    }
  }
}

/* Exports to the top level of the Zigbee Application submodule */
exports = {
  bdbModule: bdbModule,
  bdbOnChange: bdbOnChange
};
