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
 * Test Name: signal_add_acquire_release_ordering
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_add_release and the
 * hsa_signal_add_acquire APIs enforce correct memory ordering.
 *
 * Test Description:
 * 1) Create several signals and store the handles in an array,
 * denoted by x[]. All the signal values should be initialized to 1.
 * 2) Create a control signal, denoted by y, also initialized to 1.
 * 3) Start one thread that
 *    a) Check the value of y in a loop using hsa_cas_relaxed using
 *    -1 as the exchange value.
 *    b) When the value of y is 1 it sets the value, the thread stops looping, and
 *    d) The value of y is incremented to 0 using hsa_signal_add_acquire.
 *    e) Checks all of the x signal values with the signal_load_relaxed
 *    API, expecting a value of 2, and replacing it with a value of 1 by
 *    calling signal_add_relaxed to add -1 to the value.
 *    f) Sets the value of y to 2, using the signal_add_release API
 *    to add value of 2.
 *    g) Starts over.
 * 4) Start a second thread that does exactly the same set of operations,
 * but uses 1 in place of 2, 2 in place of 1 and 1 in place of -1.
 * 5) Let both threads run for millions of iterations.
 *
 * Expected Results: For each cycle, the reported x values for the first thread should be 2
 * and the reported x values for the second thread should be 1, i.e. all memory operations
 * that occured in thread one should be appropriately ordered in thread two after
 * the y signal value was modified, and vice versa.
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>
#include "config.h"

hsa_signal_t y;
hsa_signal_t x[NUM_X];

// Check y, if y equal to 1, set y to 0, and then check if all x values equal to 2 and set them to 1, and then set y value to 2
void test_signal_add_acquire_release_t1(void *data) {
    int ii, jj;
    // repeat NUM_ITER_MEM_ORD times
    for (jj = 0; jj < NUM_ITER_MEM_ORD; ++jj) {
        while (hsa_signal_cas_acq_rel(y, 1, -1) != 1);
        hsa_signal_add_acquire(y, 1);

        for (ii = 0; ii < NUM_X; ++ii) {
            hsa_signal_value_t sig_val_t1 = hsa_signal_load_relaxed(x[ii]);
            ASSERT(sig_val_t1 == 2);

            hsa_signal_add_relaxed(x[ii], -1);
        }

        hsa_signal_add_release(y, 2);
    }
    return;
}


// Check y, if y equal to 2, set y to 0, and then check if all x values equal to 1 and set them to 2, and then set y value to 1
void test_signal_add_acquire_release_t2(void *data) {
    int ii, jj;
    // repeat NUM_ITER_MEM_ORD times
    for (jj = 0; jj < NUM_ITER_MEM_ORD; ++jj) {
        while (hsa_signal_cas_acq_rel(y, 2, -1) != 2);
        hsa_signal_add_acquire(y, 1);

        for (ii = 0; ii < NUM_X; ++ii) {
            hsa_signal_value_t sig_val_t2 = hsa_signal_load_relaxed(x[ii]);
            ASSERT(sig_val_t2 == 1);

            hsa_signal_add_relaxed(x[ii], 1);
        }

        hsa_signal_add_release(y, 1);
    }
    return;
}

int test_signal_add_acquire_release_ordering() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    int ii;
    for (ii = 0; ii < NUM_X; ++ii) {
        // initialize all x values to 2
        status = hsa_signal_create(2, 0, NULL, x + ii);
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    // initialize y to 1
    status = hsa_signal_create(1, 0, NULL, &y);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // create test_group
    struct test_group *test = test_group_create(2);

    // add test func one to the test group
    test_group_add(test, test_signal_add_acquire_release_t1, NULL, 1);

    // add test func two to the test_group
    test_group_add(test, test_signal_add_acquire_release_t2, NULL, 1);

    // create threads for each test
    test_group_thread_create(test);

    // start test functions
    test_group_start(test);

    // wait all tests functions finish
    test_group_wait(test);

    // exit all tests
    test_group_exit(test);

    // cleanup resources
    test_group_destroy(test);

    for (ii = 0; ii < NUM_X; ++ii) {
        status = hsa_signal_destroy(x[ii]);
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    status = hsa_signal_destroy(y);
    ASSERT(status == HSA_STATUS_SUCCESS);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);
    return 0;
}
