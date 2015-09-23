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
 * Test Name: queue_size_create
 * Scope: Conformance

 * Purpose: Verifies that all queue sizes that are a power of 2 between
 * 2 and HSA_AGENT_INFO_QUEUE_MAX_SIZE can be used to create a valid
 * queue on the agent.
 *
 * Test Description:
 * 1) For each agent in the system query the HSA_AGENT_INFO_QUEUE_MAX_SIZE.
 * 2) For the agent create queues using size values between 2 and
 * HSA_AGENT_INFO_QUEUE_MAX_SIZE, performing the following for each:
 *    a) Check that the write and read indexes are initialized to 0.
 *    b) Check that every packet in the queue buffer has its type
 *    initialized to HSA_PACKET_TYPE_INVALID.
 *    c) Dispatch a number of no-op kernels to the queue equal to the
 *    size of the queue, using a signal for the last dispatch to determine
 *    when the dispatches have finished. This is required if the queue
 *    has the HSA_QUEUE_FEATURE_DISPATCH feature.
 *    d) Destroy each queue.
 * 3) Repeat for each agent.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>

int test_queue_size_create() {
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

        // Get the maximum number of queues that is supported on this agent
        uint32_t queue_max;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queue_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (queue_max < 1) {
            continue;
        }

        // Finalize the executable
        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
        hsa_code_object_t code_object;
        hsa_executable_t executable;

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

        // Get the queue's maximum size
        uint32_t queue_max_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // The queue_size must be power of 2
        uint32_t queue_size;
        for (queue_size = 2; queue_size <= queue_max_size; queue_size *= 2) {
            // Create a queue with specified "queue_size"
            hsa_queue_t* queue;
            status = hsa_queue_create(agent_list.agents[ii], queue_size, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Verify that the read_index and write_index are initialized to zero
            uint64_t read_index, write_index;
            read_index = hsa_queue_load_read_index_acquire(queue);
            ASSERT((uint64_t)0 == read_index);
            write_index = hsa_queue_load_write_index_acquire(queue);
            ASSERT((uint64_t)0 == write_index);

            // Create the signal with initial value of 1.
            // The last packet in the queue will set the signal
            hsa_signal_t signal;
            status = hsa_signal_create(1, 0, NULL, &signal);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Fill in info for the default dispatch packet
            hsa_kernel_dispatch_packet_t dispatch_packet;
            memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
            dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
            dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
            dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
            dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
            dispatch_packet.setup  |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
            dispatch_packet.workgroup_size_x = 256;
            dispatch_packet.workgroup_size_y = 1;
            dispatch_packet.workgroup_size_z = 1;
            dispatch_packet.grid_size_x = 256;
            dispatch_packet.grid_size_x = 1;
            dispatch_packet.grid_size_x = 1;
            dispatch_packet.group_segment_size = symbol_record.group_segment_size;
            dispatch_packet.private_segment_size = symbol_record.private_segment_size;
            dispatch_packet.kernel_object = symbol_record.kernel_object;
            dispatch_packet.kernarg_address = 0;

            int jj;
            for (jj = 0; jj < queue_size; ++jj) {
                // Request a new packet ID
                uint64_t write_index = hsa_queue_add_write_index_acquire(queue, 1);

                // Compute packet offset
                hsa_kernel_dispatch_packet_t* queue_packet = (hsa_kernel_dispatch_packet_t*) queue->base_address
                        + write_index % queue->size;

                // Verify the packet header is initialized to INVALID
                ASSERT((uint8_t) HSA_PACKET_TYPE_INVALID == *(uint8_t*)(&queue_packet->header));

                if (jj == queue_size - 1) {
                    dispatch_packet.completion_signal = signal;
                }

                enqueue_dispatch_packet_at(write_index, queue, &dispatch_packet);
            }

            // Wait until all the kernels are complete
            hsa_signal_wait_relaxed(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

            // Destroy the signal
            hsa_signal_destroy(signal);

            // Destroy the queue
            status = hsa_queue_destroy(queue);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
