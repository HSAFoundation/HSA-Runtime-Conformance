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
 * Test Name: aql_barrier_cross_queue_dependency
 * Scope: Conformance
 *
 * Purpose: Verifies that that barrier packets can utilize
 * dependency signals across dispatch queues.
 *
 * Test Description:
 * 1) Generate a list of agents that support the HSA_QUEUE_FEATURE_DISPATCH
 * queue feature.
 * 2) If the agent list has less than two agents the test cannot be run. Pass
 * the test.
 * 3) Create a queue for each agent in the list.
 * 4) If n is the number of agents, create n+1 signals, initialized to 1.
 * 5) For each agent, identified with an index i, enqueue a barrier packet
 * using signal i as the dependency signal and i+1 as the completion signal.
 * The final signal will not be a dependency signal but will be the final
 * test completion signal.
 * 6) Check the final signal's value and make sure it hasn't been modified before
 * the first signal's value is set.
 * 7) Set the value of the first signal to 0.
 * 8) Wait on the final signal until its value is 0.
 * 9) Check all of the intermediate signals and verify that they have a signal
 * value of 0.
 *
 * Expected results: Modifying the first signal's value to 0 should propagate the
 * change to all barrier packet values.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>

int test_aql_barrier_cross_queue_dependency() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Collect all the dispatch agents
    int num_dispatch_agents = 0;
    hsa_agent_t dispatch_agents[agent_list.num_agents];
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        dispatch_agents[num_dispatch_agents] = agent_list.agents[ii];
        ++num_dispatch_agents;
    }

    // Skip this test case if the number of dispatch agents is less than 2
    if (num_dispatch_agents < 2) {
        // Number of dispatch agents is less than 2, skip Cross Dependency test
        free_agent_list(&agent_list);
        return 0;
    }

    // Create signals
    hsa_signal_t signals[num_dispatch_agents + 1];
    for (ii = 0; ii < num_dispatch_agents + 1; ++ii) {
        status = hsa_signal_create((hsa_signal_value_t)1, 0, NULL, &signals[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Create a queue on each dispatch agent, enqueue a barrier on this queue
    hsa_queue_t* queues[num_dispatch_agents];
    for (ii = 0; ii < num_dispatch_agents; ++ii) {
        // Create a queue
        status = hsa_queue_create(dispatch_agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &(queues[ii]));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Enqueue a barrier
        const size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);
        uint64_t packet_id = hsa_queue_add_write_index_relaxed(queues[ii], 1);
        hsa_barrier_and_packet_t* barrier_packet = (hsa_barrier_and_packet_t*)(queues[ii]->base_address + packet_id * packet_size);
        memset(barrier_packet, 0, packet_size);
        // Set the dependency signal
        barrier_packet->dep_signal[0] = signals[ii];
        // Set the completion signal
        barrier_packet->completion_signal = signals[ii + 1];

        uint16_t header = 0;
        header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        header |= HSA_PACKET_TYPE_BARRIER_AND << HSA_PACKET_HEADER_TYPE;

        __atomic_store_n((uint16_t*)(&barrier_packet->header), header, __ATOMIC_RELEASE);

        // Signal the door bell to launch the barrier packet
        hsa_signal_store_release(queues[ii]->doorbell_signal, packet_id);
    }

    // Verify none of the signals has been changed
    for (ii = 0; ii < num_dispatch_agents+1; ++ii) {
        int signal_value = (int)hsa_signal_load_relaxed(signals[ii]);
        ASSERT(1 == signal_value);
    }

    // Trigger the barrier in the 1st queue
    hsa_signal_store_release(signals[0], 0);

    // Wait for all barriers to complete
    while (0 != hsa_signal_wait_relaxed(signals[num_dispatch_agents], HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED));

    // Verify all signals have been updated
    for (ii = 0; ii < num_dispatch_agents+1; ++ii) {
        int signal_value = (int)hsa_signal_load_relaxed(signals[ii]);
        ASSERT(0 == signal_value);
    }

    // Destroy all signals
    for (ii = 0; ii < num_dispatch_agents+1; ++ii) {
        status = hsa_signal_destroy(signals[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Destroy all queues
    for (ii = 0; ii < num_dispatch_agents; ++ii) {
        status = hsa_queue_destroy(queues[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
