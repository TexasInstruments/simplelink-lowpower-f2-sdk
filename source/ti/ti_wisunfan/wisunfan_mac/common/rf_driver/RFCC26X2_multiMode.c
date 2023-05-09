/*
* Copyright (c) 2015-2017, Texas Instruments Incorporated
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

#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/SwiP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/rf/RF.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_rfc_rat.h)
#include DeviceFamily_constructPath(inc/hw_rfc_dbell.h)
#include DeviceFamily_constructPath(driverlib/rfc.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/ioc.h)
#include DeviceFamily_constructPath(driverlib/aon_ioc.h)
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/adi.h)
#include DeviceFamily_constructPath(driverlib/aon_rtc.h)
#include DeviceFamily_constructPath(driverlib/chipinfo.h)

#if defined(__IAR_SYSTEMS_ICC__)
#pragma diag_remark=Pa082
#endif

#if defined(RF_SINGLEMODE)
#error "An incompatible symbol (RF_SINGLEMODE) is defined in the project. \
        To use this driver, remove the RF_SINGLEMODE token definition."
#endif

/*-------------- Typedefs, structures & defines ---------------*/

typedef enum {
    Fsm_EventBootDone           = (1<<30),
    Fsm_EventPowerStep          = (1<<29),
    Fsm_EventLastCommandDone    = (1<<1),
    Fsm_EventWakeup             = (1<<2),
    Fsm_EventPowerDown          = (1<<3),
    Fsm_EventInitChangePhy      = (1<<10),
    Fsm_EventChangePhy1         = (1<<11),
    Fsm_EventChangePhy2         = (1<<12),
    Fsm_EventHwInt              = (1<<13),
    Fsm_EventCpeInt             = (1<<14),
    Fsm_EventIeeeInt            = (1<<15),
} Fsm_Event;

typedef enum
{
    RF_PowerConstraintCmdQ      = (1 << 0),  // Power constraint from command queue
    RF_PowerConstraintRat       = (1 << 1),  // Power constraint from RAT
    RF_PowerConstraintAll       = (1 << 7)   // Power constraint to disable power management
} RF_PowerConstraintSrc;

typedef enum
{
    RF_ScheduleCmdSuccess       = 0,         // Schedule command success
    RF_ScheduleCmdAllocError    = 1,         // Schedule command allocation error (such as queue full)
    RF_ScheduleCmdSchError      = 2          // SChedule command scheduler error (timing or priority conflict)
}RF_ScheduleCmdStatus;

/*-------------- Macros ---------------*/

#define ABS(x)     (((x) < 0)   ? -(x) : (x))
#define MAX(x,y)   (((x) > (y)) ?  (x) : (y))
#define UDIFF(x,y) (((y) > (x)) ? ((y) - (x)) : ((~0) + (y) - (x) + (1)))

/*-------------- Defines ---------------*/

/* Max # of RF driver clients */
#define N_MAX_CLIENTS                          2
/* 8 RF_Cmds in pool */
#define N_CMD_POOL                             8
/* Modulus mask used for RF_CmdHandle calculations */
#define N_CMD_MODMASK                          0xFFF

/*-------------- Internal RF constants ---------------*/

#define RF_CMD0                                0x0607
#define RF_BOOT0                               0xE0000011
#define RF_BOOT1                               0x00000080
/* Accessible RF Core interrupts mask MSB 32 bits : RFHW int, LSB 32 bits : RF CPE int */
#define RF_INTERNAL_IFG_MASK                   0xFFFFFFDF60001002
#define RF_CMD_ABORTED_FLAG                    (1 << 0)
#define RF_CMD_STOPPED_FLAG                    (1 << 1)
#define RF_CMD_CANCELLED_FLAG                  (1 << 2)
#define RF_CMD_PREEMPT_FLAG                    (1 << 3)
#define RF_CMD_BG_CMD_FLAG                     (1 << 4)
#define RF_CMD_ALLOC_FLAG                      (1 << 7)
#define RF_CMD_CANCEL_MASK                     (RF_CMD_ABORTED_FLAG | RF_CMD_STOPPED_FLAG | RF_CMD_CANCELLED_FLAG)
#define RF_RUN_CMD_ON_RESTART_FLAG             (1 << 0)
#define RF_RAT_ERR_CB_EVENT_FLAG               (1 << 1)
#define RF_RAT_RPT_CPT_FLAG                    (1 << 2)
#define RF_HW_INT_RAT_CH_MASK                  (RFC_DBELL_RFHWIFG_RATCH7 | RFC_DBELL_RFHWIFG_RATCH6 | RFC_DBELL_RFHWIFG_RATCH5)
#define RF_MAX_RAT_CH_CNT                      3
#define RF_INTERNAL_HW_IFG_MASK_32_BIT         RFC_DBELL_RFHWIFG_MDMSOFT
/* Default value for power up duration (in us) used before first power cycle */
#define RF_DEFAULT_POWER_UP_TIME               2500
/* Default minimum power up duration (in us) */
#define RF_DEFAULT_MIN_POWER_UP_TIME           500
/* Default power-up margin (in us) to account for wake-up sequence outside the RF power state machine */
#define RF_DEFAULT_POWER_UP_MARGIN             314
/* Default power down duration in us */
#define RF_DEFAULT_POWER_DOWN_TIME             1000
#define RF_MAX_CHAIN_CMD_LEN                   32
#define RF_RAT_CH_5_INDEX                      0x0
#define RF_RAT_CH_6_INDEX                      0x1
#define RF_RAT_CH_7_INDEX                      0x2
#define RF_RAT_CH_MAX_INDEX                    15
/* RAT channel (0-4) are used by RF Core. Only 5,6,7 are available for application */
#define RF_RAT_CH_LOWEST                       5
#define RF_RAT_CH_HIGHEST                      7
#define RF_SEND_RAT_STOP_RATIO                 7
#define RF_RTC_CONV_TO_US_SHIFT                12
#define RF_SHIFT_32_BITS                       32
#define RF_SHIFT_8_BITS                        8
#define RF_SHIFT_16_BITS                       16
#define RF_RTC_TICK_INC                        (0x100000000LL/32768)
#define RF_SCALE_RTC_TO_4MHZ                   4000000
#define RF_NUM_RAT_TICKS_IN_1_US               4
/* (3/4)th of a full RAT cycle, in us */
#define RF_DISPATCH_MAX_TIME_US                (UINT32_MAX / RF_NUM_RAT_TICKS_IN_1_US * 3 / 4)
/* (1/4)th of a full RAT cycle, in us */
#define RF_DISPATCH_MAX_TIME_WRAPAROUND_US     (int32_t)(RF_DISPATCH_MAX_TIME_US - UINT32_MAX / RF_NUM_RAT_TICKS_IN_1_US)
#define RF_INTERNAL_CPE_IFG_MASK_32_BIT        0xFFFFFFFF
#define RF_XOSC_HF_SWITCH_CHECK_PERIOD         50
#define RF_DEFAULT_AVAILRATCH_VAL              0xFFFF
#define RF_ABORT_CANCEL_MULTIPLE_CMD           0x2
#define RF_ABORT_CANCEL_SINGLE_CMD             0x1
#define RF_CMDSTA_REG_VAL_MASK                 0xFF
#define RF_RAT_CPT_CONFIG_RATCH_MASK           0x0F00
#define RF_RAT_CPT_CONFIG_REP_MASK             0x1000
#define RF_PHY_SWITCHING_MODE                  1
#define RF_PHY_BOOTUP_MODE                     0
#define RF_SCH_CMD_PRI_PREEMPT_STOP            0x1
#define RF_SCH_CMD_PRI_PREEMPT_ABORT           0x2
#define RF_SCH_CMD_PRI_PREEMPT_ADD             0x3
#define RF_SCH_CMD_TIMING_INSERT               0x4
#define RF_REQ_ACCESS_MAX_DUR_US               1000000
/* Additional analog config time for setup command */
#define RF_ANALOG_CFG_TIME_US                  96
/* Update analog configuration in setup */
#define RF_SETUP_ANALOGCFG_UPDATE              0
/* Don't update analog configuration in setup */
#define RF_SETUP_ANALOGCFG_NOUPDATE            0x2D
#define RF_SCH_CMD_STARTTIME_NOW               0
#define RF_SCH_CMD_ENDTIME_IGNORE              0
#define RF_DEFAULT_PHY_SWITCHING_TIME          500
#define RF_RADIOFREECB_PREEMPT_FLAG            0x1
#define RF_RADIOFREECB_REQACCESS_FLAG          0x2
#define RF_RADIOFREECB_CMDREJECT_FLAG          0x4
#define RF_SCH_CMD_INSERT_QUEUE_TOP            0x1
#define RF_SCH_CMD_INSERT_QUEUE_LATER          0x2
#define RF_SCH_CMD_PREEMPT                     0x4
#define RF_DEFAULT_RAT_RTC_ERR_TOL_IN_US       5
/* Approx for 1e6 / 500. XTAL drift is 500 ppm */
#define RF_DEFAULT_COMB_XTAL_DRIFT_BITS_SHIFT  11
/* Window (in us) to decide if wakeup was from RF power up clock */
#define RF_WAKEUP_DETECTION_WINDOW_IN_US       300
/* IEEE control codes */
#define IEEE_REQUEST_FG_CMD                    0
#define IEEE_SUSPEND_BG_CMD                    1

/*-------------- Structures ---------------*/

/* RF command queue member*/
typedef struct RF_Cmd_s RF_Cmd;

/* RF command member*/
struct RF_Cmd_s {
    RF_Cmd* volatile     pNext;       /* Pointer to next #RF_Cmd in queue */
    RF_Callback volatile pCb;         /* Pointer to callback function */
    RF_Op*               pOp;         /* Pointer to (chain of) RF operations(s) */
    RF_Object*           pClient;     /* Pointer to client */
    RF_EventMask         bmEvent;     /* Enable mask for interrupts from the command */
    RF_EventMask         pastifg;     /* Accumulated value of events happened within a command chain */
    uint32_t             rfifg;       /* Return value for callback 0:31 - RF_CPE0_INT, 32:63 - RF_HW_INT */
    uint32_t             startTime;   /* command start time (in RAT ticks) */
    uint32_t             endTime;     /* command end time (in RAT ticks) */
    RF_CmdHandle         ch;          /* Command handle */
    RF_Priority          ePri;        /* Priority of RF command */
    uint8_t volatile     flags;       /* [0: Aborted, 1: Stopped, 2: cancelled] */
};

/* RF Direct Immediate command*/
typedef struct RF_CmdDirImm_s RF_CmdDirImm;

/* RF direct immediate command member*/
struct RF_CmdDirImm_s {
    RF_Handle            pClient[RF_MAX_RAT_CH_CNT];  /* Pointer to current client */
    RF_Callback          pRatCb[RF_MAX_RAT_CH_CNT];   /* Array of callback pointers for each RAT chan */
    uint32_t volatile    hwIntEvent;                  /* RF_HW events received */
    uint16_t             availRatCh;                  /* bit-map for available RAT channels */
    uint8_t              rptCptRatChNum;              /* Channel allocated for repeat mode */
    uint8_t              flags;                       /* [0:runOnRestart, 1:errEventCb, 2: rptCptFlag] */
};

/*-------------- RTOS objects ---------------*/

/* RF core software interrupts */
static SwiP_Struct swiFsm;
static void swiFxnFsm(uintptr_t a, uintptr_t b);

/* RF core hardware interrupts */
static HwiP_Struct hwiCpe0;
static void hwiFxnCpe0PowerFsm(uintptr_t a);
static void hwiFxnCpe0Active(uintptr_t a);
static void hwiFxnCpe0ChangePhy(uintptr_t a);

/* RF core HW software interrupts */
static SwiP_Struct swiHw;
static void swiFxnHw(uintptr_t a, uintptr_t b);

/* RF core HW hardware interrupts */
static HwiP_Struct hwiHw;
static void hwiFxnHw(uintptr_t a);

/* Clock used for triggering power-up sequences */
static ClockP_Struct clkPowerUp;
static void clkPowerUpFxn(uintptr_t a);

/* Common inactivity timeout clock callback */
static void clkInactivityFxn(RF_Object* pObj);

/* Common request access timeout clock callback */
static void clkReqAccessFxn(RF_Object* pObj);

/*-------------- Static structures ---------------*/

/* RAT sync start/stop radio operation command */
static union {
    rfc_CMD_SYNC_START_RAT_t start;
    rfc_CMD_SYNC_STOP_RAT_t  stop;
} opRatSync;

/* Default RF parameters structure */
static const RF_Params RF_defaultParams = {
    .nInactivityTimeout     = SemaphoreP_WAIT_FOREVER,
    .nPowerUpDuration       = RF_DEFAULT_POWER_UP_TIME,
    .pPowerCb               = NULL,
    .pErrCb                 = NULL,
    .bPowerUpXOSC           = true,
    .nPowerUpDurationMargin = RF_DEFAULT_POWER_UP_MARGIN,
    .pClientEventCb         = NULL,
    .nClientEventMask       = 0,
};

/* Common RF command queue shared by all clients */
static struct {
    RF_Cmd* volatile      pPend;     /* Queue of RF_Cmds pending execution */
    RF_Cmd* volatile      pCurrCmd;  /* RF_Cmd currently executing */
    RF_Cmd* volatile      pCurrCmdCb;/* Holds current command while callback is processed */
    RF_Cmd* volatile      pCurrCmdBg;/* Holds background command while foreground command is processed */
    RF_Cmd* volatile      pDone;     /* Queue of RF_Cmds that are done but not retired */
    RF_CmdHandle volatile nSeqPost;  /* Sequence # for previously posted command */
    RF_CmdHandle volatile nSeqDone;  /* Sequence # for last done command */
} cmdQ;

/* RF scheduler variables data structure */
static struct {
    RF_Handle       clientHnd[N_MAX_CLIENTS];                   /* client handles for each registered client */
    uint32_t        phySwitchingTimeInUs[N_MAX_CLIENTS];        /* phy switching time 0: client 1 -> 2, 1 : client 2 -> 1 */
    RF_AccessParams accReq[N_MAX_CLIENTS];                      /* input parameters from any RF_requestAccess API calls */
    RF_Handle       clientHndRadioFreeCb;                       /* client handle for the radio callback */
    /* structure to store items when scheduling results in preemption */
    struct {
        RF_Handle       client;                                 /* RF_Handle for the preempted client */
        RF_CmdHandle    cancelStartCmdHandle;                   /* RF_CmdHandle of the preemption start */
        bool            cancelType;                             /* 1: flush gracefully, 0: flush abort */
    } preemptCmd;
    uint8_t         issueRadioFreeCbFlags;                      /* indicate if driver needs to issue RF_EventRadioFree callback {0:pre-emption, 1:requestAccess running, 2: reject command} */
    uint8_t         cmdInsertFlags;                             /* indicate if the command was inserted based on timing information */
} RF_Sch;

/*-------------- Global variables ---------------*/

/* RF_Cmd container pool. Containers with extra information about RF commands */
static RF_Cmd aCmdPool[N_CMD_POOL];

/* Static object used to subscribe from early notification in the power driver */
static Power_NotifyObj RF_wakeupNotifyObj;

/* Power constraints set by the RF driver */
static volatile uint8_t RF_powerConstraint;

/* Pointer to current radio client (indicates also whether the radio is powered) */
static RF_Object* pCurrClient;

/* Static container of a direct/immediate commands */
static RF_CmdDirImm cmdDirImm;

/* Variables used for powerUpDuration, phySwitchingTime and RAT sync time calculation */
static struct {
    uint64_t nRtc1;             /* RTC timer value power-up and active time calculation */
    uint32_t nRtc2;             /* RTC timer value for switching time calculation */
    uint32_t activeTimeInUs;    /* total radio active time value since last power up (in us) */
    uint8_t  errTolValInUs;     /* max allowed error between RAT/RTC drift to enable resync at power-down (in us) */
} powerDurCalc;

/* Counter of radio clients */
static uint8_t  nClients;

/*-------------- Externs ---------------*/

/* Hardware attribute structure populated in board.c file to set HWI and SWI priorities */
extern const RFCC26XX_HWAttrs RFCC26XX_hwAttrs;

/*-------------- Booleans ---------------*/

/* Indicates if power down operation needs to send CMD_RAT_SYNC_STOP. Only used with RF_yield() */
static bool bRatSyncWasRequested;

/* Indicates the first instance the radio power up/down is done after rf_init() */
static bool bRadioHasBeenBootedBefore;

/* variable to indicate power-up requested when powering down */
static bool volatile bRadioRestartIsRequested;

/* Keeps track of whether RF Core is active */
static bool bRadioIsActive;

/* Indicate when RF Core is in power-up FSM */
static bool bRadioIsBeingPoweredUp;

/* Indicate when RF Core is in power-down FSM */
static bool bRadioIsBeingPoweredDown;

/* Indicate if RF_yield requested to power-down the RF Core */
static bool bRadioYieldingWasRequested;

/* Indicate request to cancel a power-down request from RF_yield */
static bool bRadioYieldWasCancelled;

/* Indicate that switch change phy is active */
static bool bRadioPhyIsBeingSwitched;

/* variable to indicate if the XOSC_HF_SWITCHING is done by the RF Driver */
static bool bRadioDelegateXOSCHFSwitching;

/* variable to indicate with the FLASH is disable during the power up */
static bool bDisableFlashInIdleConstraint;

/*-------------- State machine functions ---------------*/

/* FSM typedefs */
typedef void (*Fsm_StateFxn)(RF_Object*, Fsm_Event const);

/* Current state of radio state-machine */
static Fsm_StateFxn pFsmState;

/* FSM helper functions */
static void FsmNextState(Fsm_StateFxn pTarget);

/* FSM state functions */
static void fsmPowerUpState(RF_Object *pObj, Fsm_Event e);
static void fsmSetupState(RF_Object *pObj, Fsm_Event e);
static void fsmActiveState(RF_Object *pObj, Fsm_Event e);
static void fsmXOSCState(RF_Object *pObj, Fsm_Event e);
static void fsmChangePhyState(RF_Object *pObj, Fsm_Event e);

/*-------------- Helper functions ---------------*/

/* RAT channel free fxn */
static void ratChanFree(int8_t ratChanNum, bool checkPowerDown);

/*-------------- Command queue internal functions ---------------*/

/*
 *  Compares the client of a command.
 *
 *  Input:  h     - Client to check against.
 *          pCmd  - Command to check.
 *  Return: true  - If the client owns the command.
 *          false - Otherwise.
 */
