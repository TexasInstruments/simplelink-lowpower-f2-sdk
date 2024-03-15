/**************************************************************************************************
  Filename:       gpd_temperaturesensor.c
  Revised:        $Date: 2017-8-15 16:04:46 -0700 (Tue, 15 Aug 2017) $
  Revision:       $Revision: 40796 $


  Description:    Green power device - Temperature Sensor application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application implements a Green Power Temperature Sensor, based on Z-Stack 3.0. It can be configured to
  have all supported security levels, application ID's or keys from gpd_config.h file. Some of these configurations are
  set as compilation flags.

  Application-specific UI peripherals being used:

  - LEDs:
    LED Red: Blinks if a frame(s) got transmitted but not all of them. If none of the frames got able to be transmitted then a solid On is set.

    LED Green: If all duplicate frames are send (GPDF_FRAME_DUPLICATES), then Green LED becomes solid On.

  Application-specific temperature sensor button actions:

  If a button is pressed less than a second is consider a short press, while more than a second is a long press.
  This is defined by GPD_LONG_PRESS_KEY_TICK (ms) and GPD_NUMBER_OF_TICKS.

    Short press:
    <BTN-1>: Increase temperature to be reported.
    <BTN-2>: Decrease temperature to be reported.

    Long press:
    <BTN-1>: Sends Attribute Reporting GPDF and trigger a timer to constantly send Attribute Reporting GPDF every 10 seconds (GPD_REPORT_TEMP_DELAY).
    <BTN-2>: Sends Commissioning GPDF

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "gpd_temperaturesensor.h"

#include "ti_drivers_config.h"
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>
#ifndef CUI_DISABLE
#include "cui.h"
#endif
#include <ti/sysbios/knl/Semaphore.h>
#include "api_mac.h"
#include "mac_util.h"
#include "string.h"
#include "gpd.h"
#include "gpd_memory.h"
#include "util_timer.h"

#include "ti_zstack_config.h"


/*********************************************************************
 * MACROS
 */

#define CONFIG_FH_ENABLE             false

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;
static uint8_t duplicates;
static ApiMac_mcpsDataReq_t gpdfDuplicate;

extern uint8_t _macTaskId;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
// Semaphore used to post events to the application thread
static Semaphore_Handle appSemHandle;


static ClockP_Handle ReportTempClkHandle;
static ClockP_Struct ReportTempClkStruct;

// Key press parameters
static Button_Handle keys = NULL;

// Task pending events
static uint16_t events = 0;

// Flag for commissioning message
static uint8_t commissioningMsg;

uint8_t tempKeyState = 0;
static Button_Handle gRightButtonHandle;
static Button_Handle gLeftButtonHandle;
static LED_Handle gGreenLedHandle;
static LED_Handle gRedLedHandle;
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void     gpdSampleTempSensor_initialization(void);
static void     gpdSampleTempSensor_changeKeyCallback(Button_Handle _btn, Button_EventMask _buttonEvents);
static void     gpdSampleTempSensor_processKey(Button_Handle keysPressed, Button_Events btnEvent);
static uint16_t gpdSampleTempSensor_process_loop( void );
static void     gpdSampleTempSensor_Init(void);
static void     gpdSampleTempSensor_processReportTempTimeoutCallback(UArg a0);
static void     gpdSampleTempSensor_initializeClocks(void);
static void     dataCnfCB(ApiMac_mcpsDataCnf_t *pDataCnf);
static void     Initialize_UI(void);
/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * STATUS STRINGS
 */

/*********************************************************************
 * REFERENCED EXTERNALS
 */

/******************************************************************************
 Callback tables
 *****************************************************************************/

/*! API MAC Callback table */
static ApiMac_callbacks_t Gpd_macCallbacks =
    {
      /*! Associate Indicated callback */
      NULL,
      /*! Associate Confirmation callback */
      NULL,
      /*! Disassociate Indication callback */
      NULL,
      /*! Disassociate Confirmation callback */
      NULL,
      /*! Beacon Notify Indication callback */
      NULL,
      /*! Orphan Indication callback */
      NULL,
      /*! Scan Confirmation callback */
      NULL,
      /*! Start Confirmation callback */
      NULL,
      /*! Sync Loss Indication callback */
      NULL,
      /*! Poll Confirm callback */
      NULL,
      /*! Comm Status Indication callback */
      NULL,
      /*! Poll Indication Callback */
      NULL,
      /*! Data Confirmation callback */
      dataCnfCB,
      /*! Data Indication callback */
      NULL,
      /*! Purge Confirm callback */
      NULL,
      /*! WiSUN Async Indication callback */
      NULL,
      /*! WiSUN Async Confirmation callback */
      NULL,
      /*! Unprocessed message callback */
      NULL
    };

