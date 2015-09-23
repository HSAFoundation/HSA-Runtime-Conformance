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
 * Test Name: signal_wait_conditions
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_wait APIs properly use all of the
 * hsa_signal_condition_t specifiers.
 *
 * Test Description:
 * 1) Create a signal.
 * 2) For each of the hsa_signal_condition_t specifiers
 *    a) Set the signal's value and a compare value such that the
 *    condition is not satisfied.
 *    b) Wait on the signal using one of the hsa_signal_wait APIs
 *    c) In another thread, modify the signal value to satisfy the
 *    condition.
 * 3) Repeat this for all hsa_signal_wait APIs
 *
 * Expected Results: The waiting thread should return from the wait API
 * when the signal value satisfies the condition.
 *
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>

typedef struct wait_cb_s {
    hsa_signal_t signal;
    hsa_signal_t response_signal;
    hsa_signal_condition_t wait_condition;
    hsa_signal_value_t wait_value;
    hsa_signal_value_t response_value;
} wait_cb_t;

void signal_wait_relaxed_condition(void* data) {
    wait_cb_t* wait_cb = (wait_cb_t*) data;
    hsa_signal_wait_relaxed(wait_cb->signal, wait_cb->wait_condition, wait_cb->wait_value,
                            UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    hsa_signal_store_relaxed(wait_cb->response_signal, wait_cb->response_value);

    return;
}

void signal_wait_acquire_condition(void* data) {
    wait_cb_t* wait_cb = (wait_cb_t*) data;
    hsa_signal_wait_acquire(wait_cb->signal, wait_cb->wait_condition,wait_cb->wait_value,
                            UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    hsa_signal_store_release(wait_cb->response_signal, wait_cb->response_value);

    return;
}

void do_signal_wait_conditions(hsa_signal_condition_t wait_condition,
                                 hsa_signal_value_t first_wait_value,
                                 hsa_signal_value_t first_set_value,
                                 hsa_signal_value_t second_wait_value,
                                 hsa_signal_value_t second_set_value) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal;
    status = hsa_signal_create(0, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t response_signal;
    status = hsa_signal_create(0, 0, NULL, &response_signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Initialize the control blocks
    int ii;
    wait_cb_t wait_cb[2];
    for (ii = 0; ii < 2; ++ii) {
        wait_cb[ii].signal = signal;
        wait_cb[ii].response_signal = response_signal;
        wait_cb[ii].wait_condition = wait_condition;
    }

    wait_cb[0].wait_value = first_wait_value;
    wait_cb[0].response_value = first_set_value;
    wait_cb[1].wait_value = second_wait_value;
    wait_cb[1].response_value = second_set_value;

    // Initialize the thread group
    struct test_group *test_group = test_group_create(2);

    // Add the specific scenarios
    test_group_add(test_group, signal_wait_acquire_condition, &wait_cb[0], 1);
    test_group_add(test_group, signal_wait_relaxed_condition, &wait_cb[1], 1);

    test_group_thread_create(test_group);
    test_group_start(test_group);

    // Set the first value
    hsa_signal_store_relaxed(signal, first_set_value);
    // Wait for the response
    hsa_signal_wait_relaxed(response_signal, HSA_SIGNAL_CONDITION_EQ, first_set_value, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

    // Set the second value
    hsa_signal_store_relaxed(signal, second_set_value);
    // Wait for the response
    hsa_signal_wait_relaxed(response_signal, HSA_SIGNAL_CONDITION_EQ, second_set_value, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

    test_group_wait(test_group);
    test_group_exit(test_group);
    test_group_destroy(test_group);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

int test_signal_wait_conditions_eq() {
    do_signal_wait_conditions(HSA_SIGNAL_CONDITION_EQ, 1, 1, 2, 2);
    return 0;
}

int test_signal_wait_conditions_ne() {
    do_signal_wait_conditions(HSA_SIGNAL_CONDITION_NE, 0, 1, 1, 2);
    return 0;
}

int test_signal_wait_conditions_lt() {
    do_signal_wait_conditions(HSA_SIGNAL_CONDITION_LT, 0, -1, -1, -2);
    return 0;
}

int test_signal_wait_conditions_gte() {
    do_signal_wait_conditions(HSA_SIGNAL_CONDITION_GTE, 1, 1, 2, 2);
    do_signal_wait_conditions(HSA_SIGNAL_CONDITION_GTE, 1, 2, 3, 4);
    return 0;
}

int test_signal_wait_conditions() {
    test_signal_wait_conditions_eq();
    test_signal_wait_conditions_ne();
    test_signal_wait_conditions_lt();
    test_signal_wait_conditions_gte();
    return 0;
}
