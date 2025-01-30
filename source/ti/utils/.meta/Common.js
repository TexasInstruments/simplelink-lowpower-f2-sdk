/*
 * Copyright (c) 2018-2024 Texas Instruments Incorporated - http://www.ti.com
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
 */

/*
 *  ======== Common.js ========
 */

exports = {
    isCName: isCName,              /* validate name is C identifier */
    device2Family: device2Family,  /* convert device object to device family */
    autoForceModules: autoForceModules
};

/*!
 *  ======== device2Family ========
 *  Map a pinmux device object to a device family string
 *
 *  @param device   - a pinmux device object (system.deviceData)
 *
 *  @returns String - the name of a device family that's used to
 *                    create family specific-implementation module
 *                    names.  Returns null, in the event of an
 *                    unsupported device.
 */
function device2Family(device)
{
    /* device.deviceId prefix -> device family name */
    let DEV2FAMILY = [
        {prefix: "CC2653",   family: "CC26X4"},
        {prefix: "CC13.4",   family: "CC26X4"},
        {prefix: "CC26.4",   family: "CC26X4"},
        {prefix: "CC13.2",   family: "CC26X2"},
        {prefix: "CC26.2",   family: "CC26X2"},
        {prefix: "CC13",     family: "CC26XX"},
        {prefix: "CC26",     family: "CC26XX"},
    ];

    /* deviceId is the directory name within the pinmux/deviceData */
    let deviceId = device.deviceId;

    for (let i = 0; i < DEV2FAMILY.length; i++) {
        let d2f = DEV2FAMILY[i];

        if (deviceId.match(d2f.prefix)) {
            return (d2f.family);
        }
    }
    return (null);
}

/*
 *  ======== isCName ========
 *  Determine if specified id is either empty or a valid C identifier
 *
 *  @param id  - String that may/may not be a valid C identifier
 *
 *  @returns true if id is a valid C identifier OR is the empty
 *           string; otherwise false.
 */
function isCName(id)
{
    if ((id != null && id.match(/^[a-zA-Z_][0-9a-zA-Z_]*$/) != null)
            || id == '') { /* '' is a special value that means "default" */
        return true;
    }
    return false;
}

/*
 *  ======== autoForceModules ========
 *  Returns an implementation of a module's modules method that
 *  forces the addition of the specified modules
 *
 *  @param kwargs An array of module name strings.
 *
 *  @return An array with module instance objects
 *
 *  Example:
 *     modules: Common.autoForceModules(["ti/drivers/Board", "JSON"])
 */
function autoForceModules(kwargs)
{
    return (function(){

        let modArray = [];

        if (kwargs == undefined || kwargs == null || !Array.isArray(kwargs)) {
            console.log("Common.js:autoForceModules('kwargs'): 'kwargs' invalid!");
            return (modArray);
        }

        for (let args = kwargs.length - 1; args >= 0; args--) {
            let modPath = kwargs[args];
            if (modPath.indexOf('/') == -1) {
                modPath = "/ti/utils/" + modPath;
            }
            modArray.push({
                name      : modPath.substring(modPath.lastIndexOf('/') + 1),
                moduleName: modPath,
                hidden    : true
            });
        }

        return modArray;
    });
}
