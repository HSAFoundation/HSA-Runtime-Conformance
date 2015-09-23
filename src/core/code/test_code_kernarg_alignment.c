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

/**
 * Test Name: test_code_kernarg_alignment
 * Scope: Conformance
 *
 * Purpose: Ensures that symbol's declared with module scope
 * and program scope can be extracted from a code object
 * and an executable and, if the symbols are kernels,
 * dispatched to a user mode queue.
 *
 * Test Description:
 * 1. Create a code object by loading the kernarg_alignment Brig module
 *    adding it to a program and finalizing it.
 * 2. Use the hsa_code_object_get_symbol API and the kernels' module
 *    qualified and program scope symbol name to obtain the
 *    code object's associated symbol.
 * 3. Create an executable for each agent that supports kernel dispatch
 *    using the code object.
 * 4. Use the hsa_executable_get_symbol API and the kernels' module qualified
 *    and program scope symbol name's to obtain the executable's associated symbol.
 * 5. Each kernal has different kernarg segment alignment
 * 6. Execute the kernels on the agent.
 *
 * Expected Result: The kernels should execute correctly on each agent.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hsa.h>
#include <agent_utils.h>
#include <dispatch_utils.h>
#include <framework.h>
#include <finalize_utils.h>

#define DATA_SIZE 256
#define FIRST_ARG 2
#define SECOND_ARG 3
#define EXPECTED_OUTPUT 5

const int num_kernels = 144;

char* kernarg_align_symbol_names[] = {
        "&__kernarg_8_u64_8_u64_kernel",
        "&__kernarg_8_u64_16_u64_kernel",
        "&__kernarg_8_u64_32_u64_kernel",
        "&__kernarg_8_u64_64_u64_kernel",
        "&__kernarg_8_u64_128_u64_kernel",
        "&__kernarg_8_u64_256_u64_kernel",
        "&__kernarg_16_u64_8_u64_kernel",
        "&__kernarg_16_u64_16_u64_kernel",
        "&__kernarg_16_u64_32_u64_kernel",
        "&__kernarg_16_u64_64_u64_kernel",
        "&__kernarg_16_u64_128_u64_kernel",
        "&__kernarg_16_u64_256_u64_kernel",
        "&__kernarg_32_u64_8_u64_kernel",
        "&__kernarg_32_u64_16_u64_kernel",
        "&__kernarg_32_u64_32_u64_kernel",
        "&__kernarg_32_u64_64_u64_kernel",
        "&__kernarg_32_u64_128_u64_kernel",
        "&__kernarg_32_u64_256_u64_kernel",
        "&__kernarg_64_u64_8_u64_kernel",
        "&__kernarg_64_u64_16_u64_kernel",
        "&__kernarg_64_u64_32_u64_kernel",
        "&__kernarg_64_u64_64_u64_kernel",
        "&__kernarg_64_u64_128_u64_kernel",
        "&__kernarg_64_u64_256_u64_kernel",
        "&__kernarg_128_u64_8_u64_kernel",
        "&__kernarg_128_u64_16_u64_kernel",
        "&__kernarg_128_u64_32_u64_kernel",
        "&__kernarg_128_u64_64_u64_kernel",
        "&__kernarg_128_u64_128_u64_kernel",
        "&__kernarg_128_u64_256_u64_kernel",
        "&__kernarg_256_u64_8_u64_kernel",
        "&__kernarg_256_u64_16_u64_kernel",
        "&__kernarg_256_u64_32_u64_kernel",
        "&__kernarg_256_u64_64_u64_kernel",
        "&__kernarg_256_u64_128_u64_kernel",
        "&__kernarg_256_u64_256_u64_kernel",
        "&__kernarg_8_u64_8_u32_kernel",
        "&__kernarg_8_u64_16_u32_kernel",
        "&__kernarg_8_u64_32_u32_kernel",
        "&__kernarg_8_u64_64_u32_kernel",
        "&__kernarg_8_u64_128_u32_kernel",
        "&__kernarg_8_u64_256_u32_kernel",
        "&__kernarg_16_u64_8_u32_kernel",
        "&__kernarg_16_u64_16_u32_kernel",
        "&__kernarg_16_u64_32_u32_kernel",
        "&__kernarg_16_u64_64_u32_kernel",
        "&__kernarg_16_u64_128_u32_kernel",
        "&__kernarg_16_u64_256_u32_kernel",
        "&__kernarg_32_u64_8_u32_kernel",
        "&__kernarg_32_u64_16_u32_kernel",
        "&__kernarg_32_u64_32_u32_kernel",
        "&__kernarg_32_u64_64_u32_kernel",
        "&__kernarg_32_u64_128_u32_kernel",
        "&__kernarg_32_u64_256_u32_kernel",
        "&__kernarg_64_u64_8_u32_kernel",
        "&__kernarg_64_u64_16_u32_kernel",
        "&__kernarg_64_u64_32_u32_kernel",
        "&__kernarg_64_u64_64_u32_kernel",
        "&__kernarg_64_u64_128_u32_kernel",
        "&__kernarg_64_u64_256_u32_kernel",
        "&__kernarg_128_u64_8_u32_kernel",
        "&__kernarg_128_u64_16_u32_kernel",
        "&__kernarg_128_u64_32_u32_kernel",
        "&__kernarg_128_u64_64_u32_kernel",
        "&__kernarg_128_u64_128_u32_kernel",
        "&__kernarg_128_u64_256_u32_kernel",
        "&__kernarg_256_u64_8_u32_kernel",
        "&__kernarg_256_u64_16_u32_kernel",
        "&__kernarg_256_u64_32_u32_kernel",
        "&__kernarg_256_u64_64_u32_kernel",
        "&__kernarg_256_u64_128_u32_kernel",
        "&__kernarg_256_u64_256_u32_kernel",
        "&__kernarg_8_u32_8_u64_kernel",
        "&__kernarg_8_u32_16_u64_kernel",
        "&__kernarg_8_u32_32_u64_kernel",
        "&__kernarg_8_u32_64_u64_kernel",
        "&__kernarg_8_u32_128_u64_kernel",
        "&__kernarg_8_u32_256_u64_kernel",
        "&__kernarg_16_u32_8_u64_kernel",
        "&__kernarg_16_u32_16_u64_kernel",
        "&__kernarg_16_u32_32_u64_kernel",
        "&__kernarg_16_u32_64_u64_kernel",
        "&__kernarg_16_u32_128_u64_kernel",
        "&__kernarg_16_u32_256_u64_kernel",
        "&__kernarg_32_u32_8_u64_kernel",
        "&__kernarg_32_u32_16_u64_kernel",
        "&__kernarg_32_u32_32_u64_kernel",
        "&__kernarg_32_u32_64_u64_kernel",
        "&__kernarg_32_u32_128_u64_kernel",
        "&__kernarg_32_u32_256_u64_kernel",
        "&__kernarg_64_u32_8_u64_kernel",
        "&__kernarg_64_u32_16_u64_kernel",
        "&__kernarg_64_u32_32_u64_kernel",
        "&__kernarg_64_u32_64_u64_kernel",
        "&__kernarg_64_u32_128_u64_kernel",
        "&__kernarg_64_u32_256_u64_kernel",
        "&__kernarg_128_u32_8_u64_kernel",
        "&__kernarg_128_u32_16_u64_kernel",
        "&__kernarg_128_u32_32_u64_kernel",
        "&__kernarg_128_u32_64_u64_kernel",
        "&__kernarg_128_u32_128_u64_kernel",
        "&__kernarg_128_u32_256_u64_kernel",
        "&__kernarg_256_u32_8_u64_kernel",
        "&__kernarg_256_u32_16_u64_kernel",
        "&__kernarg_256_u32_32_u64_kernel",
        "&__kernarg_256_u32_64_u64_kernel",
        "&__kernarg_256_u32_128_u64_kernel",
        "&__kernarg_256_u32_256_u64_kernel",
        "&__kernarg_8_u32_8_u32_kernel",
        "&__kernarg_8_u32_16_u32_kernel",
        "&__kernarg_8_u32_32_u32_kernel",
        "&__kernarg_8_u32_64_u32_kernel",
        "&__kernarg_8_u32_128_u32_kernel",
        "&__kernarg_8_u32_256_u32_kernel",
        "&__kernarg_16_u32_8_u32_kernel",
        "&__kernarg_16_u32_16_u32_kernel",
        "&__kernarg_16_u32_32_u32_kernel",
        "&__kernarg_16_u32_64_u32_kernel",
        "&__kernarg_16_u32_128_u32_kernel",
        "&__kernarg_16_u32_256_u32_kernel",
        "&__kernarg_32_u32_8_u32_kernel",
        "&__kernarg_32_u32_16_u32_kernel",
        "&__kernarg_32_u32_32_u32_kernel",
        "&__kernarg_32_u32_64_u32_kernel",
        "&__kernarg_32_u32_128_u32_kernel",
        "&__kernarg_32_u32_256_u32_kernel",
        "&__kernarg_64_u32_8_u32_kernel",
        "&__kernarg_64_u32_16_u32_kernel",
        "&__kernarg_64_u32_32_u32_kernel",
        "&__kernarg_64_u32_64_u32_kernel",
        "&__kernarg_64_u32_128_u32_kernel",
        "&__kernarg_64_u32_256_u32_kernel",
        "&__kernarg_128_u32_8_u32_kernel",
        "&__kernarg_128_u32_16_u32_kernel",
        "&__kernarg_128_u32_32_u32_kernel",
        "&__kernarg_128_u32_64_u32_kernel",
        "&__kernarg_128_u32_128_u32_kernel",
        "&__kernarg_128_u32_256_u32_kernel",
        "&__kernarg_256_u32_8_u32_kernel",
        "&__kernarg_256_u32_16_u32_kernel",
        "&__kernarg_256_u32_32_u32_kernel",
        "&__kernarg_256_u32_64_u32_kernel",
        "&__kernarg_256_u32_128_u32_kernel",
        "&__kernarg_256_u32_256_u32_kernel",
};

const uint32_t first_offset[] = {8,8,8,8,8,8,16,16,16,16,16,16,32,32,32,32,32,32,64,64,64,64,64,64,128,128,128,128,128,128,256,256,256,256,256,256,8,8,8,8,8,8,16,16,16,16,16,16,32,32,32,32,32,32,64,64,64,64,64,64,128,128,128,128,128,128,256,256,256,256,256,256,8,8,8,8,8,8,16,16,16,16,16,16,32,32,32,32,32,32,64,64,64,64,64,64,128,128,128,128,128,128,256,256,256,256,256,256,8,8,8,8,8,8,16,16,16,16,16,16,32,32,32,32,32,32,64,64,64,64,64,64,128,128,128,128,128,128,256,256,256,256,256,256};

const uint32_t second_offset[] = {16,16,32,64,128,256,24,32,32,64,128,256,40,48,64,64,128,256,72,80,96,128,128,256,136,144,160,192,256,256,264,272,288,320,384,512,16,16,32,64,128,256,24,32,32,64,128,256,40,48,64,64,128,256,72,80,96,128,128,256,136,144,160,192,256,256,264,272,288,320,384,512,16,16,32,64,128,256,24,32,32,64,128,256,40,48,64,64,128,256,72,80,96,128,128,256,136,144,160,192,256,256,264,272,288,320,384,512,16,16,32,64,128,256,24,32,32,64,128,256,40,48,64,64,128,256,72,80,96,128,128,256,136,144,160,192,256,256,264,272,288,320,384,512};

const size_t first_arg_size[] = {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};

const size_t second_arg_size[] = {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};

// All sizes must be a multiple of 16
const uint32_t kernarg_size[] = {32,32,48,80,144,272,32,48,48,80,144,272,48,64,80,80,144,272,80,96,112,144,144,272,144,160,176,208,272,272,272,288,304,336,400,528,32,32,48,80,144,272,32,48,48,80,144,272,48,64,80,80,144,272,80,96,112,144,144,272,144,160,176,208,272,272,272,288,304,336,400,528,32,32,48,80,144,272,32,48,48,80,144,272,48,64,80,80,144,272,80,96,112,144,144,272,144,160,176,208,272,272,272,288,304,336,400,528,32,32,48,80,144,272,32,48,48,80,144,272,48,64,80,80,144,272,80,96,112,144,144,272,144,160,176,208,272,272,272,288,304,336,400,528};

int test_code_kernarg_alignment() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);
    exit;
   
    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("kernarg_align.brig", &module));  
   
    // Get a list of agents, and iterate through the list
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);
   
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Skip if this agent does not support kernel dispatch
        uint32_t feature = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &feature);
        ASSERT(HSA_STATUS_SUCCESS==status);
        if (HSA_AGENT_FEATURE_KERNEL_DISPATCH != feature) {
            continue;
        }
       
        // Create a queue for dispatching
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL,UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
       
        // Find a global memory region that supports fine grained memory
        hsa_region_t global_region;
        global_region.handle = (uint64_t)-1;
        status = hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        if(status != HSA_STATUS_INFO_BREAK) {
            continue;
        }
        ASSERT((uint64_t)-1 != global_region.handle);

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Finalize the executable
        hsa_code_object_t code_object;
        hsa_executable_t executable;
        hsa_ext_control_directives_t control_directives;

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

        // Allocate data to be used for the kernel arguments
        uint64_t* output;
        status = hsa_memory_allocate(global_region, sizeof(uint64_t) * DATA_SIZE, (void**) &output);

        uint32_t* args_32bit;
        status = hsa_memory_allocate(global_region, sizeof(uint32_t) * 2, (void**) &args_32bit);

        uint64_t* args_64bit;
        status = hsa_memory_allocate(global_region, sizeof(uint64_t) * 2, (void**) &args_64bit);

        args_32bit[0] = FIRST_ARG;
        args_32bit[1] = SECOND_ARG;
        args_64bit[0] = FIRST_ARG;
        args_64bit[1] = SECOND_ARG;

        int jj;
        for (jj = 0; jj < num_kernels; ++jj) {
            // Reset the output data
            memset(output, 0, DATA_SIZE * sizeof(uint32_t));

            // Get the target symbol information
            symbol_record_t symbol_record;
            memset(&symbol_record, 0, sizeof(symbol_record_t));
            status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, &kernarg_align_symbol_names[jj], &symbol_record);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Verify that the kernarg size if expected
            ASSERT_MSG(kernarg_size[jj] == symbol_record.kernarg_segment_size, "The kernel argument size isn't what was expected for the %s kernel. %d != %d\n", kernarg_align_symbol_names[jj], kernarg_size[jj], symbol_record.kernarg_segment_size);

            // Allocate the kernel argument buffer from the correct region
            char* kernarg_start;
            status = hsa_memory_allocate(kernarg_region, symbol_record.kernarg_segment_size, (void**) &kernarg_start);
            ASSERT(HSA_STATUS_SUCCESS == status);
            memset(kernarg_start, 0, symbol_record.kernarg_segment_size);

            char* kernarg_ptr = kernarg_start;

            // Setup the kernel arguments
            memcpy(kernarg_ptr, &output, sizeof(uint32_t*));

            kernarg_ptr = kernarg_start + first_offset[jj];
            if(first_arg_size[jj] == 4) {
                memcpy(kernarg_ptr, &args_32bit[0], sizeof(uint32_t));
            } else {
                memcpy(kernarg_ptr, &args_64bit[0], sizeof(uint64_t));
            }

            kernarg_ptr = kernarg_start + second_offset[jj];
            if(first_arg_size[jj] == 4) {
                memcpy(kernarg_ptr, &args_32bit[1], sizeof(uint32_t));
            } else {
                memcpy(kernarg_ptr, &args_64bit[1], sizeof(uint64_t));
            }

            // Launch the vector_copy kernel
            dispatch_kernel_1d_data(queue, DATA_SIZE, symbol_record.kernel_object, (void*) kernarg_start);

            // Verify the output data block is updated
            int kk;
            for(kk = 0; kk < DATA_SIZE; ++kk) {
                ASSERT_MSG(EXPECTED_OUTPUT == output[kk], "Invalid value found in the %dth element : %d.\n",kk,output[kk]);
            }

            // Free the kernarg memory buffer
            status = hsa_memory_free(kernarg_start);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }
       
        // Free output data and argument buffers
        status = hsa_memory_free((void*) output);
        status = hsa_memory_free((void*) args_32bit);
        status = hsa_memory_free((void*) args_64bit);
       
        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS==status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    free_agent_list(&agent_list);
   
    // Destroy the loaded module
    destroy_module(module);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
