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

#ifndef RADIO_PVT_H
#define RADIO_PVT_H

#include <stdint.h>
#include <stdbool.h>

/* Defines */
#if defined(DUAL_RADIOS)
#define NUM_RADIOS              2
#else
#define NUM_RADIOS              1
#endif

/** Structure defining all the virtual functions of the Radio Object. */
typedef struct Radio_Fxns_t{
    void (*init)(void);
    bool (*setupPhy)(uint8_t phyIndex, uint8_t phyIndex2);
    bool (*enableMdr)(uint8_t region);
    bool (*packetTx)(uint16_t numPkts, uint32_t *pktLen);
    bool (*packetRx)(uint16_t pktLen);
    bool (*packetMdrTx)(uint16_t numPkts, uint32_t *pktLen);
    bool (*packetMdrRx)(uint16_t pktLen);
    bool (*packetMdrCsTx)(uint16_t numPkts, uint32_t *pktLen);
    bool (*packetCsTx)(uint16_t numPkts, uint32_t *pktLen);
    bool (*contTx)(bool cw);
    bool (*contRx)(void);
    void (*setFreq)(uint32_t freq, uint32_t mdrFreq);
    void (*getFreq)(RF_Frequency *freqs);
    bool (*setPower)(int8_t i8TxPowerDbm);
    int8_t (*getPower)(void);
    int8_t (*getCurrentRssi)(void);
    int32_t (*getAvgRssi)(void);
    int8_t (*getMinRssi)(void);
    int8_t (*getMaxRssi)(void);
    void (*cancelRx)(void);
    void (*cancelTx)(void);
    uint16_t (*getNumRxPackets)(void);
    uint16_t (*getNumRxPacketsNok)(void);
    uint16_t (*getNumRxSync)(void);
    uint8_t (*getRxPhyIndex)(void);
    bool (*checkPacketLength)(uint32_t *perPktLen);
    uint8_t (*getNumSupportedPhys)(void);
    char *(*getPhyName)(uint8_t phyIndex);
	char *(*getRadioVersion)(void);
}Radio_Fxns;

/* Global multi-radio functions object. */
extern Radio_Fxns radios[NUM_RADIOS];

#endif // RADIO_PVT_H
