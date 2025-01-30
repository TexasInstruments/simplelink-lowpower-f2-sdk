/*
 * Copyright (c) 2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== mcuboot.syscfg.js ========
 */

const MCUbootTemplate = system.getScript("/ti/common/mcuboot/mcubootTemplate.syscfg.js");
const mcubootSettings = MCUbootTemplate.mcubootSettings;
const deviceGroup = MCUbootTemplate.deviceGroup;
const externalFlashSectorSize = MCUbootTemplate.externalFlashSectorSize;
const externalFlashSize = MCUbootTemplate.externalFlashSize;
const externalFlashBase = MCUbootTemplate.externalFlashBase;

const Common = system.getScript("/ti/drivers/Common.js");
let logError = Common.logError;
let logWarning  = Common.logWarning;

const moduleDesc = `
This section is used to generate config files for MCUboot.`;

let base = {
    displayName: "MCUboot",
    description: "MCUboot Configuration",
    longDescription: moduleDesc,
    alwaysShowLongDescription : true,
    maxInstances: 1,
    templates : {
        "/ti/common/templates/flash_map_backend.h.xdt"  : "/ti/common/templates/flash_map_backend.h.xdt",
        "/ti/common/templates/mcuboot_config.h.xdt"     : "/ti/common/templates/mcuboot_config.h.xdt",
        "/ti/utils/build/GenMap.cmd.xdt":{modName: "/ti/common/mcuboot", getLinkerDefs: getLinkerDefs}
    },
    moduleStatic: {
        validate: validate,
        config: [
            {
                name: "enableMCUbootCodeGeneration",
                displayName: "Enable SysConfig for MCUboot Configuration",
                description: "Enables or disables generation of mcuboot_config.h and flash_map_backend.h.",
                hidden: true,
                default: false,
                onChange: (inst, ui) => {

                    const configs = Object.keys(ui).filter((key) => (!key.includes("$")
                        && key !== "MCUbootTemplate"));

                    // Hide module configs if code generation is disabled
                    configs.forEach((cfgName) => {
                        if (cfgName === "enableCodeGeneration") {
                            const readOnly = "MCUboot is not available.";

                            ui[cfgName].hidden = inst.enableCodeGeneration;
                            ui[cfgName].readOnly = inst.enableCodeGeneration ? false : readOnly;
                        }
                        else {
                            const cfg = inst.$module.$configByName[cfgName];
                            // Reset config visibility if code generation is enabled
                            ui[cfgName].hidden = inst.enableCodeGeneration ? cfg.hidden : true;
                            inst[cfgName] = cfg.default;
                        }
                    });
                }
            },
            {
                name        : "tzEnabled",
                displayName : "Enable TrustZone",
                description : "Enable Mcuboot to store 2 images - secure and non secure image.",
                default     : false,
                readOnly    : mcubootSettings["tzEnabled"]["readOnly"],
                hidden      : mcubootSettings["tzEnabled"]["hidden"],
                onChange    : changeTzEnable
            },
            {
                name: "mode",
                displayName: "Upgrade Mode",
                longDescription: "TI supports two upgrade methods - Overwrite and XIP ",
                default     : "xip",
                onChange    : changeMode,
                options     :
                [
                    {
                        name: "overwrite",
                        displayName : "Overwrite",
                        longDescription: "Mcuboot only runs an image from the primary slot. " +
                                     "Mcuboot will copy the image from the secondary to the primary slot and erase the secondary slot afterwards."
                    },
                    {
                        name: "xip",
                        displayName : "XIP",
                        longDescription: "Mcuboot runs an image directly from either the primary or the secondary slot, without moving and or copying it into the primary slot."
                    }
                ],
                readOnly: false,
                hidden: false
            },
            {
                name        : "externalFlash",
                displayName : "Enable External Flash",
                description : "Enable Mcuboot to search for valid images in external Flash",
                default     : false,
                hidden      : true,
                onChange    : changeExternalFlash
            },
            {
                name        : "enableEncryptedImage",
                displayName : "Upgrade Using Encrypted Images",
                description : "Enable Mcuboot to search for valid encrypted images",
                default     : false,
                hidden      : true
            },
            {
                name        : "antiRollbackProtection",
                displayName : "Anti Rollback Protection",
                description : "Enable hardware-based downgrade prevention feature by using security counters",
                default     : false,
                readOnly    : mcubootSettings["antiRollbackProtection"]["readOnly"],
                hidden      : mcubootSettings["antiRollbackProtection"]["hidden"]
            },
            {
                displayName: "Bootloader",
                description: "Bootloader Configurations",
                collapsed: true,
                config  : [
                    {
                        name        : "bootloaderBaseAddress",
                        displayName : "Base",
                        description : "Base Address of Mcuboot",
                        displayFormat: { radix: "hex", bitSize: 32 },
                        default     : mcubootSettings["bootloader"]["base"],
                        readOnly    : false,
                        hidden      : false
                    },
                    {
                        name        : "bootloaderSize",
                        displayName : "Size",
                        description : "Size of Mcuboot",
                        displayFormat: { radix: "hex", bitSize: 32 },
                        default     : mcubootSettings["bootloader"]["size"],
                        readOnly    : false,
                        hidden      : false
                    }
                ]
            },
            {
                name: "image1",
                displayName: "Image 1",
                description: "Image 1 (Secure image if TrustZone is enabled).",
                collapsed: true,
                config  : [
                    {
                        displayName: "Primary Image",
                        description: "Image stored in primary slot",
                        collapsed: false,
                        config: [
                            {
                                name        : "primaryBase1",
                                displayName : "Base Address",
                                description : "Base Address of Primary Image",
                                displayFormat: { radix: "hex", bitSize: 32 },
                                default     : mcubootSettings["image1"]["primaryBase"],
                                readOnly    : false,
                                hidden      : false
                            },
                            {
                                name        : "primarySize1",
                                displayName : "Image Size",
                                description : "Size of Primary Image",
                                displayFormat: { radix: "hex", bitSize: 32 },
                                default     : mcubootSettings["image1"]["primarySize"],
                                readOnly    : false,
                                hidden      : false
                            }
                        ]
                    },
                    {
                        displayName: "Secondary Image",
                        description: "Image stored in secondary slot",
                        collapsed: false,
                        config: [
                            {
                                name        : "secondaryBase1",
                                displayName : "Base Address",
                                description : "Base Address of Secondary Image",
                                displayFormat: { radix: "hex", bitSize: 32 },
                                default     : mcubootSettings["image1"]["secondaryBase"],
                                readOnly    : false,
                                hidden      : false
                            },
                            {
                                name        : "secondarySize1",
                                displayName : "Image Size",
                                description : "Size of Secondary Image",
                                displayFormat: { radix: "hex", bitSize: 32 },
                                default     : mcubootSettings["image1"]["secondarySize"],
                                readOnly    : false,
                                hidden      : false
                            }
                        ]
                    }
                ]
            },
            {
                name: "image2",
                displayName: "Image 2",
                description: "Non-secure image",
                collapsed: true,
                config  : [
                    {
                        displayName: "Primary Image",
                        description: "Image stored in primary slot",
                        collapsed: false,
                        config: [
                            {
                                name        : "primaryBase2",
                                displayName : "Base",
                                description : "Base of Primary Image",
                                displayFormat: { radix: "hex", bitSize: 32 },
                                default     : mcubootSettings["image2"]["primaryBase"],
                                readOnly    : true,
                                hidden      : true
                            },
                            {
                                name        : "primarySize2",
                                displayName : "Size",
                                description : "Size of Primary Image",
                                displayFormat: { radix: "hex", bitSize: 32 },
                                default     : mcubootSettings["image2"]["primarySize"],
                                readOnly    : false,
                                hidden      : true
                            }
                        ]
                    },
                    {
                        displayName: "Secondary Image",
                        description: "Image stored in secondary slot",
                        collapsed: false,
                        config: [
                            {
                                name        : "secondaryBase2",
                                displayName : "Base",
                                description : "Base of Secondary Image",
                                displayFormat: { radix: "hex", bitSize: 32 },
                                default     : mcubootSettings["image2"]["secondaryBase"],
                                readOnly    : false,
                                hidden      : true
                            },
                            {
                                name        : "secondarySize2",
                                displayName : "Size",
                                description : "Size of Secondary Image",
                                displayFormat: { radix: "hex", bitSize: 32 },
                                default     : mcubootSettings["image2"]["secondarySize"],
                                readOnly    : false,
                                hidden      : true
                            }
                        ]
                    }
                ]
            }
        ]
    },
    initPriority: 0
};

