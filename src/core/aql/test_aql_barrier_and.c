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
 * Test Name: aql_barrier_AND
 * Scope: Conformance
 *
 * Purpose: Verifies that a Barrier-AND packet allows an
 * application to specify up to five signal dependencies
 * and requires the packet processor to resolve those dependencies
 * before it can proceed.
 *
 * Test Description:
 * 1) Generate a list of agents that support the HSA_QUEUE_FEATURE_DISPATCH
 * queue feature.
 * 2) Load and initialize the no-op kernel.
 * 3) Select an agent from the list and create a queue.
 * 4) Create 6 signals associated with the agent (or all agents), and
 * initialize all signals to have a value of 1.
 * 5) Enqueue a barrier packet to the queue and use n of the signals
 * as dependencies. The variable n has a value from 1 to 5.
 * 6) Enqueue a dispatch packet that executes the no-op kernel and uses
 * the final signal as the completion signal.
 * 7) Check the completion signal of the no-op kernel, and verify that
 * it has not decremented.
 * 8) Set one of the barrier's dependant signal values to 0 and repeat
 * step 7 until all n signals have been decremented.
 * 9) Wait until the kernels completion signal has decremented.
 *
 * Expected results: The kernels completion signal should not decrement until
 * all of the barrier's dependant signals have decremented.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <time.h>

int test_aql_barrier_and() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("no_op.brig", &module));

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

        // Create the queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Finalize the executable
        hsa_code_object_t code_object;
        hsa_executable_t executable;

        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));

        status = finalize_executable(agent_list.agents[ii],
                                     1,
                                     &module,
                                     HSA_MACHINE_MODEL_LARGE,
                                     HSA_PROFILE_FULL,
                                     HSA_DEFAULT_FLOAT_ROUNDING_MODE_ZERO,
                                     HSA_CODE_OBJECT_TYPE_PROGRAM,
                                     0,
                                     control_directives,
                                     &code_object,
                                     &executable);

        ASSERT(HSA_STATUS_SUCCESS == status);

        // Get the symbol and the symbol info
        symbol_record_t symbol_record;
        memset(&symbol_record, 0, sizeof(symbol_record_t));

        char* symbol_names[1];
        symbol_names[0] = "&__no_op_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create six signals
        int jj;
        hsa_signal_t signals[6];
        for (jj = 0; jj < 6; ++jj) {
            status = hsa_signal_create((hsa_signal_value_t) 1, 0, NULL, &signals[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        const size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);

        // Create and enqueue a barrier packet
        uint64_t packet_id = hsa_queue_add_write_index_relaxed(queue, 1);
        hsa_barrier_and_packet_t* barrier_packet = (hsa_barrier_and_packet_t*)(queue->base_address + packet_id * packet_size);
        memset(barrier_packet, 0, packet_size);


        // Add dependency signals to the barrier packet
        for (jj = 0; jj < 5; ++jj) {
            barrier_packet->dep_signal[jj] = signals[jj];
        }

        uint16_t header = 0;
        header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        header |= HSA_PACKET_TYPE_BARRIER_AND << HSA_PACKET_HEADER_TYPE;

        __atomic_store_n((uint16_t*)(&barrier_packet->header), header, __ATOMIC_RELEASE);

        // Signal the door bell to launch the packet
        hsa_signal_store_release(queue->doorbell_signal, packet_id);

        // Create and enqueue a dispatch kernel
        packet_id = hsa_queue_add_write_index_relaxed(queue, 1);
        hsa_kernel_dispatch_packet_t* dispatch_packet = (hsa_kernel_dispatch_packet_t*)(queue->base_address + packet_id * packet_size);
        memset(dispatch_packet, 0, packet_size);
        dispatch_packet->completion_signal = signals[5];
        dispatch_packet->setup = 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet->workgroup_size_x = 256;
        dispatch_packet->workgroup_size_y = 1;
        dispatch_packet->workgroup_size_z = 1;
        dispatch_packet->grid_size_x = 256;
        dispatch_packet->grid_size_y = 1;
        dispatch_packet->grid_size_z = 1;
        dispatch_packet->group_segment_size = symbol_record.group_segment_size;
        dispatch_packet->private_segment_size = symbol_record.private_segment_size;
        dispatch_packet->kernel_object = symbol_record.kernel_object;

        header = 0;
        header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

        __atomic_store_n((uint16_t*)(&dispatch_packet->header), header, __ATOMIC_RELEASE);

        // Signal the door bell to launch the packet
        hsa_signal_store_release(queue->doorbell_signal, packet_id);

        // Decrement signal 1-4, the barrier packet should not complete until
        // the 5th signal is triggered
        for (jj = 0; jj < 4; ++jj) {
            hsa_signal_store_release(signals[jj], 0);
            uint32_t time_now = time(NULL);
            if (0 == hsa_signal_wait_relaxed(signals[5], HSA_SIGNAL_CONDITION_EQ, 0, time_now + 1, HSA_WAIT_STATE_BLOCKED)) {
                // signals[5] should never be decremented
                ASSERT(0);
            }
        }

        // Decrement signal 5
        hsa_signal_store_release(signals[4], 0);
        // The barrier packet should be processed.

        // Wait for the no_op kernel to finish
        while (0 != hsa_signal_wait_relaxed(signals[5], HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED));

        // Destroy all signals
        for (jj = 0; jj < 6; ++jj) {
            status = hsa_signal_destroy(signals[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
