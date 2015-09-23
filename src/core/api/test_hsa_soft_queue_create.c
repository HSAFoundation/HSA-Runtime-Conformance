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

/**
 *
 * Test Name: hsa_queue_create
 * Purpose: Verify that API of hsa_queue_create() works as expected
 *
 * Description:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <hsa.h>
#include <framework.h>
#include <agent_utils.h>
#include "test_helper_func.h"


int test_hsa_soft_queue_create() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    const uint32_t queue_size = 32;

    // Find an agent that supports agent dispatch
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(get_cpu_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Find memory region that is accessible from this agent
    hsa_region_t region;
    region.handle = (uint64_t)-1;
    status = hsa_agent_iterate_regions(agent,
                callback_get_region_global_allocatable, &region);

    // Use this region to create a soft queue
    // no error should occur
    hsa_queue_t* queue;
    hsa_signal_t doorbell_signal;
    doorbell_signal.handle = (uint64_t)-1;
    status = hsa_signal_create(0, 0, NULL, &doorbell_signal);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_soft_queue_create(region, queue_size,
        HSA_QUEUE_TYPE_SINGLE, HSA_QUEUE_FEATURE_KERNEL_DISPATCH,
        doorbell_signal, &queue);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Verify all the queue members are consistent with the way it was created:
    // size, type, features, doorbell_signal
    ASSERT(queue->size == queue_size);
    ASSERT(queue->type == HSA_QUEUE_TYPE_SINGLE);
    ASSERT(queue->features & HSA_QUEUE_FEATURE_KERNEL_DISPATCH);
    ASSERT(0 == (queue->features & HSA_QUEUE_FEATURE_AGENT_DISPATCH));
    ASSERT(queue->doorbell_signal.handle == doorbell_signal.handle);
    // Verify that both read index and write index are initialized to 0
    uint64_t read_index = hsa_queue_load_read_index_relaxed(queue);
    ASSERT(0 == read_index);
    uint64_t write_index = hsa_queue_load_write_index_relaxed(queue);
    ASSERT(0 == write_index);
    // Verify that the header of each packet that is reserved by the queue is
    // initialized as HSA_PACKET_TYPE_INVALID
    uint16_t packet_type_bits_mask = (1 << HSA_PACKET_HEADER_WIDTH_TYPE) - 1;
    hsa_kernel_dispatch_packet_t* packets = (hsa_kernel_dispatch_packet_t*)queue->base_address;
    uint32_t i;
    for (i = 0; i < queue_size; ++i) {
        uint16_t packet_type = packets[i].header & packet_type_bits_mask;
        ASSERT(HSA_PACKET_TYPE_INVALID == packet_type);
    }
    // Destroy the soft queue
    status = hsa_queue_destroy(queue);


    // Create a soft queue with invalid parameters:
    // (1) queue_size is not power of 2
    status = hsa_soft_queue_create(region, queue_size + 1,
        HSA_QUEUE_TYPE_SINGLE, HSA_QUEUE_FEATURE_KERNEL_DISPATCH,
        doorbell_signal, &queue);
    ASSERT(HSA_STATUS_ERROR_INVALID_ARGUMENT == status);

    // (2) queue_size is 0;
    status = hsa_soft_queue_create(region, 0,
        HSA_QUEUE_TYPE_SINGLE, HSA_QUEUE_FEATURE_KERNEL_DISPATCH,
        doorbell_signal, &queue);
    ASSERT(HSA_STATUS_ERROR_INVALID_ARGUMENT == status);

    // (3) type is an invalid type
    // status = hsa_soft_queue_create(region, queue_size
    // HSA_QUEUE_TYPE_SINGLE + 1, HSA_QUEUE_FEATURE_KERNEL_DISPATCH,
    // doorbell_signal, &queue);
    // ASSERT(HSA_STATUS_ERROR_INVALID_ARGUMENT == status);

    // (4) doorbell handle is 0
    hsa_signal_t invalid_signal;
    invalid_signal.handle = 0;
    status = hsa_soft_queue_create(region, queue_size,
        HSA_QUEUE_TYPE_SINGLE, HSA_QUEUE_FEATURE_KERNEL_DISPATCH,
        invalid_signal, &queue);
    ASSERT(HSA_STATUS_ERROR_INVALID_ARGUMENT == status);

    // (5) queue is NULL
    status = hsa_soft_queue_create(region, queue_size,
        HSA_QUEUE_TYPE_SINGLE, HSA_QUEUE_FEATURE_KERNEL_DISPATCH,
        doorbell_signal, NULL);
    ASSERT(HSA_STATUS_ERROR_INVALID_ARGUMENT == status);

    // Create soft queue until out-of-resource
    uint32_t queues_max = 65536;
    hsa_queue_t** queues = (hsa_queue_t**)malloc(sizeof(hsa_queue_t*) * queues_max);
    uint32_t queues_created = 0;
    for (i = 0; i < queues_max; ++i) {
        status = hsa_soft_queue_create(region, queue_size,
            HSA_QUEUE_TYPE_SINGLE, HSA_QUEUE_FEATURE_KERNEL_DISPATCH,
            doorbell_signal, &queues[i]);
        if (HSA_STATUS_SUCCESS == status) {
            ++queues_created;
        } else if (HSA_STATUS_ERROR_OUT_OF_RESOURCES != status) {
            // unexpected error occurred
            ASSERT(0);
        } else {}
    }

    for (i = 0; i < queues_created; ++i) {
        status = hsa_queue_destroy(queues[i]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    free(queues);

    status = hsa_signal_destroy(doorbell_signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}
