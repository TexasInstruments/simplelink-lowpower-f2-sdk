/******************************************************************************

 @file NwkDiscovery.c

 @brief Sub1G Over the Air Download Protocol Module

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2024, Texas Instruments Incorporated
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

/***** Includes *****/
#include "nwk_discovery.h"
#include "zstackapi.h"
#include "zcl_ha.h"
#include "zd_object.h"

#include <ti/drivers/utils/List.h>

#include <ti/drivers/dpl/ClockP.h>

/***** Defines *****/

#define NwkDiscovery_START_DISCOVERY_EVENT          0x0001
#define NwkDiscovery_DEVICE_DISCOVERY_REQ_EVENT     0x0002

/***** Variable declarations *****/

/* Set Default parameters structure */
static const NwkDiscovery_Params_t NwkDiscovery_defaultParams = {
     .nwkDiscoveryPeriod      = NwkDiscovery_DEFAULT_NWK_DISCOVERY_PERIOD,      //No Periodic Nwk Discovery
     .deviceDiscoveryPeriod  = NwkDiscovery_DEFAULT_DEVICE_DISCOVERY_PERIOD,   //500ms between device discovery messages
};

static NwkDiscovery_Params_t nwkDiscovery_params;
static NwkDiscovery_clientFnxs *pNwkDiscovery_clientFnxs;

/* NwkDiscovery device container pool */
static NwkDeviceListEntry_t NwkDiscovery_devicePool[NwkDiscovery_MAX_DEVICES];

static List_List NwkDiscovery_deviceList;

ClockP_Struct nwkDiscoveryTimer;
ClockP_Handle nwkDiscoveryTimerHndl;
ClockP_Struct deviceDiscoveryTimeoutTimer;
ClockP_Handle deviceDiscoveryTimeoutTimerHndl;
ClockP_Struct deviceDiscoveryTimer;
ClockP_Handle deviceDiscoveryTimerHndl;

static uint32_t nwkDiscovery_Event = 0;

/***** Private Functions *****/

static NwkDeviceListEntry_t* NwkDiscovery_deviceAlloc(void);
static void NwkDiscovery_deviceFree(NwkDeviceListEntry_t* nwkDeviceListEntry);
static void NwkDiscovery_deviceListClear(void);
static xdc_Void nwkDiscoveryTimerCb(xdc_UArg arg1);
static xdc_Void deviceDiscoveryTimerCb(xdc_UArg arg1);
static xdc_Void deviceDiscoveryTimeoutTimerCb(xdc_UArg arg1);


/*
 *  Allocate a device entry from the device pool.
 *
 *  Input:  none
 *  Return: NwkDeviceListEntry_t
 */
static NwkDeviceListEntry_t* NwkDiscovery_deviceAlloc(void)
{
    uint32_t i;
    for (i = 0; i < NwkDiscovery_MAX_DEVICES; i++)
    {
        /* Find the first available entry in the command pool */
        if (NwkDiscovery_devicePool[i].discoveryState == discoveryState_free)
        {
            /* Clear entry */
            memset(&NwkDiscovery_devicePool[i], 0, sizeof(NwkDeviceListEntry_t));
            /* Set state to new device */
            NwkDiscovery_devicePool[i].discoveryState = discoveryState_new;
            return(&NwkDiscovery_devicePool[i]);
        }
    }
    return(NULL);
}

/*
 *  Free a device entry from the device pool.
 *
 *  Input:  NwkDeviceListEntry to be freed
 *  Return: none
 */
static void NwkDiscovery_deviceFree(NwkDeviceListEntry_t* nwkDeviceListEntry)
{
    nwkDeviceListEntry->discoveryState = discoveryState_free;
}

/*
 *  Search command in the command pool.
 *
 *  Input:  nwkAddr   - Address of device to search for
 *  Return: NwkDeviceListEntry
 */
NwkDeviceListEntry_t* NwkDiscovery_deviceGet(uint16_t nwkAddr)
{
    NwkDeviceListEntry_t* pDeviceEntry = (NwkDeviceListEntry_t*) List_head(&NwkDiscovery_deviceList);

    while (pDeviceEntry != NULL)
    {
        if ( (pDeviceEntry->discoveredDevice.nwkAddr == nwkAddr) &&
             (pDeviceEntry->discoveryState != discoveryState_free) )

        {
            break;
        }

        pDeviceEntry = (NwkDeviceListEntry_t*) List_next((List_Elem*) pDeviceEntry);
    }

    return pDeviceEntry;
}

