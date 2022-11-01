/*
 * Copyright (c) 2015-2022, Texas Instruments Incorporated
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

#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>
#include <stdbool.h>

#define RADIO_ID_BUILTIN        (1)
#define RADIO_ID_TRX            (2)
#define RADIO_ID_DEFAULT        (RADIO_ID_BUILTIN)

#define RADIO_ERROR_VALUE       (-128)

/* Unique identifier for each radio. Strictly positive: 1,2,3... */
typedef uint8_t Radio_Id;

/***************************************************************************************************
 *
 * Verify the radio identifier is valid
 *
 * param id - Unique identifier for each radio.
 *
 * return true if radio id is a valid number, else false
 ***************************************************************************************************/
extern bool Radio_verifyRadioId(Radio_Id id);

/***************************************************************************************************
 *
 * Set the current radio, does not verify the identifier.
 *
 * Must call Radio_verifyRadioId() prior.
 *
 * param id - Unique identifier for each radio.
 *
 * return none
 ***************************************************************************************************/
extern void Radio_setCurrentRadio(Radio_Id id);

/***************************************************************************************************
 *
 * Get the current radio
 *
 * return none
 *
 * return id - Unique identifier of the current radio.
 ***************************************************************************************************/
extern Radio_Id Radio_getCurrentRadio(void);

/***************************************************************************************************
 *
 * Initialize commands that are common for all PHYs
 *
 * param id - Unique identifier for each radio.
 *
 * return none
 ***************************************************************************************************/
extern void Radio_Init(void);

/***************************************************************************************************
 *
 * Initialize a PHY based on which setup command is used (AT_setupType)
 *
 * param id - Unique identifier for each radio.
 * param phyIndex - Number between 0 and AT_numSupportedPhys
 *
 * return none
 ***************************************************************************************************/
extern void Radio_setupPhy(uint8_t phyIndex);

/***************************************************************************************************
 *
 * Add application specific fields to the different TX commands imported from sysConf.
 * Setup the length properly based on AT_apiType, AT_cmdType and AT_testType.
 * Set the frequency, and transmit a given number of packets
 *
 * param id - Unique identifier for each radio.
 * param numPkts - Number of packets to be transmitted during the TX PER Test (TEST_MODE_PER_TX)
 * param *pktLen - Pointer to the length of the packet to be transmitted
 *
 * return true or false, depending on success or not
 *
 ***************************************************************************************************/
extern bool Radio_packetTx(uint16_t numPkts, uint32_t *pktLen);

/***************************************************************************************************
 *
 * Define a queue for the received packets.
 * Add application specific fields to the different RX commands imported from sysConf.
 * Set the frequency and start RX in non-blocking mode. RX mode can only be terminated by
 * receiving an AT command to exit (EXIT_TEST_MODE)
 *
 * param id - Unique identifier for each radio.
 * param pktLen - Used for packet length filtering
 *
 * return true or false, depending on success or not
 *
 ***************************************************************************************************/
extern bool Radio_packetRx(uint8_t pktLen);

/***************************************************************************************************
 *
 * Set the frequency and enter continuous TX mode
 *
 * param id - Unique identifier for each radio.
 * param cw - 1: Carrier Wave
 *            0: Modulated
 *
 * return true or false, depending on success or not
 *
 ***************************************************************************************************/
extern bool Radio_contTx(bool cw);

/***************************************************************************************************
 *
 * Set the frequency and enter continuous RX mode
 *
 * param none
 *
 * return true or false, depending on success or not
 *
 ***************************************************************************************************/
extern bool Radio_contRx(void);

/***************************************************************************************************
 *
 * Set the frequency in the CMD_FS and calculate and set other frequency dependent parameters.
 * If BLE is selected and the application shall communicate with SmartRF Studio, the BLE channel
 * is set to channel 17 if the input frequency is not one of the valid BLE channels
 *
 * param freq - Frequency in Hz
 *
 * return none
 ***************************************************************************************************/
