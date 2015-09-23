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
 * Test Name: memory_vector_copy_heap_not_registered
 * Purpose: Test that non registered stack memory can be accessed by an agent if
 * it supports the full profile, and that a kernel can copy data from/to another place.
 *
 * Test Description:
 * 1. Declare two stack arrays of the same size.
 * 2. Initialize the first array with non zero values and the second with zero values.
 * 3. Launch a kernel to copy data from the first array to the second.
 * 4. Repeat step 3 for several times.
 *
 * Expected Results: No error status is returned during the process and the values are copied
 * correctly.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

int test_memory_vector_copy_stack_not_registered() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("vector_copy.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Verify that the agent supports the full profile
        hsa_profile_t profile;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_PROFILE, &profile);
        if (HSA_PROFILE_FULL != profile) {
            continue;
        }

        // Check if the queue supports dispatch
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
        symbol_names[0] = "&__vector_copy_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Declare the arrays
        // The size of the data block must be able to fit into one workgroup
        uint32_t block_size = 1024;
        uint32_t data_in[block_size];
        uint32_t data_out[block_size];

        // Initialize the data
        memset(data_out, 0, block_size * sizeof(uint32_t));
        int kk;
        for (kk = 0; kk < block_size; ++kk) {
            data_in[kk] = kk;
        }

        // Allocate the kernel argument buffer from the correct region
        kernarg_vector_copy_t* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_vector_copy_t), (void*) &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Setup the kernarg
        kernarg_buffer->in  = data_in;
        kernarg_buffer->out = data_out;

        // Execute the kernel several times
        int jj;
        int repeat_count = 8;
        for (jj = 0; jj < repeat_count; ++jj) {
            // Clear the output block
            memset(data_out, 0, sizeof(uint32_t) * block_size);

            // Launch the vector_copy kernel
            launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*)kernarg_buffer);

            // Verify the output data block is updated
            for (kk = 0; kk < block_size; ++kk) {
                ASSERT(data_in[kk] == data_out[kk]);
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
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
