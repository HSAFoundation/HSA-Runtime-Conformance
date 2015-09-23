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
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

void launch_memory_atomic_kernel(
        hsa_agent_t* agent,
        hsa_queue_t* queue,
        symbol_record_t* symbol_record,
        void* data,
        void* value,
        int num_kernel_instances) {
    hsa_status_t status;
    kernarg_memory_atomic_t arg;

    // Find a memory region that supports kernel arguments
    hsa_region_t kernarg_region;
    hsa_agent_iterate_regions(*agent, get_kernarg_memory_region, &kernarg_region);
    ASSERT(0 != kernarg_region.handle);

    // Allocate the kernel argument buffer from the correct region
    kernarg_memory_atomic_t* kernarg_buffer = NULL;
    status = hsa_memory_allocate(kernarg_region, sizeof(arg), (void**)(&kernarg_buffer));
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Setup the kernarg
    arg.data = data;
    arg.value = value;
    memcpy(kernarg_buffer, &arg, sizeof(arg));

    // Create the signal with initial value of "num_kernel_instances"
    hsa_signal_t signal;
    status = hsa_signal_create((hsa_signal_value_t)1, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Request a new packet ID
    uint64_t packet_id = hsa_queue_add_write_index_acquire(queue, 1);

    // Holding on not to write any new packet to the queue if the queue is full.
    while (packet_id - hsa_queue_load_read_index_relaxed(queue) >= queue->size) {}

    // Compute packet offset
    hsa_kernel_dispatch_packet_t* dispatch_packet = (hsa_kernel_dispatch_packet_t*)queue->base_address
            + packet_id % queue->size;

    // Initialize the packet
    memset(dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));

    // Initialize the packet
    dispatch_packet->completion_signal = signal;
    dispatch_packet->setup  |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    dispatch_packet->workgroup_size_x = num_kernel_instances;
    dispatch_packet->workgroup_size_y = 1;
    dispatch_packet->workgroup_size_z = 1;
    dispatch_packet->grid_size_x = num_kernel_instances;
    dispatch_packet->grid_size_y = 1;
    dispatch_packet->grid_size_z = 1;
    dispatch_packet->kernel_object = symbol_record->kernel_object;
    dispatch_packet->kernarg_address = (void*) kernarg_buffer;
    dispatch_packet->header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

    uint16_t header = 0;
    header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

    __atomic_store_n((uint16_t*)(&dispatch_packet->header), header, __ATOMIC_RELEASE);

    // Signal the door bell to launch the packet
    hsa_signal_store_release(queue->doorbell_signal, packet_id);

    // Wait until all the kernels are complete
    while (0 != hsa_signal_wait_relaxed(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED)) {}

    // Destroy the signal
    hsa_signal_destroy(signal);

    status = hsa_memory_free(kernarg_buffer);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}
