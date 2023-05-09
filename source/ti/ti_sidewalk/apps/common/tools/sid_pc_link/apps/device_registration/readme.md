# Sidewalk Device Registration Script

Tool to register a Sidewalk supported device (1P/3P) to the Sidewalk network
from a PC or from an already registered sidewalk enabled Gateway Device

Registration can be performed using 2 methods:
 - LWA token
 - Sidewalk ID of Ring Gateway in combination with Z-asset token

**P.S - LWA Token is recommended**

# Pre-requisites

Manufacturing information should be available on Nordic DK board in order to
perform Sidewalk registration with cloud.

# Description of the app_config.json file entries
| Entry                    | Description                                                                                                                                                      | Default value | Possible value        | Comments                                                             |
|--------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------|-----------------------|----------------------------------------------------------------------|
| REGISTRATION_ENVIRONMENT | Cloud environment. Defines endpoint to register the device                                                                                                       | beta          | beta<br>gamma<br>prod |                                                                      |
| BLUETOOTH_ADAPTER        | Name of the Bluetooth adapter to use in the registration process                                                                                                 | hci0          | hci0<br>hci1          | Run hcitool devices to get the adapter information                   |
| COMMAND_TIMEOUT          | Time in seconds to wait for a response from selected edge device                                                                                                 | 2             | Any value             |                                                                      |
| DEVICE_IDENTIFIER        | BLE MAC address of the selected edge device to register                                                                                                          | -             | -                     | Run sudo hcitool lescan to get Nordic-DK BLE mac address             |
| GATEWAY_ID               | Sidewalk ID of the Gateway device that will be used in the registration process                                                                                  | -             | -                     | Only required when Z-ASSET token is used                             |
| ENDPOINT_ID              | 32 byte SMSN value ("metadata.smsn" field) that is present in the acs console generated json file <br>or 5 bytes Sidewalk-ID if the mfg page has the sidewalk-id | -             | -                     | Can be read from manufacturing page                                  |
| AUTH_TOKEN               | z-asset to authenticate calls to cloud endpoints                                                                                                                 | -             | -                     | Should be obtained from gateway registered to the account            |
| LWA_TOKEN                | LWA token to authenticate calls to cloud endpoints                                                                                                               | -             | -                     | Retrieved from following steps under "Steps using LWA Token" section |
| REFRESH_TOKEN            | Refresh token to retrieve a new LWA token                                                                                                                        | -             | -                     | Retrieved from following steps under "Steps using LWA Token" section |

# Steps using LWA Token
## Create Amazon developer account

- Use https://developer.amazon.com/ to register amazon account
- Login to https://developer.amazon.com/
- Click on Developer Console
    * Click Login with Amazon
        + Click "Create a new security profile"
            - Fill in Security Profile Name
            - Fill in Security Profile Description
            - Fill in Consent Privacy Notice URL with https://ring.com/privacy
                * If this part does not appear, finish setting up the security
                  profile then click Login with Amazon at the top of the page
                  and ensure to select your security group to enable LWA.
                  After this you will need to enter the Consent URL and save.
                  LWA token generation will not work without this step.
            - Click Save button
        + Click in created Security Profile
            - Go to Web Settings and click Edit
            - Fill "Allowed Origins" as http://localhost:8000/
            - Save

## Link Ring account to amazon account

- QA and PROD environment
    * Linking Ring account to amazon account can be done using Ring Beta mobile
      application (https://confluence.atl.ring.com/pages/viewpage.action?spaceKey=AD&title=Release+Management+for+Halo+2.0+Beta .
      Use link in Section "Internal Beta debug version")
    * After installation of Ring mobile application
        + Long press on Ring icon on Login page to select environment
        + Create account in QA / PROD if not already existing. Login to
          the account
        + Under Account setting page, select 'Link your Amazon account'
            - Enter Ring account password
            - Enter Amazon account username and password. Verify OTP and
              accounts should get linked
- DEV environment
    * Manual account linking is required. Contact Cloud team for manual
      linking of Ring DEV account to Amazon PROD account (Chintan Desai)
        + To link account, below are the credentials required from cloud team:
            - Amazon user name : <Email address>
            - Amazon Cutomer ID : https://developer.amazon.com/mycid.html
            - Ring User name : <Email address>
            - Ring Customer ID : Can be retrieved from
              https://admin.dev.ring.com/  → Click Customers
              (Provide Email Id, First and Last name associated with account)

# Script Execution Steps

## Pre-requisites:

- Python 3 or greater
- Ubuntu 18.04 or newer
- `pip3 install -r requirements.txt`

## Using LWA Token

### Obtaining LWA and Refresh Tokens

- Navigate to `sid_pc_link/apps/device_registration`
    * To retrieve an LWA Token:
        * Fetching LWA token only: ./main.py --lwa --client-id **[your client ID]**
          - This option will launch a Web browser (Note: Allow popup) and request your Amazon Developer account credentials.
        * Fetching LWA Token and Refresh Token: ./main.py --lwa-cg --client-id **[your client ID]** --client-secret **[your client secret]**
          - This option will launch a Web browser (Note: Allow popup) and request your Amazon Developer account credentials.
        * Refreshing your LWA Token: ./main.py --refresh-token  --client-id **[your client ID]** --client-secret **[your client secret]**
          - This option will automatically refresh your LWA Token. This option uses your previously generated refresh token. Please successfully execute the section `Fetching LWA Token and Refresh Token` at least once.
    * Client ID and Client Secret can be fetched from developer.amazon.com under
      (Security Profile → Web Settings → Client ID/Client Secret)
    * **Note: LWA token is valid for 1 hour, Refresh Token is valid forever. LWA token must be refreshed after it expires**

