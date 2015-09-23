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
 * Test Name: signal_wait_store
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_wait API will respond appropriately
 * to signal value changes made by the hsa_signal_store API.
 *
 * Test Description:
 * 1) Create a signal with an initial value of 0.
 * 2) Create several threads
 *    a) Each thread should specify await condition that requires
 *       a signal value that won't awake any of the other threads.
 *    b) Each thread should wait on the signal with that condition.
 * 3) In the main thread, use the various flavors of hsa_signal_store
 * to satisfy those conditions, one at a time.
 * 4) For each modification of the signal value, check to see if the
 * appropriate thread, and only the appropriate thread, finished
 * waiting.
 * 4) Repeat for all versions of the hsa_signal_wait, using all of the
 * memory ordering variants.
 *
 * Expected Results: Only the thread that satisfies a specific condition
 * should quit waiting.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "config.h"

int num_waked_thread = 0;

pthread_mutex_t test_mutex;

typedef struct {
    int signal_wait_val;
    hsa_signal_t signal;
} param;

// test function for hsa_signal_wait_acquire
void test_signal_wait_acquire(void *data) {
    param* param_ptr = (param*) data;
    int signal_wait_val = param_ptr->signal_wait_val;
    hsa_signal_t signal = param_ptr->signal;
    hsa_signal_value_t signal_val = hsa_signal_wait_acquire(signal, HSA_SIGNAL_CONDITION_EQ, signal_wait_val, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

    // increment the number of waked-up threads
    pthread_mutex_lock(&test_mutex);
    num_waked_thread += 1;
    pthread_mutex_unlock(&test_mutex);
}

// test function for hsa_signal_wait_relaxed
void test_signal_wait_relaxed(void *data) {
    param* param_ptr = (param*) data;
    int signal_wait_val = param_ptr->signal_wait_val;
    hsa_signal_t signal = param_ptr->signal;

    hsa_signal_value_t signal_val = hsa_signal_wait_relaxed(signal, HSA_SIGNAL_CONDITION_EQ, signal_wait_val, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    // increment the number of waked-up threads
    pthread_mutex_lock(&test_mutex);
    num_waked_thread += 1;
    pthread_mutex_unlock(&test_mutex);
}

int test_signal_wait_store(int use_release) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    hsa_signal_t signal;
    status = hsa_signal_create(0, 0, NULL, &signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    int *wait_signal_vals = (int *)malloc(sizeof(int) * NUM_THREADS);

    // create threads, one half with hsa_signal_wait_relaxed, one half with
    // hsa_signal_wait_acquire
    struct test_group *tg_signal_wt = test_group_create(NUM_THREADS);

    // Declare the parameter array
    param params[NUM_THREADS];

    int ii;
    for (ii = 0; ii < NUM_THREADS; ++ii) {
        params[ii].signal = signal;
        params[ii].signal_wait_val = ii + 1;
        test_group_add(tg_signal_wt, ii % 2 ? &test_signal_wait_acquire : &test_signal_wait_relaxed, &params[ii], 1);
    }
    pthread_mutex_init(&test_mutex, NULL);

    test_group_thread_create(tg_signal_wt);

    test_group_start(tg_signal_wt);

    // increment signal value to wake up corresponding threads
    for (ii = 0; ii < NUM_THREADS; ++ii) {
        if (use_release) {
            hsa_signal_store_release(signal, ii + 1);
        } else {
            hsa_signal_store_relaxed(signal, ii + 1);
        }

        // wait until threads ii wake up
        while (test_group_test_status(tg_signal_wt, ii) != TEST_STOP);

        // check if the number of waked-up thread equals to number of signal
        // value set-up.
        ASSERT_MSG(num_waked_thread == ii + 1, "more than one thread has been waked up\n");
    }

    test_group_exit(tg_signal_wt);
    test_group_destroy(tg_signal_wt);

    status = hsa_signal_destroy(signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free(wait_signal_vals);
    return 0;
}

int test_signal_wait_store_relaxed() {
    return test_signal_wait_store(0);
}

int test_signal_wait_store_release() {
    return test_signal_wait_store(1);
}