/*
 *  Allocate a device entry from the device pool.
 *
 *  Input:  none
 *  Return: NwkDeviceListEntry_t
 */
static void NwkDiscovery_deviceListClear(void)
{
    uint32_t i;

    /* Free all device entries in pool */
    for (i = 0; i < NwkDiscovery_MAX_DEVICES; i++)
    {
        NwkDiscovery_devicePool[i].discoveryState = discoveryState_free;
    }

    /* Remove all devices from list */
    List_clearList(&NwkDiscovery_deviceList);
}

/*
 *  Device discovery timer callback.
 *
 *  Input:  arg1 arguments
 *  Return: none
 */
static xdc_Void nwkDiscoveryTimerCb(xdc_UArg arg1)
{
    // Trigger the network discovery event
    if (pNwkDiscovery_clientFnxs->pfnPostClientNwkDiscoveryEventFxn)
    {
        nwkDiscovery_Event |= NwkDiscovery_START_DISCOVERY_EVENT;
        pNwkDiscovery_clientFnxs->pfnPostClientNwkDiscoveryEventFxn();
    }
}

/*
 *  Device discovery timer callback.
 *
 *  Input:  arg1 arguments
 *  Return: none
 */
static xdc_Void deviceDiscoveryTimerCb(xdc_UArg arg1)
{
    // Trigger the discovery event
    if (pNwkDiscovery_clientFnxs->pfnPostClientNwkDiscoveryEventFxn)
    {
        nwkDiscovery_Event |= NwkDiscovery_DEVICE_DISCOVERY_REQ_EVENT;
        pNwkDiscovery_clientFnxs->pfnPostClientNwkDiscoveryEventFxn();
    }
}

/*
 *  Device discovery timeout timer callback.
 *
 *  Input:  arg1 arguments
 *  Return: none
 */
static xdc_Void deviceDiscoveryTimeoutTimerCb(xdc_UArg arg1)
{
    // Trigger the discovery event again
    if (pNwkDiscovery_clientFnxs->pfnPostClientNwkDiscoveryEventFxn)
    {
        nwkDiscovery_Event |= NwkDiscovery_DEVICE_DISCOVERY_REQ_EVENT;
        pNwkDiscovery_clientFnxs->pfnPostClientNwkDiscoveryEventFxn();
    }
}

/***** Public function definitions *****/

void NwkDiscovery_init(void)
{

}

void NwkDiscovery_Params_init(NwkDiscovery_Params_t *params)
{
    *params = NwkDiscovery_defaultParams;
}

void NwkDiscovery_open(NwkDiscovery_Params_t *params)
{
    ClockP_Params clockParams;
    uint32_t clockTicks;

    /* Populate default params if not provided */
    if (params == NULL)
    {
        NwkDiscovery_Params_init(&nwkDiscovery_params);
        params = &nwkDiscovery_params;
        memcpy(params, &nwkDiscovery_params, sizeof(NwkDiscovery_Params_t));
    }
    else
    {
        memcpy(&nwkDiscovery_params, params, sizeof(NwkDiscovery_Params_t));
    }

    /* Setup timer default params. */
    ClockP_Params_init(&clockParams);
    /* Period is 0, this is a one-shot timer. */
    clockParams.period = 0;
    clockParams.startFlag = false;

    /* Setup device discovery timer. */
    /* Convert deviceDiscoveryPeriod in milliseconds to ticks. */
    clockTicks = nwkDiscovery_params.deviceDiscoveryPeriod * (1000 / Clock_tickPeriod);
    /*/ Initialize clock instance. */
    deviceDiscoveryTimerHndl = ClockP_construct(&deviceDiscoveryTimer, deviceDiscoveryTimerCb, clockTicks, &clockParams);

    /* Setup device discovery timeout timer. */
    /* Convert NwkDiscovery_DEVICE_DISCOVERY_TIMEOUT in milliseconds to ticks. */
    clockTicks = NwkDiscovery_DEVICE_DISCOVERY_TIMEOUT * (1000 / Clock_tickPeriod);
    /*/ Initialize clock instance. */
    deviceDiscoveryTimeoutTimerHndl = ClockP_construct(&deviceDiscoveryTimeoutTimer, deviceDiscoveryTimeoutTimerCb, clockTicks, &clockParams);

    /* Setup network discovery timer. */
    /* Convert deviceDiscoveryPeriod in milliseconds to ticks. */
    clockTicks = nwkDiscovery_params.nwkDiscoveryPeriod * (1000000 / Clock_tickPeriod);
    clockParams.period = clockTicks;
    /*/ Initialize clock instance. */
    nwkDiscoveryTimerHndl = ClockP_construct(&nwkDiscoveryTimer, nwkDiscoveryTimerCb, clockTicks, &clockParams);
}

