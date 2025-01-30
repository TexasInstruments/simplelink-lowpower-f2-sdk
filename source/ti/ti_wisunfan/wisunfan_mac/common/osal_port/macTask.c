
/******************************************************************************

 @file  macTask.c

 @brief Mac Stack interface function implementation

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2013-2025, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#include "macTask.h"
#include "mac_main.h"
#include "mac_data.h"
#include "mac_pib.h"
#include "macs.h"
#include "mac_mgmt.h"
#include "mac_security.h"
#include "mac_security_pib.h"
#include "hal_mcu.h"
#include "macwrapper.h"

#include "crypto_mac_api.h"
#include <chipinfo.h>
#include <ti/drivers/utils/Random.h>

#ifdef FEATURE_SECURE_COMMISSIONING
#include <ti/drivers/TRNG.h>
#endif /* FEATURE_SECURE_COMMISSIONING */

#include "mac_symbol_timer.h"

#include "ti_drivers_config.h"

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SystemP.h>

#include <pthread.h>
#include <semaphore.h>

#include "rom_jt_154.h"

#include <stdint.h>

#include "osal_port.h"
#include "osal_port_timers.h"

#ifdef FEATURE_FREQ_HOP_MODE
#include "ti_wisunfan_config.h"
extern FHIE_channelPlan_t FHIE_channelPlan[];
#endif

// TODO: move this
#define OSALPORT_CLEAN_UP_TIMERS_EVT    0x4000

#ifdef FEATURE_WISUN_SUPPORT
extern void eventOS_scheduler_signal(void);
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

#define MAC_TASK_PRIORITY   4

#ifdef FEATURE_TIMAC_SUPPORT
#define MAC_TASK_STACK_SIZE 1500
#else
#define MAC_TASK_STACK_SIZE 2048
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */
pthread_t macThread;
sem_t macSemHandle;

static uint32_t macEvents = 0;
static uint32_t macTaskEvents = 0;
static uint8_t _macTaskId;

/* Size table for MAC structures */
const uint8 msacbackSizeTable [] =
{
  0,                                   /* unused */
  sizeof(macMlmeAssociateInd_t),       /* MAC_MLME_ASSOCIATE_IND */\
  sizeof(macMlmeAssociateCnf_t),       /* MAC_MLME_ASSOCIATE_CNF */
  sizeof(macMlmeDisassociateInd_t),    /* MAC_MLME_DISASSOCIATE_IND */
  sizeof(macMlmeDisassociateCnf_t),    /* MAC_MLME_DISASSOCIATE_CNF */
  sizeof(macMlmeBeaconNotifyInd_t),    /* MAC_MLME_BEACON_NOTIFY_IND */
  sizeof(macMlmeOrphanInd_t),          /* MAC_MLME_ORPHAN_IND */
  sizeof(macMlmeScanCnf_t),            /* MAC_MLME_SCAN_CNF */
  sizeof(macMlmeStartCnf_t),           /* MAC_MLME_START_CNF */
  sizeof(macMlmeSyncLossInd_t),        /* MAC_MLME_SYNC_LOSS_IND */
  sizeof(macMlmePollCnf_t),            /* MAC_MLME_POLL_CNF */
  sizeof(macMlmeCommStatusInd_t),      /* MAC_MLME_COMM_STATUS_IND */
#ifdef FEATURE_WISUN_SUPPORT
  sizeof(macMcpsDataCnfAck_t),         /* MAC_MCPS_DATA_CNF */
#else
  sizeof(macMcpsDataCnf_t),            /* MAC_MCPS_DATA_CNF */
#endif
  sizeof(macMcpsDataInd_t),            /* MAC_MCPS_DATA_IND */
  sizeof(macMcpsPurgeCnf_t),           /* MAC_MCPS_PURGE_CNF */
  sizeof(macEventHdr_t),               /* MAC_PWR_ON_CNF */
  sizeof(macMlmePollInd_t),            /* MAC_MLME_POLL_IND */
  sizeof(macMlmeWSAsyncCnf_t),         /* MAC_MLME_WS_ASYNC_CNF */
  sizeof(macMlmeWSAsyncInd_t)          /* MAC_MLME_WS_ASYNC_IND */
};

