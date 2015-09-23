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
 * Test Name: region_alignment
 *
 * Purpose: Test that if each region's attribute information is following
 * the constraints, and if the information is consistent across multiple threads
 *
 * Test Description:
 * 1) For each agent
 * 2) For each memory region of the agent that has HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED alloc memory and make sure
 * 3) It is aligned as specified by the HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT
 * 4) That the alignment attribute is a power of 2.
 *
 * Expected Results: The queried alignment size should be consistent
 * as specified by the HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>

int test_memory_region_alignment() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Get a list of all regions available to this agent
        struct region_list_s region_list;
        get_region_list(agent_list.agents[ii], &region_list);

        // For each of the region, get region info
        int jj;
        for (jj = 0; jj < region_list.num_regions; ++jj) {
            // Find the max_size defined by the region
            size_t max_size;
            status = hsa_region_get_info(region_list.regions[jj], HSA_REGION_INFO_ALLOC_MAX_SIZE, &max_size);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Find the region that has HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT
            size_t alignment_size;
            status = hsa_region_get_info(region_list.regions[jj], HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT, &alignment_size);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Verify the alignment attribute is a power of 2
            if (max_size == 0) {
               ASSERT(alignment_size == 0);
            } else {
               ASSERT(alignment_size&&(!(alignment_size&(alignment_size-1))));
            }
        }
        // Free the region list
        free_region_list(&region_list);
    }

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