/** @brief  Register client functions
 *
 *  @param  clientFnxs  client functions
 */
NwkDiscovery_Status_t NwkDiscovery_registerClientFxns(NwkDiscovery_clientFnxs* pClientFnxs)
{
    pNwkDiscovery_clientFnxs = pClientFnxs;

    return NwkDiscovery_Status_Success;
}

/** @brief  Function to start Nwk Discovery process
 *
 *  @param  none
 */
NwkDiscovery_Status_t NwkDiscovery_start(void)
{
    zstack_zdoMgmtLqiReq_t zdoMgmtLqiReq;
    zstack_ZStatusValues zstackStatus;
    NwkDiscovery_Status_t status = NwkDiscovery_Failed;

    /* stop any timers */
    ClockP_stop(nwkDiscoveryTimerHndl);
    ClockP_stop(deviceDiscoveryTimerHndl);
    ClockP_stop(deviceDiscoveryTimeoutTimerHndl);

    /* Reset the list */
    NwkDiscovery_deviceListClear();

    NwkDeviceListEntry_t* pCoordDevice = NwkDiscovery_deviceAlloc();

    /* Create the device for Coord */
    pCoordDevice->discoveredDevice.nwkAddr = 0x0000;
    pCoordDevice->discoveredDevice.deviceType = zstack_LogicalTypes_COORDINATOR;
    pCoordDevice->discoveredDevice.rxOnWhenIdle = zstack_RxOnWhenIdleTypes_ON;
    pCoordDevice->discoveredDevice.parentAddress = 0xFFFE; // Coord does not have a parent
    pCoordDevice->discoveredDevice.lightEndPoint = 0xFF;

    /* Start discovery process by sending request to Coord */
    zdoMgmtLqiReq.nwkAddr = 0x0000;
    zdoMgmtLqiReq.startIndex = 0;
    zstackStatus = Zstackapi_ZdoMgmtLqiReq( nwkDiscovery_params.appServiceTaskId ,
                             &zdoMgmtLqiReq);

    /* Add Coord as the first device */
    List_put(&NwkDiscovery_deviceList, (List_Elem*)pCoordDevice);

    /* call update callback to trigger notification over BLE */
    if (pNwkDiscovery_clientFnxs->pfnDeviceDiscoveryCb != NULL)
    {
        pNwkDiscovery_clientFnxs->pfnDeviceDiscoveryCb(&(pCoordDevice->discoveredDevice));
    }

    if(zstackStatus == zstack_ZStatusValues_ZSuccess)
    {
        status = NwkDiscovery_Status_Success;
    }

    //Start the periodic discovery timer if needed
    if(nwkDiscovery_params.nwkDiscoveryPeriod > 0)
    {
        ClockP_start(nwkDiscoveryTimerHndl);
    }

    return status;
}

NwkDiscovery_Status_t NwkDiscovery_processMatchDescRspInd(zstackmsg_zdoMatchDescRspInd_t* pZdoMatchDescRspInd)
{
    NwkDeviceListEntry_t *pDiscoveredDevice;
    NwkDiscovery_Status_t status = NwkDiscovery_Failed;

    /* Get device from List */
    pDiscoveredDevice = NwkDiscovery_deviceGet(pZdoMatchDescRspInd->rsp.nwkAddrOfInterest);

    if (pDiscoveredDevice == NULL)
    {
        return status;
    }

    if (pDiscoveredDevice->discoveryState == discoveryState_discovered)
    {
        return status;
    }

    if (pZdoMatchDescRspInd->rsp.n_matchList != 0x00)
    {
        pDiscoveredDevice->discoveredDevice.lightEndPoint = pZdoMatchDescRspInd->rsp.pMatchList[0];
        status = NwkDiscovery_Status_Success;

    }

    /* Update the state of the device */
    pDiscoveredDevice->discoveryState = discoveryState_discovered;

    /*
     * call update callback to trigger notification over BLE
     * But not for the coordinator as the notification was sent already
     */
    if (pNwkDiscovery_clientFnxs->pfnDeviceDiscoveryCb != NULL
            && pDiscoveredDevice->discoveredDevice.nwkAddr != 0x0000)
    {
        pNwkDiscovery_clientFnxs->pfnDeviceDiscoveryCb(&(pDiscoveredDevice->discoveredDevice));
    }

    return status;
}

