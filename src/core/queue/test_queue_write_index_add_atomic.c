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
 * Test Name: queue_write_index_add_atomic
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_queue_write_index_add operations is atomic,
 * and 'torn' adds do not occur when this API is executed concurrently.
 *
 * Test Description:
 * 1) Create a queue, with an initial write index value of 0.
 * 2) Create 4 threads, that
 *    a) Call hsa_signal_add in a loop, incrementing the value of
 *       the signal by 1.
 *    b) Each performs millions of additions to the write index value.
 * 3) After the threads have finished, check the final index value.
 * 4) Repeat several times.
 * 5) Repeat for all versions of the hsa_queue_write_index_add APIs, i.e. acquire,
 * release, relaxed and acq_rel memory ordering versions.
 *
 * Expected Results: The final write index value should equal the sum of
 * all of the additions from all threads.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>

#define QUEUE_WRITE_INDEX_NUM_OF_ADD_ATOMIC     1*1024*1024

typedef struct write_index_add_atomic_data_s {
    hsa_queue_t* queue;
    int memory_ordering_type;
} write_index_add_atomic_data_t;

void thread_proc_write_index_add_atomic(void* data) {
    write_index_add_atomic_data_t* thread_data = (write_index_add_atomic_data_t*)data;
    int ii;
    for (ii = 0; ii < QUEUE_WRITE_INDEX_NUM_OF_ADD_ATOMIC; ++ii) {
        switch (thread_data->memory_ordering_type) {
        case 0:
            hsa_queue_add_write_index_acq_rel(thread_data->queue, 1);
            break;
        case 1:
            hsa_queue_add_write_index_acquire(thread_data->queue, 1);
            break;
        case 2:
            hsa_queue_add_write_index_relaxed(thread_data->queue, 1);
            break;
        case 3:
            hsa_queue_add_write_index_release(thread_data->queue, 1);
            break;
        default:
            break;
        }
    }
}

int test_queue_write_index_add_atomic(void) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // check if the queue supports dispatch
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

        // Get max number of queues
        uint32_t queue_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queue_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create a queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], queue_size, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        int jj;
        const int repeat = 2;
        for (jj = 0; jj < repeat; ++jj) {
            int memory_ordering_type;
            for (memory_ordering_type = 0; memory_ordering_type < 4; ++memory_ordering_type) {
                // Thread data
                write_index_add_atomic_data_t thread_data;
                thread_data.queue = queue;
                thread_data.memory_ordering_type = memory_ordering_type;

                // Create the test group
                const int num_threads = 4;
                struct test_group* tg = test_group_create(num_threads);
                int kk;
                for (kk = 0; kk < num_threads; ++kk) {
                    test_group_add(tg, &thread_proc_write_index_add_atomic, &thread_data, 1);
                }
                test_group_thread_create(tg);
                test_group_start(tg);
                test_group_wait(tg);
                test_group_exit(tg);
                test_group_destroy(tg);

                // Verify the write_index
                uint64_t write_index = hsa_queue_load_write_index_relaxed(queue);
                uint64_t expected = (uint64_t)(QUEUE_WRITE_INDEX_NUM_OF_ADD_ATOMIC * num_threads);
                ASSERT(expected == write_index);

                // Restore the write_index of the queue
                hsa_queue_store_write_index_release(queue, 0);
            }
        }

        // Destroy queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    return 0;
}
