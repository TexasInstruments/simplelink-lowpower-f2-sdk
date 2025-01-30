/******************************************************************************

 @file lpstk.c

 @brief Launchpad Sensortag Kit Common framework

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

/******************************************************************************
 Includes
 *****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>

#include "ti_drivers_config.h"

#include "lpstk/lpstk.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/* LPSTK Events */
typedef enum
{
    LPSTK_EV_FACTORY_DEFAULT             = Event_Id_00,
    LPSTK_EV_SENSOR_POWER_UP             = Event_Id_01,
    LPSTK_EV_SENSOR_READ_AND_SHUTDOWN    = Event_Id_02,
    LPSTK_EV_SENSOR_READ                 = Event_Id_03,
    LPSTK_EV_SENSOR_SHUTDOWN             = Event_Id_04,
    LPSTK_EV_ACCEL_TILT_ALERT            = Event_Id_05,
} Lpstk_Events;

typedef struct
{
    Lpstk_SensorMask powerupMask;
    Lpstk_SensorMask readAndShutdownMask;
    Lpstk_SensorMask readMask;
    Lpstk_SensorMask shutdownMask;
} Lpstk_SensorEvMasks;

/*
 * time in milliseconds to wait for first sensor
 * measurement after calling sensor Open
 * */
#define SENSOR_STARTUP_TIME 1

/******************************************************************************
 External variables
 *****************************************************************************/

/******************************************************************************
 Public variables
 *****************************************************************************/


/******************************************************************************
 Local variables
 *****************************************************************************/

/* The application's semaphore */
static Semaphore_Handle applicationSem;
static Lpstk_Sensors lpstkSensors;
static uint16_t lpstkEvents;
static Lpstk_SensorEvMasks lpstkSensorMask;

/* this clock should expire SENSOR_STARTUP_TIME ms
 * after sensor_readClock */
static Clock_Struct sensor_startUpClock;
static Clock_Handle sensor_startUpClockHandle;
/* this clock should expire SENSOR_STARTUP_TIME before sensor_startUpClock */
static Clock_Struct sensor_readClock;
static Clock_Handle sensor_readClockHandle;
static Lpstk_SensorMask periodicReadSensors;
static Lpstk_AccelTiltCb accelTiltCB;

/******************************************************************************
 Local function prototypes
 *****************************************************************************/
static void setEvent(uint16_t eventMask);
static void clearEvent(uint16_t eventMask);
static void processSensorRead(Lpstk_SensorMask sensors, bool shutdown);
static void powerUpSensors(Lpstk_SensorMask sensors);
static void scAccelTaskAlertCallback(void);
static void sensorReadTimeoutCallback(UArg a0);

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 The application calls this function during initialization

 Public function defined in lpstk.h
 */
void Lpstk_init(void *evntHandle, Lpstk_AccelTiltCb accelCb)
{
    applicationSem = evntHandle;
    accelTiltCB = accelCb;

    Lpstk_initHumidityAndTempSensor(0, 0, 0, 0, NULL);
    Lpstk_initLightSensor(0, 0,NULL);
    // Hall effect sensor doesn't require init
    Lpstk_initSensorControllerAccelerometer(scAccelTaskAlertCallback);
    Lpstk_openAccelerometerSensor();
}

void Lpstk_initSensorReadTimer(Lpstk_SensorMask sensors, uint32_t clockPeriod)
{
    Clock_Params clockParams;

    periodicReadSensors = sensors;
    /* Convert clockDuration in milliseconds to ticks. */
    uint32_t clockTicks = SENSOR_STARTUP_TIME * (1000 / Clock_tickPeriod);

    /* Setup parameters. */
    Clock_Params_init(&clockParams);

    /* Setup argument. */
    clockParams.arg = LPSTK_EV_SENSOR_READ_AND_SHUTDOWN;

    /* If period is 0, this is a one-shot timer. */
    clockParams.period = 0;

    /*
     Starts immediately after construction if true, otherwise wait for a
     call to start.
     */
    clockParams.startFlag = false;

    /*/ Initialize clock instance. */
    Clock_construct(&sensor_startUpClock, sensorReadTimeoutCallback, clockTicks, &clockParams);
    sensor_startUpClockHandle = Clock_handle(&sensor_startUpClock);

    clockTicks = (clockPeriod - SENSOR_STARTUP_TIME) * (1000 / Clock_tickPeriod);

    /* Setup parameters. */
    //Clock_Params_init(&clockParams);

    /* Setup argument. */
    clockParams.arg = LPSTK_EV_SENSOR_POWER_UP;

    /* If period is 0, this is a one-shot timer. */
    clockParams.period = 0;

    /*
     Starts immediately after construction if true, otherwise wait for a
     call to start.
     */
    clockParams.startFlag = true;
    Clock_construct(&sensor_readClock, sensorReadTimeoutCallback, clockTicks, &clockParams);
    sensor_readClockHandle = Clock_handle(&sensor_readClock);
}

