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
 * Purpose: Tests that the hsa_memory_copy API can copy data between memory
 * allocated by the hsa runtime allocator API's and registered memory.
 *
 * Test Description:
 *
 * 1. Iterate through all of the agents in the system, and for each agent:
 * 2. Find a region in the agent's global segment and use hsa_memory_allocate
 *    to allocate one buffer, one denoted as the source buffer.
 * 3. Allocate memory from the system, and register it with the hsa_register_memory
 *    API, and denote it as the destination.
 * 4. For all source buffers, initialize the both the source and destination
 *    buffers to different values and use hsa_memory_copy to copy source to destination.
 *
 * Expected Results: For all buffer copies, the data from the source should be
 * successfully copied to the destination.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include <stdlib.h>

int test_memory_copy_allocated_to_registered() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

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

        // Check if this agent has a global memory region
        hsa_region_t global_region;
        global_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region, &global_region);
        if ((uint64_t)-1 != global_region.handle) {
            continue;
        }

        // Allocate the source buffer on the global segment
        size_t data_size = 1024;
        uint32_t* src_buffer;
        status = hsa_memory_allocate(global_region, data_size * sizeof(uint32_t), (void*) &src_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate the destination buffer on system memory
        uint32_t* dst_buffer;
        dst_buffer = (uint32_t*)malloc(data_size * sizeof(uint32_t));
        // Register the destination buffer
        status = hsa_memory_register(dst_buffer, data_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Initialize the buffer
        int kk;
        for (kk = 0; kk < data_size; ++kk) {
            src_buffer[kk] = kk;
            dst_buffer[kk] = 0;
        }

        hsa_memory_copy(dst_buffer, src_buffer, sizeof(uint32_t) * data_size);

        // Verify data are successfully copied
        for (kk = 0; kk < data_size; ++kk) {
            if (dst_buffer[kk] != src_buffer[kk]) {
                ASSERT(0);
            }
        }

        // Free the buffer
        status = hsa_memory_free(src_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        status = hsa_memory_deregister(dst_buffer, data_size * sizeof(uint32_t));
        ASSERT(HSA_STATUS_SUCCESS == status);
        free(dst_buffer);
    }

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
