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
 * Test Name: signal_store_release_load_acquire_ordering
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
 * 2) Create a control signal, denoted by y, also initialized
 * to 0.
 * 3) Start one thread that
 *    a) Check the value of y in a loop using hsa_load_acquire.
 *    b) When the value of y is 0, the thread stops looping and
 *    c) Checks all of the x signal values with the signal_load_relaxed
 *    API, expecting a value of 0.
 *    d) Changes all of the x signal values to 1 using the
 *    signal_store_relaxed API.
 *    e) Sets the value of y to 1, using the signal_store_release API.
 *    f) Starts over.
 * 4) Start a second thread that does exactly the same set of operations,
 * but uses 0 in place of 1 and 1 in place of 0.
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
hsa_signal_t x[NUM_X];

void test_signal_store_release_load_acquire_t1(void *data) {
    int ii, jj;
    for (ii = 0; ii < NUM_ITER_MEM_ORD; ++ii) {
        // check if y equals to 0
        while (hsa_signal_load_acquire(y) != 0);

        for (jj = 0; jj < NUM_X; ++jj) {
            // every x value should equal to 0
            hsa_signal_value_t sig_val_t1 = hsa_signal_load_relaxed(x[jj]);
            ASSERT(sig_val_t1 == 0);

            // change x value to 1
            hsa_signal_store_relaxed(x[jj], 1);
        }

        // change y to 1
        hsa_signal_store_release(y, 1);
    }
    return;
}

void test_signal_store_release_load_acquire_t2(void *data) {
    int ii, jj;
    for (ii = 0; ii < NUM_ITER_MEM_ORD; ++ii) {
        // check if y equals to 1
        while (hsa_signal_load_acquire(y) != 1);

        for (jj = 0; jj < NUM_X; ++jj) {
            // every x value should equal to 1
            hsa_signal_value_t sig_val_t1 = hsa_signal_load_relaxed(x[jj]);
            ASSERT(sig_val_t1 == 1);

            // change x value to 0
            hsa_signal_store_relaxed(x[jj], 0);
        }

        // change y to 0
        hsa_signal_store_release(y, 0);
    }
    return;
}

int test_signal_store_release_load_acquire_ordering() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    int ii;
    for (ii = 0; ii < NUM_X; ++ii) {
        // initialize every x value to 1
        status = hsa_signal_create(1, 0, NULL, &x[ii]);
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    // initialize y to 1
    status = hsa_signal_create(1, 0, NULL, &y);
    ASSERT(status == HSA_STATUS_SUCCESS);

    struct test_group *tg_t1 = test_group_create(1);
    test_group_add(tg_t1, test_signal_store_release_load_acquire_t1, NULL, 1);
    test_group_thread_create(tg_t1);

    struct test_group *tg_t2 = test_group_create(1);
    test_group_add(tg_t2, test_signal_store_release_load_acquire_t2, NULL, 1);
    test_group_thread_create(tg_t2);

    test_group_start(tg_t1);
    test_group_start(tg_t2);

    test_group_wait(tg_t1);
    test_group_wait(tg_t2);

    test_group_exit(tg_t1);
    test_group_exit(tg_t2);

    test_group_destroy(tg_t1);
    test_group_destroy(tg_t2);

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