function changeExternalFlash(inst, ui)
{
    if(inst.externalFlash)
    {
        inst.secondaryBase1 = externalFlashBase;
    }
    else
    {
        if(!inst.tzEnabled)
        {
            inst.secondaryBase1 = mcubootSettings["image1"]["secondaryBase"];
        }
        else
        {
            inst.secondaryBase1 = 0x86800;
        }
    }
}

function changeMode(inst, ui)
{
    ui.externalFlash.hidden = !ui.externalFlash.hidden;
    inst.externalFlash = false;

    ui.enableEncryptedImage.hidden = !ui.enableEncryptedImage.hidden && mcubootSettings["enableEncryptedImage"]["enabled"];
    inst.enableEncryptedImage = false;
}

function changeTzEnable(inst, ui)
{
    // change mode to overwrite
    inst.mode = 'overwrite';
    ui.mode.readOnly = !ui.mode.readOnly;
    ui.externalFlash.hidden = false;

    //change anti rollback protection settings
    inst.antiRollbackProtection = false;
    ui.antiRollbackProtection.readOnly = !ui.antiRollbackProtection.readOnly;
    ui.antiRollbackProtection.hidden = !ui.antiRollbackProtection.hidden;

    // set hidden properties for second image slots
    ui.primaryBase2.hidden = !ui.primaryBase2.hidden;
    ui.primarySize2.hidden = !ui.primarySize2.hidden;
    ui.secondaryBase2.hidden = !ui.secondaryBase2.hidden;
    ui.secondarySize2.hidden = !ui.secondarySize2.hidden;

    //set readonly properties for first image slots
    ui.primaryBase1.readOnly = !ui.primaryBase1.readOnly;
    ui.primarySize1.readOnly = !ui.primarySize1.readOnly;
    // ui.secondaryBase1.readOnly = !ui.secondaryBase1.readOnly;
    ui.secondarySize1.readOnly = !ui.secondarySize1.readOnly;

    //change readonly properties for bootloader
    ui.bootloaderBaseAddress.readOnly = !ui.bootloaderBaseAddress.readOnly;
    ui.bootloaderSize.readOnly = !ui.bootloaderSize.readOnly;

    /*  update enable encrypted image settings - any device that supports tzEnable will support encrypted images as well,
    therefore no need to reference template to see if encrypted images are enabled.*/
    ui.enableEncryptedImage.hidden = false;
    inst.enableEncryptedImage = false;

    // if tz_enable is selected
    if(inst.tzEnabled)
    {
        // set bootloader size and base and make them uneditable
        inst.bootloaderBaseAddress = 0x800;
        inst.bootloaderSize = 0x6000;

        // set first image slots
        inst.primaryBase1 = 0x0000d000;
        inst.primarySize1 = 0x0002b000;
        inst.secondaryBase1 = 0x00086800;
        inst.secondarySize1 = 0x0002b000;

        // set second image slots
        inst.primaryBase2 = mcubootSettings["image2"]["primaryBase"];
        inst.primarySize2 = mcubootSettings["image2"]["primarySize"];
        inst.secondaryBase2 = mcubootSettings["image2"]["secondaryBase"];
        inst.secondarySize2 = mcubootSettings["image2"]["secondarySize"];
    }
    // else revert to default
    else
    {
        inst.bootloaderBaseAddress = mcubootSettings["bootloader"]["base"];
        inst.bootloaderSize = mcubootSettings["bootloader"]["size"];

        inst.primaryBase1 = mcubootSettings["image1"]["primaryBase"];
        inst.primarySize1 = mcubootSettings["image1"]["primarySize"];
        inst.secondaryBase1 = mcubootSettings["image1"]["secondaryBase"];
        inst.secondarySize1 = mcubootSettings["image1"]["secondarySize"];

        inst.primaryBase2 = mcubootSettings["image2"]["primaryBase"];
        inst.primarySize2 = mcubootSettings["image2"]["primarySize"];
        inst.secondaryBase2 = mcubootSettings["image2"]["secondaryBase"];
        inst.secondarySize2 = mcubootSettings["image2"]["secondarySize"];
    }
    
}

