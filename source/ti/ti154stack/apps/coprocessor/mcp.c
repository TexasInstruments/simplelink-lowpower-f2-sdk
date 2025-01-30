/******************************************************************************

 @file  mcp.c

 @brief TIMAC Co-Processor Application

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
#include <string.h>

#include "api_mac.h"
#include "npi_task.h"

#include "mcp.h"
#include "mt.h"
#include "mt_mac.h"

#include "ti_drivers_config.h"
#ifdef TI154STACK_BLINK_LEDS_DURING_MESSAGING
#include "board_led.h"
#endif
#include "util_timer.h"

/* use macNpiMTMsg_t */
#include "mac_api.h"

#ifdef MAC_DUTY_CYCLE_CHECKING
#include "mac_duty_cycle/mac_duty_cycle.h"
#endif

/******************************************************************************
 External variables
 *****************************************************************************/
#ifdef OSAL_PORT2TIRTOS
extern uint8_t appTaskId;
extern uint8_t npiAppEntityID;
#else
/* ICall thread entity for ApiMac */
extern ICall_EntityID ApiMac_appEntity;

/* ICall thread entity for NPI */
extern ICall_EntityID npiAppEntityID;
#endif

/******************************************************************************
 Local variables
 *****************************************************************************/
/* Number of requests sent to MAC */
static uint16_t numRxMsgs = 0;

/* Number of responses sent to host */
static uint16_t numTxMsgs = 0;

/******************************************************************************
 Local function prototypes
 *****************************************************************************/

static void processMsg(uint16_t p1, uint16_t p2, void *pMsg);
static void relayTxMsg(void *pMsg);
static void relayRxMsg(void *pMsg);

/******************************************************************************
 ApiMac MAC callback table
 *****************************************************************************/
/*!
 API MAC Callback table
 */
static ApiMac_callbacks_t macCallbacks =
{
    /*! Associate Indication callback */
    MtMac_AssociateInd,
    /*! Associate Confirmation callback */
    MtMac_AssociateCnf,
    /*! Disassociate Indication callback */
    MtMac_DisassociateInd,
    /*! Disassociate Confirmation callback */
    MtMac_DisassociateCnf,
    /*! Beacon Notify Indication callback */
    MtMac_BeaconNotifyInd,
    /*! Orphan Indication callback */
    MtMac_OrphanInd,
    /*! Scan Confirmation callback */
    MtMac_ScanCnf,
    /*! Start Confirmation callback */
    MtMac_StartCnf,
    /*! Sync Loss Indication callback */
    MtMac_SyncLossInd,
    /*! Poll Confirm callback */
    MtMac_PollCnf,
    /*! Comm Status Indication callback */
    MtMac_CommStatusInd,
    /*! Poll Indication Callback */
    MtMac_PollInd,
    /*! Data Confirmation callback */
    MtMac_DataCnf,
    /*! Data Indication callback */
    MtMac_DataInd,
    /*! Purge Confirm callback */
    MtMac_PurgeCnf,
    /*! WiSUN Async Indication callback */
    MtMac_AsyncInd,
    /*! WiSUN Async Confirmation callback */
    MtMac_AsyncCnf,
    /*! Unprocessed message callback */
    processMsg
};

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Application task processing.

 Public function defined in mcp.h
 */
#ifdef OSAL_PORT2TIRTOS
void MCP_task(uint8_t _macTaskId)
#else
void MCP_task(void)
#endif
{
    /* Initialize this application */
#ifdef OSAL_PORT2TIRTOS
    MCP_init(_macTaskId);
#else
    MCP_init();
#endif
    while(true)
    {
        /* Wait for MAC/NPI messages */
        ApiMac_processIncoming();
    }
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief   Initialize this application
 */
#ifdef OSAL_PORT2TIRTOS
void MCP_init(uint8_t macTaskId)
#else
void MCP_init(void)
#endif
{
    /* Initialize the MAC interface, do not enable FH */
#ifdef OSAL_PORT2TIRTOS
    ApiMac_init(macTaskId,false);
#else
    (void)ApiMac_init(false);
#endif
    /* Register the MAC Callbacks */
    ApiMac_registerCallbacks(&macCallbacks);

#ifdef MAC_DUTY_CYCLE_CHECKING
    ApiMac_mlmeSetReqBool(ApiMac_attribute_dutyCycleEnabled, true);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleRegulated,
                            DUTY_CYCLE_MEAS_PERIOD*MAC_DUTY_CYCLE_THRESHOLD/100);

    /* Critical and limited duty cycle modes unused, set to max
     * value to avoid entering state */
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleCritical,
                            UINT32_MAX);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleLimited,
                            UINT32_MAX);
#endif

