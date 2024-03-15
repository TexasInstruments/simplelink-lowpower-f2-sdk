#include "radio.h"
#include "radio_pvt.h"

#include <string.h>

/** Global multi-radio functions object. */
Radio_Fxns radios[NUM_RADIOS];

/** Global radio ID */
Radio_Id currentRadio = RADIO_ID_DEFAULT;

bool Radio_verifyRadioId(Radio_Id id){
    return ((id <= NUM_RADIOS) && (id > 0));
}

void Radio_setCurrentRadio(Radio_Id id){
    currentRadio = id;
}

Radio_Id Radio_getCurrentRadio(void){
    return (currentRadio);
}

extern void Radio_Builtin_registerFxns(Radio_Fxns *fxns);
#if defined(DUAL_RADIOS)
extern void Radio_Trx_registerFxns(Radio_Fxns *fxns);
#endif

void Radio_Init(void)
{
    static bool firstTime = true;
    if(firstTime){
        Radio_Builtin_registerFxns(&radios[RADIO_ID_BUILTIN-1]);
#if defined(DUAL_RADIOS)
        Radio_Trx_registerFxns(&radios[RADIO_ID_TRX-1]);
#endif
        firstTime = false;
    }
    radios[currentRadio-1].init();

}

bool Radio_setupPhy(uint8_t phyIndex, uint8_t phyIndex2){
    return radios[currentRadio-1].setupPhy(phyIndex, phyIndex2);
}

bool Radio_enableMdr(uint8_t region){
    if (radios[currentRadio-1].enableMdr == NULL){
        return RADIO_UNSUPPORTED_CMD;
    }
    else{
        return(radios[currentRadio-1].enableMdr(region));
    }
}

bool Radio_packetTx(uint16_t numPkts, uint32_t *pktLen){
    return(radios[currentRadio-1].packetTx(numPkts, pktLen));
}

bool Radio_packetRx(uint16_t pktLen){
    return(radios[currentRadio-1].packetRx(pktLen));
}

bool Radio_packetMdrTx(uint16_t numPkts, uint32_t *pktLen){
    if (radios[currentRadio-1].packetMdrTx == NULL){
        return RADIO_UNSUPPORTED_CMD;
    }
    else{
        return(radios[currentRadio-1].packetMdrTx(numPkts, pktLen));
    }
}

bool Radio_packetMdrRx(uint16_t pktLen){
    if (radios[currentRadio-1].packetMdrRx == NULL){
        return RADIO_UNSUPPORTED_CMD;
    }
    else{
        return(radios[currentRadio-1].packetMdrRx(pktLen));
    }
}

bool Radio_packetMdrCsTx(uint16_t numPkts, uint32_t *pktLen){
    if (radios[currentRadio-1].packetMdrCsTx == NULL){
        return RADIO_UNSUPPORTED_CMD;
    }
    else{
        return(radios[currentRadio-1].packetMdrCsTx(numPkts, pktLen));
    }
}

bool Radio_packetCsTx(uint16_t numPkts, uint32_t *pktLen){
    if (radios[currentRadio-1].packetCsTx == NULL){
        return RADIO_UNSUPPORTED_CMD;
    }
    else{
        return(radios[currentRadio-1].packetCsTx(numPkts, pktLen));
    }
}

bool Radio_contTx(bool cw){
    return(radios[currentRadio-1].contTx(cw));
}

bool Radio_contRx(void){
    return(radios[currentRadio-1].contRx());
}

void Radio_setFreq(uint32_t freq, uint32_t mdrFreq){
    radios[currentRadio-1].setFreq(freq,mdrFreq);
}

void Radio_getFreq(RF_Frequency *freqs){
    radios[currentRadio-1].getFreq(freqs);
}

bool Radio_setPower(int8_t i8TxPowerDbm){
    return(radios[currentRadio-1].setPower(i8TxPowerDbm));
}

int8_t Radio_getPower(void){
    return(radios[currentRadio-1].getPower());
}

int8_t Radio_getCurrentRssi(void){
    return(radios[currentRadio-1].getCurrentRssi());
}

int32_t Radio_getAvgRssi(void){
    return(radios[currentRadio-1].getAvgRssi());
}

int8_t Radio_getMinRssi(void){
    return(radios[currentRadio-1].getMinRssi());
}

int8_t Radio_getMaxRssi(void){
    return(radios[currentRadio-1].getMaxRssi());
}

void Radio_cancelRx(void){
    radios[currentRadio-1].cancelRx();
}

void Radio_cancelTx(void){
    radios[currentRadio-1].cancelTx();
}

uint16_t Radio_getNumRxPackets(void){
    return(radios[currentRadio-1].getNumRxPackets());
}

uint16_t Radio_getNumRxPacketsNok(void){
    return(radios[currentRadio-1].getNumRxPacketsNok());
}

uint16_t Radio_getNumRxSync(void){
    return(radios[currentRadio-1].getNumRxSync());
}

uint8_t Radio_getRxPhyIndex(void){
    return(radios[currentRadio-1].getRxPhyIndex());
}

bool Radio_checkPacketLength(uint32_t *perPktLen){
    return(radios[currentRadio-1].checkPacketLength(perPktLen));
}

uint8_t Radio_getNumSupportedPhys(void){
    return(radios[currentRadio-1].getNumSupportedPhys());
}

char *Radio_getPhyName(uint8_t phyIndex){
    return(radios[currentRadio-1].getPhyName(phyIndex));
}

char *Radio_getRadioVersion(void){
    if (radios[currentRadio-1].getRadioVersion == NULL){
        return RADIO_UNSUPPORTED_CMD;
    }
    else{
        return (radios[currentRadio-1].getRadioVersion());
    }
}
