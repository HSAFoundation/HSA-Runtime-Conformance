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
 * Test Name: region_info_check_concurrency
 *
 * Purpose: Test that if each region's attribute information is following
 * the constraints, and if the information is consistent across multiple threads
 *
 * Test Description:
 * 1. Initialize the runtime by calling hsa_init.
 * 2. Iterate all of the agents in the system, and for each agent:
 * 3. Iterate all of the regions associated with agent, and for each region:
 * 4. Get attribute info, check if the results are following the constraints,
 *    and store these attribute information.
 * 5. Launch several threads to query region information again, and compare
 *    the concurrently queried information with the data generated by the main
 *    thread.
 *
 * Expected Results: The concurrently queried information should be consistent
 * with the information queried by the main thread.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>

typedef struct region_info_s {
    hsa_region_segment_t segment;
    hsa_region_global_flag_t flags;
    size_t size;
    size_t alloc_max_size;
    bool alloc_allowed;
    size_t alloc_granule;
    size_t alloc_alignemnt;
} region_info_t;

// Get memory region info
void get_region_info(hsa_region_t region, region_info_t* info) {
    hsa_status_t status;

    status = hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &info->segment);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_region_get_info(region, HSA_REGION_INFO_GLOBAL_FLAGS, &info->flags);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_region_get_info(region, HSA_REGION_INFO_SIZE, &info->size);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_region_get_info(region, HSA_REGION_INFO_ALLOC_MAX_SIZE, &info->alloc_max_size);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_region_get_info(region, HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED, &info->alloc_allowed);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_region_get_info(region, HSA_REGION_INFO_RUNTIME_ALLOC_GRANULE, &info->alloc_granule);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_region_get_info(region, HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT, &info->alloc_alignemnt);
    ASSERT(HSA_STATUS_SUCCESS == status);
}

typedef struct thread_data_get_region_info_s {
    // The current region
    hsa_region_t region;
    // The region info retrieved from main thread
    region_info_t* info;
    // Consistency check result
    int consistency;
} thread_data_get_region_info_t;

void thread_proc_get_region_info(void* data) {
    thread_data_get_region_info_t* thread_data = (thread_data_get_region_info_t*) data;

    region_info_t info;
    memset(&info, 0, sizeof(region_info_t));
    get_region_info(thread_data->region, &info);

    if (0 == memcmp(thread_data->info, &info, sizeof(region_info_t))) {
        // The region info is consistent with the one got from the main thread
        thread_data->consistency = 1;
    } else {
        thread_data->consistency = 0;
    }
}

int test_memory_region_concurrent_get_info() {
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
            // Get the region info
            region_info_t info;
            memset(&info, 0, sizeof(region_info_t));
            get_region_info(region_list.regions[jj], &info);

            // Prepare thread data
            const int num_threads = 10;
            thread_data_get_region_info_t thread_data[num_threads];
            int kk;
            for (kk = 0; kk < num_threads; ++kk) {
                thread_data[kk].region = region_list.regions[jj];
                thread_data[kk].info = &info;
                thread_data[kk].consistency = 0;
            }

            // Launch threads to get the region info concurrently
            struct test_group* tg_region_info = test_group_create(num_threads);
            for (kk = 0; kk < num_threads; ++kk) {
                test_group_add(tg_region_info, &thread_proc_get_region_info, thread_data + kk, 1);
            }
            test_group_thread_create(tg_region_info);
            test_group_start(tg_region_info);
            test_group_wait(tg_region_info);
            test_group_exit(tg_region_info);
            test_group_destroy(tg_region_info);

            // Verify region info is consistent among all threads
            for (kk = 0; kk < num_threads; ++kk) {
                ASSERT(1 == thread_data[kk].consistency);
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