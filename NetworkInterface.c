/* Copyright 2018 Espressif Systems (Shanghai) PTE LTD */
/* */
/* Licensed under the Apache License, Version 2.0 (the "License"); */
/* you may not use this file except in compliance with the License. */
/* You may obtain a copy of the License at */
/* */
/*     http://www.apache.org/licenses/LICENSE-2.0 */
/* */
/* Unless required by applicable law or agreed to in writing, software */
/* distributed under the License is distributed on an "AS IS" BASIS, */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/* See the License for the specific language governing permissions and */
/* limitations under the License. */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_DNS.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

#include "tusb.h"
#include "network.h"
#include "crude_ip.h" // frame_t
#include "bsp/board.h" // board_millis()

enum if_state_t
{
    INTERFACE_DOWN = 0,
    INTERFACE_UP,
};

static const char * TAG = "NetInterface";
volatile static uint32_t xInterfaceState = INTERFACE_DOWN;

static NetworkInterface_t * pxMyInterface;

static BaseType_t xRNDIS_Eth_NetworkInterfaceInitialise( NetworkInterface_t * pxInterface );

static BaseType_t xRNDIS_Eth_NetworkInterfaceOutput( NetworkInterface_t * pxInterface,
                                                     NetworkBufferDescriptor_t * const pxDescriptor,
                                                     BaseType_t xReleaseAfterSend );

static BaseType_t xRNDIS_Eth_GetPhyLinkStatus( NetworkInterface_t * pxInterface );

NetworkInterface_t * pxRNDIS_Eth_FillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                          NetworkInterface_t * pxInterface );

/*-----------------------------------------------------------*/

#if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )

/* Do not call the following function directly. It is there for downward compatibility.
 * The function FreeRTOS_IPInit() will call it to initialice the interface and end-point
 * objects.  See the description in FreeRTOS_Routing.h. */
    NetworkInterface_t * pxFillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                    NetworkInterface_t * pxInterface )
    {
        pxRNDIS_Eth_FillInterfaceDescriptor( xEMACIndex, pxInterface );
    }

#endif
/*-----------------------------------------------------------*/


NetworkInterface_t * pxRNDIS_Eth_FillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                          NetworkInterface_t * pxInterface )
{
    static char pcName[ 8 ];

/* This function pxESP32_Eth_FillInterfaceDescriptor() adds a network-interface.
 * Make sure that the object pointed to by 'pxInterface'
 * is declared static or global, and that it will remain to exist. */

    snprintf( pcName, sizeof( pcName ), "eth%ld", xEMACIndex );

    memset( pxInterface, '\0', sizeof( *pxInterface ) );
    pxInterface->pcName = pcName;                    /* Just for logging, debugging. */
    pxInterface->pvArgument = ( void * ) xEMACIndex; /* Has only meaning for the driver functions. */
    pxInterface->pfInitialise = xRNDIS_Eth_NetworkInterfaceInitialise;
    pxInterface->pfOutput = xRNDIS_Eth_NetworkInterfaceOutput;
    pxInterface->pfGetPhyLinkStatus = xRNDIS_Eth_GetPhyLinkStatus;

    FreeRTOS_AddNetworkInterface( pxInterface );
    pxMyInterface = pxInterface;

    return pxInterface;
}
/*-----------------------------------------------------------*/

static BaseType_t xRNDIS_Eth_NetworkInterfaceInitialise( NetworkInterface_t * pxInterface )
{
    static BaseType_t xMACAdrInitialized = pdFALSE;
    uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ];

    if( xInterfaceState == INTERFACE_UP )
    {
        if( xMACAdrInitialized == pdFALSE )
        {
            //esp_wifi_get_mac( ESP_IF_WIFI_STA, ucMACAddress );
            FreeRTOS_UpdateMACAddress( ucMACAddress );
            xMACAdrInitialized = pdTRUE;
        }

        return pdTRUE;
    }

    return pdFALSE;
}

static BaseType_t xRNDIS_Eth_GetPhyLinkStatus( NetworkInterface_t * pxInterface )
{
    BaseType_t xResult = pdFALSE;

    if( xInterfaceState == INTERFACE_UP )
    {
        xResult = pdTRUE;
    }

    return xResult;
}

static BaseType_t xRNDIS_Eth_NetworkInterfaceOutput( NetworkInterface_t * pxInterface,
                                                     NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                                     BaseType_t xReleaseAfterSend )
{
    if( ( pxNetworkBuffer == NULL ) || ( pxNetworkBuffer->pucEthernetBuffer == NULL ) || ( pxNetworkBuffer->xDataLength == 0 ) )
    {
        //ESP_LOGE( TAG, "Invalid params" );
        return pdFALSE;
    }

    BaseType_t ret;

    if( xInterfaceState == INTERFACE_DOWN )
    {
        //ESP_LOGD( TAG, "Interface down" );
        ret = pdFALSE;
    }
    else
    {
        ret = linkoutput_fn_v2(pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength) ? pdTRUE : pdFALSE;
    }

    #if ( ipconfigHAS_PRINTF != 0 )
        {
            /* Call a function that monitors resources: the amount of free network
             * buffers and the amount of free space on the heap.  See FreeRTOS_IP.c
             * for more detailed comments. */
            vPrintResourceStats();
        }
    #endif /* ( ipconfigHAS_PRINTF != 0 ) */

    if( xReleaseAfterSend == pdTRUE )
    {
        vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
    }

    return ret;
}

void vNetworkNotifyIFDown()
{
    IPStackEvent_t xRxEvent = { eNetworkDownEvent, NULL };

    if( xInterfaceState != INTERFACE_DOWN )
    {
        xInterfaceState = INTERFACE_DOWN;
        xSendEventStructToIPTask( &xRxEvent, 0 );
    }
}

void vNetworkNotifyIFUp()
{
    xInterfaceState = INTERFACE_UP;
}

extern uint32_t last_rx;
extern uint32_t last_tx;

// Always accept incoming frames and return true
// Should return false here if overrun
bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
    NetworkBufferDescriptor_t * pxNetworkBuffer;
    IPStackEvent_t xRxEvent = { eNetworkRxEvent, NULL };
    const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS( 250 );

	last_rx = board_millis();

    #if ( ipconfigHAS_PRINTF != 0 )
        {
            vPrintResourceStats();
        }
    #endif /* ( ipconfigHAS_PRINTF != 0 ) */

    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( size, xDescriptorWaitTime );

    if( pxNetworkBuffer != NULL )
    {
        /* Set the packet size, in case a larger buffer was returned. */
        pxNetworkBuffer->xDataLength = size;
        pxNetworkBuffer->pxInterface = pxMyInterface;
        pxNetworkBuffer->pxEndPoint = FreeRTOS_MatchingEndpoint( pxMyInterface, src );

        /* Copy the packet data. */
        memcpy( pxNetworkBuffer->pucEthernetBuffer, src, size );
        xRxEvent.pvData = ( void * ) pxNetworkBuffer;

        if( xSendEventStructToIPTask( &xRxEvent, xDescriptorWaitTime ) == pdFAIL )
        {
            vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
            return false;
        }

        tud_network_recv_renew();

        return true;
    }
    else
    {
        return false;
    }
}

// dst = tinyusb transmit queue pointer
// ref = packet structure
uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
	frame_t* frame = (frame_t*)ref;
	(void)arg;

	memcpy(dst, frame->data, frame->size);

	last_tx = board_millis();

	return frame->size;
}

// the network is re-initializing
void tud_network_init_cb(void) {
}