static bool isClientOwner(RF_Handle h, RF_Cmd* pCmd)
{
    if(pCmd && (pCmd->pClient == h))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
 *  Peek at first entry in simple queue.
 *
 *  Input:  ppHead - Pointer to the head of the command queue
 *  Return: RF command
 */
static RF_Cmd* Q_peek(RF_Cmd* volatile* ppHead)
{
    return (RF_Cmd*)*ppHead;
}

/*
 *  Pop first entry in simple queue.
 *
 *  Input:  ppHead - Pointer to the head of the command queue
 *  Return: RF command
 */
static RF_Cmd* Q_pop(RF_Cmd* volatile* ppHead)
{
    RF_Cmd* pCmd;
    uint32_t key = HwiP_disable();
    if ((pCmd=(RF_Cmd*)*ppHead))
    {
        *ppHead = (RF_Cmd*)pCmd->pNext;
    }
    HwiP_restore(key);
    return pCmd;
}

/*
 *  Push entry onto simple queue.
 *
 *  Input:  h        - RF handle
 *          ppHead   - Pointer to the head of the command queue
 *          pCmd     - Pointer to command
 *          bHeadPos - Boolean to indicate if this was the first command in the queue
 *  Return: true     - If command was pushed to the queue.
 *          false    - If command was rejected.
 */
static bool Q_push(RF_Cmd* volatile* ppHead, RF_Cmd* pCmd, bool* bHeadPos)
{
    uint32_t key;

    /* Start at the head of queue */
    RF_Cmd* pEnd = (RF_Cmd*)*ppHead;

    /* Set the next to be null*/
    pCmd->pNext = NULL;

    /* If the queue is empty */
    if (pEnd == NULL)
    {
        /* Enter critical section */
        key = HwiP_disable();

        /* First entry into queue */
        *ppHead = pCmd;

        /* Indicate that it is the first entry */
        if(bHeadPos)
        {
          *bHeadPos = true;
        }
    }
    else
    {
        /* Indicate that this is not the first entry*/
        if(bHeadPos)
        {
          *bHeadPos = false;
        }

        /* Enter critical section */
        key = HwiP_disable();

        /* Find the tail */
        while (pEnd->pNext)
        {
            pEnd = (RF_Cmd*)pEnd->pNext;
        }

        /* Sanity check - to avoid pEnd->pNext pointing to self */
        if (pEnd == pCmd)
        {
            HwiP_restore(key);
            return false;
        }

        /* Append the command */
        pEnd->pNext = pCmd;
    }

    /* Exit critical section */
    HwiP_restore(key);

    /* Return with success */
    return true;
}

/*
 *  Search last entry in simple queue  for particular client.
 *
 *  Input:  h      - RF handle
 *          ppHead - Pointer to the head of the command queue
 *  Return: RF command
 */
static RF_Cmd* Q_end(RF_Handle h, RF_Cmd* volatile* ppHead)
{
    /* Local variables */
    RF_Cmd* pEnd        = (RF_Cmd*)*ppHead;
    RF_Cmd* pClientEnd  = NULL;

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* If there is at least one entry in the queue */
    if (pEnd)
    {
        do
        {
            /* If it belongs to the client */
            if (isClientOwner(h, pEnd))
            {
                /* Set the pointer to store its location */
                pClientEnd = pEnd;
            }

            /* Step the queue */
            pEnd = (RF_Cmd*)pEnd->pNext;
        } while(pEnd);
    }

    /* Exit critical section */
    HwiP_restore(key);

    /* Return with the last entry belongs to the client */
    return pClientEnd;
}

/*
 *  Allocate cmd buffer from cmd pool.
 *
 *  Input:  none
 *  Return: RF command
 */
static RF_Cmd* allocCmd()
{
    uint32_t i;
    for (i = 0; i < N_CMD_POOL; i++)
    {
        /* Find the first available entry in the command pool */
        if (!(aCmdPool[i].flags & RF_CMD_ALLOC_FLAG))
        {
            return &aCmdPool[i];
        }
    }
    return NULL;
}

/*
 *  Search cmd from cmd pool.
 *
 *  Input:  h    - Handle to the client which the command should belong to.
 *          ch   - Handle to the command to search for.
 *          mask - Optional mask of flags to compare to.
 *  Return: RF command
 */
static RF_Cmd* getCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mask)
{
    uint32_t i;
    for (i = 0; i < N_CMD_POOL; i++)
    {
        /* Find the allocated command pool entry corresponding to ch */
        if (aCmdPool[i].ch == ch)
        {
            if(isClientOwner(h, &aCmdPool[i]))
            {
                /* If a mask is provided, check the flags too */
                if(mask)
                {
                  /* If the flags pass, return with the command */
                  if(aCmdPool[i].flags & mask)
                  {
                    return &aCmdPool[i];
                  }
                }
                else
                {
                  /* No filtering on the flags, return the command
                     upon match of the command handle*/
                  return &aCmdPool[i];
                }
            }
        }
    }
    return NULL;
}


/*
 *  Wake up notification callback from the power driver. If the callback is from RF wakeup
 *  set constraint to let RF Driver control the XOSC switching else do nothing in the
 *  callback.
 *
 *  Input:  eventType - The type of event when the notification is invoked
 *          eventArg  - na
 *          clientArg - na
 *  Return: Power_NOTIFYDONE
 */
static uint8_t RF_wakeupNotifyFxn(uint8_t eventType, uint32_t *eventArg, uint32_t *clientArg)
{
    /* Check if the callback is for wakeup from standby and if power up clock is running */
    if ((eventType == PowerCC26XX_AWAKE_STANDBY) && (ClockP_isActive(&clkPowerUp)))
    {
        /* Calculate time (in us) until next trigger (assume next trigger is max ~70 min away) */
        uint32_t timeInUsUntilNextTrig = ClockP_tickPeriod * ClockP_getTimeout(&clkPowerUp);

        /* Check if the next trig time is close enough to the actual power up */
        if (timeInUsUntilNextTrig < RF_WAKEUP_DETECTION_WINDOW_IN_US)
        {
            /* Stop power up clock */
            ClockP_stop(&clkPowerUp);

            /* Setup RF Driver to do the XOSC_HF switching */
            Power_setConstraint(PowerCC26XX_SWITCH_XOSC_HF_MANUALLY);

            /* Set variable to indicate RF Driver will do the XOSC_HF switching */
            bRadioDelegateXOSCHFSwitching = true;

            /* Start the RF Core power up */
            SwiP_or(&swiFsm, Fsm_EventWakeup);
        }
    }

    return Power_NOTIFYDONE;
}

/*-------------- Scheduler internal functions --------------------------------*/

/*
 *  Issue RF_EventRadioFree callback to the client. The callback is issued -
 *  1. After pre-emption is complete
 *  2. Dedicated request access period expires or released
 *  3. command reject because of other high priority command running
 *
 *  Input:  src - Flag indicating the source of callback request.
 *  Return: none
 */
static void RF_issueRadioFreeCb(uint8_t src)
{
    /* Enter critical section*/
    uint32_t key = HwiP_disable();

    /* Clear the reason why the callback is being invoked */
    RF_Sch.issueRadioFreeCbFlags &= ~src;

    /* Local variable */
    bool isReqAccessActive = false;

    /* If any of the clients has active request access, indicate it */
    if (RF_Sch.clientHnd[0])
    {
        isReqAccessActive |= ClockP_isActive(&RF_Sch.clientHnd[0]->state.clkReqAccess);
    }
    if (RF_Sch.clientHnd[1])
    {
        isReqAccessActive |= ClockP_isActive(&RF_Sch.clientHnd[1]->state.clkReqAccess);
    }

    /* If we cleared all the potential sources and there is no request access*/
    if ((RF_Sch.issueRadioFreeCbFlags == 0) && !isReqAccessActive)
    {
        /* If a valid client handle is provided through the global pointer */
        if (RF_Sch.clientHndRadioFreeCb)
        {
            /* If the client subscribed for any events */
            if (RF_Sch.clientHndRadioFreeCb->clientConfig.nClientEventMask & RF_ClientEventRadioFree)
            {
                /* Get a pointer to the client event callback */
                RF_ClientCallback pClientEventCb = (RF_ClientCallback)RF_Sch.clientHndRadioFreeCb->clientConfig.pClientEventCb;

                /* Exit critical section */
                HwiP_restore(key);

                /* Invoek the client event callback */
                pClientEventCb(RF_Sch.clientHndRadioFreeCb, RF_ClientEventRadioFree, NULL);
            }
            else
            {
                /* If the client did not subscribe for client event callback, take the power callback */
                RF_Callback pPowerCb = (RF_Callback)RF_Sch.clientHndRadioFreeCb->clientConfig.pPowerCb;

                /* Exit critical section */
                HwiP_restore(key);

                /* Invoke the power callback */
                pPowerCb(RF_Sch.clientHndRadioFreeCb, 0, RF_EventRadioFree);
            }

            /* Clear the client pointer in any case */
            RF_Sch.clientHndRadioFreeCb = NULL;
        }
    }
    else
    {
        /* Exit critical section */
        HwiP_restore(key);
    }
}

/*
 *  Check if new request can inserted between the previous and next command in the
 *  current queue.
 *
 *  Input:  newCmd  - RF_Cmd pointer for the new command request
 *          prevCmd - RF_Cmd pointer for the previous cmd in the queue
 *          nextCmd - RF_Cmd pointer for the next cmd in the queue
 *  Return: true    - If command can be inserted in the queue else
 *          false   - Otherwise.
 */
static bool RF_schChkCmdInsert(RF_Cmd* newCmd, RF_Cmd* prevCmd, RF_Cmd* nextCmd)
{
    bool        insertNewCmdAfterPrev;
    bool        insertNewCmdBeforeNext;
    int32_t     deltaInUs;
    int32_t     clientSwitchingTime;

    /* Initalize global variables */
    insertNewCmdAfterPrev  = false;
    insertNewCmdBeforeNext = false;
    if (!nextCmd)
    {
        insertNewCmdBeforeNext = true;
    }
    clientSwitchingTime = 0;

    /*
        Check if newCmd can be inserted between prevCmd and nextCmd.
        if prevCmd is not NULL check if startTime of newCmd is more endTime of the
        prevCmd.
        if nextCmd is not NULL check if the endTime of the newCmd is less than the
        startTime of the nextCmd.
    */
    if ((prevCmd) && (prevCmd->endTime != RF_SCH_CMD_ENDTIME_IGNORE) && (newCmd->startTime != RF_SCH_CMD_STARTTIME_NOW))
    {
        /* if prevCmd and newCmd are from different client then switching time needs to be included in deltaInUs calc */
        if (prevCmd->pClient != newCmd->pClient)
        {
            if (prevCmd->pClient == RF_Sch.clientHnd[0])
            {
                clientSwitchingTime = RF_Sch.phySwitchingTimeInUs[0];
            }
            else
            {
                clientSwitchingTime = RF_Sch.phySwitchingTimeInUs[1];
            }
        }

        deltaInUs = ((int32_t)(newCmd->startTime >> 2) - ((int32_t)(prevCmd->endTime >> 2) + clientSwitchingTime));
        /*
           check if startTime or endTime has overflowed
           Limitation: Assumes that difference between the startTime and endTime is less than 8 min
        */
        if (deltaInUs >= (int32_t)RF_DISPATCH_MAX_TIME_US)
        {
            /* endTime has wrapped around */
            return 0;
        }
        else if ((deltaInUs < ((int32_t)RF_DISPATCH_MAX_TIME_WRAPAROUND_US)) || (deltaInUs > 0))
        {
            /* startTime has wrapped around or no wrap around and we can insert the command */
            insertNewCmdAfterPrev = true;
        }
        else
        {
            return 0;
        }
    }

    if ((nextCmd) && (nextCmd->startTime != RF_SCH_CMD_STARTTIME_NOW) && (newCmd->endTime != RF_SCH_CMD_ENDTIME_IGNORE))
    {
        clientSwitchingTime = 0;
        /* if nextCmd and newCmd are from different client then switching time needs to be included in deltaInUs calc */
        if (newCmd->pClient != nextCmd->pClient)
        {
            if (nextCmd->pClient == RF_Sch.clientHnd[0])
            {
                clientSwitchingTime = RF_Sch.phySwitchingTimeInUs[0];
            }
            else
            {
                clientSwitchingTime = RF_Sch.phySwitchingTimeInUs[1];
            }
        }

        deltaInUs = (((int32_t)(nextCmd->startTime >> 2) - clientSwitchingTime) - (int32_t)(newCmd->endTime >> 2));
        /*
            check if startTime or endTime has overflowed.
            Limitation: Assumes that difference between the startTime and endTime is less than 8 min
        */
        if (deltaInUs >= (int32_t)RF_DISPATCH_MAX_TIME_US)
        {
            /* endTime has wrapped around */
            return 0;
        }
        else if ((deltaInUs < ((int32_t)RF_DISPATCH_MAX_TIME_WRAPAROUND_US)) || (deltaInUs > 0))
        {
            /* startTime has wrapped around or no wrap around and we can insert the command */
            insertNewCmdBeforeNext = true;
        }
        else
        {
            return 0;
        }
    }

    /* Return with true if the command can be inserted into the queue (before or after)*/
    return (insertNewCmdBeforeNext & insertNewCmdAfterPrev);
}

/*
 *  Check newReqPriority can preempt the currPriority.
 *
 *  Input:  currPriority   - RF_Prioirty of the existing command
 *          newReqPriority - RF_Priority of the new request
 *  Return: int8_t         - Indicate if new request priority can preempt the existing
 *                           command and the preemption type.
 */
static int8_t RF_schPriorityCompare(RF_Priority currPriority, RF_Priority newReqPriority)
{
    /* Reject lower or equal priorities */
    if (newReqPriority <= currPriority)
    {
        return 0;
    }
    else if (newReqPriority == RF_PriorityHigh)
    {
        /* If the priority is high, stop the running command */
        return RF_SCH_CMD_PRI_PREEMPT_STOP;
    }
    else
    {
        /* If the priority is highest, abort the running command */
        return RF_SCH_CMD_PRI_PREEMPT_ABORT;
    }
}

/*
 *  Scan through the command queue to check new command with RF_Priority = priority.
 *  can be added by preempted existing commands in the queue.
 *
 *  Input:  h2       - RF_Handle of client not requesting the new command.
 *          priority - RF_Priority of the new request.
 *  Return: 1        - New request can be granted, required cmds from h2 are cancelled.
 *          2        - No command from client h2, new command can be added.
 *          3        - New command priority cannot preempt any command in current queue.
 */
static uint8_t RF_schCmdPriority(RF_Handle h2, RF_Priority priority)
{
    /* Local variables */
    int8_t cmdQReq          = 0;    /* Priority weight */
    int8_t cmdQReq_curr     = 0;    /* Priority weight*/
    RF_Cmd* pPreemptCmd     = NULL; /* Command from client 2 to be preempted */
    RF_Cmd* pCmdSearchStart = NULL; /* Temporary pointer to command to be compared with */
    bool noCmdFromClient2   = true; /* No command from client 2 i nthe queue */

    /*
         currCmd  pendCmd
      1.   NULL   NULL        Add to end of queue
      2.   NULL   all h1      Add to end of queue
      3.   NULL   h1 and h2   Check cmd parameters
      4.   h1     NULL        Add to end of queue
      5.   h1     all h1      Add to end of queue
      6.   h1     h1 and h2   Check cmd parameters
      7.   h2     NULL        Check cmd parameters
      8.   h2     all h1      Add to end of queue
      9.   h2     h1 and h2   Check cmd parameters

       search through all the radio commands in the aCmdPool (current command and pending commands)
       to decide if the new command requires pre-empting any commands or simply add to end of the
       queue.
       Commands are preempted only if priority of the new request is greater than ALL the commands
       in the current command from client h2.
    */

    /* If the current running command belongs to client 2 */
    if (isClientOwner(h2, cmdQ.pCurrCmd))
    {
        /* There is a command from client 2 then */
        noCmdFromClient2 = false;

        /* Compare the priority levels */
        cmdQReq = RF_schPriorityCompare(cmdQ.pCurrCmd->ePri, priority);

        /* The current command can be preempted */
        if (cmdQReq)
        {
            pPreemptCmd = cmdQ.pCurrCmd;
        }
    }

    /* If there is at least one command in the pend queue */
    if (cmdQ.pPend)
    {
        /* Start to search from the beginning of the queue */
        pCmdSearchStart = cmdQ.pPend;

        /* Check the pending queue */
        while(pCmdSearchStart)
        {
            /* In case the command belongs to client 2*/
            if (isClientOwner(h2, pCmdSearchStart))
            {
                /* Then there is a command from client 2 */
                noCmdFromClient2 = false;

                /* Compare the priority of the command */
                cmdQReq_curr = RF_schPriorityCompare(pCmdSearchStart->ePri, priority);

                /* This command can be preempted */
                if ((cmdQReq_curr != 0) && (cmdQReq == 0))
                {
                    pPreemptCmd = pCmdSearchStart;
                }

                /* Store the command weight */
                if (((cmdQReq_curr != 0) && (cmdQReq == 0)) ||
                    (cmdQReq_curr == 0))
                {
                    cmdQReq = cmdQReq_curr;
                }
            }

            /* Walk the queue */
            pCmdSearchStart = pCmdSearchStart->pNext;
        }
    }

    /* There is no command from client 2 in the queue, no need for preemption */
    if (noCmdFromClient2)
    {
        return 2;
    }

    /* If there it is possible to preempt a command */
    if ((cmdQReq) && (pPreemptCmd))
    {
        /* Set command preempt flag, used to issue RF_EventCmdPreempted in the cmd cancel cb */
        RF_Cmd* pTmpCmd = pPreemptCmd;
        if (pTmpCmd == cmdQ.pCurrCmd)
        {
            if(isClientOwner(h2, cmdQ.pCurrCmd))
            {
              /* Mark the current command as being preempted */
              cmdQ.pCurrCmd->flags |= RF_CMD_PREEMPT_FLAG;
              pTmpCmd = cmdQ.pPend;
            }
        }

        /* Walk the pend queue and mark all the commands belong to client 2 as being preempted */
        while(pTmpCmd)
        {
            /* If the command belongs to client 2 */
            if (isClientOwner(h2, pTmpCmd))
            {
                pTmpCmd->flags |= RF_CMD_PREEMPT_FLAG;
            }

            /* Step the queue */
            pTmpCmd = pTmpCmd->pNext;
        }

        /* Set flag to issue the RF_EventRadioFree once the new command(s) from h2 is done */
        RF_Sch.clientHndRadioFreeCb = h2;
        RF_Sch.issueRadioFreeCbFlags |= RF_RADIOFREECB_PREEMPT_FLAG;

        /* Save the variables needed to call RF_flushCmd() because of preemption */
        RF_Sch.preemptCmd.client = h2;
        RF_Sch.preemptCmd.cancelStartCmdHandle = pPreemptCmd->ch;
        RF_Sch.preemptCmd.cancelType = cmdQReq & 0x1;

        return 1;
    }
    else
    {
        /* Preemption not possible */
        return 0;
    }
}

/*
 *  Check if new command can be scheduled based on to the current command queue
 *  and the request access.
 *
 *  Input:  h1         - RF_Handle of requesting command.
 *          h2         - RF_Handle of the client not requesting the new command.
 *          pOp        - RF_Op pointer for the requesting command.
 *          pSchParams - Pointer to user input RF_ScheduleCmdParams structure of the requesting command.
 *
 *  Output: pSchStatus - Pointer to schedule error.
 *
 *  Return: NULL       - If new command request cannot be met.
 *          RF_Cmd*    - Valid aCmdPool pointer to the slot allocated for the new cmd request.
 */
static RF_Cmd* RF_schCmdRunInsertPreempt(RF_Handle h1, RF_Handle h2, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams,
                                         RF_ScheduleCmdStatus *pSchStatus)
{
    RF_Cmd* pCmd;
    RF_Cmd* pTmp;
    uint8_t bPrioritySch;

    /* initalize local variables */
    pCmd = NULL;
    bPrioritySch = 0;

    /* Initialize global variables */
    RF_Sch.cmdInsertFlags = 0;

    /* Initialize schedule command status */
    *pSchStatus = RF_ScheduleCmdSuccess;

    /*
       Check if command queue has free entries and allocate RF_Op* container
       if command queue is full reject the command
    */
    if ((pCmd = allocCmd()))
    {
        pCmd->ch = (cmdQ.nSeqPost + 1) & N_CMD_MODMASK;

        /* cancel the radio free callback if new command is from the same client */
        if ((RF_Sch.clientHndRadioFreeCb == h1) &&
            (RF_Sch.issueRadioFreeCbFlags & RF_RADIOFREECB_PREEMPT_FLAG))
        {
            RF_Sch.issueRadioFreeCbFlags &= ~RF_RADIOFREECB_PREEMPT_FLAG;
        }

        /* Update the default endTime based on the scheduling parameters */
        if (pSchParams->endTime)
        {
            pCmd->endTime = pSchParams->endTime;
        }
        else
        {
            pCmd->endTime = RF_SCH_CMD_ENDTIME_IGNORE;
        }

        /* Update the default startTime based on the command parameters */
        if (pOp->startTrigger.triggerType == TRIG_ABSTIME)
        {
            pCmd->startTime = pOp->startTime;
        }
        else
        {
            pCmd->startTime = RF_SCH_CMD_STARTTIME_NOW;
        }

        /* No need to run scheduler if only one client is setup */
        if (!h2)
        {
            return pCmd;
        }
    }
    else
    {
        /* Set the status value to alloc_error if we could not allocate space */
        *pSchStatus = RF_ScheduleCmdAllocError;

        /* Return with NULL in case of error */
        return NULL;
    }

    /*
       If there is RF_requestAccess (currently always RF_PriorityHighest)
       from a different client running, reject any new commands from client 1
    */
    if (h2 && (ClockP_isActive(&h2->state.clkReqAccess)))
    {
        /* Set the status value to schedule_error if we could not allocate space */
        *pSchStatus = RF_ScheduleCmdSchError;

        /* Return with NULL in case of error */
        return NULL;
    }

    /*
       Scheduler resolution order -
       1a. Check if new command can be inserted based on the timing information
           at the top of the pending queue .
       1b. Check if new command can be inserted based on the timing information
           in the middle/end of the pending queue.
       2.  Check if new command can preempt existing commands based on priority.
       3.  Check if new command can be appended to the end of pending queue.
    */

    /* Step 1a */
    pTmp = Q_end(h1, &cmdQ.pPend);

    if (!pTmp && cmdQ.pPend)
    {
        RF_Cmd tmpCmd;
        tmpCmd.endTime = RF_getCurrentTime();

        if ((cmdQ.pCurrCmd && RF_schChkCmdInsert(pCmd, cmdQ.pCurrCmd, cmdQ.pPend)) ||
            (!cmdQ.pCurrCmd && RF_schChkCmdInsert(pCmd, &tmpCmd, cmdQ.pPend)))
        {
            /* Insert command at the beginning of the queue */
            pCmd->pNext = cmdQ.pPend;
            cmdQ.pPend  = pCmd;

            /* Indicate that the command was put on the top of the queue */
            RF_Sch.cmdInsertFlags = RF_SCH_CMD_INSERT_QUEUE_TOP;
        }
    }

    /* Step 1b */
    if (!RF_Sch.cmdInsertFlags)
    {
        if (!pTmp)
        {
            pTmp = cmdQ.pPend;
        }
        while (pTmp)
        {
            if (RF_schChkCmdInsert(pCmd, pTmp, pTmp->pNext))
            {
                /* Insert command between pTmp and pTmp->pNext */
                pCmd->pNext = pTmp->pNext;
                pTmp->pNext = pCmd;
                RF_Sch.cmdInsertFlags = RF_SCH_CMD_INSERT_QUEUE_LATER;
                break;
            }
            else
            {
                pTmp = pTmp->pNext;
            }
        }
    }

    /* Step 2 */
    if (!RF_Sch.cmdInsertFlags)
    {
        bPrioritySch = RF_schCmdPriority(h2, pSchParams->priority);
        if (bPrioritySch == 1)
        {
            RF_Sch.cmdInsertFlags = RF_SCH_CMD_PREEMPT;
        }
    }

    /* Step 3 */
    if (!RF_Sch.cmdInsertFlags && (bPrioritySch != 1))
    {
        /* Traverse to the end of the command queue */
        pTmp = cmdQ.pPend;
        if (pTmp)
        {
            while(pTmp->pNext)
            {
                pTmp = pTmp->pNext;
            }
        }

        /* If new command has TRIG_ABSTIME triggerType and pastTrig = 0 ->
           check if the timing can be meet to append to command to the end of the queue,
           reject command is cannot be appended */
        if ((pOp->startTrigger.triggerType == TRIG_ABSTIME) && !pOp->startTrigger.pastTrig)
        {
            if ((pTmp && !RF_schChkCmdInsert(pCmd, pTmp, NULL)) ||
                (!pTmp && cmdQ.pCurrCmd && !RF_schChkCmdInsert(pCmd, cmdQ.pCurrCmd, NULL)))
            {
                *pSchStatus = RF_ScheduleCmdSchError;
                return NULL;
            }
        }
    }

    /* Return with a pointer to the new command */
    return pCmd;
}

