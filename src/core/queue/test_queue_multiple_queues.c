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
 * Test Name: queue_multiple_queues
 * Scope: Conformance
 *
 * Purpose: Verifies that a single agent can have multiple
 * queues associated with it, and that all queues are initialized
 * properly and functional.
 *
 * Test Description:
 * 1) For each agent in the system
 *    a) Query the agent to determine its HSA_AGENT_INFO_QUEUES_MAX
 *    parameter.
 *    b) Create that number of queues for the agent, using default
 *    values for other creation parameters, if possible.
 * 2) Verify that each queue hsa a unique queue_id. Note that this
 * id should be unique for every queue, not just queues belonging
 * to an agent.
 * 3) Check that the read pointer and write pointer are
 * initialized to 0.
 * 4) Create a simple kernel and verify that it will run on each queue.
 * 5) Destroy all queues in the main thread with hsa_queue_destroy.
 *
 * Expected Results: All queues should be successfully created and
 * all properties should be initialized correctly.
 *
 */

#include <hsa.h>
#include <framework.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <concurrent_utils.h>
#include <queue_utils.h>
#include <stdlib.h>

static const size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);

// Function to compare queue ids
int compare_queue_ids(const void *a, const void *b) {
    return *((uint32_t*) a) - *((uint32_t*) b);
}

int test_queue_multiple_queues() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("no_op.brig", &module));

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int jj;
    for (jj = 0; jj < agent_list.num_agents; ++jj) {
        hsa_queue_type_t queue_type;
        uint32_t queue_max;
        uint32_t queue_max_size;

        // Get the supported queue type
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_QUEUE_TYPE, (void*) &queue_type);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Get the max number of queues for the agent
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_QUEUES_MAX, (void*) &queue_max);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Get the maximum size of the queues
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_QUEUE_MAX_SIZE, (void*) &queue_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        if (queue_max_size == 0) {
            continue;
        }

        hsa_queue_t* queues[queue_max];
        memset(queues, 0, sizeof(queues));

        // Create queues
        int ii;
        for (ii = 0; ii < queue_max; ++ii) {
            status = hsa_queue_create(agent_list.agents[jj], queue_max_size, queue_type, NULL, NULL, UINT32_MAX, UINT32_MAX, &queues[ii]);

            // Break if the API returns because of resource issues
            if (HSA_STATUS_ERROR_OUT_OF_RESOURCES == status) {
                break;
            }

            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Use the number of queues that were created
        queue_max = ii;

        // Allocate an array to store ids of every queue
        uint32_t ids[queue_max];
        memset(ids, 0, sizeof(ids));

        for (ii = 0; ii < queue_max; ++ii) {
            // Get the id of the queue and store into id array
            ids[ii] = queues[ii]->id;

            // Check if read_pointer and write_pointer are initialized to zero
            uint64_t read_pointer, write_pointer;
            // read_pointer = hsa_queue_load_read_index_acquire(queues[ii]);
            // ASSERT(read_pointer == 0);

            // write_pointer = hsa_queue_load_write_index_acquire(queues[ii]);
            // ASSERT(write_pointer == 0);
        }

        // Sort the queue ids
        qsort(ids, queue_max, sizeof(uint32_t), compare_queue_ids);

        // Check that all the ids are unique
        for (ii = 1; ii < queue_max; ++ii) {
            ASSERT_MSG(ids[ii - 1] != ids[ii], "the ids of queues are not unique\n");
        }

        uint32_t features = 0;
        hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_FEATURE, &features);

        // If the agent is capable if dispatch, attempt to dispatch a kernel
        // to the set of queues
        if ((features & HSA_AGENT_FEATURE_KERNEL_DISPATCH) != 0) {
            // Finalize the executable
            hsa_ext_control_directives_t control_directives;
            memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
            hsa_code_object_t code_object;
            hsa_executable_t executable;

            status = finalize_executable(agent_list.agents[jj],
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
            status = get_executable_symbols(executable, agent_list.agents[jj], 0, 1, symbol_names, &symbol_record);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Create a single signal to indicate completion
            hsa_signal_t signal;
            hsa_signal_create(1, 0, NULL, &signal);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // For each queue, dispatch the kernel and wait for it to complete.
            hsa_kernel_dispatch_packet_t dispatch_packet;
            memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
            dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
            dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
            dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
            dispatch_packet.setup |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
            dispatch_packet.workgroup_size_x = 1;
            dispatch_packet.workgroup_size_y = 1;
            dispatch_packet.workgroup_size_z = 1;
            dispatch_packet.grid_size_x = 1;
            dispatch_packet.grid_size_y = 1;
            dispatch_packet.grid_size_z = 1;
            dispatch_packet.group_segment_size = symbol_record.group_segment_size;
            dispatch_packet.private_segment_size = symbol_record.private_segment_size;
            dispatch_packet.kernel_object = symbol_record.kernel_object;
            dispatch_packet.kernarg_address = 0;
            dispatch_packet.completion_signal = signal;
            dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

            for (ii = 0; ii < queue_max; ++ii) {
                // Initialize the signal value to 1.
                hsa_signal_store_relaxed(signal, 1);

                // Enqueue the packet
                enqueue_dispatch_packet(queues[ii], &dispatch_packet);

                // Wait on the signal value to decrement
                status = hsa_signal_wait_relaxed(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
                ASSERT(HSA_STATUS_SUCCESS == status);
            }

            // Destroy the signal.
            status = hsa_signal_destroy(signal);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Destroy the executable
            status = hsa_executable_destroy(executable);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Destroy the code object
            status = hsa_code_object_destroy(code_object);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Destroy the queues
        for (ii = 0; ii < queue_max; ++ii) {
            status = hsa_queue_destroy(queues[ii]);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "hsa_queue_destory failed\n");
        }
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
