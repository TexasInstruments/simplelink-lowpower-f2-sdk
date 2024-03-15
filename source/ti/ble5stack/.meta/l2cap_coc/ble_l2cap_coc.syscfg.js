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
 *  ======== ble_l2cap_coc.syscfg.js ========
 */

"use strict";

// Get l2cap long descriptions
const Docs = system.getScript("/ti/ble5stack/l2cap_coc/ble_l2cap_coc_docs.js");

const config = {
    name: "l2capConfig",
    displayName:"L2CAP",
    description: "Configure L2CAP Settings",
    longDescription: Docs.L2CAPConfigLongDescription,
    config: [
        {
            name: "hideL2CAPGroup",
            default: true,
            hidden: true
        },
        {
            name: "L2CAPCOCpsmId",
            displayName: "L2CAP Connection PSM ID",
            longDescription: Docs.L2CAPCOCpsmIdLongDescription,
            hidden: true,
            default: 1,
        },
        {
            name: "L2CAPCOCmaxMTU",
            displayName: "MTU (Maximum L2CAP Transission Unit). MTU corresponds to the SDU size",
            longDescription: Docs.L2CAPCOCmaxMTULongDescription,
            hidden: true,
            default: 1400,
        },
        {
            name: "L2CAPCOCmaxMPS",
            displayName: "MPS (Maximum PDU Payload Size)",
            longDescription: Docs.L2CAPCOCmaxMPSLongDescription,
            hidden: true,
            default: 1024,
        },
        {
            name: "L2CAPCOCnofCredits",
            displayName: "Credits Number",
            longDescription: Docs.L2CAPCOCcreditsLongDescription,
            hidden: true,
            default: 0xfff0,
        },
        {
            name: "L2CAPCOCcreditsThreshold",
            displayName: "Peer Credits Threshold",
            longDescription: Docs.L2CAPCOCcreditsThresholdLongDescription,
            hidden: true,
            default: 5,
        },
        {
            name: "L2CAPCOCInitiator",
            displayName: "L2CAP Connection Role",
            default: "L2CAPCOC_RESPONDER",
            longDescription: Docs.L2CAPCOCConnRoleLongDescription,
            hidden: true,
            onChange: onL2CAPCOCInitiatorChange,
            options: [
                {
                    displayName: "COC Initiator",
                    name: "L2CAPCOC_INITIATOR",
                    description: "The device Initiates the COC request"
                },
                {
                    displayName: "COC Responder",
                    name: "L2CAPCOC_RESPONDER",
                    description: "The device responds to the COC request"
                },
            ]
        },
        {
            name: "L2CAPCOPeerPsmId",
            displayName: "Peer PSM Id",
            longDescription: Docs.L2CAPCOCPeerPsmIdLongDescription,
            hidden: true,
            default: 1,
        },
    ]
}

/*
 *  ======== onL2CAPCOCInitiatorChange ========
 * Add/remove the L2CAPCOPeerPsmId
 *
 * @param inst  - Module instance containing the config that changed
 * @param ui    - The User Interface object
 */
function onL2CAPCOCInitiatorChange(inst,ui)
{
    ui.L2CAPCOPeerPsmId.hidden = (inst.L2CAPCOCInitiator ==  "L2CAPCOC_RESPONDER") ? true : false;
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - L2CAP Manager instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    //check what is the max value
    if ( (inst.L2CAPCOCmaxMTU < 64 ) ||
         (inst.L2CAPCOCmaxMTU > 0xFFF7 /* (0xffff - 8) - considering the l2cap headers */ ) )
    {
       validation.logError("Maximum MTU Size should be in range [64-65,535]" , inst);
    }
    if ( (inst.L2CAPCOCmaxMPS < 64 ) ||
         (inst.L2CAPCOCmaxMPS > inst.L2CAPCOCmaxMTU ) )
    {
       validation.logError("Maximum MPS Size should be in range [64-MTU]" , inst);
    }
    if ( (inst.L2CAPCOCnofCredits < 0) ||
         (inst.L2CAPCOCnofCredits > 0xffff) )
    {
        validation.logError("Credits Number should be in range [0-65,535] \n" +
        "The (\"Credits Number\" + \"Peer Credits Threshold\") should be in range [0-65.535]", inst);
    }

    if ( (inst.L2CAPCOCcreditsThreshold < 0) ||
         (inst.L2CAPCOCcreditsThreshold > 0xffff) ||
         (inst.L2CAPCOCcreditsThreshold + inst.L2CAPCOCnofCredits) > 0xffff)
    {
        validation.logError("Peer Credits Threshold should be in range [0-65,535] \n" +
        "The (\"Credits Number\" + \"Peer Credits Threshold\") should be in range [0-65.535]", inst);
    }

    if ( (inst.L2CAPCOCpsmId < 0)      ||
         (inst.L2CAPCOCpsmId > 0xffff) ||
         (inst.L2CAPCOCpsmId & 0x100)  ||
         !(inst.L2CAPCOCpsmId & 0x1) )
    {
        validation.logError("All PSM values shall have the least significant bit of the most significant octet " +
        "equal to 0 and the least significant bit of all other octets equal to 1.", inst);
    }
}

/*
 *  ======== exports ========
 *  Export the BLE L2CAP manager Configuration
 */
exports = {
    config: config,
    validate: validate
};
