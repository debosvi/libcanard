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

/*
 * Copyright (c) 2016-2018 UAVCAN Team
 *
 * Distributed under the MIT License, available in the file LICENSE.
 *
 */

#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#include <libcanard.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    int fd;
} socketcan_drv_t;

/**
 * Initializes the SocketCAN instance.
 * Returns 0 on success, negative on error.
 */
int16_t socketcanInit(socketcan_drv_t* out_ins, const char* can_iface_name);

/**
 * Deinitializes the SocketCAN instance.
 * Returns 0 on success, negative on error.
 */
int16_t socketcanClose(socketcan_drv_t* ins);

/**
 * Transmits a CAN_frame_t to the CAN socket.
 * Use negative timeout to block infinitely.
 * Returns 1 on successful transmission, 0 on timeout, negative on error.
 */
int16_t socketcanTransmit(socketcan_drv_t* ins, const CAN_frame_t* frame, int32_t timeout_msec);

/**
 * Receives a CAN_frame_t from the CAN socket.
 * Use negative timeout to block infinitely.
 * Returns 1 on successful reception, 0 on timeout, negative on error.
 */
int16_t socketcanReceive(socketcan_drv_t* ins, CAN_frame_t* out_frame, int32_t timeout_msec);

/**
 * Returns the file descriptor of the CAN socket.
 * Can be used for external IO multiplexing.
 */
int socketcanGetSocketFileDescriptor(const socketcan_drv_t* ins);

#ifdef __cplusplus
}
#endif

#endif
