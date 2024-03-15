/*
 * Copyright (c) 2018-2022 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ble_l2cap_coc_docs.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ble5stack/ble_common.js");

// Long description for the L2CAPCOCpsmId configuration parameter
const L2CAPCOCpsmIdLongDescription = `PSM ID is the Protocol/Service Multiplexer ID\
for L2CAP Connection Oriented Channel. \
For more information, refer to the User Guide \n
__Default__: 1\n`

// Long description for the L2CAPCOCmaxMTU configuration parameter
const L2CAPCOCmaxMTULongDescription = `The maximum size of payload data, in octets, that the upper \
layer entity can accept (that is, the MTU corresponds to the maximum SDU (Service Data Unit) size). \
Note: This is different than ATT_MTU.\n
For more information, refer to the User Guide \n
__Default__: 1400\n`

// Long description for the L2CAPCOCmaxMPS configuration parameter
const L2CAPCOCmaxMPSLongDescription = `The maximum size of payload data in octets that the L2CAP layer \
entity can accept (that is, the MPS corresponds to the maximum PDU payload size).\n
For more information, refer to the User Guide \n
__Default__: 1024\n`

// Long description for the L2CAPCOCcredits configuration parameter
const L2CAPCOCcreditsLongDescription = `The number of LE-frames that the device can receive. \
Credits may range between 1 and 65535, and are used as a flow control mechanism between devices. \n
For more information, refer to the User Guide \n
__Default__: 65535\n`

// Long description for the L2CAPCOCcreditsThreshold configuration parameter
const L2CAPCOCcreditsThresholdLongDescription = `A device will send an L2CAP_FLOW_CONTROL_CREDIT_IND packet when
it is capable of receiving additional frames, and the peer device has reached the credit threshold. \n
For more information, refer to the User Guide \n
__Default__: 5\n`

// Long description for the L2CAPCOCConnRoleLongDescription configuration parameter
const L2CAPCOCConnRoleLongDescription = `For COC Initiator option, the device will initiate the Connection Oriented \n
Channel establishment request to the peer device. (by sending the LCAP_ConnectReq) \n
For COC Reponder option, the device will wait for the COC establishment request from the peer device\n
__Default__: L2CAPCOC_RESPONDER\n`

// Long description for the L2CAPCOCPeerPsmIdLongDescription configuration parameter
const L2CAPCOCPeerPsmIdLongDescription = `PSM ID of the peer device, to be used in the COC establishment request\n
__Default__: 1\n`

// Long description for the L2CAPConfig configuration parameter
const L2CAPConfigLongDescription = `Configurations in this section are valid only for l2cap feature\n`


 // Exports the long descriptions for each configurable
 exports = {
    L2CAPCOCpsmIdLongDescription: L2CAPCOCpsmIdLongDescription,
    L2CAPCOCmaxMTULongDescription: L2CAPCOCmaxMTULongDescription,
    L2CAPCOCmaxMPSLongDescription: L2CAPCOCmaxMPSLongDescription,
    L2CAPCOCcreditsLongDescription: L2CAPCOCcreditsLongDescription,
    L2CAPCOCcreditsThresholdLongDescription: L2CAPCOCcreditsThresholdLongDescription,
    L2CAPCOCConnRoleLongDescription: L2CAPCOCConnRoleLongDescription,
    L2CAPCOCPeerPsmIdLongDescription: L2CAPCOCPeerPsmIdLongDescription
};