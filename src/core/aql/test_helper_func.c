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
#include <stdio.h>
#include "test_helper_func.h"

// Clear the data, launch the kernel, and wait for the execution to complete
void launch_kernel(
        hsa_queue_t* queue,
        uint32_t* data,
        uint32_t total_size,
        uint32_t value,
        int dim,
        hsa_dim3_t grid_dim,
        hsa_dim3_t workgroup_dim,
        uint64_t kernel_obj_address,
        void* kernarg_address) {
    hsa_status_t status;

    // Clear the data
    memset(data, 0, sizeof(uint32_t) * total_size);

    // Setup the kernarg data structure
    kernarg_t args;
    args.data = data;
    args.value = value;
    args.row_pitch = grid_dim.x;
    args.slice_pitch = grid_dim.x * grid_dim.y;
    memcpy((void*)kernarg_address, &args, sizeof(args));

    // Create a signal with initial value of 1
    hsa_signal_t signal;
    status = hsa_signal_create(1, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Request a new packet ID
    uint64_t packet_id = hsa_queue_add_write_index_acquire(queue, 1);

    while (packet_id - hsa_queue_load_read_index_relaxed(queue) >= queue->size) {}
    hsa_kernel_dispatch_packet_t* dispatch_packet = (hsa_kernel_dispatch_packet_t*)queue->base_address
            + packet_id % queue->size;

    // Initialize the packet
    memset(dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
    dispatch_packet->completion_signal = signal;
    dispatch_packet->setup = dim << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    dispatch_packet->workgroup_size_x = (uint16_t)workgroup_dim.x;
    dispatch_packet->workgroup_size_y = (uint16_t)workgroup_dim.y;
    dispatch_packet->workgroup_size_z = (uint16_t)workgroup_dim.z;
    dispatch_packet->grid_size_x = grid_dim.x;
    dispatch_packet->grid_size_y = grid_dim.y;
    dispatch_packet->grid_size_z = grid_dim.z;
    dispatch_packet->kernel_object = kernel_obj_address;
    dispatch_packet->kernarg_address = (void*) kernarg_address;

    uint16_t header = 0;
    header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

    __atomic_store_n((uint16_t*)(&dispatch_packet->header), header, __ATOMIC_RELEASE);

    // Signal the door bell to launch the packet
    hsa_signal_store_release(queue->doorbell_signal, packet_id);

    // Wait until the kernel completes
    while (0 != hsa_signal_wait_acquire(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED)) {}

    // Destroy the signal
    hsa_signal_destroy(signal);

    return;
}


void launch_memory_kernel(
        hsa_queue_t* queue,
        void* in,
        void* out,
        uint32_t data_size,
        uint32_t private_memory_size,
        uint32_t group_memory_size,
        uint64_t kernel_obj_address,
        void* kernarg_address) {
    hsa_status_t status;

    // Clear the data
    memset(out, 0, sizeof(int) * data_size);

    // Setup the kernarg
    kernarg_memory_t args;
    args.in = in;
    args.out = out;
    args.count = data_size;
    memcpy((void*)kernarg_address, &args, sizeof(args));

    // Create a signal with initial value of 1
    hsa_signal_t signal;
    status = hsa_signal_create(1, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Request a new packet ID
    uint64_t packet_id = hsa_queue_add_write_index_acquire(queue, 1);

    while (packet_id - hsa_queue_load_read_index_relaxed(queue) >= queue->size) {}

    hsa_kernel_dispatch_packet_t* dispatch_packet = (hsa_kernel_dispatch_packet_t*)queue->base_address
            + packet_id % queue->size;

    // Initialize the packet
    memset(dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
    dispatch_packet->completion_signal = signal;
    dispatch_packet->header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    dispatch_packet->header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    dispatch_packet->setup = 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    dispatch_packet->workgroup_size_x = (uint16_t)data_size;
    dispatch_packet->workgroup_size_y = (uint16_t)1;
    dispatch_packet->workgroup_size_z = (uint16_t)1;
    dispatch_packet->grid_size_x = (uint16_t)data_size;
    dispatch_packet->grid_size_y = 1;
    dispatch_packet->grid_size_z = 1;
    dispatch_packet->private_segment_size = private_memory_size;
    dispatch_packet->group_segment_size = group_memory_size;
    dispatch_packet->kernel_object = kernel_obj_address;
    dispatch_packet->kernarg_address = (void*) kernarg_address;

    uint16_t header = 0;
    header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

    __atomic_store_n((uint16_t*)(&dispatch_packet->header), header, __ATOMIC_RELEASE);

    // Signal the door bell to launch the packet
    hsa_signal_store_release(queue->doorbell_signal, packet_id);

    // Wait until the kernel complete
    while (0 != hsa_signal_wait_acquire(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED)) {}

    hsa_signal_destroy(signal);

    return;
}
