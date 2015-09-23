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
 * Test Name: memory_copy_system_and_global
 * Purpose: Test that the hsa_memory_copy() can copy data between system memory
 * and global memory.
 *
 * Test Description:
 *
 * 1. Initialize hsa runtime by calling hsa_init();
 * 2. Declare two system memory blocks, block_s1 and block_s2;
 * 3. Allocate two memory blocks on the global region of an agent that supports
 *    kernel dispatch, block_g1 and block_g2;
 * 4. Initialize block_s1 with non zero value and block_s2 with zero value;
 * 5. Initialize block_g1 with non zero value and block_g2 with zero value;
 * 6. Use hsa_memory_copy() to copy data
 *    1) from block_s1 to block_s2;
 *    2) from block_g1 to block_g2;
 *    3) from block_s1 to block_g2;
 *    4) from block_g1 to block_s2;
 * 7. Verify data have been successfully copied in each of sub-steps in 6
 * 8. Shut down hsa runtime.

 * Expected Results: No error status is returned during the process and the
 * value copied is always correct.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

int test_memory_copy_system_and_global() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if this agent supports kernel dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Allocate system memory blocks
        const int block_size = 4096;
        int* block_s1 = (int*)malloc(sizeof(int) * block_size);
        int* block_s2 = (int*)malloc(sizeof(int) * block_size);

        // Find the global region for this agent
        hsa_region_t global_region;
        global_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        ASSERT((uint64_t)-1 != global_region.handle);

        // Allocate memory blocks on the global region
        int* block_g1;
        int* block_g2;
        status = hsa_memory_allocate(global_region, sizeof(int) * block_size, (void**) &block_g1);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_allocate(global_region, sizeof(int) * block_size, (void**) &block_g2);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Initialize the values in the memory blocks
        int jj;
        for (jj = 0; jj < block_size; ++jj) {
            block_s1[jj] = jj;
            block_g1[jj] = jj;
        }

        // 6.
        int* src[4]  = {block_s1, block_g1, block_s1, block_g1};
        int* dest[4] = {block_s2, block_g2, block_g2, block_s2};
        for (jj = 0; jj < 4; ++jj) {
            // Clear the destination memory block
            memset(dest[jj], 0, sizeof(int) * block_size);

            // Apply the memory copy
            status = hsa_memory_copy(dest[jj], src[jj], sizeof(int) * block_size);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Verify the copied data are correct
            int kk;
            for (kk == 0; kk < block_size; ++kk) {
                if (kk != dest[jj][kk]) {
                    // Data inconsistency occured
                    ASSERT(0);
                }
            }
        }

        // Free the memory blocks
        status = hsa_memory_free(block_g1);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(block_g2);
        ASSERT(HSA_STATUS_SUCCESS == status);
        free(block_s1);
        free(block_s2);
    }

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
