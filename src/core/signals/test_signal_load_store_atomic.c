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
 * Test Name: signal_load_store_atomic
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_load and store operations are atomic,
 * and 'torn' loads or stores do not occur when these APIs are executed
 * concurrently.
 *
 * Test Description:
 * 1) Create a signal.
 * 2) Create 2 threads, that
 *    a) Update the signal value, the first to 0 and the second to INT(32|64)_MAX.
 * 3) Create 2 threads, that
 *    b) Read the signal value, and check if it 0 or INT(32|64)_MAX.
 * 4) Run the threads for millions of iterations of loads and stores, with no
 * explicit synchronization between the threads.
 * 5) Repeat for all versions of the hsa_signal_load and store APIs, i.e. acquire,
 * release and relaxed memory ordering versions.
 *
 * Expected Results: The reading threads should only see two possible signal values,
 * 0 or INT(32|64)_MAX.
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>

#ifdef HSA_LARGE_MODEL
#define MAX_VAL INT64_MAX
#else
#define MAX_VAL INT32_MAX
#endif

#define NUM_ITER 10000

// Set signal value to zero
void set_signal_zero(void *data) {
    hsa_signal_t signal = (*(hsa_signal_t *)data);
    int rand_num = rand() % 2;
    // Randomly choose store function
    if (rand_num)
        hsa_signal_store_relaxed(signal, 0);
    else
        hsa_signal_store_release(signal, 0);
}

// Set signal value to INT(32|64)_MAX
void set_signal_max(void *data) {
    hsa_signal_t signal = (*(hsa_signal_t *)data);
    int rand_num = rand() % 2;
    // Randomly choose store function
    if (rand_num)
        hsa_signal_store_relaxed(signal, MAX_VAL);
    else
        hsa_signal_store_release(signal, MAX_VAL);
}

// Read signal value and check if the returned value is either ZERO or MAX
void read_signal(void *data) {
    hsa_signal_t signal = (*(hsa_signal_t *)data);
    int rand_num = rand() % 2;
    hsa_signal_value_t signal_val;
    // Randomly choose load function
    if (rand_num)
        signal_val = hsa_signal_load_acquire(signal);
    else
        signal_val = hsa_signal_load_relaxed(signal);
    ASSERT(signal_val == 0 || signal_val == MAX_VAL);
}

int test_signal_load_store_atomic() {
    // Init hsa_runtime
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    // Create signal
    hsa_signal_t signal;
    status = hsa_signal_create(1, 0, NULL, &signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // Create 2 threads for setting signal value, one is to set signal value to zero,
    // one is to set signal value to MAX
    struct test_group *tg_set_signal = test_group_create(2);
    test_group_add(tg_set_signal, &set_signal_zero, &signal, 1);
    test_group_add(tg_set_signal, &set_signal_max, &signal, 1);
    test_group_thread_create(tg_set_signal);


    // Create 2 threads for loading signal value
    struct test_group *tg_load_signal = test_group_create(2);
    test_group_add(tg_load_signal, &read_signal, &signal, 2);
    test_group_thread_create(tg_load_signal);

    int ii;
    for (ii = 0; ii < NUM_ITER; ++ii) {
        // Start threads for setting signal value and wait them finish
        test_group_start(tg_set_signal);
        test_group_wait(tg_set_signal);

        // Start threads for reading signal value
        test_group_start(tg_load_signal);
        test_group_wait(tg_load_signal);
    }

    // Exit threads
    test_group_exit(tg_set_signal);
    test_group_exit(tg_load_signal);

    // Cleanup resources
    test_group_destroy(tg_set_signal);
    test_group_destroy(tg_load_signal);

    status = hsa_signal_destroy(signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    status= hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}
