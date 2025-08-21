#include <stdint.h>

#include <inc/hw_ccfg.h>
#include <inc/hw_ccfg_simple_struct.h>
#include <inc/hw_memmap.h>
#include <inc/hw_fcfg1.h>
#include <inc/hw_types.h>

#include "mac_hl_patch.h"
#include "fhss_ws_extension.h"
#include "mac_assert.h"
#include "rcp_mac_common.h"
#include "rcp_types.h"

#include <ti/drivers/GPIO.h>
#include <driverlib/ioc.h>

#ifndef FREERTOS_SUPPORT
#include <ti/sysbios/knl/Task.h>
#else
#include <FreeRTOS.h>
#endif

extern rcp_lmac_dbg_t rcp_lmac_dbg;
extern rcp_btie_debug_t rcp_btie_dbg;

bool disableBCRequeue = false;

void timac_LogEvent(uint32_t eventID,uint32_t timestamp,uint32_t st1,uint32_t st2,uint32_t st3,uint32_t st4) {}
uint8 MAC_MlmeGetSecurityReq(uint8 pibAttribute, void *pValue) { return 0;}
void macSecurityPibReset(void) {}
#ifdef WISUN_RCP_LMAC
void ns_put_char_blocking(const char ch) {}
#endif

#define ENABLE_GPIO_MPL

#define FH_UNICAST_GPIO             IOID_18
#define HOST_RX_GPIO                IOID_19
#define MAC_BROADCAST_GPIO          IOID_20
#define FH_BROADCAST_GPIO           IOID_21

#define RF_RX_GPIO                  IOID_23
#define RF_TX_GPIO                  IOID_24


