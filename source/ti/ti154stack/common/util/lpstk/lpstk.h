/******************************************************************************

 @file lpstk.h

 @brief Sensor Specific Functions API

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
#ifndef LPSTK_H
#define LPSTK_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include <ti/sysbios/knl/Event.h>

#include "lpstk/lpstk_sensors.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Typedefs
 *****************************************************************************/

/*!
 *  @brief    A handler to receive accelerometer tilt alert callbacks.
 */
typedef void (*Lpstk_AccelTiltCb)(void);

/* LPSTK Events */
typedef enum
{
    LPSTK_HUMIDITY      = Event_Id_00,
    LPSTK_TEMPERATURE   = Event_Id_01,
    LPSTK_LIGHT         = Event_Id_02,
    LPSTK_ACCELEROMETER = Event_Id_03,
    LPSTK_HALL_EFFECT   = Event_Id_04,
} Lpstk_SensorMask;

/******************************************************************************
 Public Variables
 *****************************************************************************/


/******************************************************************************
 Function Prototypes
 *****************************************************************************/
void Lpstk_init(void *evntHandle, Lpstk_AccelTiltCb accelTiltCb);
void Lpstk_processEvents(void);
void Lpstk_initSensorReadTimer(Lpstk_SensorMask sensors, uint32_t clockPeriod);
void Lpstk_setSensorReadTimer(Lpstk_SensorMask sensors, uint32_t clockPeriod);
void shutDownSensors(Lpstk_SensorMask sensors);

float Lpstk_getTemperature(void);
float Lpstk_getHumidity(void);
void Lpstk_getAccelerometer(Lpstk_Accelerometer *accel);
bool Lpstk_getHallEffectSwitch(void);
float Lpstk_getLux(void);

#ifdef __cplusplus
}
#endif

#endif /* LPSTK_H */

