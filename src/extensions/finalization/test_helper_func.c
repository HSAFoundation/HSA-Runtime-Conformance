/*
 * =============================================================================
 *   HSA Runtime Conformance Release License
 * =============================================================================
 * The University of Illinois/NCSA
 * Open Source License (NCSA)
 *
 * Copyright (c) 2014, Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Developed by:
 *
 *                 AMD Research and AMD HSA Software Development
 *
 *                 Advanced Micro Devices, Inc.
 *
 *                 www.amd.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimers.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimers in
 *    the documentation and/or other materials provided with the distribution.
 *  - Neither the names of <Name of Development Group, Name of Institution>,
 *    nor the names of its contributors may be used to endorse or promote
 *    products derived from this Software without specific prior written
 *    permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE.
 *
 */

#include <hsa.h>
#include <framework.h>
#include <stdlib.h>
#include <unistd.h>
#include "test_helper_func.h"

void launch_kernel_no_kernarg(hsa_queue_t* queue, uint64_t kernel_object, int num_packets) {
    hsa_status_t status;

    // Signal and dispatch packet
    hsa_signal_t* signals = (hsa_signal_t*) malloc(sizeof(hsa_signal_t) * num_packets);
    hsa_kernel_dispatch_packet_t dispatch_packet;

    int jj;
    for (jj = 0; jj < num_packets; ++jj) {
        status = hsa_signal_create(1, 0, NULL, &signals[jj]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Get size of dispatch_packet
    const size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);

    // Fill info for the default dispatch_packet
    memset(&dispatch_packet, 0, packet_size);
    dispatch_packet.header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    dispatch_packet.header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    dispatch_packet.setup |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    dispatch_packet.workgroup_size_x = 256;
    dispatch_packet.workgroup_size_y = 1;
    dispatch_packet.workgroup_size_z = 1;
    dispatch_packet.grid_size_x = 256;
    dispatch_packet.grid_size_y = 1;
    dispatch_packet.grid_size_z = 1;
    dispatch_packet.group_segment_size = 0;
    dispatch_packet.private_segment_size = 0;
    dispatch_packet.kernel_object = kernel_object;
    dispatch_packet.kernarg_address = 0;

    // Enqueue dispatch packets
    hsa_kernel_dispatch_packet_t* queue_packet;
    for (jj = 0; jj < num_packets; ++jj) {
        // Increment the write index of the queue
        uint64_t write_index = hsa_queue_add_write_index_relaxed(queue, 1);
        // Set the value fo the dispatch packet to the correct signal
        dispatch_packet.completion_signal = signals[jj];
        // Obtain the address of the queue packet entry
        queue_packet = (hsa_kernel_dispatch_packet_t*)(queue->base_address + write_index * packet_size);
        // Copy the initialized packet to the queue packet entry
        memcpy(queue_packet, &dispatch_packet, packet_size);
        // Set the queue packet entries header.type value to HSA_PACKET_TYPE_KERNEL_DISPATCH
        // This allows the command processor to process this packet.
        queue_packet->header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        // Ring the doorbell
        hsa_signal_store_relaxed(queue->doorbell_signal, write_index);
    }

    // Wait until all dispatch packets finish executing
    for (jj = 0; jj < num_packets; ++jj) {
        hsa_signal_value_t value = hsa_signal_wait_relaxed(signals[jj], HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
        ASSERT(0 == value);
    }

    // Destroy signals
    for (jj = 0; jj < num_packets; ++jj) {
        status = hsa_signal_destroy(signals[jj]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    free(signals);

    return;
}