void Lpstk_setSensorReadTimer(Lpstk_SensorMask sensors, uint32_t clockPeriod)
{
    uint32_t clockTicks = (clockPeriod - SENSOR_STARTUP_TIME) * (1000 / Clock_tickPeriod);
    periodicReadSensors = sensors;

    /* Start clock instance */
    Clock_stop(sensor_readClockHandle);
    Clock_setTimeout(sensor_readClockHandle, clockTicks);
    /* Start clock instance */
    Clock_start(sensor_readClockHandle);
}

/*!
 The application must call this function periodically to
 process any events that this module needs to process.

 Public function defined in ssf.h
 */
void Lpstk_processEvents(void)
{
    if(!lpstkEvents)
    {
        return;
    }
    if(lpstkEvents & LPSTK_EV_FACTORY_DEFAULT)
    {

        clearEvent(LPSTK_EV_FACTORY_DEFAULT);
    }
    if(lpstkEvents & LPSTK_EV_SENSOR_POWER_UP)
    {
        powerUpSensors((Lpstk_SensorMask)lpstkSensorMask.powerupMask);

        if(sensor_readClockHandle && !Clock_isActive(sensor_readClockHandle))
        {
            /* Start clock instance */
            Clock_start(sensor_startUpClockHandle);
        }
        lpstkSensorMask.powerupMask = (Lpstk_SensorMask)0;
        clearEvent(LPSTK_EV_SENSOR_POWER_UP);
    }
    if(lpstkEvents & LPSTK_EV_SENSOR_READ_AND_SHUTDOWN)
    {
        processSensorRead((Lpstk_SensorMask)lpstkSensorMask.readAndShutdownMask, true);

        if(sensor_startUpClockHandle && !Clock_isActive(sensor_startUpClockHandle))
        {
            Clock_start(sensor_readClockHandle);
        }

        lpstkSensorMask.readAndShutdownMask = (Lpstk_SensorMask)0;
        clearEvent(LPSTK_EV_SENSOR_READ_AND_SHUTDOWN);
    }
    if(lpstkEvents & LPSTK_EV_SENSOR_READ)
    {
        processSensorRead((Lpstk_SensorMask)lpstkSensorMask.readMask, false);
        lpstkSensorMask.readMask = (Lpstk_SensorMask)0;
        clearEvent(LPSTK_EV_SENSOR_READ);
    }
    if(lpstkEvents & LPSTK_EV_SENSOR_SHUTDOWN)
    {
        shutDownSensors((Lpstk_SensorMask)lpstkSensorMask.shutdownMask);

        lpstkSensorMask.shutdownMask = (Lpstk_SensorMask)0;
        clearEvent(LPSTK_EV_SENSOR_SHUTDOWN);
    }
    if(lpstkEvents & LPSTK_EV_ACCEL_TILT_ALERT)
    {
        processSensorRead((Lpstk_SensorMask)LPSTK_ACCELEROMETER, false);
        // Acknowledge the ALERT event
        scifAckAlertEvents();

        if(accelTiltCB)
        {
            accelTiltCB();
        }

        clearEvent(LPSTK_EV_ACCEL_TILT_ALERT);
    }
}

