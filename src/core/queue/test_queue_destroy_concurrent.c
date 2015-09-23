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
 * Test Name: queue_destroy_concurrent
 * Scope: Conformance
 *
 * Purpose: Verifies that queues can be destroyed concurrently.
 *
 * Test Description:
 * 1) For all agents in the system,
 *    a) Query the agent to determine its HSA_AGENT_INFO_QUEUES_MAX
 *    parameter.
 *    b) Create HSA_AGENT_INFO_QUEUES_MAX queues for that agent.
 *    c) The queues should be stored in a global array, in no particular
 *    order.
 * 2) Create n threads, each assigned a range in the array. Each thread will
 *    a) Destroy m queues, such that n * m = Total # of queues.
 * 7) Repeat this several times.
 *
 * Expected Results: All queues should be successfully created and destroyed.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>

#define N_THREADS 16

static int m_queues;

hsa_queue_t **queues;

typedef struct queue_destroy_params {
    hsa_queue_t** queues;
    int count;
} queue_destroy_params_t;

// work function for destroying queues
void test_destroy_queue(void *data) {
    int ii;
    hsa_status_t status;

    // Get offset of queues
    queue_destroy_params_t* params = (queue_destroy_params_t*)data;

    for (ii = 0; ii < params->count; ++ii) {
        status = hsa_queue_destroy(params->queues[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }
}

int test_queue_destroy_concurrent() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        uint32_t queue_max = 0;
        int jj;

        // Get max number of queues that is supported by the agent
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queue_max);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate queue pointers for current agent.
        queues = (hsa_queue_t **) malloc(sizeof(hsa_queue_t *) * queue_max);
        memset(queues, 0, sizeof(hsa_queue_t*) * queue_max);

        // Create queues on current agent.
        for (jj = 0; jj < queue_max; ++jj) {
            status = hsa_queue_create(agent_list.agents[ii], 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queues[jj]);
            if (HSA_STATUS_SUCCESS != status) {
                ASSERT(HSA_STATUS_ERROR_OUT_OF_RESOURCES == status);
                queue_max = jj;
            }
        }

        // Calculate the number of queues assigned to each thread to be destroyed.
        // Each thread may have different num of queues, depending on (0 == queue_max % N_THREADS).
        queue_destroy_params_t params[N_THREADS];

        int k;
        for (k = 0; k < N_THREADS; ++k) {
            params[k].count = queue_max / N_THREADS;
            if ((queue_max % N_THREADS != 0) && k + 1 <= (queue_max % N_THREADS - 1)) {
                params[k].count += 1;
            }
        }
        int total_count = 0;
        for (k = 0; k < N_THREADS; ++k) {
            params[k].queues = &(queues[total_count]);
            total_count += params[k].count;
        }

        struct test_group *tg_concurrent_create_queue = test_group_create(N_THREADS);

        for (k = 0; k < N_THREADS; ++k) {
            test_group_add(tg_concurrent_create_queue, &test_destroy_queue, &params[k], 1);
        }

        test_group_thread_create(tg_concurrent_create_queue);
        test_group_start(tg_concurrent_create_queue);
        test_group_wait(tg_concurrent_create_queue);
        test_group_exit(tg_concurrent_create_queue);
        test_group_destroy(tg_concurrent_create_queue);

        free(queues);
    }

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
