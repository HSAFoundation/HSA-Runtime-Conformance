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
 *
 * Test Name: concurrent_memory_free
 *
 * Purpose: Test that memory can be freed concurrently for each region
 *
 * Test Description:
 *
 * 1. Iterate over all of the agents in the system and for each agent:
 *
 * 2. Iterate over all of the regions associated with the agent, and for each region:
 *
 * 3. If the region is allocatable, allocate N regions.
 *
 * 4. Launch N child threads to free the memory blocks concurrently.
 *
 * 5. Wait in the main thread until all of the threads finish
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

#define NUM_THREADS 32

typedef struct control_block {
    hsa_region_t* region;
    size_t alloc_size;
    void* alloc_pointer;
} cb_t;

void test_hsa_memory_free_func(void *data) {
    hsa_status_t status;
    cb_t *cb = (cb_t*) data;

    status = hsa_memory_free(cb->alloc_pointer);
    if (status != HSA_STATUS_SUCCESS) {
        cb->alloc_pointer = NULL;
    }

    return;
}

int test_memory_concurrent_free() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    const char *err_str;

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        int jj;
        // Get the list of regions
        struct region_list_s region_list;
        get_region_list(agent_list.agents[ii], &region_list);

        for (jj = 0; jj < region_list.num_regions; ++jj) {
            // Determine if memory can be allocated in this region
            bool allowed;
            status = hsa_region_get_info(region_list.regions[jj], HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED, &allowed);
            if (!allowed) {
                continue;
            }

            // Get the maximum allocation size
            size_t alloc_size;
            status = hsa_region_get_info(region_list.regions[jj], HSA_REGION_INFO_ALLOC_MAX_SIZE, &alloc_size);

            // Adjust the size to the minimum of 1024 or max alloc size
            alloc_size = (alloc_size < 1024) ? alloc_size: 1024;

            // Create a test group
            struct test_group* tg_concurr_init = test_group_create(NUM_THREADS);

            // The control blocks are used to pass data to the threads
            int kk;
            cb_t cb[NUM_THREADS];
            for (kk = 0; kk < NUM_THREADS; kk++) {
                cb[kk].region = &(region_list.regions[jj]);
                cb[kk].alloc_size = alloc_size;
                status = hsa_memory_allocate(*(cb[kk].region), cb[kk].alloc_size, &(cb[kk].alloc_pointer));
                ASSERT(HSA_STATUS_SUCCESS == status);
                test_group_add(tg_concurr_init, &test_hsa_memory_free_func, &cb[kk], 1);
            }

            // Create threads for each test
            test_group_thread_create(tg_concurr_init);

            // Start to run tests
            test_group_start(tg_concurr_init);

            // Wait all tests finish
            test_group_wait(tg_concurr_init);

            // Exit all tests
            test_group_exit(tg_concurr_init);

            // Destroy thread group and cleanup resources
            test_group_destroy(tg_concurr_init);
        }

        free_region_list(&region_list);
    }

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