/*
 *  Check if new access request can be granted.
 *
 *  Input:  h1 - RF_Handle of requesting command.
 *          h2 - RF_Handle of the client not requesting the new command.
 *  Return: 0  - Request cannot be granted.
 *          1  - New request can be granted.
 */
static uint8_t RF_schReqAccessCheck(RF_Handle h1, RF_Handle h2)
{
    /* reject access request if any request is ongoing */
    if ((h1 && ClockP_isActive(&h1->state.clkReqAccess)) ||
        (h2 && ClockP_isActive(&h2->state.clkReqAccess)))
    {
        return 0;
    }

    if (h2)
    {
        return RF_schCmdPriority(h2, RF_PriorityHighest);
    }
    else
    {
        return 2;
    }
}

/*
 *  Execute RF power down sequence.
 *
 *  Input:  none
 *  Return: none
 */
static void RF_executePowerDown()
{
    /* Powerdown RF core by switching the analog parts off first and the
       digital parts after */
    uint32_t rtcValTmp1;
    uint32_t rtcValTmp2;

    /* Set VCOLDO reference */
    RFCAdi3VcoLdoVoltageMode(false);

    /* Remap HWI to the startup function (preparing for next wake up) */
    HwiP_setFunc(&hwiCpe0, hwiFxnCpe0PowerFsm, (uintptr_t)NULL);

    /* Take wake up timestamp and the current timestamp */
    rtcValTmp1 = (uint32_t) powerDurCalc.nRtc1;
    rtcValTmp2 = (uint32_t) AONRTCCurrent64BitValueGet();

    /* Find the radio core active time since the last power up */
    powerDurCalc.activeTimeInUs   = UDIFF(rtcValTmp1, rtcValTmp2);
    powerDurCalc.activeTimeInUs >>= RF_RTC_CONV_TO_US_SHIFT;

    /* Decide whether to send the CMD_SYNC_STOP_RAT command. If this is first power down (bStart is false), active time
       is longer than the time that can cause max allowed error between RAT and RTC clocks or if RF_yield was called
       (bRatSync is true) */
    if ((bRadioHasBeenBootedBefore == false) ||
        (bRatSyncWasRequested == true)       ||
        (powerDurCalc.activeTimeInUs > (powerDurCalc.errTolValInUs << RF_DEFAULT_COMB_XTAL_DRIFT_BITS_SHIFT)))
    {
        /* Mark that we handled it */
        bRadioHasBeenBootedBefore = true;

        /* Clear any request for RAT sync as it is being handled */
        bRatSyncWasRequested = false;

        /* Stop and synchronize the RAT if it is running */
        if(HWREG(RFC_PWR_BASE + RFC_PWR_O_PWMCLKEN) & RFC_PWR_PWMCLKEN_RAT_M)
        {
            /* Setup RAT_SYNC command to follow FsPowerdown command */
            opRatSync.stop.commandNo                 = CMD_SYNC_STOP_RAT;
            opRatSync.stop.condition.rule            = COND_NEVER;
            opRatSync.stop.startTrigger.triggerType  = TRIG_NOW;
            opRatSync.stop.pNextOp                   = NULL;

            /* Enter critical section */
            uint32_t key = HwiP_disable();

            /* Disable CPE last command done interrupt */
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) &= ~RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M;

            /* Clear CPE last command done interrupt flag */
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG) = ~RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M;

            /* Send RAT Stop command */
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = (uint32_t)&opRatSync.stop;

            /* Wait till RAT Stop command to complete */
            while (!(HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG) & RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M));

            /* Clear CPE last command done interrupt flag and enable last command done interrupt */
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG) = ~RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M;
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) |= RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M;

            /* Leave critical section */
            HwiP_restore(key);
        }
    }

    /* Reset nRtc1 */
    powerDurCalc.nRtc1 = 0;

    /* Reset the active time */
    powerDurCalc.activeTimeInUs = 0;

#ifndef TIMAC_AGAMA_FPGA
    /* Turn off Synth */
    RFCSynthPowerDown();
#endif

    /* Closing all handles */
    if (!nClients)
    {
        /* Release the semaphore to be sure no one is pending on it */
        SemaphoreP_post(&pCurrClient->state.semSync);
    }

    /* Turn off CPE by disabling its clock */
    RFCClockDisable();
}

/*-------------- Power constraints internal functions ------------------------*/

/*
 * Set RF power constraints.
 *
 * Input:  src - RF_PowerConstraintSrc (Source: Queue or RAT)
 * Return: status
 */
RF_Stat RF_setPowerConstraint(RF_PowerConstraintSrc src)
{
    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Set constraint based on source */
    RF_powerConstraint |= src;

    /* Exit critical section */
    HwiP_restore(key);

    return RF_StatSuccess;
}

/*
 * Release RF power constraints.
 *
 * Input:  src - RF_PowerConstraintSrc (Source: Queue or RAT)
 * Return: status
 */
RF_Stat RF_releasePowerConstraint(RF_PowerConstraintSrc src)
{
     RF_Stat status = RF_StatRadioInactiveError;

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* No need to do anything if all constraints are clear already */
    if (RF_powerConstraint)
    {
        /* Release this constraint */
        RF_powerConstraint &= ~src;

        /* Check if all constraints are clear */
        if (RF_powerConstraint == 0)
        {
            /* Initiate power down if all constraints are clear */
            SwiP_or(&swiFsm, Fsm_EventPowerDown);
        }

        /* Return with success since the resource was cleared */
        status = RF_StatSuccess;
    }

    /* Exit critical section */
    HwiP_restore(key);

    /* Return with the status code */
    return status;
}

/*
 *  Check whether to dispatch operation immediately or power down radio and wait.
 *  If the former it returns 0, if the latter it returns the #us until when RF
 *  need to execute the command.
 *
 *  Calling context: Hwi, Swi
 *
 *  Input:   pOp - Pointer to radio operation command structure.
 *  Return:  #us - If command is far away in future.
 *           0   - If command is too close and should be scheduled now.
 */
static uint32_t calculateDispatchTime(RF_Op* pOp)
{
    uint32_t nPowerUpDuration;

    /* Only recognizes TRIG_ABSTIME triggers, everything else gets dispatched immediately */
    if (pOp->startTrigger.triggerType == TRIG_ABSTIME)
    {
        /* Calculate the difference of startTime from current time in us */
        uint32_t deltaTimeUs = (pOp->startTime - RF_getCurrentTime()) / RF_NUM_RAT_TICKS_IN_1_US;

        /* If pCurrClient is NULL (very initial state), use default powerup duration */
        if (pCurrClient)
        {
            nPowerUpDuration = pCurrClient->clientConfig.nPowerUpDuration;
        }
        else
        {
            nPowerUpDuration = RF_DEFAULT_POWER_UP_TIME;
        }

        /* Check if delta time is greater than (powerup duration + power down duration) for a
           power cycle, and is less than 3/4 of a RAT cycle (~17 minutes) */
        if ((deltaTimeUs > (int32_t)(nPowerUpDuration + RF_DEFAULT_POWER_DOWN_TIME)) &&
            (deltaTimeUs <= RF_DISPATCH_MAX_TIME_US))
        {
            /* Dispatch command in the future */
            return MAX((deltaTimeUs - nPowerUpDuration)/ ClockP_tickPeriod, 1);
        }
    }

    /* Dispatch command immediately */
    return 0;
}


/*
 *  Additional queue and state machine handling related to IEEE commands.
 *
 *  Calling context: Hwi, Task
 *
 *  Input:  control - Selects the core behaviour of the function.
 *  Return: none
 */
static void RF_ieeeExpansion(uint32_t control)
{
    /* If the running command is a BG command*/
    if(cmdQ.pCurrCmd && (cmdQ.pCurrCmd->flags & RF_CMD_BG_CMD_FLAG))
    {
        /* If the next command is a foreground command, which belongs to the same client.
           This implicitely means it is an IEEE command */
        if(cmdQ.pPend && !(cmdQ.pPend->flags & RF_CMD_BG_CMD_FLAG) &&
          (cmdQ.pCurrCmd->pClient == cmdQ.pPend->pClient))
        {
            switch(control)
            {
              case IEEE_REQUEST_FG_CMD: /* Post SWI, which will ensure that the BG
                                           command is activated before any FG command is posted */
                                        SwiP_or(&swiFsm, Fsm_EventIeeeInt);
                                        break;

              case IEEE_SUSPEND_BG_CMD: /* Internally store the pointer to the
                                           running background command */
                                        cmdQ.pCurrCmdBg = cmdQ.pCurrCmd;

                                        /* Clear the current command, so the
                                           dispatcher can execute the new foreground command */
                                        cmdQ.pCurrCmd   = NULL;
                                        break;
            }

        }
    }
}

/*
 *  Update the cached FS command within the client's context.
 *
 *  Calling context: Hwi, Swi
 *
 *  Input:  pCmd - Pointer to radio operation command.
 *  Return: none
 */
static void cacheFsCmd(RF_Cmd* pCmd)
{
    /* Upper limit of the number of operations in a chain */
    uint8_t nCmdChainMax = RF_MAX_CHAIN_CMD_LEN;

    /* Traverse the chain */
    RF_Op* pOp = pCmd->pOp;
    while (pOp && nCmdChainMax)
    {
        /* If the operation is a CMD_FS or CMD_FS_OFF */
        if ((pOp->commandNo == CMD_FS) || (pOp->commandNo == CMD_FS_OFF))
        {
            /* Create a copy of the first CMD_FS command (or CMD_FS_OFF) for later power up */
            memcpy(&pCmd->pClient->state.mode_state.cmdFs, pOp, sizeof(pCmd->pClient->state.mode_state.cmdFs));
            break;
        }

        /* Step the chain */
        pOp = pOp->pNextOp;

        /* Avoid infinit loop (in case of closed loops) */
        --nCmdChainMax;
    }
}

/*
 *  Submit the pending command to the RF Core.
 *
 *  Input:  none
 *  Return: none
 */
static void RF_dispatchNextCmd()
{
    /* First element in the pend queue */
    RF_Cmd* pNextCmd = Q_peek(&cmdQ.pPend);

    /* If the running command is a BG command and there is a FG in the pend queue, suspend it*/
    RF_ieeeExpansion(IEEE_SUSPEND_BG_CMD);

    /* If there is no running command, dispatch the next pending in the queue */
    if (!cmdQ.pCurrCmd)
    {
        if(pNextCmd)
        {
            if (pNextCmd->pClient != pCurrClient)
            {
                /* We need to change radio client, signal to FSM. */
                SwiP_or(&swiFsm, Fsm_EventInitChangePhy);
            }
            else
            {
                /* Calculate the time of execution of command */
                uint32_t dispatchTime = calculateDispatchTime(pNextCmd->pOp);

                /* Dispatch command in the future */
                if (dispatchTime && !cmdQ.pCurrCmdBg)
                {
                    /* Command sufficiently far into future that it shouldn't be dispatched yet
                       Release RF power constraint and potentially power down radio */
                    if (RF_releasePowerConstraint(RF_PowerConstraintCmdQ) == RF_StatSuccess)
                    {
                        /* Stop inactivity timer */
                        ClockP_stop(&pCurrClient->state.clkInactivity);

                        /* Start the clock to trigger radio wakeup */
                        ClockP_setTimeout(&clkPowerUp, dispatchTime);
                        ClockP_start(&clkPowerUp);
                    }
                }
                else
                {
                    /* Dispatch command immediately. Enable the requested interrupt sources of the command */
                    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) |= (pNextCmd->bmEvent &  RF_INTERNAL_CPE_IFG_MASK_32_BIT);
                    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN)  |= (pNextCmd->bmEvent >> RF_SHIFT_32_BITS);

                    /* Dispatch immediately, set command to current and remove from pending queue */
                    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
                    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR)     = (uint32_t)pNextCmd->pOp;
                    while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));

                    /* Command now sent to the RF Core. Move it from pending queue to the current command */
                    cmdQ.pCurrCmd = Q_pop(&cmdQ.pPend);

                    /* Check pend queue for any foreground command (IEEE 15.4 mode) */
                    RF_ieeeExpansion(IEEE_REQUEST_FG_CMD);
                }
            }
        }
        else
        {
            /* No more pending command in the queue */
            SwiP_or(&swiFsm, Fsm_EventLastCommandDone);
        }
    }
}

/*
 *  Check if there was an error with the synth while running CMD_FS
 *  error callback is not issued in this function.
 *
 *  Input:  none
 *  Return: true  - If there was an error.
 *          false - If there was no error.
 */
static bool RF_checkCmdFsError(void)
{
    /* Take the handle of the current client */
    RF_Handle pObj = pCurrClient;

    /* Find the FS command stored in the context of the client */
    RF_Op *tmp1 = (RF_Op*)&pObj->clientConfig.pOpSetup->prop;
    while (tmp1->pNextOp && tmp1->pNextOp != (RF_Op*)&pObj->state.mode_state.cmdFs)
    {
      tmp1 = tmp1->pNextOp;
    }

    /* Evaluate if the FS command succeeded */
    if ((tmp1->condition.rule == COND_ALWAYS) &&
        (pObj->state.mode_state.cmdFs.status == ERROR_SYNTH_PROG))
    {
       /* CMD_FS completed with error so return true */
       return true;
    }
    else
    {
       /* There is no synth error so return false */
       return false;
    }
}

/*
 *  RF HW ISR when radio is active.
 *
 *  Input:  na
 *  Return: none
 */
static void hwiFxnHw(uintptr_t a)
{
    /* Prepare a direct command */
    RF_Cmd*       pCmd;
    RF_CmdDirImm* pDirImmCmd = &cmdDirImm;

    /* Read and clear the interrupt flags */
    uint32_t rfchwifg = RFCHWIntGetAndClear(RF_INTERNAL_HW_IFG_MASK_32_BIT | RF_HW_INT_RAT_CH_MASK);
    uint32_t rfchwien = HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) & RF_INTERNAL_HW_IFG_MASK_32_BIT;
    uint32_t rathwien = HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) & RF_HW_INT_RAT_CH_MASK;

    if ((pCmd = cmdQ.pCurrCmd))
    {
        /* Post SWI_FSM if MODEM_SOFT event occured and the interrupt was enabled */
        if (rfchwifg & rfchwien)
        {
            /* Store the command which callback need to be served */
            cmdQ.pCurrCmdCb = pCmd;

            /* Accumulate the event within the command chain. This will be used if RF_pendCmd is called. */
            pCmd->pastifg |= ((RF_EventMask)rfchwifg << RF_SHIFT_32_BITS);

            /* Trig the state machine to handle this event */
            SwiP_or(&swiFsm, Fsm_EventHwInt);
        }
    }

    /* Post the SWI_HW if any RAT channel event occured */
    if (rfchwifg & rathwien)
    {
        /* Store the channel which cause the interrupt */
        pDirImmCmd->hwIntEvent |= (rfchwifg & rathwien);

        /* Post the swi to handle its callback */
        SwiP_or(&swiHw, 0);
    }
}

/*
 *  RF HW SWI used for user callback to any RAT event (RF_HW HWI).
 *
 *  Input:  na
 *  Return: none
 */
static void swiFxnHw(uintptr_t a, uintptr_t b)
{
    /* Prepare a direct command */
    RF_CmdDirImm* pDirImmCmd = &cmdDirImm;

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* If the interrupt was trigged due to one of the RAT channels*/
    if (pDirImmCmd->hwIntEvent & RF_HW_INT_RAT_CH_MASK)
    {
        /* Convert the RAT channel Id */
        uint8_t ratCbIdx = pDirImmCmd->hwIntEvent >> RFC_DBELL_RFHWIFG_RATCH5_BITN;

        /* Process lower channel first and allow multiple interrupt flags to be processed sequentially */
        if (ratCbIdx & (1 << RF_RAT_CH_5_INDEX))
        {
            ratCbIdx = RF_RAT_CH_5_INDEX;
        }
        else if (ratCbIdx & (1 << RF_RAT_CH_6_INDEX))
        {
            ratCbIdx = RF_RAT_CH_6_INDEX;
        }
        else if (ratCbIdx & (1 << RF_RAT_CH_7_INDEX))
        {
            ratCbIdx = RF_RAT_CH_7_INDEX;
        }

        /* Free RAT channel but do not release RAT constraint yet */
        ratChanFree(ratCbIdx + RF_RAT_CH_LOWEST, false);

        /* Clear the interrupt flag */
        pDirImmCmd->hwIntEvent &= ~(1 << (ratCbIdx + RFC_DBELL_RFHWIFG_RATCH5_BITN));

        /* Exit critical section */
        HwiP_restore(key);

        /* Issue RAT callback if it is provided */
        if (pDirImmCmd->pRatCb[ratCbIdx])
        {
            pDirImmCmd->pRatCb[ratCbIdx](pDirImmCmd->pClient[ratCbIdx],
                                         ratCbIdx + RF_RAT_CH_LOWEST,
                                         RF_EventRatCh);
        }

        /* Check for any pending RAT channels running and release RAT power constraint
           if possible */
        if (!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN))
        {
            RF_releasePowerConstraint(RF_PowerConstraintRat);
        }
    }
    else
    {
        /* Exit critical section */
        HwiP_restore(key);
    }

    /* Trigg self again if multiple interrupt flags are still set */
    if (pDirImmCmd->hwIntEvent)
    {
        SwiP_or(&swiHw, 0);
    }
}

/*
 *  RF CPE0 ISR when radio is active. Assume that all IRQs relevant to command
 *  dispatcher are mapped here. Furthermore, assume that there is no need for
 *  critical sections here (i.e. that this ISR has higher priority than
 *  any HWI calling a RF API function or that HWIs can't call the RF API).
 *
 *  Input:  na
 *  Return: none
 */
