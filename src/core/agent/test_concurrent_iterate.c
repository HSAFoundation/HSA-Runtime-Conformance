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
 * Test Name: concurrent_iterate
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_iterate_agents API is thread safe.
 *
 * Test Description:
 * 1) Use hsa_iterate_agents to obtain a list of agents on the system
 * and cache the result.
 * 2) Create several threads that concurrently,
 *    a) Call hsa_iterate_agents to obtain a new list of agents.
 *    b) Compare the list of agent handles to the original list.
 * 3) Repeat this several times.
 *
 * Expected Results: The concurrently generated lists should match the
 * initial list.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>

#define NUM_ITER 10
#define NUM_TESTS 10

void wrapper_check_agent_list(void *data) {
    struct agent_list_s *agent_list_orig = (struct agent_list_s *)data;

    // Get a new list of agents
    struct agent_list_s agent_list_new;
    get_agent_list(&agent_list_new);

    // Check if the number of agents in the new list is identical
    // to the original list
    ASSERT(agent_list_orig->num_agents == agent_list_new.num_agents);

    // Check agent handles to the original list
    int ii;
    for (ii = 0; ii < agent_list_orig->num_agents; ii++)
        ASSERT(agent_list_orig->agents[ii].handle ==
               agent_list_new.agents[ii].handle);

    free_agent_list(&agent_list_new);

    return;
}

int test_concurrent_iterate() {
    int num_iter = NUM_ITER;
    int num_threads = NUM_TESTS;

    // Repeat the test num_iter times
    int ii;
    for (ii = 0; ii < num_iter; ++ii) {
        hsa_status_t status;

        // Init hsa runtime
        status = hsa_init();
        ASSERT(status == HSA_STATUS_SUCCESS);

        // Get a list of agent
        struct agent_list_s agent_list_orig;
        get_agent_list(&agent_list_orig);

        // Create a test group
        struct test_group *tg_agent = test_group_create(num_threads);

        // Add test functions with num_threads copies
        test_group_add(tg_agent, &wrapper_check_agent_list,
                       &agent_list_orig, num_threads);

        // Create threads for the test group
        test_group_thread_create(tg_agent);

        // Start tests
        test_group_start(tg_agent);

        // Wait all tests finish
        test_group_wait(tg_agent);

        // Exit all test threads
        test_group_exit(tg_agent);

        // Clean up resources
        test_group_destroy(tg_agent);

        free_agent_list(&agent_list_orig);

        // Shutdown runtime
        status = hsa_shut_down();
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    return 0;
}
