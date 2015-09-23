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
#include <queue_utils.h>
#include "test_async_utils.h"

hsa_status_t global_status;
hsa_queue_t* global_queue_handle;
hsa_signal_t global_signal;

void async_callback(hsa_status_t status, hsa_queue_t* queue_handle, void* data) {
    global_status = status;
    global_queue_handle = queue_handle;
    hsa_signal_store_relaxed(global_signal, 1);
    return;
}

void async_test(hsa_agent_t agent, hsa_kernel_dispatch_packet_t* dispatch_packet, hsa_status_t expected_status) {
    // Initialize the global signal
    hsa_status_t status = hsa_signal_create(0, 0, NULL, &global_signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    uint32_t queue_max;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUES_MAX, &queue_max);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create queue_max queues for the agent.
    int ii;
    hsa_queue_t* queue[queue_max];
    for (ii = 0; ii < queue_max; ++ii) {
        status = hsa_queue_create(agent, 1024, HSA_QUEUE_TYPE_SINGLE, async_callback, NULL, UINT32_MAX, UINT32_MAX, &queue[ii]);
        if (HSA_STATUS_SUCCESS != status) {
            ASSERT(HSA_STATUS_ERROR_OUT_OF_RESOURCES == status);
            queue_max = ii;
        }
    }

    for (ii = 0; ii < queue_max; ++ii) {
        // Initialize the global variables
        global_status = HSA_STATUS_SUCCESS;
        global_queue_handle = 0;
        hsa_signal_store_relaxed(global_signal, 0);

        // Dispatch the packet.
        enqueue_dispatch_packet(queue[ii], dispatch_packet);

        // Wait on the global signal value to change to 1
        while (1 != hsa_signal_wait_relaxed(global_signal, HSA_SIGNAL_CONDITION_EQ, 1, UINT64_MAX, HSA_WAIT_STATE_BLOCKED)) {}

        // Verify the global_status and global_queue_handle values were set correctly
        ASSERT(global_queue_handle == queue[ii]);
        ASSERT(expected_status == global_status);
    }

    // Destroy the queues
    for (ii = 0; ii < queue_max; ++ii) {
        status = hsa_queue_destroy(queue[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Destroy the global signal
    status = hsa_signal_destroy(global_signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}