static void hwiFxnCpe0Active(uintptr_t a)
{
    /* Local variable */
    RF_Cmd* pCmd;

    /* Get and clear doorbell IRQs */
    uint32_t rfcpeifg = RFCCpeIntGetAndClear();

    /* If there was a command running */
    if ((pCmd = cmdQ.pCurrCmd))
    {
        /* The interrupt is either LAST_FG_COMMAND_DONE or LAST_CMD_DONE (only acceptable if there is no suspended background command) */
        if ((rfcpeifg &  RFC_DBELL_RFCPEIFG_LAST_FG_COMMAND_DONE_M) || ((rfcpeifg & RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M) && !cmdQ.pCurrCmdBg))
        {
            /* Retire the command by reseting the current command */
            cmdQ.pCurrCmd = NULL;

            /* Save RFCPEIFG for callback */
            pCmd->rfifg |= ((rfcpeifg & (RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M | RFC_DBELL_RFCPEIFG_LAST_FG_COMMAND_DONE_M | pCmd->bmEvent)));

            /* Accumulate the event within the command chain. This will be used if RF_pendCmd is called. */
            pCmd->pastifg |= (RF_EventMask)pCmd->rfifg;

            /* Disable interrupt sources were subsribed by the command which is done now */
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) &= ~(pCmd->bmEvent &  RF_INTERNAL_CPE_IFG_MASK_32_BIT);
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN)  &= ~(pCmd->bmEvent >> RF_SHIFT_32_BITS);

            /* Move current command to done queue */
            Q_push(&cmdQ.pDone, pCmd, NULL);

            /* Post SWI to handle registered callbacks and potentially start inactivity timer */
            SwiP_or(&swiFsm, Fsm_EventLastCommandDone);

            /* Check for background command in IEEE 15.4 mode */
            if((cmdQ.pCurrCmdBg) && (pCmd->pClient == cmdQ.pCurrCmdBg->pClient))
            {
                /* Determine if we need to restore the background command or not */
                uint8_t cancelFlags = (cmdQ.pCurrCmdBg->flags | pCmd->flags) & RF_CMD_CANCEL_MASK;

                /* Retire the command if it was aborted or stopped */
                if(cancelFlags)
                {
                    /* Set the flags based on priority */
                    if(cancelFlags & RF_CMD_ABORTED_FLAG)
                    {
                      cmdQ.pCurrCmdBg->flags |= RF_CMD_ABORTED_FLAG;
                    }
                    else if(cancelFlags & RF_CMD_STOPPED_FLAG)
                    {
                      cmdQ.pCurrCmdBg->flags |= RF_CMD_ABORTED_FLAG;
                    }
                    else
                    {
                      cmdQ.pCurrCmdBg->flags |= cancelFlags;
                    }

                    /* Push the command to the done queue */
                    Q_push(&cmdQ.pDone, cmdQ.pCurrCmdBg, NULL);
                }
                else
                {
                    /* Restore the background command */
                    cmdQ.pCurrCmd = cmdQ.pCurrCmdBg;

                    /* Reenable interrupts subscribed by the background command */
                    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) |= (cmdQ.pCurrCmdBg->bmEvent &  RF_INTERNAL_CPE_IFG_MASK_32_BIT);
                    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN)  |= (cmdQ.pCurrCmdBg->bmEvent >> RF_SHIFT_32_BITS);
                }

                /* The background command is either restored or retired at this point. No need or storing it in the pCurrCmdBg */
                cmdQ.pCurrCmdBg = NULL;
            }
        }
        else if (rfcpeifg & (pCmd->bmEvent))
        {
            /* The interrupt is for other reasons than (LAST_COMMAND_DONE or LAST_FG_COMMAN_DONE) and the command is subscribed for this event */
            cmdQ.pCurrCmdCb = pCmd;

            /* Save CPEIFG for callback */
            pCmd->rfifg |= (rfcpeifg & pCmd->bmEvent);

            /* Accumulate the event within the command chain. This will be used if RF_pendCmd is called. */
            pCmd->pastifg |= (RF_EventMask)pCmd->rfifg;

            /* Post SWI to handle registered callbacks */
            SwiP_or(&swiFsm, Fsm_EventCpeInt);
        }
    }

    /* Dispatch the next pending command if exists */
    RF_dispatchNextCmd();
}

/*
 *  Clock callback due to inactivity timeout.
 *
 *  Input:  na
 *  Return: none
 */
static void clkInactivityFxn(RF_Object* pObj)
{
    /* If there are no pending commands in the queue */
    if (cmdQ.nSeqPost == cmdQ.nSeqDone)
    {
        /* Release the constraint on the command queue and if nothing prevents, power down the radio */
        RF_releasePowerConstraint(RF_PowerConstraintCmdQ);
    }
}

/*
 *  Clock callback due to request access timeout.
 *
 *  Input:  na
 *  Return: none
 */
static void clkReqAccessFxn(RF_Object* pObj)
{
    RF_issueRadioFreeCb(RF_RADIOFREECB_REQACCESS_FLAG   |
                        RF_RADIOFREECB_PREEMPT_FLAG     |
                        RF_RADIOFREECB_CMDREJECT_FLAG);
}

/*
 *  Callback used to post semaphore for runCmd() and pendCmd().
 *
 *  Input:  h    - Handle to the client.
 *          ch   - Handle to the command which callback to be invoked.
 *          e    - Events causing the function call.
 *  Return: na
 */
static void syncCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    /* Local variables */
    RF_Cmd* pCmd;

    /* If there is a user callback provided */
    if (h->state.pCbSync != NULL)
    {
        /* Invoke the user callback */
        ((RF_Callback)h->state.pCbSync)(h, ch, e);
    }

    /* Mask the possible causes of releasing the semaphore */
    RF_EventMask maskedEvents = (e & h->state.eventSync);

    /* Release the semaphore on any of the reasons: last command done,
       subscribed event happened, last FG command is done in IEEE mode */
    if (maskedEvents)
    {
        /* Find the command. We do it here within the SWI context. */
        pCmd = getCmd(h, ch, RF_CMD_ALLOC_FLAG);

        /* Store the events in the context of the client */
        h->state.unpendCause = maskedEvents;

        /* Find the command. We do it here within the SWI context. */
        if(pCmd)
        {
          /* Clear the handled past events so it is possible to pend again */
          pCmd->pastifg &= ~h->state.unpendCause;

          /* Exhange the callback function: use the user callback from this point */
          pCmd->pCb = (RF_Callback)h->state.pCbSync;
        }

        /* Clear temporary storage of user callback (it was restored and served at this point) */
        h->state.pCbSync = NULL;

        /* Post the semaphore to release the RF_pendCmd() */
        SemaphoreP_post(&h->state.semSync);
    }
}

/*
 *  Default callback function.
 *
 *  Input:  h    - Handle to the client.
 *          ch   - Handle to the command which callback to be invoked.
 *          e    - Events causing the function call.
 *  Return: na
 */
static void defaultCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
  /* Do nothing */;
}

/*-------------- RF powerup/powerdown FSM functions ---------------*/

/*
 *  Set target state transition.
 *
 *  Input:  pTarget - Pointer to the next state.
 *  Return: na
 */
static void FsmNextState(Fsm_StateFxn pTarget)
{
    pFsmState = pTarget;
}

/*
 *  The SWI handler for FSM events.
 *
 *  Input:  na
 *  Return: none
 */
static void swiFxnFsm(uintptr_t a0, uintptr_t a1)
{
    pFsmState(pCurrClient, (Fsm_Event)SwiP_getTrigger());
}

/*
 *  Clock callback called upon powerup.
 *
 *  Input:  na
 *  Return: none
 */
static void clkPowerUpFxn(uintptr_t a)
{
    if (pFsmState == fsmActiveState)
    {
        /* Set power constraint on the command queue */
        RF_setPowerConstraint(RF_PowerConstraintCmdQ);

        /* Trig the HWI to dispatch the next command */
        HwiP_post(INT_RFC_CPE_0);
    }
    else
    {
        /* Trigger FSM SWI to start the wake up sequence of the radio */
        SwiP_or(&swiFsm, Fsm_EventWakeup);
    }
}

/*
 *  RF CPE0 ISR during FSM powerup/powerdown.
 *
 *  Input:  na
 *  Return: none
 */
static void hwiFxnCpe0PowerFsm(uintptr_t a0)
{
    /* Read all IRQ flags in doorbell and then clear them */
    uint32_t rfcpeifg = RFCCpeIntGetAndClear();

    /* If the radio is active */
    if (pFsmState == fsmActiveState)
    {
        /* Change HWI handler to the correct one */
        HwiP_setFunc(&hwiCpe0, hwiFxnCpe0Active, (uintptr_t)NULL);

        /* Mark radio and client as being active */
        RF_setPowerConstraint(RF_PowerConstraintCmdQ);
        bRadioIsActive         = true;
        bRadioIsBeingPoweredUp = false;

        /* If this is the first power-up for a RAT channel after a power cycle then
           send CMD_GET_FW_INFO command to update the value available RAT channels */
        if (cmdDirImm.availRatCh == RF_DEFAULT_AVAILRATCH_VAL)
        {
            /* Prepare the radio command */
            rfc_CMD_GET_FW_INFO_t getFwInfoCmd;
            getFwInfoCmd.commandNo = CMD_GET_FW_INFO;

            /* Sent the command to the RF core */
            RFCDoorbellSendTo((uint32_t)&getFwInfoCmd);

            /* Read the updated data and store it in a global variable */
            cmdDirImm.availRatCh = getFwInfoCmd.availRatCh;
        }

        /* No synth error */
        if (!RF_checkCmdFsError())
        {
            /* Dispatch the next command */
            RF_dispatchNextCmd();
        }
    }

    /* Handle special events as boot, etc */
    if ((rfcpeifg & IRQ_BOOT_DONE) || (rfcpeifg & IRQ_LAST_COMMAND_DONE))
    {
        SwiP_or(&swiFsm, Fsm_EventPowerStep);
    }
}

/*
 *  RF CPE0 ISR during Change PHY switching.
 *
 *  Input:  na
 *  Return: none
 */
static void hwiFxnCpe0ChangePhy(uintptr_t a0)
{
    /* Clear all IRQ flags in doorbell and then clear them */
    uint32_t rfcpeifg = RFCCpeIntGetAndClear();

    if (rfcpeifg & IRQ_LAST_COMMAND_DONE)
    {
        /* Proceed to the second phase of the phy switching process */
        SwiP_or(&swiFsm, Fsm_EventChangePhy2);
    }
}

/*-------------- Power management state functions ---------------*/
/*
 *  Handles RF Core patching for CPE, MCE, RFE (if required) in setup state during power-up.
 *
 *  Input:  mode   - RF_PHY_BOOTUP_MODE:    First boot of the RF core.
 *                 - RF_PHY_SWITCHING_MODE: Switching between two phys.
 *  Return: none
 */
static void applyRfCorePatch(bool mode)
{
    /* Patch CPE if relevant */
    if (pCurrClient->clientConfig.pRfMode->cpePatchFxn)
    {
        if (mode == RF_PHY_SWITCHING_MODE)
        {
            RFCCPEPatchReset();
        }
        pCurrClient->clientConfig.pRfMode->cpePatchFxn();
    }

    if ((pCurrClient->clientConfig.pRfMode->mcePatchFxn != 0) || (pCurrClient->clientConfig.pRfMode->rfePatchFxn != 0))
    {
        /* Wait for clocks to be turned ON */
        while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;

        /* Patch MCE if relevant */
        if (pCurrClient->clientConfig.pRfMode->mcePatchFxn)
        {
            pCurrClient->clientConfig.pRfMode->mcePatchFxn();
        }

        /* Patch RFE if relevant */
        if (pCurrClient->clientConfig.pRfMode->rfePatchFxn)
        {
            pCurrClient->clientConfig.pRfMode->rfePatchFxn();
        }

        /* Turn off additional clocks */
        RFCDoorbellSendTo(CMDR_DIR_CMD_2BYTE(RF_CMD0, 0));
    }
}

/*
 *  Handle callback to client for RF_EventLastCmdDone and issue radio free callback if required.
 *
 *  Input:  none
 *  Return: none
 */
static void radioOpDoneCb()
{
    /* Serve the first entry in the done queue */
    RF_Cmd* pCmd = Q_peek(&cmdQ.pDone);

    /* Radio command done */
    if (pCmd)
    {
        /* Update implicit radio state (chained FS command if any) */
        cacheFsCmd(pCmd);

        /* Read and clear the events */
        uint32_t tmp = pCmd->rfifg;
        pCmd->rfifg  = 0;

        /* Issue callback, free container and dequeue */
        if (pCmd->pCb)
        {
            RF_EventMask cb_e;

            /* If the command was cancelled, stopped or aborted, overwrite the events */
            switch (pCmd->flags & RF_CMD_CANCEL_MASK)
            {
                case RF_CMD_ABORTED_FLAG:
                    cb_e = RF_EventCmdAborted;
                    break;
                case RF_CMD_STOPPED_FLAG:
                    cb_e = RF_EventCmdStopped;
                    break;
                case RF_CMD_CANCELLED_FLAG:
                    cb_e = RF_EventCmdCancelled;
                    break;
                default:
                    cb_e = (RF_EventMask)(tmp);
                    break;
            }

            /* Handle the special case when a command was prempted */
            if ((pCmd->flags & RF_CMD_PREEMPT_FLAG) && (pCmd->flags & RF_CMD_CANCEL_MASK))
            {
                cb_e |= RF_EventCmdPreempted;
            }

            /* Invoke the use callback */
            pCmd->pCb(pCmd->pClient, pCmd->ch, cb_e);
        }

        /* Enter critical section */
        uint32_t key = HwiP_disable();

        /* Update num of radio command done */
        cmdQ.nSeqDone = (cmdQ.nSeqDone+1) & N_CMD_MODMASK;

        /* Commmand completed reset command flags  */
        cmdQ.pDone->flags = 0;

        /* Command completed, free command queue container */
        Q_pop(&cmdQ.pDone);

        /* Exit critical section */
        HwiP_restore(key);

        /* Check if there are any more pending commands */
        if (cmdQ.nSeqDone == cmdQ.nSeqPost)
        {
            /* Issue radio free callback after pre-emption if required */
            uint8_t tmp = RF_RADIOFREECB_PREEMPT_FLAG | RF_RADIOFREECB_CMDREJECT_FLAG;

            /* If the radio was yielded, add the flag */
            if (pCmd->pClient->state.bYielded)
            {
                tmp |= RF_RADIOFREECB_REQACCESS_FLAG;
            }

            /* Call the radio free callback */
            RF_issueRadioFreeCb(tmp);

            /* Identify which is the other client */
            RF_Object *pOtherClient = RF_Sch.clientHnd[0];
            if (pCmd->pClient == RF_Sch.clientHnd[0])
            {
                pOtherClient = RF_Sch.clientHnd[1];
            }

            /* If the client was not yielded, update the inactivity time */
            uint32_t inactivityTime = 0;
            if (!(pCmd->pClient->state.bYielded))
            {
                inactivityTime = pCmd->pClient->clientConfig.nInactivityTimeout;
            }

            /* Clear bYielded flag for this client */
            pCmd->pClient->state.bYielded = false;

            /* Check bYielded flag and inactivity time for the other client */
            if (pOtherClient != NULL)
            {
                if (pOtherClient->state.bYielded)
                {
                    /* Clear bYielded flag for the other clients */
                    pOtherClient->state.bYielded = false;
                }
                else
                {
                    /* Set the inactivity time to the max between the two clients */
                    inactivityTime = MAX(inactivityTime, pOtherClient->clientConfig.nInactivityTimeout);
                }
            }

            /* If immediate power down is reuqested  */
            if (inactivityTime == 0)
            {
                /* We can powerdown immediately */
                RF_releasePowerConstraint(RF_PowerConstraintCmdQ);
            }
            else
            {
                /* If there is a valid inactivity time set */
                if (inactivityTime != SemaphoreP_WAIT_FOREVER)
                {
                    /* Reprogram and start inactivity timer */
                    uint32_t timeout = MAX(inactivityTime/ClockP_tickPeriod, 1);
                    ClockP_setTimeout(&pCmd->pClient->state.clkInactivity, timeout);
                    ClockP_start(&pCmd->pClient->state.clkInactivity);
                }
            }
        }
    }
}

/*
 *  RF state machine function during power up state.
 *
 *  Input:  pObj - Pointer to RF object.
 *          e    - State machine event.
 *  Return: none
 */
static void fsmPowerUpState(RF_Object *pObj, Fsm_Event e)
{
    /* Note: pObj is NULL in this state */
    if (e & Fsm_EventLastCommandDone)
    {
        /* Invoke the user provided callback function */
        radioOpDoneCb();

        /* We've handled this event now */
        e &= ~Fsm_EventLastCommandDone;

        /* Retrig the SWI if there is more command to be served */
        if (Q_peek(&cmdQ.pDone))
        {
            /* Trigger self if there are more commands in callback queue */
            SwiP_or(&swiFsm, (e | Fsm_EventLastCommandDone));
        }
        else if ((e & Fsm_EventWakeup) || (Q_peek(&cmdQ.pPend)))
        {
            /* If there is pending cmd, just wake up
               Device will put to sleep if the pending cmd is future cmd */
            ClockP_stop(&clkPowerUp);

            /* Trigger power up */
            SwiP_or(&swiFsm, Fsm_EventWakeup);
        }
    }
    else if(e & Fsm_EventWakeup)
    {
        /* Notify the power driver that FLASH is needed in IDLE */
        bDisableFlashInIdleConstraint = true;
        Power_setConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);

        /* Store the current RTC tick for nPowerUpDuration calculation */
        powerDurCalc.nRtc1 = AONRTCCurrent64BitValueGet();

        /* Set current client from first command in command queue */
        RF_Cmd* pNextCmd = Q_peek(&cmdQ.pPend);
        if (pNextCmd)
        {
            RF_Object* pNextClient = pNextCmd->pClient;

            /* If the next command belongs to another client, initiate PHY switching */
            if ((pCurrClient) && (pCurrClient != pNextClient))
            {
                /* Invoke the client switch callback if it was provided */
                if (pNextClient->clientConfig.nClientEventMask & RF_ClientEventSwitchClientEntered)
                {
                    RF_ClientCallback pClientEventCb = (RF_ClientCallback)pNextClient->clientConfig.pClientEventCb;
                    pClientEventCb(pNextClient, RF_ClientEventSwitchClientEntered, NULL);
                }
            }

            /* Set the current client to be the next client */
            pCurrClient = pNextClient;
        }

        /* Set the RF mode in the PRCM register (RF_open already verified that it is valid) */
        HWREG(PRCM_BASE + PRCM_O_RFCMODESEL) = pCurrClient->clientConfig.pRfMode->rfMode;

        /* Notiy the power driver that Standby is not allowed and RF core need to be powered */
        Power_setConstraint(PowerCC26XX_SB_DISALLOW);
        Power_setDependency(PowerCC26XX_DOMAIN_RFCORE);

        /* Indicate that the power-up sequence is being started */
        bRadioIsBeingPoweredUp = true;

        /* Notify the power driver that the XOSC_HF is needed by the RF core
           - Only for the first power up
           - The client explicitely asked for it */
        if((pCurrClient->clientConfig.bPowerUpXOSC == false) || bRadioHasBeenBootedBefore == false)
        {
            Power_setDependency(PowerCC26XX_XOSC_HF);
        }

        /* If there are RFE and MCE patches, turn on their clocks */
        if ((pCurrClient->clientConfig.pRfMode->mcePatchFxn != 0) || (pCurrClient->clientConfig.pRfMode->rfePatchFxn != 0))
        {
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR)     = CMDR_DIR_CMD_2BYTE(RF_CMD0, RFC_PWR_PWMCLKEN_MDMRAM | RFC_PWR_PWMCLKEN_RFERAM);
        }

        /* Turn on the clock to the RF core */
        RFCClockEnable();

        /* Map all radio interrupts to CPE0 and enable some sources */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEISL) = 0;

        /* Enable some of the interrupt sources */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) = RFC_DBELL_RFCPEIEN_BOOT_DONE_M |
                                                       RFC_DBELL_RFCPEIEN_LAST_FG_COMMAND_DONE_M |
                                                       RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M;

        /* Next state: fsmSetupState*/
        FsmNextState(&fsmSetupState);

        /* Enable interrupts: continue when boot is done */
        HwiP_enableInterrupt(INT_RFC_CPE_0);
    }
}

/*
 *  RF state machine function during setup state.
 *
 *  Input:  pObj - Pointer to RF object.
 *          e    - State machine event.
 *  Return: none
 */
