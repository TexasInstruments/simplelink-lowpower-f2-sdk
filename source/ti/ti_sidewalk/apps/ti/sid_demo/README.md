TI Sidewalk Demo Example
==========================

## Table of Contents
* [Prerequisites](#Prerequisites)
* [Example Summary](#ExampleSummary)
* [Project Configurations](#ProjectConfigurations)
* [Proxy Settings](#ProxySettings)
* [Device Provisioning and Webapp Deployment](#DeviceProvisioningAndWebappDeployment)
* [Example Usage](#ExampleUsage)

## <a name="Prerequisites"></a>Prerequisites
* 1x Sidewalk Compatible Launchpad
* 1x Sidewalk Enabled Amazon Echo Device (Echo 4th Generation is recommended)
* AWS Account
    * [Create an AWS account](https://aws.amazon.com/)
* IAM User
    * [Create an IAM User](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users_create.html#id_users_create_console)
* User Group with *AdministratorAccess* policy
    * [Create IAM User Group](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_groups_create.html)
    * In Step 4, make sure to select the IAM User you created to add it to the group
* AWS Access Keys for your IAM User
    * Follow instructions under section [**"To create, modify, or delete the access keys of another IAM user (console)"**](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_credentials_access-keys.html#Using_CreateAccessKey)
* ARM-GCC Compiler v12.3.1

## <a name="ExampleSummary"></a>Example Summary
The Sidewalk Demo example application demonstrates how to provision, register, and test a sidewalk device. It can use either BLE or FSK 900 MHz radio to communicate with the gateway. It starts by initializing the clock and the timers. Then, various initializations will take place like key-value store, radio configs and memory. Finally, the main thread will start executing. The main thread starts checking the registration status and starts the Sidewalk gateway discovery. As a gateway is found, it will try to time sync with that gateway. After receiving time sync, the app starts to send capabilites discovery notification indicating support of 1 button and 2 LEDs. When discovery response is received, the app sends temperature sensor data every 15 seconds.

## <a name="ProjectConfigurations"></a>Project Configurations
**aws_credentials.yaml** file must first be populated with your AWS account credentials. Note that you will have these credentials after following the [prerequisites](#Prerequisites)  

1. **aws_access_key_id**: Your user's AWS Access Key ID
2. **aws_secret_access_key**: Your user;s AWS Secred Access Key

Sidewalk configurations are collected in a TI Sidewalk module in Sysconfig. The important ones are the following:  

1. **Link Type**: Select which PHY to be used at the application level.
    * **BLE**                                   : BLE is used for both registration and data
    * **FSK 900MHz**                            : FSK is used for both registration and data
    * **BLE Registration + FSK 900MHz Data**    : BLE is used for registration and FSK is used for data
2. **Enabled PHYs**: Select which PHYs can be used by Sidewalk.
3. **AWS Profile**: Name of AWS profile.  
    The default value is **default**.
    Note that this value must match the profile name provided in **aws_credentials.yaml** which corresponds to the correct AWS Access Key ID and AWS Secret Access Key.
4. **Destination Name**: Sidewalk destination used for uplink traffic routing.  
    The default destination name is **SensorAppDestination**  
    Note that this value must match the destination name used when deploying the AWS web application. Out of the box, this value is set to **SensorAppDestination**
5. **Region Name**: Region name where AWS operates.  
    The default region is **us-east-1**.
6. **Deploy Web Application**: Deploy the example web application to the AWS account provided in **aws_credentials.yaml**  
    To remove the web application from your AWS account, the following can be executed: simplelink_sdk_directory/source/ti/ti_sidewalk/apps/common/tools/web_app/bin/delete_stack
7. **Web Application Username**: Username for Web Application login.
8. **Web Application Password**: Password for Web Application login.

## <a name="ProxySettings"></a>Proxy Settings
Depending on your network, proxies may need to be set on CCS for the post-build steps to work, i.e., device auto-provisioning and webapp auto-deployment. To set proxies on CCS, go to **Window > Preferences > Code Composer Studio > Build > Environment**. Here, you can add the proxy name as "Variable" and the proxy as "Value".

## <a name="DeviceProvisioningAndWebappDeployment"></a>Device Provisioning and Webapp Deployment
During the build process the following are done automatically as post-build steps

1. Provision a new Sidewalk Edge Device
2. Generate a binary for the MFG region
3. Merge the MFG region binary with the main image to create the final image, **project_name_merged.hex**.
4. (Optional) Deploy the Web Application

NOTE: The merge is done so that only one image must be loaded to the target device. The final image will contain all the necessary code and credentials for proper operation.  
NOTE: The Web Application deployment can take up to 10 minutes to complete depending on your internet connection and/or operating system.  
NOTE: Full source code for provisioning and web application can be found [here](https://github.com/TexasInstruments/amazon-sidewalk-sample-iot-app)

## <a name="ExampleUsage"></a>Example Usage

##### Controls
* **Button 1 (Single Click) - Send Button Notification**  
    This action will queue a message to the cloud.
    If Sidewalk is not ready, it will simply show an error and do nothing.
    The queue will be processed eventually and all the queued messages will be sent.
* **Button 2 (Long press) - Factory Reset**  
    The app informs the Sidewalk stack about the factory reset event.
    The Sidewalk library clears its configuration from the non-volatile storage.
    The Sidewalk link status resets to `#SID_STATE_DISABLED`.
    After a successful reset, the device needs to be registered with the cloud services again.
* **Button 2 (Single Click - BLE+FSK only) - Set Connection Request**  
    This action is only available for BLE+FSK mode.
    This action will start BLE connection request to the Sidewalk Gateway.
    If device has not registered or time synchronized with a gateway, it will simply show an error and do nothing.
    After a successful connection, the app will switch to FSK link for data communication.

After loading the image to the target device, press the reset button and wait for about 2 minutes to complete initilization procedures (setting clocks, timers, key-value store, radio configurations, memory, etc.), registration, and time sync. After that, a device will show up in the Web Application.

1. (Optional) Open a serial session (e.g. PuTTY, TeraTerm) to the appropriate COM port with the following settings
```
    Baud-rate:      921600
    Data bits:           8
    Stop bits:           1
    Parity:           None
    Flow Control:     None
```
2. If `BLE Registration + FSK 900MHz Data` is selected, press BTN2 after registration and time sync to set connection request.
3. Press BTN1 on the Launchpad to see the device engage. This can be seen on the Webapp.
4. Periodically, the app sends temperature data to the cloud. This can be seen on the Webapp.
5. On the Webapp, toggle the LED buttons to see the LEDs turn on and off on the Launchpad.