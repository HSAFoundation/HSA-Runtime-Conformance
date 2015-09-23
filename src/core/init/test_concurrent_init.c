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
 * Test Name: concurrent_init
 * Scope: Conformance
 *
 * Purpose: Verifies that hsa_init is thread safe with respect
 * to itself
 *
 * Test Description:
 * 1) Create N threads, and from each thread
 *    a) Call hsa_init to initialize the HSA runtime
 * 2) Join all the threads to the main thread.
 * 3) Shutdown the runtime by calling hsa_shutdown N times.
 *
 * Expected Results: Verify that the query operation executes successfully
 * and that no undefined behavior occurs.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>

#define NUM_TESTS 100

void test_hsa_init_func(void *data) {
    hsa_status_t status;
    const char *err_str;

    // Initialize hsa runtime
    status = hsa_init();
    hsa_status_string(status, &err_str);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "\nErr_code: %d Err_Str: %s\n", status, err_str);
    return;
}


int test_concurrent_init() {
    hsa_status_t status;
    const char *err_str;

    // Create a test group
    struct test_group * tg_concurr_init = test_group_create(NUM_TESTS);

    // Add test into the test group, each test has same test function - wrapper_hsa_init
    test_group_add(tg_concurr_init, &test_hsa_init_func, NULL, NUM_TESTS);

    // Create threads for each test
    test_group_thread_create(tg_concurr_init);

    // Start to run tests
    test_group_start(tg_concurr_init);

    // Wait all tests finish
    test_group_wait(tg_concurr_init);

    // Exit all tests
    test_group_exit(tg_concurr_init);

    // Shutdown hsa runtime num_tests times
    int ii;
    for (ii = 0; ii < NUM_TESTS; ++ii) {
        status = hsa_shut_down();
        hsa_status_string(status, &err_str);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "\nErr_code: %d Err_Str: %s\n", status, err_str);
    }

    // Destroy tests, cleanup resources
    test_group_destroy(tg_concurr_init);

    return 0;
}
