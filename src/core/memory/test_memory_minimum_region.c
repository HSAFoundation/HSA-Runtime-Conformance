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
 * Test Name: memory_minimum_region
 * Purpose: Test that the system includes at least one system global region
 * and one KERNARG region and that the size is the same for the different
 * agents.
 *
 * Test Description:
 * 1. Iterate all of the agents in the system, and for each agent:
 * 2. Iterate all of the regions associated with the agent, and for each
 *    region:
 * 3. Get the region the region's segment and flag INFO.
 * 4. Check that a primary coherent memory is listed for each agent, and
 *    that the size is the same across different agents.
 * 5. Check that is a KERNARG segment is included, the size is the same
 *    across different agents.
 *
 * Expected Results: All agents should have a system global memory region,
 * and the size should be the same for all of them. If they have a KERNARG
 * region, the size should be the same for all of them.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include <stdlib.h>
#include <string.h>

int test_memory_minimum_region() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // An array of size_t to record the sizes of "minimum" global region
    // on each agent
    size_t* global_region_sizes = (size_t*)malloc(agent_list.num_agents * sizeof(size_t));
    memset(global_region_sizes, 0, agent_list.num_agents * sizeof(size_t));

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Get a list of all regions available to this agent
        struct region_list_s region_list;
        get_region_list(agent_list.agents[ii], &region_list);

        // For each of the regions, get the region's info
        int jj;
        size_t size;
        hsa_region_segment_t segment;
        for (jj = 0; jj < region_list.num_regions; ++jj) {
            // Get the region size
            status = hsa_region_get_info(region_list.regions[jj], HSA_REGION_INFO_SEGMENT, &segment);
            if (HSA_REGION_SEGMENT_GLOBAL == segment) {
                status = hsa_region_get_info(region_list.regions[jj], HSA_REGION_INFO_SIZE, &size);
                global_region_sizes[ii] = size;
            }
        }

        // Free the region list
        if (region_list.num_regions > 0) {
            free(region_list.regions);
        }
    }

    // Verify the sizes of global region of each agent is the same
    for (ii = 1; ii < agent_list.num_agents; ++ii) {
        if (global_region_sizes[ii] != global_region_sizes[0]) {
            ASSERT(0);
        }
    }

    free(global_region_sizes);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
