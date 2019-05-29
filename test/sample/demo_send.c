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

#define UAVCAN_MY_MSG_MESSAGE_SIZE              20
#define UAVCAN_MY_MSG_DATA_TYPE_ID              49876
#define UAVCAN_MY_MSG_DATA_TYPE_SIGNATURE       0x5A5A5A5A5A5A5A5A

#define UAVCAN_MY_MSG2_MESSAGE_SIZE             7
#define UAVCAN_MY_MSG2_DATA_TYPE_ID             65
#define UAVCAN_MY_MSG2_DATA_TYPE_SIGNATURE      0x1234567890987654

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
static unsigned char mymsg2_buf[UAVCAN_MY_MSG2_MESSAGE_SIZE];

static const canard_item_t canard_storage[] = {
    { .type = CanardTransferTypeResponse, .hash = UAVCAN_MY_MSG2_DATA_TYPE_SIGNATURE, .id = UAVCAN_MY_MSG2_DATA_TYPE_ID, .lg = UAVCAN_MY_MSG2_MESSAGE_SIZE, .buf = mymsg2_buf },
    { .type = CanardTransferTypeBroadcast, .hash = UAVCAN_MY_MSG_DATA_TYPE_SIGNATURE, .id = UAVCAN_MY_MSG_DATA_TYPE_ID, .lg = UAVCAN_MY_MSG_MESSAGE_SIZE, .buf = mymsg_buf },
    { .hash = 0, .id = 0, .lg = 0, .buf = 0}    
};

/**
 * iopause reverse indx management.
 */
typedef struct {
    unsigned int x_index;
    const onMessageReceived on_msg;
    void *priv;
} iopause_item_t;

static void g_omr(const canard_id_type_t id, 
           const void const *buf, const canard_length_type_t lg, 
           const void const* priv) {
    
    fprintf(stderr, "1: message ID '%u'\n", id);
    fprintf(stderr, "1: message pointer '%p'\n", priv);
    fprintf(stderr, "1: message length '%u'\n", lg);
    
    unsigned int i=0;
    for(; i<lg; i++) {
        fprintf(stderr, "0x%02X ", *(char*)(buf+i));
    }
    fprintf(stderr, "\n");
                
}

static void g_omr2(const canard_id_type_t id, 
           const void const *buf, const canard_length_type_t lg, 
           const void const* priv) {
    
    fprintf(stderr, "2: message ID '%u'\n", id);
    fprintf(stderr, "2: message pointer '%p'\n", priv);
    fprintf(stderr, "2: message length '%u'\n", lg);
    
    unsigned int i=0;
    for(; i<lg; i++) {
        fprintf(stderr, "0x%02X ", *(char*)(buf+i));
    }
    fprintf(stderr, "\n");
                
}

static uint64_t getMonotonicTimestampUSec(void) {
    struct timespec ts;    
    timespec_from_tain (&ts, &STAMP);    
    return (uint64_t)((ts.tv_sec * 1000000LL) + (ts.tv_nsec / 1000LL));
}

/**
 * This callback is invoked by the library when a new message or request or response is received.
 */