/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* Holds the value for query transmit, passed by the application.
 * A value of 0 indicates no continuation of retry,
 * A value of 1 indicates continue retransmission.
 */
static uint8 queryRetransmit = 0;

/* Holds the value for pending indirect msgs, passed by the application.
 * A value of 0 indicates no pending indirect msg.
 */
static uint8 checkPending = 0;

/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* The Application will send the address of the MAC_CbackQueryRetransmit()
 * as part of MAC_INIT message. Save to function pointer below
 */
uint8 (*pMacCbackQueryRetransmit)() = NULL;

/* The Application will send the address of the MAC_CbackCheckPending()
 * as part of MAC_INIT message. Save to function pointer below
 */
uint8 (*pMacCbackCheckPending)() = NULL;

uint8_t AppTaskId;

#ifdef FEATURE_SECURE_COMMISSIONING
extern TRNG_Handle TRNG_handle;
#endif /* FEATURE_SECURE_COMMISSIONING */

/* ------------------------------------------------------------------------------------------------
 *                                           Functions
 * ------------------------------------------------------------------------------------------------
 */
static void macApp(macCmd_t *pMsg);
static void macInit(macUserCfg_t *pUserCfg);
static void *macTaskFxn(void *a0);
extern uint8 MAC_MlmeGetReqSize( uint8 pibAttribute );
extern uint8 MAC_MlmeGetSecurityReqSize( uint8 pibAttribute );
extern uint8 MAC_MlmeFHGetReqSize( uint16 pibAttribute );

/**************************************************************************************************
 * @fn          macTaskInit
 *
 * @brief       This function is called initialize the MAC task.
 *
 * input parameters
 *
 * @param       pUserCfg - MAC user config
 *
 * output parameters
 *
 *
 * @return      MAC Task ID.
 **************************************************************************************************
 */
uint8_t macTaskInit(macUserCfg_t *pUserCfg)
{
    /* create semaphores for messages / events
     */
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    /* create semaphores for messages / events
     */
    retc = sem_init(&macSemHandle, 0, 0);
    if (retc != 0) {
        while (1);
    }

    /* create the Mac Thread
     */

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = MAC_TASK_PRIORITY;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, MAC_TASK_STACK_SIZE);

    if (retc != 0) {
        /* failed to set attributes */
        while (1) {}
    }

    retc = pthread_create(&macThread, &attrs, macTaskFxn, pUserCfg);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1) {}
    }
#ifdef TIMAC_ROM_IMAGE_BUILD
  TIMAC_ROM_Init();
#endif

    _macTaskId = OsalPort_registerTask(macThread, &macSemHandle, &macTaskEvents);
    macMainSetTaskId(_macTaskId);

    return _macTaskId;
}

/**************************************************************************************************
 * @fn          macTaskGetTaskHndl
 *
 * @brief       This function returns the TIRTOS Task handle of the MAC Task.
 *
 * input parameters
 *
 * @param       pUserCfg - MAC user config
 *
 * output parameters
 *
 *
 * @return      MAC Task ID.
 **************************************************************************************************
 */
void* macTaskGetTaskHndl(void)
{
    return macThread;
}

#ifdef FEATURE_FREQ_HOP_MODE
/*
 * 0 ==> regulatory domain
 * 1 ==> explicit freq
 */
uint8_t FH_ChanPlan = 0;

