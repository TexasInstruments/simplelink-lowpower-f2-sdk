/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ANSIX936KDF.syscfg.js ========
 *  ANSI X9.63 Key Derivation Function (KDF)
 */

"use strict";

/* get Common /ti/drivers utility functions */
let Common = system.getScript("/ti/drivers/Common.js");

/* generic configuration parameters for ANSIX936KDF instances */
let config = []; /* nothing (yet) beyond generic driver configs */

/*
 *  ======== base ========
 *  Define the base ANSIX936KDF properties and methods
 */
let base = {
    displayName         : "ANSIX936KDF",
    description         : "ANSI X9.36 Key Derivation Function Driver",
    alwaysShowLongDescription : true,
    longDescription     : `
The [__ANSIX936KDF driver__][1] utilizes the SHA-256 hash
function to derive a key from a shared secret value and optional shared
info. See ANSI X9.63-2011 standard for more information.

* [Usage Synopsis][2]
* [Examples][3]

[1]: /drivers/doxygen/html/_a_n_s_i_x936_k_d_f_8h.html#details "C API reference"
[2]: /drivers/doxygen/html/_a_n_s_i_x936_k_d_f_8h.html#ti_drivers_ANSIX936KDF_Synopsis "Basic C usage summary"
[3]: /drivers/doxygen/html/_a_n_s_i_x936_k_d_f_8h.html#ti_drivers_ANSIX936KDF_Examples "C usage examples"
[4]: /drivers/syscfg/html/ConfigDoc.html#ANSIX936KDF_Configuration_Options "Configuration options reference"
`,
    config              : Common.addNameConfig(config, "/ti/drivers/ansix936kdf", "CONFIG_ANSIX936KDF_"),
    defaultInstanceName : "CONFIG_ANSIX936KDF_"
};

/* extend the base exports to include family-specific content */
/* For now, there is only one generic implementation. We may get dedicated hardware
 * backed implementations later.
 */
let devANSIX936KDF = system.getScript("/ti/drivers/ansix936kdf/ANSIX936KDFXX");

exports = devANSIX936KDF.extend(base);
