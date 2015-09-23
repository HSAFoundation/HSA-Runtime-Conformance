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
 * Test Name: signal_and_acq_rel_ordering_transitive
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_and_acq_rel API
 * enforces transitive memory ordering.
 *
 * Test Description:
 * 1) Create several signals and store the handles in an array,
 * denoted by x[]. All the signal values should be initialized
 * to have their first bit set.
 * 2) Create two control signals, denoted by y and z. The initial
 * y signal value should have its last bit set and the z signal
 * value should be 0.
 * 3) Start one thread that
 *    a) Uses signal_load_relaxed to load the value of y in a loop, stopping
 *    when the value has the last bit set.
 *    b) Uses signal_and_acq_rel to set the value of y to 0.
 *    c) Checks all of the x signal values with the signal_load_relaxed
 *    API, expecting each value to have the last bit set.
 *    d) Replaces the x values by using signal_load_relaxed to set all bits,
 *    and then uses signal_and_relaxed to mask out all bits but the first.
 *    e) Replaces the value of y by using signal_store_relaxed to set all bits,
 *    and then uses signal_and_acq_rel to mask out all bits but the first.
 *    f) Starts over.
 *    g) If it detects that the value of y is -2, it terminates.
 * 4) Start a second thread that does exactly the same set of operations,
 * but sets the last bit for all values, not the first, and triggers when
 * the first bit of z is set, not the last. It also operates on signal z,
 * not y.
 * 5) Start a third thread that
 *    a) Waits until y has its first bit set using signal_wait_acquire.
 *    c) Changes the value of z, using signal_store_relaxed to set all
 *    bits and then signal_and_release to mask out all but the first.
 *    e) Waits until z has only its last bit set with signal_wait_acquire.
 *    f) Changes the value of y, using signal_store_relaxed to set all bits
 *    and then signal_and_release to mask out all but the last.
 *    g) Starts over.
 *    h) After a set number of iterations the third thread should set
 *       both y and z signal values to -2 and terminate.
 * 5) Let both threads run for thousands of iterations.
 *
 * Expected Results: For each cycle all memory operations that occured in thread one should
 * be appropriately ordered in thread two after the y signal value was modified, and vice versa.
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>
#include "config.h"

hsa_signal_t y;
hsa_signal_t z;
hsa_signal_t x[NUM_X];

void test_signal_and_acq_rel_ordering_t1(void *data) {
    int ii;
    while (1) {
        hsa_signal_value_t y_val = 0;
        // loop until last bit of y has been set or y = -1
        while ((y_val = hsa_signal_load_relaxed(y)) != LAST_BIT)
            if (y_val == -2) return;

        hsa_signal_and_acq_rel(y, 0);

        for (ii = 0; ii < NUM_X; ++ii) {
            // only last bit of every x should be set
            hsa_signal_value_t sig_val_t1 = hsa_signal_load_relaxed(x[ii]);
            ASSERT_MSG(sig_val_t1 == LAST_BIT, "only last bit of x value should be set\n");

            // change first bit of x to 1
            hsa_signal_store_relaxed(x[ii], ALL_BIT);
            hsa_signal_and_relaxed(x[ii], FIRST_BIT);
        }

        // set first bit of y
        hsa_signal_store_relaxed(y, ALL_BIT);
        hsa_signal_and_acq_rel(y, FIRST_BIT);
    }
    return;
}


void test_signal_and_acq_rel_ordering_t2(void *data) {
    int ii;
    while (1) {
        hsa_signal_value_t z_val = 0;
        // loop until first bit of z has been set or z = -1
        while ((z_val = hsa_signal_load_relaxed(z)) != FIRST_BIT)
            if (z_val == -2) return;

        hsa_signal_and_acq_rel(z, 0);

        for (ii = 0; ii < NUM_X; ++ii) {
            // only first bit of every x should be set
            hsa_signal_value_t sig_val_t2 = hsa_signal_load_relaxed(x[ii]);
            ASSERT_MSG(sig_val_t2 == FIRST_BIT, "only first bit of x value should be set\n");

            // change last bit of x
            hsa_signal_store_relaxed(x[ii], ALL_BIT);
            hsa_signal_and_relaxed(x[ii], LAST_BIT);
        }

        // set last bit of z
        hsa_signal_store_relaxed(z, ALL_BIT);
        hsa_signal_and_acq_rel(z, LAST_BIT);
    }
    return;
}


void test_signal_and_acq_rel_ordering_t3(void *data) {
    int ii;
    for (ii = 0; ii < NUM_ITER_MEM_ORD; ++ii) {
        // wait until first bit of y has been set up
        hsa_signal_wait_acquire(y, HSA_SIGNAL_CONDITION_EQ, FIRST_BIT, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

        // set first bit of z
        hsa_signal_store_relaxed(z, ALL_BIT);
        hsa_signal_and_release(z, FIRST_BIT);

        // wait until last bit of z has been set up
        hsa_signal_wait_acquire(z, HSA_SIGNAL_CONDITION_EQ, LAST_BIT, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

        // set last bit of y to 1
        hsa_signal_store_relaxed(y, ALL_BIT);
        hsa_signal_and_release(y, LAST_BIT);
    }

    // set y to -1 until t1 finish to avoid deadlock
    while (hsa_signal_load_relaxed(y) != FIRST_BIT);
    hsa_signal_store_release(y, -2);
    hsa_signal_store_release(z, -2);
    return;
}

int test_signal_and_acq_rel_ordering_transitive() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    int ii;
    for (ii = 0; ii < NUM_X; ++ii) {
        // initialize all x values with setting last bit
        status = hsa_signal_create(LAST_BIT, 0, NULL, x + ii);
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    // initialize y to 0
    status = hsa_signal_create(LAST_BIT, 0, NULL, &y);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // initialize z to 1
    status = hsa_signal_create(0, 0, NULL, &z);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // create test_group
    struct test_group *test = test_group_create(3);

    // add test func one to the test group
    test_group_add(test, test_signal_and_acq_rel_ordering_t1, NULL, 1);

    // add test func two to the test_group
    test_group_add(test, test_signal_and_acq_rel_ordering_t2, NULL, 1);

    // add test func three to the test_group
    test_group_add(test, test_signal_and_acq_rel_ordering_t3, NULL, 1);

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
