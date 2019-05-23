/*
 * This demo application is distributed under the terms of CC0 (public domain dedication).
 * More info: https://creativecommons.org/publicdomain/zero/1.0/
 */

// This is needed to enable necessary declarations in sys/
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <skalibs/iopause.h>
#include <skalibs/tai.h>

#include <canard.h>
#include <drivers/socketcan.h>      // CAN backend driver for SocketCAN, distributed with Libcanard

#include <libcanard.h>

#define UAVCAN_MY_MSG_MESSAGE_SIZE                                  20
#define UAVCAN_MY_MSG_DATA_TYPE_ID                                  27
#define UAVCAN_MY_MSG_DATA_TYPE_SIGNATURE                           0x5A5A5A5A5A5A5A5A

static uint8_t dest_id=0;
static uint8_t node_id=0;

/*
 * Library instance.
 * In simple applications it makes sense to make it static, but it is not necessary.
 */
static CanardInstance g_canard;                     ///< The library instance
static uint8_t g_canard_memory_pool[1024];          ///< Arena for memory allocation, used by the library

// libcanard
static unsigned char mymsg_buf[UAVCAN_MY_MSG_MESSAGE_SIZE];

static const canard_item_t canard_storage[] = {
    { .hash = UAVCAN_MY_MSG_DATA_TYPE_SIGNATURE, .id = UAVCAN_MY_MSG_DATA_TYPE_ID, .lg = UAVCAN_MY_MSG_MESSAGE_SIZE, .buf = mymsg_buf, .name = "mymsg" },
    { .hash = 0, .id = 0, .lg = 0, .buf = 0, .name = 0}    
};

static uint64_t getMonotonicTimestampUSec(void) {
    struct timespec ts;    
    timespec_from_tain (&ts, &STAMP);    
    return (uint64_t)((ts.tv_sec * 1000000LL) + (ts.tv_nsec / 1000LL));
}

/**
 * This callback is invoked by the library when a new message or request or response is received.
 */
static void onTransferReceived(CanardInstance* ins,
                               CanardRxTransfer* transfer)
{

    if (transfer->transfer_type == CanardTransferTypeResponse) 
        printf("onTransferReceived transfer Response\n");
    else if (transfer->transfer_type == CanardTransferTypeRequest) 
        printf("onTransferReceived transfer Request\n");
    else
        printf("onTransferReceived transfer Unknown\n");
    
    printf("onTransferReceived data type %d\n", transfer->data_type_id);
    
    if ((transfer->transfer_type == CanardTransferTypeResponse) &&
        (transfer->data_type_id == UAVCAN_MY_MSG_DATA_TYPE_ID))
    {
        printf("My Msg from %d\n", transfer->source_node_id);
        printf("My Msg data size %d\n", transfer->payload_len);
        return;
    }
}

/**
 * This callback is invoked by the library when it detects beginning of a new transfer on the bus that can be received
 * by the local node.
 * If the callback returns true, the library will receive the transfer.
 * If the callback returns false, the library will ignore the transfer.
 * All transfers that are addressed to other nodes are always ignored.
 */
static bool shouldAcceptTransfer(const CanardInstance* ins,
                                 uint64_t* out_data_type_signature,
                                 uint16_t data_type_id,
                                 CanardTransferType transfer_type,
                                 uint8_t source_node_id)
{
    (void)source_node_id;

    if (canardGetLocalNodeID(ins) == CANARD_BROADCAST_NODE_ID) {
        fprintf(stderr, "node ID not set");
    }
    else
    {
        if ((transfer_type == CanardTransferTypeResponse) &&
            (data_type_id == UAVCAN_MY_MSG_DATA_TYPE_ID))
        {
            *out_data_type_signature = UAVCAN_MY_MSG_DATA_TYPE_SIGNATURE;
            return true;
        }
    }

    return false;
}

