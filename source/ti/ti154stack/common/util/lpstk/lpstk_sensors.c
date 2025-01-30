/******************************************************************************

 @file lpstk_sensors.c

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

/******************************************************************************
 Includes
 *****************************************************************************/
#include <unistd.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/ADC.h>

#include <ti/common/sail/hdc2010/hdc2010.h>
#include <ti/common/sail/opt3001/opt3001.h>
#include "lpstk/adxl362/scif.h"

#include "ti_drivers_config.h"

#include "lpstk/lpstk_sensors.h"

/*
 *  =============================== OPT3001 ===============================
 */
OPT3001_Object OPT3001_object[1];

const OPT3001_HWAttrs OPT3001_hwAttrs[1] = {
    {
        .slaveAddress = OPT3001_SA1,
        .gpioIndex = CONFIG_GPIO_OPT_INT,
    },
};

 OPT3001_Config OPT3001_config[] = {
    {
        .hwAttrs = &OPT3001_hwAttrs[0],
        .object  = &OPT3001_object[0],
    },
    {NULL, NULL},
};

/*
 *  =============================== HDC2010 ===============================
 */
HDC2010_Object HDC2010_object[1];

const HDC2010_HWAttrs HDC2010_hwAttrs[1] = {
    {
        .slaveAddress = HDC2010_SA2,
        .gpioIndex = CONFIG_GPIO_HDC_INT,
    },
};

 HDC2010_Config HDC2010_config[] = {
    {
        .hwAttrs = &HDC2010_hwAttrs[0],
        .object =  &HDC2010_object[0],
    },
    {NULL, NULL},
};



static I2C_Handle      i2cHandle;
static I2C_Params      i2cParams;
/* Change the offset value according to the working environment. For environment without
 *magnetic field the value of offset should be 1.65V for a 3.3V power supply
*/
#define OFFSET 1650.0f

static HDC2010_Handle  hdc2010Handle;
static HDC2010_Params  hdc2010Params;
static float humHiLim;
static float humLoLim;
static float tempHiLim;
static float tempLoLim;

static OPT3001_Handle  opt3001Handle;
static OPT3001_Params  opt3001Params;
static float luxHiLim;
static float luxLoLim;

static volatile bool sc_ready;

// SCIF driver callback: Task control interface ready (non-blocking task
// control opstatic eration completed)
static void scCtrlReadyCallback(void);

/* Local Functions */

static void closeI2C(void)
{
    if (!(hdc2010Handle || opt3001Handle))
    {
        I2C_close(i2cHandle);
        i2cHandle = NULL;
    }
}

/* Public Funtions */

void Lpstk_initHumidityAndTempSensor(float hHiLim, float hLoLim,
                                     float tHiLim, float tLoLim,
                                     GPIO_CallbackFxn hdc2010Callback)
{
    /* Call driver init functions */
    GPIO_init();
    I2C_init();
    HDC2010_init();
    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;

    /* Initialize hdc2010Params structure to defaults */
    HDC2010_Params_init(&hdc2010Params);
    hdc2010Params.humResolution = HDC2010_H14_BITS;
    hdc2010Params.tempResolution = HDC2010_T14_BITS;
    hdc2010Params.interruptPinPolarity = HDC2010_ACTIVE_LO;
    hdc2010Params.measurementMode = HDC2010_HT_MODE;

    if(hdc2010Callback)
    {
        hdc2010Params.interruptEn = HDC2010_ENABLE_MODE;
        hdc2010Params.interruptMask = (HDC2010_InterruptMask) (HDC2010_TH_MASK | HDC2010_TL_MASK | HDC2010_HH_MASK | HDC2010_HL_MASK);
        hdc2010Params.interruptMode = HDC2010_COMP_MODE;
        hdc2010Params.callback = hdc2010Callback;
        humHiLim = hHiLim;
        humLoLim = hLoLim;
        tempHiLim = tHiLim;
        tempLoLim = tLoLim;
    }
}

void Lpstk_initLightSensor(float hHiLim, float hLoLim,GPIO_CallbackFxn opt3001Callback)
{
    /* Call driver init functions */
    GPIO_init();
    I2C_init();
    OPT3001_init();
    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    /* Initialize opt3001Params structure to defaults */
    OPT3001_Params_init(&opt3001Params);

    /* Callback for INT pin event */
    opt3001Params.callback = opt3001Callback;
    if(opt3001Callback)
    {
        luxHiLim = hHiLim;
        luxLoLim = hLoLim;
    }
    else
    {
        luxHiLim = OPT3001_IGNORE;
        luxLoLim = OPT3001_IGNORE;
    }

}

void Lpstk_initHallEffectSensor()
{
    // TOOD: Shall this be kept?
    return;
}

void Lpstk_initSensorControllerAccelerometer(SCIF_VFPTR scTaskAlertCallback)
{
    // Initialize and start the Sensor Controller
    scifOsalInit();
    scifOsalRegisterCtrlReadyCallback(scCtrlReadyCallback);
    scifOsalRegisterTaskAlertCallback(scTaskAlertCallback);
    scifInit(&scifDriverSetup);
}