NwkDiscovery_Status_t NwkDiscovery_processMgmtLqiRspInd(zstackmsg_zdoMgmtLqiRspInd_t* pZdoMgmtLqiRspInd)
{
    NwkDeviceListEntry_t *pDiscoveredDevice, *pDeviceSearch;
    NwkDiscovery_Status_t status = NwkDiscovery_Failed;
    uint32_t i;

    /* Stop Discovery Timers */
    ClockP_stop(deviceDiscoveryTimerHndl);
    ClockP_stop(deviceDiscoveryTimeoutTimerHndl);

    /* Get device from List */
    pDiscoveredDevice = NwkDiscovery_deviceGet(pZdoMgmtLqiRspInd->rsp.srcAddr);

    if(pDiscoveredDevice == NULL)
    {
        return NwkDiscovery_InvalidParam;
    }

    /* Set number of neighbors to discover (number of entries includes self) */
    pDiscoveredDevice->discoveredDevice.neighborLqiEntries = pZdoMgmtLqiRspInd->rsp.neighborLqiEntries;

    /* Got a response reset timeout */
    pDiscoveredDevice->retryCount = 0;

    /* Add neighbor devices to list */
    for (i=0; i < pZdoMgmtLqiRspInd->rsp.n_lqiList; i++)
    {
        //check device is not already in the list and it is a child
        if ((NwkDiscovery_deviceGet(pZdoMgmtLqiRspInd->rsp.pLqiList[i].nwkAddr) == NULL) &&
            ((pZdoMgmtLqiRspInd->rsp.pLqiList[i].relationship == zstack_RelationTypes_CHILD) ||
             (pZdoMgmtLqiRspInd->rsp.pLqiList[i].relationship == zstack_RelationTypes_SIBLING)))
        {
            NwkDeviceListEntry_t* pNewDeviceEntry = NwkDiscovery_deviceAlloc();

            if (pNewDeviceEntry != NULL)
            {
                pNewDeviceEntry->discoveredDevice.nwkAddr = pZdoMgmtLqiRspInd->rsp.pLqiList[i].nwkAddr;
                memcpy(&pNewDeviceEntry->discoveredDevice.extendedAddr,
                       &pZdoMgmtLqiRspInd->rsp.pLqiList[i].extendedAddr,
                       EXTADDR_LEN);
                pNewDeviceEntry->discoveredDevice.parentAddress = pZdoMgmtLqiRspInd->rsp.srcAddr;
                pNewDeviceEntry->discoveredDevice.rxLqi = pZdoMgmtLqiRspInd->rsp.pLqiList[i].rxLqi;
                pNewDeviceEntry->discoveredDevice.deviceType = pZdoMgmtLqiRspInd->rsp.pLqiList[i].deviceType;
                pNewDeviceEntry->discoveredDevice.rxOnWhenIdle = pZdoMgmtLqiRspInd->rsp.pLqiList[i].rxOnWhenIdle;
                pNewDeviceEntry->discoveredDevice.lightEndPoint = 0xFF; // Default lightendPoint

                /* Add device to List */
                List_put(&NwkDiscovery_deviceList, (List_Elem*)pNewDeviceEntry);
            }
            else
            {
                status = NwkDiscovery_FailedMaxDevices;
            }
        }

        /* Increase the number of devices reported */
        pDiscoveredDevice->discoveredDevice.neighborLqiEntriesReported++;

        /* If we have finished LQI discovery of this device then start Ep discovery */
        if(pDiscoveredDevice->discoveredDevice.neighborLqiEntriesReported ==
                pDiscoveredDevice->discoveredDevice.neighborLqiEntries)
        {
            /* Update discovery state */
            pDiscoveredDevice->discoveryState = discoveryState_lqi_rsp_rcvd;
        }
    }


    /* Search List for devices to be discovered and start disc timer */
    pDeviceSearch = (NwkDeviceListEntry_t*) List_head(&NwkDiscovery_deviceList);
    while (pDeviceSearch != NULL)
    {
        if (pDeviceSearch->discoveryState != discoveryState_discovered)
        {
            ClockP_start(deviceDiscoveryTimerHndl);
            ClockP_start(deviceDiscoveryTimeoutTimerHndl);
            break;
        }

        pDeviceSearch = (NwkDeviceListEntry_t*) List_next((List_Elem*) pDeviceSearch);
    }


    return status;
}

/** @brief  Function to process NwkDiscovery events
 *
 *  @param  none
 */