extern void Radio_setFreq(uint32_t freq);

/***************************************************************************************************
 *
 * Read the current frequency
 *
 * param none
 *
 * return the frequency in Hz
 *
 ***************************************************************************************************/
extern uint32_t Radio_getFreq(void);

/***************************************************************************************************
 *
 * Set the TX Power
 *
 * param i8TxPowerDbm - TX power in dBm
 *
 * return true or false, depending on success or not
 *
 ***************************************************************************************************/
extern bool Radio_setPower(int8_t i8TxPowerDbm);

/***************************************************************************************************
 *
 * Get the current TX power
 *
 * param id - Unique identifier for each radio.
 *
 * return the TX power in dBm, or RADIO_ERROR_VALUE if the TX Power cannot be determined
 *
 ***************************************************************************************************/
extern int8_t Radio_getPower(void);

/***************************************************************************************************
 *
 * Get the current RSSI
 *
 * param id - Unique identifier for each radio.
 *
 * return the RSSI in dBm, or RADIO_ERROR_VALUE if the RSSI cannot be determined
 *
 ***************************************************************************************************/
extern int8_t Radio_getCurrentRssi(void);

/***************************************************************************************************
 *
 * Get the average RSSI during PER RX
 *
 * param id - Unique identifier for each radio.
 *
 * return the RSSI in dBm
 *
 ***************************************************************************************************/
extern int32_t Radio_getAvgRssi(void);

/***************************************************************************************************
 *
 * Get the minimum/lowest RSSI during PER RX
 *
 * param id - Unique identifier for each radio.
 *
 * return the RSSI in dBm
 *
 ***************************************************************************************************/
extern int8_t Radio_getMinRssi(void);

/***************************************************************************************************
 *
 * Get the maximum/highest RSSI during PER RX
 *
 * param id - Unique identifier for each radio.
 *
 * return the RSSI in dBm
 *
 ***************************************************************************************************/
extern int8_t Radio_getMaxRssi(void);

/***************************************************************************************************
 *
 * Cancels RX commands
 *
 * param id - Unique identifier for each radio.
 *
 * return none
 *
 ***************************************************************************************************/
extern void Radio_cancelRx(void);

/***************************************************************************************************
 *
 * Cancels TX commands
 *
 * param id - Unique identifier for each radio.
 *
 * return none
 *
 ***************************************************************************************************/
extern void Radio_cancelTx(void);

/***************************************************************************************************
 *
 * Get the number of received packets
 *
 * param id - Unique identifier for each radio.
 *
 * return the number of packets received
 *
 ***************************************************************************************************/
extern uint16_t Radio_getNumRxPackets(void);

/***************************************************************************************************
 *
 * Check if new packet length is valid. Acceptable range is 2-255 bytes. When using BLE PHY for
 * Smart RF Studio, the acceptable range is 12-37 bytes. Packet length is set to 37 if the
 * new packet length is higher than 37 and the packet length will be set to 12
 * if the new packet length is lower than 12 (a true is still returned in both cases).
 *
 * param id - Unique identifier for each radio.
 * param *perPktLen - Pointer to packet length
 *
 * return true if packet length is valid, false if it is invalid
 *
 ***************************************************************************************************/
extern bool Radio_checkPacketLength(uint32_t *perPktLen);

/***************************************************************************************************
 *
 * Get the number of supported phys for the selected radio.
 *
 * param id - Unique identifier for each radio.
 *
 * return Number of supported Phys for this radio
 *
 ***************************************************************************************************/
extern uint8_t Radio_getNumSupportedPhys(void);

/***************************************************************************************************
 *
 * Get the name of a phy from a chosen radio.
 *
 * param id - Unique identifier for each radio.
 * param phyIndex index of the phy
 *
 * return Name of supported Phys for this radio
 *
 ***************************************************************************************************/
extern char *Radio_getPhyName(uint8_t phyIndex);

#endif // RADIO_H

