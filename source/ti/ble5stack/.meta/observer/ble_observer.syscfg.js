/*
 * Copyright (c) 2018 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ble_observer.syscfg.js ========
 */

"use strict";

// Get observer long descriptions
const Docs = system.getScript("/ti/ble5stack/observer/ble_observer_docs.js");

// Get common utility functions
const Common = system.getScript("/ti/ble5stack/ble_common.js");

const config = {
    name: "observerConfig",
    displayName: "Observer Configuration",
    description: "Configure Observer Role Settings",
    config: [
        {
            name: "hideObserverGroup",
            default: true,
            hidden: true
        },
        {
            name: "scanPHY",
            displayName: "Scanning PHY",
            default: "SCAN_PRIM_PHY_1M",
            description: "The scanning PHY is the primary PHY",
            hidden: true,
            longDescription: Docs.scanPHYLongDescription,
            options: [
                {
                    displayName: "1M",
                    name: "SCAN_PRIM_PHY_1M"
                },
                {
                    displayName: "Coded",
                    name: "SCAN_PRIM_PHY_CODED"
                }
            ]
        },
        {
            name: "scanType",
            displayName: "Scan Type",
            default: "SCAN_TYPE_ACTIVE",
            hidden: true,
            longDescription: Docs.scanTypeLongDescription,
            options: [
                {
                    displayName: "Active",
                    name: "SCAN_TYPE_ACTIVE",
                    description: "Active Scanning. Scan request PDUs may be sent."
                },
                {
                    displayName: "Passive",
                    name: "SCAN_TYPE_PASSIVE",
                    description: "Passive Scanning. No scan request PDUs shall be sent."
                }
            ]
        },
        {
            name: "scanPeriod",
            displayName: "Scan period (sec)",
            default: 0,
            hidden: true,
            longDescription: Docs.scanPeriodLongDescription
        },
        {
            name: "scanInt",
            displayName: "Scan Interval (ms)",
            description: "Time interval from when the Controller started its last scan until it "
                         + "begins the subsequent scan on the primary advertising channel.",
            default: 500,
            hidden: true,
            longDescription: Docs.scanIntLongDescription
        },
        {
            name: "scanWin",
            displayName: "Scan Window (ms)",
            description: "Duration of the scan on the primary advertising channel.",
            default: 500,
            hidden: true,
            longDescription: Docs.scanWinLongDescription
        },
        {
            name: "scanDuration",
            displayName: "Scan Duration (ms)",
            description: "Scan duration",
            default: 1000,
            hidden: true,
            longDescription: Docs.scanDurationLongDescription
        },
        {
            name: "advRptFields",
            displayName: "Advertise Report Fields",
            description: "Advertising report fields to keep in the list",
            default: ["SCAN_ADVRPT_FLD_ADDRTYPE", "SCAN_ADVRPT_FLD_ADDRESS"],
            hidden: true,
            longDescription: Docs.advRptFieldsLongDescription,
            options: [
                {
                    displayName: "Event Type",
                    name: "SCAN_ADVRPT_FLD_EVENTTYPE"
                },
                {
                    displayName: "Address Type",
                    name: "SCAN_ADVRPT_FLD_ADDRTYPE"
                },
                {
                    displayName: "Address",
                    name: "SCAN_ADVRPT_FLD_ADDRESS"
                },
                {
                    displayName: "Primary PHY",
                    name: "SCAN_ADVRPT_FLD_PRIMPHY"
                },
                {
                    displayName: "Secondary PHY",
                    name: "SCAN_ADVRPT_FLD_SECPHY"
                },
                {
                    displayName: "Advertise Set Id",
                    name: "SCAN_ADVRPT_FLD_ADVSID"
                },
                {
                    displayName: "TX Power",
                    name: "SCAN_ADVRPT_FLD_TXPOWER"
                },
                {
                    displayName: "RSSI",
                    name: "SCAN_ADVRPT_FLD_RSSI"
                },
                {
                    displayName: "dirAddrType",
                    name: "SCAN_ADVRPT_FLD_DIRADDRTYPE"
                },
                {
                    displayName: "dirAddress",
                    name: "SCAN_ADVRPT_FLD_DIRADDRESS"
                },
                {
                    displayName: "prdAdvInterval",
                    name: "SCAN_ADVRPT_FLD_PRDADVINTERVAL"
                },
                {
                    displayName: "Data Length",
                    name: "SCAN_ADVRPT_FLD_DATALEN"
                }
            ]
        },
        {
            name: "fltPolicy",
            displayName: "Filter Policy",
            default: "SCAN_FLT_POLICY_ALL",
            hidden: true,
            longDescription: Docs.fltPolicyLongDescription,
            options: [
                { displayName: "Accept all", name: "SCAN_FLT_POLICY_ALL" },
                { displayName: "Accept only from acceptlist", name: "SCAN_FLT_POLICY_AL"  },
                { displayName: "Accept RPA", name: "SCAN_FLT_POLICY_ALL_RPA" },
                { displayName: "Accept only from acceptlist and RPA", name: "SCAN_FLT_POLICY_AL_RPA" }
            ]
        },
        {
            name: "fltPduType",
            displayName: "PDU Type Filter",
            description: "PDU Types for PDU Type Filter",
            getDisabledOptions: generateDisabledOptions("fltPduType"),
            default: ["SCAN_FLT_PDU_CONNECTABLE_ONLY", "SCAN_FLT_PDU_COMPLETE_ONLY"],
            longDescription: Docs.fltPduTypeLongDescription,
            hidden: true,
            minSelections: 0,
            options: [
                {
                    displayName: "Non-connectable only",
                    name: "SCAN_FLT_PDU_NONCONNECTABLE_ONLY"
                },
                {
                    displayName: "Connectable only",
                    name: "SCAN_FLT_PDU_CONNECTABLE_ONLY"
                },
                {
                    displayName: "Non-scannable only",
                    name: "SCAN_FLT_PDU_NONSCANNABLE_ONLY"
                },
                {
                    displayName: "Scannable only",
                    name: "SCAN_FLT_PDU_SCANNABLE_ONLY"
                },
                {
                    displayName: "Undirected only",
                    name: "SCAN_FLT_PDU_UNDIRECTED_ONLY"
                },
                {
                    displayName: "Directed only",
                    name: "SCAN_FLT_PDU_DIRECTED_ONLY"
                },
                {
                    displayName: "Advertisement only",
                    name: "SCAN_FLT_PDU_ADV_ONLY"
                },
                {
                    displayName: "Scan Response only",
                    name: "SCAN_FLT_PDU_SCANRSP_ONLY"
                },
                {
                    displayName: "Extended only",
                    name: "SCAN_FLT_PDU_EXTENDED_ONLY"
                },
                {
                    displayName: "Legacy only",
                    name: "SCAN_FLT_PDU_LEGACY_ONLY"
                },
                {
                    displayName: "Truncated only",
                    name: "SCAN_FLT_PDU_TRUNCATED_ONLY"
                },
                {
                    displayName: "Complete only",
                    name: "SCAN_FLT_PDU_COMPLETE_ONLY"
                }
            ]
        },
        {
            name: "fltMinRssi",
            displayName: "Filter Minimum RSSI Values",
            default: "SCAN_FLT_RSSI_ALL",
            hidden: true,
            longDescription: Docs.fltMinRssiLongDescription,
            options: [
                { displayName: "All", name: "SCAN_FLT_RSSI_ALL" },
                { displayName: "None", name: "SCAN_FLT_RSSI_NONE"  }
            ]
        },
        {
            name: "fltDiscMode",
            displayName: "Discoverable Mode Filter",
            default: "SCAN_FLT_DISC_DISABLE",
            longDescription: Docs.fltDiscModeLongDescription,
            hidden: true,
            options: [
                { displayName: "Non-discoverable", name: "SCAN_FLT_DISC_NONE" },
                { displayName: "General", name: "SCAN_FLT_DISC_GENERAL"  },
                { displayName: "Limited", name: "SCAN_FLT_DISC_LIMITED" },
                { displayName: "General or Limited", name: "SCAN_FLT_DISC_ALL"  },
                { displayName: "Disable", name: "SCAN_FLT_DISC_DISABLE" }
            ]
        },
        {
            name: "dupFilter",
            displayName: "Duplicate Filter",
            default: "SCAN_FLT_DUP_ENABLE",
            hidden: true,
            options: [
                { displayName: "Disabled", name: "SCAN_FLT_DUP_DISABLE" },
                { displayName: "Enabled", name: "SCAN_FLT_DUP_ENABLE"  },
                { displayName: "Enabled, reset for each scan period", name: "SCAN_FLT_DUP_RESET" }
            ]
        },
        {
            name: "disDevBySerUuid",
            displayName: "Discover Devices By Service UUID",
            description: "Check in order to discover only peripheral devices "
                       + "with a desired service UUID",
            longDescription: Docs.disDevBySerUuidLongDescription,
            default: false,
            hidden: true
        },
        {
            name: "advReportChanNum",
            displayName: "Add Advertisement Channel Number",
            description: "When enabled, adds the advertising channel number "
                        + "to the advertise report",
            longDescription: Docs.advReportChanNumLongDescription,
            default: false,
            hidden: true
        },
        {
            name: "maxNumAdvReport",
            displayName: "Maximum number of advertising reports",
            longDescription: Docs.maxNumAdvReportLongDescription,
            default: 20,
            hidden: true
        }
    ]
};