static void fsmSetupState(RF_Object *pObj, Fsm_Event e)
{
    if (e & Fsm_EventPowerStep)
    {
        /* Apply RF Core patches (if required) */
        applyRfCorePatch(RF_PHY_BOOTUP_MODE);

        /* Set VCOLDO reference to true */
        RFCAdi3VcoLdoVoltageMode(true);

        /* Next state: fsmActiveState */
        FsmNextState(&fsmActiveState);

        /* Configure the RAT_SYNC command which will follow SETUP command */
        opRatSync.start.commandNo                = CMD_SYNC_START_RAT;
        opRatSync.start.status                   = IDLE;
        opRatSync.start.startTrigger.triggerType = TRIG_NOW;
        opRatSync.start.pNextOp                  = NULL;
        opRatSync.start.condition.rule           = COND_NEVER;

        /* Configure the SETUP command */
        RF_RadioSetup* pOpSetup = pObj->clientConfig.pOpSetup;
        pOpSetup->prop.status   = IDLE;

        /* For power saving - analogCfgMode is set to 0 only for the first power up */
        if ((bRadioHasBeenBootedBefore == false) || (pObj->clientConfig.bUpdateSetup))
        {
            if ((pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_RADIO_SETUP) ||
                (pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_BLE5_RADIO_SETUP))
            {
                /* BLE, BLE 5.0 modes share same offset */
                pOpSetup->common.config.analogCfgMode = RF_SETUP_ANALOGCFG_UPDATE;
            }
            else
            {
                /* PROPRIETARY mode */
                pOpSetup->prop.config.analogCfgMode = RF_SETUP_ANALOGCFG_UPDATE;
            }

            /* Clear the update request flag */
            pObj->clientConfig.bUpdateSetup = false;
        }

        /* Trim directly the radio register values based on the ID of setup command */
        rfTrim_t rfTrim;
        RFCRfTrimRead((rfc_radioOp_t*)pObj->clientConfig.pOpSetup, (rfTrim_t*)&rfTrim);
        RFCRfTrimSet((rfTrim_t*)&rfTrim);

        /* Search or speific commands in the command chain */
        RF_Op* tmp = (RF_Op*)&pOpSetup->prop;
        while ((tmp->pNextOp) && (tmp->pNextOp->commandNo != CMD_SYNC_START_RAT) &&
               (tmp->pNextOp->commandNo != CMD_FS) &&
               (tmp->pNextOp->commandNo != CMD_FS_OFF))
        {
            /* Trace to the end of chain */
            tmp = tmp->pNextOp;
        }

        /* Add the CMD_RAT_SYNC to the end of chain */
        tmp->pNextOp         = (RF_Op*)&opRatSync.start;
        tmp->condition.rule  = COND_ALWAYS;

        /* Setup FS command to follow SETUP command */
        rfc_CMD_FS_t* pOpFs;
        RF_Op* pOpFirstPend = Q_peek(&cmdQ.pPend)->pOp;
        if ((pOpFirstPend->commandNo == CMD_FS) || (pOpFirstPend->commandNo == CMD_FS_OFF))
        {
            /* First command is FS command so no need to chain an implicit FS command -> Reset nRtc1 */
            powerDurCalc.nRtc1 = 0;
        }
        else
        {
            if (pObj->state.mode_state.cmdFs.commandNo)
            {
                /* Chain in the implicit FS command */
                pOpFs                           = &pObj->state.mode_state.cmdFs;
                pOpFs->status                   = IDLE;
                pOpFs->pNextOp                  = NULL;
                pOpFs->startTrigger.triggerType = TRIG_NOW;
                pOpFs->condition.rule           = COND_NEVER;
                opRatSync.start.pNextOp         = (RF_Op*)pOpFs;
                opRatSync.start.condition.rule  = COND_ALWAYS;
            }
        }

        /* Power up the frequency synthesizer when SWTUP command runs */
        pObj->clientConfig.pOpSetup->common.config.bNoFsPowerUp = 0;

        /* Run the XOSC_HF switching if the pre-notify function setup the power
           constraint PowerCC26XX_SWITCH_XOSC_HF_MANUALLY */
        if (bRadioDelegateXOSCHFSwitching == true)
        {
            /* Wait untill the XOSC_HF is stable */
            while(!PowerCC26XX_isStableXOSC_HF());

            /* Invoke the XOSC_HF switching */
            PowerCC26XX_switchXOSC_HF();
        }
#ifndef TIMAC_AGAMA_FPGA        
        else if (OSCClockSourceGet(OSC_SRC_CLK_HF) != OSC_XOSC_HF)
        {
            /* If the XOSC_HF is not ready yet, only execute the first hal of the chain*/
            tmp->condition.rule = COND_NEVER;

            /* Next state: fsmXOSCState (polling XOSC_HF)*/
            FsmNextState(&fsmXOSCState);
        }
#endif
        /* Send the setup chain to the RF core */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = (uint32_t)&pObj->clientConfig.pOpSetup->commandId;
    }
}

/*
 *  RF state machine function during XOSC state.
 *
 *  Input:  pObj - Pointer to RF object.
 *          e    - State machine event.
 *  Return: none
 */
static void fsmXOSCState(RF_Object *pObj, Fsm_Event e)
{
    if ((e & Fsm_EventPowerStep) || (e & Fsm_EventWakeup))
    {
        /* If XOSC_HF is now ready */
        if(OSCClockSourceGet(OSC_SRC_CLK_HF) == OSC_XOSC_HF)
        {
            /* Next state: fsmActiveState */
            FsmNextState(&fsmActiveState);

            /* Continue with the CMD_RAT_SYNC */
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = (uint32_t)&opRatSync.start;
        }
        else
        {
            /* Clock source not yet switched to XOSC_HF: schedule new polling */
            uint32_t timeout = MAX(RF_XOSC_HF_SWITCH_CHECK_PERIOD/ClockP_tickPeriod, 1);
            ClockP_setTimeout(&clkPowerUp, timeout);
            ClockP_start(&clkPowerUp);
        }
    }
}

/*
 *  RF state machine function during active state.
 *
 *  Input:  pObj - Pointer to RF object.
 *          e    - State machine event.
 *  Return: none
 */
static void fsmActiveState(RF_Object *pObj, Fsm_Event e)
{
    RF_Cmd*  pCmd;
    uint32_t rtcValTmp1;
    uint32_t rtcValTmp2;
    uint32_t rfifgTmp;
    uint32_t key;

    if (e & Fsm_EventCpeInt)
    {
        /* Enter critical section */
        key = HwiP_disable();

        /* Dereference the command which requested the callback*/
        pCmd = Q_peek(&cmdQ.pCurrCmdCb);

        /* If this is due to other event than LastCmdDone */
        if (pCmd && !(pCmd->rfifg & RF_EventLastCmdDone))
        {
            /* Temporarily store the reason of callback */
            rfifgTmp = pCmd->rfifg;

            /* Clear the events which are handled here */
            pCmd->rfifg &= (~rfifgTmp);

            /* Exit critical section */
            HwiP_restore(key);

            /* Invoke the user callback if it is provided */
            if (pCmd->pCb && rfifgTmp)
            {
                pCmd->pCb(pCmd->pClient, pCmd->ch, (RF_EventMask)rfifgTmp);
            }
        }
        else
        {
            /* Exit critical section */
            HwiP_restore(key);
        }

        /* We've handled this event now */
        e &= ~Fsm_EventCpeInt;
    }
    /* Coming from powerup states */
    else if (e & Fsm_EventPowerStep)
    {
        /* RF core boot process is now finished */
        HWREG(PRCM_BASE + PRCM_O_RFCBITS) |= RF_BOOT1;

        /* Release the constraint on the FLASH in IDLE */
        if (bDisableFlashInIdleConstraint)
        {
            Power_releaseConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);
            bDisableFlashInIdleConstraint = false;
        }

        /* Enter critical section */
        key = HwiP_disable();

        /* For power saving, set analogCfgMode to 0x2D */
        if ((pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_RADIO_SETUP) ||
            (pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_BLE5_RADIO_SETUP))
        {
            /* BLE, BLE 5.0 modes share same offset */
            pObj->clientConfig.pOpSetup->common.config.analogCfgMode = RF_SETUP_ANALOGCFG_NOUPDATE;
        }
        else
        {
            /* PROPRIETARY mode */
            pObj->clientConfig.pOpSetup->prop.config.analogCfgMode = RF_SETUP_ANALOGCFG_NOUPDATE;
        }

        /* Update power up duration if coming from the clkPowerUpFxn. Skip the calcualtion
           if coming from boot, since the LF clock is derived from RCOSC_HF without calibration. */
        if (powerDurCalc.nRtc1 && (OSCClockSourceGet(OSC_SRC_CLK_LF) != OSC_RCOSC_HF))
        {
            /* Take wake up timestamp and the current timestamp */
            rtcValTmp1  = (uint32_t)powerDurCalc.nRtc1;
            rtcValTmp2  = (uint32_t)AONRTCCurrent64BitValueGet();

            /* Temporary storage to be able to compare the new value to the old measurement */
            uint32_t prevPowerUpDuration = pObj->clientConfig.nPowerUpDuration;

            /* Calculate the difference of the timestamps and convert it to us units */
            pObj->clientConfig.nPowerUpDuration   = UDIFF(rtcValTmp1, rtcValTmp2);
            pObj->clientConfig.nPowerUpDuration >>= RF_RTC_CONV_TO_US_SHIFT;

            /* Add margin to the power up duration to account for the TIRTOS/Drivers overhead */
            pObj->clientConfig.nPowerUpDuration += pObj->clientConfig.nPowerUpDurationMargin;

            /* Low pass filter on power up durations less than in the previous cycle */
            if(prevPowerUpDuration > pObj->clientConfig.nPowerUpDuration)
            {
                /* Expect that the values are small and the calculation can be done in 32 bits */
                pObj->clientConfig.nPowerUpDuration = (prevPowerUpDuration + pObj->clientConfig.nPowerUpDuration)/2;
            }

            /* Power up duration should be within certain upper and lower bounds */
            if ((pObj->clientConfig.nPowerUpDuration > RF_DEFAULT_POWER_UP_TIME) ||
                (pObj->clientConfig.nPowerUpDuration < RF_DEFAULT_MIN_POWER_UP_TIME))
            {
                pObj->clientConfig.nPowerUpDuration = RF_DEFAULT_POWER_UP_TIME;
            }
        }

        /* Exit critical section */
        HwiP_restore(key);

        /* Check the status of the CMD_FS, if it was sent (chained) to the setup command.
           If it failed, return an error callback to the client.
           The client can either resend the CMD_FS or ignore the error as per Errata on PG2.1 */
        if (RF_checkCmdFsError())
        {
            /* Invoke the error callback: deault is do nothing */
            RF_Callback pErrCb = (RF_Callback)pObj->clientConfig.pErrCb;
            pErrCb(pObj, RF_ERROR_CMDFS_SYNTH_PROG, RF_EventError);

            /* Check if there is pending command */
            if (Q_peek(&cmdQ.pPend))
            {
                /* Make sure the next pending command gets dispatched by issuing CPE0 IRQ */
                HwiP_post(INT_RFC_CPE_0);
            }
            else
            {
                /* No pending command */
                e |= Fsm_EventLastCommandDone;
            }
        }

        /* Issue power up callback: the RF core is active */
        RF_Callback pPowerCb = (RF_Callback)pObj->clientConfig.pPowerCb;
        pPowerCb(pObj, 0, RF_EventPowerUp);

        /* We've handled this event now */
        e &= ~Fsm_EventPowerStep;
    }
    else if (e & Fsm_EventLastCommandDone)
    {
        /* Issue radio operation done callback */
        radioOpDoneCb();

        /* We've handled this event now */
        e &= ~Fsm_EventLastCommandDone;

        /* Take the next command in the done queue if any left */
        if (Q_peek(&cmdQ.pDone))
        {
            /* Trigger self if there are more commands in done queue */
            e |= Fsm_EventLastCommandDone;
        }
    }
    else if (e & Fsm_EventInitChangePhy)
    {
        /* Enter critical section */
        key = HwiP_disable();

        /* Take the next command from the pend queue */
        RF_Cmd* pNextCmd = Q_peek(&cmdQ.pPend);

        /* We will decide about the PHY change based on it's client */
        if (pNextCmd)
        {
            /* If there is an active client */
            if (pCurrClient)
            {
                /* Stop inactivity clock of the current client if running */
                ClockP_stop(&pCurrClient->state.clkInactivity);
            }

            /* Switch the current client to the commands client */
            pCurrClient = pNextCmd->pClient;

            /* Exit critical section */
            HwiP_restore(key);

            /* Store the timestamp or measurement of the switching time */
            powerDurCalc.nRtc2 = AONRTCCurrent64BitValueGet();

            /* Next state: fsmChangePhyState */
            FsmNextState(&fsmChangePhyState);

            /* Trigg the next state */
            SwiP_or(&swiFsm, Fsm_EventChangePhy1);
        }

        /* Cancel any canceling of ongoing power down request (we need to power down to change phy) */
        bRadioYieldWasCancelled = false;

        /* We've handled this event now */
        e &= ~Fsm_EventInitChangePhy;
    }
    else if (e & Fsm_EventPowerDown)
    {
        if(bRadioYieldWasCancelled == true)
        {
            /* Cancel any ongoing power down */
            bRadioYieldWasCancelled = false;
        }
        else if(cmdQ.pCurrCmd == NULL)
        {
            /* Indicate that the RF core is being powered down from now */
            bRadioIsActive           = false;
            bRadioIsBeingPoweredDown = true;

            /* Next state: fsmPowerUpState */
            FsmNextState(&fsmPowerUpState);

            /* Execute power down sequence of the RF core */
            RF_executePowerDown();

            /* Notify the power driver that Standby mode is allowed and the RF core can be powered down */
            Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);
            Power_releaseDependency(PowerCC26XX_DOMAIN_RFCORE);

            /* If there is no specific client request or the XOSC, release the dependency */
            if(pCurrClient->clientConfig.bPowerUpXOSC == false)
            {
                Power_releaseDependency(PowerCC26XX_XOSC_HF);
            }

            /* Release constraint of switching XOSC_HF from the RF driver itself */
            if (bRadioDelegateXOSCHFSwitching == true)
            {
                bRadioDelegateXOSCHFSwitching = false;
                Power_releaseConstraint(PowerCC26XX_SWITCH_XOSC_HF_MANUALLY);
            }

            /* Indicate that the RF core is now powered down */
            bRadioIsBeingPoweredDown   = false;
            bRadioYieldingWasRequested = false;

            /* If it was requested, immediately restart the power sequence */
            if (bRadioRestartIsRequested == true)
            {
                bRadioRestartIsRequested = false;
                SwiP_or(&swiFsm, Fsm_EventWakeup);
            }

            /* Issue radio available callback if RF_yield was called with no
               pending commands in the queue */
            uint8_t tmp = RF_RADIOFREECB_REQACCESS_FLAG;
            if (cmdQ.nSeqDone == cmdQ.nSeqPost)
            {
                tmp |= RF_RADIOFREECB_PREEMPT_FLAG | RF_RADIOFREECB_CMDREJECT_FLAG;
            }
            RF_issueRadioFreeCb(tmp);
        }
        else
        {
          /* Skip power down */
        }

        /* We've handled this event now */
        e &= ~Fsm_EventPowerDown;
    }
    else if (e & Fsm_EventHwInt)
    {
        /* RF_HW callback processing expect for RAT channel event */
        pCmd = Q_peek(&cmdQ.pCurrCmdCb);

        /* Invoke the callback of the command if provided */
        if (pCmd && pCmd->pCb)
        {
            pCmd->pCb(pCmd->pClient, pCmd->ch, RF_EventMdmSoft);
        }

        /* We've handled this event now */
        e &= ~Fsm_EventHwInt;
    }
    else if (e & Fsm_EventChangePhy2)
    {
        /* Check the status of the CMD_FS, if it was sent (chained) to the setup command.
           If it failed, invoke the error callback of the client.
           The client can either resend the CMD_FS or ignore the error. */
        if (RF_checkCmdFsError())
        {
            RF_Callback pErrCb = (RF_Callback)pObj->clientConfig.pErrCb;
            pErrCb(pObj, RF_ERROR_CMDFS_SYNTH_PROG, RF_EventError);
        }

        /* Change HWI handler */
        HwiP_setFunc(&hwiCpe0, hwiFxnCpe0Active, (uintptr_t)NULL);

        /* Indicate the phy switching is done and the RF core is active */
        bRadioPhyIsBeingSwitched = false;
        bRadioIsActive           = true;

        /* For power saving, set analogCfgMode to 0x2D and clear bUpdateSetup flag */
        if ((pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_RADIO_SETUP) ||
            (pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_BLE5_RADIO_SETUP))
        {
            /* BLE, BLE 5.0 modes share same offset */
            pObj->clientConfig.pOpSetup->common.config.analogCfgMode = RF_SETUP_ANALOGCFG_NOUPDATE;
        }
        else
        {
            /* PROPRIETARY mode */
            pObj->clientConfig.pOpSetup->prop.config.analogCfgMode = RF_SETUP_ANALOGCFG_NOUPDATE;
        }

        /* Disable the request update flag */
        pObj->clientConfig.bUpdateSetup = false;

        /* Record the timestamp for switching time measurement */
        rtcValTmp1 = (uint32_t)powerDurCalc.nRtc2;
        rtcValTmp2 = (uint32_t)AONRTCCurrent64BitValueGet();

        /* Locally store which client is the active client */
        uint8_t clientId = 1;

        /* Identiy the valid client ID */
        if (pCurrClient == RF_Sch.clientHnd[0])
        {
            clientId = 0;
        }

        /* Only compute PHY switching time if rtcValTmp1 is not zero (was initialized) */
        if (rtcValTmp1)
        {
            RF_Sch.phySwitchingTimeInUs[clientId]   = UDIFF(rtcValTmp1, rtcValTmp2);
            RF_Sch.phySwitchingTimeInUs[clientId] >>= RF_RTC_CONV_TO_US_SHIFT;
        }

        /* Reset nRtc2 value */
        powerDurCalc.nRtc2 = 0;

        /* Post hwi to dispatch the pending command in the command queue */
        HwiP_post(INT_RFC_CPE_0);

        /* We have handled this event now */
        e &= ~Fsm_EventChangePhy2;
    }
    else if (e & Fsm_EventIeeeInt)
    {
        /* Wait until the RF core starts to execute the command */
        while((cmdQ.pCurrCmd->pOp->status == IDLE) ||   /* Idle */
              (cmdQ.pCurrCmd->pOp->status == PENDING)); /* Pending */

        /* Trigg the HWI to dispatch the new oreground command */
        HwiP_post(INT_RFC_CPE_0);

        /* We have handled this event now */
        e &= ~Fsm_EventIeeeInt;
    }
    else
    {
        /* No other FSM event has any effect */
    }

    /* Call self again if there are outstanding events to be processed */
    if (e)
    {
        /* Trig the SWI with the remained/unhandled events */
        SwiP_or(&swiFsm, e);
    }
}

/*
 *  RF state machine function during PHY change state.
 *
 *  Input:  pObj - pointer to RF object
 *          e    - state machine event
 *  Return: none
 */
