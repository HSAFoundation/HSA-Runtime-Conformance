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
 * Test Name: signal_xor_acq_rel_ordering
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_xor_acq_rel API enforces
 * correct memory ordering.
 *
 * Test Description:
 * 1) Create 1024 signals and store the handles in an array,
 * denoted by x[1024]. All the signal values should be initialized to have
 * the last bit set.
 * 2) Create a control signal, denoted by y, also initialized to have only
 * the last bit set.
 * 3) Start one thread that
 *    a) Checks the value of y in a loop using hsa_cas_acq_rel using
 *    0 as the exchange value.
 *    b) When the value of y has only the last bit set, the thread stops looping, and
 *    c) Checks all of the x signal values with the signal_load_relaxed
 *    API, expecting all x values to have their last bit set.
 *    d) Replaces the x values by using signal_store_relaxed to set them to 0,
 *    and then uses signal_or_relaxed to set the first bit only.
 *    d) Replaces the value of y by using signal_load_relaxed to set the value to 0,
 *    and then uses signal_xor_acq_rel to set the first bit only.
 *    f) Starts over.
 * 4) Start a second thread that does exactly the same set of operations,
 * but sets the last bit for all values, not the first, and triggers when
 * the first bit of y is set, not the last.
 * 5) Let both threads run for 32K iterations.
 *
 * Expected Results: For each cycle all memory operations
 * that occured in thread one should be appropriately ordered in thread two after
 * the y signal value was modified, and vice versa.
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>
#include "config.h"

hsa_signal_t y;
hsa_signal_t x[NUM_X];

// test func one:
// check y, if the last bit of y has been set, set y to 0, and then check if last bits of all x have been set and set the first bit of all x, and then set the first bit of y
void test_signal_xor_acq_rel_t1(void *data) {
    int ii, jj;
    // repeat NUM_ITER_MEM_ORD times
    for (jj = 0; jj < NUM_ITER_MEM_ORD; ++jj) {
        // change y to 0 if y equals to 1
        while (hsa_signal_cas_acq_rel(y, LAST_BIT, 0) != LAST_BIT);

        for (ii = 0; ii < NUM_X; ++ii) {
            // every x value should equal to 1
            hsa_signal_value_t sig_val_t1 = hsa_signal_load_relaxed(x[ii]);
            ASSERT_MSG(sig_val_t1 == LAST_BIT, "the last bit of the signal value should be set\n");

            // change x value to 0
            hsa_signal_store_relaxed(x[ii], 0);

            // set first bit of x's
            hsa_signal_or_relaxed(x[ii], FIRST_BIT);
        }

        // set first bit of y
        hsa_signal_xor_acq_rel(y, FIRST_BIT);
    }
    return;
}


// test func two:
// check y, if the first bit of y has been set, set y to 0, and then check if first bits of all x have been set and set the last bit of all x, and then set the last bit of y
void test_signal_xor_acq_rel_t2(void *data) {
    int ii, jj;
    // repeat NUM_ITER_MEM_ORD times
    for (jj = 0; jj < NUM_ITER_MEM_ORD; ++jj) {
        // change y to 0 if y equals to 2
        while (hsa_signal_cas_acq_rel(y, FIRST_BIT, 0) != FIRST_BIT);

        for (ii = 0; ii < NUM_X; ++ii) {
            // every x value should equal to 0x80000000 on 32bit machine or 0x8000000000000000 on 64bit machine
            hsa_signal_value_t sig_val_t2 = hsa_signal_load_relaxed(x[ii]);
            ASSERT_MSG(sig_val_t2 == FIRST_BIT, "the first bit of the signal value should be set\n");

            // change x value to 0
            hsa_signal_store_relaxed(x[ii], 0);

            // set last bit of x's
            hsa_signal_or_relaxed(x[ii], LAST_BIT);
        }

        // set last bit of y
        hsa_signal_xor_acq_rel(y, LAST_BIT);
    }
    return;
}

int test_signal_xor_acq_rel_ordering() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    int ii;
    for (ii = 0; ii < NUM_X; ++ii) {
        // initialize all x with setting last bit to 1
        status = hsa_signal_create(LAST_BIT, 0, NULL, x + ii);
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    // initialize y with setting last bit to 1
    status = hsa_signal_create(LAST_BIT, 0, NULL, &y);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // create test_group
    struct test_group *test = test_group_create(2);

    // add test func one to the test group
    test_group_add(test, test_signal_xor_acq_rel_t1, NULL, 1);

    // add test func two to the test_group
    test_group_add(test, test_signal_xor_acq_rel_t2, NULL, 1);

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
