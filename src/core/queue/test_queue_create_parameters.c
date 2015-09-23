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
 * Test Name: queue_create_parameters
 * Scope: Conformance
 *
 * Purpose: Verifies that each agent on the platform can
 * create a queue that satisfies all specified queue parameters.
 *
 * Test Description:
 * 1) For each agent in the system.
 *    a) Query the HSA_AGENT_INFO_FEATURE attribute.
 *    b) Query the HSA_AGENT_INFO_QUEUE_MAX_SIZE attribute.
 *    c) Query the HSA_AGENT_INFO_QUEUE_TYPE attribute.
 * 2) For each agent create a using the maximal set of parameters,
 * i.e. if a queue supports a given maximum size, HSA_QUEUE_TYPE_MULTI
 * and HSA_AGENT_FEATURE_KERNEL_DISPATCH, create a queue with those
 * attributes.
 * 3) Attempt to dispatch a simple kernel to the queue, if the
 * queue supports HSA_AGENT_FEATURE_KERNEL_DISPATCH.
 * 4) Destroy each queue.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <queue_utils.h>

// The NUM_KERNELS must be a power of 2
#define NUM_KERNELS 16

int test_queue_create_parameters() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("no_op.brig", &module));

    // Get the agent list
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        uint32_t queue_max_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_max_size);
        ASSERT(status == HSA_STATUS_SUCCESS);

        hsa_queue_type_t queue_type;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_TYPE, &queue_type);
        ASSERT(status == HSA_STATUS_SUCCESS);

        hsa_queue_feature_t feature;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &feature);
        ASSERT(status == HSA_STATUS_SUCCESS);

        if (feature & HSA_AGENT_FEATURE_KERNEL_DISPATCH) {
            // Create the queue
            ASSERT(HSA_QUEUE_TYPE_SINGLE == queue_type || HSA_QUEUE_TYPE_MULTI == queue_type);
            hsa_queue_t *queue;
            status = hsa_queue_create(agent_list.agents[ii], queue_max_size, queue_type, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
            ASSERT(status == HSA_STATUS_SUCCESS);

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

            // Signal array
            hsa_signal_t signals[NUM_KERNELS];

            int jj;
            for (jj = 0; jj < NUM_KERNELS; ++jj) {
                status = hsa_signal_create(1, 0, NULL, &signals[jj]);
                ASSERT(HSA_STATUS_SUCCESS == status);
            }

            // Fill info for the default dispatch_packet
            hsa_kernel_dispatch_packet_t dispatch_packet;
            memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
            dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
            dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
            dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
            dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
            dispatch_packet.setup  |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
            dispatch_packet.workgroup_size_x = 256;
            dispatch_packet.workgroup_size_y = 1;
            dispatch_packet.workgroup_size_z = 1;
            dispatch_packet.grid_size_x = 256;
            dispatch_packet.grid_size_y = 1;
            dispatch_packet.grid_size_z = 1;
            dispatch_packet.group_segment_size = symbol_record.group_segment_size;
            dispatch_packet.private_segment_size = symbol_record.private_segment_size;
            dispatch_packet.kernel_object = symbol_record.kernel_object;
            dispatch_packet.kernarg_address = 0;

            // Enqueue dispatch packets
            hsa_kernel_dispatch_packet_t* queue_packet;
            for (jj = 0; jj < NUM_KERNELS; ++jj) {
                dispatch_packet.completion_signal = signals[jj];
                enqueue_dispatch_packet(queue, &dispatch_packet);
            }

            // Wait until all dispatch packets finish executing
            for (jj = 0; jj < NUM_KERNELS; ++jj) {
                hsa_signal_value_t value = hsa_signal_wait_relaxed(signals[jj], HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
                ASSERT(0 == value);
            }

            // Destroy signals
            for (jj = 0; jj < NUM_KERNELS; ++jj) {
                status = hsa_signal_destroy(signals[jj]);
                ASSERT(HSA_STATUS_SUCCESS == status);
            }

            // Destroy queue
            status = hsa_queue_destroy(queue);
            ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    return 0;
}