void send_mymsg() {
        
    uint8_t buffer[UAVCAN_MY_MSG_MESSAGE_SIZE];
    for(int i=0; i<UAVCAN_MY_MSG_MESSAGE_SIZE; i++)
        buffer[i] = 0x11+i;
    
    static uint8_t transfer_id;  // Note that the transfer ID variable MUST BE STATIC (or heap-allocated)!

    const int16_t bc_res = canardRequestOrRespond(&g_canard,
                                dest_id,
                                UAVCAN_MY_MSG_DATA_TYPE_SIGNATURE,
                                UAVCAN_MY_MSG_DATA_TYPE_ID,
                                &transfer_id,
                                CANARD_TRANSFER_PRIORITY_LOWEST,
                                CanardResponse,
                                &buffer[0],
                                UAVCAN_MY_MSG_MESSAGE_SIZE);
    if (bc_res <= 0) {
        (void)fprintf(stderr, "Could not broadcast node status; error %d\n", bc_res);
    }
    else
        (void)fprintf(stderr, "my msg sent: %d\n", bc_res);
}

/**
 * Transmits all frames from the TX queue, receives up to one frame.
 */
static void processTxOnce(SocketCANInstance* socketcan)
{
    const CanardCANFrame* txf = NULL;
    
    // Transmitting
    for (; (txf = canardPeekTxQueue(&g_canard)) != NULL;)
    {
        const int16_t tx_res = socketcanTransmit(socketcan, txf, 0);
        if (tx_res < 0)         // Failure - drop the frame and report
        {
            canardPopTxQueue(&g_canard);
            (void)fprintf(stderr, "Transmit error %d, frame dropped, errno '%s'\n", tx_res, strerror(errno));
        }
        else if (tx_res > 0)    // Success - just drop the frame
        {
            canardPopTxQueue(&g_canard);
            
            // send only one
            break;
        }
        else                    // Timeout - just exit and try again later
        {
            break;
        }
    }
}

static void processRxOnce(SocketCANInstance* socketcan) {
    // Receiving
    CanardCANFrame rx_frame;
    const uint64_t timestamp = getMonotonicTimestampUSec();
    const int16_t rx_res = socketcanReceive(socketcan, &rx_frame, 0);
    if (rx_res < 0)             // Failure - report
    {
        (void)fprintf(stderr, "Receive error %d, errno '%s'\n", rx_res, strerror(errno));
    }
    else if (rx_res > 0)        // Success - process the frame
    {
        canardHandleRxFrame(&g_canard, &rx_frame, timestamp);
    }
    else
    {
        ;                       // Timeout - nothing to do
    }
}


int main(int argc, char** argv)
{
    if (argc < 4)
    {
        (void)fprintf(stderr,
                      "Usage:\n"
                      "\t%s <can iface name> <src_id> <dest_id>\n",
                      argv[0]);
        return 1;
    }

    /*
     * Initializing the CAN backend driver; in this example we're using SocketCAN
     */
    SocketCANInstance socketcan;
    const char* const can_iface_name = argv[1];
    int16_t res = socketcanInit(&socketcan, can_iface_name);
    if (res < 0)
    {
        (void)fprintf(stderr, "Failed to open CAN iface '%s'\n", can_iface_name);
        return 1;
    }

    /*
     * Initializing the Libcanard instance.
     */
    canardInit(&g_canard,
               g_canard_memory_pool,
               sizeof(g_canard_memory_pool),
               onTransferReceived,
               shouldAcceptTransfer,
               NULL);

    const char* const can_node_id = argv[2];
    const char* const can_dest_id = argv[3];
    
    node_id=atoi(can_node_id);
    dest_id=atoi(can_dest_id);

    canardSetLocalNodeID(&g_canard, node_id);
    printf("Dynamic node ID allocation complete [%d]\n", canardGetLocalNodeID(&g_canard));

    // skarnet
    const tain_t tto = { .sec=1, .nano=0 };
    tain_t deadline;
    
    tain_now_g();
    tain_add_g(&deadline, &tto) ;

    for(;;) {
        iopause_fd x[2];
        
        x[0].fd = socketcanGetSocketFileDescriptor(&socketcan);
        x[0].events = IOPAUSE_READ | (canardPeekTxQueue(&g_canard)?IOPAUSE_WRITE:0);
        
        int r=iopause_g(x, 1, &deadline);
        
        if(r<0) {
            fprintf(stderr, "iopause error");
        }
        else if(!r) {
            send_mymsg();
            tain_add_g(&deadline, &tto) ;
        }
        else {
            if(x[0].revents & IOPAUSE_WRITE) {
                processTxOnce(&socketcan);
            }
            else if(x[0].revents & IOPAUSE_READ) {
                processRxOnce(&socketcan);
            }
            
            
        }
        
    }
    
    return 0;
}
