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
 * Test Name: async_invalid_group_memory
 * Scope: Conformance
 *
 * Purpose: Verifies that if an aql packet specifies an invalid group
 * group memory segment size, the queue's error handling
 * callback will trigger.
 *
 * Test Description:
 * 1) For each agent on the platform that supports kernel dispatch,
 *    create a max queues each with a valid callback.
 * 2) Dispatch a packet with a large group memory size to each
 *    queue.
 *
 * Expected Results: The queues callback should trigger, and the queue
 * id should be correctly passed to the callback.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_async_utils.h"

int test_async_invalid_group_memory() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("group_memory.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Setup the dispatch packet
    hsa_kernel_dispatch_packet_t dispatch_packet;
    memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
    dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
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
    dispatch_packet.completion_signal.handle = 0;

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Make sure the agent supports kernel dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

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
        symbol_names[0] = "&__group_memory_static_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate the kernel argument buffer from the correct region
        void* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(symbol_record.kernarg_segment_size), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);
        memset(kernarg_buffer, 0, sizeof(symbol_record.kernarg_segment_size));

        // Fill in the kernel dispatch information
        dispatch_packet.kernel_object = symbol_record.kernel_object;
        dispatch_packet.private_segment_size = symbol_record.private_segment_size;
        dispatch_packet.kernarg_address = kernarg_buffer;

        // Request a large group memory segment size
        dispatch_packet.group_segment_size = (uint32_t)-1;

        // Run the test for this agent
        async_test(agent_list.agents[ii], &dispatch_packet, HSA_STATUS_ERROR_INVALID_ALLOCATION);

        // Free the kernarg
        status = hsa_memory_free(kernarg_buffer);
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
