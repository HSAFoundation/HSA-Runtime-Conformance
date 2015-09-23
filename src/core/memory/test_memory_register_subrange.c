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
 * Test Name: memory_register_subrange
 * Scope: Conformance
 *
 * Purpose: Test that a whole memory block can be registered separately.
 *
 * Test Description:
 * 1. Malloc a block of memory using std allocation function(i.e malloc())
 * 2. Divide this block into several sub-range.
 * 3. Call hsa_memory_register on each of the sub-blocks.
 * 4. Execute vector_copy on each sub-block, and ensure that the kernel executes correctly.
 * 5. Deregister each sub-block.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

int test_memory_register_subrange() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("vector_copy.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

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
            // Continue if this agent does not support DISPATCH
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Create the queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
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

        // Allocate the kernel argument buffer from the correct region
        kernarg_vector_copy_t* kernarg_buffer;
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_vector_copy_t), (void*) &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        const size_t subrange_size = 1024 * sizeof(int32_t);
        const size_t num_subranges = 128;
        size_t data_size = subrange_size * num_subranges;
        size_t data_size_in_bytes = sizeof(int) * data_size;
        int32_t* data = (int*) malloc(data_size_in_bytes);
        int32_t* sub_data[num_subranges];

        int jj;
        // Set up pointers of each subrange
        for (jj = 0; jj < num_subranges; ++jj) {
            sub_data[jj] = data + jj * subrange_size;
        }

        // Register all the subranges
        for (jj = 0; jj < num_subranges; ++jj) {
            status = hsa_memory_register(sub_data[jj], subrange_size);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Launch the init_data kernel on each subrange data
        int32_t values[1024];

        for (jj = 0; jj < num_subranges; ++jj) {
            int kk;
            for (kk = 0; kk < 1024; ++kk) {
                 values[kk] = jj;
            }

            // Fill in the kernel arguments
            kernarg_buffer->in = (void*) values;
            kernarg_buffer->out = (void*) sub_data[jj];
            launch_vector_copy_kernel(queue,
                    1024,
                    (uint64_t)(symbol_record.kernel_object),
                    (void*) kernarg_buffer);

            // Verify the kernel was executed correctly
            for (kk = 0; kk < 1024; ++kk) {
                if (sub_data[jj][kk] != jj) {
                    ASSERT(0);
                }
            }
        }

        // Deregister all the subranges
        for (jj = 0; jj < num_subranges; ++jj) {
            status = hsa_memory_deregister(sub_data[jj], subrange_size);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        free(data);

        // Free the kernarg_buffer that was allocated on kernarg_region
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy queues
        status = hsa_queue_destroy(queue);
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