void macFH_ChanPlanInit(void)
{
    uint8_t i;
    FHIE_channelPlan_t *pChPlan;

    /* init the FH plan to zero */
    for(i=0; i < NO_OF_CHANNEL_PLAN; i++)
    {
        pChPlan = &(FHIE_channelPlan[i]);
        memset(pChPlan,0x00,sizeof(FHIE_channelPlan_t));
    }
    /* base the CONFIG_CUSTOM_PHY, we decide the chanplan 0 or1
     * WS bootstrap : ws_channel_plan_zero_compare (Wi-SUN Spec)
     * WS bootstrap : ws_channel_plan_one_compare (customer design)
     */
    if (CONFIG_CUSTOM_PHY == false)
        FH_ChanPlan = 0;
    else
        FH_ChanPlan = 1;

    /* set up the FH chan plan
     * in order to reuse the existing code, we keep use the FHIE_channelPlan
     * we hardcode the first entry in the ChanPlan and all remaining entries are
     * not used.
     */
    FHIE_channelPlan[0].ch0 = CONFIG_CENTER_FREQ *1000;
    FHIE_channelPlan[0].noOfChannels = CONFIG_TOTAL_CHANNELS;
    FHIE_channelPlan[0].operatingClass = CONFIG_OP_MODE_CLASS;

    FHIE_channelPlan[0].regulatoryDomain = CONFIG_REG_DOMAIN ;

    /* update the exclude channel list for node's USIE/BSIE
     * regChanList bit value: 1 == valid regulation channel
     * see PHY Spec 2V00 (specification, page #18)
     */
    uint8_t regChanList[MAX_NO_OF_CHANNELS] = CONFIG_REGULATION_CHANNEL_MASK; // Sysconfig defined channel mask based regulatory domain and operating class.
    MAP_osal_memcpy(FHIE_channelPlan[0].regChannelList, &regChanList, MAX_NO_OF_CHANNELS);

    if (CONFIG_CHANNEL_SPACING == 100)
        FHIE_channelPlan[0].channelSpacing = 3;
    else
    {
        FHIE_channelPlan[0].channelSpacing = CONFIG_CHANNEL_SPACING/200 -1;
    }
}
#endif

/**************************************************************************************************
 * @fn          macInit
 *
 * @brief       This function is called to initialize the MAC.
 *
 * input parameters
 *
 * @param       pUserCfg - MAC user config
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void macInit(macUserCfg_t *pUserCfg)
{
  uint32_t key;
#ifndef USE_DMM
  int_fast16_t status;
#endif /* USE_DMM */

#ifdef FEATURE_MAC_SECURITY
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC13X4)
  CryptoCC26XX_Params CryptoCC26XXParams;
#elif !defined(BLE_START)
  AESCCM_Params AESCCMParams;
#ifdef FEATURE_SECURE_COMMISSIONING
  TRNG_Params TRNGParams;
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif
#endif //FEATURE_MAC_SECURITY

#if defined( DEBUG_SW_TRACE )
  #if defined(HAL_UART) && (HAL_UART==TRUE)
    // Enable tracer output on DIO24/ATEST1.
    // Set max drive strength and reduced slew rate.
    // PortID = 46 = RFcore tracer.
    HWREG(IOC_BASE + IOC_O_IOCFG24) = (3 << IOC_IOCFG24_IOSTR_S) | (1 << IOC_IOCFG24_SLEW_RED_S) | 46;

  #else // no UART so allow trace on Tx pin - Debug Package only

    // Enable tracer output on DIO23/ATEST0 or DIO24/ATEST1:
    // For 7x7 EM on SmartRF06EB:
    // Port ID for RF Tracer is 46
    // [see ioc.h: #define IOC_PORT_RFC_TRC  0x0000002E // RF Core Tracer]

    // DIO 23 maps to IOID 8
    // [see Board.h: #define Board_SPI0_MISO IOID_8     /* RF1.20 */]
    HWREG(IOC_BASE + IOC_O_IOCFG23) = (3 << IOC_IOCFG23_IOSTR_S) | 46;

    // OR

    // DIO 24 maps to IOID 24
    // [see Board.h: #define Board_SPI1_MISO IOID_24   /* RF2.10 */]
    //HWREG(IOC_BASE + IOC_O_IOCFG24) = (3 << IOC_IOCFG24_IOSTR_S) | 46;

  #endif // HAL_UART
