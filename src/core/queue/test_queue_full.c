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
 * Test Name: queue_full
 * Scope: Conformance
 *
 * Purpose: Verifies a queue can be completely filled with packets
 * before execution starts and that all submissions still execute correctly.
 *
 * Test Description:
 * 1) Obtain the list of all agents that support the HSA_AGENT_FEATURE_KERNEL_DISPATCH
 * queue feature.
 * 2) For each agent create a queue with a specified size equal to the
 * HSA_AGENT_INFO_QUEUE_MAX_SIZE agent attribute.
 * 3) Create one signal.
 * 4) Load and initialize the init_data kernel.
 * 5) Allocate small memory locations that can be used by the execution of the
 * init_data kernel. There should be enough for each packet slot.
 * 6) Populate (HSA_AGENT_INFO_QUEUE_MAX_SIZE - 1) queue packet slots with a
 * dispatch packet that will dispatch the init_data kernel, but set the packet type
 * to HSA_PACKET_TYPE_ALWAYS_RESERVED. Set the signal value for each one to 0,
 * indicating no signal will be used.
 * 7) Populate the last slot with a dispatch packet that will dispatch the init_data
 * kernel and set the packet type to HSA_PACKET_TYPE_DISPATCH. Set the signal value
 * to the single valid signal created.
 * 8) Step backward through the queue's packets, setting the type to HSA_PACKET_TYPE_DISPATCH.
 * 9) Ring the queue's doorbell.
 * 10) Wait for the signal to indicate that the last packet has finished executing.
 * 11) Check that the read and write index are equal.
 * 12) Validate that all of the memory locations have been successfully updated.
 * 14) Repeat 3 through 13 for each agent.
 *
 * Expected Results: All queues should be successfully created, all dispatches
 * should finish and all of the data should be initialized correctly.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>

#define ARGUMENT_ALIGN_BYTES 16

#define ARRAY_SIZE 128

int test_queue_full() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("init_data.brig", &module));

    // Get the agent list
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii, jj, kk;

    for (jj = 0; jj < agent_list.num_agents; ++jj) {
        // Only test on agents the support the queue dispatch feature.
        hsa_queue_feature_t feature;
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_FEATURE, &feature);
        ASSERT(HSA_STATUS_SUCCESS == status);

        if (!(feature & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // At least one queue should be supported for the agent.
        uint32_t queue_max;
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_QUEUES_MAX, &queue_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(0 < queue_max);

        // Query the agent properties
        uint32_t queue_max_size;
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Find a global region
        hsa_region_t global_region;
        global_region.handle=(uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[jj], get_global_memory_region_fine_grained, &global_region);
        if ((uint64_t)-1 != global_region.handle) {
            // Skip this agent if global fine grained memory isn't available
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[jj], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

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
        symbol_names[0] = "&__init_int_data_kernel";
        status = get_executable_symbols(executable, agent_list.agents[jj], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the queue for the agent.
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[jj], queue_max_size, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create a single signal for the last dispatch and initialize it to 1
        hsa_signal_t signal;
        hsa_signal_create(1, 0, NULL, &signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the prototype packet data.
        hsa_kernel_dispatch_packet_t dispatch_packet;
        size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);
        memset(&dispatch_packet, 0, packet_size);
        dispatch_packet.header |= HSA_PACKET_TYPE_INVALID << HSA_PACKET_HEADER_TYPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
        dispatch_packet.setup |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet.workgroup_size_x = ARRAY_SIZE;
        dispatch_packet.workgroup_size_y = 1;
        dispatch_packet.workgroup_size_z = 1;
        dispatch_packet.grid_size_x = ARRAY_SIZE;
        dispatch_packet.grid_size_y = 1;
        dispatch_packet.grid_size_z = 1;
        dispatch_packet.group_segment_size = symbol_record.group_segment_size;
        dispatch_packet.private_segment_size = symbol_record.private_segment_size;
        dispatch_packet.kernel_object = symbol_record.kernel_object;
        dispatch_packet.kernarg_address = 0;
        dispatch_packet.completion_signal.handle = 0;

        // This array contains pointers to allocated argument buffers, one per dispatch
        void* kernel_arg_buffer[queue->size];
        uint32_t* data_buffer[queue->size];

        // Argument prototype
        struct __attribute__((aligned(ARGUMENT_ALIGN_BYTES))) args_t {
            uint64_t data;
            uint32_t value;
            uint32_t row_pitch;
            uint32_t slice_pitch;
        } args;

        // Kernel argument allocation and buffer allocation
        for (ii = 0; ii < queue->size; ++ii) {
            // Allocate the kernel argument structure for the dispatch
            status = hsa_memory_allocate(kernarg_region,
                                         symbol_record.kernarg_segment_size,
                                         &kernel_arg_buffer[ii]);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Allocate the data buffer
            status = hsa_memory_allocate(global_region,
                                         ARRAY_SIZE*sizeof(uint32_t),
                                         (void**) &data_buffer[ii]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Block until the queue is empty
        uint64_t delta;
        uint64_t write_index = hsa_queue_load_write_index_relaxed(queue);
        do {
            delta = write_index - hsa_queue_load_read_index_relaxed(queue);
        } while (delta > 0);

        // Enqueue all of the packets
        const uint32_t queue_mask = queue->size - 1;
        for (ii = 0; ii < queue->size; ++ii) {
            // Set the kernel arguments and initialize the data
            memset(data_buffer[ii], 0, ARRAY_SIZE*sizeof(uint32_t));
            args.data = (uint64_t) data_buffer[ii];
            args.value = (uint32_t) ii;
            args.row_pitch = 0;
            args.slice_pitch = 0;
            memcpy(kernel_arg_buffer[ii], &args, sizeof(args));
            // Set the packet kernel argument data.
            dispatch_packet.kernarg_address = (void*) kernel_arg_buffer[ii];
            // Increment the write index of the queue, reserving a slot
            write_index = hsa_queue_add_write_index_relaxed(queue, 1);

            // Initialize the packets.
            ((hsa_kernel_dispatch_packet_t*)(queue->base_address))[write_index&queue_mask]=dispatch_packet;
        }

        // For the last queue, set the completion signal
        ((hsa_kernel_dispatch_packet_t*)(queue->base_address))[queue_mask].completion_signal = signal;

        // Step backward through the queue, setting the packet type to HSA_PACKET_TYPE_DISPATCH
        for (ii = queue->size-1; ii >= 0; --ii) {
            ((hsa_kernel_dispatch_packet_t*)(queue->base_address))[ii&queue_mask].header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        }

        // Ring the doorbell using the last write_index
        hsa_signal_store_relaxed(queue->doorbell_signal, write_index);

        // Wait on the signal value to decrement
        status = hsa_signal_wait_relaxed(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Validate the data
        for (ii = 0; ii < queue->size; ++ii) {
            uint32_t* ptr = data_buffer[ii];
            for (kk = 0; kk < ARRAY_SIZE; ++kk) {
                ASSERT(*(ptr+kk) == ii);
            }
        }

        // Deallocate the data
        for (ii = 0; ii < queue->size; ++ii) {
            // Free the associated data arrays
            status = hsa_memory_free(data_buffer[ii]);
            ASSERT(HSA_STATUS_SUCCESS == status);
            // Free the kernel argument structure
            status = hsa_memory_free(kernel_arg_buffer[ii]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the signal
        status = hsa_signal_destroy(signal);
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
