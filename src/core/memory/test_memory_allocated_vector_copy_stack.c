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
 * Test Name: memory_allocated_vector_copy_heap
 *
 * Purpose: Test that memory allocated by hsa runtime can be accessed by
 * component, and that kernel can copy data from/to allocated memory
 * to/from heap, which is either registered or not.
 *
 * Test Description:
 *
 * 1. Repeat the following test for every agent that supports the full profile.
 *
 * 2. Allocate two memory blocks from the system's stack.
 *
 * 3. Allocate the same sized memory block using the HSA runtime APIs, allocating the
 *    memory in the global segment.
 *
 * 4. Initialize the first buffer to non-zero values, and the other buffers to zero.
 *
 * 5. Launch a kernel that copies data from the first buffer to last buffer, and then
 *    back to the second.
 *
 * 6. Check that the buffer was copied correctly.
 *
 * 7. Register the system allocated buffers, and repeat steps 5 and 6.
 *
 * Expected Results: No error status should be returned during the execution.
 *
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"
#include <stdlib.h>

int test_memory_allocated_vector_copy_stack() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("vector_copy.brig", &module));

    // Allocate the system data buffers
    const uint32_t block_size = 1024;
    uint32_t sysbuf_1[block_size];
    uint32_t sysbuf_2[block_size];
    uint32_t* agent_buf;

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

        // Check that the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Get the agent's global region, and allocate the agent buffer
        hsa_region_t global_region;
        global_region.handle=(uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        ASSERT((uint64_t)-1 != global_region.handle);
        status = hsa_memory_allocate(global_region, block_size* sizeof(uint32_t), (void *)&agent_buf);
        ASSERT(status == HSA_STATUS_SUCCESS);

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle=(uint64_t)-1;
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

        // Allocate the kernel argument buffer from the correct region
        void* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_vector_copy_t), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);
        kernarg_vector_copy_t args;

        // Initialize the data
        int kk;
        for (kk = 0; kk < block_size; ++kk) {
            sysbuf_1[kk] = kk;
        }

        // Setup the kernarg
        args.in  = sysbuf_1;
        args.out = agent_buf;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Clear the destination block
        memset(agent_buf, 0, sizeof(uint32_t) * block_size);

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*)kernarg_buffer);

        // Setup the kernarg
        args.in  = agent_buf;
        args.out = sysbuf_2;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Clear the destination block
        memset(sysbuf_2, 0, sizeof(uint32_t) * block_size);

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*)kernarg_buffer);

        // Verify the output data block is updated
        for (kk = 0; kk < block_size; ++kk) {
            ASSERT(sysbuf_2[kk] == sysbuf_1[kk]);
        }

        // -----------------------------------
        // Repeat with registered memory.
        // -----------------------------------
        status = hsa_memory_register(sysbuf_1, block_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);

        status = hsa_memory_register(sysbuf_2, block_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Setup the kernarg
        args.in  = sysbuf_1;
        args.out = agent_buf;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Clear the destination block
        memset(agent_buf, 0, sizeof(uint32_t) * block_size);

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*)kernarg_buffer);

        // Setup the kernarg
        args.in  = agent_buf;
        args.out = sysbuf_2;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Clear the destination block
        memset(sysbuf_2, 0, sizeof(uint32_t) * block_size);

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*)kernarg_buffer);

        // Verify the output data block is updated
        for (kk = 0; kk < block_size; ++kk) {
            ASSERT(sysbuf_2[kk] == sysbuf_1[kk]);
        }

        status = hsa_memory_deregister(sysbuf_1, block_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_deregister(sysbuf_2, block_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the kernarg memory buffer
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the agent allocated buffer
        status = hsa_memory_free(agent_buf);
        ASSERT(status == HSA_STATUS_SUCCESS);

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ++ii;
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
