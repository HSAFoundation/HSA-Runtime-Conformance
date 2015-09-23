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
 * Test Name: queue_cas_write_index_atomic
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_queue_cas_write_index operations is atomic,
 * and 'torn' compare and swaps do not occur when this API is executed
 * concurrently.
 *
 * Test Description:
 * 1) Create a signal, assigning it an initial value of 0.
 * 2) Create 4 threads, that
 *    a) Call hsa_queue_cas_write_index in a loop, comparing the value
 *       to an expected value and then advancing the value to the
 *       next value in the cycle.
 *    b) Thread 0 will exchange value to value + 1 when value%4=0
 *    c) Thread 1 will exchange value to value + 1 when value%4=1
 *    d) Thread 2 will exchange value to value + 1 when value%4=2
 *    e) Thread 3 will exchange value to value + 1 when value%4=3
 * 4) Run the threads for millions of iterations of exchanges, with no
 * explicit synchronization between the threads.
 * 5) Repeat for all versions of the hsa_queue_case_write_index APIs, i.e. acquire,
 * release, relaxed and acq_rel memory ordering versions.
 *
 * Expected Results: The value of the write index should increase monotonically, and
 * advance through all expected values.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>

#define QUEUE_WRITE_INDEX_NUM_OF_CAS_ATOMIC     1*1024*1024

typedef struct write_index_cas_thread_data_s {
    hsa_queue_t* queue;
    int thread_index;
    int num_threads;
    uint64_t termination_value;
    int memory_ordering_type;
} write_index_cas_thread_data_t;

void thread_proc_write_index_cas_atomic(void* data) {
    write_index_cas_thread_data_t* thread_data = (write_index_cas_thread_data_t*)data;

    int ii;
    for (ii = thread_data->thread_index; ii < thread_data->termination_value; ii += thread_data->num_threads) {
        switch (thread_data->memory_ordering_type) {
        case 0:
            while ((uint64_t)ii != hsa_queue_cas_write_index_acq_rel(thread_data->queue, ii, ii + 1))
            {}
            break;
        case 1:
            while ((uint64_t)ii != hsa_queue_cas_write_index_acquire(thread_data->queue, ii, ii + 1))
            {}
            break;
        case 2:
            while ((uint64_t)ii != hsa_queue_cas_write_index_relaxed(thread_data->queue, ii, ii + 1))
            {}
            break;
        case 3:
            while ((uint64_t)ii != hsa_queue_cas_write_index_release(thread_data->queue, ii, ii + 1))
            {}
            break;
        }
    }
}

int test_queue_write_index_cas_atomic() {
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
            // This agent does not support any queues
            continue;
        }

        // Get the queue size
        uint32_t queue_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create a queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], queue_size, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Repeat for all four versions of hsa_queue_cas_write_index
        int memory_ordering_type;
        for (memory_ordering_type = 0; memory_ordering_type < 4; ++memory_ordering_type) {
            // Thread data
            const int num_threads = 4;
            write_index_cas_thread_data_t thread_data[num_threads];

            // Create the test group
            struct test_group* tg = test_group_create(num_threads);
            int jj;
            for (jj = 0; jj < num_threads; ++jj) {
                thread_data[jj].queue = queue;
                thread_data[jj].thread_index = jj;
                thread_data[jj].num_threads = num_threads;
                thread_data[jj].memory_ordering_type = memory_ordering_type;
                thread_data[jj].termination_value = QUEUE_WRITE_INDEX_NUM_OF_CAS_ATOMIC;
                test_group_add(tg, &thread_proc_write_index_cas_atomic, thread_data + jj, 1);
            }
            test_group_thread_create(tg);
            test_group_start(tg);
            test_group_wait(tg);
            test_group_exit(tg);
            test_group_destroy(tg);

            // Verify the write_index
            uint64_t write_index = hsa_queue_load_write_index_relaxed(queue);
            uint64_t expected = (uint64_t)(QUEUE_WRITE_INDEX_NUM_OF_CAS_ATOMIC);
            ASSERT(expected == write_index);

            // Restore the write_index of the queue
            hsa_queue_store_write_index_release(queue, 0);
        }

        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    return 0;
}