#ifdef MAC_OVERRIDE_TX_DELAY
    ApiMac_mlmeSetReqBool(ApiMac_attribute_customMinTxOffEnabled, true);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_minTxOffTime,
                            (uint32_t)MAC_CONFIG_MIN_TX_OFF);
#endif

    /* Register callback function for incoming NPI messages */
    NPITask_registerIncomingRXEventAppCB((npiIncomingEventCBack_t)relayRxMsg,
                                                                  INTERCEPT);
    /* Register callback function for outgoing NPI messages */
    NPITask_registerIncomingTXEventAppCB((npiIncomingEventCBack_t)relayTxMsg,
                                                                  ECHO);

#ifdef OSAL_PORT2TIRTOS
    /* Configure and create the NPI task */
    NPITask_createTask(macTaskId);
    /* Start up the MT message handler */
    MT_init(npiAppEntityID, npiAppEntityID);
#else
    /* Configure and create the NPI task */
    NPITask_createTask(0);
    /* Start up the MT message handler */
    MT_init(npiAppEntityID, ICALL_SERVICE_CLASS_NPI);
#endif

#ifdef TI154STACK_BLINK_LEDS_DURING_MESSAGING
    /* Turn off all LEDs */
    Board_Led_initialize();
#endif
}

/*!
 * @brief   Process a message from ApiMac
 *
 * @param   entityID - ICall application entity ID
 * @param   param2   - not used
 * @param   pMsg     - pointer to incoming message buffer
 */
static void processMsg(uint16_t entityID, uint16_t param2, void *pMsg)
{
    /* Intentionally not used */
    (void)param2;
    (void)entityID;

#ifdef OSAL_PORT2TIRTOS

#else
    /* Verify it's an NPI message */
    //if(entityID == npiAppEntityID)
#endif
    {
        /* Hand it to MT incoming message handler */
        MT_processIncoming(pMsg);

        /* Count an incoming MT message */
        numRxMsgs += 1;
#ifdef TI154STACK_BLINK_LEDS_DURING_MESSAGING
        Board_Led_control(board_led_type_LED1, board_led_state_BLINK);
#endif
    }
}

/*!
 * @brief   Relay an NPI task RX message to ApiMac task,
 *          allows MT message processing at ApiMac task context
 *
 * @param   pMsg - pointer to incoming message buffer
 */
static void relayRxMsg(void *pMsg)
{
    NPIMSG_msg_t *pNpiMsg = pMsg;

    macNpiMtMsg_t *pMacNpiMtMsg;
    uint16_t mtMsgLen;
    uint8_t *pData;

#ifdef OSAL_PORT2TIRTOS
    OsalPort_MsgHdr* hdr = (OsalPort_MsgHdr *) (pNpiMsg->pBuf) - 1;
#else
    ICall_MsgHdr *hdr = (ICall_MsgHdr *) (pNpiMsg->pBuf) - 1;
#endif
    mtMsgLen = hdr->len;

    /* Allocate memory for the data */
    pMacNpiMtMsg = (macNpiMtMsg_t *) MAP_ICall_allocMsg(sizeof(macNpiMtMsg_t) + mtMsgLen);

    if (pMacNpiMtMsg == NULL)
    {
        MAP_ICall_free(pNpiMsg);
        return;
    }

    /* set up the event */
    pMacNpiMtMsg->hdr.event = MAC_NCP_MT_MSG;

    /* copy message to new created MAC NPI MT Msg */
    pData = (uint8_t *)pMacNpiMtMsg + sizeof(macNpiMtMsg_t);
    memcpy(pData,pNpiMsg->pBuf,mtMsgLen);

    /* free old allocated MSg */
    MAP_ICall_freeMsg(pNpiMsg->pBuf);

#ifdef OSAL_PORT2TIRTOS
    /* Relay the message to ApiMac */
    OsalPort_msgSend(appTaskId, (uint8_t*)pMacNpiMtMsg );
#else
    /* Relay the message to ApiMac */
    ICall_send(npiAppEntityID, ApiMac_appEntity,
     //          ICALL_MSG_FORMAT_KEEP, pNpiMsg->pBuf);
               ICALL_MSG_FORMAT_KEEP, pMacNpiMtMsg);

#endif
    MAP_ICall_free(pNpiMsg);

}

/*!
 * @brief   Count an NPI task TX message from ApiMac task
 *
 * @param   pMsg - pointer to incoming message buffer
 */
static void relayTxMsg(void *pMsg)
{
    /* Intentionally not used */
    (void)pMsg;

    /* Count an outgoing MT message */
    numTxMsgs += 1;
#ifdef TI154STACK_BLINK_LEDS_DURING_MESSAGING
    Board_Led_control(board_led_type_LED2, board_led_state_BLINK);
#endif
}