/*******************************************************************************
 * @fn          sampleApp_task
 *
 * @brief       Application task entry point for the Z-Stack
 *              Sample Application
 *
 * @param       pfnNV - pointer to the NV functions
 *
 * @return      none
 */
void sampleApp_task(NVINTF_nvFuncts_t *pfnNV)
{
  #ifdef NV_RESTORE
  // Save and register the function pointers to the NV drivers
  pfnZdlNV = pfnNV;
  #endif

  // Initialize application
  gpdSampleTempSensor_initialization();

  // No return from task process
  gpdSampleTempSensor_process_loop();

  #ifndef NV_RESTORE
  //To avoid warnings.
  (void)pfnZdlNV;
  #endif
}

/*******************************************************************************
 * @fn          zcl_SampleLight_initialization
 *
 * @brief       Initialize the application
 *
 * @param       none
 *
 * @return      none
 */
static void gpdSampleTempSensor_initialization(void)
{
    /* Initialize user clocks */
    gpdSampleTempSensor_initializeClocks();

    Initialize_UI();

    //Initialize stack
    gpdSampleTempSensor_Init();

    memset(&duplicates, frameDuplicates, sizeof(uint8_t));

    gpdDuplicateFrameInit(&gpdfDuplicate);

    if(gp_nv_item_init(GP_NV_APP_BUTTON, sizeof(uint8_t), &commissioningMsg) == NV_ITEM_UNINIT)
    {
      commissioningMsg = 0;
    }
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
    if(gpdAESCCMInit() != SUCCESS)
    {
        while(true);
    }
#endif
}

/*******************************************************************************
 * @fn      gpdSampleTempSensor_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void gpdSampleTempSensor_initializeClocks(void)
{
    // Initialize the timers for reporting the temperature
    ReportTempClkHandle = UtilTimer_construct(
    &ReportTempClkStruct,
    gpdSampleTempSensor_processReportTempTimeoutCallback,
    GPD_REPORT_TEMP_DELAY,
    0, false, 0);
}

/*******************************************************************************
 * @fn      gpdSampleTempSensor_processReportTempTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void gpdSampleTempSensor_processReportTempTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    events |= SAMPLEAPP_REPORT_TEMP_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}

/*********************************************************************
 * @fn          gpdSampleTempSensor_Init
 *
 * @brief       Initialization function for the gpd layer.
 *
 * @param       none
 *
 * @return      none
 */
static void gpdSampleTempSensor_Init(void)
{
    /* Initialize the MAC */
    appSemHandle = ApiMac_init(_macTaskId, CONFIG_FH_ENABLE);

    /* Register the MAC Callbacks */
    ApiMac_registerCallbacks(&Gpd_macCallbacks);

#if defined(CONFIG_PA_TYPE) && (CONFIG_PA_TYPE==APIMAC_HIGH_PA)
    /* Set the PA type */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_paType,
                           (uint8_t)CONFIG_PA_TYPE);
#endif

    /* Set the transmit power */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyTransmitPowerSigned,
                           (uint8_t)TXPOWER);

#ifdef NV_RESTORE
    gp_appNvInit(pfnZdlNV);
#endif
}

/*********************************************************************
 * @fn          gpdSampleTempSensor_process_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
static uint16_t gpdSampleTempSensor_process_loop( void )
{
    /* Forever loop */
    for(;;)
    {
        if(events & SAMPLEAPP_KEY_EVT)
        {
            // Process Key Presses
            gpdSampleTempSensor_processKey(keys, Button_EV_CLICKED);

            keys = NULL;
            events &= ~SAMPLEAPP_KEY_EVT;
        }

        if(events & SAMPLEAPP_LONG_KEY_EVT)
        {
            // Process Key Presses
            gpdSampleTempSensor_processKey(keys, Button_EV_LONGCLICKED);

            keys = NULL;
            events &= ~SAMPLEAPP_LONG_KEY_EVT;
        }


        if( events & SAMPLEAPP_REPORT_TEMP_EVT )
        {
            LED_stopBlinking(gGreenLedHandle);
            LED_stopBlinking(gRedLedHandle);
            LED_setOff(gGreenLedHandle);
            LED_setOff(gRedLedHandle);

            GreenPowerAttributeReportingSend(ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, 1, &attributeReportCmd, gpdChannel);

            UtilTimer_setTimeout( ReportTempClkHandle, GPD_REPORT_TEMP_DELAY );
            UtilTimer_start(&ReportTempClkStruct);

            events &= ~SAMPLEAPP_REPORT_TEMP_EVT;
        }

        ApiMac_processIncoming();
    }
}

/*********************************************************************
 * @fn      gpdSampleTempSensor_changeKeyCallback
 *
 * @brief   Key event handler function
 *
 * @param   keysPressed - keys to be process in application context
 *
 * @return  none
 */