void rcp_timacSetTrackParent(broadcast_timing_info_t *bc_timing_info, const bool force_synch)
{
    halIntState_t intState;
    uint32_t val32;
    uint16_t val16;
    uint8_t  val8;
    uint8_t bitMap[MAC_154G_CHANNEL_BITMAP_SIZ];

    rcp_lmac_dbg.num_rcp_set_parent++;
    if (force_synch) {
        rcp_lmac_dbg.num_forch_synch++;
    }

    if (macPanCoordinator) {
        return;
    }

    // enter critical sectic on
    HAL_ENTER_CRITICAL_SECTION(intState);

    // save the BS IE to FH PIB
    // save broadcast_interval (32 bit)
    val32 = bc_timing_info->broadcast_interval;
    FHPIB_set(MAC_FHPIB_BC_INTERVAL, &val32);
    // save broadcast schedule ID (16 bit)
    val16 = bc_timing_info->broadcast_schedule_id;
    FHPIB_set(MAC_FHPIB_BROCAST_SCHED_ID, &val16);
    // save BC dwell time (8 bit)
    val8 = bc_timing_info->broadcast_dwell_interval;
    FHPIB_set(MAC_FHPIB_BC_DWELL_INTERVAL, &val8);
    // save the BC channel function (8 bit)
    val8 = bc_timing_info->broadcast_channel_function;
    MAP_FHPIB_set(MAC_FHPIB_BC_CHANNEL_FUNCTION, &val8);

    if (bc_timing_info->broadcast_channel_function == FHIE_CF_SINGLE_CHANNEL)
    {   // fixed channel (single channel) (16 bits)
        val16 = bc_timing_info->fixed_channel;
        MAP_FHPIB_set(MAC_FHPIB_BC_FIXED_CHANNEL, &val16);
    }
    else if (bc_timing_info->broadcast_channel_function == FHIE_CF_DH1CF)
    {   // save the exclude channel
        uint8_t i,mask;

        for (i=0; i< MAC_154G_CHANNEL_BITMAP_SIZ; i++)
        {
            // nanoStack provides the active channel, we need to convert to exclude list
            // nanoStack channel mask is uint32
            mask = bc_timing_info->bc_channel_list.channel_mask2[i] ;

            // flip over the bit, convert into the excluded list
            mask = ~mask;
            bitMap[i] = mask;
        }
        FHPIB_set(MAC_FHPIB_BC_EXCLUDED_CHANNELS, bitMap);

        // also update the number of BC channels
        FH_hnd.bcNumChannels = bc_timing_info->bc_channel_list.channel_count;
    }

    // mark the BS-IE is received
    FH_hnd.fhBSRcvd = 1;

    // save BT-IE
    FH_hnd.rxSlotIdx   = bc_timing_info->broadcast_slot;
    FH_hnd.btie        = bc_timing_info->broadcast_interval_offset;
    // mark the BT-IE is received
    FH_hnd.btiePresent = 1;

    //  convert the BTIE time into tick count
    FH_hnd.rcp_btie_ts = bc_timing_info->bt_rx_timestamp ;

    /* recording BTIE delay */
    rcp_btie_dbg.btieDelay = FHUTIL_getElapsedTime(FH_hnd.rcp_btie_ts);

    if (rcp_btie_dbg.btieDelay > rcp_btie_dbg.btieMaxDelay)
    {
        rcp_btie_dbg.btieMaxDelay = rcp_btie_dbg.btieDelay;
    }
    if (rcp_btie_dbg.btieDelay < rcp_btie_dbg.btieMinDelay)
    {
        rcp_btie_dbg.btieMinDelay = rcp_btie_dbg.btieDelay;
    }

    if (rcp_btie_dbg.btieDelay > (bc_timing_info->broadcast_interval * 2))
    {
        rcp_btie_dbg.num_bfio_error_long_delay++;
    }

    // save the current timestamp
    // or use the icall : MAP_ICall_getTicks
    rcp_btie_dbg.curTimeStamp = ClockP_getSystemTicks();
    rcp_btie_dbg.rx_btie_TimeStamp = FH_hnd.rcp_btie_ts ;

    rcp_btie_dbg.rx_bfio = FH_hnd.btie; // save the FH Hnd's btie to rx_bfio
    if (force_synch)
    {   // if nanostack force resync, we need to stop BC timer,and treat as BS is not started
        FHMGR_macCancelFHTimer(&FH_hnd.bcTimer);    // stop the BC timer
        FH_hnd.bsStarted = 0 ;  // mark BS is not started
    }
    // update the BTIE
    FHUTIl_updateBTIE(FH_hnd.btie, FH_hnd.rxSlotIdx);
    rcp_btie_dbg.rx_bfio_update = FH_hnd.btie; // save the update FH Hnd's btie to rx_bfio_update

    /* save the stats*/
    if (FH_hnd.btie_need_update)
    {
        rcp_btie_dbg.num_bfio_need_udate++;
        if (FH_hnd.rx_bfio_delta > 0)
        {
            rcp_btie_dbg.num_bfio_udate_positive++;
        }
        else
        {
            rcp_btie_dbg.num_bfio_udate_negative++;
        }
    }

    if (FH_hnd.bsStarted == 0 )
    {   // start the BS
        MAP_FHAPI_startBS();
    }

    // clear the BTIE flag
    FH_hnd.btiePresent = 0;

    // save the last rx_btir time stamp
    rcp_btie_dbg.last_rx_btie_TimeStamp = rcp_btie_dbg.rx_btie_TimeStamp;

    //exit critical section
    HAL_EXIT_CRITICAL_SECTION(intState);
}

void ccfg_read_mac_addr(uint8_t *mac_addr)
{
    uint8_t invalidExtAddr[] =
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint32_t read_mac_addr;
    uint8_t tmp_mac_addr[MAC_ADDR_LEN];

    read_mac_addr = (( HWREG(
            CCFG_BASE + CCFG_O_IEEE_MAC_0 ) &
            CCFG_IEEE_MAC_0_ADDR_M ) >>
            CCFG_IEEE_MAC_0_ADDR_S );
    memcpy(tmp_mac_addr, (uint8_t *)&read_mac_addr, (MAC_ADDR_LEN / 2));

    read_mac_addr = (( HWREG(
            CCFG_BASE + CCFG_O_IEEE_MAC_1 ) &
            CCFG_IEEE_MAC_1_ADDR_M ) >>
            CCFG_IEEE_MAC_1_ADDR_S );
    memcpy(tmp_mac_addr + (MAC_ADDR_LEN / 2), (uint8_t *)&read_mac_addr,
           (MAC_ADDR_LEN / 2));

    /* Check to see if the CCFG IEEE is valid */
    if(memcmp(tmp_mac_addr, invalidExtAddr, MAC_ADDR_LEN) == 0)
    {
        /* No, it isn't valid.  Get the Primary IEEE Address */
        memcpy(tmp_mac_addr, (uint8_t *)(FCFG1_BASE + FCFG1_O_MAC_15_4_0),
               (MAC_ADDR_LEN));
    }

    sAddrExtRevCpy(mac_addr, tmp_mac_addr);
}

