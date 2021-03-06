// The MIT License (MIT)

// Copyright (c) 2019 Vincent de RIBOU <v.deribou@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __LIBCANARD_H__
#define __LIBCANARD_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This data type holds a standard CAN 2.0B data frame with 29-bit ID.
 */
typedef struct
{
    /**
     * Refer to the following definitions:
     *  - CANARD_CAN_FRAME_EFF
     *  - CANARD_CAN_FRAME_RTR
     *  - CANARD_CAN_FRAME_ERR
     */
    uint32_t id;
    uint8_t data[8];
    uint8_t data_len;
} CAN_frame_t;
    
/**
 * Transfer types are defined by the UAVCAN specification.
 */
typedef enum
{
    TRANSFER_RESPONSE  = 0,
    TRANSFER_REQUEST,
    TRANSFER_BROADCAST,
    TRANSFER_COUNT
} canard_transfer_t;

typedef uint16_t canard_id_type_t;
typedef uint64_t canard_hash_type_t;
typedef size_t canard_length_type_t;

typedef struct {
    const canard_transfer_t type;
    const canard_hash_type_t hash;
    const canard_id_type_t id;
    const canard_length_type_t lg;
    void* const buf;
} canard_item_t;

typedef void (*onMessageReceived)(
    const canard_id_type_t id,     
    const void* const buf,
    const canard_length_type_t lg,
    const void* const priv); 

// typedef enum {
//     CANARD_DRV_SOCKET=0,
//     CANARD_DRV_COUNT,
//     CANARD_DRV_INVALID=255
// } canard_driver_t;    
//     
// typedef struct {
//     const canard_driver_t drv;
//     const uint16_t node_id;
//     const canard_item_t* const items;
//     const onMessageReceived omsg;
// } canard_node_t;

#ifdef __cplusplus
}
#endif

#endif // __LIBCANARD_H__
