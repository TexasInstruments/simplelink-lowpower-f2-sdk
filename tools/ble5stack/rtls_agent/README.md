# RTLS Agent

RTLS Agent folder contains custom python package (rtls, unpi and rtls_util) and example to present capability of RTLS CCI, AoA and CL AOA for CC26x2. 

## Getting Started

Instructions below will setup your python environment with required packages.  

### Prerequisites

- The latest version of [Python 3.9]( https://www.python.org/downloads/release/python-3913/ )

    Note: Python 3.9 should be installed at **_C:\Python39_**. 

- [Code Composer Studio (CCS)](http://www.ti.com/tool/CCSTUDIO) 
- Set of three CC26x2 devices.

     
## Running Non-Visual Demo
Non-visual demo its python example that uses rtls_util packages to start and run RTLS CCI / CL AOA / AOA functionality for CC26x2.   

### Preparing Environment
* Create python virtual environment 
    ```commandline
    cd <rtls_agent folder>
    c:\Python39\python.exe -m pip install virtualenv [--proxy <www.proxy.com>]
    c:\Python39\python.exe -m venv .venv
    ```
* Setup external packages in virtual environment
    ```
    cd <rtls_agent folder>
    .venv\Scripts\activate.bat
    python -m pip install -r requirements.txt [--proxy <www.proxy.com>]
    ```
* Import the examples rtls_coordinator, rtls_responder and (if needed) rtls_passive under CCS. The examples are stored under examples\rtos\CC26X2R1_LAUNCHXL\ble5stack.
Build the binaries and flash the devices.
  
### Executing example

Before executing example open and edit **<rtls_agent dir>/examples/rtls_connected.py** or **<rtls_agent dir>/examples/rtls_connectionless.py** in order to enable / disable functionality.

Executing example for connected mode: 
```
cd <rtls_agent folder>
.venv\Scripts\activate.bat

cd <rtls_agent folder>/examples
python.exe rtls_connected.py  
```

Executing example for connectionless mode: 
```
cd <rtls_agent folder>
.venv\Scripts\activate.bat

cd <rtls_agent folder>/examples
python.exe rtls_connectionless.py  
```


## Running Visual Demo

##### Visual demo based on two elements
* Backend - Application that communicates with devices and reports to the frontend via WebSocket
* Frontend - Based on Angular 8

##### Required
* Installed latest Chrome browser 

##### Step by step guide to start visual demo:

1. Open CMD / Terminal at **<rtls_agent folder>/rtls_ui**
2. Start **rtls_ui** 
    ```
    rtls_ui.exe [Windows 10]
    rtls_ui [Ubuntu 20.04]
    rtls_ui_macos [macOS Monterey]
    ``` 
3. Wait for the Chrome to start on http://127.0.0.1:5005 [ <sup id="n1">[1](#f1)</sup> ]
4. Refresh UI to the latest by hitting CTRL + F5 or CTRL + R
5. Press on "Get Started !"
6. Select RTLS mode
7. Select RTLS coordinator and passive (if any) devices
8. Press on "Auto Play" to start the auto process of:
    1. Scanning for Responders
    2. Selecting best Responder by RSSI
    3. Connecting to selected Responders
    4. Starting available features

##### Notes
<b id="n1">1</b>: In case UI not start in Chrome. Open Chrome and type URL manually [↩](#a1)
