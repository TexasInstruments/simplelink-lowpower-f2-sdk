/******************************************************************************

 @file lpstk_sensors.h

 @brief Common API layer to interface with sensors in the LPSTK

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/



#ifndef APPLICATION_LPSTK_SENSORS_H_
#define APPLICATION_LPSTK_SENSORS_H_

/******************************************************************************
 Includes
 *****************************************************************************/

#include <stdint.h>

#include <ti/drivers/GPIO.h>

#include "lpstk/adxl362/scif.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    SENSOR_INIT,
    SENSOR_SHUTDOWN,
    SENSOR_STARTUP,
    SENSOR_READY
} Lpstk_SensorStatus;

typedef enum
{
    LPSTK_SUCCESS,
    LPSTK_FAIL,
    LPSTK_INIT_FAIL,
    LPSTK_NULL_HANDLE,
    LPSTK_READY
} Lpstk_StatusCode;

typedef struct
{
    int16_t  x;          ///< For observation: Latest X-axis sample
    uint16_t xTiltDet;   ///< 1 when tilting along the X-axis is detected, otherwise 0
    int16_t  y;          ///< For observation: Latest Y-axis sample
    uint16_t yTiltDet;   ///< 1 when tilting along the Y-axis is detected, otherwise 0
    int16_t  z;          ///< For observation: Latest Z-axis sample
} Lpstk_Accelerometer;

typedef struct
{
    float      humidity;
    float   temperature;
    float         lux;
    Lpstk_Accelerometer accelerometer;
    bool    halleffectSwitchValue;
} Lpstk_Sensors;

void Lpstk_initHumidityAndTempSensor(float hHiLim, float hLoLim,
                                     float tHiLim, float tLoLim,
                                     GPIO_CallbackFxn hdc2010Callback);
void Lpstk_initLightSensor(float hHiLim, float hLoLim,GPIO_CallbackFxn opt3001Callback);
void Lpstk_initSensorControllerAccelerometer(SCIF_VFPTR scTaskAlertCallback);

uint8_t Lpstk_openHumidityTempSensor(void);
uint8_t Lpstk_openLightSensor(void);
uint8_t Lpstk_openAccelerometerSensor(void);

bool Lpstk_readTemperatureSensor(float *temperature);
bool Lpstk_readHumiditySensor(float *humidity);
bool Lpstk_readLightSensor(float *lux);
bool Lpstk_readHallEffectSensor(bool *switchValue);
void Lpstk_readAccelerometerSensor(Lpstk_Accelerometer *accel);

void Lpstk_shutdownHumidityTempSensor(void);
void Lpstk_shutdownLightSensor(void);
void Lpstk_shutdownAccelerometerSensor(void);


#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_LPSTK_SENSORS_H_ */
