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
 * Test Name: queue_cas_write_index_acquire_release_ordering
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_queue_cas_write_index_release and
 * hsa_queue_cas_write_index_acquire APIs enforce correct memory
 * ordering.
 *
 * Test Description:
 * 1) Query the platform for a list of agents that support the
 * HSA_AGENT_FEATURE_KERNEL_DISPATCH feature.
 * 2) For each agent,
 * 3) Query the HSA_AGENT_INFO_QUEUES_MAX parameter.
 * 4) Create HSA_AGENT_INFO_QUEUES_MAX queues.
 * 5) Creates one thread that:
 *    a) Loops on the first queue's write_index value, setting the value
 *    to v + 1 when v % 2 == 0 with hsa_queue_cas_write_index_acquire.
 *    b) Attempts to set all the other queue's except the first and last,
 *    write index to v + 1 with hsa_queue_cas_write_index_relaxed,
 *    expecting the API to return v.
 *    c) Loops on the last queue's write_index value, setting the value
 *    to v + 1 when v % 2 == 0 with hsa_queue_cas_write_index_release.
 *    d) Terminates when v = termination_value.
 * 6) Create a second thread that:
 *    a) Loops on the first queue's write_index value, setting the value
 *    to v + 1 when v % 2 == 1 with hsa_queue_cas_write_index_acquire.
 *    b) Attempts to set all the other queue's except the first and last,
 *    write index to v + 1 with hsa_queue_cas_write_index_relaxed,
 *    expecting the API to return v.
 *    c) Loops on the last queue's write_index value, setting the value
 *    to v + 1 when v % 2 == 1 with hsa_queue_cas_write_index_release.
 *    d) Terminates when v = termination_value+1.
 *
 * Expected Results: The value of v should monotonically increase, and
 * the queues that are operated on using the relaxed versions of the
 * API should all have the expected value.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>

typedef struct write_index_cas_thread_data_s {
    hsa_queue_t** queues;
    int num_queues;
    uint64_t termination_value;
} write_index_cas_thread_data_t;

void thread_proc_write_index_cas_acquire_release_even(void* data) {
    write_index_cas_thread_data_t* thread_data = (write_index_cas_thread_data_t*)data;
    int ii;
    for (ii = 2; ii < thread_data->termination_value; ii += 2) {
        uint64_t v = (uint64_t)ii;

        // This is to verify the write_indices of all queues in the middle.
        uint64_t old_v = v - 1;

        // Increment the write_index of the first queue.
        while (old_v != hsa_queue_cas_write_index_acquire(
                thread_data->queues[0], old_v, v)) {}

        // Verify the write_indices of queues in the middle has been updated
        // by the other thread.
        // Also, increment the write_indices.
        int jj;
        for (jj = 1; jj < thread_data->num_queues - 1; ++jj) {
            ASSERT(hsa_queue_cas_write_index_relaxed(thread_data->queues[jj], old_v, v) == old_v);
        }

        // This doesn't have to be in a loop.
        // Increment the write_index of the first queue.
        ASSERT(old_v == hsa_queue_cas_write_index_release(thread_data->queues[thread_data->num_queues - 1], old_v, v));
    }
}

void thread_proc_write_index_cas_acquire_release_odd(void* data) {
    // Since the write_index of all queues are initialized to 0, this thread
    // will get to run first
    write_index_cas_thread_data_t* thread_data = (write_index_cas_thread_data_t*)data;
    int ii;
    for (ii = 1; ii < thread_data->termination_value; ii += 2) {
        uint64_t v = (uint64_t)ii;

        // This is to verify the write_indices of all queues in the middle.
        uint64_t old_v = v - 1;

        // Increment the write_index of the last queue.
        while (old_v != hsa_queue_cas_write_index_acquire(
                thread_data->queues[thread_data->num_queues - 1], old_v, v)) {}

        // Verify the write_indices of queues in the middle has been updated
        // by the other thread.
        // And increment the write_indices.
        int jj;
        for (jj = thread_data->num_queues - 2; jj >= 1; --jj) {
            ASSERT(hsa_queue_cas_write_index_relaxed(thread_data->queues[jj], old_v, v) == old_v);
        }

        // This doesn't have to be in a loop.
        // Increment the write_index of the first queue.
        ASSERT(old_v == hsa_queue_cas_write_index_release(thread_data->queues[0], old_v, v));
    }
}

int test_queue_write_index_cas_acquire_release_ordering() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Get max number of queues
        uint32_t queue_max;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queue_max);
        ASSERT(HSA_STATUS_SUCCESS == status);

        if (queue_max < 1) {
            // This agent does not support any queue
            continue;
        }

        uint32_t queue_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the queues
        hsa_queue_t** queues = (hsa_queue_t**)malloc(queue_max * sizeof(hsa_queue_t*));

        int num_queues = 0;
        while (num_queues < queue_max) {
            status = hsa_queue_create(agent_list.agents[ii], queue_size, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, queues + num_queues);
            if (HSA_STATUS_ERROR_OUT_OF_RESOURCES == status) {
                break;
            }
            ASSERT(HSA_STATUS_SUCCESS == status);
            ++num_queues;
        }

        // Thread data
        write_index_cas_thread_data_t thread_data;
        thread_data.queues = queues;
        thread_data.num_queues = num_queues;
        // Choose a termination_value >= 2
        thread_data.termination_value = 256;

        // Create the test group
        struct test_group* tg = test_group_create(2);
        test_group_add(tg, &thread_proc_write_index_cas_acquire_release_even, &thread_data, 1);
        test_group_add(tg, &thread_proc_write_index_cas_acquire_release_odd, &thread_data, 1);
        test_group_thread_create(tg);
        test_group_start(tg);
        test_group_wait(tg);
        test_group_exit(tg);
        test_group_destroy(tg);

        int jj;
        for (jj = 0; jj < num_queues; ++jj) {
            status = hsa_queue_destroy(queues[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        free(queues);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    return 0;
}
