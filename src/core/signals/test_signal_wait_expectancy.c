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
 * Test Name: signal_wait_expectancy
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_wait APIs will perform
 * properly when passed all values of the hsa_wait_expectancy_t
 * parameter.
 *
 * Test Description:
 * 1) Create a signal.
 * 2) Specify a value, condition operator and compare value that
 *    create a unsatisfied condition.
 * 3) Wait on the signal, using one of the hsa_wait_expectancy_t
 *    values.
 * 4) In another thread, set the signal value so the condition is
 *    satisfied.
 * 5) Repeat this for all possible hsa_wait_expectancy_t values.
 *
 * Expected Results: The wait API should return once the signal
 * condition is satisfied.
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>

typedef struct wait_cb_s {
    hsa_signal_t signal;
    hsa_signal_t response_signal;
    hsa_wait_state_t wait_expectancy;
    hsa_signal_value_t wait_value;
    hsa_signal_value_t response_value;
} wait_cb_t;

void signal_wait_expectancy(void* data) {
    wait_cb_t* wait_cb = (wait_cb_t*) data;
    hsa_signal_wait_relaxed(wait_cb->signal, HSA_SIGNAL_CONDITION_EQ, wait_cb->wait_value,
                            UINT64_MAX, wait_cb->wait_expectancy);
    hsa_signal_store_release(wait_cb->response_signal, wait_cb->response_value);

    return;
}


int test_signal_wait_expectancy() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signals[2];
    hsa_signal_t response_signals[2];
    wait_cb_t wait_cbs[2];
    hsa_signal_value_t wait_value = 1;
    int ii;
    for (ii = 0; ii < 2; ++ii) {
        // Initialize the signals with 0
        status = hsa_signal_create(0, 0, NULL, signals + ii);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_signal_create(0, 0, NULL, response_signals + ii);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Initialize the callback data structures
        wait_cbs[ii].signal = signals[ii];
        wait_cbs[ii].response_signal = response_signals[ii];
        wait_cbs[ii].wait_value = wait_value;
        wait_cbs[ii].response_value = wait_value;
    }

    wait_cbs[0].wait_expectancy = HSA_WAIT_STATE_BLOCKED;
    wait_cbs[1].wait_expectancy = HSA_WAIT_STATE_ACTIVE;

    // Initialize the thread group
    struct test_group *test_group = test_group_create(2);

    // Add the specific scenarios
    test_group_add(test_group, signal_wait_expectancy, &wait_cbs[0], 1);
    test_group_add(test_group, signal_wait_expectancy, &wait_cbs[1], 1);

    test_group_thread_create(test_group);
    test_group_start(test_group);

    // Set the signal values
    hsa_signal_store_relaxed(signals[0], wait_value);
    hsa_signal_store_relaxed(signals[1], wait_value);

    // Wait for the response
    hsa_signal_wait_relaxed(response_signals[0], HSA_SIGNAL_CONDITION_EQ, wait_value, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    hsa_signal_wait_relaxed(response_signals[1], HSA_SIGNAL_CONDITION_EQ, wait_value, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

    test_group_wait(test_group);
    test_group_exit(test_group);
    test_group_destroy(test_group);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
