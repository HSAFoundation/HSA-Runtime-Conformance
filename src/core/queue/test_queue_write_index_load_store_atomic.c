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
 * Test Name: queue_write_index_load_store_atomic
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_queue_write_index_load and store operations
 * are atomic, and 'torn' loads or stores do not occur when these APIs are executed
 * concurrently.
 *
 * Test Description:
 * 1) Create a queue.
 * 2) Create 2 threads, that
 *    a) Update the queue write index value, the first to 0 and the second
 *    to the HSA_AGENT_INFO_QUEUE_MAX_SIZE - 1, using a
 *    hsa_queue_write_index_store operation.
 *    b) Packets should not be initialized for dispatch.
 * 3) Create 2 threads, that
 *    b) Read the signal value, and check if it is 0 or HSA_AGENT_INFO_QUEUE_MAX_SIZE - 1,
 *    using a hsa_write_index_load operation.
 * 4) Run the threads for millions of iterations of loads and stores, with no
 * explicit synchronization between the threads.
 * 5) Repeat for all versions of the hsa_write_index load and store APIs, i.e.
 * for stores us both hsa_store_write_index_acquire and hsa_store_write_index_relaxed
 * and for loads use hsa_load_write_index_release and hsa_load_write_index_release.
 *
 * Expected Results: The reading threads should only see two possible signal values,
 * 0 or INT64_MAX.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>

#define QUEUE_WRITE_INDEX_NUM_OF_LOAD_STORE_ATOMIC     1*128*1024

uint64_t STORE_VALUE;

typedef struct write_index_load_atomic_thread_data_s {
    hsa_queue_t* queue;
    uint64_t num_iterations;
    int memory_ordering_type;
} write_index_load_atomic_thread_data_t;

typedef struct write_index_store_atomic_thread_data_s {
    hsa_queue_t* queue;
    uint64_t store_value;
    uint64_t num_iterations;
    int memory_ordering_type;
} write_index_store_atomic_thread_data_t;

void thread_proc_write_index_load_atomic(void* data) {
    write_index_load_atomic_thread_data_t* thread_data =
        (write_index_load_atomic_thread_data_t*)data;

    int ii;
    for (ii = 0; ii < thread_data->num_iterations; ++ii) {
        uint64_t write_index;
        if (0 == thread_data->memory_ordering_type) {
            write_index = hsa_queue_load_write_index_acquire(thread_data->queue);
        } else if (1 == thread_data->memory_ordering_type) {
            write_index = hsa_queue_load_write_index_relaxed(thread_data->queue);
        } else {
            ASSERT(0);
        }
        // The only two possible values
        ASSERT(0 == write_index || STORE_VALUE == write_index);
    }
}

void thread_proc_write_index_store_atomic(void* data) {
    write_index_store_atomic_thread_data_t* thread_data =
        (write_index_store_atomic_thread_data_t*)data;

    int ii;
    for (ii = 0; ii < thread_data->num_iterations; ++ii) {
        uint64_t write_index;
        if (0 == thread_data->memory_ordering_type) {
            hsa_queue_store_write_index_release(thread_data->queue, thread_data->store_value);
        } else if (1 == thread_data->memory_ordering_type) {
            hsa_queue_store_write_index_relaxed(thread_data->queue, thread_data->store_value);
        } else {
            ASSERT(0);
        }
    }
}

int test_queue_write_index_load_store_atomic() {
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

        // Get queue size
        uint32_t queue_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create a queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], queue_size, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Use a 64-bit value to test the atomicity
        STORE_VALUE = UINT64_MAX;

        // Repeat for all four versions of hsa_queue_cas_write_index
        int memory_ordering_type;
        for (memory_ordering_type = 0; memory_ordering_type < 2; ++memory_ordering_type) {
            // Thread data
            write_index_load_atomic_thread_data_t  load_thread_data[2];
            write_index_store_atomic_thread_data_t store_thread_data[2];
            load_thread_data[0].queue = queue;
            load_thread_data[0].num_iterations = QUEUE_WRITE_INDEX_NUM_OF_LOAD_STORE_ATOMIC;
            load_thread_data[0].memory_ordering_type = memory_ordering_type;
            load_thread_data[1].queue = queue;
            load_thread_data[1].num_iterations = QUEUE_WRITE_INDEX_NUM_OF_LOAD_STORE_ATOMIC;
            load_thread_data[1].memory_ordering_type = memory_ordering_type;
            store_thread_data[0].queue = queue;
            store_thread_data[0].store_value = 0;
            store_thread_data[0].num_iterations = QUEUE_WRITE_INDEX_NUM_OF_LOAD_STORE_ATOMIC;
            store_thread_data[0].memory_ordering_type = memory_ordering_type;
            store_thread_data[1].queue = queue;
            store_thread_data[1].store_value = STORE_VALUE;
            store_thread_data[1].num_iterations = QUEUE_WRITE_INDEX_NUM_OF_LOAD_STORE_ATOMIC;
            store_thread_data[1].memory_ordering_type = memory_ordering_type;

            // Create the test group
            struct test_group* tg = test_group_create(4);
            test_group_add(tg, &thread_proc_write_index_load_atomic, load_thread_data, 1);
            test_group_add(tg, &thread_proc_write_index_load_atomic, load_thread_data  + 1, 1);
            test_group_add(tg, &thread_proc_write_index_store_atomic, store_thread_data, 1);
            test_group_add(tg, &thread_proc_write_index_store_atomic, store_thread_data + 1, 1);
            test_group_thread_create(tg);
            test_group_start(tg);
            test_group_wait(tg);
            test_group_exit(tg);
            test_group_destroy(tg);
        }

        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    return 0;
}