#endif // DEBUG_SW_TRACE

  /* User App Reconfiguration of TIMAC */
  macSetUserConfig( pUserCfg );

  /* Disable interrupts
   */
  key = HwiP_disable();

#ifndef USE_DMM
  // Setup the PRNG
  status = Random_seedAutomatic();

  if (status != Random_STATUS_SUCCESS) {
      while(1);
//      SystemP_abort("Random_seedAutomatic() failed");
  }
#endif /* USE_DMM */

#ifdef FEATURE_MAC_SECURITY
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC13X4)
  extern CryptoCC26XX_Handle Crypto_handle;

  CryptoCC26XX_init();
  CryptoCC26XX_Params_init(&CryptoCC26XXParams);
  Crypto_handle = CryptoCC26XX_open(Board_CRYPTO0, false, &CryptoCC26XXParams);
  if (!Crypto_handle)
  {
    /* abort */
      while(1);
//      SystemP_abort("Crypto open failed");
  }

  HalAesInit();
#else
#if !defined(USE_DMM) || !defined(BLE_START)
  AESCCM_init();
  AESCCM_Params_init(&AESCCMParams);
  AESCCMParams.returnBehavior = AESCCM_RETURN_BEHAVIOR_POLLING;
  AESCCM_handle = AESCCM_open(CONFIG_AESCCM_0, &AESCCMParams);
#ifdef FEATURE_SECURE_COMMISSIONING
  TRNG_init();
  TRNG_Params_init(&TRNGParams);
  TRNGParams.returnBehavior = TRNG_RETURN_BEHAVIOR_POLLING;
  TRNG_handle = TRNG_open(CONFIG_TRNG_0, &TRNGParams);
#endif /* FEATURE_SECURE_COMMISSIONING */
#else
  //BLE Stack has opened the AESCCM driver
  extern AESCCM_Handle encHandleCCM;
  AESCCM_handle = encHandleCCM;

  // BLE Stack has opened the AESECB driver
  extern AESECB_Handle encHandleECB;
  extern AESECB_Handle AESECB_handle;
  AESECB_handle = encHandleECB;

#ifdef FEATURE_SECURE_COMMISSIONING
  //Open separate TRNG driver instance than BLE Stack
  TRNG_Params trngParams;
  TRNG_init();
  TRNG_Params_init(&trngParams);
  trngParams.returnBehavior = TRNG_RETURN_BEHAVIOR_POLLING;
  TRNG_handle = TRNG_open(CONFIG_TRNG_1, &trngParams);
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif /*!USE_DMM */
  if (!AESCCM_handle)
  {
      /* abort */
      while(1);
//        SystemP_abort("Crypto open failed");
  }
#ifdef FEATURE_SECURE_COMMISSIONING
  if (!TRNG_handle)
  {
      /* abort */
      while(1);
      // SystemP_abort("TRNG open failed");
  }
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif /*!defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC13X4)*/
#endif /* FEATURE_MAC_SECURITY */

#ifdef FEATURE_WISUN_SUPPORT
  /*
   *  start timac tasklet
   */
  timac_tasklet_init();
#endif

  /* Initialize MAC */
  MAC_Init();

#ifdef COPROCESSOR
  /* Initialize the MAC function tables and features */
  MAC_InitDevice();
#ifdef FEATURE_FULL_FUNCTION_DEVICE
  MAC_InitCoord();
#endif /* FEATURE_FULL_FUNCTION_DEVICE */
#else /* ! COPROCESSOR */
  /* Initialize the MAC function tables and features */

#ifdef FEATURE_FULL_FUNCTION_DEVICE
  MAC_InitCoord();
