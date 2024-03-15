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
 *  ======== ti154stack_toad_config_docs.js ========
 */

"use strict";

const toadModule = {
    description: "Configure settings for Turbo OAD",
    longDescription: `
The Turbo OAD module enables the support of using delta software updates in \
the OAD protocol. Delta updates are software updates that contain only the \
changed or updated content in the new software image. This results on average \
around 5x to 10x faster software updates for typical small or medium sized \
software updates.
`
};

const enabled = {
    description: "Configures if the application is built with Turbo OAD",
    longDescription: `
Configures if the application is built with Turbo OAD Support. Applications \
with Turbo OAD enabled allow the use of software upgrades using delta images \
in addition to supporting regular OAD images.
`
};

const oldImgPath = {
    description: `Configures the path to the old image for delta encoding the \
new image`,
    longDescription: `
Configures the path to the old image for delta encoding the new image. If no \
path is provided, no delta image will be created but the image will be able \
to support decoding delta updates.
`
};

const blockCacheSize = {
    description: `Configures the amount of RAM allocated in bytes for storing \
delta encoded OAD blocks`,
    longDescription: `
Configures the amount of RAM allocated in bytes for storing OAD blocks before \
they are decoded by Turbo OAD. The larger this value is, the less likely the \
OAD will need to temporarily pause itself during the decoding process. The \
smaller this value is, the quicker this buffer will fill up and require the \
OAD to pause requesting blocks until space is free in the block cache to \
receive additional blocks.
\n\
__Default__: 1024
\n\
__Range__: 0 and 81920. Note that this value must be greater than \
OAD Block Size.
`
};

const writeBufferSize = {
    description: `Configures the amount of RAM allocated in bytes for storing \
decoded delta blocks`,
    longDescription: `
Configures the amount of RAM allocated in bytes for storing OAD blocks after \
they are decoded by Turbo OAD. The larger this value is, the less likely \
this buffer will fill up when decoding a delta block. The smaller this value \
is, the quicker this buffer will fill up and require more frequently to \
flush the decoded image data to flash memory, thereby increasing the duration \
of the decoding process.
\n\
__Default__: 4096
\n\
__Range__: 0 and 81920
`
};

exports = {
    toadModule: toadModule,
    enabled: enabled,
    oldImgPath: oldImgPath,
    blockCacheSize: blockCacheSize,
    writeBufferSize: writeBufferSize
};
