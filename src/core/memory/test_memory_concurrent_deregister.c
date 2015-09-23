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
 * Test Name: concurrent_memory_deregister
 * Purpose: Test that if memory can be deregistered concurrently.
 *
 * Test Description:
 *
 * 1. Malloc a block of memory using a system allocation API (i.e. malloc())
 *
 * 2. Divide this block into N segments and register the segments sequentially.
 *
 * 3. Launch 10 child threads to deregister the segments concurrently.
 *
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

#define NUM_THREADS 10
#define BLOCK_SIZE 1024

void test_hsa_deregister_func(void *data) {
    hsa_status_t status;
    status = hsa_memory_deregister(data, BLOCK_SIZE * sizeof(char));
    ASSERT(status == HSA_STATUS_SUCCESS);
    return;
}

int test_memory_concurrent_deregister() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    char *ptr;
    ptr = (char*) malloc(NUM_THREADS * BLOCK_SIZE * sizeof(char));

    // Register the memory segments
    int ii;
    for (ii = 0; ii < NUM_THREADS; ++ii) {
        status = hsa_memory_register(ptr + (ii * BLOCK_SIZE), BLOCK_SIZE * sizeof(char));
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Create a test group
    struct test_group * tg_concurr_init = test_group_create(NUM_THREADS);

    // Add the tests into the test group
    for (ii = 0; ii < NUM_THREADS; ii++) {
        test_group_add(tg_concurr_init, &test_hsa_deregister_func, ptr + (ii * BLOCK_SIZE), 1);
    }

    // Create threads for each test
    test_group_thread_create(tg_concurr_init);

    // Start to run tests
    test_group_start(tg_concurr_init);

    // Wait all tests finish
    test_group_wait(tg_concurr_init);

    // Exit all tests
    test_group_exit(tg_concurr_init);

    for (ii = 0; ii < NUM_THREADS; ii++) {
       status = test_group_test_status(tg_concurr_init, ii);
       ASSERT(TEST_ERROR != status);
    }

    // Destroy tests, cleanup resources
    test_group_destroy(tg_concurr_init);

    free(ptr);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
