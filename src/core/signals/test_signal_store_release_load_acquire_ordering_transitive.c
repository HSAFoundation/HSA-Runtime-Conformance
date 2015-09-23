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
 * Test Name: signal_store_release_load_acquire_ordering_transitive
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_load_acquire and
 * hsa_signal_store_release APIs enforce correct memory
 * ordering.
 *
 * Test Description:
 * 1) Create several signals and store the handles in an array,
 * denoted by x[]. All the signal values should be initialized
 * to 0.
 * 2) Create two control signals, denoted by y and z. The initial
 * y signal value should be 0 and the z signal value should be 0.
 * 3) Start one thread that
 *    a) Check the value of y in a loop using signal_load_acquire.
 *    b) When the value of y is 0, the thread stops looping and
 *    c) Checks all of the x signal values with the signal_load_relaxed
 *    API, expecting a value of 0.
 *    d) Changes all of the x signal values to 1 using the
 *    signal_store_relaxed API.
 *    e) Sets the value of y to 1, using the signal_store_release API.
 *    f) Starts over.
 *    g) If it detects that the value of y is -1, it terminates.
 * 4) Start a second thread that does exactly the same set of operations,
 * but uses 0 in place of 1 and 1 in place of 0, and operates on signal z
 * instead of signal y.
 * 5) Start a third thread that
 *    a) Loops on the  value of y using signal_load_acquire to get the value.
 *    b) When the value of y is 1, the thread stops looping and
 *    c) Sets the value of z to 1 using signal_store_release.
 *    d) Loops the value of z using signal_load_acquire to get the value.
 *    e) When the value of z is 0, the thread stops looping and
 *    f) Set the value of y to 0 and starts over.
 *    g) After a set number of iterations the third thread should set
 *       both y and z signal values to -1 and terminate.
 * 5) Let both threads run for millions of iterations.
 *
 * Expected Results: For each cycle, the x values for the first thread should be 1
 * and the x values for the second thread should be 0, i.e. all memory operations
 * that occured in thread one should be appropriately ordered in thread two after
 * the y signal value was modified, and vice versa.
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>
#include "config.h"

hsa_signal_t y;
hsa_signal_t z;
hsa_signal_t x[NUM_X];

void test_signal_store_release_load_acquire_ordering_t1(void *data) {
    int ii;
    while (1) {
        hsa_signal_value_t y_val = 0;
        // loop till y = 0 or y = -1
        while ((y_val = hsa_signal_load_acquire(y)) != 0) {
            if (y_val == -1) {
                return;
            }
        }

        for (ii = 0; ii < NUM_X; ++ii) {
            // every x value should equal to 0
            hsa_signal_value_t sig_val_t1 = hsa_signal_load_relaxed(x[ii]);
            ASSERT_MSG(sig_val_t1 == 0, "signal value should be 0, but is %d\n", sig_val_t1);

            // change x value to 1
            hsa_signal_store_relaxed(x[ii], 1);
        }

        // set y to 1
        hsa_signal_store_release(y, 1);
    }
    return;
}


void test_signal_store_release_load_acquire_ordering_t2(void *data) {
    int ii;
    while (1) {
        hsa_signal_value_t z_val = 0;
        // loop till z = 0 or z = -1
        while ((z_val = hsa_signal_load_acquire(z)) != 1) {
            if (z_val == -1) {
                return;
            }
        }

        for (ii = 0; ii < NUM_X; ++ii) {
            // every x value should equal to 1
            hsa_signal_value_t sig_val_t1 = hsa_signal_load_relaxed(x[ii]);
            ASSERT_MSG(sig_val_t1 == 1, "signal value should be 0, but is %d\n", sig_val_t1);

            // change x value to 1
            hsa_signal_store_relaxed(x[ii], 0);
        }

        // set z to 1
        hsa_signal_store_release(z, 0);
    }
    return;
}

void test_signal_store_release_load_acquire_ordering_t3(void *data) {
    int ii;
    for (ii = 0; ii < NUM_ITER_MEM_ORD; ++ii) {
        // loop until y = 1
        while (hsa_signal_load_acquire(y) != 1);

        // set z to 0
        hsa_signal_store_release(z, 1);

        // loop until z = 1
        while (hsa_signal_load_acquire(z) != 0);

        // set y to 0
        hsa_signal_store_release(y, 0);
    }

    // set y to -1 until t1 finish to avoid deadlock
    while (hsa_signal_load_relaxed(y) != 1);
    hsa_signal_store_release(y, -1);
    hsa_signal_store_release(z, -1);

    return;
}

int test_signal_store_release_load_acquire_ordering_transitive() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    int ii;
    for (ii = 0; ii < NUM_X; ++ii) {
        // initialize all x values to 2
        status = hsa_signal_create(0, 0, NULL, x + ii);
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    // initialize y to 0
    status = hsa_signal_create(0, 0, NULL, &y);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // initialize z to 1
    status = hsa_signal_create(0, 0, NULL, &z);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // create test_group
    struct test_group *test = test_group_create(3);

    // add test func one to the test group
    test_group_add(test, test_signal_store_release_load_acquire_ordering_t1, NULL, 1);

    // add test func two to the test_group
    test_group_add(test, test_signal_store_release_load_acquire_ordering_t2, NULL, 1);

    // add test func three to the test_group
    test_group_add(test, test_signal_store_release_load_acquire_ordering_t3, NULL, 1);

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

    status = hsa_signal_destroy(z);
    ASSERT(status == HSA_STATUS_SUCCESS);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);
    return 0;
}