static void fsmChangePhyState(RF_Object *pObj, Fsm_Event e)
{
    if (e & Fsm_EventChangePhy1)
    {
        /* Indicate that we are changing phy on the RF core */
        bRadioIsActive           = false;
        bRadioPhyIsBeingSwitched = true;

        /* Do client switch callback if provided */
        if (pCurrClient->clientConfig.nClientEventMask & RF_ClientEventSwitchClientEntered)
        {
            RF_ClientCallback pClientEventCb = (RF_ClientCallback)pCurrClient->clientConfig.pClientEventCb;
            pClientEventCb(pCurrClient, RF_ClientEventSwitchClientEntered, NULL);
        }

        /* Change HWI handler while switching the phy */
        HwiP_setFunc(&hwiCpe0, hwiFxnCpe0ChangePhy, (uintptr_t)NULL);

        /* Next state: fsmActiveState */
        FsmNextState(&fsmActiveState);

        /* If patches are provided, enable RFE and MCE clocks */
        if ((pCurrClient->clientConfig.pRfMode->mcePatchFxn != 0) || (pCurrClient->clientConfig.pRfMode->rfePatchFxn != 0))
        {
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR)     = CMDR_DIR_CMD_2BYTE(RF_CMD0, RFC_PWR_PWMCLKEN_MDMRAM | RFC_PWR_PWMCLKEN_RFERAM);
        }

        /* Apply the new RF Core patch */
        applyRfCorePatch(RF_PHY_SWITCHING_MODE);

        /* Configure the SETUP command */
        RF_RadioSetup* pOpSetup         = pObj->clientConfig.pOpSetup;
        pOpSetup->prop.status           = IDLE;

        /* Need to write analog configuration when PHY is changed */
        if ((pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_RADIO_SETUP) ||
            (pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_BLE5_RADIO_SETUP))
        {
            /* BLE, BLE 5.0 modes share same offset */
            pOpSetup->common.config.analogCfgMode = RF_SETUP_ANALOGCFG_UPDATE;
        }
        else
        {
            /* PROPRIETARY mode */
            pOpSetup->prop.config.analogCfgMode = RF_SETUP_ANALOGCFG_UPDATE;
        }

        /* Request update */
        pObj->clientConfig.bUpdateSetup = true;

        /* Walk the chain and search or specific commands */
        RF_Op* tmp                      = (RF_Op*)&pOpSetup->prop;
        while ((tmp->pNextOp) && (tmp->pNextOp->commandNo != CMD_SYNC_START_RAT) &&
               (tmp->pNextOp->commandNo != CMD_FS) &&
               (tmp->pNextOp->commandNo != CMD_FS_OFF))
        {
            tmp = tmp->pNextOp;
        }

        /* Clear any of the found specific command */
        tmp->pNextOp                    = NULL;
        tmp->condition.rule             = COND_NEVER;

        /* Setup FS command to follow SETUP command */
        rfc_CMD_FS_t* pOpFs;
        RF_Op* pOpFirstPend = Q_peek(&cmdQ.pPend)->pOp;
        if ((pOpFirstPend->commandNo == CMD_FS) || (pOpFirstPend->commandNo == CMD_FS_OFF))
        {
            /* First command is FS command so no need to chain an implicit FS command -> reset nRtc2 */
            powerDurCalc.nRtc2 = 0;
        }
        else
        {
            if (pObj->state.mode_state.cmdFs.commandNo)
            {
                /* Chain in the implicit FS command */
                pOpFs                            = &pObj->state.mode_state.cmdFs;
                pOpFs->status                    = IDLE;
                pOpFs->pNextOp                   = NULL;
                pOpFs->startTrigger.triggerType  = TRIG_NOW;
                pOpFs->condition.rule            = COND_NEVER;
                tmp->pNextOp                     = (RF_Op*)pOpFs;
                tmp->condition.rule              = COND_ALWAYS;
            }
        }

        /* Trim directly the radio register values based on the ID of setup command */
        rfTrim_t rfTrim;
        RFCRfTrimRead((rfc_radioOp_t*)pObj->clientConfig.pOpSetup, (rfTrim_t*)&rfTrim);
        RFCRfTrimSet((rfTrim_t*)&rfTrim);

        /* Send the command chain */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = (uint32_t)&pObj->clientConfig.pOpSetup->commandId;

        /* We've handled this event now */
        e &= ~Fsm_EventChangePhy1;
    }
}

/*
 *  It decides whether the command should be dispatch now or not.
 *
 *  Input:  pObj - Pointer to RF object.
 *  Return: none
 */
static void RF_prepareRadio()
{
    /* If there is no running command */
    if(!cmdQ.pCurrCmd)
    {
        if(cmdQ.pPend)
        {
            if (bRadioIsActive)
            {
                /* Set RF power constraint */
                RF_setPowerConstraint(RF_PowerConstraintCmdQ);

                /* Kick the HWI to dispatch the next pending command */
                HwiP_post(INT_RFC_CPE_0);
            }
            else if (!bRadioIsBeingPoweredUp)
            {
                /* Enter critical section */
                uint32_t key = HwiP_disable();

                /* Calculate dispatch time */
                uint32_t dispatchTime = calculateDispatchTime(cmdQ.pPend->pOp);

                /* Decide whether the command should be dispatched */
                if (bRadioIsBeingPoweredDown)
                {
                    /* In the middle of power down, indicate that the radio will
                    need to be powered up again immediately */
                    bRadioRestartIsRequested = true;
                }
                else if (dispatchTime)
                {
                    /* Dispatch command in the future */
                    ClockP_setTimeout(&clkPowerUp, dispatchTime);
                    ClockP_start(&clkPowerUp);
                }
                else if (!bRadioPhyIsBeingSwitched)
                {
                    /* Initiate PHY change through the radio state machine */
                    SwiP_or(&swiFsm, Fsm_EventWakeup);
                }

                /* Exit critical section */
                HwiP_restore(key);
            }
        }
    }
    else
    {
        RF_ieeeExpansion(IEEE_REQUEST_FG_CMD);
    }
}

/*-------------- Initialization & helper functions ---------------*/

/*
 *  Initialize RF driver.
 *
 *  Input:  none
 *  Return: none
 */
static void rf_init()
{
    union {
        HwiP_Params hp;
        SwiP_Params sp;
    } params;

    /* Power init */
    Power_init();

    /* Enable output RTC clock for Radio Timer Synchronization */
    HWREG(AON_RTC_BASE + AON_RTC_O_CTL) |= AON_RTC_CTL_RTC_UPD_EN_M;

    /* Set the automatic bus request */
    HWREG(PRCM_BASE + PRCM_O_RFCBITS) = RF_BOOT0;

    /* Initialize SWI used for the CPE interrupts */
    SwiP_Params_init(&params.sp);
    params.sp.priority = RFCC26XX_hwAttrs.swiCpe0Priority;
    SwiP_construct(&swiFsm, swiFxnFsm, &params.sp);

    /* Initialize HWI used for the CPE interrupts */
    HwiP_Params_init(&params.hp);
    params.hp.enableInt = false;
    params.hp.priority = RFCC26XX_hwAttrs.hwiCpe0Priority;
    HwiP_construct(&hwiCpe0, INT_RFC_CPE_0, hwiFxnCpe0PowerFsm, &params.hp);

    /* Initialize SWI used for the HW interrupts */
    SwiP_Params_init(&params.sp);
    params.sp.priority = RFCC26XX_hwAttrs.swiHwPriority;
    SwiP_construct(&swiHw, swiFxnHw, &params.sp);

    /* Initialize HWI used for the HW interrupts */
    HwiP_Params_init(&params.hp);
    params.hp.enableInt = true;
    params.hp.priority = RFCC26XX_hwAttrs.hwiHwPriority;
    HwiP_construct(&hwiHw, INT_RFC_HW_COMB, hwiFxnHw, &params.hp);

    /* Initialize clock object used as power-up trigger */
    ClockP_construct(&clkPowerUp, &clkPowerUpFxn, 0, NULL);

    /* Set FSM state to power up */
    FsmNextState(fsmPowerUpState);

    /* Subscribe to wakeup notification from the Power driver */
    Power_registerNotify(&RF_wakeupNotifyObj,                   /* Object to register */
                         PowerCC26XX_AWAKE_STANDBY,             /* Event the notification to be invoked upon */
                         (Power_NotifyFxn) RF_wakeupNotifyFxn,  /* Function to be invoked */
                         (uintptr_t) NULL);                     /* Parameters */

    /* Initialize global variables */
    RF_CmdDirImm* pCmdDirImm       = &cmdDirImm;
    pCmdDirImm->availRatCh         = RF_DEFAULT_AVAILRATCH_VAL;
    pCmdDirImm->flags              = 0;
    bRadioHasBeenBootedBefore      = false;
    bRadioIsBeingPoweredUp         = false;
    bRadioIsBeingPoweredDown       = false;
    bRadioYieldingWasRequested     = false;
    bRadioYieldWasCancelled        = false;
    bRadioPhyIsBeingSwitched       = false;
    bRadioDelegateXOSCHFSwitching  = false;
    bDisableFlashInIdleConstraint  = false;
    powerDurCalc.nRtc1             = 0;
    powerDurCalc.errTolValInUs     = RF_DEFAULT_RAT_RTC_ERR_TOL_IN_US;
    RF_powerConstraint             = 0;
    RF_Sch.phySwitchingTimeInUs[0] = RF_DEFAULT_PHY_SWITCHING_TIME;
    RF_Sch.phySwitchingTimeInUs[1] = RF_DEFAULT_PHY_SWITCHING_TIME;
}

/*
 *  Allocate RAT channel.
 *
 *  Input:  pDirImmCmd     - Pointer to the immediate command to be used to configure
 *                           the RAT channels.
 *  Return: RAT channel    - If success.
 *          RF_ALLOC_ERROR - If failed.
 */
static int8_t ratChanAlloc(RF_CmdDirImm *pDirImmCmd)
{
    int8_t i;

    /* If RAT channels are available allocate one of the available RAT channels */
    if (pDirImmCmd->availRatCh != RF_DEFAULT_AVAILRATCH_VAL)
    {
        /* Walk the RAT channel IDs */
        for (i = 0; i < RF_RAT_CH_MAX_INDEX; i++)
        {
            /* If an available channel is found */
            if (pDirImmCmd->availRatCh & (1 << i))
            {
                /* Mark the channel as occupied */
                pDirImmCmd->availRatCh &= ~(1<<i);

                /* Enable the interrupt source */
                HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) |= (1<<(i+RFC_DBELL_RFHWIFG_RATCH0_BITN));

                /* Set RF power constraint in order to keep the RF core powered */
                RF_setPowerConstraint(RF_PowerConstraintRat);

                /* Return with the channel number */
                return i;
            }
        }
    }

    /* Return with an invalid channel ID in case of error*/
    return (int8_t)RF_ALLOC_ERROR;
}

/*
 *  Free RAT channel.
 *
 *  Input:  ratChannelNum  - The ID of RAT channel to be freed.
 *          checkPowerDown - Enabling to power down the radio.
 *  Return: none
 */
static void ratChanFree(int8_t ratChannelNum, bool checkPowerDown)
{
    /* Prepare an immediate command */
    RF_CmdDirImm* pCmd = &cmdDirImm;

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* If the provided RAT channel was in use */
    if (!((pCmd->flags & RF_RAT_RPT_CPT_FLAG) && (ratChannelNum == pCmd->rptCptRatChNum)))
    {
        /* Mark the channel as available again */
        pCmd->availRatCh |= (1 << ratChannelNum);

        /* Disable interrupt source */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) &= ~(1 << (ratChannelNum + RFC_DBELL_RFHWIFG_RATCH0_BITN));

        /* If there are no other RAT interrupt sources, release the cosntraint, the RF core can be powered down */
        if ((!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN)) && checkPowerDown)
        {
            RF_releasePowerConstraint(RF_PowerConstraintRat);
        }
    }

    /* Exit critical section */
    HwiP_restore(key);
}

/*
 *  Trace through the pending queue and flush the command(s).
 *
 *  Input:  h         - Handle to the client calling this function.
 *          pCmd      - Pointer to the command where the cancelling should start with.
 *          bFlushAll - Decides weather one or more commands should be aborted.
 *  Return: Number of commands was terminated.
 */
static uint32_t discardPendCmd(RF_Handle h, RF_Cmd *pCmd, bool bFlushAll)
{
    /* Local variable counts the number of command cancelled. */
    uint32_t nDiscarded = 0;

    /* If it is possible, start to flush from the beginning of pend queue. */
    if(pCmd == cmdQ.pPend)
    {
        /* If the head of the queue belongs to the client, remove the command. */
        while (isClientOwner(h, pCmd))
        {
            /* Mark the command as being cancelled. */
            pCmd->flags |= RF_CMD_CANCELLED_FLAG;

            /* Step the head of the queue. */
            Q_pop(&cmdQ.pPend);

            /* Push the command to the done queue. */
            Q_push(&cmdQ.pDone, pCmd, NULL);

            /* Take the next command as the head of the queue. */
            pCmd = cmdQ.pPend;

            /* Increment the counter. */
            nDiscarded += 1;

            /* Break the loop if only single cancel was requested. */
            if(!bFlushAll)
            {
                return nDiscarded;
            }
        }
    }
    else
    {
        /* Start the search from the beginning of the queue. */
        RF_Cmd* pTmp = cmdQ.pPend;

        /* Stop the search before we reach the command. */
        while(pTmp && pTmp->pNext && (pTmp->pNext != pCmd))
        {
            pTmp = pTmp->pNext;
        }

        /* We will flush from pCmd->Next. */
        pCmd = pTmp;
    }

    /* Cancel the commands from pCmd->pNext if it belongs to the client. */
    while(pCmd)
    {
        if (isClientOwner(h, pCmd->pNext))
        {
            /* Remove command from the pending queue. */
            RF_Cmd* pCmdCancel = (RF_Cmd*)pCmd->pNext;

            /* Mark the command as being cancelled. */
            pCmdCancel->flags |= RF_CMD_CANCELLED_FLAG;

            /* Delete reference to the cancelled command. */
            pCmd->pNext = pCmdCancel->pNext;

            /* Push the command to the done queue. */
            Q_push(&cmdQ.pDone, pCmdCancel, NULL);

            /* Increment the counter. */
            nDiscarded += 1;

            /* Break the loop if only single cancel was requested. */
            if(!bFlushAll)
            {
                return nDiscarded;
            }
        }
        else
        {
            /* Step the queue if it belong to the other client. */
            pCmd = pCmd->pNext;
        }
    }

    /* Return with the number of cancelled commands. */
    return nDiscarded;
}

/*
 *  Process cancel commands. It is used by RF_cancelCmd, RF_flushCmd API.
 *
 *  Input:  h    - Handle to the client calling this function.
 *          ch   - Handle to the command where the cancelling should start with.
 *          type - Flags decide how should we terminate the command.
 *  Return: status
 */
static RF_Stat abortCmd(RF_Handle h, RF_CmdHandle ch, uint8_t type)
{
  /* Assert */
  DebugP_assert(h != NULL);

  /* Initialize local variables */
  RF_Cmd* pCmd   = NULL;
  bool bGraceful = type & RF_ABORT_CANCEL_SINGLE_CMD;
  bool bFlush    = type & RF_ABORT_CANCEL_MULTIPLE_CMD;
  RF_Stat status = RF_StatInvalidParamsError;

  /* Enter critical section */
  uint32_t key = HwiP_disable();

  /* Handle FLUSH_ALL request */
  if (ch == RF_CMDHANDLE_FLUSH_ALL)
  {
      /* If the current command belongs to this client, cancel it first.
         Otherwise, walk the pend queue and cancel commands belong to this client. */
      pCmd = (isClientOwner(h, cmdQ.pCurrCmd) ? cmdQ.pCurrCmd : cmdQ.pPend);
  }
  else
  {
      /* Search for the command in the command pool based on its handle. The command can
         locate on any of the queues at this point. */
      pCmd = getCmd(h, ch, 0x00);
  }

  /* If command handle is valid, proceed to cancel. */
  if(pCmd)
  {
      /* If command is still in the pool but it is not allocated anymore, i.e. it was already served. */
      if(!(pCmd->flags & RF_CMD_ALLOC_FLAG))
      {
          /* Return with a special status byte to the user. */
          status = RF_StatCmdEnded;
      }
      else
      {
          /* If the command we want to cancel is actively running. */
          if (pCmd == cmdQ.pCurrCmd)
          {
              /* Flag that the command has been aborted. */
              pCmd->flags |= (1 << bGraceful);

              /* Send the abort/stop command through the doorbell to the RF core. */
              RFCDoorbellSendTo((bGraceful) ? CMDR_DIR_CMD(CMD_STOP) : CMDR_DIR_CMD(CMD_ABORT));

              /* Remove all commands from the pend queue belong to this client. Only do it
                 if it was explicitely requested through the RF_ABORT_CANCEL_MULTIPLE_CMD flag. */
              if (bFlush)
              {
                  discardPendCmd(h, cmdQ.pPend, bFlush);
              }

              /* Return with success as we cancelled at least the currently running command. */
              status = RF_StatSuccess;
          }
          else
          {
              /* Remove one/all commands from the pend queue belong to this client based on
                 the RF_ABORT_CANCEL_MULTIPLE_CMD argument. If at least one command is cancelled
                 the operation was succesful. Otherwise, either the pend queue is empty or pCmd
                 have terminated earlier or no command in the pend queue (belongs to this client).*/
              if(discardPendCmd(h, pCmd, bFlush))
              {
                  /* Stop the scheduling clock. We will restart the clock according to the first
                     entry in the new queue. */
                  ClockP_stop(&clkPowerUp);

                  /* Kick the state machine to handle the done queue. This is not necessary
                    when the RF is currently performing a power-up. */
                  if(bRadioIsBeingPoweredUp == false)
                  {
                    SwiP_or(&swiFsm, Fsm_EventLastCommandDone);
                  }

                  /* At least one command was cancelled. */
                  status = RF_StatSuccess;
              }
              else
              {
                  /* The command is not running and is not in the pend queue. It is located on the
                     done queue, hence return RF_StatCmdEnded. */
                  status = RF_StatCmdEnded;
              }
          }
      }
  }

  /* Exit critical section */
  HwiP_restore(key);

  /* Return with the result:
     - RF_StatSuccess if at least one command was cancelled.
     - RF_StatCmdEnded, when the command has already finished.
     - RF_StatInvalidParamsError otherwise.  */
  return status;
}

/*
 *  Send a direct or immediate command to the RF Core.
 *
 *  Input:  h           - Handle to the client calling this function.
 *          pCmdStruct  - Pointer to the command which shall be sent to the RF core.
 *          bStartRadio - NA.
 *  Return: RF_StatCmdDoneSuccess       - If the command was sent and accepted by the RF core.
 *          RF_StatCmdDoneError         - Command was rejected by the RF core.
 *          RF_StatInvalidParamsError   - Client do not have the right to send commands now.
 *          RF_StatRadioInactiveError   - The RF core is OFF.
 */