static void onTransferReceived(CanardInstance* ins,
                               CanardRxTransfer* transfer, 
                               const void* const item
                              )
{

    if (transfer->transfer_type == CanardTransferTypeResponse) {
//         fprintf(stderr, "onTransferReceived transfer Response\n");
    }
    else if (transfer->transfer_type == CanardTransferTypeRequest) {
//         fprintf(stderr, "onTransferReceived transfer Request\n");
    }
    else if (transfer->transfer_type == CanardTransferTypeBroadcast) {
//         fprintf(stderr, "onTransferReceived transfer Broadcast\n");
    }
    else {
        fprintf(stderr, "onTransferReceived transfer Unknown\n");
    }
    
//     fprintf(stderr, "onTransferReceived data type %d\n", transfer->data_type_id);
    
    canard_item_t* c_item=(canard_item_t*)&canard_storage[0];
        
    for(;;) {
        if(!c_item->hash && !c_item->id) break;
        
        if ((transfer->transfer_type == c_item->type) &&
            (transfer->data_type_id == c_item->id)) {
            
            unsigned int lg=transfer->payload_len;
            if(lg>c_item->lg) {
                fprintf(stderr, "payload exceeds message capability\n");
                lg=c_item->lg;
            }
            else if(lg<c_item->lg) {
                fprintf(stderr, "payload length less than expected\n");
            }
            
            if(c_item->buf) {
                unsigned int i=0;
                for(; i<lg; i++) {
                    char c;
                    int16_t r = canardDecodeScalar(transfer, i*8U, 8U, true, &c);  
                    if(r<8U)   
                        fprintf(stderr, "partial byte not retrieved\n");
                    else {
                        *(char*)(c_item->buf+i) = c;
                    }
                }
                
                const iopause_item_t* const p=(iopause_item_t*)item;
                p->on_msg(c_item->id, c_item->buf, lg, p->priv);
            }
            return;
        }    
        c_item++;
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
    fprintf(stderr, "%s: data ID %d\n",  __PRETTY_FUNCTION__, data_type_id);
    (void)source_node_id;

    if (canardGetLocalNodeID(ins) == CANARD_BROADCAST_NODE_ID) {
        fprintf(stderr, "node ID not set");
    }
    else {
        canard_item_t* item=(canard_item_t*)&canard_storage[0];
        
        for(;;) {
            if(!item->hash && !item->id) break;
            
            if ((transfer_type == item->type) && (data_type_id == item->id)) {
                *out_data_type_signature = item->hash;
                return true;
            }
            
            item++;
        }
    }

    return false;
}

void send_mymsg(const unsigned int idx) {
    int16_t bc_res=0;    
    canard_item_t const *item=&canard_storage[idx];
    const canard_length_type_t lg=item->lg;
    uint8_t buffer[lg];
    for(int i=0; i<lg; i++)
        buffer[i] = node_id+i;
    
    static uint8_t transfer_id;  // Note that the transfer ID variable MUST BE STATIC (or heap-allocated)!

    if( (item->type == CanardTransferTypeResponse) ||
        (item->type == CanardTransferTypeRequest) ) {
        bc_res = canardRequestOrRespond(&g_canard,
                dest_id,
                item->hash, 
                item->id,
                &transfer_id,
                CANARD_TRANSFER_PRIORITY_LOWEST,
                item->type,
                &buffer[0],
                item->lg);
    }
    else if(item->type == CanardTransferTypeBroadcast) {
        bc_res = canardBroadcast(&g_canard,
                item->hash, 
                item->id,
                &transfer_id,
                CANARD_TRANSFER_PRIORITY_LOWEST,
                &buffer[0],
                item->lg);
    }
    
    if (bc_res <= 0) {
        (void)fprintf(stderr, "Could not broadcast node status; error %d\n", bc_res);
    }
//     else
//         (void)fprintf(stderr, "my msg sent: %d\n", bc_res);
}

/**
 * Transmits all frames from the TX queue, receives up to one frame.
 */
static void processTxOnce(SocketCANInstance* socketcan) {
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

static void processRxOnce(SocketCANInstance* socketcan, iopause_item_t *item) {
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
        canardHandleRxFrame(&g_canard, &rx_frame, timestamp, item);
    }
    else
    {
        ;                       // Timeout - nothing to do
    }
}

iopause_item_t g_io_items[] = {
    { .x_index=-1, .on_msg=g_omr, .priv=mymsg_buf },
    { .x_index=-1, .on_msg=g_omr2, .priv=mymsg2_buf }
};

int main(int argc, char** argv)
{
    if (argc < 5) {
        (void)fprintf(stderr,
                      "Usage:\n"
                      "\t%s <can iface name> <src_id> <dest_id> <idx>\n",
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

    g_io_items[0].x_index=0;
    
    for(;;) {
        iopause_fd x[2];
        
        x[0].fd = socketcanGetSocketFileDescriptor(&socketcan);
        x[0].events = IOPAUSE_READ | (canardPeekTxQueue(&g_canard)?IOPAUSE_WRITE:0);
        
        int r=iopause_g(x, 1, &deadline);
        
        if(r<0) {
            fprintf(stderr, "iopause error");
        }
        else if(!r) {
            send_mymsg(atoi(argv[4]));
            tain_add_g(&deadline, &tto) ;
        }
        else {
            if(x[g_io_items[0].x_index].revents & IOPAUSE_WRITE) {
                processTxOnce(&socketcan);
            }
            else if(x[g_io_items[0].x_index].revents & IOPAUSE_READ) {
                processRxOnce(&socketcan, &g_io_items[0]);
            }
            
            
        }
        
    }
    
    return 0;
}