//get last temp
float Lpstk_getTemperature()
{
    return lpstkSensors.temperature;
}
//get last hum
float Lpstk_getHumidity()
{
    return lpstkSensors.humidity;
}
//get last accel
void Lpstk_getAccelerometer(Lpstk_Accelerometer *accel)
{
    memcpy(accel, &lpstkSensors.accelerometer, sizeof(Lpstk_Accelerometer));
}
//get last hall effect switch value
bool Lpstk_getHallEffectSwitch()
{
    return lpstkSensors.halleffectSwitchValue;
}
//get last light
float Lpstk_getLux()
{
    return lpstkSensors.lux;
}

void shutDownSensors(Lpstk_SensorMask sensors)
{
    /* Humidity and temperature are measured by the same sensor */
    if(sensors & LPSTK_HUMIDITY || sensors & LPSTK_TEMPERATURE)
    {
        Lpstk_shutdownHumidityTempSensor();
    }
    if(sensors & LPSTK_LIGHT)
    {
        Lpstk_shutdownLightSensor();
    }
    if(sensors & LPSTK_ACCELEROMETER)
    {
        /* Accelerometer should be shutdown when another SPI device
         * will be in use from the main application given that the sensor
         * controller has no way of knowing that the SPI is currently in use*/
        Lpstk_shutdownAccelerometerSensor();
    }
}

static void setEvent(uint16_t eventMask)
{
    lpstkEvents |= eventMask;
    /* Wake up the application thread when it waits for keys event */
    Semaphore_post(applicationSem);
}

static void clearEvent(uint16_t eventMask)
{
    lpstkEvents ^= eventMask;

    /* Wake up the application thread when it waits for keys event */
    Semaphore_post(applicationSem);
}

/*!
 * @brief   Reading timeout handler function.
 *
 * @param   a0 - ignored
 */
static void sensorReadTimeoutCallback(UArg a0)
{
    if(a0 == LPSTK_EV_SENSOR_READ_AND_SHUTDOWN)
    {
        lpstkSensorMask.readAndShutdownMask = periodicReadSensors;
    }
    else
    {
        lpstkSensorMask.powerupMask = periodicReadSensors;
    }
    setEvent(a0);
}

static void scAccelTaskAlertCallback(void)
{
  // Clear the ALERT interrupt source
  scifClearAlertIntSource();
  // Trigger an event to read the accelerometer
  setEvent(LPSTK_EV_ACCEL_TILT_ALERT);
}

static void powerUpSensors(Lpstk_SensorMask sensors)
{
    /* Humidity and temperature are measured by the same sensor */
    if(sensors & LPSTK_HUMIDITY || sensors & LPSTK_TEMPERATURE)
    {
        Lpstk_openHumidityTempSensor();
    }
    if(sensors & LPSTK_LIGHT)
    {
        Lpstk_openLightSensor();
    }
    if(sensors & LPSTK_ACCELEROMETER)
    {
        Lpstk_openAccelerometerSensor();
    }
}

static void processSensorRead(Lpstk_SensorMask sensors, bool shutdown)
{
    /* Try to power up and open the sensors
     * if they are not already powered up */
    powerUpSensors(sensors);
    if(sensors & LPSTK_HUMIDITY)
    {
        Lpstk_readHumiditySensor(&lpstkSensors.humidity);
    }
    if(sensors & LPSTK_TEMPERATURE)
    {
        Lpstk_readTemperatureSensor(&lpstkSensors.temperature);
    }
    if(sensors & LPSTK_LIGHT)
    {
        Lpstk_readLightSensor(&lpstkSensors.lux);
    }
    if(sensors & LPSTK_ACCELEROMETER)
    {
        Lpstk_readAccelerometerSensor(&lpstkSensors.accelerometer);

        /* Accelerometer should only be shut down from a LPSTK_EV_SENSOR_SHUTDOWN
         * since the accelerometer is handled by the sensor controller
         * therefore un-set the accelerometer from the mask here to prevent it
         * from being shut down if shutdown == true*/
        sensors ^= LPSTK_ACCELEROMETER;
    }
    if(sensors & LPSTK_HALL_EFFECT)
    {
        Lpstk_readHallEffectSensor(&lpstkSensors.halleffectSwitchValue);
    }

    if(shutdown)
    {
        shutDownSensors(sensors);
    }
}
