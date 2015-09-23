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
 * Test Name: memory_group_dynamic_allocation
 * Purpose: Verify that group memory can be dynamically allocated and used in
 * a kernel.
 *
 * Test Description:
 * 1) Generate a list of agents that support dispatch.
 * 2) For each agent, load and finalize the group_memory_dynamic kernel.
 * 3) Create suitable memory buffers to correctly execute the
 *    group_memory_dynamic kernel.
 * 4) Execute the kernel on the target agent, and wait for the execution to
 *    complete.
 *
 * Expected Results: The kernel should execute successfully, and the memory
 * should be modified as expected.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

int test_memory_group_dynamic_allocation() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("group_memory.brig", &module));

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

        // Get the agent's global region and allocate input and output buffers
        hsa_region_t global_region;
        global_region.handle=(uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        ASSERT((uint64_t)-1 != global_region.handle);

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Create the queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 16, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
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
        symbol_names[0] = "&__group_memory_dynamic_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate the data block to be used by the kernel
        // The size of the data block must be able to fit into one workgroup
        const uint32_t num_workitems = 256;
        const uint32_t num_workgroups = 4;
        const uint32_t group_memory_size = 1024;

        uint32_t block_size = num_workitems * num_workgroups;
        uint32_t* data_in;
        uint32_t* data_out;
        status = hsa_memory_allocate(global_region, block_size * sizeof(uint32_t), (void**) &data_in);
        status = hsa_memory_allocate(global_region, block_size * sizeof(uint32_t), (void**) &data_out);

        // Initialize the data
        memset(data_out, 0, sizeof(uint32_t) * block_size);
        int kk;
        for (kk = 0; kk < block_size; ++kk) {
            data_in[kk] = kk;
        }

        // Calculate the offset of dynamic group memory (or, the size of static
        // group memory)
        // The dynamic group memory starts right after the static group memory,
        // if the kernel defines one.
        uint32_t grp_offset = symbol_record.group_segment_size;

        // The total byte size of group memory, static + dynamic
        uint32_t total_grp_byte_sizse = symbol_record.group_segment_size + group_memory_size * sizeof(uint32_t);

        // Allocate the kernel argument buffer from the correct region
        void* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_group_memory_dynamic_alloc_t), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Setup the kernarg
        kernarg_group_memory_dynamic_alloc_t args;
        args.data_in  = data_in;
        args.data_out = data_out;
        args.grp_offset = (uint32_t)grp_offset;
        args.count = group_memory_size;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Create a signal with initial value of 1
        hsa_signal_t signal;
        status = hsa_signal_create(1, 0, NULL, &signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Enqueue the dispatch packet
        uint64_t packet_id = hsa_queue_add_write_index_acquire(queue, 1);

        while (packet_id - hsa_queue_load_read_index_relaxed(queue) >= queue->size) {}

        hsa_kernel_dispatch_packet_t* dispatch_packet = (hsa_kernel_dispatch_packet_t*)queue->base_address
                + packet_id % queue->size;

        memset(dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
        dispatch_packet->completion_signal = signal;
        dispatch_packet->setup  |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet->workgroup_size_x = (uint16_t)num_workitems;
        dispatch_packet->workgroup_size_y = (uint16_t)1;
        dispatch_packet->workgroup_size_z = (uint16_t)1;
        dispatch_packet->grid_size_x = (uint32_t)block_size;
        dispatch_packet->grid_size_y = 1;
        dispatch_packet->grid_size_z = 1;
        dispatch_packet->kernel_object = (uint64_t) symbol_record.kernel_object;
        dispatch_packet->kernarg_address = (void*) kernarg_buffer;
        dispatch_packet->group_segment_size = total_grp_byte_sizse;

        uint16_t header = 0;
        header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

        __atomic_store_n((uint16_t*)(&dispatch_packet->header), header, __ATOMIC_RELEASE);

        // Ring the signal door bell to launch the packet
        hsa_signal_store_release(queue->doorbell_signal, packet_id);

        // Wait until the kernel complete
        while (0 != hsa_signal_wait_acquire(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED)) {}

        hsa_signal_destroy(signal);

        // Validate the kernel was executed correctly
        for (kk = 0; kk < block_size; ++kk) {
            ASSERT(data_out[kk] == data_in[kk]);
        }

        // Free the kernarg memory buffer
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the data buffers
        status = hsa_memory_free(data_in);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(data_out);
        ASSERT(HSA_STATUS_SUCCESS == status);

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