static void gpdSampleTempSensor_changeKeyCallback(Button_Handle _btn, Button_EventMask _buttonEvents)
{
    if (_buttonEvents & Button_EV_CLICKED)
    {
        keys = _btn;
        /* Start the device */
        Util_setEvent(&events, SAMPLEAPP_KEY_EVT);

        Semaphore_post(appSemHandle);
    }

    if (_buttonEvents & Button_EV_LONGCLICKED)
    {
        keys = _btn;
        Util_setEvent(&events, SAMPLEAPP_LONG_KEY_EVT);

        Semaphore_post(appSemHandle);
    }
}


/*********************************************************************
 * @fn      gpdSampleTempSensor_processKey
 *
 * @brief   Process the key pressed in the application context and start
 *          the timer for counting the ticks
 *
 * @param   none
 *
 * @return  none
 */
static void gpdSampleTempSensor_processKey(Button_Handle keysPressed, Button_Events btnEvent)
{
    if (btnEvent & Button_EV_LONGCLICKED)
    {
        //LongPress detected
        if(keysPressed == gLeftButtonHandle)
        {
            GreenPowerAttributeReportingSend(ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, 1, &attributeReportCmd, gpdChannel);
            UtilTimer_setTimeout( ReportTempClkHandle, GPD_REPORT_TEMP_DELAY );
            UtilTimer_start(&ReportTempClkStruct);
        }
        else if (keysPressed == gRightButtonHandle)
        {
            GreenPowerCommissioningSend( &commissioningReq, gpdChannel);
        }
    }
    else if (btnEvent & Button_EV_CLICKED)
    {
        //ShortPress detected
        if(keysPressed == gLeftButtonHandle)
        {
            gpd_attr_CurrentTemperature++;
        }
        else if (keysPressed == gRightButtonHandle)
        {
            gpd_attr_CurrentTemperature--;
        }
    }


    //clear the states
    LED_stopBlinking(gGreenLedHandle);
    LED_stopBlinking(gRedLedHandle);
    LED_setOff(gGreenLedHandle);
    LED_setOff(gRedLedHandle);
}

/*!
 * @brief      MAC Data Confirm callback.
 *
 * @param      pDataCnf - pointer to the data confirm information
 */
/*!
 * @brief      MAC Data Confirm callback.
 *
 * @param      pDataCnf - pointer to the data confirm information
 */
static void dataCnfCB(ApiMac_mcpsDataCnf_t *pDataCnf)
{
  /* Record statistics */
  if(pDataCnf->status == ApiMac_status_channelAccessFailure)
  {
      if(duplicates < frameDuplicates)
      {
          //Keep blinking to indicate fail to send all the duplicates
          LED_startBlinking(gRedLedHandle, 500, LED_BLINK_FOREVER);
      }
      else
      {
          //Fail and not a single frame got send
          LED_stopBlinking(gRedLedHandle);
          LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
      }
  }
  else if(pDataCnf->status == ApiMac_status_success)
  {
    if(duplicates > 0)
    {
      uint8_t secNum;
      ApiMac_mlmeGetReqUint8(ApiMac_attribute_dsn, (uint8_t*)&secNum);
      secNum--;
      ApiMac_mlmeSetReqUint8(ApiMac_attribute_dsn, secNum);
      ApiMac_mcpsDataReq(&gpdfDuplicate);
      duplicates--;
    }
    else
    {
      //Solid Green due to successful transmission
      LED_stopBlinking(gGreenLedHandle);
      LED_setOn(gGreenLedHandle, LED_BRIGHTNESS_MAX);
      OsalPort_free(gpdfDuplicate.msdu.p);
      duplicates = frameDuplicates;
    }
  }
}


/*******************************************************************************
 * @fn          Initialize_UI
 *
 * @brief       Initialize the User Interface
 *
 * @param       none
 *
 * @return      none
 */
static void Initialize_UI(void)
{
    /* Initialize btns */
    Button_Params bparams;
    Button_Params_init(&bparams);
    gLeftButtonHandle = Button_open(CONFIG_BTN_LEFT, &bparams);
    // Open Right button without appCallBack
    gRightButtonHandle = Button_open(CONFIG_BTN_RIGHT, &bparams);

    // Set button callback
    Button_setCallback(gLeftButtonHandle, gpdSampleTempSensor_changeKeyCallback);
    Button_setCallback(gRightButtonHandle, gpdSampleTempSensor_changeKeyCallback);

    /* Initialize the LEDS */
    LED_Params ledParams;
    LED_Params_init(&ledParams);
    gGreenLedHandle = LED_open(CONFIG_LED_GREEN, &ledParams);
    gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);
}
