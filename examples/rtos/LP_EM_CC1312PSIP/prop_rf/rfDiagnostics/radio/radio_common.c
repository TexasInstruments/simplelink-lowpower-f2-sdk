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

void Radio_setupPhy(uint8_t phyIndex){
    radios[currentRadio-1].setupPhy(phyIndex);
}

bool Radio_packetTx(uint16_t numPkts, uint32_t *pktLen){
    return(radios[currentRadio-1].packetTx(numPkts, pktLen));
}

bool Radio_packetRx(uint8_t pktLen){
    return(radios[currentRadio-1].packetRx(pktLen));
}

bool Radio_contTx(bool cw){
    return(radios[currentRadio-1].contTx(cw));
}

bool Radio_contRx(void){
    return(radios[currentRadio-1].contRx());
}

void Radio_setFreq(uint32_t freq){
    radios[currentRadio-1].setFreq(freq);
}

uint32_t Radio_getFreq(void){
    return(radios[currentRadio-1].getFreq());
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

bool Radio_checkPacketLength(uint32_t *perPktLen){
    return(radios[currentRadio-1].checkPacketLength(perPktLen));
}

uint8_t Radio_getNumSupportedPhys(void){
    return(radios[currentRadio-1].getNumSupportedPhys());
}

char *Radio_getPhyName(uint8_t phyIndex){
    return(radios[currentRadio-1].getPhyName(phyIndex));
}
