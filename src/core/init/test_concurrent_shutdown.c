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
 * Test Name: concurrent_shutdown
 * Scope: Conformance
 *
 * Purpose: Verifies that hsa_shutdown is thread safe with respect
 * to itself
 *
 * Test Description:
 * 1) Call hsa_init N times in the main thread.
 * 2) Query the agent list using hsa_iterate_agents.
 * 3) Create N threads, and from each thread
 *    b) Call hsa_shutdown.
 * 4) Join all the threads to the main thread.
 * 5) Repeat this several times.
 *
 * Expected Results: Verify that all the query operation executes
 * successfully and that no undefined behavior occurs.
 *
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>

#define NUM_TESTS 1000
#define NUM_ITER 5

void test_hsa_shutdown_func(void *data) {
    hsa_status_t status;
    const char *err_str;

    // Shutdown hsa runtime
    status = hsa_shut_down();
    hsa_status_string(status, &err_str);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "\nErr_code: %d Err_Str: %s\n", status, err_str);
    return;
}

int test_concurrent_shutdown() {
    hsa_status_t status;

    // Create a test group
    struct test_group *tg_concurr_shutdown = test_group_create(NUM_TESTS);

    // Add tests into the test group, each test has wrapper_hsa_shutdown
    test_group_add(tg_concurr_shutdown, &test_hsa_shutdown_func, NULL, NUM_TESTS);

    // Create threads for each test
    test_group_thread_create(tg_concurr_shutdown);

    int ii, jj;
    for (jj = 0; jj < NUM_ITER; ++jj) {
        // Initialize hsa runtime num_tests times
        for (ii = 0; ii < NUM_TESTS; ++ii) {
            status = hsa_init();
            ASSERT(HSA_STATUS_SUCCESS==status);
        }

        // Start tests - concurrently shutdown runtime
        test_group_start(tg_concurr_shutdown);
        test_group_wait(tg_concurr_shutdown);
    }

    test_group_exit(tg_concurr_shutdown);
    test_group_destroy(tg_concurr_shutdown);
    return 0;
}
