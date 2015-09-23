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
 * Test Name: queue_create_concurrent
 * Scope: Conformance
 *
 * Purpose: Verifies that queues can be created concurrently
 * using a single agent as a parameter. All agents should
 * be checked.
 *
 * Test Description:
 * 1) For each agent in the system,
 *    a) Query the agent to determine its HSA_AGENT_INFO_QUEUES_MAX
 *    parameter.
 *    b) Create n threads, each of which attempts to create m queues
 *    such that m * n > HSA_AGENT_INFO_QUEUES_MAX.
 *    c) The threads should gracefully exit, and not indicate a test
 *    failure if HSA_STATUS_ERROR_OUT_OF_RESOURCES is returned.
 *    d) Each thread should increment a global, atomic value that
 *    indicates the total number of queues created.
 * 2) After all threads have returned, count the number of created queues.
 * This number should be equal to the maximum number of queues supported
 * by the agent.
 * 3) Check that each queue has a unique queue_id.
 * 4) Check that the read pointer and writer pointer are
 * initialized to 0.
 * 5) Create a simple kernel and verify that it will run on each queue.
 * 6) Destroy all queues in the main thread with hsa_queue_destroy.
 * 7) Repeat this for the same agent several times.
 *
 * Expected Results: All queues should be successfully created and all properties
 * should be initialized correctly.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>
#include <stdlib.h>

#define N_THREADS 8

static int m_queues;
static int num_queues;

static pthread_mutex_t queue_idx_mutex = PTHREAD_MUTEX_INITIALIZER;

hsa_queue_t **queues;

// Function to compare agent ids
int compare_agent_ids(const void *a, const void *b) {
    return *((uint32_t*) a) - *((uint32_t*) b);
}

// Work function for creating queues
void test_create_queue(void *data) {
    hsa_status_t status;
    hsa_agent_t* agent = (hsa_agent_t *)data;
    hsa_queue_t* queue;

    // Create m_queues queue
    int ii;
    for (ii = 0; ii < m_queues; ++ii) {
        status = hsa_queue_create(*agent, 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);

       // Check status, if status is HSA_STATUS_ERROR_OUT_OF_RESOURCES, it
       // indicates the total number of queues reaches the upper bound
       ASSERT(status == HSA_STATUS_SUCCESS || status == HSA_STATUS_ERROR_OUT_OF_RESOURCES);

       if (HSA_STATUS_SUCCESS == status) {
            pthread_mutex_lock(&queue_idx_mutex);
            queues[num_queues] = queue;
            ++num_queues;
            pthread_mutex_unlock(&queue_idx_mutex);
        }
    }
}

int test_queue_create_concurrent() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("no_op.brig", &module));

    // Get agent list
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int jj;
    // For each agent
    for (jj = 0; jj < agent_list.num_agents; ++jj) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);

        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            // Continue if this agent does not support DISPATCH
            continue;
        }

        uint32_t queue_max;

        m_queues = 4;
        num_queues = 0;

        // Get max number of queues
        status = hsa_agent_get_info(agent_list.agents[jj], HSA_AGENT_INFO_QUEUES_MAX, &queue_max);
        ASSERT(status == HSA_STATUS_SUCCESS);

        // Set the total number of queues larger than the max number of queues
        while ((N_THREADS * m_queues) <= queue_max) {
            m_queues *= 2;
        }

        queues = (hsa_queue_t **) malloc(sizeof(hsa_queue_t *) * N_THREADS * m_queues);

        struct test_group *tg_concurrent_create_queue = test_group_create(N_THREADS);

        test_group_add(tg_concurrent_create_queue, &test_create_queue, agent_list.agents + jj, N_THREADS);
        test_group_thread_create(tg_concurrent_create_queue);
        test_group_start(tg_concurrent_create_queue);
        test_group_wait(tg_concurrent_create_queue);
        test_group_exit(tg_concurrent_create_queue);
        test_group_destroy(tg_concurrent_create_queue);

        // Create an array to store id for every queue
        uint32_t ids[num_queues];

        int ii;
        for (ii = 0; ii < num_queues; ++ii) {
            ids[ii] = queues[ii]->id;

            // check if the read_pointer and write_pointer are initialized to zero
            uint64_t read_pointer, write_pointer;
            read_pointer = hsa_queue_load_read_index_acquire(queues[ii]);
            ASSERT(read_pointer == 0);

            write_pointer = hsa_queue_load_write_index_acquire(queues[ii]);
            ASSERT(write_pointer == 0);
        }

        // Sort ids
        qsort(ids, num_queues, sizeof(uint32_t), compare_agent_ids);

        // Check if two ids are same
        for (ii = 1; ii < num_queues; ++ii) {
            ASSERT_MSG(ids[ii - 1] != ids[ii], "the ids of queues are not unique\n");
        }

        // Finalize the executable
        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
        hsa_code_object_t code_object;
        hsa_executable_t executable;

        status = finalize_executable(agent_list.agents[jj],
                                     1,
                                     &module,
                                     HSA_MACHINE_MODEL_LARGE,
                                     HSA_PROFILE_FULL,
                                     HSA_DEFAULT_FLOAT_ROUNDING_MODE_ZERO,
                                     HSA_CODE_OBJECT_TYPE_PROGRAM,
                                     0,
                                     control_directives,
                                     &code_object,
                                     &executable);

        ASSERT(HSA_STATUS_SUCCESS == status);

        // Get the symbol and the symbol info
        symbol_record_t symbol_record;
        memset(&symbol_record, 0, sizeof(symbol_record_t));

        char* symbol_names[1];
        symbol_names[0] = "&__no_op_kernel";
        status = get_executable_symbols(executable, agent_list.agents[jj], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create a completion signal.
        hsa_signal_t signal;
        status = hsa_signal_create(1, 0, NULL, &signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Fill in info for the default dispatch packet
        hsa_kernel_dispatch_packet_t dispatch_packet;
        memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
        dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        dispatch_packet.setup  |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet.workgroup_size_x = 256;
        dispatch_packet.workgroup_size_y = 1;
        dispatch_packet.workgroup_size_z = 1;
        dispatch_packet.grid_size_x = 256;
        dispatch_packet.grid_size_x = 1;
        dispatch_packet.grid_size_x = 1;
        dispatch_packet.group_segment_size = symbol_record.group_segment_size;
        dispatch_packet.private_segment_size = symbol_record.private_segment_size;
        dispatch_packet.kernel_object = symbol_record.kernel_object;
        dispatch_packet.kernarg_address = 0;
        dispatch_packet.completion_signal = signal;

        for (ii = 1; ii < num_queues; ++ii) {
            // Reset the signal value
            hsa_signal_store_relaxed(signal, 1);

            // launch the kernel
            enqueue_dispatch_packet(queues[ii], &dispatch_packet);

            // Wait for the kernel to finish
            hsa_signal_value_t value = hsa_signal_wait_relaxed(signal,
                                                               HSA_SIGNAL_CONDITION_EQ,
                                                               0,
                                                               UINT64_MAX,
                                                               HSA_WAIT_STATE_BLOCKED);
            ASSERT(value == 0);
        }

        // Destroy the signal
        status = hsa_signal_destroy(signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy queues
        for (ii = 0; ii < num_queues; ++ii) {
            hsa_queue_destroy(queues[ii]);
        }

        free(queues);
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    return 0;
}