### Executing Device Registration

- Navigate to `sid_pc_link/apps/device_registration`
    * Open app_config.json, and write:
      * DEVICE_IDENTIFIER: Your device MAC Address
      * BLUETOOTH_ADAPTER: Your Bluetooth adapter ID, eg: hci0, hci1.
      * ENDPOINT_ID: Your device Sidewalk ID or SMSN.
    * RUN `./main.py -r`

### Example app_config.json files with LWA Token
```
Example of app_config.json with SMSN:
{
  "REGISTRATION_ENVIRONMENT": "beta",
  "BLUETOOTH_ADAPTER": "hci1",
  "COMMAND_TIMEOUT": 2,
  "GATEWAY_ID": null,
  "ENDPOINT_ID": "3E3069414973133E681C7AB5145B6C539C1E7401B99FBA8FA7D148D9EC73BF33",
  "AUTH_TOKEN": null,
  "LWA_TOKEN": "Bearer Atza|IwEBIIeBeFjPXwKVvbwgq_QyokXCx-OrlRaiUOJ9czXBvpfc_I61E10Wu7O2AImNV2eXOi6yuG7yOJ5dmtxuzshHwqHnw5XeCdD1ZHELXI0kDPM4iK4MJmV1k4TsEtqz2kBhdl9_40wB3bq13_wou_VZTwoxTsZ6NxbZ67e447yShMS3WWcZ9JSr3A9mfLTR6c-y__MPfzUV6EAgyEWxhoo4B-H9q52NmP0eqsEe59EXQ9d6gMHC3UrySdpDo8i21es6dP4dZWW8MSUoiLAXtyChHvY-m94X1f9TNZZs1RcNWZhvsPkyNgSnAaHAGuKwIjfxMO8yyz57WS8bU2RobBpo_fomWflay_6X7Zh8MP6bMPZqU3Ir3-zXRxAq7chPpQiq1SsizvJ4qDl5nyKsACGCERzFqpOhH-neKVzm5kU3oPhbXA",
  "REFRESH_TOKEN": null,
  "SCAN_RETRIES": 5,
  "BLUETOOTH_SCAN_TIMEOUT": 5
}


BLUETOOTH_ADAPTER : Run hcitool devices to get the adapter information
DEVICE_IDENTIFIER : Run sudo hcitool lescan to get Nordic-DK BLE mac address
ENDPOINT_ID : SMSN of the Nordic-DK (that which is present in the dk's manufacturing page).
              SMSN is also present in the "metadata.smsn" field of acs json file, while sidewalk-id is
              the in the name of the acs json file (i.e certificate_${SIDEWALK_ID}.json -> certificate_A0FFFFFFFF.json SIDEWALK_ID=A0FFFFFFFF)
LWA_TOKEN : LWA token from Step 3
```

## Using Z-Asset Token

### Executing Device Registration

- Navigate to `sid_pc_link/apps/device_registration`
    * Open app_config.json, and write:
      * BLUETOOTH_ADAPTER: Your Bluetooth adapter ID, eg: hci0, hci1.
      * ENDPOINT_ID: Your device Sidewalk ID or SMSN.
      * GATEWAY_ID: Sidewalk ID of the Gateway device that will be used in the registration process.
      * AUTH_TOKEN: Z-Asset to authenticate calls to cloud endpoints
    * RUN `./main.py -r`

### Example app_config.json files with z-asset Token
```
app_config.json example with SMSN:
{
  "REGISTRATION_ENVIRONMENT": "beta",
  "BLUETOOTH_ADAPTER": "hci1",
  "COMMAND_TIMEOUT": 2,
  "GATEWAY_ID": "A0002C5CC0",
  "ENDPOINT_ID": "3E3069414973133E681C7AB5145B6C539C1E7401B99FBA8FA7D148D9EC73BF33",
  "AUTH_TOKEN": "z-asset MjkwNzMyODo5MmE5YWJlMC05NjA5LTQxODYtNWExMi1lMjNiYzc2MmM2M2Y=",
  "LWA_TOKEN": null,
  "REFRESH_TOKEN": null,
  "SCAN_RETRIES": 5,
  "BLUETOOTH_SCAN_TIMEOUT": 5
}

BLUETOOTH_ADAPTER : Run hcitool devices to get the adapter information
GATEWAY_ID : Registered gateway id with the account
ENDPOINT_ID : SMSN of the Nordic-DK (that which is present in the dk's manufacturing page).
              SMSN is also present in the "metadata.smsn" field of acs json file, while sidewalk-id is
              the in the name of the acs json file (i.e certificate_${SIDEWALK_ID}.json -> certificate_A0FFFFFFFF.json SIDEWALK_ID=A0FFFFFFFF)
AUTH_TOKEN : z-asset token
```
