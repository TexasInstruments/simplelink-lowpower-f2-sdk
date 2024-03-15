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
 *  ======== zigbee_bdb_config_docs.js ========
 */

"use strict";

const bdbReportingMaxAnalogAttrSizeLongDescription = 'This is the size of the reportable change value in bytes, which will depend' +
                                            ' on the type of the attribute being reported. For more information about this' +
                                            ' field, look in the Zigbee Cluster Library v7 specification, section 2.5.7.1.7';

const bdbReportableChangeLongDescription = 'The reportable change field SHALL contain the minimum change to the' +
                                           ' attribute that will result in a report being issued. For more information,' +
                                           ' see the Zigbee Cluster Library v7 specification, section 2.5.7.1.7.';

const bdbMinimumReportingIntervalLongDescription = 'The minimum reporting interval field is 16 bits in length and SHALL' +
                                            ' contain the minimum interval, in seconds, between issuing reports of the' +
                                            ' specified attribute. If this value is set to 0x0000, then there is no minimum limit.';

const bdbMaximumReportingIntervalLongDescription = 'The maximum reporting interval field is 16 bits in length and SHALL' +
                                            ' contain the maximum interval, in seconds, between issuing reports of the' +
                                            ' specified attribute. If this value is set to 0xffff, then the device SHALL not' +
                                            ' issue reports for the specified attribute. If this value is set to 0x0000, and' +
                                            ' the minimum reporting interval field does not equal 0xffff there SHALL be no' +
                                            ' periodic reporting, but change based reporting SHALL still be operational.' +
                                            ' If this value is set to 0x0000 and the Minimum Reporting Interval Field equals' +
                                            ' 0xffff, then the device SHALL revert to its default reporting configuration. The ' +
                                            ' reportable change field, if present, SHALL be set to zero.';

// Exports the long descriptions for each configurable in BDB Settings
exports = {
  bdbReportingMaxAnalogAttrSizeLongDescription: bdbReportingMaxAnalogAttrSizeLongDescription,
  bdbReportableChangeLongDescription: bdbReportableChangeLongDescription,
  bdbMinimumReportingIntervalLongDescription: bdbMinimumReportingIntervalLongDescription,
  bdbMaximumReportingIntervalLongDescription: bdbMaximumReportingIntervalLongDescription,
};