void timac_setup_Test_GPIO(void)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_setConfig(FH_UNICAST_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(HOST_RX_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(MAC_BROADCAST_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(FH_BROADCAST_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    // Map LNA PIN RFC_GPO0
    IOCPortConfigureSet(RF_RX_GPIO, IOC_PORT_RFC_GPO0,IOC_IOMODE_NORMAL);

    // Map TX start pin RFC_GPO3
    IOCPortConfigureSet(RF_TX_GPIO, IOC_PORT_RFC_GPO3,IOC_IOMODE_NORMAL);
#endif
}

void timac_GetBC_Slot_BFIO(uint16_t *slot, uint32_t *bfio)
{
    FHAPI_status st;
    st = FHUTIL_getCurBfio(bfio, slot);
}

void timac_Set_FH_Broadcast_GPIO(uint8_t st)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_write(FH_BROADCAST_GPIO,st);
#endif
}

void timac_Set_FH_UNICAST_GPIO(uint8_t st)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_write(FH_UNICAST_GPIO,st);
#endif
}

void timac_Set_HOST_RX_GPIO(uint8_t st)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_write(HOST_RX_GPIO,st);
#endif
}

void timac_Set_MAC_BROADCAST_GPIO(uint8_t st)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_write(MAC_BROADCAST_GPIO,st);
#endif
}

#ifdef FREERTOS_SUPPORT
#define RF_CB_THREADSTACKSIZE 2000

pthread_t rfCdThreadHndl = NULL;
sem_t rfCbSemHandle;

uint32_t rxCount;
bool rxRec;
// rf queue variables
uint8_t macRfQueueSize;
macRfQueue_t macRfQueue[MAC_FREERTOS_RF_SLOTS];
uint8_t wRfPtr;
uint8_t rRfPtr;
uint8_t numRfCmdQueued;

void runRxCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    //pushing to rf queue
    macRfQueue[wRfPtr].rfCbHndl = h;
    macRfQueue[wRfPtr].rfCbCmdHndl = ch;
    macRfQueue[wRfPtr].rfCbEventMask = e;
    macRfQueue[wRfPtr].rfCb = macRxCb;
    wRfPtr = (wRfPtr + 1) % macRfQueueSize;
    numRfCmdQueued++;
    rxCount++;

    /* Unblock the task by releasing the semaphore. */
    sem_post(&rfCbSemHandle);
}

void *rfCbThread(void *arg0)
{
    while(true)
    {
        sem_wait(&rfCbSemHandle);
        if (numRfCmdQueued)
        {
            macRfQueue[rRfPtr].rfCb(macRfQueue[rRfPtr].rfCbHndl,
                                   macRfQueue[rRfPtr].rfCbCmdHndl,
                                   macRfQueue[rRfPtr].rfCbEventMask);
            portENTER_CRITICAL();
            numRfCmdQueued--;
            rRfPtr = (rRfPtr + 1) % macRfQueueSize;
            portEXIT_CRITICAL();
        }
    }
}

void startRfCbThread(void)
{
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    rxCount = 0;
    rxRec = true;
    numRfCmdQueued = 0;

    // initialize the RF Queue
    wRfPtr = 0;
    rRfPtr = 0;
    macRfQueueSize = MAC_FREERTOS_RF_SLOTS;
    for (int i = 0; i < macRfQueueSize; i++)
    {
        memset(&macRfQueue[i], 0, sizeof(macRfQueue_t));
    }

    if(rfCdThreadHndl == NULL)
    {
        /* create semaphores
         */
        retc = sem_init(&rfCbSemHandle, 0, 0);
        if (retc != 0) {
            while (1) {}
        }

        /* Initialize the attributes structure with default values */
        pthread_attr_init(&attrs);

        /* Set priority, detach state, and stack size attributes */
        priParam.sched_priority = (configMAX_PRIORITIES - 1);
        retc = pthread_attr_setschedparam(&attrs, &priParam);
        retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
        retc |= pthread_attr_setstacksize(&attrs, RF_CB_THREADSTACKSIZE);
        if (retc != 0) {
            /* failed to set attributes */
            while (1) {}
        }

        retc = pthread_create(&rfCdThreadHndl, &attrs, rfCbThread, NULL);
        if (retc != 0) {
            /* pthread_create() failed */
            while (1) {}
        }
    }
}

#endif //FREERTOS_SUPPORT