NwkDiscovery_Status_t NwkDiscovery_processEvents(void)
{
    NwkDiscovery_Status_t status = NwkDiscovery_Failed;

    if(nwkDiscovery_Event & NwkDiscovery_START_DISCOVERY_EVENT)
    {
        status = NwkDiscovery_start();
        nwkDiscovery_Event &= ~NwkDiscovery_START_DISCOVERY_EVENT;
    }

    if(nwkDiscovery_Event & NwkDiscovery_DEVICE_DISCOVERY_REQ_EVENT)
    {
        zstack_zdoMgmtLqiReq_t zdoMgmtLqiReq;
        NwkDeviceListEntry_t* pDeviceToDiscover;

        /* stop timers */
        ClockP_stop(deviceDiscoveryTimerHndl);
        ClockP_stop(deviceDiscoveryTimeoutTimerHndl);

        pDeviceToDiscover = (NwkDeviceListEntry_t*) List_head(&NwkDiscovery_deviceList);

        /* Search List for devices to be discovered */
        while (pDeviceToDiscover != NULL)
        {
            if (pDeviceToDiscover->discoveryState != discoveryState_discovered)
            {
                break;
            }

            pDeviceToDiscover = (NwkDeviceListEntry_t*) List_next((List_Elem*) pDeviceToDiscover);
        }

        if (pDeviceToDiscover != NULL)
        {
            if(pDeviceToDiscover->retryCount < NwkDiscovery_DEVICE_DISCOVERY_MAX_RETRIES)
            {
                if(pDeviceToDiscover->discoveryState == discoveryState_new)
                {
                    /* Send device a MngtLqiReq */
                    zdoMgmtLqiReq.nwkAddr = pDeviceToDiscover->discoveredDevice.nwkAddr;
                    zdoMgmtLqiReq.startIndex = pDeviceToDiscover->discoveredDevice.neighborLqiEntriesReported;
                    Zstackapi_ZdoMgmtLqiReq( nwkDiscovery_params.appServiceTaskId ,
                                             &zdoMgmtLqiReq);
                }
                else if (pDeviceToDiscover->discoveryState == discoveryState_lqi_rsp_rcvd)
                {
                    /* Send Match Desc Req to obtain proper lightEndPoint data */
                    zstack_zdoMatchDescReq_t zdoMatchDescReq;
                    uint16_t cluster = ZCL_CLUSTER_ID_GENERAL_ON_OFF;
                    zdoMatchDescReq.dstAddr = pDeviceToDiscover->discoveredDevice.nwkAddr;
                    zdoMatchDescReq.nwkAddrOfInterest = pDeviceToDiscover->discoveredDevice.nwkAddr;
                    zdoMatchDescReq.n_outputClusters = 0;
                    zdoMatchDescReq.n_inputClusters = 1;
                    zdoMatchDescReq.pInputClusters = &cluster;
                    zdoMatchDescReq.profileID = ZCL_HA_PROFILE_ID;
                    Zstackapi_ZdoMatchDescReq (nwkDiscovery_params.appServiceTaskId,
                                                              &zdoMatchDescReq);
                }

                pDeviceToDiscover->retryCount++;

                /* Start discovery timeout timer */
                ClockP_start(deviceDiscoveryTimeoutTimerHndl);
            }
            else
            {
                if(pDeviceToDiscover->discoveryState == discoveryState_lqi_rsp_rcvd)
                {
                    //device may not support match desc rsp
                    pDeviceToDiscover->discoveryState = discoveryState_discovered;

                    /* call update callback to trigger notification over BLE */
                    if (pNwkDiscovery_clientFnxs->pfnDeviceDiscoveryCb != NULL)
                    {
                        pNwkDiscovery_clientFnxs->pfnDeviceDiscoveryCb(&(pDeviceToDiscover->discoveredDevice));
                    }
                }
                else
                {
                    /* device not responding, remove from list and free device element */
                    List_remove(&NwkDiscovery_deviceList, (List_Elem*) pDeviceToDiscover);
                    NwkDiscovery_deviceFree(pDeviceToDiscover);
                }

            }

            /* Are there any other devices to discover */
            pDeviceToDiscover = (NwkDeviceListEntry_t*) List_head(&NwkDiscovery_deviceList);
            while (pDeviceToDiscover != NULL)
            {
                if (pDeviceToDiscover->discoveryState != discoveryState_discovered)
                {
                    /* Start start discovery timer for next device */
                    ClockP_start(deviceDiscoveryTimerHndl);
                }

                pDeviceToDiscover = (NwkDeviceListEntry_t*) List_next((List_Elem*) pDeviceToDiscover);
            }
        }

        nwkDiscovery_Event &= ~NwkDiscovery_DEVICE_DISCOVERY_REQ_EVENT;
    }

    return status;
}

