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
 * Test Name: aql_barrier_negative_value
 * Scope: Conformance
 *
 * Purpose: Verifies that a Barrier packet allows an
 * application to specify up to five signal dependencies
 * and, if any of the dependent signal values become negative,
 * the packet processor assigns an error value to the completion
 * signal.
 *
 * Test Description:
 * 1) Generate a list of agents that support the HSA_QUEUE_FEATURE_DISPATCH
 * queue feature.
 * 2) Select an agent from the list and create a queue.
 * 3) Create 6 signals associated with the agent (or all agents), and
 * initialize all signals to have a value of 1.
 * 4) Enqueue a barrier packet to the queue and use 5 of the signals
 * as dependencies.
 * 5) The final signal should be used as the barrier packets completion signal.
 * 6) Set one of the barrier's dependant signal values to -1.
 * 7) Wait for the barrier packet's completion signal's value to become
 * negative, indicating that the packet processor has assigned a correct
 * error value.
 * 8) Repeat the test using a different signal in the dep_signal array. All
 * indexes should be checked.
 *
 * Expected results: When any of the dependency signal's value becomes
 * negative, the completion signal of the barrier packet should indicate
 * that an error occured.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>

int test_aql_barrier_negative_value() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        int jj;
        // Create six signals
        hsa_signal_t signals[6];
        for (jj = 0; jj < 6; ++jj) {
            status = hsa_signal_create((hsa_signal_value_t)1, 0, NULL, &signals[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        const size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);

        // Repeat on setting the kk-th signal to execute the barrier
        int kk;
        for (kk = 0; kk < 5; ++kk) {
            // Create the queue
            hsa_queue_t* queue;
            status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Create and enqueue a barrier packet
            uint64_t packet_id = hsa_queue_add_write_index_relaxed(queue, 1);
            hsa_barrier_and_packet_t* barrier_packet = (hsa_barrier_and_packet_t*)(queue->base_address + packet_id * packet_size);
            memset(barrier_packet, 0, packet_size);
            barrier_packet->completion_signal = signals[5];

            // Add dependency signals to the barrier packet
            for (jj = 0; jj < 5; ++jj) {
                barrier_packet->dep_signal[jj] = signals[jj];
            }

            uint16_t header = 0;
            header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
            header |= HSA_PACKET_TYPE_BARRIER_AND << HSA_PACKET_HEADER_TYPE;
            __atomic_store_n((uint16_t*)(&barrier_packet->header), header, __ATOMIC_RELEASE);

            // Signal the door bell to launch the barrier packet
            hsa_signal_store_release(queue->doorbell_signal, packet_id);

            // Assign a negative value to the kk-th signal
            hsa_signal_store_release(signals[kk], -1);

            // The barrier packet's completion signal should be set to a
            // negative value by the runtime.
            // Wait for the negative value on the 5th signal
            hsa_signal_wait_relaxed(signals[5], HSA_SIGNAL_CONDITION_LT, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

            // Reinitialize the signals for next iteration
            // Decrement signal 5
            hsa_signal_store_release(signals[5], 1);
            // Decrement signal kk
            hsa_signal_store_release(signals[kk], 1);

            // Destroy the queue
            status = hsa_queue_destroy(queue);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Destroy all signals
        for (jj = 0; jj < 6; ++jj) {
            status = hsa_signal_destroy(signals[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }
    }

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
