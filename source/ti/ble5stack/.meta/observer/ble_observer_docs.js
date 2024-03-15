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
 *  ======== ble_observer_docs.js ========
 */

"use strict";

// Get common Script
const Common = system.getScript("/ti/ble5stack/ble_common.js");

// Long description for the scanPHY configuration parameter
const scanPHYLongDescription = `The primary PHY that is used for scanning.\n
__Default__: 1M\n
Options |
--- |
1 M |
Coded |
`

// Long description for the scanType configuration parameter
const scanTypeLongDescription = `The Scan Type parameter specifies the type of\
scan to perform.\n
__Default__: Active\n
Options | Description
--- | ---
Active | Active Scanning. Scan request PDUs may be sent
Passive | Passive Scanning. No scan request PDUs shall be sent
`

// Long description for the scanInt configuration parameter
const scanIntLongDescription = `Time interval from when the Controller started\
its last scan until it begins the subsequent scan on the primary advertising\
channel.\n
__Default__: 500 ms\n
__Range__:  ${Common.connParamsRanges.scanIntMinValue} ms to \
${Common.connParamsRanges.scanIntMaxValue} ms\n
__Note__: Scan Interval shall be greater than or equal to Scan Window.\n
Scan Duration shall be greater than Scan Interval`

// Long description for the scanWin configuration parameter
const scanWinLongDescription = `Duration of the scan on the primary\
advertising channel.\n
__Default__: 500 ms\n
__Range__: ${Common.connParamsRanges.scanWinMinValue} ms to \
${Common.connParamsRanges.scanWinMaxValue} ms\n
__Note__: Scan Window shall be less than or equal to Scan Interval.`

// Long description for the scanDuration configuration parameter
const scanDurationLongDescription = `Scan Duration\n
__Default__: 1000 ms\n
__Range__: ${Common.connParamsRanges.scanDurationMinValue} ms to \
${Common.connParamsRanges.scanDurationMaxValue} ms\n
__Note__: Scan Duration shall be greater than Scan Interval.`

// Long description for the advRptFields configuration parameter
const advRptFieldsLongDescription = `Select which fields of an advertising\
report will be stored in the AdvRptList.\n
__Default__: Address Type, Address\n
Options |
--- |
Event Type |
Address Type |
Address |
Primary PHY |
Secondary PHY |
Advertise Set Id |
TX Power |
RSSI |
dirAddrType |
dirAddress |
prdAdvInterval |
Data Length |`

// Long description for the disDevBySerUuid configuration parameter
const disDevBySerUuidLongDescription = `When set to TRUE (checked), the \
application will filter and connect to peripheral devices with a desired \
service UUID. \n
__Note__: By defult, the service UUID is the TI Simple Profile UUID. \n
__Default__: False (unchecked)`

// Long description for the advReportChanNum configuration parameter
const advReportChanNumLongDescription = `When enabled, the channel number which \
the scanner received the advertising packet on, will be part of the advertising report.\n
__Note__: This feature is only available for legacy advertising packet\n
__Default__: False (unchecked)`

// Long description for the flyPolicy configuration parameter
const fltPolicyLongDescription = `Choose the GAP scanner filter policy\n
__Default__: SCAN_FLT_POLICY_ALL\n
Options | Description
--- | ---
Accept all | Accept all advertising packets except directed advertising packets not addressed to this device
Accept only from acceptlist | Accept only advertising packets from devices where the advertiser's address \
is in the acceptlist. Directed advertising packets which are not addressed \
to this device shall be ignored
Accept RPA | Accept all advertising packets except directed advertising packets where \
the TargetA does not addrress this device. Note that directed advdertising \
packets where the TargetA is a resolvable private address that cannot be \
resolved are also accepted
Accept only from acceptlist and RPA | Accept all advertising packets except advertising packets where the \
advertiser's identity address is not in the acceptlist and directed \
advertising packets where the TargetA does not address this device. \
Note that directed advertising packets where the TargetA is a resolvable \
private address that cannot be resolved are also accepted`

// Long description for the fltPduType configuration parameter
const fltPduTypeLongDescription = `Choose the GAP scanner filter PDU type\n
__Default__: Connectable only & Complete only \n
Options | Description
--- | ---
Non-connectable only | Mutually exclusive with Connectable only
Connectable only | Mutually exclusive with Non-connectable only
Non-scannable only | Mutually exclusive with Scannable only
Scannable only | Mutually exclusive with Non-scannable only
Undirected only | Mutually exclusive with Directed only
Directed only | Mutually exclusive with Undirected only
Advertisement only | Mutually exclusive with Scan Response only
Scan Response only | Mutually exclusive with Advertisement only
Extended only | Mutually exclusive with Legacy only
Legacy only | Mutually exclusive with Extended only
Truncated only | Mutually exclusive with Complete only
Complete only | Mutually exclusive with Truncated only`

// Long description for the fltMinRssi configuration parameter
const fltMinRssiLongDescription = `Filter by RSSI values. \n
Only packets received with the specified RSSI or above will be reported.\n
__Default__: All \n
Options | Minimum RSSI Value
--- | ---
All | -128
None | 127`

// Long description for the fltDiscMode configuration parameter
const fltDiscModeLongDescription = `Choose the GAP scanner discoverable mode filter\n
__Default__: Disable \n
Options |
--- |
Non-discoverable |
General |
Limited |
General or Limited |
Disable |`

// Long description for the scanPeriod configuration parameter
const scanPeriodLongDescription = `Ignored if duration is zero. 1.28 sec unit.\n
__Default__: 0 \n
__Range__:  ${Common.connParamsRanges.scanPeriodMinValue} sec to \
${Common.connParamsRanges.scanPeriodMaxValue} sec, where 0 is continuously scanning.\n`

// Long description for the scanPeriod configuration parameter
const maxNumAdvReportLongDescription = `If non-zero, the list of advertising reports (the number of which is up to maxNumReport) will be generated.\n
__Default__: 0 \n
__Range__:  0 to 255 \n`

 // Exports the long descriptions for each configurable
exports = {
    scanPHYLongDescription: scanPHYLongDescription,
    scanTypeLongDescription: scanTypeLongDescription,
    scanIntLongDescription: scanIntLongDescription,
    scanWinLongDescription: scanWinLongDescription,
    fltPolicyLongDescription: fltPolicyLongDescription,
    fltPduTypeLongDescription: fltPduTypeLongDescription,
    fltMinRssiLongDescription: fltMinRssiLongDescription,
    fltDiscModeLongDescription: fltDiscModeLongDescription,
    scanDurationLongDescription: scanDurationLongDescription,
    scanPeriodLongDescription: scanPeriodLongDescription,
    advRptFieldsLongDescription: advRptFieldsLongDescription,
    disDevBySerUuidLongDescription: disDevBySerUuidLongDescription,
    advReportChanNumLongDescription: advReportChanNumLongDescription,
    maxNumAdvReportLongDescription: maxNumAdvReportLongDescription
};