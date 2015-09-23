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
 * Test Name: memory_vector_copy_between_stack_and_heap
 * Purpose: Test that registered/non-registered stack memory can be copied
 * to/from registered/non-registered heap memory.
 *
 * Test Description:
 *
 * 2. Allocate a memory block using system allocation API like malloc().
 * 3. Declare the same size block on the stack.
 * 4. Set all of the stack values to non zero value, and the heap value
 *    to zero value.
 * 5. Register both sets of memory with HSA.
 * 6. Launch a kernel to copy data from stack to the heap.
 * 7. Set stack values to zero and launch a kernel to copy data
      the heap to the stack
 * 8. Deregister block #2 and repeat step 4. Register block #1, launch a kernel
 *    to copy data from    block #2 to block #1, after kernel finishes, check
 *    the correctness of copying.
 * 9. Set all of the value in block#2 to zero, launch a kernel to copy data
 *    from block #1 to block #2, check the correctness of copying.
 * 10. Deregister block #1, and shut down hsa runtime;
 *
 * Expected results: No error code should be returned during the process and
 * value copied must be as the same the source.
 *
 */

#include <hsa.h>
#include <finalize_utils.h>
#include <agent_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

int test_memory_vector_copy_between_stack_and_heap() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("vector_copy.brig", &module));

    // Get the list of agents
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

        // Allocate the data block to be used by the kernel.
        const uint32_t block_size = 1024;
        uint32_t* heap_block = (uint32_t*)malloc(sizeof(uint32_t) * block_size);
        uint32_t stack_block[block_size];

        // Initialize the data
        memset(heap_block, 0, block_size * sizeof(uint32_t));
        int kk;
        for (kk = 0; kk < block_size; ++kk) {
            stack_block[kk] = kk;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Allocate the kernel argument buffer from the correct region
        void* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_vector_copy_t), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // The kernarg data structure
        kernarg_vector_copy_t args;

        // -----------------------------------
        // Step 5, 6:
        // Register the block #2 only.
        // Launch the kernel to copy stack_block --> heap_block
        // -----------------------------------
        status = hsa_memory_register(stack_block, block_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Setup the kernarg
        args.in  = stack_block;
        args.out = heap_block;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*) kernarg_buffer);

        // Verify the output data block is updated
        for (kk = 0; kk < block_size; ++kk) {
            ASSERT(heap_block[kk] == stack_block[kk]);
        }

        // ----------------------------------------------
        // Step 7:
        // Launch the kernel to copy heap_block --> stack_block
        // ----------------------------------------------
        args.in  = heap_block;
        args.out = stack_block;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Clear the destination block
        memset(stack_block, 0, sizeof(uint32_t) * block_size);

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*) kernarg_buffer);

        // Verify the output data block is updated
        for (kk = 0; kk < block_size; ++kk) {
            ASSERT(heap_block[kk] == stack_block[kk]);
        }

        // -----------------------------------------------
        // Step 8:
        // Deregister stack_block, register heap_block, then copy stack_block --> heap_block
        // -----------------------------------------------
        status = hsa_memory_register(heap_block, block_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_deregister(stack_block, block_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);

        args.in  = stack_block;
        args.out = heap_block;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Clear the destination block
        memset(heap_block, 0, sizeof(uint32_t) * block_size);

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*) kernarg_buffer);

        // Verify the output data block is updated
        for (kk = 0; kk < block_size; ++kk) {
            ASSERT(heap_block[kk] == stack_block[kk]);
        }

        // -----------------------------------------------
        // Step 9:
        // Clear stack_block, then copy heap_block --> stack_block
        // -----------------------------------------------
        args.in  = heap_block;
        args.out = stack_block;
        memcpy((void*)kernarg_buffer, &args, sizeof(args));

        // Clear the destination block
        memset(stack_block, 0, sizeof(uint32_t) * block_size);

        // Launch the vector_copy kernel
        launch_vector_copy_kernel(queue, block_size, (uint64_t)symbol_record.kernel_object, (void*) kernarg_buffer);

        // Verify the output data block is updated
        for (kk = 0; kk < block_size; ++kk) {
            if (heap_block[kk] != stack_block[kk]) {
                ASSERT(0);
            }
        }

        // Deregister the heap_block
        status = hsa_memory_deregister(heap_block, block_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the kernarg memory buffer
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        free(heap_block);

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