static RF_Stat postDirImmCmd(RF_Handle h, uint32_t pCmdStruct, bool bStartRadio)
{
    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Only the current client is allowed to send direct commands */
    if (h != pCurrClient)
    {
        /* Return with an error code it is a different client */
        return RF_StatInvalidParamsError;
    }

    /* If the RF core is ON, we can send the command */
    if (bRadioIsActive == true)
    {
        /* Submit the command to the doorbell */
        uint32_t cmdstaVal = RFCDoorbellSendTo(pCmdStruct);

        /* Exit critical section */
        HwiP_restore(key);

        /* Check the return value of the RF core through the CMDSTA register within the doorbell */
        if ((cmdstaVal & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
        {
            /* The command was accepted */
            return RF_StatCmdDoneSuccess;
        }
        else
        {
            /* The command was rejected */
            return RF_StatCmdDoneError;
        }
    }
    else
    {
        /* Exit critical section */
        HwiP_restore(key);

        /* The RF core is not capable of receiving the command */
        return RF_StatRadioInactiveError;
    }
}

/*-------------- API functions ---------------*/
/*
 * ======== RF_open ========
 * Open an RF handle
 */
RF_Handle RF_open(RF_Object *pObj, RF_Mode *pRfMode, RF_RadioSetup *pOpSetup, RF_Params *params)
{
    /* Assert */
    DebugP_assert(pObj != NULL);

    /* Read available RF modes from the PRCM register */
    uint32_t availableRfModes = HWREG(PRCM_BASE + PRCM_O_RFCMODEHWOPT);

    /* Verify that the provided configuration is supported by this device.
       Reject any request which is not compliant. */
    if (pRfMode && pOpSetup && (availableRfModes & (1 << pRfMode->rfMode)))
    {
        /* Register the (valid) RF mode to the client */
        pObj->clientConfig.pRfMode = pRfMode;

        /* Trim the override list; The implementation of RFCOverrideUpdate is device specific */
        RFCOverrideUpdate((RF_Op*)pOpSetup, NULL);

        /* Register the setup command to the client */
        pObj->clientConfig.pOpSetup = pOpSetup;
    }
    else
    {
        /* Return with null if the device do not support the requested configuration */
        return NULL;
    }

    /* Exit critical section */
    uint32_t key = HwiP_disable();

    /* Check whether RF driver is accepting more clients */
    if (nClients < N_MAX_CLIENTS)
    {
        /* Initialize shared objects on first client opening */
        if (nClients == 0) rf_init();

        /* Save the new RF_Handle */
        RF_Sch.clientHnd[nClients++] = pObj;

        /* Exit critical section */
        HwiP_restore(key);

        /* Populate default RF parameters if not provided */
        RF_Params rfParams;
        if (params == NULL)
        {
            RF_Params_init(&rfParams);
            params = &rfParams;
        }

        /* Initialize RF_Object configuration */
        pObj->clientConfig.nInactivityTimeout     = params->nInactivityTimeout;
        pObj->clientConfig.nPowerUpDuration       = params->nPowerUpDuration;
        pObj->clientConfig.bPowerUpXOSC           = params->bPowerUpXOSC;
        pObj->clientConfig.nClientEventMask       = params->nClientEventMask;
        pObj->clientConfig.bUpdateSetup           = true;
        pObj->clientConfig.nPowerUpDurationMargin = params->nPowerUpDurationMargin;

        /* Set all the callbacks to the default (do nothing) callback */
        pObj->clientConfig.pErrCb                 = (void*) defaultCallback;
        pObj->clientConfig.pClientEventCb         = (void*) defaultCallback;
        pObj->clientConfig.pPowerCb               = (void*) defaultCallback;

        /* If a user specified callback is provided, overwrite the default */
        if (params->pErrCb)
        {
            pObj->clientConfig.pErrCb = (void *)params->pErrCb;
        }
        if (params->pClientEventCb)
        {
            pObj->clientConfig.pClientEventCb = (void *)params->pClientEventCb;
        }
        if (params->pPowerCb)
        {
            pObj->clientConfig.pPowerCb = (void *)params->pPowerCb;
        }

        /* Initialize client state & variables to zero */
        memset((void*)&pObj->state, 0, sizeof(pObj->state));

        /* Initialize client specific semaphore object */
        SemaphoreP_constructBinary(&pObj->state.semSync, 0);

        /* Initialize client specific clock objects */
        ClockP_Params cp;
        ClockP_Params_init(&cp);
        cp.arg = (uintptr_t)pObj;
        ClockP_construct(&pObj->state.clkInactivity,    /* Clock object */
                         (ClockP_Fxn)clkInactivityFxn,  /* Clock function */
                         0,                             /* Clock timeout */
                         &cp);                          /* Clock parameters */

        ClockP_construct(&pObj->state.clkReqAccess,     /* Clock object */
                         (ClockP_Fxn)clkReqAccessFxn,   /* Clock function */
                         0,                             /* Clock timeout */
                         &cp);                          /* Clock parameters */

        /* Return with and RF handle (RF_Obj*) */
        return pObj;
    }
    else
    {
        /* Exit critical section */
        HwiP_restore(key);

        /* Return with null if no more clients are accepted */
        return NULL;
    }
}

/*
 * ======== RF_close ========
 * Close an RF handle
 */
void RF_close(RF_Handle h)
{
    /* Assert */
    DebugP_assert(h != NULL);

    /* If there is at least one active client */
    if(nClients)
    {
        /* Wait for all issued commands to finish before freeing the resources */
        if (cmdQ.nSeqPost != cmdQ.nSeqDone)
        {
            /* Trace the end of pending queue for client h */
            RF_Cmd *pCmd = Q_end(h, &cmdQ.pPend);

            /* If the queue is empty */
            if (!pCmd)
            {
                /* If the currently running command belongs to the client */
                if (isClientOwner(h, cmdQ.pCurrCmd))
                {
                    /* no pending commands for client h
                       -> check if the current command is the last command */
                    pCmd = cmdQ.pCurrCmd;
                }
            }

            /* Pend until the running command terminates */
            if (pCmd)
            {
                RF_pendCmd(h, pCmd->ch, (RF_EventCmdAborted | RF_EventCmdStopped | RF_EventCmdCancelled));
            }
        }

        /* Enter critical section */
        uint32_t key = HwiP_disable();

        /* Clear the RF_sch client handle */
        if (h == RF_Sch.clientHnd[0])
        {
            RF_Sch.clientHnd[0] = NULL;
        }
        else
        {
            RF_Sch.clientHnd[1] = NULL;
        }

        /* Check whether this is the last client */
        if (--nClients == 0)
        {
            /* If this is the last client, set it to be the active client */
            pCurrClient = h;

            if (bRadioIsActive == true)
            {
                /* Release the constraint on the RF resources */
                RF_releasePowerConstraint(RF_PowerConstraintCmdQ);
                RF_releasePowerConstraint(RF_PowerConstraintRat);

                /* Exit critical section */
                HwiP_restore(key);

                /* Wait until the radio is powered down (outside critical section) */
                SemaphoreP_pend(&h->state.semSync, SemaphoreP_WAIT_FOREVER);

                /* Enter critical section */
                key = HwiP_disable();
            }

            /* Unregister shared RTOS objects initalized during rf_init by the first client */
            SwiP_destruct(&swiFsm);
            HwiP_destruct(&hwiCpe0);
            SwiP_destruct(&swiHw);
            HwiP_destruct(&hwiHw);
            ClockP_destruct(&clkPowerUp);

            /* Unregister the wakeup notify callback */
            Power_unregisterNotify(&RF_wakeupNotifyObj);

            /* Release XOSC_HF dependency if it was set by this client */
            if (h->clientConfig.bPowerUpXOSC == true)
            {
                Power_releaseDependency(PowerCC26XX_XOSC_HF);
            }
        }

        /* If we're the current RF client, stop being it */
        if (pCurrClient == h)
        {
            pCurrClient = NULL;
        }

        /* Exit critical section */
        HwiP_restore(key);

        /* Unregister client specific RTOS objects (these are not shared between clients) */
        SemaphoreP_destruct(&h->state.semSync);
        ClockP_destruct(&h->state.clkInactivity);
        ClockP_destruct(&h->state.clkReqAccess);
    }
}

/*
 * ======== RF_getCurrentTime ========
 * Get current time in RAT ticks
 */
uint32_t RF_getCurrentTime(void)
{
    /* Local variable */
    uint64_t nCurrentTime = 0;

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* If radio is active, read the RAT */
    if (bRadioIsActive)
    {
        /* Read the RAT timer through register access */
        nCurrentTime = HWREG(RFC_RAT_BASE + RFC_RAT_O_RATCNT);

        /* Exit critical section */
        HwiP_restore(key);
    }
    else
    {
        /* Exit critical section */
        HwiP_restore(key);

        /* The radio is inactive, read the RTC instead */
        nCurrentTime  = AONRTCCurrent64BitValueGet();

        /* Conservatively assume that we are just about to increment the RTC
           Scale with the 4 MHz that the RAT is running
           Add the RAT offset for RTC==0 */
        nCurrentTime += RF_RTC_TICK_INC;
        nCurrentTime *= RF_SCALE_RTC_TO_4MHZ;
        nCurrentTime += ((uint64_t)opRatSync.start.rat0) << RF_SHIFT_32_BITS;
        nCurrentTime >>= RF_SHIFT_32_BITS;
    }

    /* Return with the current value */
    return (uint32_t) nCurrentTime;
}

/*
 * ======== RF_postCmd ========
 * Post radio command
 */
RF_CmdHandle RF_postCmd(RF_Handle h, RF_Op* pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent)
{
    /* Assert */
    DebugP_assert(h   != NULL);
    DebugP_assert(pOp != NULL);

    /* Local pointer to a radio commands */
    bool    bHeadPos;
    RF_Cmd* pCmd;

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Try to allocate container */
    pCmd = allocCmd();

    /* If allocation failed */
    if (pCmd == NULL)
    {
        /* Exit critical section */
        HwiP_restore(key);

        /* No more RF command containers available -> set status and return error code */
        return (RF_CmdHandle)RF_ALLOC_ERROR;
    }

    /* Stop inactivity clock if running */
    ClockP_stop(&h->state.clkInactivity);

    /* Increment the sequence number and mask the value */
    cmdQ.nSeqPost = (cmdQ.nSeqPost + 1) & N_CMD_MODMASK;

    /* Populate container with reset values */
    pCmd->pOp     = pOp;
    pCmd->ePri    = ePri;
    pCmd->pCb     = pCb;
    pCmd->ch      = cmdQ.nSeqPost;
    pCmd->pClient = h;
    pCmd->bmEvent = bmEvent & ~RF_INTERNAL_IFG_MASK;
    pCmd->flags   = 0;
    pCmd->pastifg = 0;

    /* Submit to pending command queue */
    if (Q_push(&cmdQ.pPend, pCmd, &bHeadPos))
    {
        /* Store the sequence number for the client */
        h->state.chLastPosted = pCmd->ch;

        /* Cancel ongoing yielding */
        h->state.bYielded = false;

        /* Mark the command that it is allocated */
        pCmd->flags |= RF_CMD_ALLOC_FLAG;

        /* If this is the first entry in the queue*/
        if(bHeadPos == true)
        {
            /* Cancel any previous request of powering down the radio */
            if (bRadioYieldingWasRequested && bRadioIsActive)
            {
                bRadioYieldWasCancelled    = true;
                bRadioYieldingWasRequested = false;
            }
        }

        /* Exit critical section */
        HwiP_restore(key);

        /* Trigger dispatcher HWI if there is no running command */
        if (bHeadPos)
        {
            RF_prepareRadio();
        }

        /* Return with the command handle as success */
        return pCmd->ch;
    }
    else
    {
        /* Exit critical section */
        HwiP_restore(key);

        /* Return with an error code */
        return (RF_CmdHandle)RF_ALLOC_ERROR;
    }
}

/*
 *  ==================== RF_scheduleCmd ============================
 *  Process request to schedule new command from a particular client
 */
RF_CmdHandle RF_scheduleCmd(RF_Handle h, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams, RF_Callback pCb, RF_EventMask bmEvent)
{
    /* Local variable declaration */
    bool        bHeadPos;
    RF_Cmd*     pCmd;
    RF_Handle   h2;

    /* Assert */
    DebugP_assert(h   != NULL);
    DebugP_assert(pOp != NULL);

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Assign h2 to client not requesting the command
       h is the client requesting the new command */
    if (h == RF_Sch.clientHnd[0])
    {
        h2 = RF_Sch.clientHnd[1];
    }
    else
    {
        h2 = RF_Sch.clientHnd[0];
    }

    /* scheduler results for new command
         1. new commmand is rejected -> return RF_ALLOC_ERROR
         2. new command is added to the queue (preemption or append)
         3. new command is inserted in the pending queue, but not at the top
         4. new command is inserted at the top of the pending queue

       After all the cases, if the new command is added/inserted to top of the
       pending queue, post new command if there is no current running command.
    */
    RF_ScheduleCmdStatus schStatus;
    pCmd = RF_schCmdRunInsertPreempt(h, h2, pOp, pSchParams, &schStatus);

    /* Case 1. The command was rejected */
    if (!pCmd)
    {
        /* Store the reason and the handle why the callback is being invoked */
        RF_Sch.issueRadioFreeCbFlags |= RF_RADIOFREECB_CMDREJECT_FLAG;
        RF_Sch.clientHndRadioFreeCb   = h;

        /* Exit critical section*/
        HwiP_restore(key);

        /* Return with the status code */
        if (schStatus == RF_ScheduleCmdSchError)
        {
            return RF_SCHEDULE_CMD_ERROR;
        }
        else
        {
            return RF_ALLOC_ERROR;
        }
    }

    /* Case 2. New command is added to the queue: preemption or append  */
    /* Stop inactivity clock if running */
    ClockP_stop(&h->state.clkInactivity);

    /* Populate RF_Op container */
    cmdQ.nSeqPost = pCmd->ch;

    pCmd->pOp     = pOp;
    pCmd->ePri    = pSchParams->priority;
    pCmd->pCb     = pCb;
    pCmd->pClient = h;
    pCmd->bmEvent = bmEvent & ~RF_INTERNAL_IFG_MASK;
    pCmd->flags   = 0;
    pCmd->pastifg = 0;

    if(pSchParams->bIeeeBgCmd == true)
    {
        /* Mark the command as a background command */
        pCmd->flags |= RF_CMD_BG_CMD_FLAG;
    }

    /* Exit critical section */
    HwiP_restore(key);

    /* Send the RF_flushCmd if the new command preempts others */
    if (RF_Sch.cmdInsertFlags == RF_SCH_CMD_PREEMPT)
    {
        RF_flushCmd(RF_Sch.preemptCmd.client,
                    RF_Sch.preemptCmd.cancelStartCmdHandle,
                    RF_Sch.preemptCmd.cancelType);
    }

    /* Enter critical section */
    key = HwiP_disable();

    /* Case 3. New command was inserted to the middle of pending queue */
    if (RF_Sch.cmdInsertFlags == RF_SCH_CMD_INSERT_QUEUE_LATER)
    {
        /* Mark the command as being allocated */
        pCmd->flags |= RF_CMD_ALLOC_FLAG;

        /* Disable previous yielding */
        h->state.bYielded = false;

        /* Exit critical section */
        HwiP_restore(key);

        /* Return with the command handle */
        return pCmd->ch;
    }

    /* Case 4. Inserted command to the top of the pend queue */
    if (RF_Sch.cmdInsertFlags == RF_SCH_CMD_INSERT_QUEUE_TOP)
    {
        /* Stop the scheduling clock */
        ClockP_stop(&clkPowerUp);

        /* Mark the command as being allocated */
        pCmd->flags |= RF_CMD_ALLOC_FLAG;

        /* Disable previous yielding */
        h->state.bYielded = false;

        /* if there is already current command running -> return */
        if (cmdQ.pCurrCmd)
        {
            HwiP_restore(key);
            return pCmd->ch;
        }
        else
        {
            HwiP_restore(key);
        }
    }

    /* Push the new command to the pend queue */
    if ((RF_Sch.cmdInsertFlags == 0) || (RF_Sch.cmdInsertFlags == RF_SCH_CMD_PREEMPT))
    {
        /* Submit to pending command queue */
        if (Q_push(&cmdQ.pPend, pCmd, &bHeadPos) == false)
        {
            /* Exit critical section */
            HwiP_restore(key);

            /* Return with error code */
            return (RF_CmdHandle)RF_ALLOC_ERROR;
        }

        /* Mark the last command of this client */
        h->state.chLastPosted = pCmd->ch;

        /* Mark the command as being allocated */
        pCmd->flags |= RF_CMD_ALLOC_FLAG;

        /* Disable previous yielding */
        h->state.bYielded = false;

        /* This is not the first item in the queue */
        if (bHeadPos == false)
        {
            /* Exit critical section */
            HwiP_restore(key);

            /* Return with the command handle */
            return pCmd->ch;
        }

        /* Cancel any ongoing power down sequence*/
        if (bRadioYieldingWasRequested && bRadioIsActive)
        {
            bRadioYieldWasCancelled    = true;
            bRadioYieldingWasRequested = false;
        }

        /* Exit critical section */
        HwiP_restore(key);
    }

    /* Trigger dispatcher HWI if there is no running command */
    RF_prepareRadio();

    /* Return with the command handle in case of success */
    return pCmd->ch;
}

/*
 * ======== RF_pendCmd ========
 * Pend on radio command
 */
RF_EventMask RF_pendCmd(RF_Handle h, RF_CmdHandle ch, RF_EventMask bmEvent)
{
    /* Assert */
    DebugP_assert(h  != NULL);

    /* If the command handle is invalid (i.e. RF_ALLOC_ERROR) */
    if(ch < 0)
    {
      /* Return with zero means the command was rejected earlier */
      return 0;
    }

    /* Enter critical section */
    uint32_t key = SwiP_disable();

    /* Find the command based on its handle in the command pool */
    RF_Cmd* pCmd = getCmd(h, ch, RF_CMD_ALLOC_FLAG);

    /* If the command was already disposed */
    if (!pCmd || !(pCmd->flags & RF_CMD_ALLOC_FLAG))
    {
        /* Exit critical section */
        SwiP_restore(key);

        /* Return with last command done event */
        return RF_EventLastCmdDone;
    }

    /* If the command still exist, but was cancelled */
    if (pCmd->flags & (RF_CMD_ABORTED_FLAG | RF_CMD_STOPPED_FLAG | RF_CMD_CANCELLED_FLAG | RF_CMD_PREEMPT_FLAG))
    {
        /* Exit critical section */
        SwiP_restore(key);

        /* Wait until the command is being disposed by the radioOpDone method */
        while(pCmd->flags & (RF_CMD_ABORTED_FLAG | RF_CMD_STOPPED_FLAG | RF_CMD_CANCELLED_FLAG | RF_CMD_PREEMPT_FLAG));

        /* Return with last command done */
        return RF_EventLastCmdDone;
    }

    /* Expand the pend mask to accept RF_EventLastCmdDone and RF_EventLastFGCmdDone events even if it is not given explicitely */
    bmEvent = (bmEvent | RF_EventLastCmdDone | RF_EventLastFGCmdDone);

    /* If the command is being executed, but the event we pending on has already happend (i.e. in a chain),
       return the past events */
    if(pCmd->pastifg & bmEvent)
    {
        /* Exit critical section */
        SwiP_restore(key);

        /* Store the cause of returning */
        h->state.unpendCause = pCmd->pastifg & bmEvent;

        /* Clear the handled past events so it is possible to pend again */
        pCmd->pastifg &= ~h->state.unpendCause;

        /* Return with the events */
        return h->state.unpendCause;
    }

    /* Command has still not finished, override user callback with one that calls the user callback then posts to semaphore */
    if (pCmd->pCb != syncCb)
    {
        /* Temporarily store the callback function */
        h->state.pCbSync = (void*)pCmd->pCb;

        /* Exhange the callback function: this will invoke the user callback and post to the semaphore if needed */
        pCmd->pCb = syncCb;
    }

    /* Store the event subscriptions in the clients context. This can only be one of the already enabled
       interrupt sources by RF_postCmd (including RF_EventLastCmdDone) */
    h->state.eventSync = bmEvent;

    /* Exit critical section */
    SwiP_restore(key);

    /* Wait for semaphore */
    SemaphoreP_pend(&h->state.semSync, SemaphoreP_WAIT_FOREVER);

    /* Return the events that resulted in releasing the RF_pend() call */
    return h->state.unpendCause;
}

/*
 *  ======== RF_runCmd ========
 *  Run to completion a posted command
 */
RF_EventMask RF_runCmd(RF_Handle h, RF_Op* pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent)
{
    /* If no user callback was provided, use the one which posts to the semaphore */
    if (pCb == NULL)
    {
        pCb = syncCb;
    }

    /* Post the requested command */
    RF_CmdHandle ch = RF_postCmd(h, pOp, ePri, pCb, bmEvent);

    /* If the command was accepted, pend until one of the special events occur */
    return RF_pendCmd(h, ch, (RF_EventCmdAborted | RF_EventCmdStopped | RF_EventCmdCancelled));
}

/*
 *  ======== RF_runScheduleCmd ========
 *  Run to completion a scheduled command
 */
RF_EventMask RF_runScheduleCmd(RF_Handle h, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams, RF_Callback pCb, RF_EventMask bmEvent)
{
    /* If no user callback was provided, use the one which posts to the semaphore */
    if (pCb == NULL)
    {
        pCb = syncCb;
    }

    /* Post the requested command */
    RF_CmdHandle ch = RF_scheduleCmd(h, pOp, pSchParams, pCb, bmEvent);

    /* If the command was accepted, pend until one of the special events occur */
    return RF_pendCmd(h, ch, (RF_EventCmdAborted | RF_EventCmdStopped | RF_EventCmdCancelled));
}

/*
 *  ======== RF_yieldCmd ========
 *  Release client access
 */
void RF_yield(RF_Handle h)
{
    /* Assert */
    DebugP_assert(h != NULL);

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Set bRatSyncWasRequested to indicate rat/stc sync at every power down, equivalent to
       setting powerDurCalc.errTolValInUs = 0 */
    bRatSyncWasRequested = true;

    /* Stop ongoing request access and issue callback if the radio is off */
    ClockP_stop((&h->state.clkReqAccess));

    /* If all commands are done */
    if(cmdQ.nSeqDone == cmdQ.nSeqPost)
    {
        if (!bRadioIsActive && RF_Sch.issueRadioFreeCbFlags)
        {
            /* Exit critical section. */
            HwiP_restore(key);

            /* Invoke the radio free callback provided by the user. */
            RF_issueRadioFreeCb(RF_RADIOFREECB_REQACCESS_FLAG |
                                RF_RADIOFREECB_PREEMPT_FLAG   |
                                RF_RADIOFREECB_CMDREJECT_FLAG);

            /* Enter critical section. */
            key = HwiP_disable();
        }
    }

    /* If the radioFreeCb did not post new commands. */
    if(cmdQ.nSeqDone == cmdQ.nSeqPost)
    {
        /* All commands are done. Stop inactivity timer. */
        ClockP_stop(&h->state.clkInactivity);

        /* Potentially power down the RF core. */
        RF_releasePowerConstraint(RF_PowerConstraintCmdQ);

        /* Indicate that the client requested yielding. */
        if (bRadioIsActive == true)
        {
            bRadioYieldingWasRequested = true;
        }
    }
    else
    {
        /* There are still client commands that haven't finished.
           set flag to indicate immediate powerdown when last command is done */
        h->state.bYielded = true;
    }

    /* Exit critical section */
    HwiP_restore(key);
}

/*
 *  ======== RF_cancelCmd ========
 *  Cancel single radio command
 */
RF_Stat RF_cancelCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mode)
{
    /* Invoke the aborting process with the input arguments on a single command */
    return abortCmd(h, ch, mode);
}

/*
 *  ======== RF_flushCmd ========
 *  Cancel multiple radio commands from a client
 */
RF_Stat RF_flushCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mode)
{
    /* Abort multiple radio commands implicitly */
    return (abortCmd(h, ch, (mode | RF_ABORT_CANCEL_MULTIPLE_CMD)));
}

