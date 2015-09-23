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
 * Test Name: memory_assign_agent
 * Purpose: Test that the hsa_memory_assign_agent() assigns a new owner
 * (another agent) to a coarse-grained global buffer, and verifies that the new
 * owner can access the buffer.
 *
 * Test Description:
 * 1. Get a list of all available agents, and find the one that has a coarse-
 *    grained memory region.
 * 2. Allocate a memory buffer on this coarse-grained region. This buffer is
 *    going to be accessed by other agents after the ownership of this buffer
 *    is re-assigned.
 * 3. For each agent that supports kernel dispatch (other than the agent that
 *    already has a coarse-grained region identified), perform the following
 *    actions:
 *    1) Assign this agent as the owner of the coarse-grained buffer by using
 *       hsa_memory_assign_agent() with Read & Write access permissions.
 *    2) Allocate a fine-grained buffer from this agent's global region.
 *    3) Launch the vector_copy kernel to transfer data between the coarse-
 *       grained buffer and the fine-grained buffer.
 *    4) Verify that data are correctly transferred.
 *
 *
 * Expected Results: The ownership of a coarse-grained buffer can be re-assigned
 * to another agent, and that agent can access the buffer.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

int test_memory_assign_agent() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("vector_copy.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Find the agent that has a coarse-grained memory region
    hsa_agent_t coarse_grained_agent;
    hsa_region_t coarse_grained_region;
    coarse_grained_agent.handle = (uint64_t)-1;
    coarse_grained_region.handle = (uint64_t)-1;

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        hsa_agent_iterate_regions(agent_list.agents[ii],
                get_global_memory_region_coarse_grained, &coarse_grained_region);
        if ((uint64_t)-1 != coarse_grained_region.handle) {
            // Found the agent that has a coarse-grained region
            coarse_grained_agent = agent_list.agents[ii];
        }
    }

    // If we didn't find the coarse-grained agent, stop here.
    if ((uint64_t)-1 ==  coarse_grained_agent.handle) {
        return 0;
    }

    // Allocate memory on the coarse-grained region
    const int data_size = 1024;
    const size_t buffer_byte_size = (size_t)(sizeof(uint32_t) * data_size);
    uint32_t* coarse_grained_buffer;
    status = hsa_memory_allocate(coarse_grained_region, buffer_byte_size, (void**)&coarse_grained_buffer);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Repeat the test for each agent that supports kernel dispatch
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Skip the agent that is the coarse-grained agent
        if (agent_list.agents[ii].handle == coarse_grained_agent.handle) {
            continue;
        }

        // Check if the agent supports kernel dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Find the agent's kernarg region
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Allocate memory on this agent's global region
        hsa_region_t global_region;
        global_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii],
                get_global_memory_region_fine_grained, &global_region);
        ASSERT((uint64_t)-1 != global_region.handle);

        uint32_t* fine_grained_buffer;
        status = hsa_memory_allocate(global_region, buffer_byte_size, (void**)&fine_grained_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Assign the new owner to the coarse-grained buffer
        status = hsa_memory_assign_agent(coarse_grained_buffer, agent_list.agents[ii], HSA_ACCESS_PERMISSION_RW);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create a kernel dispatch queue
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
        symbol_names[0] = "&__vector_copy_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Setup kernel arguments
        void* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_vector_copy_t), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // the kernarg data structure
        kernarg_vector_copy_t args;

        // Initialize data for write operation
        int jj;
        for (jj = 0; jj < data_size; ++jj) {
            fine_grained_buffer[jj] = (uint32_t)jj;
        }
        memset(coarse_grained_buffer, 0, buffer_byte_size);
        args.in  = fine_grained_buffer;
        args.out = coarse_grained_buffer;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, data_size, (uint64_t)symbol_record.kernel_object, (void*)kernarg_buffer);

        // Verify the coarse-grained buffer has the correct data
        for (jj = 0; jj < data_size; ++jj) {
            if (coarse_grained_buffer[jj] != (uint32_t)jj) {
                ASSERT(0);
            }
        }

        // Initialize data for read operation
        memset(fine_grained_buffer, 0, buffer_byte_size);
        args.in  = coarse_grained_buffer;
        args.out = fine_grained_buffer;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, data_size, (uint64_t)symbol_record.kernel_object, (void*)kernarg_buffer);

        // Verify the coarse-grained buffer has the correct data
        for (jj = 0; jj < data_size; ++jj) {
            if (fine_grained_buffer[jj] != (uint32_t)jj) {
                ASSERT(0);
            }
        }

        // Free the kernarg memory buffer
        status = hsa_memory_free(kernarg_buffer);
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

        // Free the fine-grained buffer
        status = hsa_memory_free(fine_grained_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_memory_free(coarse_grained_buffer);
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
