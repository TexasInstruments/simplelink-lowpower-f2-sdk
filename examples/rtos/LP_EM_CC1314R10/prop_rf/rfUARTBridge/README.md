# UARTbridge

---

Project Setup using the System Configuration Tool (SysConfig)
-------------------------
The purpose of SysConfig is to provide an easy to use interface for configuring
drivers, RF stacks, and more. The .syscfg file provided with each example
project has been configured and tested for that project. Changes to the .syscfg
file may alter the behavior of the example away from default. Some parameters
configured in SysConfig may require the use of specific APIs or additional
modifications in the application source code. More information can be found in
SysConfig by hovering over a configurable and clicking the question mark (?)
next to it's name.

Example Summary
---------------
The Uart Bridge illustrates how to do simple packet transmitting and receiving from one UART to
another over the RF driver. This example is meant to be used with two RF devices. For every packet received, 
the packet is printed on the UART (terminal). The red led changes state when a packet is received, 
while the green led changes state when a packet is being transmitted. 
Packets are sent when the UART data is longer than the chosen maximum packet length.

Peripherals Exercised
---------------------


Resources & Jumper Settings
---------------------------
> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your project
directory for resources used and board-specific jumper settings. Otherwise, you can find
Board.html in the directory &lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

Board Specific Settings
-----------------------
1. The default frequency is:
    - 433.92 MHz for the CC1352P-4-LAUNCHXL and LP_CC1352P7_4
    - 2440 MHz on the CC26X2R1-LAUNCHXL and 2.4GHz-only devices
    - 868.0 MHz for other launchpads

Note: Frequency can be changed in SysConfig
2. On the CC1352P1 the high PA is enabled (high output power) for all
Sub-1 GHz modes by default.
3. On the CC1352P-2 and CC1352P-4 the high PA operation for Sub-1 GHz modes is not supported

Example Usage
-------------
Run the application on two boards. Open the corresponding terminals to the connected devices. When both devices are running the application, one should see that 
when typing into one terminal, the other prints the message and opposite. You can change the terminal you type into anytime, 
then the devices would switch their role (TX/RX). Please make sure that the terminal supports all keyboard inputs as keyboards can conatin various 
characters.

SmartRF studio is also an alternative, with manual alternating between TX and RX. Running this program on one device will make the device able to receive/transmit packages. 

__How to set up SmartRF studio for TX__

1. Open SmartRF studio and choose the device you want to use by double clicking it in the menu of connected devices (the other device should run the code)

2. Choose Packet TX in the upper right corner.

3. Ensure that the frequency, symbol rate, RX Filter BW, TX Power, deviation and whitening corresponds to the settings in the sysconfig file. 

4. Choose your expected packet count and your message format.

5. Run the code on the other device and click the green play button in the upper right corner. The message should be shown in the terminal. 

__How to set up SmartRF studio for RX__

1. Open SmartRF studio and choose the device you want to use by double clicking it in the menu of connected devices (the other device should run the code)

2. Choose Packet TX in the upper right corner.

3. Ensure that the frequency, symbol rate, RX Filter BW, TX Power, deviation and whitening corresponds to the settings in the sysconfig file.

4. Choose expected packet count and a format you want to view the incoming packets.

5. Press the green play button in the upper right corner and send a packet from the terminal by writing a letter/word. (you need to ensure that SmartRF studio runs in receive mode while you send the packet)


Application Design Details
--------------------------
This example consists of one thread which polls variables to check if callbacks for UART RX and RF RX have been executed.

When mainThread() is executed, it follows the procedure below.

1. Initialization of RF params, data queue to receive RF data, GPIO, and RF commands (CMD_PROP_TX and CMD_PROP_RX)

2. Initiailize and get access to UART2 with callback read mode and partial read return mode

3. Gets access to the radio

4. Set the frequency using CMD_FS command

5. Post CMD_PROP_RX command to start receiving RF data and execute UART2_read() to start recieving UART data

6. Check RF RX callback flag, packetRxCb. If it is set, then use UART to write/transmit the data recieved via RF 

7. Check UART RX callback flag, bytesReadCount. If it is set, then:
    - Copy UART data to RF TX packet
    - Cancel ongoing CMD_PROP_RX command
    - Transmit the packet over RF and toggle CONFIG_GPIO_GLED to indicate RF TX
    - Reset bytesReadCount
    - Post CMD_PROP_RX to resume RF RX and resume UART2_read()

8. When RF data with CRC OK is received, a callback funtion is called. The callback function sets the beginning and the length of the packet and copies it to the variable, packet. A flag, packetRxCb, is set to indicate RF RX callback has executed. CONFIG_GPIO_RLED is toggled to indicate RF RX.

9. When UART data is recieved, a callback function is called where bytesReadCount is set to the amount of bytes that have been received via UART