/*
 *  ======== RF_Params_init ========
 *  Initialize the RF_params to default value
 */
void RF_Params_init(RF_Params *params)
{
    /* Assert */
    DebugP_assert(params != NULL);

    /* Assign default values for RF_params */
    *params = RF_defaultParams;
}

/*
 *  ======== RF_runImmediateCmd ========
 *  Run immediate command
 */
RF_Stat RF_runImmediateCmd(RF_Handle h, uint32_t* pCmdStruct)
{
    /* Assert */
    DebugP_assert(h != NULL);

    /* Submit the command to the RF core */
    return postDirImmCmd(h, (uint32_t)pCmdStruct, false);
}

/*
 *  ======== RF_runDirectCmd ========
 *  Run direct command
 */
RF_Stat RF_runDirectCmd(RF_Handle h, uint32_t cmd)
{
    /* Assert */
    DebugP_assert(h != NULL);

    /* Submit the command to the RF core */
    return postDirImmCmd(h, cmd, false);
}

/*
 *  ======== RF_getRssi ========
 *  Get RSSI value
 */
int8_t RF_getRssi(RF_Handle h)
{
    /* return error if this is not the current client or radio is off */
    if ((h!= pCurrClient) || (!bRadioIsActive))
    {
        return RF_GET_RSSI_ERROR_VAL;
    }

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Request the RSSI value from the RF core */
    uint32_t cmdsta = RFCDoorbellSendTo(CMDR_DIR_CMD(CMD_GET_RSSI));

    /* Exit critical section */
    HwiP_restore(key);

    /* Check CMDSTA value to return valid RSSI value or return an error code */
    if ((cmdsta & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
    {
        return (int8_t)((cmdsta >> RF_SHIFT_16_BITS) & RF_CMDSTA_REG_VAL_MASK);
    }
    else
    {
        return (int8_t)RF_GET_RSSI_ERROR_VAL;
    }
}

/*
 *  ======== RF_getInfo ========
 *  Get RF driver info
 */
RF_Stat RF_getInfo(RF_Handle h, RF_InfoType type, RF_InfoVal *pValue)
{
    /* Local variables */
    int8_t kk = 0;
    RF_ScheduleMapElement *pScheduleMap;

    /* Prepare the default status value */
    RF_Stat status = RF_StatSuccess;

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Serve the different flavor of requests */
    switch(type)
    {
        case RF_GET_CURR_CMD:
            /* Get the handle of the currently running command. It can be conerted
               to a pointer through the RF_getCmdOp() API. */
            if (cmdQ.pCurrCmd)
            {
                pValue->ch = cmdQ.pCurrCmd->ch;
            }
            else
            {
                status = RF_StatError;
            }
            break;

        case RF_GET_AVAIL_RAT_CH:
            /* Get available RAT channels through the direct command if available.
               These channels can be allocated and used by the application. */
            if (cmdDirImm.availRatCh != RF_DEFAULT_AVAILRATCH_VAL)
            {
                pValue->availRatCh = cmdDirImm.availRatCh;
            }
            else
            {
                status = RF_StatError;
            }
            break;

        case RF_GET_RADIO_STATE:
            /* Get current radio state */
            pValue->bRadioState = bRadioIsActive;
            break;

        case RF_GET_SCHEDULE_MAP:
            /* Get scheduler timing map. This can be used to determine the recent
               time slots which are occupied by posted commands; and can help to
               find out if a command can be inserted to the queue or not. In dual-mode
               applications, this can help to sync the two protocol. */
            pScheduleMap = (RF_ScheduleMapElement *)pValue->pScheduleMap;
            memset(pScheduleMap, 0, sizeof(RF_ScheduleMapElement) * RF_NUM_SCHEDULE_MAP_ENTRIES);

            for (kk = 0; kk < RF_NUM_SCHEDULE_ACCESS_ENTRIES; kk++)
            {
                /* Copy access request info to schedule map */
                pScheduleMap[kk].pClient   = RF_Sch.clientHnd[kk];
                pScheduleMap[kk].priority  = RF_Sch.accReq[kk].priority;
                uint32_t startTime = RF_Sch.accReq[kk].startTime;
                pScheduleMap[kk].startTime = startTime;
                pScheduleMap[kk].endTime = startTime + RF_Sch.accReq[kk].duration;
            }

            /* Check if there is current command running */
            if (cmdQ.pCurrCmd)
            {
                /* Copy current command info to schedule map */
                pScheduleMap[kk].pClient   = cmdQ.pCurrCmd->pClient;
                pScheduleMap[kk].priority  = cmdQ.pCurrCmd->ePri;
                pScheduleMap[kk].startTime = cmdQ.pCurrCmd->startTime;
                pScheduleMap[kk].endTime   = cmdQ.pCurrCmd->endTime;
                kk++;
            }

            /* Check pending commands */
            RF_Cmd* pCmd;
            pCmd = cmdQ.pPend;
            /* Loop until end of command queue or number of entries exceed */
            while (pCmd)
            {
                if (kk < RF_NUM_SCHEDULE_MAP_ENTRIES)
                {
                    /* Copy pending command info to schedule map */
                    pScheduleMap[kk].pClient   = pCmd->pClient;
                    pScheduleMap[kk].priority  = pCmd->ePri;
                    pScheduleMap[kk].startTime = pCmd->startTime;
                    pScheduleMap[kk].endTime   = pCmd->endTime;
                    kk++;
                }
                else
                {
                    /* Number of entries exceeded, get out of loop */
                    break;
                }
                pCmd = pCmd->pNext;
            }
            break;

        default:
            status = RF_StatInvalidParamsError;
            break;
    }

    /* Exit critical section */
    HwiP_restore(key);

    /* Return with a status code */
    return status;
}

/*
 *  ======== RF_getCmdOp ========
 *  Get RF command
 */
RF_Op* RF_getCmdOp(RF_Handle h, RF_CmdHandle ch)
{
    /* Find the command in the command pool based on its handle */
    RF_Cmd* pCmd = getCmd(h, ch, RF_CMD_ALLOC_FLAG);

    /* If the command is found */
    if (pCmd)
    {
        /* Return with the first operation in the command */
        return pCmd->pOp;
    }
    else
    {
        /* Return with null in case of error */
        return NULL;
    }
}

/*
 *  ======== RF_ratCompare ========
 *  Set RAT compare
 */
int8_t RF_ratCompare(RF_Handle h, rfc_CMD_SET_RAT_CMP_t* pCmdStruct,
                            uint32_t compareTime, RF_Callback pRatCb)
{
    /* Unless the RF core is powered, the RAT cannot be accessed */
    if ((!bRadioIsActive) && (!bRadioPhyIsBeingSwitched))
    {
        /* Return with an error code */
        return RF_ALLOC_ERROR;
    }

    /* Prepare a direct command */
    RF_CmdDirImm* pDirImmCmd = &cmdDirImm;

    /* Configure the given command to RAT compare */
    pCmdStruct->commandNo   = CMD_SET_RAT_CMP;
    pCmdStruct->compareTime = compareTime;

    /* Find and allocate a RAT channel (if any is available) */
    int8_t ratCh = ratChanAlloc(pDirImmCmd);

    /* If we could allocate a RAT channel */
    if (ratCh >= RF_RAT_CH_LOWEST && ratCh <= RF_RAT_CH_HIGHEST)
    {
        /* Configure the given command to use this channel */
        pCmdStruct->ratCh = ratCh;

        /* Enter critical section */
        uint32_t key = HwiP_disable();

        /* Prepare the status value */
        RF_Stat status = RF_StatCmdDoneError;

        /* Submit the command to the doorbell */
        uint32_t cmdstaVal = RFCDoorbellSendTo((uint32_t)pCmdStruct);

        /* Exit critical section */
        HwiP_restore(key);

        /* Verify the return value from the RF core through the CMDSTA register within the doorbell */
        if ((cmdstaVal & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
        {
            /* In case othe command was accepted */
            status = RF_StatCmdDoneSuccess;
        }
        else
        {
            /* In case the command was rejected */
            status = RF_StatCmdDoneError;
        }

        /* Free the allocated RAT channel in case of failure */
        if ((status == RF_StatCmdDoneError) || (status == RF_StatRadioInactiveError))
        {
            /* Free the channel and disable interrupts */
            ratChanFree(ratCh, true);

            /* Prepare the return value to be an error code */
            ratCh = RF_ALLOC_ERROR;
        }
        else
        {
            /* Command executed successfully -> set the RAT callback fxn and client */
            pDirImmCmd->pRatCb[ratCh - RF_RAT_CH_LOWEST]  = pRatCb;
            pDirImmCmd->pClient[ratCh - RF_RAT_CH_LOWEST] = h;
        }
    }

    /* Return with the channel number or with an error code */
    return ratCh;
}

/*
 *  ======== RF_ratCapture ========
 *  Set RAT capture
 */
int8_t RF_ratCapture(RF_Handle h, uint16_t config, RF_Callback pRatCb)
{
    /* Unless the RF core is powered, the RAT can not be accessed */
    if ((!bRadioIsActive) && (!bRadioPhyIsBeingSwitched))
    {
        return RF_ALLOC_ERROR;
    }

    /* Prepare a direct command */
    RF_CmdDirImm* pDirImmCmd = &cmdDirImm;

    /* Find and allocate a RAT channel (if any is available) */
    int8_t ratCh = ratChanAlloc(pDirImmCmd);

    /* If we could allocate a RAT channel */
    if (ratCh >= RF_RAT_CH_LOWEST && ratCh <= RF_RAT_CH_HIGHEST)
    {
        /* Enter critical section */
        uint32_t key = HwiP_disable();

        /* Prepare the configuration of the RAT for capture */
        config |= (ratCh << RF_SHIFT_8_BITS) & RF_RAT_CPT_CONFIG_RATCH_MASK;

        /* Submit the command to the doorbell */
        uint32_t cmdstaVal = RFCDoorbellSendTo((uint32_t)CMDR_DIR_CMD_2BYTE(CMD_SET_RAT_CPT, config));

        /* Verify the return value from the RF core through the CMDSTA register within the doorbell */
        if ((cmdstaVal & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
        {
            /* If repeat mode is configured */
            if (config & RF_RAT_CPT_CONFIG_REP_MASK)
            {
                /* Clear the interrupt flag */
                HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIFG) &= ~(1 << (ratCh + RFC_DBELL_RFHWIFG_RATCH0_BITN));

                /* Configure the command itself */
                pDirImmCmd->rptCptRatChNum  = ratCh;
                pDirImmCmd->flags          |= RF_RAT_RPT_CPT_FLAG;
            }
            pDirImmCmd->pClient[ratCh - RF_RAT_CH_LOWEST] = h;
            pDirImmCmd->pRatCb[ratCh - RF_RAT_CH_LOWEST]  = pRatCb;
        }
        else
        {
            /* Free the allocated RAT channel */
            ratChanFree(ratCh, true);

            /* Prepare the return value to be an error code */
            ratCh = RF_ALLOC_ERROR;
        }

        /* Exit critical section*/
        HwiP_restore(key);
    }

    /* Return with the channel number or with an error code */
    return ratCh;
}

/*
 *  ======== RF_ratHwOutput ========
 *  Set RAT HW output
 */
RF_Stat RF_ratHwOutput(RF_Handle h, uint16_t config)
{
    /* Assert */
    DebugP_assert(h != NULL);

    /* Post a direct command to set up GPIO to be a RAT signal*/
    return postDirImmCmd(h, ((uint32_t)CMDR_DIR_CMD_2BYTE(CMD_SET_RAT_OUTPUT, config)), false);
}

/*
 *  ======== RF_ratDisableChannel ========
 *  Disable RAT channel
 */
RF_Stat RF_ratDisableChannel(RF_Handle h, int8_t ratChannelNum)
{
    /* Prepare for a direct command */
    RF_CmdDirImm* pCmd = &cmdDirImm;

    /* Unless the RF core is powered, the RAT can not be accessed */
    if ((!bRadioIsActive) && (!bRadioPhyIsBeingSwitched))
    {
        return RF_StatRadioInactiveError;
    }

    /* If repeat capture flag is set to this particulare RAT channel */
    if ((pCmd->flags & RF_RAT_RPT_CPT_FLAG) && (ratChannelNum == pCmd->rptCptRatChNum))
    {
        /* Clear the flag and RAT channel */
        pCmd->flags         &= ~RF_RAT_RPT_CPT_FLAG;
        pCmd->rptCptRatChNum = 0;
    }

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Free RAT channel */
    ratChanFree(ratChannelNum, true);

    /* Send CMD_DISABLE_RAT_CH as direct command to the RF core */
    uint32_t cmdStaVal = RFCDoorbellSendTo(CMDR_DIR_CMD_2BYTE(CMD_DISABLE_RAT_CH, (ratChannelNum << 8)));

    /* Clear any interrupt sourced from this channel */
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIFG) &= ~(1 << (ratChannelNum + RFC_DBELL_RFHWIFG_RATCH0_BITN));

    /* Exit critical section */
    HwiP_restore(key);

    /* Free allocated RAT channel if CMD_SET_RAT_CMP returns error */
    if ((cmdStaVal & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
    {
        return RF_StatCmdDoneSuccess;
    }
    else
    {
        return RF_StatCmdDoneError;
    }
}

/*
 *  ======== RF_control ========
 *  RF control
 */
RF_Stat RF_control(RF_Handle h, int8_t ctrl, void *args)
{
    /* Prepare the return value for worst case scenario */
    RF_Stat status = RF_StatError;

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Serve the different requests */
    switch (ctrl)
    {
        case RF_CTRL_SET_INACTIVITY_TIMEOUT:
            /* Update the inactivity timeout of the client.
               This can be used if the value given at RF_open
               need to be updated */
            h->clientConfig.nInactivityTimeout = *(uint32_t *)args;
            status = RF_StatSuccess;
            break;

        case RF_CTRL_UPDATE_SETUP_CMD:
            /* Enable a special boot process which can be controlled
               through the config field of the radio setup command.
               This will influence only the next power up sequence
               and will be reset automatically afterwards. The special
               power up process will require longer power up time, hence
               the nPowerUpDuration need to be increased */
            h->clientConfig.bUpdateSetup = true;
            h->clientConfig.nPowerUpDuration += RF_ANALOG_CFG_TIME_US;
            status = RF_StatSuccess;
            break;

        case RF_CTRL_SET_POWERUP_DURATION_MARGIN:
            /* Configure the margin which is added to the measured
               nPowerUpDuration. This can ensure that the commands
               are executed on time, depending on the load of the
               cpu */
            h->clientConfig.nPowerUpDurationMargin = *(uint32_t *)args;
            status = RF_StatSuccess;
            break;

        case RF_CTRL_SET_RAT_RTC_ERR_TOL_VAL:
            /* Configure the tolerance value which is used to determine
               the period when the RAT need to be syncronized to the RTC
               due to the frequency offset */
            powerDurCalc.errTolValInUs = *(uint32_t*)args;
            status = RF_StatSuccess;
            break;

        case RF_CTRL_SET_POWER_MGMT:
            /* The RF drivers power management can be enabled/disabled by
               directly setting the power constraints rom the application.
               It is important that the order of actions have to align */
            if (*(uint32_t*)args == 0)
            {
                RF_setPowerConstraint(RF_PowerConstraintAll);
                status = RF_StatSuccess;
            }
            else if (*(uint32_t*)args == 1)
            {
                RF_releasePowerConstraint(RF_PowerConstraintAll);
                status = RF_StatSuccess;
            }
            else
            {
                status = RF_StatInvalidParamsError;
            }
            break;

        case RF_CTRL_SET_HWI_PRIORITY:
            /* Changing priorities during run-time has constraints.
               To not mess up with the RF driver, we require the RF
               driver to be inactive. */
            if (bRadioIsBeingPoweredUp   || bRadioIsActive ||
                bRadioIsBeingPoweredDown || (Q_peek(&cmdQ.pPend)))
            {
                status = RF_StatBusyError;
            }
            else
            {
                HwiP_setPriority(INT_RFC_CPE_0, *(uint32_t *)args);
                HwiP_setPriority(INT_RFC_HW_COMB, *(uint32_t *)args);

                status = RF_StatSuccess;
            }
            break;

        case RF_CTRL_SET_SWI_PRIORITY:
            /* Changing priorities during run-time has constraints.
               To not mess up with the RF driver, we require the RF
               driver to be inactive. */
            if (bRadioIsBeingPoweredUp   || bRadioIsActive ||
                bRadioIsBeingPoweredDown || (Q_peek(&cmdQ.pPend)))
            {
                status = RF_StatBusyError;
            }
            else
            {
                SwiP_setPriority(&swiFsm, *(uint32_t *)args);
                SwiP_setPriority(&swiHw, *(uint32_t *)args);

                status = RF_StatSuccess;
            }
            break;

        default:
            /* Request can not be served */
            status = RF_StatInvalidParamsError;
            break;
    }

    /* Exit critical section */
    HwiP_restore(key);

    /* Return with the status code */
    return status;
}

/*
 *  ======== RF_requestAccess ========
 *  RF request access
 */
RF_Stat RF_requestAccess(RF_Handle h, RF_AccessParams *pParams)
{
    /* Assert */
    DebugP_assert(h       != NULL);
    DebugP_assert(pParams != NULL);

    /* Convert the requested duration to us */
    uint32_t durationInUs = (pParams->duration  >> 2);

    /* Check if RF_AccessParams are within the acceptable range.
       Only PriorityHighest can be served */
    if ((durationInUs > RF_REQ_ACCESS_MAX_DUR_US) ||
        (pParams->priority != RF_PriorityHighest))
    {
        return RF_StatInvalidParamsError;
    }

    /* Enter critical section */
    uint32_t key = HwiP_disable();

    /* Determine the Id of the requesting client */
    uint8_t clientIdx = 0;
    if (h == RF_Sch.clientHnd[1])
    {
        clientIdx = 1;
    }

    /*  Check if, dedicated access request can be served.
        If possible update the RF_Sch structure and start the timer (RTC) for the reqd duration.
        else, return RF_StatBusyError */
    uint8_t reqAccChk = RF_schReqAccessCheck(h, RF_Sch.clientHnd[clientIdx ^ 0x1]);

    /* If the request can be served */
    if (reqAccChk)
    {
        /* Update the scheduler */
        RF_Sch.accReq[clientIdx].duration = pParams->duration;
        RF_Sch.accReq[clientIdx].priority = pParams->priority;

        /* Start timeout of the request access */
        uint32_t timeout = MAX(durationInUs/ClockP_tickPeriod, 1);
        ClockP_setTimeout(&h->state.clkReqAccess, timeout);
        ClockP_start(&h->state.clkReqAccess);

        /* Exit critical section */
        HwiP_restore(key);

        /* If the request resulted in the preemption of the other client */
        if (reqAccChk == 1)
        {
            /* Flush all the commands from the other client from the queue */
            RF_flushCmd(RF_Sch.preemptCmd.client,
                        RF_Sch.preemptCmd.cancelStartCmdHandle,
                        RF_Sch.preemptCmd.cancelType);
        }

        /* Return with success after the access was granted */
        return RF_StatSuccess;
    }
    else
    {
        /* In case the request can not be served, prepare for a notiication
           callback when the radio becomes available */
        RF_Sch.issueRadioFreeCbFlags |= RF_RADIOFREECB_REQACCESS_FLAG;
        RF_Sch.clientHndRadioFreeCb   = h;

        /* Exit critical section */
        HwiP_restore(key);

        /* Return with an error code */
        return RF_StatBusyError;
    }
}