/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - Observer instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    if(inst.scanInt < Common.connParamsRanges.scanIntMinValue ||
        inst.scanInt > Common.connParamsRanges.scanIntMaxValue)
    {
        validation.logError("Scan Interval range is " + Common.connParamsRanges.scanIntMinValue 
                            + " ms to " + Common.connParamsRanges.scanIntMaxValue + " ms", inst, "scanInt");
    }
    if(inst.scanPeriod < Common.connParamsRanges.scanPeriodMinValue ||
        inst.scanPeriod > Common.connParamsRanges.scanPeriodMaxValue)
    {
        validation.logError("Scan Period range is " + Common.connParamsRanges.scanPeriodMinValue
                            + " to " + Common.connParamsRanges.scanPeriodMaxValue + " sec", inst, "scanPeriod");
    }
    if(inst.scanWin < Common.connParamsRanges.scanWinMinValue ||
        inst.scanWin > Common.connParamsRanges.scanWinMaxValue)
    {
        validation.logError("Scan Window range is " + Common.connParamsRanges.scanWinMinValue +
                             " ms to " + Common.connParamsRanges.scanWinMaxValue + " ms", inst, "scanWin");
    }
    if((inst.scanDuration < Common.connParamsRanges.scanDurationMinValue ||
        inst.scanDuration > Common.connParamsRanges.scanDurationMaxValue) && inst.scanDuration != 0)
    {
        validation.logError("Scan Duration can be set to 0 or a value in the range of "
                            + Common.connParamsRanges.scanDurationMinValue + " ms to "
                            + Common.connParamsRanges.scanDurationMaxValue + " ms", inst, "scanDuration");
    }
    if(inst.scanWin > inst.scanInt)
    {
        validation.logError("Scan Window shall be less than or equal to Scan Interval", inst, "scanWin");
        validation.logError("Scan Interval shall be greater than or equal to Scan Window", inst, "scanInt");
    }
    if(inst.scanInt >= inst.scanDuration)
    {
        validation.logError("Scan Duration shall be greater than Scan Interval", inst, ["scanDuration","scanInt"]);
    }
    if(inst.maxNumAdvReport < 0 || inst.maxNumAdvReport > 255)
    {
        validation.logError("Maximum number of advertising reports range is 0 to 255", inst, "maxNumAdvReport");
    }
}


