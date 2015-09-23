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

/*
 * Test Name: queue_callback
 * Scope: Conformance
 *
 * Purpose: Verifies that several queues associated with
 * a single agent can have a callback associated with them,
 * and that asynchronous errors detected in queue the will properly
 * trigger the callback.
 *
 * Test Description:
 * 1) Construct a list of all agents that support the
 * HSA_AGENT_FEATURE_KERNEL_DISPATCH queue feature.
 * 2) For each agent, create several queues with the same callback
 * function.
 * 3) For each queue associated with the agent:
 *    a) Write an invalid packet into the queue,
 *    using an invalid value for the packet type.
 *    b) Ring the doorbell for that queue.
 *    c) Verify that the callback executed, and that
 *    the status reported is HSA_STATUS_ERROR_INVALID_PACKET_FORMAT
 *    and that the queue id is valid.
 * 4) Delete the queues associated with the agent.
 * 5) Repeat 2 to 4 several times.
 * 6) Repeat this test for each agent.
 *
 * Expected Results: The callback function should be correctly executed
 * and the status and queue id pass to the callback should be correct.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include <queue_utils.h>

#define QUEUE_SIZE 1024

hsa_status_t global_status;
hsa_queue_t* global_queue_handle;
hsa_signal_t global_signal;

void callback(hsa_status_t status, hsa_queue_t* queue_handle, void* data) {
    global_status = status;
    global_queue_handle = queue_handle;
    hsa_signal_store_relaxed(global_signal, 1);
    return;
}

int test_queue_callback() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;

    // Get the agent list
    get_agent_list(&agent_list);

    int jj;

    for (jj = 0; jj < agent_list.num_agents; ++jj) {
        hsa_queue_feature_t feature;
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_FEATURE, &feature);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Only test on agents the support the queue dispatch feature.
        if (!(feature & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Initialize the global signal
        status = hsa_signal_create(0, 0, NULL, &global_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Query the agent properties
        uint32_t queue_max_size;
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        queue_max_size = (queue_max_size < QUEUE_SIZE) ? queue_max_size : QUEUE_SIZE;

        uint32_t queue_max;
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_QUEUES_MAX, &queue_max);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create queue_max queues for the agent.
        int ii;
        hsa_queue_t* queue[queue_max];
        for (ii = 0; ii < queue_max; ++ii) {
            status = hsa_queue_create(agent_list.agents[jj], queue_max_size, HSA_QUEUE_TYPE_SINGLE, callback, NULL, UINT32_MAX, UINT32_MAX, &queue[ii]);
            if (HSA_STATUS_SUCCESS != status) {
                ASSERT(HSA_STATUS_ERROR_OUT_OF_RESOURCES == status);
                queue_max = ii;
            }
        }

        // For each queue, write a packet with an invalid code object
        const size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);
        hsa_kernel_dispatch_packet_t dispatch_packet;
        memset(&dispatch_packet, 0, packet_size);
        dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        dispatch_packet.workgroup_size_x = 256;
        dispatch_packet.workgroup_size_y = 1;
        dispatch_packet.workgroup_size_z = 1;
        dispatch_packet.grid_size_x = 256;
        dispatch_packet.grid_size_y = 1;
        dispatch_packet.grid_size_z = 1;
        dispatch_packet.setup |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        dispatch_packet.header|= 1 << HSA_PACKET_HEADER_BARRIER;

        hsa_kernel_dispatch_packet_t* queue_packet;
        for (ii = 0; ii < queue_max; ++ii) {
            // Initialize the global variables
            global_status = HSA_STATUS_SUCCESS;
            global_queue_handle = 0;
            hsa_signal_store_relaxed(global_signal, 0);

            // Dispatch the packet.
            enqueue_dispatch_packet(queue[ii], &dispatch_packet);

            // Wait on the global signal value to change to 1
            while (1 != hsa_signal_wait_relaxed(global_signal, HSA_SIGNAL_CONDITION_EQ, 1, UINT64_MAX, HSA_WAIT_STATE_BLOCKED)) {}

            // Verify the global_status and global_queue_handle values were set correctly
            ASSERT(global_queue_handle == queue[ii]);
            ASSERT(HSA_STATUS_ERROR_INVALID_CODE_OBJECT == global_status);
        }

        // Destroy the queues
        for (ii = 0; ii < queue_max; ++ii) {
            status = hsa_queue_destroy(queue[ii]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Destroy the global signal
        status = hsa_signal_destroy(global_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    free_agent_list(&agent_list);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
