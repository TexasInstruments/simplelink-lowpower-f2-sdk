/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 * =========================== ti_sidewalk_provisioning.syscfg.js ===========================
 *
 * Submodule definition for the SysConfig TI Sidewalk Provisioning submodule
 */

"use strict";

/*
 *  ======== onDeployStackChange ========
 * When deployStack is selected, show the username and password fields
 * When it's deselected, hide the username and password fields
 * @param inst  - Module instance containing the config that changed
 * @param ui    - The User Interface object
 */
function onDeployStackChange(inst,ui)
{
    if(inst.deployStack)
    {
        ui.webappUsername.hidden = false;
        ui.webappPassword.hidden = false;
    }
    else
    {
        ui.webappUsername.hidden = true;
        ui.webappPassword.hidden = true;
    }
}

/*!
 * ======== provisioningModule ========
 *
 * Syscfg submodule for TI-Sidewalk Provisioning
 */
const provisioningModule = {
    displayName: "Provisioning",
    description: "Configure Sidewalk Device Provisioning Settings",
    collapsed: false,
    config: [
        {
                name: "deployStack",
                displayName: "Deploy Web Application",
                description: "Deploy the example web application to the AWS account provided in config/aws_credentials.yaml",
                longDescription: "Deploy the example web application to the AWS account provided in config/aws_credentials.yaml.\n\n\
To remove the web application from your AWS account, the following file can be executed:\n\
\\<simplelink_sdk_directory\\>/source/ti/ti_sidewalk/apps/common/tools/web_app/bin/delete_stack",
                default: true,
                onChange: onDeployStackChange
        },
        {
            name: "webappUsername",
            displayName: "Web Application Username",
            description: "Username for Web Application login",
            longDescription: "Username for Web Application login",
            default: "SampleUsername"
        },
        {
            name: "webappPassword",
            displayName: "Web Application Password",
            description: "Password for Web Application login",
            longDescription: "Password for Web Application login",
            default: "SamplePassword"
        },
        {
            name: "awsProfile",
            displayName: "AWS Profile",
            description: "Name of the AWS profile",
            longDescription: "Name of the AWS profile",
            default: "default"
        },
        {
            name: "region",
            displayName: "Region",
            description: "The AWS Region where you want to create new connections",
            default: "us-east-1"
        },
        {
            name: "destinationName",
            displayName: "Destination Name",
            description: "Sidewalk destination used for uplink traffic routing",
            longDescription: "Sidewalk destination used for uplink traffic routing. This __must__ match the destination name used when deploying the AWS web application.",
            default: "SensorAppDestination"
        }
    ],
};

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - EasyLink CCA Config instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    validation.logInfo("When modified, the TI.bin generated alongside the application binary must be manually deleted before building", inst, "destinationName");
}

exports = {
    config: provisioningModule,
    validate: validate
};