/*
 *  ======== generateDisabledOptions ========
 *  Generates a list of options that should be disabled in a
 *  drop-down
 *
 * @returns Array - array of strings that should be disabled
*/
function generateDisabledOptions(name)
{
	return (inst) => {

        // Find the configurable we're going to generate a disabled list from
        const configurable = inst.$module.$configByName[name];

        if (name == "fltPduType")
        {
            let disabledOptions = [];// = configurable.options;

            // Disable the option to choose both Connectable and Non-connectable for PDU Type Filter
            if(inst.fltPduType.includes("SCAN_FLT_PDU_NONCONNECTABLE_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_CONNECTABLE_ONLY", reason: "Connectable and Non-connectable can not be used in the same time"});
            }
            if(inst.fltPduType.includes("SCAN_FLT_PDU_CONNECTABLE_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_NONCONNECTABLE_ONLY", reason: "Connectable and Non-connectable can not be used in the same time"});
            }

            // Disable the option to choose both Scannable and Non-scannable for PDU Type Filter
            if(inst.fltPduType.includes("SCAN_FLT_PDU_NONSCANNABLE_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_SCANNABLE_ONLY", reason: "Scannable and Non-scannable can not be used in the same time"});
            }
            if(inst.fltPduType.includes("SCAN_FLT_PDU_SCANNABLE_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_NONSCANNABLE_ONLY", reason: "Scannable and Non-scannable can not be used in the same time"});
            }

            // Disable the option to choose both Directed and Undirected for PDU Type Filter
            if(inst.fltPduType.includes("SCAN_FLT_PDU_UNDIRECTED_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_DIRECTED_ONLY", reason: "Directed and Undirected can not be used in the same time"});
            }
            if(inst.fltPduType.includes("SCAN_FLT_PDU_DIRECTED_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_UNDIRECTED_ONLY", reason: "Directed and Undirected can not be used in the same time"});
            }

            // Disable the option to choose both Advertisement and Scan Response for PDU Type Filter
            if(inst.fltPduType.includes("SCAN_FLT_PDU_ADV_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_SCANRSP_ONLY", reason: "Advertisement and Scan Response can not be used in the same time"});
            }
            if(inst.fltPduType.includes("SCAN_FLT_PDU_SCANRSP_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_ADV_ONLY", reason: "Advertisement and Scan Response can not be used in the same time"});
            }

            // Disable the option to choose both Extended and Legacy for PDU Type Filter
            if(inst.fltPduType.includes("SCAN_FLT_PDU_EXTENDED_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_LEGACY_ONLY", reason: "Extended and Legacy can not be used in the same time"});
            }
            if(inst.fltPduType.includes("SCAN_FLT_PDU_LEGACY_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_EXTENDED_ONLY", reason: "Extended and Legacy can not be used in the same time"});
            }

            // Disable the option to choose both Truncated and Complete for PDU Type Filter
            if(inst.fltPduType.includes("SCAN_FLT_PDU_TRUNCATED_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_COMPLETE_ONLY", reason: "Truncated and Complete can not be used in the same time"});
            }
            if(inst.fltPduType.includes("SCAN_FLT_PDU_COMPLETE_ONLY"))
            {
                disabledOptions = disabledOptions.concat({name: "SCAN_FLT_PDU_TRUNCATED_ONLY", reason: "Truncated and Complete can not be used in the same time"});
            }
            return disabledOptions;
        }
    }
}

// Exports to the top level BLE module
exports = {
    config: config,
    validate: validate
};
