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
 * Test Name: queue_inactivate
 * Scope: Conformance
 *
 * Purpose: Verifies a queue will become inactive after the
 * hsa_queue_inactivate is call using the queue as a parameter.
 *
 * Test Description:
 * 1) Obtain the list of all agents that support the HSA_QUEUE_FEATURE_DISPATCH
 * queue feature.
 * 2) For each agent create a queue.
 * 4) Load and initialize the init_data kernel.
 * 5) Allocate small memory locations that can be used by the execution of the
 * init_data kernel.
 * 6) Dispatch several init_data kernel executions, and verify that they are executed
 * correctly.
 * 7) Call the hsa_queue_inactivate API on the queue.
 * 8) Dispatch several init_data kernel executions, and verify that they are not
 * executed.
 * 14) Repeat 3 through 13 for each agent.
 *
 * Expected Results: Any dispatch after the queue is inactivated should be ignored.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>
#include <stdlib.h>

#define ARGUMENT_ALIGN_BYTES 16
#define QUEUE_INACTIVATE_DEFAULT_QUEUE_SIZE    64
#define QUEUE_INACTIVATE_NUM_PACKETS        16

void launch_inactivated_test_kernels(
        hsa_agent_t* agent,
        hsa_queue_t* queue,
        symbol_record_t* symbol_record,
        int inactivated) {
    hsa_status_t status;
    int ii;

    // Find a memory region in the global segment
    hsa_region_t global_region;
    global_region.handle=(uint64_t)-1;
    hsa_agent_iterate_regions(*agent, get_global_memory_region_fine_grained, &global_region);
    ASSERT((uint64_t)-1 != global_region.handle);

    // Memory blocks used by the kernel arg
    const int block_size = 1024;
    uint32_t** data_blocks = (uint32_t**)malloc(QUEUE_INACTIVATE_NUM_PACKETS * sizeof(uint32_t*));
    for (ii = 0; ii < QUEUE_INACTIVATE_NUM_PACKETS; ++ii) {
        status = hsa_memory_allocate(global_region, block_size * sizeof(uint32_t), (void**) &data_blocks[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);
        memset(data_blocks[ii], 0, block_size * sizeof(uint32_t));
    }

    // Kernarg data structure
    typedef struct __attribute__ ((aligned(ARGUMENT_ALIGN_BYTES))) multi_queue_dispatch_arg {
        void* data;
        uint32_t value;
        uint32_t row_pitch;
        uint32_t slice_pitch;
    } multi_queue_dispatch_arg_t;
    multi_queue_dispatch_arg_t args;

    // Find a memory region that supports kernel arguments
    hsa_region_t kernarg_region;
    kernarg_region.handle = (uint64_t)-1;
    hsa_agent_iterate_regions(*agent, get_kernarg_memory_region, &kernarg_region);
    ASSERT((uint64_t)-1 != kernarg_region.handle);

    // Allocate the kernel argument buffer from the correct region
    multi_queue_dispatch_arg_t* kernarg_buffer = NULL;
    status = hsa_memory_allocate(kernarg_region, QUEUE_INACTIVATE_NUM_PACKETS * sizeof(args), (void**)(&kernarg_buffer));
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create the signal with initial value of "num_packets"
    hsa_signal_t signal;
    status = hsa_signal_create((hsa_signal_value_t) QUEUE_INACTIVATE_NUM_PACKETS, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Initialize the packet
    hsa_kernel_dispatch_packet_t dispatch_packet;
    memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
    dispatch_packet.completion_signal = signal;
    dispatch_packet.header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    dispatch_packet.header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    dispatch_packet.setup |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    dispatch_packet.workgroup_size_x = block_size;
    dispatch_packet.workgroup_size_y = 1;
    dispatch_packet.workgroup_size_z = 1;
    dispatch_packet.grid_size_x = block_size;
    dispatch_packet.grid_size_y = 1;
    dispatch_packet.grid_size_z = 1;
    dispatch_packet.group_segment_size = symbol_record->group_segment_size;
    dispatch_packet.private_segment_size = symbol_record->private_segment_size;
    dispatch_packet.kernel_object = symbol_record->kernel_object;
    dispatch_packet.kernarg_address = 0;
    dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

    // Populate dispatch packets
    for (ii = 0; ii < QUEUE_INACTIVATE_NUM_PACKETS; ++ii) {
        // Setup the kernarg
        args.data = data_blocks[ii];
        args.value = (uint32_t)(ii + 1);
        args.row_pitch = (uint32_t)0;
        args.slice_pitch = (uint32_t)0;
        memcpy(kernarg_buffer + ii, &args, sizeof(args));
        dispatch_packet.kernarg_address = (void*)(kernarg_buffer + ii);

        // Enqueue the packet
        enqueue_dispatch_packet(queue, &dispatch_packet);
    }

    // Wait until all the kernels are complete
    if (0 == inactivated) {
        while (0 != hsa_signal_wait_acquire(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED)) {}
    } else {
        hsa_signal_wait_relaxed(signal, HSA_SIGNAL_CONDITION_EQ, 0, (uint64_t) 10, HSA_WAIT_STATE_BLOCKED);
    }

    // Destroy the signal
    hsa_signal_destroy(signal);

    // Verify all kernels are executed correctly
    int valid = 1;
    int failPacketIdx = 0;
    int failDataIdx = 0;
    for (ii = 0; ii < QUEUE_INACTIVATE_NUM_PACKETS; ++ii) {
        int jj;
        for (jj = 0; jj < block_size; ++jj) {
            if (inactivated) {
                // Expect the data_block[ii] is all 0s since none of
                // Kernels have been executed
                if (0 != data_blocks[ii][jj]) {
                    valid = 0;
                    failPacketIdx = ii;
                    failDataIdx = jj;
                    break;
                }
            } else {
                // Expect the data_block[ii] hsa been updated by kernels
                if (data_blocks[ii][jj] != (ii + 1)) {
                    valid = 0;
                    failPacketIdx = ii;
                    failDataIdx = jj;
                    break;
                }
            }
        }
        if (0 == valid) {
            break;
        }
    }

    status = hsa_memory_free(kernarg_buffer);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Free the memory addresses used by kernel arg
    for (ii = 0; ii < QUEUE_INACTIVATE_NUM_PACKETS; ++ii) {
        status = hsa_memory_free(data_blocks[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    free(data_blocks);
}

int test_queue_inactivate(void) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("init_data.brig", &module));

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

        // Create the queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], QUEUE_INACTIVATE_DEFAULT_QUEUE_SIZE, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

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
        symbol_names[0] = "&__init_int_data_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Launch kernels before inactivation
        launch_inactivated_test_kernels(agent_list.agents + ii, queue, &symbol_record, 0);

        // Inactivate the queue
        status = hsa_queue_inactivate(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Launch kernels after inactivation
        launch_inactivated_test_kernels(agent_list.agents + ii, queue, &symbol_record, 1);

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

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