#else /* FEATURE_FULL_FUNCTION_DEVICE */
  MAC_InitDevice();
#endif
#endif /* COPROCESSOR */

#ifdef FEATURE_FREQ_HOP_MODE
  MAC_InitFH();
  macFH_ChanPlanInit();
#endif /* FEATURE_FREQ_HOP_MODE */

   /* Initialize MAC buffer */
  macLowLevelBufferInit();

  /* Enable interrupts
   */
  HwiP_restore(key);

  // Must be done last
  macLowLevelInit();
}

extern void timac_LogEvent(uint32_t eventID,uint32_t timestamp,uint32_t st1,uint32_t st2,uint32_t st3,uint32_t st4);

/**************************************************************************************************
 * @fn          macTaskFxn
 *
 * @brief       This function is the main event handling function of the MAC executing
 *              in task context.  This function is called by OSAL when an event or message
 *              is pending for the MAC.
 *
 * input parameters
 *
 * @param       a0  Pointer to the MAC User Config structure
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
#include "dbg.h"
static void *macTaskFxn(void *a0)
{
  macEvent_t          *pMsg;
  macEventHdr_t       hdr;
  uint32_t            key;
  uint8               status;
//  uint32_t            macEvents = 0;

  macUserCfg_t* userCfg = (macUserCfg_t*) a0;

  /* initialize the MAC
   */
  macInit(userCfg);

  OsalPortTimers_registerCleanupEvent(_macTaskId, OSALPORT_CLEAN_UP_TIMERS_EVT);

  while(1)
  {
      macEvents = OsalPort_waitEvent(_macTaskId);

      OsalPort_clearEvent(_macTaskId, macEvents);
      DBG_PRINT1(0, "macEvents:%d", macEvents);

      /* handle events on rx queue */
      if (macEvents & MAC_RX_QUEUE_TASK_EVT)
      {
        while ((pMsg = (macEvent_t *) OsalPort_msgDequeue(&macData.rxQueue)) != NULL)
        {
          /* Enter critical section
           */
          key = HwiP_disable();

          macData.rxCount--;

          /* Exit critical section
           */
          HwiP_restore(key);

          hdr.event = pMsg->hdr.event;

          macMain.pBuf = (uint8 *)pMsg;

          /* Check security processing result from the LMAC */
          status = pMsg->hdr.status;

          if (status == MAC_SUCCESS)
          {
            macExecute(pMsg);
          }
    #ifdef FEATURE_MAC_SECURITY
          else if ((status  == MAC_IMPROPER_KEY_TYPE) ||
                   (status  == MAC_IMPROPER_SECURITY_LEVEL) ||
                   (status  == MAC_SECURITY_ERROR) ||
                   (status  == MAC_UNAVAILABLE_KEY) ||
                   (status  == MAC_UNSUPPORTED_LEGACY) ||
                   (status  == MAC_UNSUPPORTED_SECURITY) ||
                   (status  == MAC_INVALID_PARAMETER) ||
                   (status  == MAC_COUNTER_ERROR))
          {
            macCommStatusInd(pMsg);

            /* discard invalid DATA_IND frame */
            if (hdr.event == MAC_RX_DATA_IND_EVT)
            {
              mac_msg_deallocate((uint8 **)&pMsg);
            }
          }
    #else
          else
          {
            /* discard invalid DATA_IND frame */
            if (hdr.event == MAC_RX_DATA_IND_EVT)
            {
              mac_msg_deallocate((uint8 **)&pMsg);
            }
          }
    #endif /* FEATURE_MAC_SECURITY */

          /* discard all frames except DATA_IND.
           * do not free data frames for app
           */
          if (hdr.event != MAC_RX_DATA_IND_EVT)
          {
              mac_msg_deallocate(&macMain.pBuf);
          }
        }
      }

      /* handle events on osal msg queue */
      if (macEvents & OsalPort_SYS_EVENT_MSG)
      {
        while ((pMsg = (macEvent_t *) OsalPort_msgReceive(_macTaskId)) != NULL)
        {
          if(pMsg->hdr.event >= 0xD0)
          {
            /* 0xE0 to 0xFF is reserved for App to send messages to TIMAC */
            macApp((macCmd_t *)pMsg);
          }
          else
          {
            macMain.pBuf = (uint8 *)pMsg;
            /* execute state machine */
            macExecute(pMsg);
            mac_msg_deallocate(&macMain.pBuf);
          }
        }
      }


      /* handle tx complete */
      if (macEvents & MAC_TX_COMPLETE_TASK_EVT)
      {
        hdr.event = MAC_INT_TX_COMPLETE_EVT;
        macExecute((macEvent_t *) &hdr);
      }
      /* handle start complete */
      if (macEvents & MAC_START_COMPLETE_TASK_EVT)
      {
        hdr.status = MAC_SUCCESS;
        hdr.event = MAC_INT_START_COMPLETE_EVT;
        macExecute((macEvent_t *) &hdr);
      }
      if (macEvents & MAC_CSMA_TIM_TASK_EVT)
      {
        hdr.status = MAC_SUCCESS;
        hdr.event = MAC_CSMA_TIM_EXP_EVT ;
        macExecute((macEvent_t *) &hdr);
      }
      if (macEvents & MAC_TX_BACKOFF_TIM_TASK_EVT)
      {
        hdr.status = MAC_SUCCESS;
        hdr.event = TX_BACKOFF_TIM_EXP_EVT;
        macExecute((macEvent_t *) &hdr);
      }

      if (macEvents & MAC_RX_BACKOFF_TIM_TASK_EVT)
      {
        hdr.status = MAC_SUCCESS;
        hdr.event =  RX_BACKOFF_TIM_EXP_EVT;
        macExecute((macEvent_t *) &hdr);
      }

      if (macEvents & MAC_MSG_LOG_EVT)
      { /* use the nanostack trace to log message */
          uint16_t st;
          MacMsgEvtLog_s MacLog;

          // dump all messages
          do
          {
              st = macReadLogMsg(&MacLog);
              if (st ==1 )
              { // log event
                timac_LogEvent(MacLog.msgLogID, MacLog.timestamp,MacLog.st1,MacLog.st2,
                               MacLog.st3,MacLog.st4);
              }
          } while (st == 1);

      }

      if (macEvents & OSALPORT_CLEAN_UP_TIMERS_EVT)
      {
          OsalPortTimers_cleanUpTimers();
      }
      /* handle pending message, if any */
      if (macMain.pPending != NULL)
      {
        uint32_t key;

        /* Disable interrupts
         */
        key = HwiP_disable();

        pMsg = macMain.pPending;
        macMain.pPending = NULL;

        /* Enable interrupts
         */
        HwiP_restore(key);

        macMain.pBuf = (uint8 *)pMsg;

        /* execute state machine */
        macExecute(pMsg);

        mac_msg_deallocate(&macMain.pBuf);
      }

      macEvents = 0;

  }
}