function validate(inst, validation) {

    // throw warning if this enabled
    if(inst.enableEncryptedImage === true)
    {
        logWarning(validation, inst, "enableEncryptedImage",
                 "Default Bootloader size may need to be increased if Encrypted Images are enabled ");
    }

    // if instance of external flash is true, flash base must be 0x800
    if(inst.tzEnabled && inst.bootloaderBaseAddress != 0x800 )
    {
        logError(validation, inst, "bootloaderBaseAddress",
                 "Value must be 0x800 if Trust Zones are enabled");
    }

    // if size of primary image is not the same as secondary image throw error
    if(inst.primarySize1 !== inst.secondarySize1 && inst.mode == 'overwrite')
    {
        logError(validation, inst, "primarySize1",
                 "Primary image must be the same size as secondary image");
        logError(validation, inst, "secondarySize1",
                 "Secondary image must be the same size as primary image");
    }

    // check if flash layout based on device group
    if(deviceGroup === "DeviceGroup_CC13X2X7_26X2X7")
    {
        // if bootloader starts less than secondary image base + secondary image size, throw error
        if(inst.secondaryBase1 + inst.secondarySize1 > inst.bootloaderBaseAddress)
        {
            logError(validation, inst, "secondaryBase1",
                    "The base of the bootloader must be greater than or equal to the base of the secondary image plus the size of the secondary image.");
            logError(validation, inst, "secondarySize1",
                    "The base of the bootloader image must be greater than or equal to the base of the secondary image plus the size of the secondary image.");
            logError(validation, inst, "bootloaderBaseAddress",
                    "The base of the bootloader image must be greater than or equal to the base of the secondary image plus the size of the secondary image.");
        }

    }
    else
    {
        // if primary image starts less than bootloader base + bootloader size, throw error
        if(inst.bootloaderBaseAddress + inst.bootloaderSize > inst.primaryBase1)
        {
            logError(validation, inst, "bootloaderBaseAddress",
                    "The base of the primary image must be greater than or equal to the base of the bootloader plus the size of the bootloader.");
            logError(validation, inst, "bootloaderSize",
                    "The base of the primary image must be greater than or equal to the base of the bootloader plus the size of the bootloader.");
            logError(validation, inst, "primaryBase1",
                    "The base of the primary image must be greater than or equal to the base of the bootloader plus the size of the bootloader.");
        }
    }

    // check if the start address for bootloader and images are sector aligned
    if(inst.primaryBase1 % mcubootSettings["alignment"]["sectorSize"] !== 0)
    {
        logError(validation, inst, "primaryBase1",
                 "The base of the primary image must be sector aligned.");
    }

    if(inst.secondaryBase1 % mcubootSettings["alignment"]["sectorSize"] !== 0)
    {
        logError(validation, inst, "secondaryBase1",
                "The base of the secondary image must be sector aligned.");
    }

    if(inst.bootloaderBaseAddress % mcubootSettings["alignment"]["sectorSize"] !== 0)
    {
        logError(validation, inst, "bootloaderBaseAddress",
                 "The base of the bootloader must be sector aligned.");
    }

    //check if the size of the bootloader and images are a multiple of the sector size
    if(inst.primarySize1 % mcubootSettings["alignment"]["sectorSize"] !== 0)
    {
        logError(validation, inst, "primarySize1",
                 "The size of the primary image must be a multiple of the sector size.");
    }

    // sector size is different for secondary image if external flash is enabled
    if(inst.externalFlash === true)
    {
        if(inst.secondarySize1 % externalFlashSectorSize !== 0)
        {
            logError(validation, inst, "secondarySize1",
                    "The size of the secondary image must be a multiple of the sector size.");
        }
    }
    if(inst.secondarySize1 % mcubootSettings["alignment"]["sectorSize"] !== 0)
    {
        logError(validation, inst, "secondarySize1",
                "The size of the secondary image must be a multiple of the sector size.");
    }

    if(inst.bootloaderSize % mcubootSettings["alignment"]["sectorSize"] !== 0)
    {
        logError(validation, inst, "bootloaderSize",
                 "The size of the bootloader must be a multiple of the sector size.");
    }


    // if image layout overlaps each other, throw error
    if(inst.secondaryBase1 >= inst.primaryBase1 && inst.primaryBase1 + inst.primarySize1 > inst.secondaryBase1 && !inst.externalFlash)
    {
        logError(validation, inst, "primaryBase1",
                 "Primary image and secondary image overlap each other.");
        logError(validation, inst, "primarySize1",
                 "Primary image and secondary image overlap each other.");
        logError(validation, inst, "secondaryBase1",
                 "Primary image and secondary image overlap each other.");
        logError(validation, inst, "secondarySize1",
                 "Primary image and secondary image overlap each other.");
    }
    
    else if(inst.primaryBase1 >= inst.secondaryBase1 && inst.secondaryBase1 + inst.secondarySize1 > inst.primaryBase1 && !inst.externalFlash)
    {
        logError(validation, inst, "primaryBase1",
                    "Primary image and secondary image overlap each other.");
        logError(validation, inst, "primarySize1",
                    "Primary image and secondary image overlap each other.");
        logError(validation, inst, "secondaryBase1",
                    "Primary image and secondary image overlap each other.");
        logError(validation, inst, "secondarySize1",
                    "Primary image and secondary image overlap each other.");
    }

    //check if all sizes are within flash boundary (for all conditions except tz_enabled)
    if(inst.bootloaderSize > mcubootSettings["alignment"]["flashBoundary"])
    {
        logError(validation, inst, "bootloaderSize",
                "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and boundaries are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
    }
    if(inst.primarySize1 > mcubootSettings["alignment"]["flashBoundary"])
    {
        logError(validation, inst, "primarySize1",
                "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and boundaries are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
    }
    if(inst.externalFlash === true)
    {
        if(inst.secondarySize1 > externalFlashSize)
        {
            logError(validation, inst, "secondarySize1",
                    "MCUboot layout exceeds the external flash boundaries. Make sure all image sizes and boundaries are less than 0x" + externalFlashSectorSize);
        }
    }
    else
    {
        if(inst.secondarySize1 > mcubootSettings["alignment"]["flashBoundary"])
        {
            logError(validation, inst, "secondarySize1",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and boundaries are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
        }
    }
    
    //check if all base addresses are within flash boundary (for all conditions except tz_enabled)
    if(inst.bootloaderBaseAddress > mcubootSettings["alignment"]["flashBoundary"])
    {
        logError(validation, inst, "bootloaderBaseAddress",
                "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and boundaries are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
    }
    if(inst.primaryBase1 > mcubootSettings["alignment"]["flashBoundary"])
    {
        logError(validation, inst, "primaryBase1",
                "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and boundaries are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
    }
    if(inst.externalFlash === true)
    {
        if(inst.secondaryBase1 > externalFlashSize)
        {
            logError(validation, inst, "secondaryBase1",
                    "MCUboot layout exceeds the external flash boundaries. Make sure all image sizes and boundaries are less than 0x" + externalFlashSectorSize);
        }
    }
    else
    {
        if(inst.secondaryBase1 > mcubootSettings["alignment"]["flashBoundary"])
        {
            logError(validation, inst, "secondaryBase1",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and boundaries are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
        }
    }

    // check if all base and size addresses are within flash boundaries
    if(inst.bootloaderBase + inst.bootloaderSize >  mcubootSettings["alignment"]["flashBoundary"] || inst.primaryBase1 + inst.primarySize1 > mcubootSettings["alignment"]["flashBoundary"])
    {
        logError(validation, inst, "bootloaderBase",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and base addresses are all within 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
        logError(validation, inst, "bootloaderSize",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and base addresses are all within 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
        logError(validation, inst, "primaryBase1",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and base addresses are all within 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
        logError(validation, inst, "primarySize1",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and base addresses are all within 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
    }
    if(inst.externalFlash === true)
    {
        if(inst.secondaryBase1 + inst.secondarySize1 > externalFlashSize)
        {
            logError(validation, inst, "secondaryBase1",
                    "MCUboot layout exceeds the external flash boundaries. Make sure all image sizes and base addresses are all within 0x" + externalFlashSize);
            logError(validation, inst, "secondarySize1",
                    "MCUboot layout exceeds the external flash boundaries. Make sure all image sizes and base addresses are all within 0x" + externalFlashSize);
        }
    }
    else
    {
        if(inst.secondaryBase1 + inst.secondarySize1 > mcubootSettings["alignment"]["flashBoundary"])
        {
            logError(validation, inst, "secondaryBase1",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and base addresses are all within 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
            logError(validation, inst, "secondarySize1",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes and base addresses are all within 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
        }
    }

    // check if non secure image sizes and bases are within bounds
    if(inst.tzEnabled === true)
    {
        if(inst.primarySize2 !== inst.secondarySize2)
        {
            logError(validation, inst, "primarySize2",
                    "Primary image must be the same size as secondary image");
            logError(validation, inst, "secondarySize2",
                    "Secondary image must be the same size as primary image");
        }
        if( inst.primaryBase2 + inst.primarySize2 + inst.secondarySize1 + inst.secondarySize2 > mcubootSettings["alignment"]["flashBoundary"])
        {
            logError(validation, inst, "primarySize2",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
            logError(validation, inst, "secondarySize1",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
            logError(validation, inst, "secondarySize2",
                    "MCUboot layout exceeds the flash boundaries. Make sure all image sizes are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
        }
        
        if(inst.externalFlash === true)
        {
            if(inst.secondarySize2 % externalFlashSectorSize !== 0)
            {
                logError(validation, inst, "secondarySize2",
                        "The size of the secondary image must be a multiple of the sector size.");
            }
            if( inst.secondaryBase2 + inst.secondarySize2 > externalFlashSize)
            {
                logError(validation, inst, "secondaryBase2",
                        "MCUboot layout exceeds the external flash boundaries. Make sure all image sizes are less than 0x" + externalFlashSize);
                logError(validation, inst, "secondarySize2",
                        "MCUboot layout exceeds the external flash boundaries. Make sure all image sizes are less than 0x" + externalFlashSize);
            }
        }
        else
        {
            if(inst.secondarySize2 % mcubootSettings["alignment"]["sectorSize"] !== 0)
            {
                logError(validation, inst, "secondarySize2",
                        "The size of the secondary image must be a multiple of the sector size.");
            }
            if(inst.secondaryBase2 + inst.secondarySize2 > mcubootSettings["alignment"]["flashBoundary"])
            {
                logError(validation, inst, "secondaryBase2",
                        "MCUboot layout exceeds the flash boundaries. Make sure all image sizes are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
                logError(validation, inst, "secondarySize2",
                        "MCUboot layout exceeds the flash boundaries. Make sure all image sizes are less than 0x" + mcubootSettings["alignment"]["flashBoundary"].toString(16));
            }
        }
        
    }
}

function getLinkerDefs()
{
    let mcuboot = system.modules["/ti/common/mcuboot"];
    let flashSize = mcuboot.$static.bootloaderSize;

    if(deviceGroup !== "DeviceGroup_CC13X2X7_26X2X7")
    {
        flashSize -= mcubootSettings["alignment"]["sectorSize"];
    }

    return [{ name: "MCUBOOT_FLASH_BASE", value: mcuboot.$static.bootloaderBaseAddress},
            { name: "MCUBOOT_FLASH_SIZE", value: flashSize}];

}

exports = base;