uint8_t Lpstk_openHumidityTempSensor(void)
{
    uint8_t openStatus = LPSTK_SUCCESS;
    if (i2cHandle == NULL)
    {
        i2cHandle = I2C_open(CONFIG_I2C_0, &i2cParams);
    }
    //check if sensor has already been open
    if (hdc2010Handle == NULL)
    {
        /* Open HDC2010 sensor with custom Params */
        hdc2010Handle = HDC2010_open(0, i2cHandle, &hdc2010Params);
        openStatus = LPSTK_NULL_HANDLE;
        if (hdc2010Handle != NULL)
        {
            openStatus = LPSTK_SUCCESS;
            /* Allow the sensor hardware to complete its first conversion */
            /* Set Object Temperature Alert Limits */
            if(hdc2010Params.callback)
            {
                if (!HDC2010_setTempLimit(hdc2010Handle, HDC2010_CELSIUS, tempHiLim, tempLoLim))
                {
                    openStatus = LPSTK_INIT_FAIL;
                }
                if (!HDC2010_setHumLimit(hdc2010Handle, humHiLim, humLoLim))
                {
                    openStatus = LPSTK_INIT_FAIL;
                }
            }
            HDC2010_triggerMeasurement(hdc2010Handle);
        }
    }
    return openStatus;
}

uint8_t Lpstk_openLightSensor(void)
{
    uint8_t openStatus = LPSTK_SUCCESS;
    if (i2cHandle == NULL)
    {
        i2cHandle = I2C_open(CONFIG_I2C_0, &i2cParams);
    }
    //check if sensor has already been open
    if (opt3001Handle == NULL)
    {
        /* Open OPT3001 sensor with custom parameters */
        opt3001Handle = OPT3001_open(0, i2cHandle, &opt3001Params);
        openStatus = LPSTK_NULL_HANDLE;
        /* Check if the open is successful */
        if(opt3001Handle != NULL)
        {
            openStatus = LPSTK_SUCCESS;
            /* Allow the sensor hardware to complete its first conversion */
            //sleep(1);
            /* Set Lux High and Low Limit Registers */
            if (!OPT3001_setLuxLimits(opt3001Handle, luxHiLim, luxLoLim))
            {
                openStatus = LPSTK_INIT_FAIL;
            }
            /* Enable interrupts from OPT3001 */
            OPT3001_enableInterrupt(opt3001Handle);
        }
    }
    return openStatus;
}

uint8_t Lpstk_openAccelerometerSensor(void)
{
    uint8_t status = LPSTK_SUCCESS;

    // Only enable if not already enabled
    if (!(scifGetActiveTaskIds() & (1 << SCIF_SPI_ACCELEROMETER_TASK_ID)))
    {
        // Start the "SPI Accelerometer" Sensor Controller task
        scifStartTasksNbl(1 << SCIF_SPI_ACCELEROMETER_TASK_ID);
        // Wait for sensor controller ready callback
        while (!sc_ready);
    }
    return status;
}

bool Lpstk_readTemperatureSensor(float *temperature)
{
    bool successRead = false;
    if(hdc2010Handle)
    {
        successRead = HDC2010_getTemp(hdc2010Handle, HDC2010_CELSIUS, temperature);
    }
    return successRead;
}

bool Lpstk_readHumiditySensor(float *humidity)
{
    bool successRead = false;
    if(hdc2010Handle)
    {
        successRead = HDC2010_getHum(hdc2010Handle, humidity);
    }
    return successRead;
}

bool Lpstk_readLightSensor(float *lux)
{
    bool successRead = false;
    if(opt3001Handle)
    {
        successRead = OPT3001_getLux(opt3001Handle, lux);
    }
    return successRead;
}

bool Lpstk_readHallEffectSensor(bool *switchValue)
{
    *switchValue = GPIO_read(CONFIG_GPIO_DRV);
    return true;
}

void Lpstk_readAccelerometerSensor(Lpstk_Accelerometer *accel)
{
    accel->x = scifTaskData.spiAccelerometer.output.x;
    accel->xTiltDet = scifTaskData.spiAccelerometer.output.xTiltDet;
    accel->y = scifTaskData.spiAccelerometer.output.y;
    accel->yTiltDet = scifTaskData.spiAccelerometer.output.yTiltDet;
    accel->z = scifTaskData.spiAccelerometer.output.z;
}

void Lpstk_shutdownHumidityTempSensor(void)
{
    if(hdc2010Handle != NULL)
    {
        /* close HDC2010 sensor with custom Params */
        HDC2010_close(hdc2010Handle);
        /* Set Handle to NULL */
        hdc2010Handle = NULL;
    }
    /* try to close i2c peripheral to enter low power mode */
    closeI2C();
}

void Lpstk_shutdownLightSensor(void)
{
    if(opt3001Handle != NULL)
    {
        /* close OPT3001 sensor with custom Params */
        OPT3001_close(opt3001Handle);
        /* Set Handle to NULL */
        opt3001Handle = NULL;
    }
    /* try to close i2c peripheral to enter low power mode */
    closeI2C();
}


void Lpstk_shutdownAccelerometerSensor(void)
{
    /* Only disable if currently enabled */
    if (scifGetActiveTaskIds() & (1 << SCIF_SPI_ACCELEROMETER_TASK_ID))
    {
      // Stop the "SPI Accelerometer" Sensor Controller task
      scifStopTasksNbl(1 << SCIF_SPI_ACCELEROMETER_TASK_ID);
      // Wait for sensor controller ready callback
      sc_ready = false;
    }
}

static void scCtrlReadyCallback(void)
{
  // Set ready flag
  sc_ready = true;
}