/**************************************************************************************************
 * @fn          macApp
 *
 * @brief       This function is called when Application sends MAC Stack a message.
 *
 * input parameters
 *
 * @param       pMsg - pointer to macCmd_t structure
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macApp(macCmd_t *pMsg)
{
  uint8 dealloc = TRUE;
  /*
   * Set this flag to TRUE to send the message back to the app at the end
   * of the function.
   */
  uint8 sendMsg = FALSE;

  switch (pMsg->hdr.event)
  {
  case MAC_STACK_INIT_PARAMS:
   AppTaskId = pMsg->macInit.srctaskid;
   queryRetransmit = pMsg->macInit.retransmit;
   checkPending = pMsg->macInit.pendingMsg;
   pMacCbackQueryRetransmit = pMsg->macInit.pMacCbackQueryRetransmit;
   pMacCbackCheckPending = pMsg->macInit.pMacCbackCheckPending;
   dealloc = TRUE;
   break;
  }

  if(sendMsg == TRUE)
  {
    /* send message to App */
    OsalPort_msgSend(AppTaskId, (uint8 *)pMsg);
  }

  if(dealloc)
  {
    OsalPort_msgDeallocate((uint8 *)pMsg);
  }
}

/**************************************************************************************************
 *
 * @fn          MAC_CbackEvent
 *
 * @brief       This callback function sends MAC events to the application.
 *              The application must implement this function.  A typical
 *              implementation of this function would allocate an OSAL message,
 *              copy the event parameters to the message, and send the message
 *              to the application's OSAL event handler.  This function may be
 *              executed from task or interrupt context and therefore must
 *              be reentrant.
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      None.
 *
 **************************************************************************************************
*/
void MAC_CbackEvent(macCbackEvent_t *pData)
{
  macCbackEvent_t *pMsg = NULL;

  uint8 len = msacbackSizeTable[pData->hdr.event];

  switch (pData->hdr.event)
  {
    case MAC_MCPS_DATA_IND:
    case MAC_MLME_WS_ASYNC_IND:
      pMsg = pData;
      break;

    default:
      pMsg = (macCbackEvent_t *)OsalPort_msgAllocate(len);
      if ((pMsg == NULL) && (pData->hdr.event == MAC_MCPS_DATA_CNF)
                      && (pData->dataCnf.pDataReq != NULL))
      {
        /*
         * The allocation failed for the data confirm, so we will
         * try to deallocate the memory used by the data request.
         * [OsalPort_msgDeallocate() will NULL pDataReq]
         */
          mac_msg_deallocate((uint8**)&(pData->dataCnf.pDataReq));

        /* Then, try the allocation again */
        pMsg = (macCbackEvent_t *) OsalPort_msgAllocate(len);
      }

      if (pMsg != NULL)
      {
        memcpy(pMsg, pData, len);
      }
      break;
  }

  if (pMsg != NULL)
  {
    OsalPort_msgSend(AppTaskId, (uint8 *) pMsg);
#ifdef FEATURE_WISUN_SUPPORT
    /*
     * post TI MAC event and wake up event handler
     */
    timacSignalEventLoop();
#endif
  }
}

/**************************************************************************************************
 * @fn          MAC_CbackQueryRetransmit
 *
 * @brief       This function callback function returns whether or not to continue MAC
 *              retransmission.
 *              A return value '0x00' will indicate no continuation of retry and a return value
 *              '0x01' will indicate to continue retransmission. This callback function shall be
 *              used to stop continuing retransmission for RF4CE.
 *              MAC shall call this callback function whenever it finishes transmitting a packet
 *              for macMaxFrameRetries times.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      0x00 to stop retransmission, 0x01 to continue retransmission.
 **************************************************************************************************
*/
uint8 MAC_CbackQueryRetransmit(void)
{
  uint8 reTransmit = queryRetransmit;

  if ( pMacCbackQueryRetransmit )
  {
    reTransmit = (*pMacCbackQueryRetransmit)();
  }

  return reTransmit;
}

/**************************************************************************************************
 * @fn          MAC_CbackCheckPending
 *
 * @brief       This callback function returns the number of pending indirect messages queued in
 *              the application. Most applications do not queue indirect data and can simply
 *              always return zero. The number of pending indirect messages only needs to be
 *              returned if macCfg.appPendingQueue to TRUE.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The number of indirect messages queued in the application or zero.
 **************************************************************************************************
*/
uint8 MAC_CbackCheckPending(void)
{
  uint8 pend = checkPending;

  if ( pMacCbackCheckPending )
  {
    pend = (*pMacCbackCheckPending)();
  }

  return pend;
}



